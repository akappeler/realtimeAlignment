/***********************************************************************************
 Ordinal Feature 
 
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

class TCL_UView_OrdinalFeature :
	public TCL_UView_Feature
{
private:
	int hbins; //initialization in constructor
	int vbins;
	int method;

public:
	Mat calculateOrdinal(Mat src);

	Mat calculateOrdinal3(Mat src);
	Mat calculateOrdinal4(Mat src);
	Mat calculateOrdinal5(Mat src);
	Mat calculateOrdinal6(Mat src);
	Mat calculateOrdinal7(Mat src);
	Mat calculateOrdinal8(Mat src);
	Mat calculateOrdinal9(Mat src);

	TCL_UView_OrdinalFeature(int hb, int vb);
	TCL_UView_OrdinalFeature(FeatureSettings);
	TCL_UView_OrdinalFeature(void);
	~TCL_UView_OrdinalFeature(void);	

	virtual Mat getFeatures(list<Mat>);	
	virtual Mat getFeature(Mat);
	virtual int getFeatureSize(void);

	int getHBins(void) {return hbins;};
	int getVBins(void) {return vbins;};


};

