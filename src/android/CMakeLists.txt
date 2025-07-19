set(java_sources
    src/org/qtproject/qt/android/notifications/QtNotificationsActionReceiver.java
    src/org/qtproject/qt/android/notifications/QtNotifications.java
)

qt_internal_add_jar(Qt${QtNotifications_VERSION_MAJOR}AndroidNotifications
    INCLUDE_JARS ${QT_ANDROID_JAR}
    SOURCES ${java_sources}
    OUTPUT_DIR "${QT_BUILD_DIR}/jar"
)

qt_path_join(destination ${INSTALL_DATADIR} "jar")

install_jar(Qt${QtNotifications_VERSION_MAJOR}AndroidNotifications
    DESTINATION ${destination}
    COMPONENT Devel
)

add_dependencies(Notifications Qt${QtNotifications_VERSION_MAJOR}AndroidNotifications)
