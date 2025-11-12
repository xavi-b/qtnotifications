#ifndef QDECLARATIVENOTIFICATIONS_P_H
#define QDECLARATIVENOTIFICATIONS_P_H

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtQml/qqml.h>
#include <qnotifications.h>

QT_BEGIN_NAMESPACE

class QDeclarativeNotifications : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Notifications)
public:
    explicit QDeclarativeNotifications(QObject *parent = nullptr);

    Q_INVOKABLE bool isSupported() const;
    Q_INVOKABLE bool sendNotification(const QString &title,
                                      const QString &message,
                                      QNotifications::NotificationType type = QNotifications::Information);
    Q_INVOKABLE bool sendNotification(const QString &title,
                                      const QString &message,
                                      const QString &iconPath,
                                      QNotifications::NotificationType type = QNotifications::Information);
    Q_INVOKABLE bool sendNotification(const QString &title,
                                      const QString &message,
                                      const QVariantMap &actions,
                                      QNotifications::NotificationType type = QNotifications::Information);
    Q_INVOKABLE bool sendNotification(const QString &title,
                                      const QString &message,
                                      const QString &iconPath,
                                      const QMap<QString, QString> &actions,
                                      QNotifications::NotificationType type = QNotifications::Information);

signals:
    void actionInvoked(uint notificationId, const QString &actionKey);
    void notificationClosed(uint notificationId);
    void notificationClicked(uint notificationId);

private:
    QNotifications m_notifications;
};

QT_END_NAMESPACE

#endif // QDECLARATIVENOTIFICATIONS_P_H
