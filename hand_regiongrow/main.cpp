#include<opencv.hpp>
#include"common.h"
Mat image(480,640,CV_8UC1),imageShow;
FileStorage fs;
Rect roi;
vector<Point> points;
char imagenames[255];
int main(){
	namedWindow("Image",1);
	namedWindow("Origin",1);
	for(int i=0;i<100;i++)
	{
	sprintf(imagenames,"F://test//photo_201306162//handDepth//1//%d.jpg",i);
	
	//fs.open(string(imagenames),FileStorage::READ);
	//fs["depth"]>>image;
	points.clear();
	image=imread(string(imagenames),0);
	if(image.data==NULL)break;
			//cout<</*int(image.at<uchar>(j,m))*/<<endl;
	//cout<<image<<endl;
	imageShow=Mat::zeros(image.rows,image.cols,CV_8UC1);
	cout<<int(image.at<uchar>(49,49))<<endl;
	//for(int j=0;j<image.rows;j++)
	//	for(int m=0;m<image.cols;m++)
	//{
    //   imageShow.at<uchar>(j,m)=255;
	// if(image.at<uchar>(j,m)>130)imageShow.at<uchar>(j,m)=255;   
	//}
     search((image.rows/2-1),(image.cols/2-1),image,roi,points,8,13);
	 for(int j=0;j<points.size();j++)
	 	imageShow.at<uchar>(points[j].x,points[j].y)=255;
	//int BIN_THRESH_OFFSET=5;
	//int handDepth=image.at<uchar>(49,49) ;
	//image = (image > (handDepth - BIN_THRESH_OFFSET)) & (image < (handDepth + BIN_THRESH_OFFSET));
	imshow("Origin",image);
	imshow("Image",imageShow);
	waitKey();
	//fs<<"depth"<<depth;
	//fs.release();
	//waitKey(10);
	}
}