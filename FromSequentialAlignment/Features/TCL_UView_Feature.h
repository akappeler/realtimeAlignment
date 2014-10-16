/***********************************************************************************
 Feature Base Class
 
 Created:
 TCL Research America
 Armin Kappeler, 09/18/2013

 Modification:
 
 ************************************************************************************/
#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <list>

//using namespace cv;
using namespace std;

struct FeatureSettings
{
	int Bins1; 
	int Bins2; 
	int featureType;

	FeatureSettings() { //set default values
		Bins1=16;
		Bins2=8;
		featureType = 1;
	}
};

class TCL_UView_Feature
{
private:	

public:	
	virtual cv::Mat getFeatures(std::list<cv::Mat>);
	virtual cv::Mat getFeature(cv::Mat) { cv::Mat tmp; return tmp;};

	virtual int getFeatureSize(void) {return -1;};

	TCL_UView_Feature(void);
	~TCL_UView_Feature(void);

};
