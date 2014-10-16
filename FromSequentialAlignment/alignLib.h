/***********************************************************************************
 Sequential Alignment Library
 
 Created:
 TCL Research America
 Armin Kappeler, 09/18/2013

 Modification:
 
 ************************************************************************************/
#pragma once

#include "Features/TCL_UView_Feature.h"
#include "Features/FeaturesImpl/TCL_UView_HSColorHistogram.h"
#include "Features/FeaturesImpl/TCL_UView_Ordinal.h"
#include "BlockAStar/BlockAStar.h"
#include "BlockAStar/KeyframeExtr.h"
#include <string>
#include <list>
// OpenCV libraries
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <time.h>       //to initialize random
#include <algorithm>
#include <sstream>
#include <fstream>

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
		startFr = 0;
	}
};

// main class
class SequentialAlignment
{
private:
	TCL_UView_Feature * vidFeature;

//PARAMETERS ---------------------------------------------------

	// VIDEO PROCESSING
	VideoSettings videoCfg[2];

	bool showFrames; //makes process slower!

	//FEATURE EXTRACTION
	FeatureSettings featureCfg;

	// Alignment
	AlignSettings alignCfg;

	// KEYFRAMES
	bool useKeyFrames;  // use keyframes to speed up the process
	KeyframeSettings kfCfg;

// END PARAMETERS ------------------------------------------------------

public:
	SequentialAlignment(void);
	~SequentialAlignment(void);

	//main methods
	int alignVideos(char * filename1, char * filename2, char * resultFilename);
	int calculateVideoFeatures(char * filename1, char * resultFilename);

	//property set methods
	void setShowFrames(int value) {showFrames = (value!=0);};
	void setUseKeyframes(int value) {useKeyFrames = (value!=0);};
	void setVideoConfig(int vidNr,VideoSettings values) {videoCfg[vidNr-1] = values;}; 
	void setFeatureConfig(FeatureSettings values) {featureCfg = values;};
	void setAlignConfig(AlignSettings values) {alignCfg = values;};
	void setKeyframeConfig(KeyframeSettings values) {kfCfg = values;};

private:
	//Mat getVideoFeaturesGPU(char *filePath, VideoSettings vidCfg, double &frameRate);
	Mat getVideoFeatures(char *filePath, VideoSettings vidCfg, double &frameRate);
	inline float* convertToFloat(Mat mt); //Mat to Array conversion (row-wise)
	inline vector<point> sparsifyPath (vector<point> path);

	//File I/O functions
	inline string getFileExtension(char * filename);
	void writeAlignResultToFile(const char * outputFilename, vector<point> path, char * filename1, char * filename2, double frameRate1, double frameRate2);
	bool setClipsFeature(char * filePath, cv::Mat &featureValues, double framerate);
	Mat getClipsFeature(char * filePath, double & framerate);
	void saveArray(char * filename, float* array, int length);


};



