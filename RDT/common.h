//read image
//dongliang 20130424
#include "opencv2/core/core.hpp"
#include <iostream>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"


using namespace std;
using namespace cv;


void saveData(const char* filename, const Mat& mat, int flag) ;
bool hasEnding (std::string const &fullString, std::string const &ending);
bool hasEndingLower (string const &fullString_, string const &_ending);
void open_imgs_dir(char* dir_name, std::vector<cv::Mat>& images, std::vector<std::string>& images_names, double downscale_factor) ;
bool saveSamples(string filename,Mat& trainsamples, Mat& trainlabels);

//region grow
bool search(unsigned int x,unsigned int y, Mat& image,Rect& roi,vector<Point>& searchPoints,unsigned short threshold,unsigned short globalT);