
#include "ibeduploader.h"
#include "src/utils/confighandler.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/history.h"
#include "src/utils/systemnotification.h"
#include "src/widgets/imagelabel.h"
#include "src/widgets/loadspinner.h"
#include "src/widgets/notificationwidget.h"
#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QCursor>
#include <QDesktopServices>
#include <QDrag>
#include <QFileInfo>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMimeData>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QRect>
#include <QScreen>
#include <QShortcut>
#include <QTemporaryFile>
#include <QTextCodec>
#include <QTimer>
#include <QUrlQuery>
#include <QUuid>
#include <QVBoxLayout>

IBedUploader::IBedUploader(const QPixmap& capture, QWidget* parent)
  : QWidget(parent)
  , m_pixmap(capture)
{
    setWindowTitle(tr("Upload to Bed"));
    setWindowIcon(QIcon(":img/app/flameshot.svg"));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    QRect position = frameGeometry();
    QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
    position.moveCenter(screen->availableGeometry().center());
    move(position.topLeft());
#endif

    m_spinner = new LoadSpinner(this);
    m_spinner->setColor(ConfigHandler().uiMainColorValue());
    m_spinner->start();

    m_infoLabel = new QLabel(tr("Uploading Image"));

    m_vLayout = new QVBoxLayout();
    setLayout(m_vLayout);
    m_vLayout->addWidget(m_spinner, 0, Qt::AlignHCenter);
    m_vLayout->addWidget(m_infoLabel);

    m_NetworkUpload = new QNetworkAccessManager(this);
    connect(m_NetworkUpload,
            &QNetworkAccessManager::finished,
            this,
            &IBedUploader::handleUploadReply);

    m_NetworkDelete = new QNetworkAccessManager(this);
    connect(m_NetworkDelete,
            &QNetworkAccessManager::finished,
            this,
            &IBedUploader::handleDeleteReply);

    setAttribute(Qt::WA_DeleteOnClose);

    upload();
    // QTimer::singleShot(2000, this, &BedUploader::onUploadOk); // testing
}

void IBedUploader::handleUploadReply(QNetworkReply* reply)
{
    m_spinner->deleteLater();
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
        QJsonObject json = response.object();
        QString url = json[QStringLiteral("url")].toString();
        QString id;
        m_imageURL.setUrl(url);
        QFileInfo fileinfo;
        fileinfo = QFileInfo(url);
        id = fileinfo.baseName();
        QByteArray userName = ConfigHandler().getUserName().toUtf8();
        if (userName.length() == 0) {
            userName = "jiale";
        }
        m_deleteImageURL.setUrl(
          QStringLiteral(IBED_UPLOAD_URL) +
          QStringLiteral("%1/%2/%3").arg(userName, id, "true"));

        // save history
        QString imageName = m_imageURL.toString();
        int lastSlash = imageName.lastIndexOf("ibed/");
        if (lastSlash >= 0) {
            imageName = imageName.mid(lastSlash).replace("/", "#");
        }

        // save image to history
        History history;
        imageName = history.packFileName("ibed", id, imageName);
        history.save(m_pixmap, imageName);

        if (ConfigHandler().copyAndCloseAfterUploadEnabled()) {
            SystemNotification().sendMessage(
              QObject::tr("URL copied to clipboard."));
            QApplication::clipboard()->setText(m_imageURL.toString());
            close();
        } else {
            onUploadOk();
        }
    } else {
        m_infoLabel->setText(reply->errorString());
    }
    new QShortcut(Qt::Key_Escape, this, SLOT(close()));
}
void IBedUploader::handleDeleteReply(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        if (reply->readAll() == "true") {
            m_notification->showMessage(tr("Delete Success"));
        }
    }
}

void IBedUploader::startDrag()
{
    QMimeData* mimeData = new QMimeData;
    mimeData->setUrls(QList<QUrl>{ m_imageURL });
    mimeData->setImageData(m_pixmap);

    QDrag* dragHandler = new QDrag(this);
    dragHandler->setMimeData(mimeData);
    dragHandler->setPixmap(m_pixmap.scaled(
      256, 256, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    dragHandler->exec();
}

void IBedUploader::upload()
{

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    m_pixmap.save(&buffer, "PNG");
    QUrlQuery urlQuery;
    QUuid id = QUuid::createUuid();
    QString uuid = id.toString();
    uuid.remove("{").remove("}").remove("-");
    m_fileName = FileNameHandler().parsedPattern();
    m_fileName += QLatin1String(".png");
    QHttpMultiPart* multiPart =
      new QHttpMultiPart(QHttpMultiPart::FormDataType);

    textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"id\""));
    textPart.setBody(uuid.toUtf8());
    multiPart->append(textPart);

    textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"user\""));
    QByteArray userName = ConfigHandler().getUserName().toUtf8();
    if (userName.length() == 0) {
        userName = "jiale";
    }
    textPart.setBody(userName);
    multiPart->append(textPart);

    textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"chunkSize\""));
    textPart.setBody("1");
    multiPart->append(textPart);

    textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"name\""));
    textPart.setBody(m_fileName.toUtf8());
    multiPart->append(textPart);

    imagePart.setHeader(QNetworkRequest::ContentTypeHeader,
                        QVariant("image/jpeg"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        QVariant("form-data; name=\"file\" ; filename=\"" +
                                 m_fileName + ".png\""));
    imagePart.setBody(byteArray);
    multiPart->append(imagePart);

    QUrl url(QStringLiteral(IBED_UPLOAD_URL) +
             QStringLiteral("actions/upload"));
    QNetworkRequest* request = new QNetworkRequest(url);

    m_NetworkUpload->post(*request, multiPart);
}

void IBedUploader::onUploadOk()
{
    m_infoLabel->deleteLater();

    m_notification = new NotificationWidget();
    m_vLayout->addWidget(m_notification);

    ImageLabel* imageLabel = new ImageLabel();
    imageLabel->setScreenshot(m_pixmap);
    imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(
      imageLabel, &ImageLabel::dragInitiated, this, &IBedUploader::startDrag);
    m_vLayout->addWidget(imageLabel);

    m_hLayout = new QHBoxLayout();
    m_vLayout->addLayout(m_hLayout);

    m_copyUrlButton = new QPushButton(tr("Copy URL"));
    m_copyMarkDownButton = new QPushButton(tr("Copy MarkDown"));
    m_openUrlButton = new QPushButton(tr("Open URL"));
    m_openDeleteUrlButton = new QPushButton(tr("Delete image"));
    m_toClipboardButton = new QPushButton(tr("Image to Clipboard."));

    m_hLayout->addWidget(m_copyUrlButton);
    m_hLayout->addWidget(m_copyMarkDownButton);
    m_hLayout->addWidget(m_openUrlButton);
    m_hLayout->addWidget(m_openDeleteUrlButton);
    m_hLayout->addWidget(m_toClipboardButton);

    connect(
      m_copyUrlButton, &QPushButton::clicked, this, &IBedUploader::copyURL);
    connect(
      m_openUrlButton, &QPushButton::clicked, this, &IBedUploader::openURL);
    connect(m_openDeleteUrlButton,
            &QPushButton::clicked,
            this,
            &IBedUploader::openDeleteURL);
    connect(m_toClipboardButton,
            &QPushButton::clicked,
            this,
            &IBedUploader::copyImage);
    connect(m_copyMarkDownButton,
            &QPushButton::clicked,
            this,
            &IBedUploader::copyMarkDown);

    m_notification->showMessage(tr("Image upload success"));
}

void IBedUploader::openURL()
{
    bool successful = QDesktopServices::openUrl(m_imageURL);
    if (!successful) {
        m_notification->showMessage(tr("Unable to open the URL."));
    }
}

void IBedUploader::copyURL()
{
    QApplication::clipboard()->setText(m_imageURL.toString());
    m_notification->showMessage(tr("URL copied to clipboard."));
}

void IBedUploader::openDeleteURL()
{
    QUrl url(this->m_deleteImageURL.toString());
    QNetworkRequest* request = new QNetworkRequest(url);
    m_NetworkDelete->deleteResource(*request);
}

void IBedUploader::copyImage()
{
    QApplication::clipboard()->setPixmap(m_pixmap);
    m_notification->showMessage(tr("Screenshot copied to clipboard."));
}

void IBedUploader::copyMarkDown()
{
    QApplication::clipboard()->setText("![" + m_fileName + "](" +
                                       m_imageURL.toString() + ")");
    m_notification->showMessage(tr("URL copied to clipboard."));
}
