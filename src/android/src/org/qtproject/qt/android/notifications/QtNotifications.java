package org.qtproject.qt.android.notifications;

import android.app.Activity;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.Manifest;
import android.os.Build;
import android.util.Log;
import android.app.Notification;
import java.util.HashMap;
import java.util.Map;

class QtNotifications
{
    private static final String TAG = "QtNotifications";
    private static final String CHANNEL_ID = "qt_notifications_channel";

    private final Context mContext;
    private final long mId;
    private final NotificationManager mNotificationManager;
    private int mNotificationId = 1;

    QtNotifications(final Context context, final long id) {
        mContext = context;
        mId = id;
        mNotificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        createNotificationChannel();
    }

    private void createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(
                CHANNEL_ID,
                "Qt Notifications",
                NotificationManager.IMPORTANCE_DEFAULT
            );
            channel.setDescription("Notifications from Qt applications");
            mNotificationManager.createNotificationChannel(channel);
        }
    }

    boolean sendNotification(String title, String text, String icon, Map<String, String> actions, int type) {
        Log.d(TAG, "Sending notification: " + title + " - " + text);

        // Debug: print the class and size of the actions map
        if (actions == null) {
            Log.d(TAG, "Actions map is null");
        } else {
            Log.d(TAG, "Actions map class: " + actions.getClass().getName() + ", size: " + actions.size());
        }

        try {
            // Create dismiss intent
            Intent dismissIntent = new Intent(QtNotificationsActionReceiver.ACTION_NOTIFICATION_CLOSED);
            dismissIntent.setClass(mContext, QtNotificationsActionReceiver.class);
            dismissIntent.putExtra("notification_id", mNotificationId);
            dismissIntent.setPackage(mContext.getPackageName());
            PendingIntent dismissPendingIntent = PendingIntent.getBroadcast(
                mContext,
                mNotificationId,
                dismissIntent,
                PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE
            );

            // Create content intent for notification click
            Intent contentIntent = new Intent(QtNotificationsActionReceiver.ACTION_NOTIFICATION_CLICKED);
            contentIntent.setClass(mContext, QtNotificationsActionReceiver.class);
            contentIntent.putExtra("notification_id", mNotificationId);
            contentIntent.setPackage(mContext.getPackageName());
            PendingIntent contentPendingIntent = PendingIntent.getBroadcast(
                mContext,
                mNotificationId + 10000, // Use different request code to avoid conflicts
                contentIntent,
                PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE
            );

            // Build notification
            Notification.Builder builder = new Notification.Builder(mContext, CHANNEL_ID)
                .setContentTitle(title)
                .setContentText(text)
                .setSmallIcon(android.R.drawable.ic_dialog_info)
                .setAutoCancel(true)
                .setDeleteIntent(dismissPendingIntent)
                .setContentIntent(contentPendingIntent);

            // Add actions if provided
            if (actions != null && !actions.isEmpty()) {
                int actionIndex = 0;
                for (Map.Entry<String, String> entry : actions.entrySet()) {
                    Intent actionIntent = new Intent(QtNotificationsActionReceiver.ACTION_NOTIFICATION);
                    actionIntent.setClass(mContext, QtNotificationsActionReceiver.class);
                    actionIntent.putExtra("notification_id", mNotificationId);
                    actionIntent.putExtra("action_key", entry.getKey());
                    actionIntent.setPackage(mContext.getPackageName());

                    PendingIntent actionPendingIntent = PendingIntent.getBroadcast(
                        mContext,
                        mNotificationId * 1000 + actionIndex,
                        actionIntent,
                        PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE
                    );

                    builder.addAction(0, entry.getValue(), actionPendingIntent);
                    actionIndex++;
                }
            }

            // Show notification
            mNotificationManager.notify(mNotificationId, builder.build());
            mNotificationId++;

            Log.d(TAG, "Notification sent successfully with ID: " + (mNotificationId - 1));
            return true;

        } catch (Exception e) {
            Log.e(TAG, "Error sending notification: " + e.getMessage());
            return false;
        }
    }

    boolean isSupported() {
        return mNotificationManager != null;
    }

    void cancelAllNotifications() {
        if (mNotificationManager != null) {
            mNotificationManager.cancelAll();
            Log.d(TAG, "All notifications cancelled");
        }
    }

    void cancelNotification(int notificationId) {
        if (mNotificationManager != null) {
            mNotificationManager.cancel(notificationId);
            Log.d(TAG, "Notification cancelled: " + notificationId);
        }
    }
}