/**********************************************************************

Bilkent University:

Mean-shift Tracker based Moving Object Tracker in Video

Version: 1.0

Compiler: Microsoft Visual C++ 6.0 (tested in both debug and release
          mode)

Modified by Mr Zhou

**********************************************************************/
#include "ObjectTracker.h"
#include "utils.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
/*
#define GetRValue(rgb)   ((UBYTE8) (rgb))
#define GetGValue(rgb)   ((UBYTE8) (((ULONG_32) (rgb)) >> 8))
#define GetBValue(rgb)   ((UBYTE8) ((rgb) >> 16))
*/
//#define RGB(r, g ,b) ((ULONG_32) (((UBYTE8) (r) | ((UBYTE8) (g) << 8)) | (((ULONG_32) (UBYTE8) (b)) << 16)))

#define min(a, b) (((a) < (b)) ? (a) : (b))

#define max(a, b) (((a) > (b)) ? (a) : (b))


#define MEANSHIFT_ITARATION_NO 5
#define DISTANCE_ITARATION_NO 1
#define ALPHA 1
#define EDGE_DETECT_TRESHOLD 32
//////////////////////////////////////////////////
/*
1 给定目标的初始位置和尺寸, 计算目标在图像中的直方图;
2 输入新图像, 迭代直到收敛:
计算图像上对应区域的新直方图;
新直方图与目标直方图比较,计算权重;
根据权重,计算图像上对应区域的形心/质心;
根据形心,修正目标位置;

直方图分为两部分, 每部分大小4096,
RGB的256*256*256种组合, 缩减为16*16*16=4096种组合.
如果目标区域的点是边缘点, 则计入直方图的后一部分,
否则计入直方图的前一部分.
*/

//////////////////////////////////////////////////

CObjectTracker::CObjectTracker(INT32 imW,INT32 imH,IMAGE_TYPE eImageType)
{

m_nImageWidth = imW;
m_nImageHeight = imH;
m_eIMAGE_TYPE = eImageType;
m_cSkipValue = 0;

for (UBYTE8 i=0;i<MAX_OBJECT_TRACK_NUMBER;i++)//初始化各个目标
{
   m_sTrackingObjectTable[i].Status = false;
      for(SINT16 j=0;j<HISTOGRAM_LENGTH;j++)
    m_sTrackingObjectTable[i].initHistogram[j] = 0;
}

m_nFrameCtr = 0;
m_uTotalTime = 0;
m_nMaxEstimationTime = 0;
m_cActiveObject = 0;
TotalDist=0.0;
LastDist=0.0;

switch (eImageType)
{
   case MD_RGBA:
    m_cSkipValue = 4 ;
break ;
   case MD_RGB:
m_cSkipValue = 3 ;
   break ;
};
};

CObjectTracker::~CObjectTracker()
{

}
//returns pixel values in format |0|B|G|R| wrt to (x.y)
/*
ULONG_32 CObjectTracker::GetPixelValues(UBYTE8 *frame,SINT16 x,SINT16 y)
{
ULONG_32 pixelValues = 0;

pixelValues = *(frame+(y*m_nImageWidth+x)*m_cSkipValue+2)|//0BGR
               *(frame+(y*m_nImageWidth+x)*m_cSkipValue+1) << 8|
      *(frame+(y*m_nImageWidth+x)*m_cSkipValue) << 16;


return(pixelValues);

}*/

//set RGB components wrt to (x.y)
void CObjectTracker::SetPixelValues(IplImage *r,IplImage *g,IplImage *b,ULONG_32 pixelValues,SINT16 x,SINT16 y)
{
// *(frame+(y*m_nImageWidth+x)*m_cSkipValue+2) = UBYTE8(pixelValues & 0xFF);
// *(frame+(y*m_nImageWidth+x)*m_cSkipValue+1) = UBYTE8((pixelValues >> 8) & 0xFF);
// *(frame+(y*m_nImageWidth+x)*m_cSkipValue) = UBYTE8((pixelValues >> 16) & 0xFF);
//setpix32f
setpix8c(r, y, x, UBYTE8(pixelValues & 0xFF));
setpix8c(g, y, x, UBYTE8((pixelValues >> 8) & 0xFF));
setpix8c(b, y, x, UBYTE8((pixelValues >> 16) & 0xFF));
}

// returns box color
ULONG_32 CObjectTracker::GetBoxColor()
{
ULONG_32 pixelValues = 0;

switch(m_cActiveObject)
{
case 0:
pixelValues = RGB(255,0,0);
break;
case 1:
pixelValues = RGB(0,255,0);
break;
case 2:
pixelValues = RGB(0,0,255);
break;
case 3:
pixelValues = RGB(255,255,0);
break;
case 4:
pixelValues = RGB(255,0,255);
break;
case 5:
pixelValues = RGB(0,255,255);
break;
case 6:
pixelValues = RGB(255,255,255);
break;
case 7:
pixelValues = RGB(128,0,128);
break;
case 8:
pixelValues = RGB(128,128,0);
break;
case 9:
pixelValues = RGB(128,128,128);
break;
case 10:
pixelValues = RGB(255,128,0);
break;
case 11:
pixelValues = RGB(0,128,128);
break;
case 12:
pixelValues = RGB(123,50,10);
break;
case 13:
pixelValues = RGB(10,240,126);
break;
case 14:
pixelValues = RGB(0,128,255);
break;
case 15:
pixelValues = RGB(128,200,20);
break;
default:
break;
}

return(pixelValues);


}
//初始化一个目标的参数
void CObjectTracker::ObjectTrackerInitObjectParameters(SINT16 x,SINT16 y,SINT16 Width,SINT16 Height)
{

   m_cActiveObject = 0;

   m_sTrackingObjectTable[m_cActiveObject].X = x;
   m_sTrackingObjectTable[m_cActiveObject].Y = y;
   m_sTrackingObjectTable[m_cActiveObject].W = Width;
   m_sTrackingObjectTable[m_cActiveObject].H = Height;

   m_sTrackingObjectTable[m_cActiveObject].vectorX = 0;
   m_sTrackingObjectTable[m_cActiveObject].vectorY = 0;


   m_sTrackingObjectTable[m_cActiveObject].Status = true;
   m_sTrackingObjectTable[m_cActiveObject].assignedAnObject = false;
}

//进行一次跟踪
void CObjectTracker::ObjeckTrackerHandlerByUser(IplImage *frame)
{
   m_cActiveObject = 0;

   if (m_sTrackingObjectTable[m_cActiveObject].Status)
   {
    if (!m_sTrackingObjectTable[m_cActiveObject].assignedAnObject)
    {
     //计算目标的初始直方图
     FindHistogram(frame,m_sTrackingObjectTable[m_cActiveObject].initHistogram);
           m_sTrackingObjectTable[m_cActiveObject].assignedAnObject = true;
    }
    else
    {
     //在图像上搜索目标
     FindNextLocation(frame);   

     DrawObjectBox(frame);
    }
   }

}
//Extracts the histogram of box
//frame: 图像
//histogram: 直方图
//在图像frame中计算当前目标的直方图histogram
//直方图分为两部分,每部分大小4096,
//RGB的256*256*256种组合,缩减为16*16*16=4096种组合
//如果目标区域的点是边缘点,则计入直方图的后一部分,
//否则计入直方图的前一部分
void CObjectTracker::FindHistogram(IplImage *frame, FLOAT32 (*histogram))
{
SINT16 i = 0;
SINT16 x = 0;
SINT16 y = 0;
UBYTE8 E = 0;
UBYTE8 qR = 0,qG = 0,qB = 0;
// ULONG_32 pixelValues = 0;
UINT32 numberOfPixel = 0;
IplImage* r, * g, * b;

r = cvCreateImage( cvGetSize(frame), frame->depth, 1 );
g = cvCreateImage( cvGetSize(frame), frame->depth, 1 );
b = cvCreateImage( cvGetSize(frame), frame->depth, 1 );
cvCvtPixToPlane( frame, b, g, r, NULL ); //divide color image into separate planes r, g, b. The exact sequence doesn't matter.


for (i=0;i<HISTOGRAM_LENGTH;i++) //reset all histogram
   histogram[i] = 0.0;

//for all the pixels in the region
for (y=max(m_sTrackingObjectTable[m_cActiveObject].Y-m_sTrackingObjectTable[m_cActiveObject].H/2,0);y<=min(m_sTrackingObjectTable[m_cActiveObject].Y+m_sTrackingObjectTable[m_cActiveObject].H/2,m_nImageHeight-1);y++)
   for (x=max(m_sTrackingObjectTable[m_cActiveObject].X-m_sTrackingObjectTable[m_cActiveObject].W/2,0);x<=min(m_sTrackingObjectTable[m_cActiveObject].X+m_sTrackingObjectTable[m_cActiveObject].W/2,m_nImageWidth-1);x++)
   {
    //边缘信息: 当前点与上下左右4点灰度差异是否超过阈值
    E = CheckEdgeExistance(r, g, b,x,y);

    qR = (UBYTE8)pixval8c( r, y, x )/16;//quantize R component
    qG = (UBYTE8)pixval8c( g, y, x )/16;//quantize G component
    qB = (UBYTE8)pixval8c( b, y, x )/16;//quantize B component

    histogram[4096*E+256*qR+16*qG+qB] += 1; //根据边缘信息, 累计直方图//HISTOGRAM_LENGTH=8192

    numberOfPixel++;

   }

for (i=0;i<HISTOGRAM_LENGTH;i++) //normalize
   histogram[i] = histogram[i]/numberOfPixel;
//for (i=0;i<HISTOGRAM_LENGTH;i++)
//   printf("histogram[%d]=%d/n",i,histogram[i]);
     // printf("numberOfPixel=%d/n",numberOfPixel);
cvReleaseImage(&r);
cvReleaseImage(&g);
cvReleaseImage(&b);

}
//Draw box around object
void CObjectTracker::DrawObjectBox(IplImage *frame)
{
SINT16 x_diff = 0;
SINT16 x_sum = 0;
SINT16 y_diff = 0;
SINT16 y_sum = 0;
SINT16 x = 0;
SINT16 y = 0;
ULONG_32 pixelValues = 0;
IplImage* r, * g, * b;

r = cvCreateImage( cvGetSize(frame), frame->depth, 1 );
g = cvCreateImage( cvGetSize(frame), frame->depth, 1 );
b = cvCreateImage( cvGetSize(frame), frame->depth, 1 );
cvCvtPixToPlane( frame, b, g, r, NULL );

pixelValues = GetBoxColor();

//the x left and right bounds
x_sum = min(m_sTrackingObjectTable[m_cActiveObject].X+m_sTrackingObjectTable[m_cActiveObject].W/2+1,m_nImageWidth-1);//右边界
x_diff = max(m_sTrackingObjectTable[m_cActiveObject].X-m_sTrackingObjectTable[m_cActiveObject].W/2,0);//左边界
//the y upper and lower bounds
y_sum = min(m_sTrackingObjectTable[m_cActiveObject].Y+m_sTrackingObjectTable[m_cActiveObject].H/2+1,m_nImageHeight-1);//下边界
y_diff = max(m_sTrackingObjectTable[m_cActiveObject].Y-m_sTrackingObjectTable[m_cActiveObject].H/2,0);//上边界

for (y=y_diff;y<=y_sum;y++)
{
   SetPixelValues(r, g, b,pixelValues,x_diff,y);
   SetPixelValues(r, g, b,pixelValues,x_diff+1,y);

      SetPixelValues(r, g, b,pixelValues,x_sum-1,y);
      SetPixelValues(r, g, b,pixelValues,x_sum,y);
}
for (x=x_diff;x<=x_sum;x++)
{
   SetPixelValues(r, g, b,pixelValues,x,y_diff);
      SetPixelValues(r, g, b,pixelValues,x,y_diff+1);

      SetPixelValues(r, g, b,pixelValues,x,y_sum-1);
      SetPixelValues(r, g, b,pixelValues,x,y_sum);
}
cvCvtPlaneToPix(b, g, r, NULL, frame);

cvReleaseImage(&r);
cvReleaseImage(&g);
cvReleaseImage(&b);
}
// Computes weights and drives the new location of object in the next frame
//frame: 图像
//histogram: 直方图
//计算权重, 更新目标的坐标
void CObjectTracker::FindWightsAndCOM(IplImage *frame, FLOAT32 (*histogram))
{
SINT16 i = 0;
SINT16 x = 0;
SINT16 y = 0;
UBYTE8 E = 0;
FLOAT32 sumOfWeights = 0;
SINT16 ptr = 0;
UBYTE8 qR = 0,qG = 0,qB = 0;
FLOAT32   newX = 0.0;
FLOAT32   newY = 0.0;
// ULONG_32 pixelValues = 0;
IplImage* r, * g, * b;


FLOAT32 *weights = new FLOAT32[HISTOGRAM_LENGTH];

for (i=0;i<HISTOGRAM_LENGTH;i++)
{
   if (histogram[i] >0.0 )
    weights[i] = m_sTrackingObjectTable[m_cActiveObject].initHistogram[i]/histogram[i]; //qu/pu(y0)
   else
    weights[i] = 0.0;
}

r = cvCreateImage( cvGetSize(frame), frame->depth, 1 );
g = cvCreateImage( cvGetSize(frame), frame->depth, 1 );
b = cvCreateImage( cvGetSize(frame), frame->depth, 1 );
cvCvtPixToPlane( frame, b, g, r, NULL ); //divide color image into separate planes r, g, b. The exact sequence doesn't matter.

for (y=max(m_sTrackingObjectTable[m_cActiveObject].Y-m_sTrackingObjectTable[m_cActiveObject].H/2,0);y<=min(m_sTrackingObjectTable[m_cActiveObject].Y+m_sTrackingObjectTable[m_cActiveObject].H/2,m_nImageHeight-1);y++)
   for (x=max(m_sTrackingObjectTable[m_cActiveObject].X-m_sTrackingObjectTable[m_cActiveObject].W/2,0);x<=min(m_sTrackingObjectTable[m_cActiveObject].X+m_sTrackingObjectTable[m_cActiveObject].W/2,m_nImageWidth-1);x++)
   {
    E = CheckEdgeExistance(r, g, b,x,y);

    qR = (UBYTE8)pixval8c( r, y, x )/16;
    qG = (UBYTE8)pixval8c( g, y, x )/16;
    qB = (UBYTE8)pixval8c( b, y, x )/16;

    ptr = 4096*E+256*qR+16*qG+qB; //some recalculation here. The bin number of (x, y) can be stroed somewhere in fact.

    newX += (weights[ptr]*x);
    newY += (weights[ptr]*y);

    sumOfWeights += weights[ptr];
   }

   if (sumOfWeights>0)
   {
    m_sTrackingObjectTable[m_cActiveObject].X = SINT16((newX/sumOfWeights) + 0.5); //update location
    m_sTrackingObjectTable[m_cActiveObject].Y = SINT16((newY/sumOfWeights) + 0.5);
   }

cvReleaseImage(&r);
cvReleaseImage(&g);
cvReleaseImage(&b);
   delete[] weights, weights = 0;
}
// Returns the distance between two histograms.
FLOAT32 CObjectTracker::FindDistance(FLOAT32 (*histogram))
{
SINT16 i = 0;
FLOAT32 distance = 0;


for(i=0;i<HISTOGRAM_LENGTH;i++)
   distance += FLOAT32(sqrt(DOUBLE64(m_sTrackingObjectTable[m_cActiveObject].initHistogram[i]
                  *histogram[i])));

return(sqrt(1-distance));
}
//An alternative distance measurement
FLOAT32 CObjectTracker::CompareHistogram(UBYTE8 (*histogram))
{
SINT16 i = 0;
FLOAT32 distance = 0.0;
FLOAT32 difference = 0.0;


for (i=0;i<HISTOGRAM_LENGTH;i++)
{
   difference = FLOAT32(m_sTrackingObjectTable[m_cActiveObject].initHistogram[i]
                         -histogram[i]);

   if (difference>0)
    distance += difference;
   else
    distance -= difference;
}
return(distance);
}
// Returns the edge insformation of a pixel at (x,y), assume a large jump of value around edge pixels
UBYTE8 CObjectTracker::CheckEdgeExistance(IplImage *r, IplImage *g, IplImage *b, SINT16 _x,SINT16 _y)
{
UBYTE8 E = 0;
SINT16 GrayCenter = 0;
SINT16 GrayLeft = 0;
SINT16 GrayRight = 0;
SINT16 GrayUp = 0;
SINT16 GrayDown = 0;
// ULONG_32 pixelValues = 0;

// pixelValues = GetPixelValues(frame,_x,_y);
GrayCenter = SINT16(3*pixval8c( r, _y, _x )+6*pixval8c( g, _y, _x )+pixval8c( b, _y, _x ));

if (_x>0)
{
//   pixelValues = GetPixelValues(frame,_x-1,_y);

   GrayLeft = SINT16(3*pixval8c( r, _y, _x-1 )+6*pixval8c( g, _y, _x-1 )+pixval8c( b, _y, _x-1 ));
}

if (_x < (m_nImageWidth-1))
{
//   pixelValues = GetPixelValues(frame,_x+1,_y);

      GrayRight = SINT16(3*pixval8c( r, _y, _x+1 )+6*pixval8c( g, _y, _x+1 )+pixval8c( b, _y, _x+1 ));
}

if (_y>0)
{
//   pixelValues = GetPixelValues(frame,_x,_y-1);

      GrayUp = SINT16(3*pixval8c( r, _y-1, _x )+6*pixval8c( g, _y-1, _x )+pixval8c( b, _y-1, _x ));
}

if (_y<(m_nImageHeight-1))
{
//   pixelValues = GetPixelValues(frame,_x,_y+1);

   GrayDown = SINT16(3*pixval8c( r, _y+1, _x )+6*pixval8c( g, _y+1, _x )+pixval8c( b, _y+1, _x ));
}

if (abs((GrayCenter-GrayLeft)/10)>EDGE_DETECT_TRESHOLD)
   E = 1;

if (abs((GrayCenter-GrayRight)/10)>EDGE_DETECT_TRESHOLD)
   E = 1;

if (abs((GrayCenter-GrayUp)/10)>EDGE_DETECT_TRESHOLD)
      E = 1;

if (abs((GrayCenter-GrayDown)/10)>EDGE_DETECT_TRESHOLD)
      E = 1;

return(E);
}
// Alpha blending: used to update initial histogram by the current histogram
void CObjectTracker::UpdateInitialHistogram(UBYTE8 (*histogram))
{
SINT16 i = 0;

for (i=0; i<HISTOGRAM_LENGTH; i++)
   m_sTrackingObjectTable[m_cActiveObject].initHistogram[i] = ALPHA*m_sTrackingObjectTable[m_cActiveObject].initHistogram[i]
                                                            +(1-ALPHA)*histogram[i];

}
// Mean-shift iteration
//frame: 图像
//MeanShift迭代找出中心点
void CObjectTracker::FindNextLocation(IplImage *frame)
{
int i, j, opti, optj;
SINT16 scale[3]={-3, 3, 0};
FLOAT32 dist, optdist;
SINT16 h, w, optX, optY;

//try no-scaling
FindNextFixScale(frame);
optdist=LastDist;
optX=m_sTrackingObjectTable[m_cActiveObject].X;
optY=m_sTrackingObjectTable[m_cActiveObject].Y;

//try one of the 9 possible scaling
i=rand()*2/RAND_MAX;
j=rand()*2/RAND_MAX;
h=m_sTrackingObjectTable[m_cActiveObject].H;
w=m_sTrackingObjectTable[m_cActiveObject].W;
if(h+scale[i]>10 && w+scale[j]>10 && h+scale[i]<m_nImageHeight/2 && w+scale[j]<m_nImageWidth/2)
{
   m_sTrackingObjectTable[m_cActiveObject].H=h+2*scale[i];
   m_sTrackingObjectTable[m_cActiveObject].W=w+2*scale[j];
   FindNextFixScale(frame);
   if( (dist=LastDist) < optdist ) //scaling is better
   {
    optdist=dist;
//    printf("Next%f->/n", dist);
   }
   else //no scaling is better
   {
    m_sTrackingObjectTable[m_cActiveObject].X=optX;
    m_sTrackingObjectTable[m_cActiveObject].Y=optY;
    m_sTrackingObjectTable[m_cActiveObject].H=h;
    m_sTrackingObjectTable[m_cActiveObject].W=w;
   }
};
TotalDist+=optdist; //the latest distance
// printf("/n");
}

void CObjectTracker::FindNextFixScale(IplImage *frame)
{
UBYTE8 iteration = 0;
SINT16 optX, optY;

FLOAT32 *currentHistogram = new FLOAT32[HISTOGRAM_LENGTH];
FLOAT32 dist, optdist=1.0;

for (iteration=0; iteration<MEANSHIFT_ITARATION_NO; iteration++)
{
   FindHistogram(frame,currentHistogram); //current frame histogram, use the last frame location as starting point
  
      FindWightsAndCOM(frame,currentHistogram);//derive weights and new location
  
      //FindHistogram(frame,currentHistogram);   //uptade histogram
  
      //UpdateInitialHistogram(currentHistogram);//uptade initial histogram
   if( ((dist=FindDistance(currentHistogram)) < optdist) || iteration==0 )
   {
    optdist=dist;
    optX=m_sTrackingObjectTable[m_cActiveObject].X;
    optY=m_sTrackingObjectTable[m_cActiveObject].Y;
//      printf("%f->", dist);
   }
   else //bad iteration, then find a better start point for next iteration
   {
   m_sTrackingObjectTable[m_cActiveObject].X=(m_sTrackingObjectTable[m_cActiveObject].X+optX)/2;
   m_sTrackingObjectTable[m_cActiveObject].Y=(m_sTrackingObjectTable[m_cActiveObject].Y+optY)/2;
   }
}//end for
m_sTrackingObjectTable[m_cActiveObject].X=optX;
m_sTrackingObjectTable[m_cActiveObject].Y=optY;
LastDist=optdist; //the latest distance
// printf("/n");

delete[] currentHistogram, currentHistogram = 0;
}

float CObjectTracker::GetTotalDist(void)
{
return(TotalDist);
}