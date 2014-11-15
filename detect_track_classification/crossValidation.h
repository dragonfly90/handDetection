#ifndef CROSS_VALIDATION
#define CROSS_VALIDATION
#include "opencv2/core/core.hpp"
#include "opencv2/ml/ml.hpp"
using namespace std;
using namespace cv;

#define Malloc(type,n) (type *)malloc((n)*sizeof(type))
void cross_validation(Mat& trainsamples, Mat& trainlabels,int nr_class,int nr_fold, int* perm,int* fold_start);
static void group_classes(Mat& trainsamples, Mat& trainlabels, int& nr_class_ret,int **label_ret, int **start_ret, int **count_ret, int *perm);
#endif