#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/objdetect.hpp>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:


    void on_opencam_clicked();

    void on_closeCam_clicked();

    void processFrame();

private:
    Ui::MainWindow *ui;

    cv::Ptr<cv::VideoCapture> cap;
    cv::Ptr<cv::QRCodeDetector> qr;
    cv::Ptr<cv::Mat> frame;
    cv::Ptr<cv::barcode::BarcodeDetector> barcode;

};
#endif // MAINWINDOW_H
