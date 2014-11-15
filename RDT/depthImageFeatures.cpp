//depth compare features
#include "depthImageFeatures.h"
bool computerDIF(const cv::Point& center,const cv::Mat& images, std::vector<float>& features,cv::Size featuresSize,int featuresDim)
{
	int width=featuresSize.width;
	int height=featuresSize.height;
	//if(width%2==0||height%2==0)return;
	int startR=center.y-height/2;
	int startC=center.x-width/2;
	if((startR<0)||(startC<0)||(startR+height)>images.rows||(startC+width)>images.cols)return false;
	int featuresNum=0;
	double bias=images.at<short>(center.y,center.x);
	for(int i=0;i<width;i+=10)
		for(int j=0;j<height;j+=10)
			//for(int m=i+5;m<width;m+=5)
				//for(int n=j+5;n<height;n+=5)
				//{
				//	if((m*height/5+n)>(i*height/5+j))
					{
					double dep1=depth(images,startC+i,startR+j);
					//double dep2=depth(images,startC+m,startR+n);
					if(dep1>400)
					{
					double col1=startC+depthScale*i/dep1;//depthScale*i/dep1;
					double row1=startR+depthScale*j/dep1;//depthScale*j/dep1;
					//double col2=startC+depthScale*m/dep2;
					//double row2=startR+depthScale*n/dep2;
					//features[featuresNum++]=depth(images,col2,row2)-depth(images,col1,row1);
					features[featuresNum++]=depth(images,col1,row1)-bias;
					}
					else 
						features[featuresNum++]=0;
					}
				//}
	//std::cout<<featuresNum<<std::endl;
	return true;
}

float depth(const cv::Mat& images,double x,double y)
{
	// �ĸ����ٽ����ص�����(i1, j1), (i2, j1), (i1, j2), (i2, j2)
    LONG    i1, i2;
    LONG    j1, j2;
     
    float    f1, f2, f3, f4;    // �ĸ����ٽ�����ֵ   
    float    f12, f34;        // ������ֵ�м�ֵ   
 
    // ����һ��ֵ���������������С�ڸ�ֵʱ��Ϊ������ͬ
    FLOAT   EXP;   
    
	LONG lHeight=images.rows;
	LONG lWidth=images.cols;

    //LONG lLineBytes;                
    //lLineBytes = WIDTHBYTES(lWidth * 8);
     
    EXP = (FLOAT) 0.0001;
     
    // �����ĸ����ٽ����ص�����
    i1 = (LONG) x;
    i2 = i1 + 1;
    j1 = (LONG) y;
    j2 = j1 + 1;
     
    // ���ݲ�ͬ����ֱ���
    if( (x < 0) || (x > lWidth - 1) || (y < 0) || (y > lHeight - 1))
    {       
        return 0;        // Ҫ����ĵ㲻��Դͼ��Χ�ڣ�ֱ�ӷ���255��
    }
    else
    {
        if (fabs(x - lWidth + 1) <= EXP)
        {
            // Ҫ����ĵ���ͼ���ұ�Ե��
            if (fabs(y - lHeight + 1) <= EXP)
            {
                // Ҫ����ĵ�������ͼ�������½���һ�����أ�ֱ�ӷ��ظõ�����ֵ
                f1 = images.at<short>(j1,i1);
					//*((unsigned char *)lpDIBBits + lLineBytes *
                    //(lHeight - 1 - j1) + i1);
                return f1;
            }
            else
            {
                // ��ͼ���ұ�Ե���Ҳ������һ�㣬ֱ��һ�β�ֵ����
                f1 =images.at<short>(j1,i1);
					//*((unsigned char *)lpDIBBits + lLineBytes *
                    //(lHeight - 1 - j1) + i1);
                f3 = images.at<short>(j1,i2);
					//*((unsigned char *)lpDIBBits + lLineBytes *
                    //(lHeight - 1 - j1) + i2);
                 
                // ���ز�ֵ���
                return ((f1 + (y -j1) * (f3 - f1)));
            }
        }
        else if (fabs(y - lHeight + 1) <= EXP)
        {
            // Ҫ����ĵ���ͼ���±�Ե���Ҳ������һ�㣬ֱ��һ�β�ֵ����
            f1 =images.at<short>(j1,i1);
				//*((unsigned char*)lpDIBBits + lLineBytes * (lHeight - 1 - j1) + i1);
            f2 =images.at<short>(j2,i1);
			//		*((unsigned char*)lpDIBBits + lLineBytes * (lHeight - 1 - j2) + i1);
             
            // ���ز�ֵ���
            return ((f1 + (x -i1) * (f2 - f1)));
        }
        else
        {
            // �����ĸ����ٽ�����ֵ
            f1 = images.at<short>(j1,i1);
				//*((unsigned char*)lpDIBBits + lLineBytes * (lHeight - 1 - j1) + i1);
            f2 = images.at<short>(j2,i1);
				//*((unsigned char*)lpDIBBits + lLineBytes * (lHeight - 1 - j2) + i1);
            f3 = images.at<short>(j1,i2);
				//*((unsigned char*)lpDIBBits + lLineBytes * (lHeight - 1 - j1) + i2);
            f4 = images.at<short>(j2,i2);
				//*((unsigned char*)lpDIBBits + lLineBytes * (lHeight - 1 - j2) + i2);
             
            // ��ֵ1
            f12 =(f1 + (x - i1) * (f2 - f1));           
            // ��ֵ2
            f34 =(f3 + (x - i1) * (f4 - f3));           
            // ��ֵ3
            return ((f12 + (y -j1) * (f34 - f12)));
        }
    }
}