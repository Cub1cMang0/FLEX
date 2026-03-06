# FLEX
An all-in-one fast and efficient file converter.

## Main Features

1. **Multi-File-Type Support**
   - Images
     * The following file types are supported: png, jpeg, jpg, ico, jfif, pbm, pgm, ppm, bmp, cur, xmb, xpm
   - Video/Audio
     * The following file types are supported: mp4, mov, avi, wmv, mkv, mp3, wav, aiff, wma, flac, alac
   - Documents
     * The following file types are supported: docx, epub, html, json, md, latex, odt, rtf, rst, org, ipynb
   - Spreadsheets
     * The following file types are supported: xlsx, xml, csv, ods, fods
   - Archives
     * The following file types are supported: zip, 7z, tar, xar (on mac), iso, cpio, ar
   
2. **Two-Way File Conevrsions**
   - No need to worry that one image file type won't convert into another. The conversion process allows for any two files of the same file type category to convert into each other seamlessly.

3. **File Conversion Preferences**
   - Ever wanted to convert a file with a specific preference of yours in mind? Well, you are in luck because each file type category has a set of preferences to choose from to suite your conversion needs.

4. **Error Logging**
   - Do you wonder why your file conversion failed and actually want to know the reason? Good, because file conversions that fail will produce error logs for you to view and understand why it failed.
  
5. **Bulk File Conversions**
   - Sometimes you got to convert multiple files and you don't have time to convert each and every single one of them at a time. Files selected for conversion will have conversion statuses to view the progress or outcome of each file converted.
  
## Windows Installations

There are two main methods of installation

1. **Setup Executable**
   - Simply run the setup and follow the instructions in the executable.
3. **Portable**
   - A simple download and unzip to begin using FLEX.

## Linux Installation

1. Download the flatpak in the release tab.
2. Install flatpak (if not already installed)
   - sudo apt install flatpak
   - flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
4. Install KDE
   - flatpak install flathub org.kde.Platform//6.10
6. Install the flatpak
   - flatpak install FLEX.flatpak
8. Run the program
   - flatpak run io.github.Cub1cMang0.FLEX
