#ifndef QPLATFORMNOTIFICATIONENGINE_WINDOWS_H
#define QPLATFORMNOTIFICATIONENGINE_WINDOWS_H

#include <QtNotifications/qplatformnotificationengine.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <windows.h>
#include <winrt/Windows.Data.Xml.Dom.h>
#include <winrt/Windows.UI.Notifications.h>

QT_BEGIN_NAMESPACE

class QPlatformNotificationEngineWindows : public QPlatformNotificationEngine
{
    Q_OBJECT
public:
    explicit QPlatformNotificationEngineWindows(QObject *parent = nullptr);
    ~QPlatformNotificationEngineWindows() = default;

    void handleActionInvoked(uint notificationId, const QString &actionKey);
    void handleNotificationClosed(uint notificationId, uint reason);
    void handleNotificationClicked(uint notificationId);

    bool isSupported() const override;
    bool sendNotification(const QString &title,
                         const QString &message,
                         const QString &iconPath,
                         const QMap<QString, QString> &actions,
                         QNotifications::NotificationType type) override;

private:
    void ensureComInitialized();
    void setAppUserModelID();

    // Event handler methods for toast events
    void onToastActivated(winrt::Windows::UI::Notifications::ToastNotification const& sender, winrt::Windows::Foundation::IInspectable const& args);
    void onToastDismissed(winrt::Windows::UI::Notifications::ToastNotification const& sender, winrt::Windows::UI::Notifications::ToastDismissedEventArgs const& args);

    QString m_appUserModelID;
    uint m_lastNotificationId = 0;
};

QPlatformNotificationEngine *qt_create_notification_engine_windows();

QT_END_NAMESPACE

#endif // QPLATFORMNOTIFICATIONENGINE_WINDOWS_H
