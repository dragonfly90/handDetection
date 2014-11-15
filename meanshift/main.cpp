// Realtime Robot Tracking System
//  Brandon Green

#include <time.h>
#include <iostream>
using namespace std;

#ifndef _EiC
#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <ctype.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#endif
using namespace cv;


//����һ��������и���;
#define MAX_OBJECTS 1

//���ݲ�����class
class params
{
public:

	Point loc1,loc2;
	string win_name;
	Mat src;
	Mat cur;
	int n;					//��¼�������
	params() : n(0) {}
	// 	~params();
};

IplImage *image = 0, *hsv = 0, *hue = 0, *mask = 0, *backproject = 0, *histimg = 0;
CvHistogram *hist = 0;

int backproject_mode = 0;
int select_object = 0;
int track_object = 0;
int show_hist = 1;
CvPoint origin;
CvRect selection;
CvRect track_window;
CvBox2D track_box;
CvConnectedComp track_comp;
int hdims = 32;
float hranges_arr[] = {0,180};
float* hranges = hranges_arr;
int vmin = 100, vmax = 256, smin = 0;
int serialno=1;            //���к� 

void on_mouse( int event, int x, int y, int flags, void* param )
{
   if( !image )                                   //����ͼ��
        return;

    if( image->origin )                         //y����
        y = image->height - y;

    if( select_object )                          //ѡ���������
    {
        selection.x = MIN(x,origin.x);            //���ڵ����ϽǺ�����
        selection.y = MIN(y,origin.y);            //���ڵ����Ͻ�������
        selection.width = selection.x + CV_IABS(x - origin.x);        //���ڵĿ��
        selection.height = selection.y + CV_IABS(y - origin.y);       //���ڵĸ߶�
        
        selection.x = MAX( selection.x, 0 );       //��ֹ���Ͻ�����С����           
        selection.y = MAX( selection.y, 0 );
        selection.width = MIN( selection.width, image->width );      //��ֹ���½����곬��ͼ���С
        selection.height = MIN( selection.height, image->height );
        selection.width -= selection.x;
        selection.height -= selection.y;

		//p->src.copyTo(tmp);
		cvRectangleR(image, selection, CV_RGB(255,0,0), 2, 8 );
		cvShowImage("MeanShiftDemo",image);
    }

    switch( event )
    {
    case CV_EVENT_LBUTTONDOWN:         //���ڳ�ʼ��
        origin = cvPoint(x,y);
        selection = cvRect(x,y,0,0);
        select_object = 1;
        break;
    case CV_EVENT_LBUTTONUP:          //����ע��
        select_object = 0;
        if( selection.width > 0 && selection.height > 0 )
            track_object = -1;
        break;
    }
}

/*
void main_on_mouse(int event,int x,int y,int flags,void* param)
{
	app.mouse(event,x,y,flags);
}
*/
CvScalar hsv2rgb( float hue )
{
    int rgb[3], p, sector;
    static const int sector_data[][3]=                             
        {{0,2,1}, {1,2,0}, {1,0,2}, {2,0,1}, {2,1,0}, {0,1,2}};
    hue *= 0.033333333333333333333333333333333f;
    sector = cvFloor(hue);
    p = cvRound(255*(hue - sector));
    p ^= sector & 1 ? 255 : 0;

    rgb[sector_data[sector][0]] = 255;
    rgb[sector_data[sector][1]] = 0;
    rgb[sector_data[sector][2]] = p;

    return cvScalar(rgb[2], rgb[1], rgb[0],0);
}

//����¼�
void mouseEvent(int event, int x, int y, int flags, void* param)  
{
	static bool check_line_state = FALSE;
	Mat tmp;
	params* p = (params*)param;
	//�������������洢��ʼλ��;
	if( event == CV_EVENT_LBUTTONDOWN )
	{
		if (p->n == MAX_OBJECTS)//�ж��Ƿ��Ѿ����˸��ٶ���;
		{
			cout<<"Fail! \n Only Can Tracking One Object!"<<endl;
			return ;
		}
		p->loc1.x = x;
		p->loc1.y = y;
		check_line_state = TRUE;
	}

	//����������������ƶ��������ο�;
	else if (check_line_state && event == CV_EVENT_MOUSEMOVE)
	{
		if (p->n == MAX_OBJECTS)//�ж��Ƿ��Ѿ����˸��ٶ���;
		{
			cout<<"Fail! \n Only Can Tracking One Object!"<<endl;
			return ;
		}
		p->src.copyTo(tmp);
		rectangle(tmp, p->loc1, Point(x,y), CV_RGB(255,0,0), 2, 8 );
		imshow(p->win_name,tmp);
	}

	//������𣬻������;
	else if(event == CV_EVENT_LBUTTONUP)
	{
		if (p->n == MAX_OBJECTS)//�ж��Ƿ��Ѿ����˸��ٶ���;
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

//��ʼ����һ֡������Ҫ���ٵĶ���
void InitialVideo(Mat& src, Rect& rect)
{
	params p;
	src.copyTo(p.src);
	p.win_name = "Initial Window";
	imshow(p.win_name, src);

	//����ٻ��¼�
	setMouseCallback(p.win_name, &mouseEvent, &p); 
	cout<<"draw rect & press any key to end"<<endl;

	waitKey();

	//�ѵõ���λ�ø���rect
	rect.x = min(p.loc1.x, p.loc2.x);
	rect.y = min(p.loc1.y, p.loc2.y);
	rect.width = abs(p.loc1.x - p.loc2.x);
	rect.height = abs(p.loc1.y - p.loc2.y);

	destroyWindow("Initial Window");
}


int main(int argc, char **argv)
{      
    char filename1[255];                      //������ļ���
    sprintf(filename1, "F://test//photo_201306162//Color//%d.jpg", serialno++);
	CvVideoWriter* videoWriter=cvCreateVideoWriter("F://test//result201306162//meanshiftcolorResult.avi",-1,10,cvSize(640,480),true);
    printf( "Hot keys: \n"
        "\tESC - quit the program\n"
        "\tc - stop the tracking\n"
        "\tb - switch to/from backprojection view\n"
        "\th - show/hide object histogram\n"
        "To initialize tracking, select the object with mouse\n" );

    cvNamedWindow( "Histogram", 1 );      //��ɫH������ֱ��ͼ
    cvNamedWindow( "MeanShiftDemo", 1 );   //��ʾ���ٵĴ���
    cvSetMouseCallback( "MeanShiftDemo", on_mouse, 0 );
    cvCreateTrackbar( "Vmin", "MeanShiftDemo", &vmin, 256, 0 );   //��
    cvCreateTrackbar( "Vmax", "MeanShiftDemo", &vmax, 256, 0 );
    cvCreateTrackbar( "Smin", "MeanShiftDemo", &smin, 256, 0 );
    IplImage* frame;
	//	=cvCreateImage(cvSize(1024,1024),8,3);  //ԭʼͼ��
    //IplImage* frame=cvCreateImage(cvSize(512,512),8,3);         //�ɼ���һ���ͼ��
	bool isFirstFrame=true;
    for(;serialno<200;)
    {
	
        int i, bin_w, c;                   
        frame=cvLoadImage(filename1);                   //����ͼ��
		//if(!frame)
		//	break;
		//cvCopy(cam_image_,frame);
        //cvPyrDown(cam_image_,frame);                         //ͼ�����ų�ԭʼ��һ���С

        if( !frame )
            break;

		 if ( isFirstFrame)
		{
			Mat frame1(frame);
			Rect rect;
			InitialVideo(frame1,rect);//����
		    selection=CvRect(rect);
			isFirstFrame = FALSE;
			track_object=-1;
			rectangle(frame1,rect,Scalar(0,255,0));
			IplImage* image=&frame1.operator IplImage();
			cvWriteFrame(videoWriter,image);
		}
		

		sprintf(filename1,"F://test//photo_201306162//Color//%d.jpg", serialno++);
        if( !image )
        {
            /* allocate all the buffers */
            image = cvCreateImage( cvGetSize(frame), 8, 3 );
            image->origin = frame->origin;
            hsv = cvCreateImage( cvGetSize(frame), 8, 3 );      //HSV�ռ�ͼ��
            hue = cvCreateImage( cvGetSize(frame), 8, 1 );      //H����
            mask = cvCreateImage( cvGetSize(frame), 8, 1 );       //maskͼ��
            backproject = cvCreateImage( cvGetSize(frame), 8, 1 );  //backprojectͼ��
            hist = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 ); //����ֱ��ͼ
            histimg = cvCreateImage( cvSize(320,200), 8, 3 );
            cvZero( histimg );
        }

        cvCopy( frame, image, 0 );                       //��ԭʼ֡ͼ�񿽱�������ͼ��

		cvCvtColor( image, hsv, CV_BGR2HSV );            //��ͼ��ת����HSV�ռ��ͼ��

        if( track_object )                                //��ʼ��������
        {
            int _vmin = vmin, _vmax = vmax;

            cvInRangeS( hsv, cvScalar(0,smin,MIN(_vmin,_vmax),0),     //����������鷶Χ
                        cvScalar(180,256,MAX(_vmin,_vmax),0), mask );
            cvSplit( hsv, hue, 0, 0, 0 );                     //ȡH����
			//InitialVideo(image,);
			//InitialVideo();
            if( track_object < 0 )
            {
                float max_val = 0.f;
                cvSetImageROI( hue, selection );         //����HUE�ĸ���Ȥ����
                cvSetImageROI( mask, selection );        //����MASK�ĸ���Ȥ����
                cvCalcHist( &hue, hist, 0, mask );        //����ͼ��hue ��ֱ��ͼ 
                cvGetMinMaxHistValue( hist, 0, &max_val, 0, 0 );  //����������Сֱ���� 
                cvConvertScale( hist->bins, hist->bins, max_val ? 255. / max_val : 0., 0 );
                cvResetImageROI( hue );
                cvResetImageROI( mask );
                track_window = selection;
                track_object = 1;

                cvZero( histimg );
                bin_w = histimg->width / hdims;    //�ռ����Ŀ��
                for( i = 0; i < hdims; i++ )
                {
                    int val = cvRound( cvGetReal1D(hist->bins,i)*histimg->height/255 );
                    CvScalar color = hsv2rgb(i*180.f/hdims);
                    cvRectangle( histimg, cvPoint(i*bin_w,histimg->height),
                                 cvPoint((i+1)*bin_w,histimg->height - val),
                                 color, -1, 8, 0 );
                }
            }

            cvCalcBackProject( &hue, backproject, hist );//���㷴��ͶӰ 
            cvAnd( backproject, mask, backproject, 0 );  //cvAnd
            cvMeanShift( backproject, track_window,
                        cvTermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ),
                        &track_comp);
            track_window = track_comp.rect;
			//track_box.center=track_window.center;
			//track_box.size=track_window.size;
			//track_box.angle=track_window.angle;
            
            if( backproject_mode )
                cvCvtColor( backproject, image, CV_GRAY2BGR );
            if( image->origin )
                track_box.angle = -track_box.angle;
            cvRectangle( image,cvPoint(track_window.x,track_window.y),cvPoint(track_window.x+track_window.width,track_window.y+track_window.height),cvScalar(255,0,0));
        }
        cvWriteFrame(videoWriter,image);
        if( select_object && selection.width > 0 && selection.height > 0 )
        {
            cvSetImageROI( image, selection );
            cvXorS( image, cvScalarAll(255), image, 0 );
            cvResetImageROI( image );
        }

        cvShowImage( "MeanShiftDemo", image );
        cvShowImage( "Histogram", histimg );

        c = cvWaitKey(1);
        if( (char) c == 27 )
            break;
        switch( (char) c )
        {
		case 'b':
            backproject_mode = 1;
            break;
        case 'c':
            track_object = 0;
            cvZero( histimg );
            break;
        case 'h':
            show_hist ^= 1;
            if( !show_hist )
                cvDestroyWindow( "Histogram" );
            else
                cvNamedWindow( "Histogram", 1 );
            break;
        default:
            ;
        }
    }
	
    cvDestroyWindow("Histogram");
    cvDestroyWindow("MeanShiftDemo");
    cvReleaseImage(&image);
	cvReleaseVideoWriter(&videoWriter);
	cvReleaseImage(&histimg);
	
    return 0;
}
