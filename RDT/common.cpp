//dongliang 20130405
#include "common.h"
bool hasEnding (std::string const &fullString, std::string const &ending)
{
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

bool hasEndingLower (string const &fullString_, string const &_ending)
{
	string fullstring = fullString_, ending = _ending;
	transform(fullString_.begin(),fullString_.end(),fullstring.begin(),::tolower); // to lower
	return hasEnding(fullstring,ending);
}

void open_imgs_dir(char* dir_name, std::vector<cv::Mat>& images, std::vector<std::string>& images_names, double downscale_factor) {
	if (dir_name == NULL) {
		return;
	}

	string dir_name_ = string(dir_name);
	vector<string> files_;

#ifndef WIN32
//open a directory the POSIX way

	DIR *dp;
	struct dirent *ep;     
	dp = opendir (dir_name);
	
	if (dp != NULL)
	{
		while (ep = readdir (dp)) {
			if (ep->d_name[0] != '.')
				files_.push_back(ep->d_name);
		}
		
		(void) closedir (dp);
	}
	else {
		cerr << ("Couldn't open the directory");
		return;
	}

#else
//open a directory the WIN32 way
	HANDLE hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA fdata;

	if(dir_name_[dir_name_.size()-1] == '\\' || dir_name_[dir_name_.size()-1] == '/') {
		dir_name_ = dir_name_.substr(0,dir_name_.size()-1);
	}

	hFind = FindFirstFile(string(dir_name_).append("\\*").c_str(), &fdata);	
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (strcmp(fdata.cFileName, ".") != 0 &&
				strcmp(fdata.cFileName, "..") != 0)
			{
				if (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					continue; // a diretory
				}
				else
				{
					files_.push_back(fdata.cFileName);
				}
			}
		}
		while (FindNextFile(hFind, &fdata) != 0);
	} else {
		cerr << "can't open directory\n";
		return;
	}

	if (GetLastError() != ERROR_NO_MORE_FILES)
	{
		FindClose(hFind);
		cerr << "some other error with opening directory: " << GetLastError() << endl;
		return;
	}

	FindClose(hFind);
	hFind = INVALID_HANDLE_VALUE;
#endif
	
	for (unsigned int i=0; i<files_.size(); i++) {
		if (files_[i][0] == '.' || !(hasEndingLower(files_[i],"jpg")||hasEndingLower(files_[i],"png")||hasEndingLower(files_[i],"bmp"))) {
			continue;
		}
		cv::Mat m_ = imread(string(dir_name_).append("/").append(files_[i]),-1);
		if(downscale_factor != 1.0)
			cv::resize(m_,m_,Size(),downscale_factor,downscale_factor);
		images_names.push_back(files_[i]);
		images.push_back(m_);
	}
		

}

void saveData(const char* filename, const Mat& mat, int flag = 0)      
{
   FILE* fp = fopen(filename, "wt");
   if (3 != flag)
   {
      fprintf(fp, "%02d\n", mat.rows);
      fprintf(fp, "%02d\n", mat.cols);
   }
   switch (flag)
   {
   case 0:
      for(int y = 0; y < mat.rows; y++)
      {
         for(int x = 0; x < mat.cols; x++)
         {
            short depth = mat.at<short>(y, x);   
            fprintf(fp, "%d\n", depth);
         }
      }
      break;
   case 1:
      for(int y = 0; y < mat.rows; y++)
      {
         for(int x = 0; x < mat.cols; x++)
         {
            uchar disp = mat.at<uchar>(y,x);
            fprintf(fp, "%d\n", disp);
         }
      }
      break;
   case 2:
      for(int y = 0; y < mat.rows; y++)
      {
         for(int x = 0; x < mat.cols; x++)
         {
            float disp = mat.at<float>(y,x);
            fprintf(fp, "%10.4f\n", disp);
         }
      }
      break;
   case 3:
      for(int y = 0; y < mat.rows; y++)
      {
         for(int x = 0; x < mat.cols; x++)
         {
            Vec3f point = mat.at<Vec3f>(y, x);   // Vec3f 是 template 类定义
            fprintf(fp, "%f %f %f\n", point[0], point[1], point[2]);
         }
      }
      break;
   case 4:
      imwrite(filename, mat);
      break;
   default:
      break;
   }

   fclose(fp);
}

bool saveSamples(string filename,Mat& trainsamples, Mat& trainlabels) //保存特征向量
{
    FileStorage fs2(filename,FileStorage::WRITE);
	fs2<<"trainsamples"<<trainsamples;
	fs2<<"trainlabels"<<trainlabels;
	fs2.release();
	return 1;
}

//region grow
bool search(unsigned int x,unsigned int y, Mat& image,Rect& roi,vector<Point>& searchPoints,unsigned short threshold,unsigned short globalT)
{
	int num=0;
	int minW,maxW,minH,maxH;
	int width=image.cols,height=image.rows;
	Point pointS;
	searchPoints.push_back(Point(x,y));
	int origin=int(image.at<uchar>(x,y));
	cout<<x<<' '<<y<<' '<<origin<<endl;
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
		if(!visit[(pointS.x-1)*width+pointS.y]&&pointS.x-1>=0&&abs(int(image.at<uchar>(pointS.x-1,pointS.y))-int(image.at<uchar>(pointS.x,pointS.y)))<threshold&&abs(int(image.at<uchar>(pointS.x-1,pointS.y))-origin)<globalT)
		{
			searchPoints.push_back(Point(pointS.x-1,pointS.y));
			visit[(pointS.x-1)*width+pointS.y]=true;
		}
		if(!visit[(pointS.x+1)*width+pointS.y]&&pointS.x+1<width&&abs(int(image.at<uchar>(pointS.x+1,pointS.y))-int(image.at<uchar>(pointS.x,pointS.y)))<threshold&&abs(int(image.at<uchar>(pointS.x+1,pointS.y))-origin)<globalT)
		{
			searchPoints.push_back(Point(pointS.x+1,pointS.y));
			visit[(pointS.x+1)*width+pointS.y]=true;
		}
		if(!visit[pointS.x*width+pointS.y-1]&&pointS.y-1>=0&&abs(int(image.at<uchar>(pointS.x,pointS.y-1))-int(image.at<uchar>(pointS.x,pointS.y)))<threshold&&abs(int(image.at<uchar>(pointS.x,pointS.y-1))-origin)<globalT)
		{
			searchPoints.push_back(Point(pointS.x,pointS.y-1));
			visit[pointS.x*width+pointS.y-1]=true;
		}
		if(!visit[pointS.x*width+pointS.y+1]&&pointS.y+1<height&&abs(int(image.at<uchar>(pointS.x,pointS.y+1))-int(image.at<uchar>(pointS.x,pointS.y)))<threshold&&abs(int(image.at<uchar>(pointS.x,pointS.y+1))-origin)<globalT)
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

