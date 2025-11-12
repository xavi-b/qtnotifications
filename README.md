# QtNotifications

A cross-platform notification module for Qt applications that provides a unified API for sending system notifications across different platforms.

## Overview

QtNotifications provides a simple and consistent interface for sending notifications on various platforms including Windows, macOS, Linux, and Android. The library abstracts platform-specific notification implementations behind a common API.

### Windows
- **Registry Configuration Required**: Before using notifications on Windows, you must set up the necessary registry values for your application.
- The notification system relies on Windows registry entries to properly register your application with the Windows notification system.
- Failure to configure these registry values will result in notifications not being displayed.

#### Windows Registry Setup

To enable notifications on Windows, you need to create registry entries under `HKEY_CURRENT_USER\Software\Classes\AppUserModelId`. Follow these steps:

1. **Open Registry Editor**:
   - Press `Win + R`, type `regedit`, and press Enter
   - Navigate to `HKEY_CURRENT_USER\Software\Classes\AppUserModelId`

2. **Create Application Entry**:
   - Right-click on `AppUserModelId` and select "New" → "Key"
   - Name the key with your application's AppUserModelId (see below how it is constructed)

3. **Set Required Values**:
   - Right-click on your new key and select "New" → "String Value"
   - Name it `DisplayName` and set its value to your application's display name
   - Create another string value named `IconUri` and set it to the path of your application's icon (e.g., `C:\Path\To\YourApp.exe`)

4. **Example Registry Structure**:
   ```
   HKEY_CURRENT_USER\Software\Classes\AppUserModelId\
   └── YourCompany.YourApp\
       ├── DisplayName = "Your Application Name"
       └── IconUri = "C:\Path\To\YourApp.exe"
   ```


#### How AppUserModelId is Constructed in QPlatformNotificationEngineWindows

The QtNotifications library automatically constructs the AppUserModelId using your Qt application's metadata:

1. **Organization Domain**: Uses `QApplication::organizationDomain()` (sanitized)
2. **Application Name**: Uses `QApplication::applicationName()` (sanitized)
3. **Combination**: The final AppUserModelId is constructed as `{organizationDomain}.{applicationName}`

**Sanitization Process**:
- All characters that are not alphanumeric, dots, or hyphens are replaced with underscores
- This ensures the AppUserModelId follows Windows naming conventions

**Example**:
- If your app has `organizationDomain = "mycompany.com"` and `applicationName = "My App"`
- The sanitized AppUserModelId becomes: `mycompany.com.My_App`

**Important**: Make sure to set your application's `organizationDomain` and `applicationName` before using notifications, as these values are used to construct the AppUserModelId that must match your registry entries.

### Android
- **Known Issue**: The `QtNotificationsActionReceiver` does not receive action callbacks on Android.
- While notifications can be sent successfully, action buttons in notifications will not trigger the expected callbacks.

## Examples

See the `examples/` directory for complete working examples:
- `examples/cpp/` - C++ examples
- `examples/qml/` - QML examples
- `examples/widgets/` - Widget-based examples

## License

See the `LICENSE` file for license information.