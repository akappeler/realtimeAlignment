/*
 * realtimeAlign.h
 *
 *  Created on: Oct 14, 2014
 *      Author: Armin Kappeler
 */

#include "realtimeAlign.h"

using namespace std;

/*******************************************************************************************/
void realtimeAlign(Keyframes db_frames,char * videoFilename)
{
	//PARAMETERS
	int interleave = 10;
	bool showFrames = 1;

	// init variables
	VideoSettings vidCfg;
    string filename(videoFilename);
    VideoCapture capture(filename);
    Mat frame,frameRoi,tmpFeature;

	//initialize Feature
    FeatureExtractor * vidFeature = new FeatureExtractor(db_frames.featureType);


	if( !capture.isOpened() ){
        cout  << "Could not open reference " << filename << endl;
	}

	//check video length
	int videoLength = capture.get(CV_CAP_PROP_FRAME_COUNT);
	if (vidCfg.startFr >= videoLength)
	{
		vidCfg.startFr = (int)videoLength - 1;
	}

	int nrFrames = (int) std::min(videoLength-vidCfg.startFr,vidCfg.maxNrFr);

	//initialize alignment variables
	double timestampProb[nrFrames/interleave];   //current timestamp probability
	double dist[db_frames.nrKeyframes];          //feature distance between queryframes and keyframes
	double cumDist[db_frames.nrKeyframes]; 		//cumulative feature distance

	double cumDistDisplay[db_frames.nrKeyframes];
	double distDisplay[db_frames.nrKeyframes];
    for(int i=0;i<db_frames.nrKeyframes;i++)
    {
    	cumDist[i]=0;
    	//cumDist_prev[i]=0;
    }


	int featureLength = vidFeature->getFeatureSize();
	//Mat featureValues(nrKeyframes,featureLength,CV_32FC1); //float;
	int progress=0,progress_old;

	if (showFrames) {
		namedWindow( "Feature Extraction", 1);
		moveWindow("Feature Extraction", 2000, 0);
		namedWindow( "Timestamp",1);
		moveWindow("Timestamp", 2000, 400);
		namedWindow( "Timestamp_Cumulative",1);
		moveWindow("Timestamp_Cumulative", 2000, 700);
	}

	capture.set(CV_CAP_PROP_POS_FRAMES, vidCfg.startFr);
	double frameRate = capture.get(CV_CAP_PROP_FPS);

	int nextKeyfrIdx = 0;

    for(int i=0 ; ; i++)
    {
		//Step 1: get next frame
    	if (i >= nrFrames)
    	{
    		cout << "\nall frames extracted";
    		break;
    	}
    	if (!capture.grab())
    	{
    		cout << "\nend of video";
    		break;
    	}
    	else if (i%interleave == 0) //only read keyframes
    	{
			capture.retrieve(frame);
			//capture >> frame;
			if ((frame.cols > 0) && (frame.rows > 0)) {
				frameRoi = frame(cv::Range(vidCfg.uBorder,frame.rows-vidCfg.dBorder), cv::Range(vidCfg.lBorder, frame.cols-vidCfg.rBorder));
				//Step 2: calculate features
				tmpFeature = vidFeature->getFeature(frameRoi);
			}

			//Step 3: calculate feature distance
			double tmpBuf = cumDist[0];
			double tmpMinBuf = 9E99;
			int predictedFrameNr = -1;
			for (int kf=0; kf<db_frames.nrKeyframes;kf++)
			{
				dist[kf] = vidFeature->getFeatureDistance(db_frames.features.row(kf), tmpFeature);
				tmpBuf = min(tmpBuf,cumDist[kf]);
				cumDist[kf] = tmpBuf+dist[kf];

				//find best match
				if(cumDist[kf] < tmpMinBuf)
				{
					tmpMinBuf = cumDist[kf];
					predictedFrameNr = db_frames.indices[kf];
				}

				//prepare for display
				distDisplay[kf] = 1/(dist[kf]+0.0001);
				cumDistDisplay[kf] = 1/(cumDist[kf]);
			}

			//show frame
			stringstream convert;
			convert << "GT: " << i << " Predicted: " << predictedFrameNr;
			if (showFrames)
			{
				imshow("Feature Extraction", frameRoi);
				CvPlot::clear("Timestamp");
				CvPlot::plot("Timestamp", distDisplay, db_frames.nrKeyframes, 1,0,0,255);
				CvPlot::clear("Timestamp_Cumulative");
				CvPlot::plot("Timestamp_Cumulative", cumDistDisplay, db_frames.nrKeyframes, 1,255,0,0);
				CvPlot::label(convert.str());
				waitKey(100); // waits to display frame
			}

			//show progress
			progress_old = progress;
			progress = (100*i)/nrFrames;
			if (progress != progress_old) {
				cout << "\r" << progress <<"% " << convert.str() << flush;
			}

			nextKeyfrIdx++;
    	}
    }
	cout << "\r100%\n";
	destroyWindow("Feature Extraction");
	destroyWindow("Timestamp");
	destroyWindow("Timestamp_Cumulative");
	capture.release();

	delete vidFeature;

}

