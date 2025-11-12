#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QFileDialog>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QStandardPaths>
#include <QtCore/QTimer>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <qnotifications.h>

class NotificationsWidget : public QMainWindow
{
public:
    NotificationsWidget(QWidget *parent = nullptr)
        : QMainWindow(parent)
        , notifications(new QNotifications(this))
    {
        setWindowTitle("Qt Notifications Widgets Example: " + qApp->organizationDomain() + "." + qApp->applicationName());
        setMinimumSize(600, 500);

        // Check if notifications are supported
        if (!notifications->isSupported()) {
            QMessageBox::warning(this, "Not Supported",
                "Notifications are not supported on this platform!");
        }

        setupUI();
        setupConnections();
    }

private:
    QVariantMap getParameters() const
    {
        QVariantMap params;
        for (int row = 0; row < parametersTable->rowCount(); ++row) {
            QTableWidgetItem *keyItem = parametersTable->item(row, 0);
            QTableWidgetItem *valueItem = parametersTable->item(row, 1);
            if (keyItem && valueItem && !keyItem->text().isEmpty()) {
                QString key = keyItem->text();
                QString value = valueItem->text();
                // Try to convert to int if it's a valid number
                bool ok;
                int intValue = value.toInt(&ok);
                if (ok && intValue != 0) {
                    params[key] = intValue;
                } else {
                    params[key] = value;
                }
            }
        }

        // Build image-data structure for Linux if image is loaded
        if (!m_loadedImage.isNull()) {
            QVariantMap imageDataStruct = buildImageDataStructure(m_loadedImage);
            if (!imageDataStruct.isEmpty()) {
                params["image-data"] = imageDataStruct;
            }

            QString tempPath = saveImageToTempFile(m_loadedImage);
            params["appLogoOverride"] = tempPath;
        }

        return params;
    }

    QVariantMap buildImageDataStructure(const QImage &image) const
    {
        if (image.isNull())
            return QVariantMap();

        // Convert to RGB or RGBA format
        QImage convertedImage = image;
        bool hasAlpha = image.hasAlphaChannel();

        if (hasAlpha) {
            convertedImage = image.convertToFormat(QImage::Format_RGBA8888);
        } else {
            convertedImage = image.convertToFormat(QImage::Format_RGB888);
        }

        int width = convertedImage.width();
        int height = convertedImage.height();
        int rowstride = convertedImage.bytesPerLine();
        int channels = hasAlpha ? 4 : 3;
        int bitsPerSample = 8;

        // Get image data in RGB/RGBA byte order
        QByteArray imageData;
        const uchar *bits = convertedImage.constBits();
        int dataSize = convertedImage.sizeInBytes();
        imageData = QByteArray(reinterpret_cast<const char*>(bits), dataSize);

        // Build the DBus structure (iiibiiay)
        QVariantMap imageDataStruct;
        imageDataStruct["width"] = width;
        imageDataStruct["height"] = height;
        imageDataStruct["rowstride"] = rowstride;
        imageDataStruct["has_alpha"] = hasAlpha;
        imageDataStruct["bits_per_sample"] = bitsPerSample;
        imageDataStruct["channels"] = channels;
        imageDataStruct["data"] = imageData;

        return imageDataStruct;
    }

    QString saveImageToTempFile(const QImage &image) const
    {
        QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                           + "/notification_icon.png";
        image.save(tempPath);
        return tempPath;
    }

    void loadImageFromResource()
    {
        QString imagePath = ":/images/test.png";

        QImage image(imagePath);
        if (image.isNull())
            return;

        m_loadedImage = image;
        imagePreviewLabel->setPixmap(QPixmap::fromImage(image.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        imagePathLabel->setText(QString("Loaded: %1 (%2x%3)").arg(imagePath).arg(image.width()).arg(image.height()));
        updateStatus(QString("Image loaded: %1 (%2x%3, %4 channels)")
                     .arg(imagePath)
                     .arg(image.width())
                     .arg(image.height())
                     .arg(image.hasAlphaChannel() ? 4 : 3));
    }

    void loadImageFromFile()
    {
        QString fileName = QFileDialog::getOpenFileName(this,
            tr("Select Image File"),
            QDir::homePath(),
            tr("Image Files (*.png *.jpg *.jpeg *.bmp);;All Files (*)"));

        if (!fileName.isEmpty()) {
            QImage image(fileName);
            if (image.isNull()) {
                QMessageBox::warning(this, "Invalid Image",
                    "Failed to load image from: " + fileName);
                return;
            }

            m_loadedImage = image;
            imagePreviewLabel->setPixmap(QPixmap::fromImage(image.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
            imagePathLabel->setText(QString("Loaded: %1 (%2x%3)").arg(QFileInfo(fileName).fileName()).arg(image.width()).arg(image.height()));
            updateStatus(QString("Image loaded from file: %1 (%2x%3, %4 channels)")
                         .arg(fileName)
                         .arg(image.width())
                         .arg(image.height())
                         .arg(image.hasAlphaChannel() ? 4 : 3));
        }
    }

    void clearImage()
    {
        m_loadedImage = QImage();
        imagePreviewLabel->clear();
        imagePreviewLabel->setText("No image");
        imagePathLabel->clear();
        updateStatus("Image cleared");
    }

    void sendSimpleNotification()
    {
        QVariantMap params = getParameters();
        uint notificationId = notifications->sendNotification(
            titleEdit->text().isEmpty() ? "Qt Notifications Test" : titleEdit->text(),
            messageEdit->toPlainText().isEmpty() ? "This is a test notification from the Qt Notifications module!" : messageEdit->toPlainText(),
            params
        );

        updateStatus(notificationId > 0 ? QString("Simple notification sent successfully! (ID: %1)").arg(notificationId) : "Failed to send simple notification");
    }

    void sendNotificationWithActions()
    {
        QMap<QString, QString> actions;
        actions["open"] = "Open";
        actions["dismiss"] = "Dismiss";
        actions["reply"] = "Reply";

        QVariantMap params = getParameters();
        uint notificationId = notifications->sendNotification(
            titleEdit->text().isEmpty() ? "Qt Notifications with Actions" : titleEdit->text(),
            messageEdit->toPlainText().isEmpty() ? "Choose an action below:" : messageEdit->toPlainText(),
            params,
            actions
        );

        updateStatus(notificationId > 0 ? QString("Notification with actions sent successfully! (ID: %1)").arg(notificationId) : "Failed to send notification with actions");
    }

    void addParameterRow()
    {
        int row = parametersTable->rowCount();
        parametersTable->insertRow(row);
        parametersTable->setItem(row, 0, new QTableWidgetItem());
        parametersTable->setItem(row, 1, new QTableWidgetItem());
        parametersTable->selectRow(row);
    }

    void removeParameterRow()
    {
        int currentRow = parametersTable->currentRow();
        if (currentRow >= 0) {
            parametersTable->removeRow(currentRow);
        }
    }

    void onNotificationClosed(uint notificationId, QNotifications::ClosedReason closedReason)
    {
        updateStatus(QString("Notification was closed %1: %2").arg(notificationId).arg(QMetaEnum::fromType<QNotifications::ClosedReason>().valueToKey(closedReason)));
    }

    void onActionInvoked(uint notificationId, const QString &actionKey)
    {
        updateStatus(QString("Action invoked for notification %1: %2").arg(notificationId).arg(actionKey));
    }

    void onNotificationClicked(uint notificationId)
    {
        updateStatus(QString("Notification %1 was clicked!").arg(notificationId));
    }

private:
    void setupUI()
    {
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

        // Title and message input group
        QGroupBox *inputGroup = new QGroupBox("Notification Content", this);
        QVBoxLayout *inputLayout = new QVBoxLayout(inputGroup);

        // Title input
        QHBoxLayout *titleLayout = new QHBoxLayout();
        titleLayout->addWidget(new QLabel("Title:", this));
        titleEdit = new QLineEdit(this);
        titleEdit->setPlaceholderText("Enter notification title...");
        titleLayout->addWidget(titleEdit);
        inputLayout->addLayout(titleLayout);

        // Message input
        QHBoxLayout *messageLayout = new QHBoxLayout();
        messageLayout->addWidget(new QLabel("Message:", this));
        messageEdit = new QTextEdit(this);
        messageEdit->setMaximumHeight(80);
        messageEdit->setPlaceholderText("Enter notification message...");
        messageLayout->addWidget(messageEdit);
        inputLayout->addLayout(messageLayout);

        mainLayout->addWidget(inputGroup);

        // Parameters group
        QGroupBox *parametersGroup = new QGroupBox("Parameters (Platform-specific)", this);
        QVBoxLayout *parametersLayout = new QVBoxLayout(parametersGroup);

        QLabel *paramsHint = new QLabel("Add platform-specific parameters (e.g., icon, urgency, appLogoOverride, etc.)", this);
        paramsHint->setWordWrap(true);
        paramsHint->setStyleSheet("color: gray; font-style: italic;");
        parametersLayout->addWidget(paramsHint);

        parametersTable = new QTableWidget(this);
        parametersTable->setColumnCount(2);
        parametersTable->setHorizontalHeaderLabels(QStringList() << "Key" << "Value");
        parametersTable->horizontalHeader()->setStretchLastSection(true);
        parametersTable->setMaximumHeight(150);
        parametersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        parametersLayout->addWidget(parametersTable);

        QHBoxLayout *paramButtonsLayout = new QHBoxLayout();
        QPushButton *addParamBtn = new QPushButton("Add Parameter", this);
        QPushButton *removeParamBtn = new QPushButton("Remove Parameter", this);
        paramButtonsLayout->addWidget(addParamBtn);
        paramButtonsLayout->addWidget(removeParamBtn);
        paramButtonsLayout->addStretch();
        parametersLayout->addLayout(paramButtonsLayout);

        // Add some common parameter examples
        int row = 0;
        parametersTable->insertRow(row);
        parametersTable->setItem(row, 0, new QTableWidgetItem("icon"));
        parametersTable->setItem(row, 1, new QTableWidgetItem(""));
        row++;
        parametersTable->insertRow(row);
        parametersTable->setItem(row, 0, new QTableWidgetItem("urgency"));
        parametersTable->setItem(row, 1, new QTableWidgetItem("1"));

        connect(addParamBtn, &QPushButton::clicked, this, &NotificationsWidget::addParameterRow);
        connect(removeParamBtn, &QPushButton::clicked, this, &NotificationsWidget::removeParameterRow);

        mainLayout->addWidget(parametersGroup);

        // Image Data group (for Linux image-data DBus structure)
        QGroupBox *imageDataGroup = new QGroupBox("Image Data (Linux image-data)", this);
        QVBoxLayout *imageDataLayout = new QVBoxLayout(imageDataGroup);

        QLabel *imageDataHint = new QLabel(
            "For Linux: Load an image to create image-data DBus structure (iiibiiay).\n"
            "The structure contains: width, height, rowstride, has_alpha, bits_per_sample, channels, data.", this);
        imageDataHint->setWordWrap(true);
        imageDataHint->setStyleSheet("color: gray; font-style: italic;");
        imageDataLayout->addWidget(imageDataHint);

        QHBoxLayout *imageButtonsLayout = new QHBoxLayout();
        QPushButton *loadFromResourceBtn = new QPushButton("Load from QRC", this);
        QPushButton *loadFromFileBtn = new QPushButton("Load from File", this);
        QPushButton *clearImageBtn = new QPushButton("Clear", this);
        imageButtonsLayout->addWidget(loadFromResourceBtn);
        imageButtonsLayout->addWidget(loadFromFileBtn);
        imageButtonsLayout->addWidget(clearImageBtn);
        imageButtonsLayout->addStretch();
        imageDataLayout->addLayout(imageButtonsLayout);

        QHBoxLayout *imagePreviewLayout = new QHBoxLayout();
        imagePreviewLabel = new QLabel("No image", this);
        imagePreviewLabel->setMinimumSize(64, 64);
        imagePreviewLabel->setMaximumSize(64, 64);
        imagePreviewLabel->setAlignment(Qt::AlignCenter);
        imagePreviewLabel->setStyleSheet("border: 1px solid gray; background-color: #f0f0f0;");
        imagePreviewLayout->addWidget(imagePreviewLabel);

        imagePathLabel = new QLabel("", this);
        imagePathLabel->setWordWrap(true);
        imagePreviewLayout->addWidget(imagePathLabel);
        imagePreviewLayout->addStretch();
        imageDataLayout->addLayout(imagePreviewLayout);

        connect(loadFromResourceBtn, &QPushButton::clicked, this, &NotificationsWidget::loadImageFromResource);
        connect(loadFromFileBtn, &QPushButton::clicked, this, &NotificationsWidget::loadImageFromFile);
        connect(clearImageBtn, &QPushButton::clicked, this, &NotificationsWidget::clearImage);

        mainLayout->addWidget(imageDataGroup);

        // Buttons group
        QGroupBox *buttonsGroup = new QGroupBox("Send Notifications", this);
        QVBoxLayout *buttonsLayout = new QVBoxLayout(buttonsGroup);

        QPushButton *simpleBtn = new QPushButton("Send Simple Notification", this);
        QPushButton *actionsBtn = new QPushButton("Send Notification with Actions", this);

        buttonsLayout->addWidget(simpleBtn);
        buttonsLayout->addWidget(actionsBtn);

        mainLayout->addWidget(buttonsGroup);

        // Status display
        QGroupBox *statusGroup = new QGroupBox("Status Log", this);
        QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);
        statusText = new QTextEdit(this);
        statusText->setReadOnly(true);
        statusText->setMaximumHeight(150);
        statusLayout->addWidget(statusText);
        mainLayout->addWidget(statusGroup);

        // Connect button signals
        connect(simpleBtn, &QPushButton::clicked, this, &NotificationsWidget::sendSimpleNotification);
        connect(actionsBtn, &QPushButton::clicked, this, &NotificationsWidget::sendNotificationWithActions);
    }

    void setupConnections()
    {
        connect(notifications, &QNotifications::notificationClosed,
                this, &NotificationsWidget::onNotificationClosed);
        connect(notifications, &QNotifications::actionInvoked,
                this, &NotificationsWidget::onActionInvoked);
        connect(notifications, &QNotifications::notificationClicked,
                this, &NotificationsWidget::onNotificationClicked);
    }

    void updateStatus(const QString &message)
    {
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
        statusText->append(QString("[%1] %2").arg(timestamp, message));
        qInfo() << message;
    }

private:
    QNotifications *notifications;
    QLineEdit *titleEdit;
    QTextEdit *messageEdit;
    QTableWidget *parametersTable;
    QLabel *imagePreviewLabel;
    QLabel *imagePathLabel;
    QImage m_loadedImage;
    QTextEdit *statusText;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    NotificationsWidget widget;
    widget.showMaximized();

    return app.exec();
}
