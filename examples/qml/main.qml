import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import QtNotifications

Window {
    id: window
    width: 600
    height: 700
    visible: true
    title: "Qt Notifications QML Example"

    Rectangle {
        anchors.fill: parent
        color: "salmon"
    }

    Notifications {
        id: notifications
    }

    FileDialog {
        id: iconFileDialog
        title: "Select Icon File"
        nameFilters: ["Image Files (*.png *.jpg *.jpeg *.bmp *.ico *.svg)", "All Files (*)"]
        onAccepted: {
            iconPathField.text = selectedFile.toString().replace("file://", "")
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        Text {
            text: "Qt Notifications QML Example"
            font.pixelSize: 20
            font.bold: true
        }

        Column {
            Layout.fillWidth: true
            spacing: 15

            // Title input
            Row {
                width: parent.width
                spacing: 10
                Label {
                    text: "Title:"
                    width: 100
                    anchors.verticalCenter: parent.verticalCenter
                }
                TextField {
                    id: titleField
                    width: parent.width - 110
                    placeholderText: "Enter notification title..."
                }
            }

            // Message input
            Row {
                width: parent.width
                spacing: 10
                Label {
                    text: "Message:"
                    width: 100
                    anchors.verticalCenter: parent.verticalCenter
                }
                TextArea {
                    id: messageField
                    width: parent.width - 110
                    height: 80
                    placeholderText: "Enter notification message..."
                    wrapMode: TextArea.Wrap
                }
            }

            // Icon path input
            Row {
                width: parent.width
                spacing: 10
                Label {
                    text: "Icon Path:"
                    width: 100
                    anchors.verticalCenter: parent.verticalCenter
                }
                TextField {
                    id: iconPathField
                    width: parent.width - 210
                    placeholderText: "/path/to/icon.png or leave empty"
                }
                Button {
                    text: "Browse..."
                    onClicked: iconFileDialog.open()
                }
            }

            // Notification type selector
            Row {
                width: parent.width
                spacing: 10
                Label {
                    text: "Type:"
                    width: 100
                    anchors.verticalCenter: parent.verticalCenter
                }
                ComboBox {
                    id: typeCombo
                    width: parent.width - 110
                    model: ["Information", "Warning", "Error", "Success"]
                    currentIndex: 0
                }
            }
        }

        Column {
            Layout.fillWidth: true
            spacing: 10

            Button {
                width: parent.width
                text: "Send Simple Notification"
                onClicked: {
                    if (notifications.isSupported()) {
                        var title = titleField.text || "QML Test"
                        var message = messageField.text || "This is a simple notification from QML!"
                        var type = typeCombo.currentIndex
                        notifications.sendNotification(title, message, type)
                        logText.text += "✓ Sent simple notification (type: " + typeCombo.currentText + ")\n"
                    } else {
                        logText.text += "⚠ Notifications not supported\n"
                    }
                }
            }

            Button {
                width: parent.width
                text: "Send Notification with Icon"
                onClicked: {
                    if (notifications.isSupported()) {
                        var title = titleField.text || "QML Test with Icon"
                        var message = messageField.text || "This notification includes an icon path"
                        var iconPath = iconPathField.text || ""
                        var type = typeCombo.currentIndex
                        if (iconPath) {
                            notifications.sendNotification(title, message, iconPath, type)
                            logText.text += "✓ Sent notification with icon (type: " + typeCombo.currentText + ")\n"
                        } else {
                            logText.text += "⚠ Please select an icon file first\n"
                        }
                    } else {
                        logText.text += "⚠ Notifications not supported\n"
                    }
                }
            }

            Button {
                width: parent.width
                text: "Send Notification with Actions"
                onClicked: {
                    if (notifications.isSupported()) {
                        var title = titleField.text || "QML Test with Actions"
                        var message = messageField.text || "Choose an action below:"
                        var actions = {
                            "open": "Open",
                            "dismiss": "Dismiss",
                            "reply": "Reply"
                        }
                        var type = typeCombo.currentIndex
                        notifications.sendNotification(title, message, actions, type)
                        logText.text += "✓ Sent notification with actions (type: " + typeCombo.currentText + ")\n"
                    } else {
                        logText.text += "⚠ Notifications not supported\n"
                    }
                }
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            height: 200
            clip: true

            Text {
                id: logText
                text: "Waiting for notification events...\n"
                font.family: "monospace"
                font.pixelSize: 12
                wrapMode: Text.WordWrap
                color: "black"
            }
        }

        Button {
            Layout.fillWidth: true
            text: "Clear Log"
            onClicked: {
                logText.text = "Log cleared.\n"
            }
        }
    }

    Connections {
        target: notifications
        function onNotificationClosed(notificationId) {
            logText.text += "✓ Notification " + notificationId + " was closed!\n"
        }
        function onActionInvoked(notificationId, actionKey) {
            logText.text += "⚡ Action invoked for notification " + notificationId + " with key: " + actionKey + "\n"
            if (actionKey === "open") {
                logText.text += "  → Opening application...\n"
            } else if (actionKey === "dismiss") {
                logText.text += "  → Dismissing notification...\n"
            } else if (actionKey === "reply") {
                logText.text += "  → Opening reply dialog...\n"
            }
        }
        function onNotificationClicked(notificationId) {
            logText.text += "👆 Notification " + notificationId + " was clicked!\n"
        }
    }

    Component.onCompleted: {
        if (!notifications.isSupported()) {
            logText.text += "⚠ Notifications are not supported on this platform\n"
        } else {
            logText.text += "✓ Notifications are supported on this platform\n"
        }
    }
}
