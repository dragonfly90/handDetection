//============================================================================
// Name        : opencv_handdetect.cpp
// Author      : andol li, andol@andol.info
// Version     : 0.1
// Copyright   : 2012
// Description : using haartraining results to detect the hand gesture of FIST in video stream.
//				 
//============================================================================
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>

#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;

const double scale = 1.1;

//1.0 api version
CvMemStorage* storage = 0;
CvHaarClassifierCascade* cascade = 0;
void detectAndDraw(IplImage *input_image);
const char* cascade_name = "palm.xml";

//define the path to cascade file
string cascadeName = "palm.xml"; /*ROBUST-fist detection haartraining file*/

int main()
{
	//1.0 api version
	//CvCapture *capture =0;
	CvVideoWriter* videoWriter=cvCreateVideoWriter("F://test//result201306162//haarcolorResult.avi",-1,10,cvSize(640,480),true);
	IplImage *frame, *frame_copy = 0;
	cascade = (CvHaarClassifierCascade*)cvLoad( cascade_name, 0, 0, 0 );
	if( !cascade ){
	        fprintf( stderr, "ERROR: Could not load classifier cascade\n" );
	        return -1;
	}
	storage = cvCreateMemStorage(0);
    char readnames[255];
	int i=5;
	//capture = cvCaptureFromCAM(0);
	cvNamedWindow("result", 1);
	//if(capture){
	//	for(;;){
		
	for (;i<200;)
	{
		//cap>>frame;
		 sprintf(readnames,"F://test//photo_201306162//Color//%d.jpg",i++);
		 frame=cvLoadImage(readnames);
			//if(!cvGrabFrame(capture)) break;
			//frame = cvRetrieveFrame( capture);
			if(!frame) break;
			
			if(!frame_copy) frame_copy = cvCreateImage(cvSize(frame->width, frame->height), IPL_DEPTH_8U, frame->nChannels);
			if(frame->origin == IPL_ORIGIN_TL)
				cvCopy(frame, frame_copy, 0);
			else
				cvFlip(frame, frame_copy, 0);
			
			
			detectAndDraw(frame_copy);
			cvWriteFrame(videoWriter,frame_copy);
			if(cvWaitKey(10) >= 0) break;
		}
	cvReleaseImage( &frame_copy );
		//cvReleaseCapture( &capture );
     cvReleaseVideoWriter(&videoWriter);

	return 0;
}

void detectAndDraw(IplImage *img)
{
	double scale = 1.1;
	IplImage* temp = cvCreateImage( cvSize(img->width/scale,img->height/scale), 8, 3 );
	CvPoint pt1, pt2;
	int i;

	cvClearMemStorage( storage );
	if(cascade){
		CvSeq* faces = cvHaarDetectObjects(
				img,
				cascade,
				storage,
				scale, 2, CV_HAAR_DO_CANNY_PRUNING,
                cvSize(24, 24) );
		for( i = 0; i < (faces ? faces->total : 0); i++ )
		{
			CvRect* r = (CvRect*)cvGetSeqElem( faces, i );
			pt1.x = r->x*scale;
			pt2.x = (r->x+r->width)*scale;
			pt1.y = r->y*scale;
			pt2.y = (r->y+r->height)*scale;
			cvRectangle( img, pt1, pt2, CV_RGB(255, 0, 0), 2, 8, 0 );
		}
	}
	cvShowImage("result", img);
	cvReleaseImage( &temp );
}
