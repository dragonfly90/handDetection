#include"classification.h"
bool computeFeatures(const cv::Mat& image,cv::Mat& features,const int& dimension)
{
	float* fptr=features.ptr<float>(0);
	//uchar level=ostu(image);
	int i=0;
	
   
	//uchar handDepth=image.at<uchar>(image.rows/2,image.cols/2);/*ostu(image);*/
	//Mat handMat = (image<handDepth+BIN_THRESH_OFFSET)&image>400;//(image > (handDepth - BIN_THRESH_OFFSET)) & (image < (handDepth + BIN_THRESH_OFFSET));
	namedWindow("Hand Threshold",CV_WINDOW_AUTOSIZE);
	imshow("Hand Threshold",image);
	waitKey(5);
	int step=std::min(image.rows,image.cols)/(sqrtf(dimension));
	for(int j=0;j<min(image.rows,image.cols);j+=step)
	{
		for(int k=0;k<min(image.rows,image.cols);k+=step)
		{
			int num=0;
			for(int m=j;m<j+step;m++)
				for(int n=k;n<k+step;n++)
					if(image.at<uchar>(m,n))num++;
			if(i>=dimension)return true;
			fptr[i++]=float(num)/(step*step);
			//std::cout<<i<<std::endl;
		}
	}
	return false;
}

uchar ostu(const Mat& image){
	uchar level;
	int hei=image.rows;
	int wid=image.cols;
	long N = hei * wid;
	int h[256];
	double p[256],u[256],w[256];
	int i,j,k;
	const uchar* ptr;
	for(i = 0; i < 256; i++)
	{
		h[i] = 0;
		p[i] = 0;
		u[i] = 0;
		w[i] = 0;
	}

	for(i = 0; i < hei; i++)
	{
		ptr=image.ptr<uchar>(i);
		for(j = 0; j < wid; j++)
		{
			for(int k = 0; k < 256; k++)
			{
				if(ptr[j]== k)
				{
					h[k]++;
				}
			}
		}
	}

	for(i = 0; i < 256; i++)
		p[i] = h[i] / double(N);

	int T = 0;
	double uT,thegma2fang;
	double thegma2fang_max = -10000;
	for(k = 0; k < 256; k++)
	{
		uT = 0;
		for(i = 0; i <= k; i++)
		{
			u[k] += i*p[i];
			w[k] += p[i];
		}
		for(i = 0; i < 256; i++)
			uT += i*p[i];

		thegma2fang = (uT*w[k] - u[k])*(uT*w[k] - u[k]) / (w[k]*(1-w[k]));
		if(thegma2fang > thegma2fang_max)
      {
          thegma2fang_max = thegma2fang;
          level = k;
       }
	}
	return level;
}

int classifylabel(const Mat& features,const CvSVM& svmClassifier)
{
	 return svmClassifier.predict(features);
}



pair<int, int> searchNearestPixel(const Mat &depth, Rect &region) {
	pair<int, int> pt;
	pt.first = -1;
	pt.second = -1;
	const unsigned short *depthptr;
	unsigned short min = (1<<15);
	for(int i=region.y; i<region.y+region.height; i++) {
		depthptr = depth.ptr<const unsigned short>(i);
		for(int j=region.x; j<region.x+region.width; j++) {
			if(depthptr[j] > SENSOR_MIN && depthptr[j] < SENSOR_MAX && depthptr[j] < min) {
				min = depthptr[j];
				pt.first = i;
				pt.second = j;
			}
		}
	}
	return pt;
}

void processNeighbor(int &pixelcount, double &mean, cv::Mat &mask, const short first, const short second, const cv::Mat &depth,queue<pair<int, int> >& _pixels)
{
	unsigned short d = depth.at<unsigned short>(first,second );

	if ( mask.at<uchar>(first,second ) == EMPTY &&
		fabs(d-mean/pixelcount) < _depthThr && d > SENSOR_MIN && d <= SENSOR_MAX)
	{
		pixelcount++;
		mean += d;
		mask.at<uchar>(first,second ) = HAND;
		_pixels.push(pair<int, int>(first,second));
	}
}

void segmentHand(cv::Mat &mask, Rect &region, const cv::Mat &depth)
{
	CV_Assert(mask.type() == CV_8UC1);
	CV_Assert(depth.type() == CV_16UC1);

	CV_Assert(mask.rows == depth.rows);
	CV_Assert(mask.cols == depth.cols);
	queue<pair<int, int> > _pixels;

	mask.setTo(EMPTY);

	pair<int, int> current = searchNearestPixel(depth, region);
	if (current.first < 0){
		//region.isIni = false;
		return;
	}

	int rowcount = depth.rows, colcount = depth.cols;


	double mean = depth.at<unsigned short>(current.first,current.second);
	int minx=depth.cols,miny=depth.rows,maxx=0,maxy=0,minz = (1<<15),maxz = 0;
	unsigned short dv = 0;
	int depthMinDiff = 50;
	int pixelcount = 1;
	_pixels.push(current);

	while((!_pixels.empty()) & (pixelcount < _maxObjectSize))
	{
		current = _pixels.front();
		_pixels.pop();

		dv = depth.at<unsigned short>(current.first,current.second);

		if (current.first < minx) minx = current.first;
		else if (current.first > maxx) maxx = current.first;
		if (current.second < miny) miny = current.second;
		else if (current.second > maxy) maxy = current.second;
		//if (dv < minz) minz = dv;
		//        else if (dv > maxz) maxz = dv;

		if ( current.first + 1 < rowcount ){
			processNeighbor(pixelcount,mean,mask,current.first + 1,current.second,depth,_pixels);
		}

		if ( current.first - 1 > -1 ){
			processNeighbor(pixelcount,mean,mask,current.first - 1,current.second,depth,_pixels);
		}

		if ( current.second + 1 < colcount ){
			processNeighbor(pixelcount,mean,mask,current.first,current.second + 1,depth,_pixels);
		}

		if( current.second - 1 > -1 ){
			processNeighbor(pixelcount,mean,mask,current.first,current.second - 1,depth,_pixels);
		}

	}
	//region.width = maxy - miny; //cols range
	//region.height = maxx - minx; //rows range
}