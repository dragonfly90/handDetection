/************************************************************************
* File:	RunTracker.cpp
* Brief: C++ demo for paper: Kaihua Zhang, Lei Zhang, Ming-Hsuan Yang,"Real-Time Compressive Tracking," ECCV 2012.
* Version: 1.0
* Author: Yang Xian
* Email: yang_xian521@163.com
* Date:	2012/08/03
* History:
* Revised by Kaihua Zhang on 14/8/2012, 23/8/2012
* Email: zhkhua@gmail.com
* Homepage: http://www4.comp.polyu.edu.hk/~cskhzhang/
* Project Website: http://www4.comp.polyu.edu.hk/~cslzhang/CT/CT.htm
************************************************************************/
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "CompressiveTracker.h"

using namespace cv;
using namespace std;

//最多对一个对象进行跟踪;
#define MAX_OBJECTS 1

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

void readConfig(char* configFileName, char* imgFilePath, Rect &box);
/*  Description: read the tracking information from file "config.txt"
    Arguments:	
	-configFileName: config file name
	-ImgFilePath:    Path of the storing image sequences
	-box:            [x y width height] intial tracking position
	History: Created by Kaihua Zhang on 15/8/2012
*/
void readImageSequenceFiles(char* ImgFilePath,vector <string> &imgNames);
/*  Description: search the image names in the image sequences 
    Arguments:
	-ImgFilePath: path of the image sequence
	-imgNames:  vector that stores image name
	History: Created by Kaihua Zhang on 15/8/2012
*/

int main(int argc, char * argv[])
{

// 	char imgFilePath[100];
//     char  conf[100];
// 	strcpy(conf,"./config.txt");
// 
// 	char tmpDirPath[MAX_PATH+1];
// 	
// 	Rect box; // [x y width height] tracking position
// 
// 	vector <string> imgNames;
//     
// 	readConfig(conf,imgFilePath,box);
// 	readImageSequenceFiles(imgFilePath,imgNames);

	//从默认摄像头输入
	//VideoCapture cap(0);
	//从视频输入
	//string filename = "D:/soccer.avi";
	//VideoCapture cap(filename); 

	//if(!cap.isOpened())  // check if we succeeded
	//{
	//	cout<<"couldn't open video file"<<endl;
	//	return -1;
	//}

	VideoWriter videoWriter("F://test//result201306162//CTcolorResult.avi",-1,10,cvSize(640,480),true);
	// CT framework
	CompressiveTracker ct;

	Mat frame;
	Mat grayImg;

	bool isFirstFrame = TRUE;
	Rect rect; 
	char readnames[255];
	int i=5;
	for (;i<200;)
	{
		//cap>>frame;
		 sprintf(readnames,"F://test//photo_201306162//Color//%d.jpg",i++);
		 frame=imread(string(readnames));
		 if(!frame.data)break;
		//初始化第一帧
		if ( isFirstFrame)
		{
			InitialVideo(frame,rect);//画框
			cvtColor(frame, grayImg, CV_RGB2GRAY);    
			
			ct.init(grayImg, rect);//初始化框里的内容
			isFirstFrame = FALSE;
		}
		
		//其他操作
		//rectangle(frame, rect, CV_RGB(255,0,0), 2, 8 );
		cvtColor(frame, grayImg, CV_RGB2GRAY);

		ct.processFrame(grayImg, rect);// Process frame

		rectangle(frame, rect, Scalar(200,0,0),2);// Draw rectangle
		videoWriter<<frame;
		imshow("CT", frame);// Display
		char key = (char)waitKey(5);
		switch (key)
		{
		case 27:
			return 0;
		case  ' ' :
			cout<<"save process"<<endl;
			break;
		default:
			break;
		}		
	}
		//fclose(resultStream);
	videoWriter.release();
		return 0;
}




void readConfig(char* configFileName, char* imgFilePath, Rect &box)	
{
	int x;
	int y;
	int w;
	int h;

	fstream f;
	char cstring[1000];
	int readS=0;

	f.open(configFileName, fstream::in);

	char param1[200]; strcpy(param1,"");
	char param2[200]; strcpy(param2,"");
	char param3[200]; strcpy(param3,"");

	f.getline(cstring, sizeof(cstring));
	readS=sscanf (cstring, "%s %s %s", param1,param2, param3);

	strcpy(imgFilePath,param3);

	f.getline(cstring, sizeof(cstring)); 
	f.getline(cstring, sizeof(cstring)); 
	f.getline(cstring, sizeof(cstring));


	readS=sscanf (cstring, "%s %s %i %i %i %i", param1,param2, &x, &y, &w, &h);

	box = Rect(x, y, w, h);
	
}

void readImageSequenceFiles(char* imgFilePath,vector <string> &imgNames)
{	
	imgNames.clear();

	char tmpDirSpec[MAX_PATH+1];
	sprintf (tmpDirSpec, "%s/*", imgFilePath);

	WIN32_FIND_DATA f;
	HANDLE h = FindFirstFile(tmpDirSpec , &f);
	if(h != INVALID_HANDLE_VALUE)
	{
		FindNextFile(h, &f);	//read ..
		FindNextFile(h, &f);	//read .
		do
		{
			imgNames.push_back(f.cFileName);
		} while(FindNextFile(h, &f));

	}
	FindClose(h);	
}