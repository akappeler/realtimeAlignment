/***********************************************************************************
 Feature Extractor

 Created:
 TCL Research America
 Armin Kappeler, 10/07/2014

 Modification:

 ************************************************************************************/
#include "FeatureExtractor.h"

/***********************************************************************************/
FeatureExtractor::FeatureExtractor(void)
{
	hbins = 8;
	vbins = 4;
	type = 1;
};

/***********************************************************************************/
FeatureExtractor::FeatureExtractor(int ftype)
{
	hbins = 8;
	vbins = 4;
	type = ftype;
};

/***********************************************************************************/
FeatureExtractor::~FeatureExtractor(void)
{

};

/***********************************************************************************/
Mat FeatureExtractor::getFeatures(list<Mat> clip, int type)
{

	Mat tmp = getFeature(clip.front());

	Mat all_feat(tmp.cols,(int)clip.size(),CV_32FC1); //float

	// main loop for clip frames
	int i=0;
	for (list<Mat>::iterator it = clip.begin(); it != clip.end(); it++)
	{
		Mat tmp = getFeature(*it).t();
		tmp.copyTo(all_feat.col(i));
		i++;
	}

	return all_feat;
}

/***********************************************************************************/
Mat FeatureExtractor::getFeature(cv::Mat img)
{
	switch (type)
	{
	case 1:
		return calculateFeature_ColorNorm(img); //color, not sorted, normalized
		break;
	default:
		return calculateFeature_ColorNorm(img);
		break;
	}
}

/***********************************************************************************/
double FeatureExtractor::getFeatureDistance(Mat feat1, Mat feat2)
{

	switch (type)
	{
	case 1:
		return norm(feat1,feat2,NORM_L1); //color, not sorted, normalized
		break;
	default:
		return norm(feat1,feat2,NORM_L1);
		break;
	}

}

/***********************************************************************************/
int FeatureExtractor::getFeatureSize(void)
{
	switch (type)
	{
	case 1:
		return hbins*vbins*3; //color, not sorted, normalized
		break;
	default:
		return hbins*vbins*3;
		break;
	}


}

/***********************************************************************************/
Mat FeatureExtractor::calculateFeature_ColorNorm(Mat src) //color, not sorted, normalized
{

	Mat imSmall;
	Mat imSmallVector;
	Mat featSortIdxFloat;

	Size dims(hbins,vbins);
	Mat imSmallVectorFloat;
	float normFactor = 2.0f/(hbins*vbins); //average sum of all values = 1

	//convert to grayscale and downsample to blocks
	//cvtColor(src, imGray,CV_RGB2GRAY );
	resize(src, imSmall, dims, 0, 0, INTER_AREA);

	//Mat testimSmall(imSmall);

	imSmallVector = imSmall.reshape(1,1);
	imSmallVector.convertTo(imSmallVectorFloat,CV_32FC1);
	normalize(imSmallVectorFloat,featSortIdxFloat,normFactor,0,NORM_MINMAX);

	return featSortIdxFloat;

}
