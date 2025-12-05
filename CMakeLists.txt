cmake_minimum_required(VERSION 3.16)

project(file_convert VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui


)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(file_convert
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        imagefileconverter.h
        mainimageconverter.h
        mainimageconverter.cpp
        mainvideoconverter.h mainvideoconverter.cpp
        videofileconverter.h
        documentfileconverter.h
        maindocumentconverter.h maindocumentconverter.cpp
        json.hpp
        changesavelocation.h changesavelocation.cpp changesavelocation.ui
        archivefileconverter.h
        mainarchiveconverter.h mainarchiveconverter.cpp
        spreadfileconverter.h
        mainspreadconverter.h mainspreadconverter.cpp
        draganddrop.h
        imagepreferences.h imagepreferences.cpp imagepreferences.ui
        videoaudiopreferences.h videoaudiopreferences.cpp videoaudiopreferences.ui
        documentpreferences.h documentpreferences.cpp documentpreferences.ui
        spreadsheetpreferences.h spreadsheetpreferences.cpp spreadsheetpreferences.ui



    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET file_convert APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(file_convert SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(file_convert
            ${PROJECT_SOURCES}
        )
    endif()
endif()

set(LibArchive_ROOT "C:/Users/Gabriel/vcpkg/installed/x64-mingw-dynamic")
set(LibArchive_INCLUDE_DIR "${LibArchive_ROOT}/include")
set(LibArchive_LIBRARY "${LibArchive_ROOT}/lib/libarchive.dll.a")

find_package(LibArchive REQUIRED)
target_include_directories(file_convert PRIVATE ${LibArchive_INCLUDE_DIR})
target_link_libraries(file_convert PRIVATE ${LibArchive_LIBRARY})

target_include_directories(file_convert PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(file_convert PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.file_convert)
endif()
set_target_properties(file_convert PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS file_convert
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(file_convert)
endif()
