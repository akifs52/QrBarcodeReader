#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <opencv2/imgproc.hpp>
#include <QtConcurrent/QtConcurrent>
#include <QMap>

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

