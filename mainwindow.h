#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QDateTime>
#include <QDir>
#include <QString>
#include <QSet>
#include <QFuture>
#include <QLabel>
#include <QTextCursor>
#include <QElapsedTimer>
#include <atomic>
#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include <QtConcurrent>

// 相机线程类
class CamThread {
public:
    cv::VideoCapture cap;
    cv::Mat latest;
    std::mutex m;
    std::atomic<bool> ok{false};

    CamThread() = default;

    bool start(const std::string& url, bool isUsb = false) {
        if (isUsb) cap.open(0, cv::CAP_DSHOW);
        else cap.open(url, cv::CAP_FFMPEG);

        if (!cap.isOpened()) return false;
        ok = true;

        std::thread([this] {
            cv::Mat f;
            while (ok) {
                if (!cap.read(f)) continue;
                {
                    std::lock_guard<std::mutex> lock(m);
                    latest = f.clone();
                }
                QThread::msleep(5);
            }
        }).detach();
        return true;
    }

    bool read(cv::Mat& dst) {
        std::lock_guard<std::mutex> lock(m);
        if (latest.empty()) return false;
        dst = latest.clone();
        return true;
    }

    void stop() {
        ok = false;
        if (cap.isOpened()) cap.release();
    }
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_launchButton_clicked();
    void on_exitButton_clicked();
    void updateFrame();

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    QTimer *timeoutCheckTimer;         // ✅ 新增：120秒超时检测
    QElapsedTimer lastQRTime;         // ✅ 新增：记录上次 QR 检测时间

    CamThread topCam;
    CamThread rtspCams[8];
    cv::QRCodeDetector qrDecoder;
    QSet<QString> processedQRCodes;

    QString pendingQR;
    std::atomic<bool> isProcessing = false;
    QFuture<void> processingThread;

    void processQR(const QString& qrCode);
};

#endif // MAINWINDOW_H
