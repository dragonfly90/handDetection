#ifndef DETECT
#define DETECT
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/features2d/features2d.hpp"
using namespace cv;
bool handDetect(const Mat& srcC,const Mat& srcD,Mat& hand,Mat& skinShow,Rect& roi,Scalar& low,Scalar& high);
void handthreshold(const Mat srcC,const Mat srcD,Scalar& low,Scalar& high);
bool searchPoint(unsigned int x,unsigned int y, const Mat& image,Rect& roi,vector<Point>& searchPoints,unsigned short threshold,unsigned short globalT);
#endif