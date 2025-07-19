#include "qplatformnotificationengine_android.h"
#include <QtCore/qdebug.h>
#include <QtCore/qglobal.h>
#include <QtCore/qhash.h>
#include <QtCore/qstring.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qvariant.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qjnienvironment.h>
#include <QtCore/qjniobject.h>
#include <QtCore/qnativeinterface.h>

QT_BEGIN_NAMESPACE

QPlatformNotificationEngineAndroid::QPlatformNotificationEngineAndroid(QObject *parent)
  : QPlatformNotificationEngine(parent)
  , m_id(0)
{
    QJniEnvironment env;

    // Get Android context
    QJniObject context = QNativeInterface::QAndroidApplication::context();
    if (!context.isValid()) {
        qWarning("QtNotifications Android: Could not get Android context");
        return;
    }

    // Create Java QtNotifications object
    m_javaObject = QJniObject("org/qtproject/qt/android/notifications/QtNotifications",
                              "(Landroid/content/Context;J)V",
                              context.object<jobject>(),
                              static_cast<jlong>(m_id));

    if (!m_javaObject.isValid()) {
        qWarning("QtNotifications Android: Could not create Java QtNotifications object");
        return;
    }

    qDebug("QtNotifications Android: Java object initialized successfully");
}

bool QPlatformNotificationEngineAndroid::isSupported() const
{
    return m_javaObject.isValid();
}

static QJniObject jniObjectFromQVariant(const QVariant &value)
{
    if (value.typeId() == QMetaType::QByteArray) {
        QByteArray byteArray = value.toByteArray();
        QJniEnvironment env;
        JNIEnv *jniEnv = env.jniEnv();
        if (jniEnv) {
            jbyteArray javaByteArray = jniEnv->NewByteArray(byteArray.size());
            jniEnv->SetByteArrayRegion(javaByteArray, 0, byteArray.size(),
                                        reinterpret_cast<const jbyte*>(byteArray.constData()));
            QJniObject jniObject = QJniObject(javaByteArray);
            jniEnv->DeleteLocalRef(javaByteArray);
            return jniObject;
        }
    } else if (value.typeId() == QMetaType::QString) {
        return QJniObject::fromString(value.toString());
    } else if (value.typeId() == QMetaType::Int || value.typeId() == QMetaType::LongLong) {
        jint intValue = static_cast<jint>(value.toLongLong());
        return QJniObject("java/lang/Integer", "(I)V", intValue);
    }
    return QJniObject();
}

uint QPlatformNotificationEngineAndroid::sendNotification(const QString &title,
                                                          const QString &message,
                                                          const QVariantMap &parameters,
                                                          const QMap<QString, QString> &actions)
{
    if (!m_javaObject.isValid()) {
        qWarning("QtNotifications Android: Java object not initialized");
        return 0;
    }

    QJniObject javaParameters("java/util/HashMap", "()V");
    QJniObject javaActions("java/util/HashMap", "()V");

    qDebug() << "QtNotifications Android: Parameters" << parameters;
    for (auto it = parameters.constBegin(); it != parameters.constEnd(); ++it) {
        QJniObject key = QJniObject::fromString(it.key());
        QJniObject value;

        if (it.value().typeId() == QMetaType::QVariantMap) {
            // Handle QVariantMap (e.g., for iconData structures)
            QVariantMap map = it.value().toMap();
            QJniObject javaMap("java/util/HashMap", "()V");
            for (auto mapIt = map.constBegin(); mapIt != map.constEnd(); ++mapIt) {
                QJniObject mapKey = QJniObject::fromString(mapIt.key());
                QJniObject mapValue = jniObjectFromQVariant(mapIt.value());

                if (mapValue.isValid()) {
                    javaMap.callObjectMethod("put",
                                             "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;",
                                             mapKey.object<jobject>(),
                                             mapValue.object<jobject>());
                }
            }
            value = javaMap;
        } else {
            value = jniObjectFromQVariant(it.value());
        }

        if (value.isValid()) {
            javaParameters.callObjectMethod("put",
                                            "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;",
                                            key.object<jobject>(),
                                            value.object<jobject>());
        }
    }

    qDebug() << "QtNotifications Android: Actions" << actions.size();
    for (auto it = actions.constBegin(); it != actions.constEnd(); ++it) {
        QJniObject key = QJniObject::fromString(it.key());
        QJniObject value = QJniObject::fromString(it.value());
        javaActions.callObjectMethod("put",
                                    "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;",
                                    key.object<jobject>(),
                                    value.object<jobject>());
    }

    // Call Java method
    jint result = m_javaObject.callMethod<jint>(
        "sendNotification",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/util/Map;Ljava/util/Map;)I",
        QJniObject::fromString(title).object<jstring>(),
        QJniObject::fromString(message).object<jstring>(),
        javaParameters.object<jobject>(),
        javaActions.object<jobject>());

    return result;
}

// JNI callback functions
extern "C" {

JNIEXPORT void JNICALL
Java_org_qtproject_qt_android_notifications_QtNotificationsActionReceiver_notifyNotificationClosed(JNIEnv *env,
                                                                                                   jobject thiz,
                                                                                                   jlong id)
{
    Q_UNUSED(env)
    Q_UNUSED(thiz)

    // Emit signal on main thread
    QMetaObject::invokeMethod(
        QCoreApplication::instance(),
        [id]() {
            emit qt_create_notification_engine_android()
                ->notificationClosed(id, QNotifications::Dismissed);
        },
        Qt::QueuedConnection);
}

JNIEXPORT void JNICALL
Java_org_qtproject_qt_android_notifications_QtNotificationsActionReceiver_notifyActionInvoked(JNIEnv *env,
                                                                                              jobject thiz,
                                                                                              jlong id,
                                                                                              jstring actionKey)
{
    Q_UNUSED(env)
    Q_UNUSED(thiz)

    QString key = QJniObject(actionKey).toString();

    // Emit signal on main thread
    QMetaObject::invokeMethod(
        QCoreApplication::instance(),
        [id, key]() {
            emit qt_create_notification_engine_android()
                ->actionInvoked(id, key);
            emit qt_create_notification_engine_android()
                ->notificationClosed(id, QNotifications::Closed);
        },
        Qt::QueuedConnection);
}

JNIEXPORT void JNICALL
Java_org_qtproject_qt_android_notifications_QtNotificationsActionReceiver_notifyNotificationClicked(JNIEnv *env,
                                                                                                     jobject thiz,
                                                                                                     jlong id)
{
    Q_UNUSED(env)
    Q_UNUSED(thiz)

    // Emit signal on main thread
    QMetaObject::invokeMethod(
        QCoreApplication::instance(),
        [id]() {
            emit qt_create_notification_engine_android()
                ->notificationClicked(id);
            emit qt_create_notification_engine_android()
                ->notificationClosed(id, QNotifications::Closed);
        },
        Qt::QueuedConnection);
}

} // extern "C"

QPlatformNotificationEngine *qt_create_notification_engine_android()
{
    static QPlatformNotificationEngineAndroid engine;
    return &engine;
}

QT_END_NAMESPACE
