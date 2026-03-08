#ifndef INFERENCE_H
#define INFERENCE_H

#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

// 仅修改：适配你的160x160输入尺寸
//#define MODEL_INPUT_SIZE 160
#define MODEL_INPUT_SIZE 128

struct Detection
{
    int class_id{0};
    std::string className{};
    float confidence{0.0};
    cv::Scalar color{};
    cv::Rect box{};
};

class Inference
{
public:
    // 仅修改：默认尺寸改为MODEL_INPUT_SIZE x MODEL_INPUT_SIZE
    Inference(const std::string &onnxModelPath, const cv::Size &modelInputShape = {MODEL_INPUT_SIZE, MODEL_INPUT_SIZE},
              const std::string &classesTxtFile = "", const bool &runWithCuda = true);
    ~Inference();
    std::vector<Detection> runInference(const cv::Mat &input);
    std::string getClassName(int classId);
    void release();

private:
    void loadClassesFromFile();
    void loadOnnxNetwork();
    cv::Mat formatToSquare(const cv::Mat &source);

    std::string modelPath{};
    std::string classesPath{};
    bool cudaEnabled{};

    // 保留你原始的类别顺序
    std::vector<std::string> classes{"front", "left", "up","right" , "down"};
    cv::Size2f modelShape{};

    // 保留你原始的阈值
    float modelConfidenceThreshold {0.25};
    float modelScoreThreshold      {0.45};
    float modelNMSThreshold        {0.50};

    // 保留你原始的letterBox设置
    bool letterBoxForSquare = true;
    cv::dnn::Net net;
};

#endif // INFERENCE_H
