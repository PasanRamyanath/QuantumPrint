cmake_minimum_required(VERSION 3.16)

project(QuantumPrint VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(WIN32_EXECUTABLE TRUE)

find_package(Qt6 COMPONENTS Core Gui Widgets Pdf PrintSupport Network QUIET)
if(Qt6_FOUND)
    message(STATUS "Found Qt6")
    set(QT_VERSION_MAJOR 6)
    set(QT_LIBS Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Pdf Qt6::PrintSupport Qt6::Network)
    set(HAS_QTPDF TRUE)
else()
    message(STATUS "Qt6 not found, falling back to Qt5")
    find_package(Qt5 COMPONENTS Core Gui Widgets PrintSupport Network REQUIRED)
    set(QT_VERSION_MAJOR 5)
    set(QT_LIBS Qt5::Core Qt5::Gui Qt5::Widgets Qt5::PrintSupport Qt5::Network)
    set(HAS_QTPDF FALSE)
endif()

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h

    FileWatcher.h
    FileWatcher.cpp
    PrinterSelectionDialog.h
    Config.h
    LicenseManager.h
    LicenseDialog.h
)

if(HAS_QTPDF)
    list(APPEND PROJECT_SOURCES
        PdfPrinter.h
        PdfPrinter.cpp
    )
endif()

if(WIN32)
    list(APPEND PROJECT_SOURCES resources.rc)
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_executable(QuantumPrint
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        resources.qrc
        mainwindow.ui

    )
else()
    add_executable(QuantumPrint
        ${PROJECT_SOURCES}
        resources.qrc
    )
endif()

target_link_libraries(QuantumPrint PRIVATE ${QT_LIBS})

if(HAS_QTPDF)
    target_compile_definitions(QuantumPrint PRIVATE HAS_QTPDF)
endif()

set_target_properties(QuantumPrint PROPERTIES
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6 AND QT_VERSION VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.QuantumPrint)
    set_target_properties(QuantumPrint PROPERTIES
        ${BUNDLE_ID_OPTION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
    )
endif()

include(GNUInstallDirs)
install(TARGETS QuantumPrint
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(QuantumPrint)
endif()
