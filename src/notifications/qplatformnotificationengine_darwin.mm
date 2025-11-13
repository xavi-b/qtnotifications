#include "qplatformnotificationengine_darwin.h"
#include <QtCore/qglobal.h>
#import <Foundation/Foundation.h>
#import <UserNotifications/UserNotifications.h>

@interface DarwinNotificationDelegate : NSObject <UNUserNotificationCenterDelegate>
@property (nonatomic, assign) QPlatformNotificationEngineDarwin *engine;
@end

@implementation DarwinNotificationDelegate
- (void)userNotificationCenter:(UNUserNotificationCenter *)center
       didReceiveNotificationResponse:(UNNotificationResponse *)response
                withCompletionHandler:(void (^)(void))completionHandler {
    NSString *actionIdentifier = response.actionIdentifier;
    NSString *notificationIdentifier = response.notification.request.identifier;
    QString notificationIdString = QString::fromNSString(notificationIdentifier);

    if (![actionIdentifier isEqualToString:UNNotificationDefaultActionIdentifier] &&
        ![actionIdentifier isEqualToString:UNNotificationDismissActionIdentifier]) {
        QString actionKey = QString::fromNSString(actionIdentifier);
        self.engine->handleActionInvoked(notificationIdString, actionKey);
        self.engine->handleNotificationClosed(notificationIdString, QNotifications::Closed);
    } else if ([actionIdentifier isEqualToString:UNNotificationDefaultActionIdentifier]) {
        // Handle notification clicked (user tapped on notification body)
        self.engine->handleNotificationClicked(notificationIdString);
        self.engine->handleNotificationClosed(notificationIdString, QNotifications::Closed);
    } else if ([actionIdentifier isEqualToString:UNNotificationDismissActionIdentifier]) {
        // Handle notification dismissed
        self.engine->handleNotificationClosed(notificationIdString, QNotifications::Dismissed);
    }
    completionHandler();
}

- (void)userNotificationCenter:(UNUserNotificationCenter *)center
       willPresentNotification:(UNNotification *)notification
         withCompletionHandler:(void (^)(UNNotificationPresentationOptions options))completionHandler {
    completionHandler(UNNotificationPresentationOptionList | UNNotificationPresentationOptionBanner | UNNotificationPresentationOptionSound);
}
@end

QT_BEGIN_NAMESPACE

QPlatformNotificationEngineDarwin::QPlatformNotificationEngineDarwin(QObject *parent)
  : QPlatformNotificationEngine(parent)
{
    m_delegate = [[DarwinNotificationDelegate alloc] init];
    m_delegate.engine = this;

    // Set the delegate first on the main thread
    dispatch_async(dispatch_get_main_queue(), ^{
        UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
        center.delegate = m_delegate;

        // Log bundle identifier for debugging
        NSBundle *bundle = [NSBundle mainBundle];
        NSString *bundleId = [bundle bundleIdentifier];
        NSLog(@"QtNotifications: Bundle identifier: %@", bundleId ? bundleId : @"(nil - app may not be properly bundled)");

        // Check current authorization status
        [center getNotificationSettingsWithCompletionHandler:^(UNNotificationSettings *settings) {
            NSLog(@"QtNotifications: Current authorization status: %ld", (long)settings.authorizationStatus);

            // Only request authorization if not already determined
            if (settings.authorizationStatus == UNAuthorizationStatusNotDetermined) {
                NSLog(@"QtNotifications: Requesting notification authorization...");
                [center requestAuthorizationWithOptions:(UNAuthorizationOptionAlert | UNAuthorizationOptionSound | UNAuthorizationOptionBadge)
                                      completionHandler:^(BOOL granted, NSError * _Nullable error) {
                    if (granted) {
                        NSLog(@"QtNotifications: Notification permission granted.");
                    } else {
                        NSLog(@"QtNotifications: Notification permission denied.");
                    }
                    if (error) {
                        NSLog(@"QtNotifications: Error requesting notification permission: %@ (code: %ld, domain: %@)",
                              error.localizedDescription ?: @"(no description)",
                              (long)error.code,
                              error.domain);
                        if (error.code == 1) {
                            NSLog(@"QtNotifications: Error code 1 (UNErrorCodeNotificationsNotAllowed) - This usually means:");
                            NSLog(@"QtNotifications:   1. The app is not properly bundled (must be run as .app bundle, not command line)");
                            NSLog(@"QtNotifications:   2. The app is not properly signed");
                            NSLog(@"QtNotifications:   3. The Info.plist is missing or incorrect");
                        }
                    }
                }];
            } else if (settings.authorizationStatus == UNAuthorizationStatusAuthorized) {
                NSLog(@"QtNotifications: Notification permission already authorized.");
            } else {
                NSLog(@"QtNotifications: Notification permission denied or restricted. Status: %ld", (long)settings.authorizationStatus);
            }
        }];
    });
}

QPlatformNotificationEngineDarwin::~QPlatformNotificationEngineDarwin()
{
    if (m_delegate) {
        [m_delegate release];
    }
}

void QPlatformNotificationEngineDarwin::handleActionInvoked(const QString &notificationIdentifier, const QString &actionKey)
{
    uint notificationId = m_notificationIdMap.value(notificationIdentifier, 0);
    emit actionInvoked(notificationId, actionKey);
}

void QPlatformNotificationEngineDarwin::handleNotificationClosed(const QString &notificationIdentifier, QNotifications::ClosedReason reason)
{
    uint notificationId = m_notificationIdMap.value(notificationIdentifier, 0);
    emit notificationClosed(notificationId, reason);
    // Remove from map after handling
    m_notificationIdMap.remove(notificationIdentifier);
}

void QPlatformNotificationEngineDarwin::handleNotificationClicked(const QString &notificationIdentifier)
{
    uint notificationId = m_notificationIdMap.value(notificationIdentifier, 0);
    emit notificationClicked(notificationId);
}

bool QPlatformNotificationEngineDarwin::isSupported() const
{
    return true;
}

uint QPlatformNotificationEngineDarwin::sendNotification(const QString &title, const QString &message, const QVariantMap &parameters, const QMap<QString, QString> &actions)
{
    UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];

    // Check authorization status - use a semaphore but only if not on main thread
    __block BOOL isAuthorized = NO;
    __block BOOL checkComplete = NO;
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);

    [center getNotificationSettingsWithCompletionHandler:^(UNNotificationSettings *settings) {
        isAuthorized = (settings.authorizationStatus == UNAuthorizationStatusAuthorized);
        checkComplete = YES;
        dispatch_semaphore_signal(semaphore);
    }];

    // Wait for the authorization check (with timeout)
    // Use a short timeout to avoid blocking too long
    dispatch_time_t timeout = dispatch_time(DISPATCH_TIME_NOW, 0.5 * NSEC_PER_SEC);
    if (dispatch_semaphore_wait(semaphore, timeout) != 0) {
        NSLog(@"QtNotifications: Timeout waiting for notification authorization status check");
        // Continue anyway - the async check will complete eventually
    }

    if (!isAuthorized) {
        NSLog(@"QtNotifications: Cannot send notification - app does not have notification authorization (status check complete: %@)", checkComplete ? @"YES" : @"NO");
        NSLog(@"QtNotifications: Make sure the app is run as a proper .app bundle and has requested permissions");
        return false;
    }

    UNMutableNotificationContent *content = [[UNMutableNotificationContent alloc] init];
    content.title = title.toNSString();
    content.body = message.toNSString();

    // Collect all image attachments from parameters
    // Supported keys: "icon", "image", "attachment", "image1", "image2", etc.
    NSMutableArray<UNNotificationAttachment *> *attachments = [NSMutableArray array];

    // Helper function to add attachment from file path
    // IMPORTANT: UNNotificationAttachment MOVES (not copies) files outside the app bundle!
    // We need to copy the file to a safe location first to prevent deletion of the original.
    auto addAttachmentFromPath = [&attachments](NSString *filePath, NSString *identifier) {
        if (!filePath || [filePath length] == 0) {
            return;
        }

        NSFileManager *fileManager = [NSFileManager defaultManager];

        // Check if file exists
        if (![fileManager fileExistsAtPath:filePath]) {
            NSLog(@"QtNotifications: Image file not found: %@ (identifier: %@)", filePath, identifier);
            return;
        }

        NSURL *originalURL = [NSURL fileURLWithPath:filePath];
        NSURL *bundleURL = [[NSBundle mainBundle] bundleURL];

        // Check if file is inside app bundle - if so, it will be copied, not moved
        BOOL isInBundle = [originalURL.path hasPrefix:bundleURL.path];

        NSURL *attachmentURL = originalURL;

        // If file is outside bundle, copy it to a temporary location to prevent deletion
        if (!isInBundle) {
            // Create a temporary file in the app's cache directory
            NSURL *cacheDir = [[fileManager URLsForDirectory:NSCachesDirectory inDomains:NSUserDomainMask] firstObject];
            if (cacheDir) {
                NSString *fileName = [NSString stringWithFormat:@"notification-attachment-%@-%@",
                                      identifier, [[NSUUID UUID] UUIDString]];
                NSString *fileExtension = [originalURL pathExtension];
                if ([fileExtension length] > 0) {
                    fileName = [fileName stringByAppendingPathExtension:fileExtension];
                }

                NSURL *tempURL = [cacheDir URLByAppendingPathComponent:fileName];

                NSError *copyError = nil;
                BOOL copied = [fileManager copyItemAtURL:originalURL toURL:tempURL error:&copyError];

                if (copied && !copyError) {
                    attachmentURL = tempURL;
                    NSLog(@"QtNotifications: Copied attachment file to cache: %@", tempURL.path);
                } else {
                    NSLog(@"QtNotifications: Failed to copy attachment file to cache: %@, error: %@",
                          filePath, copyError.localizedDescription);
                    // Fall back to using original - user should be aware it will be moved
                    NSLog(@"QtNotifications: WARNING: Original file will be moved by macOS: %@", filePath);
                }
            }
        }

        NSError *attachmentError = nil;
        UNNotificationAttachment *attachment = [UNNotificationAttachment attachmentWithIdentifier:identifier
                                                                                              URL:attachmentURL
                                                                                          options:nil
                                                                                            error:&attachmentError];
        if (attachment && !attachmentError) {
            [attachments addObject:attachment];
        } else {
            NSLog(@"QtNotifications: Failed to create notification attachment from %@: %@, error: %@",
                  identifier, attachmentURL.path, attachmentError.localizedDescription);
            // Clean up copied file if attachment creation failed
            if (!isInBundle && ![attachmentURL.path isEqualToString:filePath]) {
                [fileManager removeItemAtURL:attachmentURL error:nil];
            }
        }
    };

    QString icon = parameters.value(QStringLiteral("icon")).toString();
    if (!icon.isEmpty()) {
        addAttachmentFromPath(icon.toNSString(), @"notification-icon");
    }

    QString image = parameters.value(QStringLiteral("image")).toString();
    if (!image.isEmpty()) {
        addAttachmentFromPath(image.toNSString(), @"notification-image");
    }

    QString attachment = parameters.value(QStringLiteral("attachment")).toString();
    if (!attachment.isEmpty()) {
        addAttachmentFromPath(attachment.toNSString(), @"notification-attachment");
    }

    // Set all attachments if any were added
    if ([attachments count] > 0) {
        content.attachments = attachments;
    }

    NSMutableArray *actionArray = [NSMutableArray array];
    for (auto it = actions.constBegin(); it != actions.constEnd(); ++it) {
        NSString *actionKey = it.key().toNSString();
        NSString *actionLabel = it.value().toNSString();
        UNNotificationAction *action = [UNNotificationAction actionWithIdentifier:actionKey title:actionLabel options:UNNotificationActionOptionForeground];
        [actionArray addObject:action];
    }
    NSString *categoryId = @"qt_notification_category";
    UNNotificationCategory *category = [UNNotificationCategory categoryWithIdentifier:categoryId actions:actionArray intentIdentifiers:@[] options:UNNotificationCategoryOptionCustomDismissAction];
    NSSet *categories = [NSSet setWithObject:category];
    [center setNotificationCategories:categories];
    content.categoryIdentifier = categoryId;
    UNTimeIntervalNotificationTrigger *trigger = [UNTimeIntervalNotificationTrigger triggerWithTimeInterval:1 repeats:NO];
    NSString *identifier = [[NSUUID UUID] UUIDString];

    // Generate unique notification ID
    static uint s_notificationId = 1;
    uint notificationId = s_notificationId++;

    // Store mapping from identifier to notification ID
    QString identifierString = QString::fromNSString(identifier);
    m_notificationIdMap.insert(identifierString, notificationId);

    UNNotificationRequest *request = [UNNotificationRequest requestWithIdentifier:identifier content:content trigger:trigger];
    [center addNotificationRequest:request withCompletionHandler:^(NSError * _Nullable error) {
        if (error) {
            NSLog(@"Failed to schedule notification: %@ (code: %ld, description: %@)", error, (long)error.code, error.localizedDescription);
            // Remove from map if notification failed
            QPlatformNotificationEngineDarwin *nonConstThis = const_cast<QPlatformNotificationEngineDarwin*>(this);
            nonConstThis->m_notificationIdMap.remove(identifierString);
        }
    }];
    return notificationId;
}

QPlatformNotificationEngine *qt_create_notification_engine_darwin()
{
    static QPlatformNotificationEngineDarwin engine;
    return &engine;
}

QT_END_NAMESPACE
