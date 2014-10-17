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
	int interleave = 25;
	double var_constant = 100000;
	bool showFrames = 1;
	double learningRate = 0.1; //0..1 -> lower value means less weight to the pat predictions
	double adaptive_learningRate;

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
	vector<double> dist(db_frames.nrKeyframes,0);          //feature distance between queryframes and keyframes
	vector<double> cumDist(db_frames.nrKeyframes,0); 		//cumulative feature distance
	int timestampProb_range = nrFrames/interleave;
	int zeroIdx = 0;//timestampProb_range/2;
	vector<double> timestampProb(timestampProb_range,1/double(timestampProb_range));   //current timestamp probability
	vector<double> timestampProb_update(timestampProb_range,0);   //current timestamp probability

	//initialize display variables
	int * debugPtr = &db_frames.indices[0];
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
		namedWindow( "Timestamp_Probability",1);
		moveWindow("Timestamp_Probability", 2600,400 );
		namedWindow( "Timestamp_Probability_Cumulative",1);
		moveWindow("Timestamp_Probability_Cumulative", 2600,700 );
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

			//Step 3: calculate feature distance to each keyframe
			double tmpBuf = cumDist[0];
			double tmpMinBuf = 9E99;
			int predictedFrameNr = -1;
			int predictedFrameIdx = -1;

			fill(timestampProb_update.begin(), timestampProb_update.end(), 0);
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
					predictedFrameIdx = kf;
				}

				//build mixture of gaussian distribution
				double mean = db_frames.indices[kf]/interleave;
				double var = var_constant*pow(dist[kf],4)+0.0000000001;
				for (unsigned int pIdx = 0; pIdx<timestampProb_update.size();pIdx++)
				{

					timestampProb_update[pIdx] += gaussian(pIdx-zeroIdx,mean,var);
				}

				//prepare for display
				distDisplay[kf] = 1/(dist[kf]+0.0001);
				cumDistDisplay[kf] = 1/(cumDist[kf]);
			}

			//update posterior estimate
			for (unsigned int pIdx = timestampProb.size(); pIdx>0;pIdx--)
			{
				timestampProb[pIdx] = (1-learningRate)*timestampProb[pIdx-1] + learningRate*timestampProb_update[pIdx];
			}
			//timestampProb[0] = timestampProb_update[0];
			timestampProb[0] = learningRate*timestampProb_update[0];

			int predicted_timestamp = distance(timestampProb.begin(),max_element(timestampProb.begin(), timestampProb.end()))*interleave;


			//Last step: show frame
			stringstream convert1,convert2;
			convert1 << "GT: " << i/frameRate << "s Predicted: " << predictedFrameNr/frameRate << "s Error: " << (i-predictedFrameNr)/frameRate << "s";
			convert2 << "GT: " << i/frameRate << "s Predicted: " << predicted_timestamp/frameRate << "s Error: " << (i-predicted_timestamp)/frameRate << "s";
			if (showFrames)
			{
				imshow("Feature Extraction", frameRoi);
				CvPlot::clear("Timestamp");
				CvPlot::plot("Timestamp", distDisplay, db_frames.nrKeyframes, 1,0,0,255);
				CvPlot::clear("Timestamp_Cumulative");
				CvPlot::plot("Timestamp_Cumulative", cumDistDisplay, db_frames.nrKeyframes, 1,255,0,0);
				CvPlot::pointOfInterest(predictedFrameIdx);
				CvPlot::pointOfInterest(db_frames.indices,interleave);
				CvPlot::label(convert1.str());
				CvPlot::clear("Timestamp_Probability");
				CvPlot::plot("Timestamp_Probability", &timestampProb_update[0], timestampProb_range, 1,255,0,255);
				CvPlot::clear("Timestamp_Probability_Cumulative");
				CvPlot::plot("Timestamp_Probability_Cumulative", &timestampProb[0], timestampProb_range, 1,255,0,255);
				CvPlot::label(convert2.str());
				CvPlot::pointOfInterest(predicted_timestamp/interleave);
				CvPlot::pointOfInterest(db_frames.indices,interleave);

				waitKey(1); // waits to display frame
			}

			//show progress
			progress_old = progress;
			progress = (100*i)/nrFrames;
			if (progress != progress_old) {
				cout << "\r" << progress <<"% " << convert2.str() << flush;
			}

			nextKeyfrIdx++;
    	}
    }
	cout << "\r100%\n";
	destroyWindow("Feature Extraction");
	destroyWindow("Timestamp");
	destroyWindow("Timestamp_Cumulative");
	destroyWindow("Timestamp_Probability");
	destroyWindow("Timestamp_Probability_Cumulative");
	capture.release();

	delete vidFeature;

}

double gaussian(double x,double mean, double var)
{
	return 1/(var*sqrt(2*M_PI))*exp(-pow(x-mean,2)/(2*var));
}
