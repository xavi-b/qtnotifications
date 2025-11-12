#include "qplatformnotificationengine_windows.h"
#include <windows.h>
#include <shobjidl.h>
#include <winrt/Windows.Data.Xml.Dom.h>
#include <winrt/Windows.UI.Notifications.h>
#include <combaseapi.h>
#include <propvarutil.h>
#include <winternl.h>
#include <QtCore/qstring.h>
#include <QtCore/qdebug.h>
#include <QtCore/qmap.h>
#include <winrt/impl/Windows.Foundation.1.h>
#include <winrt/impl/Windows.UI.Notifications.1.h>
#include <winrt/Windows.Foundation.h>
#include <QtCore/QRegularExpression>

QT_BEGIN_NAMESPACE

using namespace winrt;
using namespace winrt::Windows::Data::Xml::Dom;
using namespace winrt::Windows::UI::Notifications;

// Helper to sanitize AppUserModelID parts
static QString sanitizeAppUserModelID(const QString &input)
{
    QString sanitized = input;
    sanitized.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9.-]")), QStringLiteral("_"));
    return sanitized;
}

QPlatformNotificationEngineWindows::QPlatformNotificationEngineWindows(QObject *parent)
  : QPlatformNotificationEngine(parent)
{
    // Use organizationDomain and applicationName for AppUserModelID, sanitized
    QString org = qApp ? qApp->organizationDomain() : QStringLiteral("qt");
    QString app = qApp ? qApp->applicationName() : QStringLiteral("QtNotifications");
    org = sanitizeAppUserModelID(org);
    app = sanitizeAppUserModelID(app);
    m_appUserModelID = org + QLatin1Char('.') + app;
    setAppUserModelID();
}

bool QPlatformNotificationEngineWindows::isSupported() const
{
    RTL_OSVERSIONINFOW osvi;
    ZeroMemory(&osvi, sizeof(RTL_OSVERSIONINFOW));
    osvi.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
    typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
    HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
    if (hMod) {
        RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)GetProcAddress(hMod, "RtlGetVersion");
        if (fxPtr != nullptr) {
            fxPtr(&osvi);
        }
    }
    return osvi.dwMajorVersion >= 10;
}

bool QPlatformNotificationEngineWindows::sendNotification(const QString &summary, const QString &body, const QString &icon, const QMap<QString, QString> &actions, QNotifications::NotificationType type)
{
    Q_UNUSED(type);

    ensureComInitialized();
    static uint s_notificationId = 1;
    uint notificationId = s_notificationId++;
    QString xml = QStringLiteral(
        "<toast>"
        "<visual>"
        "<binding template=\"ToastGeneric\">"
    );

    // Add icon if provided
    if (!icon.isEmpty()) {
        // Support different icon formats and placements
        // You can use: "appLogoOverride", "hero", "inline"
        QString placement = QStringLiteral("appLogoOverride");

        // If icon path contains "hero" or "inline", use that placement
        if (icon.contains(QStringLiteral("hero"), Qt::CaseInsensitive)) {
            placement = QStringLiteral("hero");
        } else if (icon.contains(QStringLiteral("inline"), Qt::CaseInsensitive)) {
            placement = QStringLiteral("inline");
        }

        xml += QStringLiteral("<image placement=\"%1\" src=\"%2\"/>").arg(placement, icon);
    }

    xml += QStringLiteral(
        "<text>%1</text>"
        "<text>%2</text>"
        "</binding>"
        "</visual>"
    ).arg(summary, body);
    if (!actions.isEmpty()) {
        xml += QStringLiteral("<actions>");
        for (auto it = actions.constBegin(); it != actions.constEnd(); ++it) {
            xml += QStringLiteral("<action content=\"%1\" arguments=\"%2\" activationType=\"foreground\"/>").arg(it.value(), it.key());
        }
        xml += QStringLiteral("</actions>");
    }
    xml += QStringLiteral("</toast>");

    try {
        auto toastXml = winrt::Windows::Data::Xml::Dom::XmlDocument();
        toastXml.LoadXml(winrt::hstring(xml.toStdWString()));
        auto toast = winrt::Windows::UI::Notifications::ToastNotification(toastXml);
        // Use TypedEventHandler for event handlers
        using ActivatedHandler = winrt::Windows::Foundation::TypedEventHandler<winrt::Windows::UI::Notifications::ToastNotification, winrt::Windows::Foundation::IInspectable>;
        using DismissedHandler = winrt::Windows::Foundation::TypedEventHandler<winrt::Windows::UI::Notifications::ToastNotification, winrt::Windows::UI::Notifications::ToastDismissedEventArgs>;
        toast.Activated(ActivatedHandler{this, &QPlatformNotificationEngineWindows::onToastActivated});
        toast.Dismissed(DismissedHandler{this, &QPlatformNotificationEngineWindows::onToastDismissed});
        // Store notificationId for use in handlers
        m_lastNotificationId = notificationId;
        auto notifier = winrt::Windows::UI::Notifications::ToastNotificationManager::CreateToastNotifier(winrt::hstring(m_appUserModelID.toStdWString()));
        notifier.Show(toast);
        return true;
    } catch (const winrt::hresult_error &e) {
        qWarning() << "WinRT error:" << QString::fromWCharArray(e.message().c_str());
        return false;
    }
}

void QPlatformNotificationEngineWindows::ensureComInitialized()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (hr != S_OK && hr != S_FALSE && hr != RPC_E_CHANGED_MODE) {
        qWarning() << "CoInitializeEx failed";
    }
}

void QPlatformNotificationEngineWindows::setAppUserModelID()
{
    // First, check if the registry key exists and create it if needed
    QString regPath = QStringLiteral("Software\\Classes\\AppUserModelId\\") + m_appUserModelID;
    std::wstring regPathW = regPath.toStdWString();

    HKEY hKey = nullptr;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, regPathW.c_str(), 0, KEY_READ, &hKey);

    if (result != ERROR_SUCCESS) {
        // Key doesn't exist, create it
        DWORD disposition = 0;
        result = RegCreateKeyExW(HKEY_CURRENT_USER, regPathW.c_str(), 0, nullptr,
                                 REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, &disposition);

        if (result == ERROR_SUCCESS) {
            QString displayName = qApp ? qApp->applicationName() : m_appUserModelID;
            std::wstring displayNameW = displayName.toStdWString();

            RegSetValueExW(hKey, L"DisplayName", 0, REG_SZ,
                          reinterpret_cast<const BYTE*>(displayNameW.c_str()),
                          static_cast<DWORD>((displayNameW.length() + 1) * sizeof(wchar_t)));

            qDebug() << "Created AppUserModelID registry key:" << m_appUserModelID;
        } else {
            qWarning() << "Failed to create AppUserModelID registry key. Error:" << result;
        }
    }

    if (hKey) {
        RegCloseKey(hKey);
    }

    // Now set the AppUserModelID for the current process
    HRESULT hr = S_OK;
    typedef HRESULT (WINAPI *SetCurrentProcessExplicitAppUserModelID_t)(PCWSTR);
    HMODULE shell32 = LoadLibraryW(L"shell32.dll");
    if (shell32) {
        auto setID = (SetCurrentProcessExplicitAppUserModelID_t)GetProcAddress(shell32, "SetCurrentProcessExplicitAppUserModelID");
        if (setID) {
            hr = setID(reinterpret_cast<const wchar_t *>(m_appUserModelID.utf16()));
        }
        FreeLibrary(shell32);
    }
    if (FAILED(hr)) {
        qWarning() << "Failed to set AppUserModelID";
    }
}

QPlatformNotificationEngine *qt_create_notification_engine_windows()
{
    static QPlatformNotificationEngineWindows engine;
    return &engine;
}

// Add these private methods to the class implementation
void QPlatformNotificationEngineWindows::onToastActivated(winrt::Windows::UI::Notifications::ToastNotification const& sender, winrt::Windows::Foundation::IInspectable const& args)
{
    QString actionKey = QStringLiteral("default");
    auto activatedArgs = args.try_as<winrt::Windows::UI::Notifications::ToastActivatedEventArgs>();
    if (activatedArgs) {
        actionKey = QString::fromWCharArray(activatedArgs.Arguments().c_str());
    }

    // If no specific action was invoked (empty arguments), emit notificationClicked
    if (actionKey.isEmpty() || actionKey == QStringLiteral("default")) {
        emit notificationClicked(m_lastNotificationId);
    } else {
        emit actionInvoked(m_lastNotificationId, actionKey);
    }
    emit notificationClosed(m_lastNotificationId, QNotifications::Closed);
}

void QPlatformNotificationEngineWindows::onToastDismissed(winrt::Windows::UI::Notifications::ToastNotification const& sender, winrt::Windows::UI::Notifications::ToastDismissedEventArgs const& args)
{
    winrt::Windows::UI::Notifications::ToastDismissalReason reason = args.Reason();
    switch (reason) {
        case winrt::Windows::UI::Notifications::ToastDismissalReason::ApplicationHidden:
            emit notificationClosed(m_lastNotificationId, QNotifications::Closed);
            break;
        case winrt::Windows::UI::Notifications::ToastDismissalReason::TimedOut:
            emit notificationClosed(m_lastNotificationId, QNotifications::Expired);
            break;
        case winrt::Windows::UI::Notifications::ToastDismissalReason::UserCanceled:
            emit notificationClosed(m_lastNotificationId, QNotifications::Dismissed);
            break;
        default:
            emit notificationClosed(m_lastNotificationId, QNotifications::Undefined);
            break;
    }
}

QT_END_NAMESPACE
