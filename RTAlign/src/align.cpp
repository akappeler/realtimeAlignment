/*
 * align.cpp
 *
 *  Created on: Oct 7, 2014
 *      Author: Armin Kappeler
 */


#include <opencv2/opencv.hpp>
#include <stdio.h>

#include "FeatureExtractor.h"
#include "realtimeAlign.h"
#include "Keyframe.h"
//#include "cvplot/cvplot.h"

//#include <boost/archive/tSSext_oarchive.hpp>
//#include <boost/archive/text_iarchive.hpp>

using namespace cv;

int plot_sample();

int main( int argc, char** argv )
{

	Mat img(480, 640, CV_8U);
	  //putText(img, "Hello Armin!", Point( 200, 300 ), FONT_HERSHEY_SIMPLEX | FONT_ITALIC, 1.0, Scalar( 255, 255, 0 ));
	  //imshow("My Window", img);
	  //waitKey();

	  // PARAMETERS
	  //char * videoFilename = "/home/armin/NU/data/RealtimeAlignment/tclDataset/videos/Megamind.avi";
	  char * videoFilename = "/home/armin/NU/data/RealtimeAlignment/tclDataset/videos/010.FLV";
	  char * featureFilename = "/home/armin/NU/data/RealtimeAlignment/tclDataset/features/feature.yaml";

	  // PREPROCESSING:
	  // 0) extract database features and save to file

	  Keyframes db_frames;
	  //Mat features = db_frames.getKeyframeFeatures(videoFilename,1);
	  //db_frames.save(featureFilename);

	  // MAIN PROCESS
	  // 1) read or extract database features
	  db_frames.load(featureFilename);


	  // 2) open query video
	  // 3) initialize probabilities
	  realtimeAlign(db_frames,videoFilename);

	  // LOOP
	  //    4) read frame
	  //    5) extract feature
	  //    6) update probabilities
	  //    7) show progress

	  // POSTPROCESSING
	  // 8) store ground truth
	  // 9) store test configuration

	  // Data / Datastructure
	  // config-file: contains all configurations except database name and query video
	  // features_db: save as Mat
	  // timestamps_db: timestamps of features
	  // D: matching cost matrix
	  // G: accumulated alignment cost matrix

	  //putText(img, "Done!", Point( 200, 400 ), FONT_HERSHEY_SIMPLEX | FONT_ITALIC, 1.0, Scalar( 255, 255, 0 ));
	  //imshow("My Window", img);
	  //waitKey();

		/*float pts[100];
		for (int i=0;i<100;i++)
			pts[i]=(i*2)%100;

		CvPlot::plot("RGB", pts, 50, 2,0,0,255);
		CvPlot::label("B");
		destroyWindow("RGB");*/
		waitKey();


	  return 0;

}
