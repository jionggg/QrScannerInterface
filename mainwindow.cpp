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
#include <QElapsedTimer>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , timer(new QTimer(this))
    , timeoutCheckTimer(new QTimer(this))
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentWidget(ui->page);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateFrame);

    lastQRTime.start();
    connect(timeoutCheckTimer, &QTimer::timeout, this, [this]() {
        if (lastQRTime.elapsed() > 120000 && !isProcessing.load()) {
            ui->textEdit->append("🕒 2 minutes passed with no QR detected. Closing app.");
            ui->textEdit->moveCursor(QTextCursor::End);
            qDebug() << "🛑 2 mins up, app closed.";
            QCoreApplication::quit();
        }
    });
    timeoutCheckTimer->start(1000);

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
    for (auto &cam : rtspCams) cam.stop();
    delete ui;
}

void MainWindow::on_launchButton_clicked() {
    QDir().mkpath("D:/zeiss-video");
    QDir().mkpath("D:/zeiss-photo");

    bool ok1 = topCam.start("0", true);
    QString urls[8] = {
        "rtsp://192.168.1.66:554/user=admin&password=&channel=1&stream=0.sdp",
        "rtsp://192.168.1.63:554/user=admin&password=&channel=1&stream=0.sdp",
        "rtsp://192.168.1.62:554/user=admin&password=&channel=1&stream=0.sdp",
        "rtsp://192.168.1.60:554/user=admin&password=&channel=1&stream=0.sdp",
        "rtsp://192.168.1.58:554/user=admin&password=&channel=1&stream=0.sdp",
        "rtsp://192.168.1.65:554/user=admin&password=&channel=1&stream=0.sdp",
        "rtsp://192.168.1.64:554/user=admin&password=&channel=1&stream=0.sdp",
        "rtsp://192.168.1.61:554/user=admin&password=&channel=1&stream=0.sdp"
    };

    QLabel* labels[8] = {
        ui->label_2, ui->label_3, ui->label_4, ui->label_5,
        ui->label_6, ui->label_7, ui->label_8, ui->label_9
    };

    QString messages;
    for (int i = 0; i < 8; ++i) {
        bool started = rtspCams[i].start(urls[i].toStdString());
        messages += started ? QString("✅ RTSP camera %1 started.\n").arg(i + 1)
                            : QString("❌ Failed to open RTSP camera %1.\n").arg(i + 1);
    }

    ui->textEdit->append(ok1 ? "✅ USB camera started." : "❌ Failed to open USB camera.");
    ui->textEdit->append(messages);
    ui->textEdit->moveCursor(QTextCursor::End);

    timer->start(30);
    ui->stackedWidget->setCurrentWidget(ui->page_2);
}

void MainWindow::on_exitButton_clicked() {
    close();
}

void MainWindow::updateFrame() {
    cv::Mat frame;
    if (topCam.read(frame)) {
        std::string decoded;
        std::vector<cv::Point> points;
        decoded = qrDecoder.detectAndDecode(frame, points);
        if (!decoded.empty() && !processedQRCodes.contains(QString::fromStdString(decoded))) {
            QString qrText = QString::fromStdString(decoded);
            processedQRCodes.insert(qrText);
            pendingQR = qrText;
            lastQRTime.restart();
        }

        QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        cv::putText(frame, ts.toStdString(), {20,30}, cv::FONT_HERSHEY_SIMPLEX, 1.0, {0,255,0}, 2);
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        QImage img(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
        ui->label->setPixmap(QPixmap::fromImage(img).scaled(ui->label->size(), Qt::KeepAspectRatio));
    }

    QLabel* labels[8] = {
        ui->label_2, ui->label_3, ui->label_4, ui->label_5,
        ui->label_6, ui->label_7, ui->label_8, ui->label_9
    };
    for (int i = 0; i < 8; ++i) {
        cv::Mat img;
        if (rtspCams[i].read(img)) {
            cv::resize(img, img, cv::Size(320, 240));
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
        ui->textEdit->moveCursor(QTextCursor::End);
    }, Qt::QueuedConnection);

    QString safe = qrCode;
    safe.replace(QRegularExpression(R"([\\/:*?"<>|])"), "_");
    QString videoDir = QString("D:/zeiss-video/%1").arg(safe);
    QString photoBase = QString("D:/zeiss-photo/%1").arg(safe);
    QDir().mkpath(videoDir);
    for (int i = 0; i < 8; ++i)
        QDir().mkpath(photoBase + QString("/cam%1").arg(i+1));

    QString videoFile = videoDir + "/recorded.avi";
    QMetaObject::invokeMethod(ui->textEdit, [=]() {
        ui->textEdit->append("⏺️ Start recording video...");
        ui->textEdit->moveCursor(QTextCursor::End);
    }, Qt::QueuedConnection);

    cv::VideoWriter writer(videoFile.toStdString(), cv::VideoWriter::fourcc('M','J','P','G'), 30, {640,480});
    if (!writer.isOpened()) {
        ui->textEdit->append("❌ Failed to open video writer");
        ui->textEdit->moveCursor(QTextCursor::End);
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

    ui->textEdit->append("🎥 Video saved: " + videoFile);
    ui->textEdit->append("📸 Start taking photos...");
    ui->textEdit->moveCursor(QTextCursor::End);
    QThread::sleep(1);

    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 95};
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 8; ++j) {
            cv::Mat img;
            if (rtspCams[j].read(img)) {
                QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
                cv::putText(img, ts.toStdString(), {20,30}, cv::FONT_HERSHEY_SIMPLEX, 0.8, {255,255,255}, 2);
                QString fname = QString("%1/cam%2/photo_%3.jpg").arg(photoBase).arg(j+1).arg(i);
                bool ok = cv::imwrite(fname.toStdString(), img, params);
                QMetaObject::invokeMethod(ui->textEdit, [=]() {
                    ui->textEdit->append(ok ? QString("✅ cam%1 Saved: %2").arg(j+1).arg(fname)
                                            : QString("❌ cam%1 Failed: %2").arg(j+1).arg(fname));
                    ui->textEdit->moveCursor(QTextCursor::End);
                }, Qt::QueuedConnection);
            }
        }
        QThread::msleep(300);
    }

    ui->textEdit->append("📸 Photo capture complete.");
    ui->textEdit->moveCursor(QTextCursor::End);
    isProcessing = false;
}
