#include"crossValidation.h"

void cross_validation(Mat& trainsamples, Mat& trainlabels,int nr_class,int nr_fold, int* perm,int* fold_start)
{
	int i;
	int l = trainsamples.rows;
  
	// stratified cv may not give leave-one-out rate
	// Each class to l folds -> some folds may have zero elements
	if( nr_fold < l)
	{
		int *start = NULL;
		int *label = NULL;
		int *count = NULL;
		group_classes(trainsamples,trainlabels,nr_class,&label,&start,&count,perm);

		// random shuffle and then data grouped by fold using the array perm
		int *fold_count =  Malloc(int,nr_fold);
		int c;
		int *index =Malloc(int,l);
		for(i=0;i<l;i++)
			index[i]=perm[i];
		for (c=0; c<nr_class; c++) 
			for(i=0;i<count[c];i++)
			{
				int j = i+rand()%(count[c]-i);
				swap(index[start[c]+j],index[start[c]+i]);
			}
		for(i=0;i<nr_fold;i++)
		{
			fold_count[i] = 0;
			for (c=0; c<nr_class;c++)
				fold_count[i]+=(i+1)*count[c]/nr_fold-i*count[c]/nr_fold;
		}
		fold_start[0]=0;
		for (i=1;i<=nr_fold;i++)
			fold_start[i] = fold_start[i-1]+fold_count[i-1];
		for (c=0; c<nr_class;c++)
			for(i=0;i<nr_fold;i++)
			{
				int begin = start[c]+i*count[c]/nr_fold;
				int end = start[c]+(i+1)*count[c]/nr_fold;
				for(int j=begin;j<end;j++)
				{
					perm[fold_start[i]] = index[j];
					fold_start[i]++;
				}
			}
		fold_start[0]=0;
		for (i=1;i<=nr_fold;i++)
		{
			fold_start[i] = fold_start[i-1]+fold_count[i-1];
		}
		free(start);	
		free(label);
		free(count);	
		free(index);
		free(fold_count);
	}
	else
	{
		for(i=0;i<l;i++) perm[i]=i;
		for(i=0;i<l;i++)
		{
			int j = i+rand()%(l-i);
			swap(perm[i],perm[j]);
		}
		for(i=0;i<=nr_fold;i++)
			fold_start[i]=i*l/nr_fold;
	}
}

// label: label name, start: begin of each class, count: #data of classes, perm: indices to the original data
// perm, length l, must be allocated before calling this subroutine
 static void group_classes(Mat& trainsamples, Mat& trainlabels, int& nr_class_ret,int **label_ret, int **start_ret, int **count_ret, int *perm)
{
	int l = trainsamples.rows;
	int max_nr_class = 16;
	int nr_class = 0;
	int *label =Malloc(int,max_nr_class);
	int *count =Malloc(int,max_nr_class);
	int *data_label =  Malloc(int,l);	
	int i;

	for(i=0;i<l;i++)
	{
		int this_label = (int)trainlabels.at<float>(i);
		int j;
		for(j=0;j<nr_class;j++)
		{
			if(this_label == label[j])
			{
				++count[j];
				break;
			}
		}
		data_label[i] = j;
		if(j == nr_class)
		{
			if(nr_class == max_nr_class)
			{
				max_nr_class *= 2;
				label = (int *)realloc(label,max_nr_class*sizeof(int));
				count = (int *)realloc(count,max_nr_class*sizeof(int));
			}
			label[nr_class] = this_label;
			count[nr_class] = 1;
			++nr_class;
		}
	}

	int *start = Malloc(int,nr_class);
	start[0] = 0;
	for(i=1;i<nr_class;i++)
		start[i] = start[i-1]+count[i-1];
	for(i=0;i<l;i++)
	{
		perm[start[data_label[i]]] = i;
		++start[data_label[i]];
	}
	start[0] = 0;
	for(i=1;i<nr_class;i++)
		start[i] = start[i-1]+count[i-1];

	nr_class_ret = nr_class;
	*label_ret = label;
	*start_ret = start;
	*count_ret = count;
	free(data_label);
}
