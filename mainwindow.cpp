#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <opencv2/imgproc.hpp>
#include <QtConcurrent/QtConcurrent>
#include <QMap>
#include <QProcess>


QTimer *timer = new QTimer();

QMap<QString,QString> QrProductMap = {
    {"A001001","Top"},
    {"B001001","Ayakkabı"},
    {"B002002","Şampuan"}
    };

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)

{
    ui->setupUi(this);


}

MainWindow::~MainWindow()
{
    delete ui;

    on_closeCam_clicked();
}

void MainWindow::connectToWifi(const QString &ssid, const QString &password)
{
#ifdef Q_OS_WIN
    QString profile = QString(
                          "<?xml version=\"1.0\"?>\n"
                          "<WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\">\n"
                          "    <name>%1</name>\n"
                          "    <SSIDConfig>\n"
                          "        <SSID>\n"
                          "            <name>%1</name>\n"
                          "        </SSID>\n"
                          "    </SSIDConfig>\n"
                          "    <connectionType>ESS</connectionType>\n"
                          "    <connectionMode>manual</connectionMode>\n"
                          "    <MSM>\n"
                          "        <security>\n"
                          "            <authEncryption>\n"
                          "                <authentication>WPA2PSK</authentication>\n"
                          "                <encryption>AES</encryption>\n"
                          "                <useOneX>false</useOneX>\n"
                          "            </authEncryption>\n"
                          "            <sharedKey>\n"
                          "                <keyType>passPhrase</keyType>\n"
                          "                <protected>false</protected>\n"
                          "                <keyMaterial>%2</keyMaterial>\n"
                          "            </sharedKey>\n"
                          "        </security>\n"
                          "    </MSM>\n"
                          "</WLANProfile>"
                          ).arg(ssid, password);

    QFile file("wifi_profile.xml");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << profile;
        file.close();
    } else {
        qDebug() << "XML dosyası oluşturulamadı!";
        return;
    }

    QProcess::execute("netsh wlan add profile filename=\"wifi_profile.xml\"");
    QProcess::execute(QString("netsh wlan connect name=\"%1\"").arg(ssid));
    qDebug() << "WiFi ağı eklendi ve bağlanıldı: " << ssid;

#elif defined(Q_OS_LINUX)
    QString command = QString("nmcli dev wifi connect \"%1\" password \"%2\"").arg(ssid, password);
#elif defined(Q_OS_MAC)
    QString command = QString("networksetup -setairportnetwork en0 \"%1\" \"%2\"").arg(ssid, password);
#endif

    qDebug() << "WiFi bağlanma süreci tamamlandı.";
}

void MainWindow::parseWiFiInfo(const std::string &qrData) {
    if (qrData.find("WIFI:") != 0) {  // "WIFI:" başta olmalı
        qDebug() << "Geçersiz QR kod formatı!";
        return;
    }

    QString ssid, password;

    size_t pos = 0;
    while (pos < qrData.length()) {
        size_t keyStart = qrData.find(";", pos);
        if (keyStart == std::string::npos) break;

        std::string keyValue = qrData.substr(pos, keyStart - pos);
        size_t separator = keyValue.find(":");

        if (separator != std::string::npos) {
            std::string key = keyValue.substr(0, separator);
            std::string value = keyValue.substr(separator + 1);

            if (key == "S") {
                ssid = QString::fromStdString(value);
            } else if (key == "P") {
                password = QString::fromStdString(value);
            }
        }
        pos = keyStart + 1;
    }

    if (ssid.isEmpty() || password.isEmpty()) {
        qDebug() << "QR kodundan SSID veya Şifre okunamadı!";
        return;
    }

    qDebug() << "SSID:" << ssid << "Şifre:" << password;
    qDebug() <<QString(QLatin1String("netsh wlan connect name=\"%1\" password=\"%2\"")).arg(ssid, password);
    connectToWifi(ssid, password);
}


void MainWindow::on_opencam_clicked()
{

    on_closeCam_clicked();

    qDebug() << "Kamera açılıyor...";

    QtConcurrent::run([this]() {
        if (!cap) {
            cap = cv::makePtr<cv::VideoCapture>(0);  // Kamera başlat
        }

        if (!cap->isOpened()) {
            qDebug() << "Kamera açılmadı!";
            return;
        }

        if (!qr) {
            qr = cv::makePtr<cv::QRCodeDetector>();  // QR kod detektörünü başlat
        }

        if (!barcode) {
            barcode = cv::makePtr<cv::barcode::BarcodeDetector>(); // Barkod detektörünü başlat
        }

        QMetaObject::invokeMethod(this, [this]() {
            try {
                connect(timer, &QTimer::timeout, this, &MainWindow::processFrame);
                timer->start(30);
            } catch (const std::exception &e) {
                qDebug() << "invokeMethod hatası: " << e.what();
            } catch (...) {
                qDebug() << "invokeMethod içinde bilinmeyen hata!";
            }
        }, Qt::QueuedConnection);
    });
}

void MainWindow::processFrame()
{
    if (!cap || !cap->isOpened()) {
        qDebug() << "Kamera aktif değil!";
        return;
    }

    if (!frame) {
        frame = cv::makePtr<cv::Mat>();  // Mat nesnesini oluştur
    }

    cap->read(*frame);  // Kameradan kare al

    if (frame->empty()) {
        qDebug() << "Boş kare alındı!";
        return;
    }

    // QR kod tespiti
    std::vector<cv::Point> qrPoints;
    std::string qrData = qr->detectAndDecode(*frame, qrPoints);

    if (!qrData.empty()) {
        qDebug() << "QR Kod İçeriği: " << QString::fromStdString(qrData);
        ui->textEdit->setText(QString::fromStdString(qrData));

        parseWiFiInfo(qrData);

        if(QrProductMap.contains(QString::fromStdString(qrData)))
        {
            ui->ProductNameTextEdit->setText(QrProductMap[QString::fromStdString(qrData)]);

        }
        // QR Kodunun etrafına dikdörtgen çiz
        if (qrPoints.size() == 4) { // 4 köşe noktası varsa
            for (int i = 0; i < 4; i++) {
                cv::line(*frame, qrPoints[i], qrPoints[(i + 1) % 4], cv::Scalar(0, 255, 0), 3);
            }
        }
    }

    //barkod tespiti

    std::vector<cv::Point> barcodePoints;
    std::string barcodeData = barcode->detectAndDecode(*frame, barcodePoints);

    if (!barcodeData.empty()) {
        qDebug() << "Barkod İçeriği: " << QString::fromStdString(barcodeData);
        ui->textEdit->setText(QString::fromStdString(barcodeData));


        // Barkodun etrafına dikdörtgen çiz
        if (barcodePoints.size() == 4) { // 4 köşe noktası varsa
            for (int i = 0; i < 4; i++) {
                cv::line(*frame, barcodePoints[i], barcodePoints[(i + 1) % 4], cv::Scalar(255, 0, 0), 3);
            }
        }
    }


    // OpenCV görüntüsünü QLabel'e aktarma
    cv::cvtColor(*frame, *frame, cv::COLOR_BGR2RGB);
    QImage qImg(frame->data, frame->cols, frame->rows, frame->step, QImage::Format_RGB888);
    ui->camLabel->setPixmap(QPixmap::fromImage(qImg).scaled(ui->camLabel->size(), Qt::KeepAspectRatio));
}

void MainWindow::on_closeCam_clicked()
{
    qDebug() << "Kamera kapatılıyor...";

    // Öncelikle zamanlayıcıyı durdur ve bağlantıyı kes
    if (timer) {
        timer->stop();
        disconnect(timer, &QTimer::timeout, this, &MainWindow::processFrame);
    }

    // Kamera cihazını serbest bırak
    if (cap && cap->isOpened()) {
        cap->release();  // Kamera bağlantısını kapat
    }
    cap.reset();  // Akıllı pointer olduğu için `delete` gerekmez

    // Kareyi temizle
    if (frame && !frame->empty()) {
        frame->release();
    }
    frame.reset();  // Mat nesnesini temizle

    // QR Code Detector'ü temizle
    qr.reset();

    barcode.reset();

    // QLabel'i temizle (siyah ekran yapmak için)
    ui->camLabel->clear();
    qDebug() << "Kamera başarıyla kapatıldı.";
}


void MainWindow::on_actionOpen_triggered()
{
    on_opencam_clicked();
}



void MainWindow::on_actionExit_triggered()
{
    this->close();
    on_closeCam_clicked();
}

