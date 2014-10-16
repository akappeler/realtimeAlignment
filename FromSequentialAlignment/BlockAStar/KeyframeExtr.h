/***********************************************************************************
 Keyframe Extraction
 
 Created:
 TCL Research America
 Armin Kappeler, 09/18/2013

 Modification:
 
 ************************************************************************************/

#pragma once

#include "../BlockAStar/AStarMatrix.h"

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <ppl.h>
#include <omp.h>

#define ONE_OVER_SQRT2 0.707106781186548f
#define KF_DTW_TYPE 2
//#define DELETE_LASTANDFIRST2KEYFRAMES

using namespace std;
using namespace cv;

struct KeyframeSettings
{
	int skipFactor; // used for Keyframe DTW (1..100)
	float tolerance; // tolerance in percent for candidate selection, higher tolerance->more candidates->slower->more accurate (0..INFINITY)
	int distanceNorm; //1 = l1, 2=l2 distance
	int saveMask;

	KeyframeSettings() { //set default values
		skipFactor = 1; //only every n-th frame is used to extract keyframes
		tolerance = 2;  //mask width (higher value = wider mask)
		distanceNorm = 1; //1=L1, 2=L2 norm
		saveMask = 0;   //saves a bitmap image of the mask
	}
};

class KeyframeExtractor
{
private:
	//parameters
	int distanceNorm;
	int skipFactor; 
	float tolerance; 
	int saveMask;

	int distanceType; 

public:
	KeyframeExtractor(void);
	KeyframeExtractor(KeyframeSettings cfg);
	~KeyframeExtractor(void);

	Mat generateMask(Mat featuresVid1, Mat featuresVid2, int blockSize); //main method
	vector<point> doWholeDTW(Mat featuresVid1, Mat featuresVid2, bool withW);
	
private:

	//methods for keyframe DTW
	vector<int>  getKeyframes(Mat seq);
	Mat calcCostMatrix(Mat seq1,Mat seq2,int seq1StartIdx,int seq1EndIdx);
	Mat calcCostMatrix32(Mat seq1,Mat seq2,int seq1StartIdx,int seq1EndIdx);
	Mat calcCostMatrix32_parallel(Mat seq1,Mat seq2,int seq1StartIdx,int seq1EndIdx);
	Mat calcCostMatrix(Mat seq1,Mat seq2,int seq1StartIdx,int seq1EndIdx,int seq2StartIdx,int seq2EndIdx);
	Mat calcCostMatrix(Mat seq1,Mat seq2,vector<int> idx1, vector<int> idx2);
	vector<point> dtw_constraint(Mat D);
	vector<point> dtw_standard(Mat D);
	vector<point> dtw_standard(Mat D, float costPerNonDiagMove);
	vector<point> backtrack(Mat G);
	vector<point> backtrack(Mat G, Mat D, float costPerNonDiagMove);
	Mat createMaskFromKeypoints(vector<point> keypoints, Size matrixSize, int blockSize, float tolerance, int recursion=5);
	bool completeMask(Mat &mask,int maskValue);
	double median(vector<double> v);



};

