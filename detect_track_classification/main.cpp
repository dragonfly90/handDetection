#include"detect.h"
#include"track.h"
#include"classification.h"
#include"time.h"
//最多对一个对象进行跟踪;
#define MAX_OBJECTS 1
char imagesname[255];
Mat imageC,imageD,imageDX,imageDC,imageSave;
Mat srcC,srcD;
Mat skinShow,hand;
Mat temp;
Rect roi;
Scalar low,high;
MSER featurePoint;
SimpleBlobDetector featureBlob;
vector<KeyPoint> points;
FileStorage fs;
CvSVM svmClassifier;
Mat naiveFeatures;
const float scaleFactor=255.0/4096;
Rect box;
bool selected;
bool drawing_box;
clock_t start,tend;
Vec3b green(0,255,0), blue (255,0,0),red(0,0,255);
VideoWriter videoWriter("F://test//detectTC.avi",-1,10,cvSize(1280,480),true);


void selectTarget(trackerTest &tracker,Rect& box,Mat temp);
//void mouse_callback(int event,int x,int y,int flag,void* param);
void Tracker(trackerTest &tracker, int TRACKER_OPTION,Mat& image,Mat& depthImage) ;
void calchistforChannel(Mat &img, Mat &b_hist,Mat &r_hist, Mat &g_hist, int histSize, const float *ranges, Mat &mask);
void drawHist(Mat &img, Mat &hist, int histSize,  Scalar color);
void morphologyTosegment(Mat &);
//void segmentHand(cv::Mat &mask, Rect &region, const cv::Mat &depth);
//void processNeighbor(int &pixelcount, double &mean, cv::Mat &mask, const short first, const short second, const cv::Mat &depth);

void selectTarget(trackerTest &tracker,Rect& box,Mat temp) {
	tracker.img.copyTo(temp);
	rectangle(temp, box, Scalar(0, 255, 0), 1);
	tracker.target = box;
	tracker.mask = Mat(tracker.depthImg.rows, tracker.depthImg.cols, CV_8UC1);
	segmentHand(tracker.mask, tracker.target, tracker.depthImg);
}


void trackerTest::update(const Mat& image,const Mat& depthImage) {
	image.copyTo(img);
	depthImage.copyTo(depthImg);
}

void Tracker(trackerTest &tracker, int TRACKER_OPTION,Mat& image,Mat& depthImage) {
	Mat roi(tracker.img, tracker.target);
	Mat droid(tracker.depthImg.size(), CV_8UC1);
	Mat maskroi(tracker.mask, tracker.target);
	double maxk,mink;
	minMaxLoc(tracker.depthImg, &mink, &maxk);
	tracker.depthImg.convertTo(droid, CV_8U, (255.0/7000.0), 0.0);
	Mat droi(droid, tracker.target);
	Mat b_hist, r_hist, g_hist, d_hist;
	int histSize = 256;
	int dhistSize = 256;
	float ranges[] = {0, 256};
	float drange[] = {0, 256};
	const float* dhistRange = {drange};
	const float* histRange = { ranges };
	cvtColor(roi, roi, CV_BGR2HSV);
	calchistforChannel(roi, b_hist, g_hist, r_hist, histSize, (const float*)ranges, maskroi);
	calcHist( &droi, 1, 0, maskroi, d_hist, 1, &dhistSize, &dhistRange);
	normalize(d_hist, d_hist, 0, 400, NORM_MINMAX, -1, Mat() );

	drawHist(tracker.img, d_hist, dhistSize, Scalar(0,255,0));
	MatND backPro, backPro_r, backPro_g, backPro_b, backPro_d;
	vector<Mat> bgr_planes;
	//for(;;) {
	tracker.update(image,depthImage);
	if(tracker.img.data==NULL)return;
	cvtColor(tracker.img, tracker.img, CV_BGR2HSV);
	split(tracker.img, bgr_planes);
	tracker.depthImg.convertTo(droid, CV_8U, (255.0/7000.0), 0.0);
	Mat roif(droid, tracker.target);
	segmentHand(tracker.mask, tracker.target, tracker.depthImg);
	maskroi = Mat(tracker.mask, tracker.target);
	//maskroi = Mat();
	calcHist( &roif, 1, 0, maskroi, d_hist, 1, &dhistSize, &dhistRange);
	normalize(d_hist, d_hist, 0, 400, NORM_MINMAX, -1, Mat() );
	calcBackProject(&bgr_planes[0], 1, 0, b_hist, backPro_b, &histRange, 1, true);
	calcBackProject(&bgr_planes[1], 1, 0, g_hist, backPro_g, &histRange, 1, true);
	calcBackProject(&bgr_planes[2], 1, 0, r_hist, backPro_r, &histRange, 1, true);
	calcBackProject(&droid, 1, 0, d_hist, backPro_d, &histRange, 1, true);
	multiply(backPro_b,backPro_g, backPro, 1./255);
	multiply(backPro, backPro_d, backPro, 1./255, CV_8UC1);
	//morphologyTosegment(backPro);
	//imshow("backProjected_d", backPro_d);
	//backPro = (backPro)*(255);
	//multiply(backPro, backPro_r, backPro, 1./255);
	//backPro = (backPro)*(1./255);
	cvtColor(tracker.img, tracker.img, CV_HSV2BGR);
	if(TRACKER_OPTION == 0) {
		meanShift(backPro, tracker.target, TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 20, 0.1 ));
		if( tracker.target.area() <= 1 ) {
			int cols = roi.cols, rows = roi.rows, r = (MIN(cols, rows) + 5)/6;
			tracker.target = Rect(tracker.target.x - r, tracker.target.y - r,tracker.target.x + r, tracker.target.y + r) & Rect(0, 0, cols, rows);
		}
		rectangle(tracker.img, tracker.target, Scalar(0,255,0), 1);
	}
	else {
		RotatedRect trackBox = CamShift(backPro, tracker.target, TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));
		ellipse(tracker.img, trackBox, Scalar(0,255,0), 1);
		if( tracker.target.area() <= 1 ) {
			int cols = roi.cols, rows = roi.rows, r = (MIN(cols, rows) + 5)/6;
			tracker.target = Rect(tracker.target.x - r, tracker.target.y - r,tracker.target.x + r, tracker.target.y + r) & Rect(0, 0, cols, rows);
		}
	}
	//tracker.target = trackBox.boundingRect();
	namedWindow("backProjected",CV_WINDOW_AUTOSIZE);
	namedWindow("tracked",CV_WINDOW_AUTOSIZE);
	imshow("backProjected", backPro);
	imshow("tracked",tracker.img);
	//if(waitKey(20) == 32)
	//	break;


	//imshow("backProjected", backPro);
	//imshow("roi", roi);
}

//calculate

void calchistforChannel(Mat &img, Mat &b_hist,Mat &g_hist, Mat &r_hist, int histSize, const float *ranges, Mat &mask) {
	vector<Mat> bgr_planes;
	split(img, bgr_planes);

	//bool uniform = true, accumulate1 = false;
	calcHist( &bgr_planes[0], 1, 0, mask, b_hist, 1, &histSize, &ranges);//, uniform, accumulate1 );
	calcHist( &bgr_planes[1], 1, 0, mask, g_hist, 1, &histSize, &ranges);//, uniform, accumulate1 );
	calcHist( &bgr_planes[2], 1, 0, mask, r_hist, 1, &histSize, &ranges);//, uniform, accumulate1 );

	int hist_w = 512; int hist_h = 400;
	int bin_w = cvRound( (double) hist_w/histSize );

	Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );

	/// Normalize the result to [ 0, histImage.rows ]
	normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
	normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
	normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

	/// Draw for each channel
	for( int i = 1; i < histSize; i++ )
	{
		line( histImage, Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1)) ) ,
			Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
			Scalar( 255, 0, 0), 2, 8, 0  );
		line( histImage, Point( bin_w*(i-1), hist_h - cvRound(g_hist.at<float>(i-1)) ) ,
			Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
			Scalar( 0, 255, 0), 2, 8, 0  );
		line( histImage, Point( bin_w*(i-1), hist_h - cvRound(r_hist.at<float>(i-1)) ) ,
			Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
			Scalar( 0, 0, 255), 2, 8, 0  );
	}

	/// Display
	namedWindow("calcHist Demo", CV_WINDOW_AUTOSIZE );
	imshow("calcHist Demo", histImage );
	waitKey(10);
}

void drawHist(Mat &img, Mat &hist, int histSize,  Scalar color) {
	int binW = img.cols/histSize;
	for(int i=0; i<histSize; i++) {
		int val = saturate_cast<int>(hist.at<float>(i)*img.rows/255);
		rectangle(img, Point(i*binW,img.rows),Point((i+1)*binW,img.rows - val), color, 1, 8);
	}
	namedWindow("hist",CV_WINDOW_AUTOSIZE);
	imshow("hist", img);
	//waitKey(0);
}

// 3D region segmentation process

void morphologyTosegment(Mat &img) {
	//erode(img, img, Mat());
	dilate(img, img, Mat());
}


//传递参数的class
class params
{
public:

	Point loc1,loc2;
	string win_name;
	Mat src;
	Mat cur;
	int n;					//记录对象个数
	params() : n(0) {}
	// 	~params();
};

//鼠标事件
void mouseEvent(int event, int x, int y, int flags, void* param)  
{
	static bool check_line_state = FALSE;
	Mat tmp;
	params* p = (params*)param;
	//按下鼠标左键，存储初始位置;
	if( event == CV_EVENT_LBUTTONDOWN )
	{
		if (p->n == MAX_OBJECTS)//判断是否已经有了跟踪对象;
		{
			cout<<"Fail! \n Only Can Tracking One Object!"<<endl;
			return ;
		}
		p->loc1.x = x;
		p->loc1.y = y;
		check_line_state = TRUE;
	}

	//按下鼠标左键且鼠标移动，画矩形框;
	else if (check_line_state && event == CV_EVENT_MOUSEMOVE)
	{
		if (p->n == MAX_OBJECTS)//判断是否已经有了跟踪对象;
		{
			cout<<"Fail! \n Only Can Tracking One Object!"<<endl;
			return ;
		}
		p->src.copyTo(tmp);
		rectangle(tmp, p->loc1, Point(x,y), CV_RGB(255,0,0), 2, 8 );
		imshow(p->win_name,tmp);
	}

	//左键弹起，画框结束;
	else if(event == CV_EVENT_LBUTTONUP)
	{
		if (p->n == MAX_OBJECTS)//判断是否已经有了跟踪对象;
		{
			cout<<"Fail! \n Only Can Tracking One Object!"<<endl;
			return ;
		}
		p->loc2.x = x;
		p->loc2.y = y;
		rectangle(p->src, p->loc1, Point(x,y), CV_RGB(255,0,0), 2, 8 );
		imshow(p->win_name,p->src);
		p->n++;
		check_line_state = FALSE;
	}
}

//初始化第一帧，画出要跟踪的对象
void InitialVideo(Mat& src, Rect& rect)
{
	params p;
	src.copyTo(p.src);
	p.win_name = "Initial Window";
	imshow(p.win_name, src);

	//鼠标召回事件
	setMouseCallback(p.win_name, &mouseEvent, &p); 
	cout<<"draw rect & press any key to end"<<endl;

	waitKey();

	//把得到的位置赋给rect
	rect.x = min(p.loc1.x, p.loc2.x);
	rect.y = min(p.loc1.y, p.loc2.y);
	rect.width = abs(p.loc1.x - p.loc2.x);
	rect.height = abs(p.loc1.y - p.loc2.y);

	destroyWindow("Initial Window");
}

int main()
{
	//初始化窗口
	namedWindow("src",1);
	namedWindow("hand",1);
	namedWindow("skinShow",1);
	
	//初始化分类器
	svmClassifier.load("F://test//20130428_result//naivelinearSVM2.txt");

	//肤色设置
	sprintf(imagesname,"F://test//photo_20130619//handColor//%d.jpg",0);
	srcC=imread(imagesname,1);
	sprintf(imagesname,"F://test//photo_20130619//handDepth//%d.jpg",0);
	srcD=imread(imagesname,-1);
	handthreshold(srcC,srcD,low,high);

	//手部检测
	//sprintf(imagesname,"F://test//photo_20130527_c//%d.jpg",1);
	//imageD=imread(imagesname);
	sprintf(imagesname,"F://test//photo_201307012_c//%d.jpg",1);
	imageC=imread(imagesname);	
	sprintf(imagesname,"F://test//photo_201307012_m//%d.xml",1);
	fs.open(imagesname,FileStorage::READ);
	fs["depth"]>>imageDX;
	fs.release();
	imageDX.convertTo(imageD,CV_8UC1,scaleFactor);
	cvtColor(imageD,imageDC,CV_GRAY2RGB);

	if(handDetect(imageC,imageD,hand,skinShow,roi,low,high)); //手部检测
	else InitialVideo(imageC,roi);                            //未检测成功的话手工选取
	{
		trackerTest tracker(imageC,imageDX);
		tracker.show = Mat(480, 640, CV_8UC1);
		imageSave=Mat(480,1280,CV_8UC3);
		selectTarget(tracker,roi,temp); 
		
		//手部跟踪
		for(int i=1;i<185;i++)
		{
			//sprintf(imagesname,"F://test//photo_20130527_c//%d.jpg",i);
			//imageD=imread(imagesname);
			sprintf(imagesname,"F://test//photo_201307012_c//%d.jpg",i);
			imageC=imread(imagesname);
			sprintf(imagesname,"F://test//photo_201307012_m//%d.xml",i);
			fs.open(imagesname,FileStorage::READ);
			fs["depth"]>>imageDX;
			fs.release();
			start=clock();
			imageDX.convertTo(imageD,CV_8UC1,scaleFactor);
			cvtColor(imageD,imageDC,CV_GRAY2RGB);

			//跟踪
			if(!imageC.data)break;
			Tracker(tracker,0,imageC,imageDX);
			handDetect(imageC,imageD,hand,skinShow,roi,low,high);

			//分类
			Mat imageDhand=Mat(imageDX,tracker.target);
			naiveFeatures=Mat::zeros(1,featuresDimension,CV_32FC1);
			Mat handMat(imageDhand.rows,imageDhand.cols,CV_8UC1);
	        segmentHand(handMat,Rect(0,0,imageDhand.cols,imageDhand.rows),imageDhand);
			computeFeatures(handMat,naiveFeatures,featuresDimension);
			int label=classifylabel(naiveFeatures,svmClassifier);
			//cout<<label<<endl;
			tend=clock();

			//显示
			sprintf(imagesname,"Gesture %d, Time: %dms",label,tend-start);
			string msg(imagesname);
			cout<<msg<<endl;
			int baseLine = 0;
			Size textSize = getTextSize(msg, 1, 1, 1, &baseLine);
			Point textOrigin(imageC.cols - 2*textSize.width - 10, imageC.rows - 2*baseLine - 10);
			rectangle(imageDC,tracker.target,Scalar(0,0,255),1,8,0);
			rectangle(imageC,tracker.target,Scalar(0,0,255),1,8,0);
			putText( imageDC, msg, textOrigin, 1, 1,Scalar(0,0,255));
			for(int i=0;i<tracker.target.height;i++)
				for(int j=0;j<tracker.target.width;j++)
					if(handMat.at<uchar>(i,j))imageDC.at<Vec3b>(tracker.target.y+i,tracker.target.x+j)=red;
			//imshow("srcC",imageC);
			//imshow("srcD",imageDC);
			imshow("skinShow",skinShow);
			//保存

			IplImage* pI1 = &imageSave.operator IplImage();
			IplImage* pI2 = &imageC.operator IplImage();
			IplImage* pI3 = &imageDC.operator IplImage();

			//图像拼接
			cvSetImageROI(pI1,cvRect(0,0,640,480));
			cvCopy(pI3,pI1);
			cvSetImageROI(pI1,cvRect(640,0,1280,480));
			cvCopy(pI2,pI1);
			cvResetImageROI(pI1);
			//skinShow.adjustROI(0,0,640,480);
			//imageC.copyTo(skinShow);
			//skinShow.adjustROI(640,480,1280,480);
			//imageDC.copyTo(skinShow);
			//imageSave(Rect(0,0,640,480))=imageC;
			//imageSave(Rect(640,480,1280,480))=imageDC;
			//imageSave.colRange(Range(1,640))=imageC;
			//imageSave.colRange(Range(641,1280))=imageDC;
			imshow("src",imageSave);
			videoWriter<<imageSave;
			waitKey(3);
		}
	}
	videoWriter.release();
}

