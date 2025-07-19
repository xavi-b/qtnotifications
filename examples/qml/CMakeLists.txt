cmake_minimum_required(VERSION 3.16)
project(notifications_example_qml LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/notifications_example_qml")

find_package(Qt6 REQUIRED COMPONENTS Core Quick Qml Notifications)

qt_standard_project_setup()

# Create the QML notifications example
qt_add_executable(notifications_example_qml
    main.cpp
)

qt_add_resources(notifications_example_qml
    "qml"
    PREFIX "/"
    FILES
    main.qml
)

# Set up the executable properties
set_target_properties(notifications_example_qml PROPERTIES
    VERSION 1.0.0
    DESCRIPTION "Qt Notifications QML Example"
    PURPOSE "Demonstrates usage of Qt Notifications QML API"
)

# Link dependencies
target_link_libraries(notifications_example_qml
    PRIVATE
        Qt6::Core
        Qt6::Qml
        Qt6::Quick
        Qt6::Notifications
)

set_target_properties(notifications_example_qml PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER "io.qt.examples.notifications_example_qml"
    MACOSX_BUNDLE_BUNDLE_VERSION 1.0.0
    MACOSX_BUNDLE_SHORT_VERSION_STRING 1.0
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(APPLE)
    set_target_properties(notifications_example_qml PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
    )
endif()

qt_finalize_target(notifications_example_qml)

install(TARGETS notifications_example_qml
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
