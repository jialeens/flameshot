
#pragma once

#include <QWidget>
#include <QUrl>
#include <QHttpMultiPart>

class QNetworkReply;
class QNetworkAccessManager;
class QHBoxLayout;
class QVBoxLayout;
class QLabel;
class LoadSpinner;
class QPushButton;
class QUrl;
class NotificationWidget;

class IBedUploader : public QWidget
{
    Q_OBJECT
public:
    explicit IBedUploader(const QPixmap& capture, QWidget* parent = nullptr);

private slots:
    void handleUploadReply(QNetworkReply* reply);
    void handleDeleteReply(QNetworkReply* reply);
    void startDrag();
    void openURL();
    void copyURL();
    void openDeleteURL();
    void copyImage();
    void copyMarkDown();

private:
    QPixmap m_pixmap;
    QNetworkAccessManager* m_NetworkUpload;
    QNetworkAccessManager* m_NetworkDelete;
    QString m_fileName;

    QVBoxLayout* m_vLayout;
    QHBoxLayout* m_hLayout;
    // loading
    QLabel* m_infoLabel;
    LoadSpinner* m_spinner;
    // uploaded
    QPushButton* m_openUrlButton;
    QPushButton* m_openDeleteUrlButton;
    QPushButton* m_copyUrlButton;
    QPushButton* m_toClipboardButton;
    QPushButton* m_copyMarkDownButton;
    QUrl m_imageURL;
    QUrl m_deleteImageURL;
    NotificationWidget* m_notification;
    QHttpPart textPart;
    QHttpPart imagePart;
    void upload();
    void onUploadOk();
};
