/***********************************************************************************
 2D Hue-Saturation Histogram feature
 
 Created:
 TCL Research America
 Armin Kappeler, 09/18/2013

 Modification:
 
 ************************************************************************************/
#pragma once

#include "../tcl_uview_feature.h"
#include <vector>
#include <list>
#include <math.h>       /* sqrt */

using namespace cv;
using namespace std;

class TCL_UView_HSColorHistogram :
	public TCL_UView_Feature
{
private:
	int hbins; //initialization in constructor
	int sbins;

public:
	Mat calculateHistogram(Mat src);

	TCL_UView_HSColorHistogram(int hb, int sb);
	TCL_UView_HSColorHistogram(FeatureSettings);
	TCL_UView_HSColorHistogram(void);
	~TCL_UView_HSColorHistogram(void);	

	virtual Mat getFeatures(list<Mat>);	
	virtual Mat getFeature(Mat);
	virtual int getFeatureSize(void) {return hbins*sbins;};

	int getHBins(void) {return hbins;};
	int getSBins(void) {return sbins;};


};

