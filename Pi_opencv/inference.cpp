#include "inference.h"
#include <chrono>   // 仅新增：计时（和你原始输出格式一致）
#include <iostream>
#include <cmath>    // 仅新增：sigmoid函数（修复置信度）

// 仅新增：sigmoid函数（YOLO logits转0~1概率，核心修复）
static float sigmoid(float x) {
    return 1.0f / (1.0f + expf(-x));
}

Inference::Inference(const std::string &onnxModelPath, const cv::Size &modelInputShape,
                     const std::string &classesTxtFile, const bool &runWithCuda)
{
    modelPath = onnxModelPath;
    modelShape = modelInputShape;
    classesPath = classesTxtFile;
    cudaEnabled = runWithCuda;
    loadOnnxNetwork();
}

Inference::~Inference()
{
    release();
}

void Inference::release()
{
    // 保留你原始的释放逻辑
    if (!net.empty()) {
        net = cv::dnn::Net();
    }
}

std::string Inference::getClassName(int classId)
{
    // 保留你原始的类别获取逻辑
    if (classId >= 0 && static_cast<size_t>(classId) < classes.size()) {
        return classes[classId];
    }
    return "unknown";
}

std::vector<Detection> Inference::runInference(const cv::Mat &input)
{
    // 仅新增：计时开始（和你原始输出格式一致）
    auto start = std::chrono::steady_clock::now();

    if (input.empty() || net.empty()) {
        return {};
    }

    cv::Mat modelInput = input;
    // 保留你原始的letterBox逻辑
    if (letterBoxForSquare && modelShape.width == modelShape.height)
        modelInput = formatToSquare(modelInput);

    // 保留你原始的blob生成逻辑
    cv::Mat blob;
    cv::dnn::blobFromImage(modelInput, blob, 1.0/255.0, modelShape, cv::Scalar(), true, false);
    net.setInput(blob);

    std::vector<cv::Mat> outputs;
    net.forward(outputs, net.getUnconnectedOutLayersNames());

    int rows = outputs[0].size[1];
    int dimensions = outputs[0].size[2];
    bool yolov8 = false;

    // 保留你原始的YOLOv8兼容逻辑
    if (dimensions > rows)
    {
        yolov8 = true;
        rows = outputs[0].size[2];
        dimensions = outputs[0].size[1];
        outputs[0] = outputs[0].reshape(1, dimensions);
        cv::transpose(outputs[0], outputs[0]);
    }

    float *data = (float *)outputs[0].data;
    // 保留你原始的缩放因子计算
    float x_factor = modelInput.cols / modelShape.width;
    float y_factor = modelInput.rows / modelShape.height;

    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    for (int i = 0; i < rows; ++i)
    {
        if (yolov8)
        {
            // 修复YOLOv8分支：类别得分做sigmoid
            float *classes_scores = data+4;
            cv::Mat scores(1, classes.size(), CV_32FC1, classes_scores);
            // 对每个类别得分做sigmoid（核心修复）
            for (int c = 0; c < classes.size(); c++) {
                scores.at<float>(0, c) = sigmoid(scores.at<float>(0, c));
            }
            cv::Point class_id;
            double maxClassScore;
            minMaxLoc(scores, 0, &maxClassScore, 0, &class_id);

            if (maxClassScore > modelScoreThreshold)
            {
                // 置信度直接用类别得分（YOLOv8无单独obj_conf）
                confidences.push_back(static_cast<float>(maxClassScore));
                class_ids.push_back(class_id.x);

                float x = data[0];
                float y = data[1];
                float w = data[2];
                float h = data[3];

                int left = int((x - 0.5 * w) * x_factor);
                int top = int((y - 0.5 * h) * y_factor);
                int width = int(w * x_factor);
                int height = int(h * y_factor);

                boxes.push_back(cv::Rect(left, top, width, height));
            }
        }
        else
        {
            // 修复YOLOv5/YOLOv11分支：obj_conf和类别得分都做sigmoid
            float confidence = sigmoid(data[4]); // obj_conf做sigmoid（核心修复）
            if (confidence >= modelConfidenceThreshold)
            {
                float *classes_scores = data+5;
                cv::Mat scores(1, classes.size(), CV_32FC1, classes_scores);
                // 对每个类别得分做sigmoid（核心修复）
                for (int c = 0; c < classes.size(); c++) {
                    scores.at<float>(0, c) = sigmoid(scores.at<float>(0, c));
                }
                cv::Point class_id;
                double max_class_score;
                minMaxLoc(scores, 0, &max_class_score, 0, &class_id);

                if (max_class_score > modelScoreThreshold)
                {
                    // 最终置信度=obj_conf×类别得分（0~1，核心修复）
                    confidences.push_back(confidence * static_cast<float>(max_class_score));
                    class_ids.push_back(class_id.x);

                    float x = data[0];
                    float y = data[1];
                    float w = data[2];
                    float h = data[3];

                    int left = int((x - 0.5 * w) * x_factor);
                    int top = int((y - 0.5 * h) * y_factor);
                    int width = int(w * x_factor);
                    int height = int(h * y_factor);

                    // 仅新增：边界检查（过滤0/352等异常框）
                    left = std::max(0, std::min(left, modelInput.cols - 1));
                    top = std::max(0, std::min(top, modelInput.rows - 1));
                    width = std::max(5, std::min(width, modelInput.cols - left));
                    height = std::max(5, std::min(height, modelInput.rows - top));

                    boxes.push_back(cv::Rect(left, top, width, height));
                }
            }
        }
        data += dimensions;
    }

    // 保留你原始的NMS逻辑
    std::vector<int> nms_result;
    cv::dnn::NMSBoxes(boxes, confidences, modelScoreThreshold, modelNMSThreshold, nms_result);

    std::vector<Detection> detections{};
    for (unsigned long i = 0; i < nms_result.size(); ++i)
    {
        int idx = nms_result[i];
        Detection result;
        result.class_id = class_ids[idx];
        result.confidence = confidences[idx]; // 0~1的归一化置信度

        // 保留你原始的随机颜色逻辑
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(100, 255);
        result.color = cv::Scalar(dis(gen), dis(gen), dis(gen));

        result.className = classes[result.class_id];
        result.box = boxes[idx];
        detections.push_back(result);
    }

    // 仅新增：计时结束+打印（和你原始输出格式一致）
    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "[YOLO] 推理耗时: " << elapsed << " ms (输入尺寸 " << MODEL_INPUT_SIZE << "x" << MODEL_INPUT_SIZE << ")" << std::endl;

    outputs.clear();
    return detections;
}

void Inference::loadClassesFromFile()
{
    // 保留你原始的加载类别逻辑
    std::ifstream inputFile(classesPath);
    if (inputFile.is_open())
    {
        std::string classLine;
        while (std::getline(inputFile, classLine))
            classes.push_back(classLine);
        inputFile.close();
    }
}

void Inference::loadOnnxNetwork()
{
    net = cv::dnn::readNetFromONNX(modelPath);
    // 保留你原始的设备选择逻辑（强制CPU，适配i.MX6ULL）
    std::cout << "\nYOLOv11n 推理模式：CPU (i.MX6ULL适配版) 输入尺寸: " << MODEL_INPUT_SIZE << "x" << MODEL_INPUT_SIZE << std::endl;
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    cv::setNumThreads(1); // 仅新增：单线程，适配嵌入式
}

cv::Mat Inference::formatToSquare(const cv::Mat &source)
{
    // 保留你原始的letterBox逻辑
    int col = source.cols;
    int row = source.rows;
    int _max = MAX(col, row);
    cv::Mat result = cv::Mat::zeros(_max, _max, CV_8UC3);
    source.copyTo(result(cv::Rect(0, 0, col, row)));
    return result;
}
