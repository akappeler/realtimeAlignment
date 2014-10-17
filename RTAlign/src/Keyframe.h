/*
 * Keyframe.h
 *
 *  Created on: Oct 7, 2014
 *      Author: Armin Kappeler
 */

#ifndef SRC_KEYFRAME_H_
#define SRC_KEYFRAME_H_

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <omp.h>
#include <stdio.h>
#include "FeatureExtractor.h"

#define ONE_OVER_SQRT2 0.707106781186548f
#define KF_DTW_TYPE 2

using namespace std;
using namespace cv;

//Configuration for preset keyframe extraction
struct VideoSettings
{
	int uBorder; //up
	int dBorder; //down
	int lBorder; //left
	int rBorder; //right

	int startFr; //starting from this frame
	int maxNrFr; //number of frames to read (9999999 = read all frames)

	VideoSettings() { //set default values
		lBorder=0;
		rBorder=0;
		uBorder=0;
		dBorder=0;
		maxNrFr = 9999999;
		startFr = 1;
	}
};

//Keyframe class declaration
class Keyframes
{
public:
	vector<float> timestamps;
	vector<int> indices;
	Mat features;
	double frameRate;
	int nrKeyframes;
	int featureType;
	bool showFrames;

private:
	VideoSettings vidCfg;
	FeatureExtractor * vidFeature;

public:
	Keyframes(void);
	~Keyframes(void);

	Mat getKeyframeFeatures(char *filePath, int featType); //main function, returns a Mat containing the features

	void selectKeyframes(int nrFrames);
	void loadKeyframes(void);
	vector<int> selectRandomKeyframes(long nrFrames_movie,long nrKeyframes);

	bool save(char *filePath);
	bool load(char *filePath);

};


#endif /* SRC_KEYFRAME_H_ */
