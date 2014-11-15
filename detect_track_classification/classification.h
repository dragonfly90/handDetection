#ifndef CLASSIFICATION
#define CLASSIFICATION
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "math.h"
#include "opencv2/ml/ml.hpp"
#include <queue>
using namespace cv;
using namespace std;
const int BIN_THRESH_OFFSET=5;
const int featuresDimension=400;
//Sensor range in mm - Now arbitrary
const unsigned short SENSOR_MIN = 600;
const unsigned short SENSOR_MAX = 7000;
const unsigned char EMPTY = 0;
const unsigned char HAND = 255;
const int _depthThr = 40;
const int _maxObjectSize = 10000;

bool computeFeatures(const cv::Mat& image,cv::Mat& features,const int& dimension);
uchar ostu(const Mat& image);
int classifylabel(const Mat& features,const CvSVM& svmClassifier);
void processNeighbor(int &pixelcount, double &mean, cv::Mat &mask, const short first, const short second, const cv::Mat &depth,queue<pair<int, int> >& _pixels);
void segmentHand(cv::Mat &mask, Rect &region, const cv::Mat &depth);
#endif