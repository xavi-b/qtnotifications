#ifndef QPLATFORMNOTIFICATIONENGINE_ANDROID_H
#define QPLATFORMNOTIFICATIONENGINE_ANDROID_H

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtNotifications/qplatformnotificationengine.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qjnienvironment.h>
#include <QtCore/qjniobject.h>
#include <QtCore/qnativeinterface.h>

QT_BEGIN_NAMESPACE

class QPlatformNotificationEngineAndroid : public QPlatformNotificationEngine
{
    Q_OBJECT
public:
    explicit QPlatformNotificationEngineAndroid(QObject *parent = nullptr);

    bool isSupported() const override;
    uint sendNotification(const QString &title,
                          const QString &message,
                          const QVariantMap &parameters,
                          const QMap<QString, QString> &actions) override;

private:
    QJniObject m_javaObject;
    long m_id;
};

// JNI callback functions
extern "C" {

JNIEXPORT void JNICALL
Java_org_qtproject_qt_android_notifications_QtNotifications_notifyNotificationClosed(JNIEnv *env,
                                                                                     jobject thiz,
                                                                                     jlong id);

JNIEXPORT void JNICALL
Java_org_qtproject_qt_android_notifications_QtNotifications_notifyActionInvoked(JNIEnv *env,
                                                                                jobject thiz,
                                                                                jlong id,
                                                                                jstring actionKey);

} // extern "C"

QPlatformNotificationEngine *qt_create_notification_engine_android();

QT_END_NAMESPACE

#endif // QPLATFORMNOTIFICATIONENGINE_ANDROID_H
