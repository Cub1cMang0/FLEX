cmake_minimum_required(VERSION 3.16)
project(FLEX VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(WIN_ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/resource.rc")
set(MAC_ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/app.icns")

set(PROJECT_SOURCES
    source/main.cpp
    source/mainwindow.cpp
    source/mainwindow.h
    source/mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(FLEX
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        $<$<PLATFORM_ID:Windows>:${WIN_ICON_FILE}>
        $<$<PLATFORM_ID:Darwin>:${MAC_ICON_FILE}>
        source/imagefileconverter.h
        source/mainimageconverter.h source/mainimageconverter.cpp
        source/mainvideoconverter.h source/mainvideoconverter.cpp
        source/videofileconverter.h
        source/documentfileconverter.h
        source/maindocumentconverter.h source/maindocumentconverter.cpp
        source/json.hpp
        source/changesavelocation.h source/changesavelocation.cpp source/changesavelocation.ui
        source/archivefileconverter.h
        source/mainarchiveconverter.h source/mainarchiveconverter.cpp
        source/spreadfileconverter.h
        source/mainspreadconverter.h source/mainspreadconverter.cpp
        source/imagepreferences.h source/imagepreferences.cpp source/imagepreferences.ui
        source/videoaudiopreferences.h source/videoaudiopreferences.cpp source/videoaudiopreferences.ui
        source/documentpreferences.h source/documentpreferences.cpp source/documentpreferences.ui
        source/spreadsheetpreferences.h source/spreadsheetpreferences.cpp source/spreadsheetpreferences.ui
        source/archivepreferences.h source/archivepreferences.cpp source/archivepreferences.ui
        source/draganddrop.h
        source/bulkconvertmanager.h source/bulkconvertmanager.cpp
    )
else()
    if(ANDROID)
        add_library(FLEX SHARED ${PROJECT_SOURCES})
    else()
        add_executable(FLEX
            ${PROJECT_SOURCES}
            $<$<PLATFORM_ID:Windows>:${WIN_ICON_FILE}>
            $<$<PLATFORM_ID:Darwin>:${MAC_ICON_FILE}>
        )
    endif()
endif()

# ── LibArchive ────────────────────────────────────────────────────────────────

if(WIN32)
    set(LIBARCHIVE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/libarchive/win64")

    add_library(LibArchive::LibArchive SHARED IMPORTED)
    set_target_properties(LibArchive::LibArchive PROPERTIES
        IMPORTED_IMPLIB   "${LIBARCHIVE_DIR}/libarchive.dll.a"
        IMPORTED_LOCATION "${LIBARCHIVE_DIR}/libarchive.dll"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/libs/libarchive/include"
    )

    # Copy libarchive and all its dependency DLLs to the build output
    add_custom_command(TARGET FLEX POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${LIBARCHIVE_DIR}"
            "$<TARGET_FILE_DIR:FLEX>"
        COMMENT "Copying libarchive and dependencies to output directory"
    )

elseif(APPLE)
    set(LIBARCHIVE_DYLIB "${CMAKE_CURRENT_SOURCE_DIR}/libs/libarchive/macos/libarchive.dylib")

    add_library(LibArchive::LibArchive SHARED IMPORTED)
    set_target_properties(LibArchive::LibArchive PROPERTIES
        IMPORTED_LOCATION "${LIBARCHIVE_DYLIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/libs/libarchive/include"
    )

    add_custom_command(TARGET FLEX POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:FLEX>/../Frameworks"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LIBARCHIVE_DYLIB}"
            "$<TARGET_FILE_DIR:FLEX>/../Frameworks/libarchive.dylib"
        COMMENT "Copying libarchive.dylib into app bundle"
    )

    set_target_properties(FLEX PROPERTIES
        BUILD_RPATH   "@executable_path/../Frameworks"
        INSTALL_RPATH "@executable_path/../Frameworks"
    )

else() # Linux
    option(USE_SYSTEM_LIBARCHIVE "Use system libarchive instead of bundled" OFF)

    if(USE_SYSTEM_LIBARCHIVE)
        if(LibArchive_LIBRARY AND LibArchive_INCLUDE_DIR)
            # Use explicitly provided paths (e.g. from Flatpak)
            add_library(LibArchive::LibArchive SHARED IMPORTED)
            set_target_properties(LibArchive::LibArchive PROPERTIES
                IMPORTED_LOCATION "${LibArchive_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${LibArchive_INCLUDE_DIR}"
            )
        else()
            # Fall back to find_package for normal system builds
            find_package(LibArchive REQUIRED)
        endif()
    else()
        set(LIBARCHIVE_SO "${CMAKE_CURRENT_SOURCE_DIR}/libs/libarchive/linux/libarchive.so")

        add_library(LibArchive::LibArchive SHARED IMPORTED)
        set_target_properties(LibArchive::LibArchive PROPERTIES
            IMPORTED_LOCATION "${LIBARCHIVE_SO}"
            INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/libs/libarchive/include"
        )

        add_custom_command(TARGET FLEX POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${LIBARCHIVE_SO}" $<TARGET_FILE_DIR:FLEX>
            COMMENT "Copying libarchive.so to output directory"
        )

        set_target_properties(FLEX PROPERTIES
            BUILD_RPATH   "$ORIGIN"
            INSTALL_RPATH "$ORIGIN"
        )
    endif()
endif()

# ── Tools (ffmpeg, pandoc) ────────────────────────────────────────────────────

# ── Tools (ffmpeg, pandoc) ────────────────────────────────────────────────────

if(WIN32)
    set(TOOLS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tools/win64")
    set(TOOLS_DEST_SUBDIR "tools/win64")
elseif(APPLE)
    set(TOOLS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tools/macos")
    set(TOOLS_DEST_SUBDIR "tools/macos")
else()
    set(TOOLS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tools/linux")
    set(TOOLS_DEST_SUBDIR "tools/linux")
endif()

if(EXISTS "${TOOLS_SRC_DIR}")
    add_custom_command(TARGET FLEX POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${TOOLS_SRC_DIR}"
            "$<TARGET_FILE_DIR:FLEX>/${TOOLS_DEST_SUBDIR}"
        COMMENT "Copying tools to output directory"
    )
else()
    message(STATUS "Tools directory not found at ${TOOLS_SRC_DIR}, skipping copy")
endif()

# ── Linking ───────────────────────────────────────────────────────────────────

target_include_directories(FLEX PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source)
target_link_libraries(FLEX PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    LibArchive::LibArchive
)

# ── Bundle settings ───────────────────────────────────────────────────────────

if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.FLEX)
endif()

set_target_properties(FLEX PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE_ICON_FILE "app.icns"
)

if(APPLE)
    set_source_files_properties(${MAC_ICON_FILE} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
endif()

include(GNUInstallDirs)
install(TARGETS FLEX
    BUNDLE  DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(FLEX)
endif()
