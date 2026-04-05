#include "mainwindow.h"
#include "uart_master.h"

// 构造函数（适配树莓派4B，保留ONNX路径/usr/local/last.onnx）
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , isCameraRunning(false)
    , cameraIndex(0)  // 树莓派摄像头默认索引0
    , frameCounter(0)
    , isYoloInit(false)
    , inferThread(nullptr)
{
    // 注册自定义类型
    qRegisterMetaType<std::vector<Detection>>("std::vector<Detection>");

    // 初始化推理线程（保留原ONNX路径：/usr/local/last.onnx）
    std::string onnxPath = "/usr/local/last.onnx";
    inferThread = new YoloInferThread(onnxPath, this);
    isYoloInit = inferThread->isInit();

    // 连接推理完成信号
    connect(inferThread, &YoloInferThread::inferenceFinished, this, &MainWindow::onInferenceFinished);
    inferThread->start();

    // 打印初始化信息
    if (isYoloInit) {
        qDebug() << "YOLOv11n推理线程初始化成功：" << QString::fromStdString(onnxPath)
                 << " 输入尺寸： " << MODEL_INPUT_SIZE << " x " << MODEL_INPUT_SIZE;
    } else {
        QMessageBox::warning(this, "警告", "YOLO模型加载失败！\n请检查/usr/local/last.onnx是否存在，或赋予读取权限");
        qDebug() << "YOLOv11n推理线程初始化失败";
    }

    // 窗口设置
    this->setWindowTitle("树莓派4B - 摄像头姿态检测（UART极速发送）");
    this->setMinimumSize(800, 600);

    // 控件创建 - 原有按钮（样式不变）
    startStopBtn = new QPushButton("启动摄像头", this);
    startStopBtn->setFixedSize(120, 40);
    startStopBtn->setStyleSheet("QPushButton{font-size:14px; background:#2196F3; color:white; border:none; border-radius:4px;} QPushButton:hover{background:#1976D2;}");

    captureBtn = new QPushButton("截图保存", this);
    captureBtn->setFixedSize(120, 40);
    captureBtn->setStyleSheet("QPushButton{font-size:14px; background:#4CAF50; color:white; border:none; border-radius:4px;} QPushButton:hover{background:#388E3C;}");
    captureBtn->setEnabled(false);

    // 方向键按钮创建（样式不变）
    forwardBtn = new QPushButton("↑ 向前 (F)", this);
    forwardBtn->setFixedSize(120, 40);
    forwardBtn->setStyleSheet("QPushButton{font-size:14px; background:#FF9800; color:white; border:none; border-radius:4px;} QPushButton:hover{background:#F57C00;}");

    backwardBtn = new QPushButton("↓ 向后 (B)", this);
    backwardBtn->setFixedSize(120, 40);
    backwardBtn->setStyleSheet("QPushButton{font-size:14px; background:#795548; color:white; border:none; border-radius:4px;} QPushButton:hover{background:#5D4037;}");

    leftBtn = new QPushButton("← 向左 (L)", this);
    leftBtn->setFixedSize(120, 40);
    leftBtn->setStyleSheet("QPushButton{font-size:14px; background:#9C27B0; color:white; border:none; border-radius:4px;} QPushButton:hover{background:#7B1FA2;}");

    rightBtn = new QPushButton("→ 向右 (R)", this);
    rightBtn->setFixedSize(120, 40);
    rightBtn->setStyleSheet("QPushButton{font-size:14px; background:#009688; color:white; border:none; border-radius:4px;} QPushButton:hover{background:#00796B;}");

    stopBtn = new QPushButton("■ 停止 (S)", this);
    stopBtn->setFixedSize(120, 40);
    stopBtn->setStyleSheet("QPushButton{font-size:14px; background:#F44336; color:white; border:none; border-radius:4px;} QPushButton:hover{background:#D32F2F;}");

    // 摄像头显示标签
    cameraLabel = new QLabel(this);
    cameraLabel->setAlignment(Qt::AlignCenter);
    cameraLabel->setStyleSheet("QLabel{border:2px solid #2196F3; background-color:#f5f5f5;}");
    cameraLabel->setText("树莓派摄像头未启动\n点击「启动摄像头」开始监控\n检测结果仅在终端打印");

    // 布局调整（不变）
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(startStopBtn);
    btnLayout->addWidget(captureBtn);
    btnLayout->addStretch();

    QVBoxLayout *dirKeyLayout = new QVBoxLayout();
    dirKeyLayout->setSpacing(5);

    QHBoxLayout *topRow = new QHBoxLayout();
    topRow->addStretch();
    topRow->addWidget(forwardBtn);
    topRow->addStretch();
    dirKeyLayout->addLayout(topRow);

    QHBoxLayout *middleRow = new QHBoxLayout();
    middleRow->addStretch();
    middleRow->addWidget(leftBtn);
    middleRow->addSpacing(10);
    middleRow->addWidget(stopBtn);
    middleRow->addSpacing(10);
    middleRow->addWidget(rightBtn);
    middleRow->addStretch();
    dirKeyLayout->addLayout(middleRow);

    QHBoxLayout *bottomRow = new QHBoxLayout();
    bottomRow->addStretch();
    bottomRow->addWidget(backwardBtn);
    bottomRow->addStretch();
    dirKeyLayout->addLayout(bottomRow);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addLayout(btnLayout);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(dirKeyLayout);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(cameraLabel);
    this->setCentralWidget(centralWidget);

    // 状态栏（适配树莓派）
    QStatusBar *statusBar = new QStatusBar(this);
    this->setStatusBar(statusBar);
    this->statusBar()->showMessage("就绪 - OpenCV版本：" + QString(CV_VERSION) +
                           " | 摄像头索引：0" +
                           " | YOLOv11n：" + (isYoloInit ? QString("已加载（%1x%1）").arg(MODEL_INPUT_SIZE) : "未加载") +
                           " | 树莓派4B UART0 (ttyAMA0)");

    // 定时器
    timer = new QTimer(this);
    timer->setInterval(80);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateCameraFrame);

    // 信号槽绑定
    connect(startStopBtn, &QPushButton::clicked, this, &MainWindow::toggleCamera);
    connect(captureBtn, &QPushButton::clicked, this, &MainWindow::captureScreenshot);
    connect(forwardBtn, &QPushButton::clicked, this, &MainWindow::onForwardBtnClicked);
    connect(backwardBtn, &QPushButton::clicked, this, &MainWindow::onBackwardBtnClicked);
    connect(leftBtn, &QPushButton::clicked, this, &MainWindow::onLeftBtnClicked);
    connect(rightBtn, &QPushButton::clicked, this, &MainWindow::onRightBtnClicked);
    connect(stopBtn, &QPushButton::clicked, this, &MainWindow::onStopBtnClicked);

    // ========== 核心修改：树莓派4B UART初始化 ==========
    uart_fd = uart_init("/dev/serial0"); // 树莓派4B默认串口（映射到ttyAMA0）
    if (uart_fd < 0) {
        fprintf(stderr, "【树莓派UART初始化失败】请检查/dev/serial0权限，或执行sudo raspi-config开启串口\n");
    } else {
        qDebug() << "【树莓派UART初始化成功】已打开/dev/serial0 (ttyAMA0)，波特率115200";
    }
}

// 析构函数（不变）
MainWindow::~MainWindow()
{
    if (cap.isOpened()) {
        cap.release();
    }
    if (inferThread) {
        inferThread->stop();
        delete inferThread;
    }
    if (uart_fd >= 0) {
        uart_close(uart_fd);
        qDebug() << "【树莓派UART已关闭】释放串口资源";
    }
}

// 方向键按钮槽函数（不变，UART发送逻辑已适配）
void MainWindow::onForwardBtnClicked() {
    if (uart_fd >= 0) {
        uart_send_char(uart_fd, 'F');
        qDebug("【手动控制】向前 → F");
        statusBar()->showMessage("手动控制：向前 (F) | 树莓派UART0发送成功");
    } else {
        qDebug("【手动控制失败】无法发送 F");
        QMessageBox::warning(this, "警告", "UART未初始化，无法发送向前指令！");
    }
}

void MainWindow::onBackwardBtnClicked() {
    if (uart_fd >= 0) {
        uart_send_char(uart_fd, 'B');
        qDebug("【手动控制】向后 → B");
        statusBar()->showMessage("手动控制：向后 (B) | 树莓派UART0发送成功");
    } else {
        qDebug("【手动控制失败】无法发送 B");
        QMessageBox::warning(this, "警告", "UART未初始化，无法发送向后指令！");
    }
}

void MainWindow::onLeftBtnClicked() {
    if (uart_fd >= 0) {
        uart_send_char(uart_fd, 'L');
        qDebug("【手动控制】向左 → L");
        statusBar()->showMessage("手动控制：向左 (L) | 树莓派UART0发送成功");
    } else {
        qDebug("【手动控制失败】无法发送 L");
        QMessageBox::warning(this, "警告", "UART未初始化，无法发送向左指令！");
    }
}

void MainWindow::onRightBtnClicked() {
    if (uart_fd >= 0) {
        uart_send_char(uart_fd, 'R');
        qDebug("【手动控制】向右 → R");
        statusBar()->showMessage("手动控制：向右 (R) | 树莓派UART0发送成功");
    } else {
        qDebug("【手动控制失败】无法发送 R");
        QMessageBox::warning(this, "警告", "UART未初始化，无法发送向右指令！");
    }
}

void MainWindow::onStopBtnClicked() {
    if (uart_fd >= 0) {
        uart_send_char(uart_fd, 'S');
        qDebug("【手动控制】停止 → S");
        statusBar()->showMessage("手动控制：停止 (S) | 树莓派UART0发送成功");
    } else {
        qDebug("【手动控制失败】无法发送 S");
        QMessageBox::warning(this, "警告", "UART未初始化，无法发送停止指令！");
    }
}

// 推理完成槽函数（保留ONNX路径提示）
void MainWindow::onInferenceFinished(const std::vector<Detection>& detections)
{
    qDebug() << "\n==================== YOLOv11n 检测结果 ====================";

    Detection bestDetection;
    float maxConfidence = 0.0f;
    for (const Detection& det : detections) {
        if (det.confidence > maxConfidence) {
            maxConfidence = det.confidence;
            bestDetection = det;
        }
    }

    if (maxConfidence > 0.0f) {
        qDebug() << "检测目标  1 :";
        qDebug() << "  头部姿态：" << QString::fromStdString(bestDetection.className);
        qDebug() << "  置信度：" << QString::number(bestDetection.confidence, 'f', 4);
        qDebug() << "  检测框坐标：x=" << bestDetection.box.x << " y=" << bestDetection.box.y
                 << " 宽度=" << bestDetection.box.width << " 高度=" << bestDetection.box.height;

        QString temp_pose = QString::fromStdString(bestDetection.className);

        if (uart_fd >= 0) {
            if (temp_pose == "up") {
                uart_send_char(uart_fd, 'S');
            } else if (temp_pose == "down") {
                uart_send_char(uart_fd, 'B');
            } else if (temp_pose == "left") {
                uart_send_char(uart_fd, 'L');
            } else if (temp_pose == "right") {
                uart_send_char(uart_fd, 'R');
            } else if (temp_pose == "front") {
                uart_send_char(uart_fd, 'F');
            } else {
                uart_send_char(uart_fd, 'S');
            }
            qDebug() << "【树莓派UART发送成功】姿态：" << temp_pose << " → 字符：" << (temp_pose == "up" ? 'S' : (temp_pose == "down" ? 'B' : (temp_pose == "left" ? 'L' : (temp_pose == "right" ? 'R' : (temp_pose == "front" ? 'F' : 'S')))));
        } else {
            qDebug() << "【树莓派UART发送失败】串口未初始化";
        }

        this->statusBar()->showMessage("YOLOv11n检测完成 | 头部姿态：" + QString::fromStdString(bestDetection.className) +
                               " | 置信度：" + QString::number(bestDetection.confidence, 'f', 2) +
                               " | 输入尺寸：" + QString("%1x%1").arg(MODEL_INPUT_SIZE) + " | 树莓派4B推理完成");
    } else {
        qDebug() << "  未检测到头部姿态";
        this->statusBar()->showMessage("YOLOv11n检测完成 | 未检测到头部姿态 | 输入尺寸：" + QString("%1x%1").arg(MODEL_INPUT_SIZE) + " | 树莓派4B推理完成");
    }
    qDebug() << "===========================================================\n";
}

// 摄像头启停（适配树莓派）
void MainWindow::toggleCamera()
{
    if (!isCameraRunning) {
        cap.release();
        bool openSuccess = false;
        int tryIndex = 0; // 树莓派摄像头默认索引0

        // 树莓派摄像头打开方式（兼容USB摄像头和板载摄像头）
        cap.open(tryIndex, cv::CAP_V4L2);
        if (cap.isOpened()) {
            cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
            openSuccess = true;
        }

        // 备用索引（仅USB摄像头可能用到）
        if (!openSuccess) {
            tryIndex = 1;
            cap.open(tryIndex);
            if (cap.isOpened()) {
                cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
                openSuccess = true;
            }
        }

        if (!openSuccess) {
            QMessageBox::critical(this, "错误", "无法打开树莓派摄像头！\n解决方案：\n1. 执行 sudo ./OpenCV_CameraMonitor 运行\n2. 执行 sudo raspi-config 开启摄像头\n3. 检查USB摄像头是否插好");
            this->statusBar()->showMessage("错误：摄像头打开失败 | 尝试索引：0,1");
            return;
        }

        // 摄像头参数（适配树莓派）
        cap.set(cv::CAP_PROP_FRAME_WIDTH, MODEL_INPUT_SIZE);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, MODEL_INPUT_SIZE * 3 / 4);
        cap.set(cv::CAP_PROP_FPS, 10);
        cap.set(cv::CAP_PROP_BUFFERSIZE, 1);
        cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 0);
        cap.set(cv::CAP_PROP_AUTOFOCUS, 0);

        frameCounter = 0;
        timer->start();
        isCameraRunning = true;
        startStopBtn->setText("停止摄像头");
        captureBtn->setEnabled(true);
        cameraLabel->setText("");

        int width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
        int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
        this->statusBar()->showMessage("树莓派摄像头已启动 | 索引：" + QString::number(tryIndex) +
                               " | 分辨率：" + QString::number(width) + "x" + QString::number(height) +
                               " | 格式：MJPG | YOLOv11n：每20帧异步检测一次");
    } else {
        timer->stop();
        cap.release();
        isCameraRunning = false;
        startStopBtn->setText("启动摄像头");
        captureBtn->setEnabled(false);
        cameraLabel->setText("树莓派摄像头已停止\n点击「启动摄像头」重新开始");
        this->statusBar()->showMessage("摄像头已停止 | OpenCV版本：" + QString(CV_VERSION) +
                               " | YOLOv11n：" + (isYoloInit ? QString("已加载（%1x%1）").arg(MODEL_INPUT_SIZE) : "未加载") + " | 树莓派4B");
    }
}

// 更新摄像头画面（不变）
void MainWindow::updateCameraFrame()
{
    if (!cap.isOpened()) return;

    cv::Mat frame;
    bool readSuccess = false;
    for (int i = 0; i < 3; i++) {
        if (cap.read(frame)) {
            readSuccess = true;
            break;
        }
        cv::waitKey(1);
    }

    if (!readSuccess) {
        this->statusBar()->showMessage("警告：树莓派摄像头帧读取失败，正在重试...");
        return;
    }

    frameCounter++;
    if (isYoloInit && frameCounter % 20 == 0) {
        inferThread->setFrame(frame);
        this->statusBar()->showMessage("YOLOv11n异步推理中 | 当前帧：" + QString::number(frameCounter) +
                               " | 输入尺寸：" + QString("%1x%1").arg(MODEL_INPUT_SIZE) + " | 树莓派4B");
    } else {
        this->statusBar()->showMessage("树莓派摄像头运行中 | 分辨率：" + QString::number(frame.cols) + "x" + QString::number(frame.rows) +
                               " | 帧率：10FPS | 检测频率：每20帧一次（当前帧：" + QString::number(frameCounter) +
                               "） | 输入尺寸：" + QString("%1x%1").arg(MODEL_INPUT_SIZE));
    }

    cv::Mat rgbFrame;
    cv::cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);
    QImage qImage(rgbFrame.data, rgbFrame.cols, rgbFrame.rows, rgbFrame.step, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(qImage).scaled(
        cameraLabel->size(), Qt::KeepAspectRatio
    );
    cameraLabel->setPixmap(pixmap);
}

// 截图保存（适配树莓派，若要保存到/root需sudo运行，这里保留原逻辑但提示权限）
void MainWindow::captureScreenshot()
{
    if (!cap.isOpened()) return;

    cv::Mat frame;
    cap.read(frame);
    if (frame.empty()) return;

    // 保留原保存路径/root（需sudo运行才能写入）
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString savePath = QString("/root/q8_yolov11n_capture_%1_%2x%2.jpg").arg(timestamp).arg(MODEL_INPUT_SIZE);
    cv::imwrite(savePath.toStdString(), frame);

    QMessageBox::information(this, "截图成功", "树莓派摄像头截图已保存：\n" + savePath + "\n（需sudo运行才能访问/root目录）");
    this->statusBar()->showMessage("截图已保存：" + savePath +
                           " | 输入尺寸：" + QString("%1x%1").arg(MODEL_INPUT_SIZE) + " | 树莓派4B");
}
