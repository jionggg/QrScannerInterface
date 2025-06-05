// ZEISS Multi‑Cam System – Top Cam 5‑s Video + 1‑s Delay + Side Cam Photos
// ---------------------------------------------------------------------------
// • Cam#1 (Top):   RTSP → QR → 5‑s video  |  D:/zeiss-video/<QR>/<QR>.mp4
// • Cam#2 (Side): RTSP (192.168.1.58)    |  After top‑cam video, wait 1 s →
//                                           snap 10 photos into
//                                           D:/zeiss-photo/<QR>/side_<idx>.jpg
// ---------------------------------------------------------------------------
// 扩展：Cam3~6 逻辑同 Cam2；只要再加 URL 陣列即可。
// ---------------------------------------------------------------------------

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <unordered_set>
#include <filesystem>

using namespace std; namespace fs = std::filesystem; using namespace cv;

// ----------------- 全局常量 -----------------
const int   FRAME_W = 1920;   // 用 1080p 防卡，可调回 3840×2160
const int   FRAME_H = 1080;
const int   TARGET_FPS = 30;
const int   RECORD_SEC = 5;      // 顶摄录像时长
const int   SNAP_CNT = 10;     // Side 连拍数
const int   QR_SCAN_INT = 10;     // 扫描间隔

const string VIDEO_ROOT = "D:/zeiss-video";
const string PHOTO_ROOT = "D:/zeiss-photo";

const string CAM1_URL = "rtsp://192.168.1.10:554/user=admin&password=&channel=1&stream=0.sdp"; // Top
const string CAM2_URL = "rtsp://192.168.1.58:554/user=admin&password=&channel=1&stream=0.sdp"; // Side
// 若要增摄像头，在此再加 URL

// ----------------- 工具函数 -----------------
inline bool ensureDir(const fs::path& p) { std::error_code ec; return fs::create_directories(p, ec) || fs::exists(p); }
inline string safeName(const string& raw) { string s; for (char c : raw) if (isalnum((unsigned char)c) || c == '_' || c == '-') s.push_back(c); return s.empty() ? "unknown" : s; }

// ----------------- Cam 抓帧线程类 -----------------
struct CamThread {
    VideoCapture cap; string url; Mat latest; mutex m;
    atomic<bool> ok{ false };
    CamThread(const string& u) :url(u) {}
    void start() { cap.open(url, CAP_FFMPEG); if (!cap.isOpened()) { cerr << "open fail " << url << endl; return; } cap.set(CAP_PROP_FRAME_WIDTH, FRAME_W); cap.set(CAP_PROP_FRAME_HEIGHT, FRAME_H); cap.set(CAP_PROP_FPS, TARGET_FPS); ok = true; thread([this] {Mat f; while (ok) { if (!cap.read(f)) { this_thread::sleep_for(1ms); continue; } { lock_guard<mutex> lk(m); latest = f; } } cap.release(); }).detach(); }
    bool read(Mat& dst) { lock_guard<mutex> lk(m); if (latest.empty()) return false; dst = latest.clone(); return true; }
};

int main() {
    // 根目录
    ensureDir(VIDEO_ROOT); ensureDir(PHOTO_ROOT);

    // ---------- 启动摄像头 ----------
    CamThread topCam(CAM1_URL);  topCam.start();
    CamThread sideCam(CAM2_URL); sideCam.start();
    QRCodeDetector qr;
    unordered_set<string> doneQR;
    // ---- Prompt control ----
    const int PROMPT_FRAMES = 30;   // ≈1 s at 30 fps
    int promptCnt = 0;

    VideoWriter writer; chrono::steady_clock::time_point recStart;
    bool recording = false; string currentQR;

    namedWindow("Top", WINDOW_NORMAL); resizeWindow("Top", 640, 360);
    namedWindow("Side", WINDOW_NORMAL); resizeWindow("Side", 640, 360);

    while (true) {
        Mat top, side; if (!topCam.read(top)) { if (waitKey(1) == 'q')break; continue; }
        if (promptCnt > 0) { putText(top, "QR Detected", { 30,50 }, FONT_HERSHEY_SIMPLEX, 1.0, { 0,255,0 }, 3); --promptCnt; }
        imshow("Top", top);
        if (sideCam.ok) {
            sideCam.read(side); if (!side.empty()) imshow("Side", side);
        }

        // QR 检测
        static int frameIdx = 0;
        if (!recording && frameIdx++ % QR_SCAN_INT == 0) {
            string qrData = qr.detectAndDecode(top); if (!qrData.empty() && !doneQR.count(qrData)) {
                currentQR = safeName(qrData);
                cout << "QR Detected: " << qrData << endl;
                promptCnt = PROMPT_FRAMES; doneQR.insert(qrData);
                // --- 开始录像 ---
                fs::path outDir = fs::path(VIDEO_ROOT) / currentQR; ensureDir(outDir);
                fs::path mp4 = outDir / (currentQR + ".mp4");
                writer.open(mp4.string(), VideoWriter::fourcc('m', 'p', '4', 'v'), TARGET_FPS, Size(FRAME_W, FRAME_H));
                if (writer.isOpened()) { cout << "▶ 录制 5s 视频 → " << mp4 << endl; recording = true; recStart = chrono::steady_clock::now(); }
            }
        }

        // 录像进行中
        if (recording) {
            writer.write(top);
            double sec = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - recStart).count();
            if (sec >= RECORD_SEC) {
                writer.release(); recording = false; cout << "■ 录制完成\n"; this_thread::sleep_for(1s);
                // --- 连拍 side ---
                if (sideCam.ok) {
                    fs::path dir = fs::path(PHOTO_ROOT) / currentQR; ensureDir(dir);
                    for (int i = 0; i < SNAP_CNT; ++i) { Mat s; if (sideCam.read(s)) { fs::path jpg = dir / ("side_" + to_string(i) + ".jpg"); imwrite(jpg.string(), s); cout << "📸 " << jpg << endl; } this_thread::sleep_for(30ms); }
                }
            }
        }
        if (waitKey(1) == 'q') break;
    }
    topCam.ok = false; sideCam.ok = false; if (writer.isOpened()) writer.release(); destroyAllWindows(); return 0;
}
