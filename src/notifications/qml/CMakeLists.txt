qt_internal_add_qml_module(NotificationsQml
    URI "QtNotifications"
    VERSION "${PROJECT_VERSION}"
    PLUGIN_TARGET NotificationsQml
    SOURCES
        qdeclarativenotifications_p.h
        qdeclarativenotifications.cpp
    LIBRARIES
        Qt::Notifications
    NO_GENERATE_CPP_EXPORTS
)