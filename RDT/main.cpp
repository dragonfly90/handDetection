#include<opencv.hpp>
#include<iostream>
#include"depthImageFeatures.h"
#include"common.h"
#include"time.h"
using namespace std;
using namespace cv;
vector<Mat> images;
vector<string> imagesName;
CvRTrees RDT;
float features[featureDim];
Point *hPoint;
Rect roi;
vector<Point> searchPoints;
vector<vector<float>> depthCpfeatures; 
vector<int> labels;
bool left_mouse_down_=false;
clock_t start,endt;
//CvFileStorage* fsRDT;
Mat depthColor;
Vec3b green(0,255,0), blue (255,0,0),red(0,0,255);

void on_mouse(int event,int x,int y,int flag,void* param) {
	hPoint = (Point*) param;
	switch( event ){
		
	case CV_EVENT_MOUSEMOVE:
         *hPoint=Point(x,y);
         break;
	case CV_EVENT_LBUTTONDOWN:
         left_mouse_down_=true;
         break;
    case CV_EVENT_LBUTTONUP:
         left_mouse_down_=false;
         break;
	}

}

bool wait_for_click()
{
  cout<<"Waiting for click..."<<endl;

  bool got_click=false;
  bool last_mouse_down=left_mouse_down_;
  //Point click;
  while(true)
  {
    if (left_mouse_down_ && last_mouse_down!=left_mouse_down_)
    {
      //click=*hPoint;
      cout<<"Got click at ("<<hPoint->x<<", "<<hPoint->y<<")"<<endl;
      return true;     
    }

    int wk=cvWaitKey(10);
    if (wk==13)
    {
		return false;
    }
  }
}


int main()
{
	char filename[255];
	FileStorage fs;
	Mat depth,depthshow(480,640,CV_8UC1);
	float scaleFactor = 255./4096.;
	Point depthP;
	namedWindow("Depth",1);


	//train
	start=clock();
	for(int m=1;m<10;m++)
	{
	sprintf(filename,"F://test//photo_20130527_m//%d.xml",m);
	fs.open(filename,FileStorage::READ);
	fs["depth"]>>depth;
	fs.release();
	if(!depth.data)break;
	depth.convertTo(depthshow,CV_8UC1,scaleFactor);
	cvtColor(depthshow,depthColor,CV_GRAY2RGB);
	imshow("Depth",depthshow);
	
	cvSetMouseCallback( "Depth", on_mouse, (void*) &depthP );
 	//waitKey();
	if(wait_for_click())
	{
	search(depthP.y,depthP.x,depthshow,roi,searchPoints,8,13);
	
	//features.resize(featureDim);
	labels.assign(depthshow.rows*depthshow.cols,-1);
	for(int i=0;i<depthshow.rows;i++)
		for(int j=0;j<depthshow.cols;j++)
		{
         vector<float> features(featureDim,0);
		 //cout<<i<<','<<j<<endl;
	     computerDIF(Point(j,i),depth, features,featuresSize,featureDim);
		 depthCpfeatures.push_back(features);
		}
	for(int i=0;i<searchPoints.size();i++)
	{
		labels[searchPoints[i].x*depthshow.cols+searchPoints[i].y]=1;
		depthColor.at<Vec3b>(searchPoints[i].x,searchPoints[i].y)=red;//Scalar(255,0,0);
	}
	//waitKey();
	}
	}
	endt=clock();
	sprintf(filename,"F://test//photo_20130527_m//originT%d.jpg",3);
	imwrite(filename,depthColor);

	cout<<"train time: "<<(endt-start)/1000<<"s"<<endl;



	Mat depthCmpM(depthCpfeatures.size(),featureDim,CV_32FC1);
	Mat trainlabelsMat;
	for(int i=0;i<depthCpfeatures.size();i++)
	{
		float* ptr=depthCmpM.ptr<float>(i);
		for(int j=0;j<featureDim;j++)
			ptr[j]=depthCpfeatures[i][j];
	}

	Mat(labels).copyTo(trainlabelsMat);
	fs.open("F://test//photo_20130527_m//3depths.xml",FileStorage::WRITE);
	fs<<"depthF"<<depthCmpM;
	fs<<"depthL"<<trainlabelsMat;
	fs.release();

	
	//fs.open("F://test//photo_20130527_m//0depthL.xml",FileStorage::WRITE);
	//fs<<"depthLF"<<depthCmpM;
	//fs.release();

	//predict
	/*CvBoost  boost;

    Mat var_types( 1, depthCmpM.cols + 1, CV_8UC1, Scalar(CV_VAR_ORDERED) );
    var_types.at<uchar>( depthCmpM.cols ) = CV_VAR_CATEGORICAL;

    CvBoostParams  params( CvBoost::DISCRETE, // boost_type
                           100, // weak_count
                           0.95, // weight_trim_rate
                           2, // max_depth
                           false, //use_surrogates
                           0 // priors
                         );

    boost.train( depthCmpM, CV_ROW_SAMPLE, trainlabelsMat, Mat(), Mat(), var_types, Mat(), params );*/

	//CvNormalBayesClassifier normalBayesClassifier( depthCmpM, trainlabelsMat );
	
	CvRTrees  rtrees;
	CvRTParams  params( 20, // max_depth,
				3, // min_sample_count,
				0.f, // regression_accuracy,
				false, // use_surrogates,
				16, // max_categories,
				0, // priors,
				false, // calc_var_importance,
				1, // nactive_vars,
				40, // max_num_of_trees_in_the_forest,
				0.01, // forest_accuracy,
				CV_TERMCRIT_ITER// termcrit_type
				);
	rtrees.train(depthCmpM, CV_ROW_SAMPLE, trainlabelsMat, Mat(), Mat(), Mat(), Mat(), params );
	//normalBayesClassifier.save("normalBayesClassifier.xml");

	start=clock();
	for(int i=0;i<81;i++)
	{
	sprintf(filename,"F://test//photo_20130527_m//%d.xml",i);
	fs.open(filename,FileStorage::READ);
	fs["depth"]>>depth;
	fs.release();
	if(!depth.data)return 0;
    depth.convertTo(depthshow,CV_8UC1,scaleFactor);
	cvtColor(depthshow,depthColor,CV_GRAY2RGB);
	//Mat depthRDF=Mat::zeros(depth.size(),CV_8UC1);
	for(int i=0;i<depthshow.rows;i++)
	{
		Vec3b* ptr=depthColor.ptr<Vec3b>(i);
		for(int j=0;j<depthshow.cols;j++)
		{
		 //cout<<i<<','<<j<<endl;
	     //Mat testSample( 1, 2, CV_32FC1 );
		 vector<float> features(featureDim,0);
	     computerDIF(Point(j,i),depth, features,featuresSize,featureDim);
		 Mat featuresMat(features);
		 int target= rtrees.predict(featuresMat.t());
		 if(target==1)
			 ptr[j]=red;
		}
	}
	imshow("Depth",depthshow);
    imshow("RDF",depthColor);
	sprintf(filename,"%d.jpg",i);
	imwrite(filename,depthColor);
	waitKey(1);
	}
	endt=clock();
	cout<<"predict time: "<<(endt-start)/1000/10<<"s"<<endl;
}
