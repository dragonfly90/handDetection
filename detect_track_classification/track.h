#ifndef TRACK
#define TRACK
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/video/tracking.hpp>
#include <queue>
#include <iostream>
#include <ctype.h>
using namespace std;
using namespace cv;


// Class for grabbing information from kinect and related image objects
class trackerTest {
public:
	trackerTest(Mat image,Mat depthImage):img(image),depthImg(depthImage){};
	Mat img, depthImg, show;
	VideoCapture cap;
	Rect target;
	Mat mask;
	void update(const Mat& image,const Mat& depthImage);
};

#endif