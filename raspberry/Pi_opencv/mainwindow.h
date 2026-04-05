#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include <QImage>
#include <QMessageBox>
#include <QDateTime>
#include <QFile>
#include <QThread>
#include <QMutex>
#include <QMetaType>
#include <QDebug>
#include "inference.h"  // 引入宏定义
#include "uart_master.h" // 新增：引入UART头文件

Q_DECLARE_METATYPE(std::vector<Detection>);

// 推理线程类（完全不变）
class YoloInferThread : public QThread
{
    Q_OBJECT
public:
    YoloInferThread(const std::string& onnxPath, QObject *parent = nullptr)
        : QThread(parent), onnxModelPath(onnxPath), running(false), newFrameAvailable(false), isInitSuccess(false) {
        // 使用宏定义初始化尺寸
        try {
            yoloInfer = new Inference(onnxModelPath, cv::Size(MODEL_INPUT_SIZE, MODEL_INPUT_SIZE), "", false);
            isInitSuccess = true;
        } catch (...) {
            isInitSuccess = false;
        }
    }

    ~YoloInferThread() {
        stop();
        if (yoloInfer) {
            yoloInfer->release();
            delete yoloInfer;
        }
    }

    void setFrame(const cv::Mat& frame) {
        QMutexLocker locker(&mutex);
        inputFrame = frame.clone();
        newFrameAvailable = true;
    }

    void stop() {
        running = false;
        wait();
    }

    bool isInit() const { return isInitSuccess; }

signals:
    void inferenceFinished(const std::vector<Detection>& detections);

protected:
    void run() override {
        running = true;
        while (running) {
            if (newFrameAvailable) {
                QMutexLocker locker(&mutex);
                std::vector<Detection> dets;
                if (yoloInfer) {
                    dets = yoloInfer->runInference(inputFrame);
                }
                newFrameAvailable = false;
                emit inferenceFinished(dets);
            }
            msleep(20);
        }
    }

private:
    std::string onnxModelPath;
    bool running;
    QMutex mutex;
    cv::Mat inputFrame;
    bool newFrameAvailable;
    bool isInitSuccess;
    Inference *yoloInfer;
};

// 主窗口类（新增方向按钮+停止按钮成员变量）
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void toggleCamera();
    void updateCameraFrame();
    void captureScreenshot();
    void onInferenceFinished(const std::vector<Detection>& detections);
    // 新增：方向按钮+停止按钮槽函数
    void onForwardBtnClicked();   // 向前 → F
    void onBackwardBtnClicked();  // 向后 → B
    void onLeftBtnClicked();      // 向左 → L
    void onRightBtnClicked();     // 向右 → R
    void onStopBtnClicked();      // 停止 → S

private:
    QLabel *cameraLabel;
    QPushButton *startStopBtn;
    QPushButton *captureBtn;

    // 方向键布局按钮
    QPushButton *forwardBtn;   // 向前（上）
    QPushButton *backwardBtn;  // 向后（下）
    QPushButton *leftBtn;      // 向左（左）
    QPushButton *rightBtn;     // 向右（右）
    QPushButton *stopBtn;      // 停止（中）

    cv::VideoCapture cap;
    QTimer *timer;
    bool isCameraRunning;
    int cameraIndex;
    int frameCounter;  // 先声明
    bool isYoloInit;   // 后声明
    YoloInferThread *inferThread;

    //UART文件描述符
    int uart_fd;
};

#endif // MAINWINDOW_H
