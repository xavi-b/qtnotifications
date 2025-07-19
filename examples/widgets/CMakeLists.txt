cmake_minimum_required(VERSION 3.16)
project(notifications_example_widgets LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/notifications_example_widgets")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Notifications)

qt_standard_project_setup()

qt_add_executable(notifications_example_widgets
    main.cpp
)

# Set up the executable properties
set_target_properties(notifications_example_widgets PROPERTIES
    VERSION 1.0.0
    DESCRIPTION "Qt Notifications Widgets Example"
    PURPOSE "Demonstrates usage of Qt Notifications with Qt Widgets GUI"
)

# Link dependencies
target_link_libraries(notifications_example_widgets
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Notifications
)

set_target_properties(notifications_example_widgets PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER "io.qt.examples.notifications_example_widgets"
    MACOSX_BUNDLE_BUNDLE_VERSION 1.0.0
    MACOSX_BUNDLE_SHORT_VERSION_STRING 1.0
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(APPLE)
    set_target_properties(notifications_example_widgets PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
    )
endif()

if(ANDROID)
    set_target_properties(notifications_example_widgets PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
    )
endif()

qt_finalize_target(notifications_example_widgets)

install(TARGETS notifications_example_widgets
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
