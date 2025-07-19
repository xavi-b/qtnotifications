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
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.Icon;
import java.io.File;
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

    int sendNotification(String title, String text, Map<String, Object> parameters, Map<String, String> actions) {
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
                .setAutoCancel(true)
                .setDeleteIntent(dismissPendingIntent)
                .setContentIntent(contentPendingIntent);

            int iconResId = mContext.getResources().getIdentifier("ic_launcher", "drawable", mContext.getPackageName());
            if (iconResId != 0) {
                builder.setSmallIcon(iconResId);
            } else {
                builder.setSmallIcon(android.R.drawable.ic_dialog_info);
            }

            if (parameters != null) {
                if (parameters.containsKey("smallIconData")) {
                    Object iconDataObj = parameters.get("smallIconData");
                    Icon iconObj = getIconFromData(iconDataObj);
                    if (iconObj != null) {
                        builder.setSmallIcon(iconObj);
                    } else {
                        Log.w(TAG, "Failed to get icon from smallIconData");
                    }
                }

                if (parameters.containsKey("smallIconPath")) {
                    String iconPath = (String) parameters.get("smallIconPath");
                    Icon iconObj = getIconFromPath(iconPath);
                    if (iconObj != null) {
                        builder.setSmallIcon(iconObj);
                    } else {
                        Log.w(TAG, "Failed to get icon from smallIconPath");
                    }
                }

                if (parameters.containsKey("largeIconData")) {
                    Object iconDataObj = parameters.get("largeIconData");
                    Icon iconObj = getIconFromData(iconDataObj);
                    if (iconObj != null) {
                        builder.setLargeIcon(iconObj);
                    } else {
                        Log.w(TAG, "Failed to get icon from largeIconData");
                    }
                }

                if (parameters.containsKey("largeIconPath")) {
                    String iconPath = (String) parameters.get("largeIconPath");
                    Icon iconObj = getIconFromPath(iconPath);
                    if (iconObj != null) {
                        builder.setLargeIcon(iconObj);
                    } else {
                        Log.w(TAG, "Failed to get icon from largeIconPath");
                    }
                }
            }

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
            return mNotificationId - 1;

        } catch (Exception e) {
            Log.e(TAG, "Error sending notification: " + e.getMessage());
            return 0;
        }
    }

    private Icon getIconFromData(Object iconDataObj) {
        if (iconDataObj == null) {
            Log.w(TAG, "Icon data object is null");
            return null;
        }

        // Check if it's a Map (QVariantMap from C++)
        if (iconDataObj instanceof Map) {
            @SuppressWarnings("unchecked")
            Map<String, Object> iconDataMap = (Map<String, Object>) iconDataObj;

            Object dataObj = iconDataMap.get("data");
            Object widthObj = iconDataMap.get("width");
            Object heightObj = iconDataMap.get("height");
            Object channelsObj = iconDataMap.get("channels");

            if (!(dataObj instanceof byte[])) {
                Log.w(TAG, "Icon data map 'data' field is not a byte array");
                return null;
            }

            byte[] iconData = (byte[]) dataObj;
            if (iconData == null || iconData.length == 0) {
                Log.w(TAG, "Icon data byte array is null or empty");
                return null;
            }

            Integer width = null;
            Integer height = null;
            Integer channels = null;

            if (widthObj instanceof Integer) {
                width = (Integer) widthObj;
            } else if (widthObj instanceof Number) {
                width = ((Number) widthObj).intValue();
            }

            if (heightObj instanceof Integer) {
                height = (Integer) heightObj;
            } else if (heightObj instanceof Number) {
                height = ((Number) heightObj).intValue();
            }

            if (channelsObj instanceof Integer) {
                channels = (Integer) channelsObj;
            } else if (channelsObj instanceof Number) {
                channels = ((Number) channelsObj).intValue();
            }

            Log.d(TAG, "Attempting to decode icon from map, size: " + iconData.length + ", dimensions: " + width + "x" + height + ", channels: " + channels);

            try {
                // First try to decode as encoded image (PNG/JPEG)
                Bitmap iconBitmap = BitmapFactory.decodeByteArray(iconData, 0, iconData.length);
                if (iconBitmap != null) {
                    Log.d(TAG, "Successfully decoded bitmap from encoded format, size: " + iconBitmap.getWidth() + "x" + iconBitmap.getHeight());
                    return getIconFromBitmap(iconBitmap);
                }

                // If that fails, create bitmap from raw pixel data using width/height/channels
                if (width != null && height != null && width > 0 && height > 0) {
                    int expectedChannels = (channels != null && channels > 0) ? channels : 4; // Default to RGBA
                    int expectedSize = width * height * expectedChannels;

                    if (iconData.length == expectedSize) {
                        Log.d(TAG, "Creating bitmap from raw pixel data: " + width + "x" + height + ", channels: " + expectedChannels);

                        if (expectedChannels == 4) {
                            // RGBA format - convert to ARGB for Android
                            int[] pixels = new int[width * height];
                            for (int i = 0; i < pixels.length; i++) {
                                int offset = i * 4;
                                int r = iconData[offset] & 0xFF;
                                int g = iconData[offset + 1] & 0xFF;
                                int b = iconData[offset + 2] & 0xFF;
                                int a = iconData[offset + 3] & 0xFF;
                                // Convert RGBA to ARGB
                                pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
                            }
                            iconBitmap = Bitmap.createBitmap(pixels, width, height, Bitmap.Config.ARGB_8888);
                        } else if (expectedChannels == 3) {
                            // RGB format - convert to ARGB for Android
                            int[] pixels = new int[width * height];
                            for (int i = 0; i < pixels.length; i++) {
                                int offset = i * 3;
                                int r = iconData[offset] & 0xFF;
                                int g = iconData[offset + 1] & 0xFF;
                                int b = iconData[offset + 2] & 0xFF;
                                // Convert RGB to ARGB (alpha = 255)
                                pixels[i] = (0xFF << 24) | (r << 16) | (g << 8) | b;
                            }
                            iconBitmap = Bitmap.createBitmap(pixels, width, height, Bitmap.Config.ARGB_8888);
                        } else {
                            Log.w(TAG, "Unsupported channel count: " + expectedChannels + " (expected 3 or 4)");
                            return null;
                        }

                        return getIconFromBitmap(iconBitmap);
                    } else {
                        Log.w(TAG, "Icon data size mismatch: expected " + expectedSize + " bytes, got " + iconData.length);
                    }
                } else {
                    Log.w(TAG, "Cannot create bitmap from raw pixel data: missing width/height in icon data map");
                }
            } catch (Exception e) {
                Log.e(TAG, "Error loading icon from data map: " + e.getMessage(), e);
            }
        }

        return null;
    }

    private Icon getIconFromPath(String iconPath) {
        try {
            File iconFile = new File(iconPath);
            if (iconFile.exists() && iconFile.isFile()) {
                Bitmap iconBitmap = BitmapFactory.decodeFile(iconFile.getAbsolutePath());
                if (iconBitmap != null) {
                    return getIconFromBitmap(iconBitmap);
                } else {
                    Log.w(TAG, "Failed to decode icon bitmap from file: " + iconPath);
                }
            }
        } catch (Exception e) {
            Log.e(TAG, "Error loading icon from path: " + iconPath + " - " + e.getMessage(), e);
        }
        return null;
    }

    private Icon getIconFromBitmap(Bitmap iconBitmap) {
        if (iconBitmap == null) {
            return null;
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            // Use Icon API for API 23+
            return Icon.createWithBitmap(iconBitmap);
        } else {
            // For older APIs, we need to use a resource ID
            // Fall back to default icon
            Log.w(TAG, "Bitmap icons not supported on API < 23, using default");
            return null;
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