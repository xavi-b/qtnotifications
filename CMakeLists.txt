cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtNotifications
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt Notifications Libraries"
    HOMEPAGE_URL "https://www.github.com/xavi-b/qtnotifications"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals)

# This should be called as early as possible, just after find_package(BuildInternals) where it is
# defined.
qt_internal_project_setup()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Core)
find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG OPTIONAL_COMPONENTS
    DBus Gui Widgets Quick Qml)

qt_build_repo()
