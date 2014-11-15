#ifndef DIF
#define DIF
#include<opencv.hpp>
#include<vector>
const cv::Size featuresSize(30,30);
const float depthScale=512;
const int featureDim=9;
bool computerDIF(const cv::Point& center,const cv::Mat& images, std::vector<float>& features,cv::Size featuresSize,int featuresDim);
float depth(const cv::Mat& images,double x,double y);
#endif