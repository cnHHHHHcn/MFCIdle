#include "opencv2/opencv.hpp"


// 基本模板匹配函数，返回最佳匹配位置
cv::Point templateMatch(std::string originalImageName, std::string templateImageName, double& confidence) {
    cv::Mat originalImage = cv::imread(originalImageName);
    cv::Mat templateImage = cv::imread(templateImageName);
    if (templateImage.empty() || templateImage.empty()) return cv::Point{ 0, 0 };
    cv::Mat result;

    // 进行模板匹配
    matchTemplate(originalImage, templateImage, result, cv::TM_CCOEFF_NORMED);
    // 找到最佳匹配位置
    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);
    confidence = maxVal; // 返回匹配度
    // 返回最佳匹配的左上角坐标
    return maxLoc;
}

int main() {
    double rate = 0;
    cv::Point pm = templateMatch("screen.png", "template.png", rate);
    std::cout << rate << std::endl;
    std::cout << pm.x << "  " << pm.y;
    return 0;
}