#include "detect.h"

bool handDetect(const Mat& srcC,const Mat& srcD,Mat& hand,Mat& skinShow,Rect& roi,Scalar& low,Scalar& high) 
{
    Mat imageD,imageC;
	Mat yuv,dst,sthreshold; 
	vector<vector<Point> > contours;


	low=Scalar(0,133,77);
	high=Scalar(255,173,127);
	//resize(srcD,imageD,Size(80,60));
	Canny(srcD,imageD,50,150);
	//resize(srcC,imageC,Size(80,60));
	
	skinShow=Mat::zeros(srcC.rows,srcC.cols,CV_8UC1); 
    cvtColor(srcC,yuv,CV_BGR2YCrCb);  
    //Mat dstTemp1(src.rows, src.cols, CV_8UC1);  
    //Mat dstTemp2(src.rows, src.cols, CV_8UC1);  
    // 对YUV空间进行量化，得到2值图像，亮的部分为手的形状  
    inRange(yuv, /*Scalar(0,133,0)*/low, /*Scalar(256,173,256)*/high, skinShow);  
    //inRange(yuv, Scalar(0,0,77), Scalar(256,256,127), dstTemp2);  
    //bitwise_and(dst, imageD, skinShow);  
	skinShow.copyTo(sthreshold);
	findContours(sthreshold,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
	double max_area=0;
	int max_contour=-1;
	/*
	for(int i=0;i<contours.size();i++)
	{
	  if(contourArea(contours[i])>max_area)
	  {
		  max_area=contourArea(contours[i]);
		  max_contour=i;
	  }
	}*/
	/*
	if(max_contour>=0)
	{
	roi=boundingRect(contours[max_contour]);
	hand=Mat(srcC,roi);
	return true;
	}
	else */
		return false;
}

void handthreshold(const Mat srcC,const Mat srcD,Scalar& low,Scalar& high)
{
	Mat yuv,dst;  
	Rect roi;
	vector<Point> searchPoints;
	searchPoint(50,50,srcD,roi,searchPoints,10,15);
	uchar yl=255,yh=0,ul=255,uh=0,vl=255,vh=0;
	
    cvtColor(srcC,yuv,CV_BGR2YCrCb);
	Mat_<Vec3b> _I = yuv;
	for(int i=0;i<searchPoints.size();i++)
	{
		if(_I(searchPoints[i].x,searchPoints[i].y)[1]>yh)
			yh=_I(searchPoints[i].x,searchPoints[i].y)[0];
		if(_I(searchPoints[i].x,searchPoints[i].y)[1]<yl)
			yl=_I(searchPoints[i].x,searchPoints[i].y)[0];
		if(_I(searchPoints[i].x,searchPoints[i].y)[1]>uh)
			uh=_I(searchPoints[i].x,searchPoints[i].y)[1];
		if(_I(searchPoints[i].x,searchPoints[i].y)[1]<ul)
			ul=_I(searchPoints[i].x,searchPoints[i].y)[1];
		if(_I(searchPoints[i].x,searchPoints[i].y)[1]>vh)
			vh=_I(searchPoints[i].x,searchPoints[i].y)[2];
		if(_I(searchPoints[i].x,searchPoints[i].y)[1]<vl)
			vl=_I(searchPoints[i].x,searchPoints[i].y)[2];
	}
	low=Scalar(max(yl-10,0),max(ul-10,0),vl);
	high=Scalar(min(yh+10,255),min(uh+10,255),vh);
}

bool searchPoint(unsigned int x,unsigned int y,const Mat& image,Rect& roi,vector<Point>& searchPoints,unsigned short threshold,unsigned short globalT)
{
	int num=0;
	int minW,maxW,minH,maxH;
	int width=image.cols,height=image.rows;
	Point pointS;
	searchPoints.push_back(Point(x,y));
	int origin=int(image.at<uchar>(x,y));
	//cout<<origin<<endl;
	//cout<<image.at<uchar>(x,y)<<endl;
	minW=x;
	maxW=x;
	minH=y;
	maxH=y;
	bool* visit=new bool[width*height];
	//memcpy(visit,0,width*height);
	for(int i=0;i<width*height;i++)
		visit[i]=false;
	visit[x*width+y]=true;
	while(num<searchPoints.size())
	{
		pointS=searchPoints[num];
		if((pointS.x-1>=0)&&(!visit[(pointS.x-1)*width+pointS.y])&&(abs(int(image.at<uchar>(pointS.x-1,pointS.y))-int(image.at<uchar>(pointS.x,pointS.y)))<threshold)&&(abs(int(image.at<uchar>(pointS.x-1,pointS.y))-origin)<globalT))
		{
			searchPoints.push_back(Point(pointS.x-1,pointS.y));
			visit[(pointS.x-1)*width+pointS.y]=true;
		}
		if((pointS.x+1<width)&&(!visit[(pointS.x+1)*width+pointS.y])&&(abs(int(image.at<uchar>(pointS.x+1,pointS.y))-int(image.at<uchar>(pointS.x,pointS.y)))<threshold)&&(abs(int(image.at<uchar>(pointS.x+1,pointS.y))-origin)<globalT))
		{
			searchPoints.push_back(Point(pointS.x+1,pointS.y));
			visit[(pointS.x+1)*width+pointS.y]=true;
		}
		if((pointS.y-1>=0)&&(!visit[pointS.x*width+pointS.y-1])&&(abs(int(image.at<uchar>(pointS.x,pointS.y-1))-int(image.at<uchar>(pointS.x,pointS.y)))<threshold)&&(abs(int(image.at<uchar>(pointS.x,pointS.y-1))-origin)<globalT))
		{
			searchPoints.push_back(Point(pointS.x,pointS.y-1));
			visit[pointS.x*width+pointS.y-1]=true;
		}
		if((pointS.y+1<height)&&(!visit[pointS.x*width+pointS.y+1])&&(abs(int(image.at<uchar>(pointS.x,pointS.y+1))-int(image.at<uchar>(pointS.x,pointS.y)))<threshold)&&(abs(int(image.at<uchar>(pointS.x,pointS.y+1))-origin)<globalT))
		{
			searchPoints.push_back(Point(pointS.x,pointS.y+1));
			visit[pointS.x*width+pointS.y+1]=true;
		}
		num++;
		//cout<<num<<' '<<searchPoints.size()<<endl;
	}
	for(int i=0;i<searchPoints.size();i++)
	{
	if(searchPoints[i].x>maxW)maxW=searchPoints[i].x;
	else
		if(searchPoints[i].x<minW)minW=searchPoints[i].x;
	if(searchPoints[i].y>maxH)maxH=searchPoints[i].y;
	else
		if(searchPoints[i].y<minH)minH=searchPoints[i].y;
	}
	delete visit;
	return true;
}