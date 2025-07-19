#include "qplatformnotificationengine.h"

QT_BEGIN_NAMESPACE

QPlatformNotificationEngine *qt_notification_engine()
{
#if defined(Q_OS_ANDROID)
    extern QPlatformNotificationEngine *qt_create_notification_engine_android();
    return qt_create_notification_engine_android();
#elif defined(Q_OS_LINUX)
    extern QPlatformNotificationEngine *qt_create_notification_engine_linux();
    return qt_create_notification_engine_linux();
#elif defined(Q_OS_WIN)
    extern QPlatformNotificationEngine *qt_create_notification_engine_windows();
    return qt_create_notification_engine_windows();
#elif defined(Q_OS_MACOS) || defined(Q_OS_IOS)
    extern QPlatformNotificationEngine *qt_create_notification_engine_darwin();
    return qt_create_notification_engine_darwin();
#else
    return nullptr;
#endif
}

QT_END_NAMESPACE
