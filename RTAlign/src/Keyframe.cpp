/*
 * Keyframe.cpp
 *
 *  Created on: Oct 7, 2014
 *      Author: Armin Kappeler
 */

#include "Keyframe.h"

using namespace cv;

/*******************************************************************************************/
Keyframes::Keyframes()
{
	nrKeyframes = 6000;
	featureType = 1;
	showFrames = 1;
	frameRate = -1;

}

/*******************************************************************************************/
Keyframes::~Keyframes()
{

}

/*******************************************************************************************/
//Calculates the features from a input video
Mat Keyframes::getKeyframeFeatures(char *filePath, int featType)
{
    string filename(filePath);
    VideoCapture capture(filename);
    Mat frame,frameRoi,tmpFeature;

	//initialize Feature
    featureType = featType;
    vidFeature = new FeatureExtractor(featType);


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

	//select frames to be extracted
	selectRandomKeyframes(nrFrames,nrKeyframes);

	int featureLength = vidFeature->getFeatureSize();
	features.create(nrKeyframes,featureLength,CV_32FC1); //float;
	int progress=0,progress_old;

	if (showFrames) {
		namedWindow( "Feature Extraction", 1);
	}

	capture.set(CV_CAP_PROP_POS_FRAMES, vidCfg.startFr);
	frameRate = capture.get(CV_CAP_PROP_FPS);

	int nextKeyfrIdx = 0;

    for(int i=0 ; ; i++)
    {
		//get next frame
    	if (nextKeyfrIdx >= nrKeyframes)
    	{
    		cout << "\nall keyframes extracted";
    		break;
    	}
    	if (!capture.grab())
    	{
    		cout << "\nend of video";
    		break;
    	}

    	else if (indices[nextKeyfrIdx]==i) //only read keyframes
    	{
			capture.retrieve(frame);
			//capture >> frame;
			if ((frame.cols > 0) && (frame.rows > 0)) {
				frameRoi = frame(cv::Range(vidCfg.uBorder,frame.rows-vidCfg.dBorder), cv::Range(vidCfg.lBorder, frame.cols-vidCfg.rBorder));
				//calculate features
				tmpFeature = vidFeature->getFeature(frameRoi);
			} else {
				//If frame is invalid, use the previous frame..
				//tmpFeature = tmpFeature
			}
			tmpFeature.copyTo(features.row(nextKeyfrIdx));

			//show progress
			progress_old = progress;
			progress = (100*i)/nrFrames;
			if (progress != progress_old) {
				cout << "\r" << progress <<"%" << flush;
			}
			//show frame
			if (showFrames)
			{
				imshow("Feature Extraction", frameRoi);
				waitKey(1); // waits to display frame
			}
			//check, if enough frames read
			if(i>=nrFrames-1) {
				break;
			}

			nextKeyfrIdx++;
    	}
    }
	cout << "\r100%\n";
	destroyWindow("Feature Extraction");
	capture.release();

	delete vidFeature;
	return features;
}

/*******************************************************************************************/
vector<int> Keyframes::selectRandomKeyframes(long nrFrames_movie,long nrKeyfr)
{
	int offset = 25; //250// do not use the beginning and the end of the movie

	indices.resize(nrKeyfr);
	for (unsigned int i=0;i<indices.size();i++)
	{
		indices[i] = rand() % (nrFrames_movie-offset*2 + offset);
	}
	std::sort(indices.begin(), indices.end());
	indices.erase( unique( indices.begin(), indices.end() ), indices.end() );
	nrKeyframes = indices.size();

	return indices;

}

/*******************************************************************************************/
// saves feature values to .yaml file
bool Keyframes::save(char * filePath)
{

	cv::FileStorage fs(filePath, FileStorage::WRITE);
	try
	{
		fs << "nrKeyframes" << nrKeyframes;
		fs << "featureType" << featureType;
		write(fs,"timestamps",timestamps);
		write(fs,"indices",indices);
		fs << "features" << features;

	}
	catch (cv::Exception &e)
	{
		fs.release();
		printf("setClipsFeature error. %s\r\n", e.msg.c_str());
		return false;
	}
	fs.release();
	return true;
}

/*******************************************************************************************/
//reads feature values from .yaml file
bool Keyframes::load(char * filePath)
{

	FileStorage fs;
	try
	{
		if(fs.open(filePath, FileStorage::READ) == false)
		{
			printf("getClipsFeature: open file error. %s\r\n", filePath);
			return false;
		}
		fs["nrKeyframes"] >> nrKeyframes;
		fs["featureType"] >> featureType;
		fs["timestamps"] >> timestamps;
		fs["indices"] >> indices;
		fs["features"] >> features;
	}
	catch (Exception &e)
	{
		fs.release();
		printf("setClipsFeature error. %s\r\n", e.msg.c_str());
	}
	fs.release();
	return true;
}
