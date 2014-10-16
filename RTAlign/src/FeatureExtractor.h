/*
 * FeatureExtractor.h
 *
 *  Created on: Oct 7, 2014
 *      Author: Armin Kappeler
 */

#ifndef SRC_FEATUREEXTRACTOR_H_
#define SRC_FEATUREEXTRACTOR_H_

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <list>

using namespace cv;
using namespace std;

class FeatureExtractor
{
private:
	int hbins;
	int vbins;
	int type; //feature type 1: downsampling, 2: binary feature, 3: deep feature

public:
	FeatureExtractor(void);
	~FeatureExtractor(void);
	FeatureExtractor(int ftype);

	void setFeatureType(int ftype) {type = ftype;};
	Mat getFeatures(list<Mat>,int type); //for a set of frames
	Mat getFeature(Mat);  //for one frame
	int getFeatureSize(void);
	double getFeatureDistance(Mat feat1, Mat feat2);

private:
	Mat calculateFeature_ColorNorm(Mat src); //color, not sorted, normalized
};



#endif /* SRC_FEATUREEXTRACTOR_H_ */
