#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QImage>
#include <QPixmap>
#include <QDateTime>
#include <QDir>
#include <QTimer>
#include <QString>
#include <QRegularExpression>
#include <QThread>
#include <QtConcurrent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , timer(new QTimer(this))
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentWidget(ui->page);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateFrame);

    processingThread = QtConcurrent::run([this]() {
        while (true) {
            if (!pendingQR.isEmpty() && !isProcessing.load()) {
                processQR(pendingQR);
                pendingQR.clear();
            }
            QThread::msleep(100);
        }
    });
}

MainWindow::~MainWindow() {
    topCam.stop();
    rtspCam.stop();
    rtspCam2.stop();
    rtspCam3.stop();
    rtspCam4.stop();
    delete ui;
}

void MainWindow::on_launchButton_clicked() {
    QDir().mkpath("D:/zeiss-video");
    QDir().mkpath("D:/zeiss-photo");
    bool ok1 = topCam.start("0", true);
    bool ok2 = rtspCam.start("rtsp://192.168.1.58:554/user=admin&password=&channel=1&stream=0.sdp");
    bool ok3 = rtspCam2.start("rtsp://192.168.1.62:554/user=admin&password=&channel=1&stream=0.sdp");
    bool ok4 = rtspCam3.start("rtsp://192.168.1.61:554/user=admin&password=&channel=1&stream=0.sdp");
    bool ok5 = rtspCam4.start("rtsp://192.168.1.64:554/user=admin&password=&channel=1&stream=0.sdp");

    ui->textEdit->append(ok1 ? "✅ USB camera started." : "❌ Failed to open USB camera.");
    ui->textEdit->append(ok2 ? "✅ RTSP camera 1 started." : "❌ Failed to open RTSP camera 1.");
    ui->textEdit->append(ok3 ? "✅ RTSP camera 2 started." : "❌ Failed to open RTSP camera 2.");
    ui->textEdit->append(ok4 ? "✅ RTSP camera 3 started." : "❌ Failed to open RTSP camera 3.");
    ui->textEdit->append(ok5 ? "✅ RTSP camera 4 started." : "❌ Failed to open RTSP camera 4.");

    timer->start(30);
    ui->stackedWidget->setCurrentWidget(ui->page_2);
}

void MainWindow::on_exitButton_clicked() {
    close();
}

void MainWindow::updateFrame() {
    // USB camera
    cv::Mat frame;
    if (topCam.read(frame)) {
        std::string decoded;
        std::vector<cv::Point> points;
        decoded = qrDecoder.detectAndDecode(frame, points);
        if (!decoded.empty() && !processedQRCodes.contains(QString::fromStdString(decoded))) {
            QString qrText = QString::fromStdString(decoded);
            processedQRCodes.insert(qrText);
            pendingQR = qrText;
        }

        QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        cv::putText(frame, ts.toStdString(), {20,30}, cv::FONT_HERSHEY_SIMPLEX, 1.0, {0,255,0}, 2);
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        QImage img(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
        ui->label->setPixmap(QPixmap::fromImage(img).scaled(ui->label->size(), Qt::KeepAspectRatio));
    }

    // RTSP camera previews
    CamThread* cams[4] = { &rtspCam, &rtspCam2, &rtspCam3, &rtspCam4 };
    QLabel* labels[4] = { ui->label_2, ui->label_3, ui->label_4, ui->label_5 };

    for (int i = 0; i < 4; ++i) {
        cv::Mat img;
        if (cams[i]->read(img)) {
            cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
            QImage qimg(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888);
            labels[i]->setPixmap(QPixmap::fromImage(qimg).scaled(labels[i]->size(), Qt::KeepAspectRatio));
        }
    }
}

void MainWindow::processQR(const QString& qrCode) {
    isProcessing = true;
    QMetaObject::invokeMethod(ui->textEdit, [=]() {
        ui->textEdit->append("🔍 QR Code Detected: " + qrCode);
    }, Qt::QueuedConnection);

    QString safe = qrCode;
    safe.replace(QRegularExpression(R"([\\/:*?"<>|])"), "_");
    QString videoDir = QString("D:/zeiss-video/%1").arg(safe);
    QString photoBase = QString("D:/zeiss-photo/%1").arg(safe);
    QString camDirs[4] = { photoBase + "/cam1", photoBase + "/cam2", photoBase + "/cam3", photoBase + "/cam4" };

    QDir().mkpath(videoDir);
    for (int i = 0; i < 4; ++i) QDir().mkpath(camDirs[i]);
    QString videoFile = videoDir + "/recorded.avi";

    QMetaObject::invokeMethod(ui->textEdit, [=]() {
        ui->textEdit->append("⏺️ Start recording video...");
    }, Qt::QueuedConnection);

    cv::VideoWriter writer(videoFile.toStdString(), cv::VideoWriter::fourcc('M','J','P','G'), 30, {640,480});
    if (!writer.isOpened()) {
        QMetaObject::invokeMethod(ui->textEdit, [=]() {
            ui->textEdit->append("❌ Failed to open video writer");
        }, Qt::QueuedConnection);
        isProcessing = false;
        return;
    }

    for (int i = 0; i < 300; ++i) {
        cv::Mat f;
        if (topCam.read(f)) {
            QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
            cv::putText(f, ts.toStdString(), {20,30}, cv::FONT_HERSHEY_SIMPLEX, 0.8, {255,255,255}, 2);
            writer.write(f);
        }
        QThread::msleep(33);
    }
    writer.release();

    QMetaObject::invokeMethod(ui->textEdit, [=]() {
        ui->textEdit->append("🎥 Video saved: " + videoFile);
        ui->textEdit->append("📸 Start taking photos...");
    }, Qt::QueuedConnection);

    QThread::sleep(1);

    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 95};
    CamThread* cams[4] = { &rtspCam, &rtspCam2, &rtspCam3, &rtspCam4 };

    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 4; ++j) {
            cv::Mat img;
            if (cams[j]->read(img)) {
                QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
                cv::putText(img, ts.toStdString(), {20,30}, cv::FONT_HERSHEY_SIMPLEX, 0.8, {255,255,255}, 2);
                QString fname = QString("%1/photo_%2.jpg").arg(camDirs[j]).arg(i);
                bool ok = cv::imwrite(fname.toStdString(), img, params);
                QMetaObject::invokeMethod(ui->textEdit, [=]() {
                    ui->textEdit->append(ok ? QString("✅ cam%1 Saved: %2").arg(j+1).arg(fname)
                                            : QString("❌ cam%1 Failed: %2").arg(j+1).arg(fname));
                }, Qt::QueuedConnection);
            }
        }
        QThread::msleep(300);
    }

    QMetaObject::invokeMethod(ui->textEdit, [=]() {
        ui->textEdit->append("📸 Photo capture complete.");
    }, Qt::QueuedConnection);

    isProcessing = false;
}
