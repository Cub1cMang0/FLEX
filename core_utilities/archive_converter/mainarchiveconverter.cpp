#include "mainarchiveconverter.h"
#include <archive.h>
#include <archive_entry.h>
#include <string>
#include <fstream>
#include <QFileDialog>
#include <QDebug>
#include "json.hpp"

MainArchiveConverter::MainArchiveConverter(QObject *parent)
    : QObject(parent)
{}

using json = nlohmann::json;
using namespace std;

bool set_output_format(struct archive* a, QString ext) {
    if (ext.toLower() == "zip") return archive_write_set_format_zip(a) == ARCHIVE_OK;
    if (ext.toLower() == "7z") return archive_write_set_format_7zip(a) == ARCHIVE_OK;
    if (ext.toLower() == "tar") return archive_write_set_format_pax_restricted(a) == ARCHIVE_OK;
    if (ext.toLower() == "xar") return archive_write_set_format_xar(a) == ARCHIVE_OK;
    if (ext.toLower() == "iso") return archive_write_set_format_iso9660(a) == ARCHIVE_OK;
    if (ext.toLower() == "cpio") return archive_write_set_format_cpio(a) == ARCHIVE_OK;
    if (ext.toLower() == "ar") return archive_write_set_format_ar_svr4(a) == ARCHIVE_OK;
    return false;
}

QString withSuffix(const QString &base, int n)
{
    QString suffix = QString::number(n);
    int maxBaseLen = 8 - suffix.size();
    return base.left(maxBaseLen) + suffix;
}

QString resolveCollision(
    const QString &base,
    const QString &ext,
    QSet<QString> &usedNames
    ) {
    QString candidate = ext.isEmpty() ? base : base + "." + ext;
    if (!usedNames.contains(candidate)) {
        usedNames.insert(candidate);
        return candidate;
    }
    for (int i = 1; i < 100; ++i) {
        QString newBase = withSuffix(base, i);
        QString name = ext.isEmpty() ? newBase : newBase + "." + ext;
        if (!usedNames.contains(name)) {
            usedNames.insert(name);
            return name;
        }
    }
    QString fallback = base.left(6) + "_X";
    QString name = ext.isEmpty() ? fallback : fallback + "." + ext;
    usedNames.insert(name);
    return name;
}

void sanitizeComponent(
    const QString &name,
    QString &base,
    QString &ext,
    const json &data
    ) {
    int dot = name.lastIndexOf('.');
    if (dot > 0) {
        base = name.left(dot);
        ext  = name.mid(dot + 1);
    } else {
        base = name;
        ext.clear();
    }
    auto &iso = data["archive"]["iso"];
    auto sanitize = [&](const QString &s, int maxLen) {
        QString out;
        for (QChar c : s) {
            QChar ch = c;
            if (iso["uppercase"][0] || iso["enforce_83"][0])
                ch = ch.toUpper();
            bool valid =
                (ch >= 'A' && ch <= 'Z') ||
                (ch >= 'a' && ch <= 'z') ||
                (ch >= '0' && ch <= '9') ||
                ch == '_' || ch == '-';
            if (!valid) {
                if (iso["rm_invalid"][0])
                    ch = '_';
                else
                    continue;
            }
            out.append(ch);
            if (iso["enforce_83"][0] && out.size() == maxLen)
                break;
        }
        return out;
    };
    base = sanitize(base, iso["enforce_83"][0] ? 8 : INT_MAX);
    ext  = sanitize(ext,  iso["enforce_83"][0] ? 3 : INT_MAX);
}

QString sanitizeArName(const QString &input, QSet<QString> &used, bool is_gnu_format)
{
    QString name = QFileInfo(input).fileName();
    QString base, ext;
    int dot = name.lastIndexOf('.');
    if (dot > 0) {
        base = name.left(dot);
        ext  = name.mid(dot + 1);
    } else {
        base = name;
    }
    auto clean = [](const QString &s) {
        QString out;
        for (QChar c : s) {
            if ((c >= 'A' && c <= 'Z') ||
                (c >= 'a' && c <= 'z') ||
                (c >= '0' && c <= '9') ||
                c == '_' || c == '-' || c == '.')
                out += c;
        }
        return out;
    };
    base = clean(base);
    ext  = clean(ext);
    if (base.isEmpty())
        base = "file";
    QString candidate = ext.isEmpty() ? base : base + "." + ext;
    if (!is_gnu_format && candidate.size() > 15) {
        int maxBase = 15 - (ext.isEmpty() ? 0 : ext.size() + 1);
        base = base.left(qMax(1, maxBase));
        candidate = ext.isEmpty() ? base : base + "." + ext;
    }
    if (!used.contains(candidate)) {
        used.insert(candidate);
        return candidate;
    }
    for (int i = 1; i < 1000; ++i) {
        QString suffix = QString::number(i);
        int maxBase = (is_gnu_format ? INT_MAX : 15)- suffix.size() - (ext.isEmpty() ? 0 : ext.size() + 1);
        QString b = base.left(qMax(1, maxBase)) + suffix;
        QString name = ext.isEmpty() ? b : b + "." + ext;
        if (!used.contains(name)) {
            used.insert(name);
            return name;
        }
    }
    QString fallback = "file" + QString::number(used.size());
    used.insert(fallback);
    return fallback;
}

QString sanitizeIsoPath(const QString &path, QHash<QString, QSet<QString>> &dirTables, QHash<QString, QString> &dirPathMap, bool isDirectory, const json &data) {
    QStringList parts = path.split('/', Qt::SkipEmptyParts);
    QStringList out;
    QString currentDir;
    QString originalDir;
    auto &iso = data["archive"]["iso"];
    for (int i = 0; i < parts.size(); ++i) {
        bool last = (i == parts.size() - 1);
        originalDir += "/" + parts[i];
        QString base, ext;
        sanitizeComponent(parts[i], base, ext, data);
        if (!last || isDirectory) {
            if (!dirPathMap.contains(originalDir)) {
                QSet<QString> &used = dirTables[currentDir];
                QString dirName;
                if (iso["enforce_83"][0]) {
                    dirName = resolveCollision(base, QString(), used);
                } else {
                    dirName = base;
                    used.insert(dirName);
                }
                dirPathMap[originalDir] = dirName;
            }
            QString dirName = dirPathMap[originalDir];
            out.append(dirName);
            currentDir += "/" + dirName;
        }
        else {
            QSet<QString> &used = dirTables[currentDir];
            QString fileName;
            if (iso["enforce_83"][0]) {
                fileName = resolveCollision(base, ext, used);
            } else {
                fileName = ext.isEmpty() ? base : base + "." + ext;
                used.insert(fileName);
            }
            out.append(fileName);
        }
    }
    return out.join('/');
}

void MainArchiveConverter::convert_archive(const QString &input_path, const QString &output_path, QString input_extension, QString output_extension)
{
    QFileInfo input_file_info(input_path);
    QString base_name = input_file_info.completeBaseName();
    QString suffix = input_file_info.suffix().toLower();
    QString full_input_ext = input_extension.toLower();
    QStringList compression_exts = {"gz", "bz2", "xz", "zst"};
    if (compression_exts.contains(suffix)) {
        QFileInfo temp_info(input_file_info.completeBaseName());
        QString inner_ext = temp_info.suffix().toLower();
        if (inner_ext == "tar" || inner_ext == "xar") {
            full_input_ext = inner_ext + "." + suffix;
            base_name = temp_info.completeBaseName();
        }
    }
    QString output_dir;
    QString output_filename;
    QFileInfo output_info(output_path);
    if (QDir(output_path).exists()) {
        output_dir = output_path;
        output_filename = base_name;
    } else {
        output_dir = output_info.absolutePath();
        output_filename = output_info.completeBaseName();
    }
    QDir().mkpath(output_dir);
    QString final_output = output_dir + "/" + output_filename + "." + output_extension.toLower();
    struct archive *input_file = archive_read_new();
    struct archive *output_file = archive_write_new();
    struct archive_entry *entry;
    archive_read_support_format_all(input_file);
    archive_read_support_filter_all(input_file);
    if (!set_output_format(output_file, output_extension))
    {
        QString error_msg = (output_extension == "XAR")
        ? "Cannot convert to xar due to OS discrepancies (Non Apple/MacOS)"
        : "Unsupported output format: " + output_extension;
        emit update_archive_progress(error_msg, false);
        archive_read_free(input_file);
        archive_write_free(output_file);
        return;
    }
    QString source_location = QString(__FILE__);
    QFileInfo file_info(source_location);
    QString json_path = file_info.absolutePath() + "/conversion_preferences.json";
    json load_data;
    std::ifstream save_json(json_path.toStdString());
    if (save_json.is_open()) {
        save_json >> load_data;
        save_json.close();
    }
    QString tar_ext;
    bool is_ar = false;
    bool enable_determ = false;
    if (load_data.contains("archive"))
    {
        auto archive_preferences = load_data["archive"];
        if (output_extension == "ZIP")
        {
            QString zip_comp_method = QString::fromStdString(archive_preferences["zip"]["comp_method"]).toLower();
            if (zip_comp_method != "n/a")
            {
                QString option;
                if (zip_comp_method == "deflate") option = "zip:compression=deflate";
                else if (zip_comp_method == "store") option = "zip:compression=store";
                else if (zip_comp_method == "bzip2") option = "zip:compression=bzip2";
                else if (zip_comp_method == "lzma") option = "zip:compression=lzma";
                else if (zip_comp_method == "zstd") option = "zip:compression=zstd";

                if (!option.isEmpty()) {
                    archive_write_set_options(output_file, option.toStdString().c_str());
                }
            }
            QString zip_comp_level = QString::fromStdString(archive_preferences["zip"]["comp_level"]).toLower();
            if (zip_comp_level != "n/a")
            {
                QString level_option;
                if (zip_comp_level == "store") level_option = "zip:compression-level=0";
                else if (zip_comp_level == "fast") level_option = "zip:compression-level=3";
                else if (zip_comp_level == "normal") level_option = "zip:compression-level=6";
                else if (zip_comp_level == "maximum") level_option = "zip:compression-level=9";

                if (!level_option.isEmpty()) {
                    archive_write_set_options(output_file, level_option.toStdString().c_str());
                }
            }
        }
        else if (output_extension == "TAR")
        {
            QString tar_comp_method = QString::fromStdString(archive_preferences["tar"]["comp_method"]).toLower();
            if (tar_comp_method != "n/a")
            {
                if (tar_comp_method == "gzip") {
                    archive_write_add_filter_gzip(output_file);
                    tar_ext = ".gz";
                }
                else if (tar_comp_method == "bzip2") {
                    archive_write_add_filter_bzip2(output_file);
                    tar_ext = ".bz2";
                }
                else if (tar_comp_method == "xz") {
                    archive_write_add_filter_xz(output_file);
                    tar_ext = ".xz";
                }
                else if (tar_comp_method == "zstd") {
                    archive_write_add_filter_zstd(output_file);
                    tar_ext = ".zst";
                }
            }
            else {
                archive_write_add_filter_none(output_file);
            }

            if (!tar_ext.isEmpty()) {
                final_output = output_dir + "/" + output_filename + ".tar" + tar_ext;
            }

            bool tar_pres_metadata = archive_preferences["tar"]["pres_metadata"][0];
            if (tar_pres_metadata) {
                archive_write_set_options(output_file, "hdrcharset=UTF-8");
            }
        }
        else if (output_extension == "XAR")
        {
            QString xar_comp_method = QString::fromStdString(archive_preferences["xar"]["comp_method"]).toLower();
            if (xar_comp_method != "n/a")
            {
                if (xar_comp_method == "gzip") archive_write_set_options(output_file, "compression=gzip");
                else if (xar_comp_method == "bzip2") archive_write_set_options(output_file, "compression=bzip2");
                else if (xar_comp_method == "xz") archive_write_set_options(output_file, "compression=xz");
            }
            else {
                archive_write_add_filter_none(output_file);
            }

            bool xar_pres_metadata = archive_preferences["xar"]["pres_metadata"][0];
            if (xar_pres_metadata)
            {
                archive_write_set_options(output_file, "xattr=true");
                archive_write_set_options(output_file, "acl=true");
            }
        }
        else if (output_extension == "CPIO")
        {
            QString cpio_format = QString::fromStdString(archive_preferences["cpio"]["format"]).toLower();
            if (cpio_format != "n/a")
            {
                if (cpio_format == "newc") archive_write_set_format_cpio_newc(output_file);
                else if (cpio_format == "odc") archive_write_set_format_cpio_odc(output_file);
            }

            bool cpio_pres_metadata = archive_preferences["cpio"]["pres_metadata"][0];
            if (cpio_pres_metadata) {
                archive_write_set_options(output_file, "hdrcharset=UTF-8");
            }
        }
        else if (output_extension == "AR")
        {
            QString ar_format = QString::fromStdString(archive_preferences["ar"]["format"]).toLower();
            if (ar_format != "n/a")
            {
                if (ar_format == "gnu") archive_write_set_format_ar_svr4(output_file);
                else archive_write_set_format_ar_bsd(output_file);
            }

            is_ar = true;
            bool determ_mode = archive_preferences["ar"]["determ_mode"][0];
            if (determ_mode) {
                enable_determ = true;
            }
        }
    }
    if (archive_write_open_filename(output_file, final_output.toStdString().c_str()) != ARCHIVE_OK)
    {
        QString error = archive_error_string(output_file)
        ? QString::fromUtf8(archive_error_string(output_file))
        : "Failed opening output file: " + final_output;
        emit update_archive_progress(error, false);
        archive_read_close(input_file);
        archive_read_free(input_file);
        archive_write_close(output_file);
        archive_write_free(output_file);
        return;
    }
    if (archive_read_open_filename(input_file, input_path.toStdString().c_str(), 10240) != ARCHIVE_OK)
    {
        QString error = archive_error_string(input_file)
        ? QString::fromUtf8(archive_error_string(input_file))
        : "Failed opening input file: " + input_path;
        emit update_archive_progress(error, false);
        archive_read_close(input_file);
        archive_read_free(input_file);
        archive_write_close(output_file);
        archive_write_free(output_file);
        return;
    }
    if (input_path.isEmpty())
    {
        emit update_archive_progress("No file selected", false);
        archive_read_close(input_file);
        archive_read_free(input_file);
        archive_write_close(output_file);
        archive_write_free(output_file);
        QFile::remove(final_output);
        return;
    }
    QHash<QString, QSet<QString>> isoDirTables;
    QHash<QString, QString> isoDirPathMap;
    QSet<QString> ar_used_names;
    bool is_gnu_ar = (output_extension == "AR" && load_data.contains("archive") &&
                      QString::fromStdString(load_data["archive"]["ar"]["format"]).toUpper() == "GNU");
    if (is_gnu_ar)
    {
        QStringList long_filenames;
        struct archive *scan_archive = archive_read_new();
        archive_read_support_format_all(scan_archive);
        archive_read_support_filter_all(scan_archive);
        if (archive_read_open_filename(scan_archive, input_path.toStdString().c_str(), 10240) == ARCHIVE_OK)
        {
            struct archive_entry *scan_entry;
            while (archive_read_next_header(scan_archive, &scan_entry) == ARCHIVE_OK)
            {
                QString filename = QFileInfo(QString::fromUtf8(archive_entry_pathname(scan_entry))).fileName();
                if (filename.length() > 15) {
                    long_filenames.append(filename);
                }
            }
        }
        archive_read_close(scan_archive);
        archive_read_free(scan_archive);
        if (!long_filenames.isEmpty())
        {
            QString strtab_content;
            for (const QString &fname : long_filenames) {
                strtab_content += fname + "/\n";
            }
            struct archive_entry *strtab_entry = archive_entry_new();
            archive_entry_set_pathname(strtab_entry, "//");
            archive_entry_set_size(strtab_entry, strtab_content.length());
            archive_entry_set_filetype(strtab_entry, AE_IFREG);
            archive_entry_set_perm(strtab_entry, 0644);
            if (archive_write_header(output_file, strtab_entry) == ARCHIVE_OK) {
                archive_write_data(output_file, strtab_content.toUtf8().constData(), strtab_content.length());
            }
            archive_entry_free(strtab_entry);
        }
    }
    while (archive_read_next_header(input_file, &entry) == ARCHIVE_OK)
    {
        const char *pathname = archive_entry_pathname(entry);
        if (pathname == NULL || *pathname == '\0') {
            const size_t buf_size = 8192;
            char buf[buf_size];
            while (archive_read_data(input_file, buf, buf_size) > 0) {
            }
            continue;
        }
        if (output_extension == "ISO")
        {
            if (load_data.contains("archive"))
            {
                const char *orig = archive_entry_pathname(entry);
                bool is_dir = archive_entry_filetype(entry) == AE_IFDIR;
                QString sanitized = sanitizeIsoPath(orig, isoDirTables, isoDirPathMap, is_dir, load_data);
                archive_entry_set_pathname(entry, sanitized.toUtf8().constData());
            }
        }
        else if (is_ar)
        {
            const char *orig = archive_entry_pathname(entry);
            QString orig_name = QString::fromUtf8(orig);
            QString file_name = QFileInfo(orig_name).fileName();
            if (is_gnu_ar) {
                archive_entry_set_pathname(entry, file_name.toUtf8().constData());
            }
            else {
                QString sanitized = sanitizeArName(orig_name, ar_used_names, false);
                archive_entry_set_pathname(entry, sanitized.toUtf8().constData());
            }
            if (enable_determ)
            {
                archive_entry_set_mtime(entry, 0, 0);
                archive_entry_set_atime(entry, 0, 0);
                archive_entry_set_ctime(entry, 0, 0);
                archive_entry_set_uid(entry, 0);
                archive_entry_set_gid(entry, 0);
                archive_entry_set_uname(entry, nullptr);
                archive_entry_set_gname(entry, nullptr);

                mode_t current_mode = archive_entry_mode(entry);
                if (S_ISDIR(current_mode)) {
                    archive_entry_set_perm(entry, 0755);
                }
                else {
                    mode_t new_perm = (current_mode & 0111) ? 0755 : 0644;
                    archive_entry_set_perm(entry, new_perm);
                }
            }
        }
        if (archive_write_header(output_file, entry) != ARCHIVE_OK) {
            qWarning() << "Write header failed:" << archive_error_string(output_file);
            continue;
        }
        const size_t buf_size = 8192;
        char buf[buf_size];
        ssize_t len;
        while ((len = archive_read_data(input_file, buf, buf_size)) > 0) {
            if (archive_write_data(output_file, buf, len) < 0) {
                qWarning() << "Write data failed:" << archive_error_string(output_file);
                break;
            }
        }
        if (len < 0) {
            qWarning() << "Read data failed:" << archive_error_string(input_file);
        }
    }
    archive_read_close(input_file);
    archive_read_free(input_file);
    archive_write_close(output_file);
    archive_write_free(output_file);
    QString final_ext = output_extension.toLower();
    if (output_extension == "TAR" && !tar_ext.isEmpty()) {
        final_ext = "tar" + tar_ext;
    }
    QString success_message = QString("Success: %1.%2 has been converted to %3.%4")
                                  .arg(base_name, full_input_ext, output_filename, final_ext);
    emit update_archive_progress(success_message, true);
}
