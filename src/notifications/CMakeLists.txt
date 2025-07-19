qt_internal_add_module(Notifications
    SOURCES
        qnotifications_global.h
        qnotifications.h
        qnotifications.cpp
        qplatformnotificationengine.h
        qplatformnotificationengine.cpp
    LIBRARIES
        Qt::CorePrivate
    PUBLIC_LIBRARIES
        Qt::Core
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
)

if(ANDROID)
    set_property(TARGET Notifications APPEND PROPERTY QT_ANDROID_BUNDLED_JAR_DEPENDENCIES
        jar/Qt${QtNotifications_VERSION_MAJOR}AndroidNotifications.jar
    )
    qt_internal_add_android_permission(Notifications
        NAME android.permission.POST_NOTIFICATIONS
        ATTRIBUTES minSdkVersion 31
    )
    qt_internal_extend_target(Notifications
        SOURCES
            qplatformnotificationengine_android.cpp
            qplatformnotificationengine_android.h
    )
endif()

if(UNIX AND NOT APPLE AND NOT ANDROID)
    qt_internal_extend_target(Notifications
        SOURCES
            qplatformnotificationengine_linux.cpp
            qplatformnotificationengine_linux.h
        PUBLIC_LIBRARIES
            Qt::DBus
    )
endif()

if(WIN32)
    qt_internal_extend_target(Notifications
        SOURCES
            qplatformnotificationengine_windows.cpp
            qplatformnotificationengine_windows.h
        PUBLIC_LIBRARIES
            windowsapp
    )
    target_compile_features(Notifications PRIVATE cxx_std_20)
endif()

if(APPLE)
    qt_internal_find_apple_system_framework(FWUserNotifications
        UserNotifications)
    qt_internal_extend_target(Notifications
        SOURCES
            qplatformnotificationengine_darwin.mm
            qplatformnotificationengine_darwin.h
        PUBLIC_LIBRARIES
            ${FWFoundation}
            ${FWUserNotifications}
    )
endif()

if(TARGET Qt::Qml)
    add_subdirectory(qml)
endif()