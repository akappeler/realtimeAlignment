/***********************************************************************************
 Ordinal Feature 
 
 Created:
 TCL Research America
 Armin Kappeler, 09/18/2013

 Modification:
 
 ************************************************************************************/
#include "TCL_UView_Ordinal.h"

using namespace cv;
using namespace std;

TCL_UView_OrdinalFeature::TCL_UView_OrdinalFeature(void)
{
	hbins = 16; 
	vbins = 8; 
	method = 1;	
}

TCL_UView_OrdinalFeature::TCL_UView_OrdinalFeature(int hb, int sb)
{
	hbins = hb;
	vbins = sb;
	method = 1;
}

TCL_UView_OrdinalFeature::TCL_UView_OrdinalFeature(FeatureSettings values)
{
	hbins = values.Bins1;
	vbins = values.Bins2;
	method = values.featureType;
}

TCL_UView_OrdinalFeature::~TCL_UView_OrdinalFeature(void)
{
}

Mat TCL_UView_OrdinalFeature::getFeatures(list<Mat> clip)
{
	Mat all_hist(hbins*vbins,(int)clip.size(),CV_32FC1); //float

	// main loop for clip frames
	int i=0;
	for (list<Mat>::iterator it = clip.begin(); it != clip.end(); it++)
	{
		Mat tmp = calculateOrdinal(*it).t();	
		tmp.copyTo(all_hist.col(i));
		i++;
	}

	return all_hist;
}

Mat TCL_UView_OrdinalFeature::getFeature(cv::Mat img)
{
	switch (method)
	{
	case 1:
		return calculateOrdinal(img);
		break;
	case 3:
		return calculateOrdinal3(img);
		break;
	case 4:
		return calculateOrdinal4(img); //bad results
		break;
	case 5:
		return calculateOrdinal5(img);
		break;
	case 6:
		return calculateOrdinal6(img);
		break;
	case 7:
		return calculateOrdinal7(img);
		break;
	case 8:
		return calculateOrdinal8(img);
		break;
	case 9:
		return calculateOrdinal9(img);
		break;
	default:
		return calculateOrdinal(img);
		break;
	}
}

int TCL_UView_OrdinalFeature::getFeatureSize(void)
{
	if (method==1 || method==3)
		return hbins*vbins;
	else
		return hbins*vbins*3;

}

//gray ordinal
Mat TCL_UView_OrdinalFeature::calculateOrdinal(Mat src)
{
	Mat imGray;
	Mat imSmall;
	Mat imSmallVector;
	Mat featSortIdx;
	Mat featSortIdxFloat;
	Mat scaledFeatSortIdx;
	Size dims(hbins,vbins);
	float normFactor = 2.0f/(hbins*hbins*vbins*vbins); //sum of all values = 1 

	//convert to grayscale and downsample to blocks
	cvtColor(src, imGray,CV_RGB2GRAY );
	resize(imGray, imSmall, dims, 0, 0, INTER_AREA); 

	//sort blocks
	imSmallVector = imSmall.reshape(0,1);
	sortIdx(imSmallVector,featSortIdx,CV_SORT_EVERY_ROW + CV_SORT_ASCENDING );
	//convertTo(Mat& m, int rtype, double alpha=1, double beta=0)
	featSortIdx.convertTo(featSortIdxFloat,CV_32FC1,normFactor);

	return featSortIdxFloat;
	
}

//gray - minmax
Mat TCL_UView_OrdinalFeature::calculateOrdinal3(Mat src)
{
	Mat imGray;
	Mat imSmall;
	Mat imSmallVector;
	Mat featSortIdx;
	Mat featSortIdxFloat;
	Mat scaledFeatSortIdx;

	Mat imSmallVectorFloat;

	Size dims(hbins,vbins);
	float normFactor = 2.0f/getFeatureSize(); //average sum of all values = 1 

	//convert to grayscale and downsample to blocks
	cvtColor(src, imGray,CV_RGB2GRAY );
	resize(imGray, imSmall, dims, 0, 0, INTER_AREA); 

	//sort blocks
	imSmallVector = imSmall.reshape(0,1);

	
	imSmallVector.convertTo(imSmallVectorFloat,CV_32FC1);
	normalize(imSmallVectorFloat,featSortIdxFloat,normFactor,0,NORM_MINMAX); //normfactor instead of 1

	return featSortIdxFloat;
	
}

//color ordinal
Mat TCL_UView_OrdinalFeature::calculateOrdinal4(Mat src)
{
	Mat imGray;
	Mat imSmall;
	Mat imSmallVector;
	Mat featSortIdx;
	Mat featSortIdxFloat;
	Mat scaledFeatSortIdx;
	Size dims(hbins,vbins);
	float normFactor = 2.0f/(hbins*hbins*vbins*vbins*3); //sum of all values = 1 

	//convert to grayscale and downsample to blocks
	//cvtColor(src, imGray,CV_RGB2GRAY );
	resize(src, imSmall, dims, 0, 0, INTER_AREA); 

	//sort blocks
	imSmallVector = imSmall.reshape(1,1);
	sortIdx(imSmallVector,featSortIdx,CV_SORT_EVERY_ROW + CV_SORT_ASCENDING );
	//convertTo(Mat& m, int rtype, double alpha=1, double beta=0)
	featSortIdx.convertTo(featSortIdxFloat,CV_32FC1,normFactor); 

	return featSortIdxFloat;
	
}

//color MinMax
Mat TCL_UView_OrdinalFeature::calculateOrdinal5(Mat src) //color, not sorted, normalized
{
	Mat imGray;
	Mat imSmall;
	Mat imSmallVector;
	Mat featSortIdx;
	Mat featSortIdxFloat;
	Mat scaledFeatSortIdx;
	Size dims(hbins,vbins);
	Mat imSmallVectorFloat;
	float normFactor = 2.0f/getFeatureSize(); //average sum of all values = 1 

	//convert to grayscale and downsample to blocks
	//cvtColor(src, imGray,CV_RGB2GRAY );
	resize(src, imSmall, dims, 0, 0, INTER_AREA); 

	//Mat testimSmall(imSmall);

	imSmallVector = imSmall.reshape(1,1);
	imSmallVector.convertTo(imSmallVectorFloat,CV_32FC1);
	normalize(imSmallVectorFloat,featSortIdxFloat,normFactor,0,NORM_MINMAX);

	return featSortIdxFloat;
	
}

//color Ordinal separate
Mat TCL_UView_OrdinalFeature::calculateOrdinal6(Mat src)
{
	Mat imGray;
	Mat imSmall;
	Mat imSmallVector;
	Mat featSortIdx;
	Mat featSortIdxFloat;
	Mat scaledFeatSortIdx;
	Size dims(hbins,vbins);
	Mat imSmallFloat;
	Mat imSmallNorm;
	float normFactor = 2.0f/(hbins*hbins*vbins*vbins*3); //sum of all values = 1 

	//convert to grayscale and downsample to blocks
	//cvtColor(src, imGray,CV_RGB2GRAY );
	resize(src, imSmall, dims, 0, 0, INTER_AREA); 
	//imSmall.convertTo(imSmallFloat,CV_32FC1);

	vector<Mat> layers;
	vector<Mat> layersSorted(3);
	split(imSmall, layers);

	for(int i=0;i<3;i++) {
		sortIdx(layers[i],layersSorted[i],CV_SORT_EVERY_ROW + CV_SORT_ASCENDING );
	}
	merge(layersSorted,imSmallNorm);

	imSmallVector = imSmallNorm.reshape(1,1);
	imSmallVector.convertTo(featSortIdx,CV_32FC1,normFactor);

	return featSortIdx;
	
}

//color MinMax separate
Mat TCL_UView_OrdinalFeature::calculateOrdinal7(Mat src)
{
	Mat imGray;
	Mat imSmall;
	Mat imSmallVector;
	Mat featSortIdx;
	Mat featSortIdxFloat;
	Mat scaledFeatSortIdx;
	Size dims(hbins,vbins);
	Mat imSmallFloat;
	Mat imSmallNorm;
	float normFactor = 2.0f/getFeatureSize(); //average sum of all values = 1 

	//convert to grayscale and downsample to blocks
	//cvtColor(src, imGray,CV_RGB2GRAY );
	resize(src, imSmall, dims, 0, 0, INTER_AREA); 
	imSmall.convertTo(imSmallFloat,CV_32FC1);

	vector<Mat> layers;
	vector<Mat> layersNormed(3);
	split(imSmallFloat, layers);

	for(int i=0;i<3;i++) {
		normalize(layers[i],layersNormed[i],normFactor,0,NORM_MINMAX);
	}
	merge(layersNormed,imSmallNorm);

	imSmallVector = imSmallNorm.reshape(1,1);


	return imSmallVector;
	
}

//weighted color MinMax
Mat TCL_UView_OrdinalFeature::calculateOrdinal8(Mat src)
{
	Mat imGray;
	Mat imSmall;
	Mat imSmallVector;
	Mat imSmallFloatNorm;
	Mat imSmallFloatNorm2;
	Mat imSmallVectorFloat;
	Mat scaledFeatSortIdx;
	Size dims(hbins,vbins);
	Mat imSmallFloat;
	float normFactor = 2.0f/getFeatureSize(); //average sum of all values = 1 

	//convert to grayscale and downsample to blocks
	//cvtColor(src, imGray,CV_RGB2GRAY );
	resize(src, imSmall, dims, 0, 0, INTER_AREA); 

	//give different weights to bgr channels - 0.2989 * R + 0.5870 * G + 0.1140 * B  

	imSmall.convertTo(imSmallFloat,CV_32FC1);
	normalize(imSmallFloat,imSmallFloatNorm,normFactor,0,NORM_MINMAX);

	vector<Mat> layers;
	split(imSmallFloatNorm, layers);
	layers[0] = layers[0]*(0.1140*3); //B
	layers[1] = layers[1]*(0.5870*3); //G
	layers[2] = layers[2]*(0.2989*3); //R
	merge(layers,imSmallFloatNorm2);

	imSmallVectorFloat = imSmallFloatNorm2.reshape(1,1);

	return imSmallVectorFloat;
	
}

//color 
Mat TCL_UView_OrdinalFeature::calculateOrdinal9(Mat src) //color, not sorted, normalized to 1
{
	Mat imGray;
	Mat imSmall;
	Mat imSmallVector;
	Mat featSortIdx;
	Mat featSortIdxFloat;
	Mat scaledFeatSortIdx;
	Size dims(hbins,vbins);
	Mat imSmallVectorFloat;
	float normFactor = 2.0f/getFeatureSize(); //average sum of all values = 1 

	//convert to grayscale and downsample to blocks
	//cvtColor(src, imGray,CV_RGB2GRAY );
	resize(src, imSmall, dims, 0, 0, INTER_AREA); 

	//Mat testimSmall(imSmall);

	imSmallVector = imSmall.reshape(1,1);
	imSmallVector.convertTo(imSmallVectorFloat,CV_32FC1);

	featSortIdxFloat = imSmallVectorFloat/(255*2);
	//normalize(imSmallVectorFloat,featSortIdxFloat,normFactor,0,NORM_MINMAX);

	return featSortIdxFloat;
	
}