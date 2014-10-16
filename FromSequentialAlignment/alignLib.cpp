/***********************************************************************************
 Sequential Alignment Library
 
 Created:
 TCL Research America
 Armin Kappeler, 09/18/2013

 Modification:
 
 ************************************************************************************/
//#define WHOLEDTW		//only for comparison purposes
//#define WHOLEDTW_W	//only for comparison purposes
//#define DO_SPARSIFYPATH

#include "alignLib.h"
#include "Shlwapi.h"
#include <windows.h>  //for timer only -> remove timer function for platform independence

SequentialAlignment::SequentialAlignment(void)
{
	//initialize default values
	videoCfg[0].lBorder=0;
	videoCfg[0].rBorder=0;
	videoCfg[0].uBorder=0;
	videoCfg[0].dBorder=0;
	videoCfg[1].lBorder=0;
	videoCfg[1].rBorder=0;
	videoCfg[1].uBorder=0;
	videoCfg[1].dBorder=0;

	videoCfg[0].maxNrFr = 9999999; //9999999 //16000
	videoCfg[1].maxNrFr = 9999999; //9999999 //16000
	videoCfg[0].startFr = 0; //27800
	videoCfg[1].startFr = 0;

	showFrames = 0;

}

/*******************************************************************************************/
SequentialAlignment::~SequentialAlignment(void)
{
}

/*******************************************************************************************/
//converts Mat objects to float arrays
inline float* SequentialAlignment::convertToFloat(Mat mt)
{
	float * arr;
	/*if (mt.isContinuous())  
	{*/
		arr = (float*)mt.data;
	/*}
	else
	{
		printf("Matrix not continuous: copying");
		arr = new float[mt.rows*mt.cols];
		for (int r = 0; r < mt.rows; ++r)
		{
			for (int c = 0; c < mt.cols; ++c)
			{
				arr[(r)+c*mt.rows] = mt.at<float>(r,c);
			} 
		}
	}*/
	return arr;
}
/*******************************************************************************************/
//returns file extension from filename
inline string SequentialAlignment::getFileExtension(char * filename)
{
    string path(filename);
	string ext;

    size_t dot = path.find_last_of(".");
    if (dot != string::npos) 
    {
        ext  = path.substr(dot, path.size() - dot);
    }
    else
    {
        ext  = "";
    }
	return ext;
}

/************************************************************************/
//main function: prepares data for sequence alignment call
int SequentialAlignment::alignVideos(char * filename1, char * filename2, char * resultFilename)
{
	//Initialization ----------------------------------------------------
	double frameRate1,frameRate2;
	Mat featureValuesMat1,featureValuesMat2;
	float* featureValues1;
	float* featureValues2;
	Mat maskMat;
	unsigned char * maskPtr;

	//CVPR LOG
	double logval1=0;
	double logval2=0;
	//CVPR LOG END

	DWORD dwTime;
	printf("Start Process\r\n");

	//Feature Extraction ------------------------------------------------
	//load Video 1
    dwTime = GetTickCount();
	string ext = getFileExtension(filename1);
	if (ext.compare(".yaml")==0) //load features from file
	{
		featureValuesMat1 = getClipsFeature(filename1,frameRate1);
		featureValues1 = convertToFloat(featureValuesMat1);
		dwTime = GetTickCount() - dwTime;
		printf("Features Video 1 loaded %d s\r\n",dwTime/1000);	
	}
	else { //calculate features
		featureValuesMat1 = getVideoFeatures(filename1, videoCfg[0],frameRate1);
		featureValues1 = convertToFloat(featureValuesMat1);
		dwTime = GetTickCount() - dwTime;
		printf("Features Video 1 extracted: %d s\r\n",dwTime/1000);
	}
	//save features
	//setClipsFeature("featVid1.yaml",featureValuesMat1,frameRate1);
	//saveArray("Friends_ord_short.csv",featureValues1,featureValuesMat1.rows*featureValuesMat1.cols);

	//load Video 2
    dwTime = GetTickCount();
	ext = getFileExtension(filename2);
	if (ext.compare(".yaml")==0) //load features from file
	{
		featureValuesMat2 = getClipsFeature(filename2,frameRate2);
		featureValues2 = convertToFloat(featureValuesMat2);
		dwTime = GetTickCount() - dwTime;
		printf("Features Video 2 loaded %d s\r\n",dwTime/1000);	
	}
	else { //calculate features
		dwTime = GetTickCount();
		featureValuesMat2 = getVideoFeatures(filename2, videoCfg[1],frameRate2);
		featureValues2 = convertToFloat(featureValuesMat2);
		dwTime = GetTickCount() - dwTime;
		printf("Features Video 2 extracted: %d s \r\n",dwTime/1000);
	}
	//save features
	//setClipsFeature("featVid2.yaml",featureValuesMat2,frameRate2);
	//saveArray("Friends_ord_edit_short.csv",featureValues2,featureValuesMat2.rows*featureValuesMat2.cols);
	
	//input file check
	if (featureValuesMat1.cols == 0 || featureValuesMat1.rows == 0) {
		cout << "Error: invalid input file 1\n";
		exit(0);
	}
	if (featureValuesMat1.cols == 0 || featureValuesMat1.rows == 0) {
		cout << "Error: invalid input file 2\n";
		exit(0);
	}
	if (featureValuesMat1.cols != featureValuesMat2.cols) {
		cout << "Error: file 1 and file 2 do not contain the same feature type\n";
		exit(0);
	}

#ifdef WHOLEDTW
	dwTime = GetTickCount();
	KeyframeExtractor kfExtractor(kfCfg);
	vector<point> path = kfExtractor.doWholeDTW(featureValuesMat1,featureValuesMat2,false);
#else
#ifdef WHOLEDTW_W
	dwTime = GetTickCount();
	KeyframeExtractor kfExtractor(kfCfg);
	vector<point> path = kfExtractor.doWholeDTW(featureValuesMat1,featureValuesMat2,true);
#else
/*#ifdef BOTHDTW
	dwTime = GetTickCount();
	KeyframeExtractor kfExtractor(kfCfg);
	vector<point> path_DTW;
	vector<point> path_DTW_W;
	vector<point> path;
	kfExtractor.doWholeDTW(featureValuesMat1,featureValuesMat2,path_DTW,path_DTW_W);

	//change path
	//wstring filename = L"C:\\MyDirectory\\MyFile.bat";

	wstring filename(resultFilename);
	wchar_t* filepart = PathFindFileName(filename.c_str());

	string str( filename.begin(), filename.end() );
	const char *cstr = str.c_str();

	//write files
	writeAlignResultToFile(cstr, path_DTW,filename1,filename2,frameRate1,frameRate2);
	writeAlignResultToFile(resultFilename, path_DTW_W,filename1,filename2,frameRate1,frameRate2);
#else*/

	BlockAStar aStarPath;
	//Keyframe Extraction----------------------------------------------------------
	if (useKeyFrames)
	{
		dwTime = GetTickCount();
		KeyframeExtractor kfExtractor(kfCfg);
		maskMat = kfExtractor.generateMask(featureValuesMat1,featureValuesMat2,alignCfg.blockSize);
		
		//convert Mat to Matrix<unsigned char>
		//maskMat = maskMat.t();		
		if (maskMat.isContinuous()) {
			maskPtr = (unsigned char*)maskMat.data;
			aStarPath.setMask(maskPtr, maskMat.cols, maskMat.rows);
		} 
		else {
			printf("Error: Mask matrix not continuous - continue without keyframes");
		}
		dwTime = GetTickCount() - dwTime;
		printf("Keyframe extracted: %d s \r\n",dwTime/1000);
		logval1 = dwTime;
	}

	//Initialize Block A*
	dwTime = GetTickCount();
	int sizeSeq1 = featureValuesMat1.rows; //SWITCHED BECAUSE OF TRANSPOSE
	int sizeSeq2 = featureValuesMat2.rows;
	int sizeFeat = featureValuesMat2.cols;
	
	//Do Block A*
	aStarPath.init(featureValues1,featureValues2,sizeSeq1,sizeSeq2,sizeFeat,alignCfg);
	vector<point> path = aStarPath.findPath();

	logval2 = aStarPath.LOG_calcBlocks;

	///clean up
	featureValuesMat1.release();
	featureValuesMat2.release();

#endif //WHOLEDTW
#endif //WHOLEDTW_W


	//show result
	dwTime = GetTickCount() - dwTime;
	printf("Alignment done. Path length: %d Time: %d s \r\n", path.size(),dwTime/1000);
	logval1 += dwTime;

#ifdef DO_SPARSIFYPATH
	vector<point> sparsePath = sparsifyPath(path);
	//save result in cvs file
	//writeAlignResultToFile(resultFilename, sparsePath,filename1,filename2,frameRate1,frameRate2);
	writeAlignResultToFile(resultFilename, sparsePath,filename1,filename2,logval1,logval2);
#else
	writeAlignResultToFile(resultFilename, path,filename1,filename2,frameRate1,frameRate2);
#endif //DO_SPARSIFYPATH

	return 1;
}

/*******************************************************************************************/
//Writes output text file
void SequentialAlignment::writeAlignResultToFile(const char * outputFilename, vector<point> path, char * filename1, char * filename2, double frameRate1, double frameRate2)
{
	//get timestamp
	time_t now = time(0);
	tm *ltm = localtime(&now);
	std::stringstream timestr;
	timestr << (ltm->tm_year + 1900) << '-'  << (ltm->tm_mon + 1) << '-' <<  ltm->tm_mday << " " << ltm->tm_hour << ltm->tm_min;

	//open file
	string filename(outputFilename);
	ofstream myfile;
	//myfile.open (filename + timestr.str() + ".txt");
	myfile.open (filename);
	//write output
	myfile << "# ALIGNMENT RESULT\n";
	myfile << "# " << timestr.str() << "\n#\n";
	myfile << "# " << filename1 <<"\n";
	myfile << "# " << filename2 <<"\n";

	// framerates
	myfile << "FPS1=" << frameRate1 <<"\n";
	myfile << "FPS2=" << frameRate2 <<"\n";
	int pLength = (int)path.size();
	//for (int i = 0;i<pLength;i++) {
	for (int i = pLength-1;i>=0;i--) {
		myfile << path[i].first << " " <<  path[i].second << "\n";
	}
	myfile.close();

}

/*******************************************************************************************/
//reduces the alignment path to the coordinates, where the direction changes
inline vector<point> SequentialAlignment::sparsifyPath (vector<point> path)
{
	vector<point> sparsePath;
	bool firstChange = 0;
	bool secondChange = 0;
	bool firstChangeOld = (path[1].first == path[0].first);
	bool secondChangeOld = (path[1].second == path[0].second);

	sparsePath.push_back(path[0]); //add first element
	for (int i = 2;i<path.size()-1;i++)
	{
		firstChange = (path[i].first == path[i-1].first);
		secondChange = (path[i].second == path[i-1].second);

		if ((firstChange != firstChangeOld) || (secondChange != secondChangeOld))
		{
			sparsePath.push_back(path[i-1]);
		}
		firstChangeOld = firstChange;
		secondChangeOld = secondChange;
	}
	sparsePath.push_back(path[path.size()-1]); //add last element
	return sparsePath;
}

/*******************************************************************************************/
//Calculates the features from a input video
Mat SequentialAlignment::getVideoFeatures(char *filePath, VideoSettings vidCfg, double &frameRate)
{
    string filename(filePath);
	//cout << "DEBUG: INIT VIDEO";
    VideoCapture capture(filename);
	//cout << " - DEBUG: INIT VIDEO DONE\n";
    Mat frame,frameRoi,tmpFeature;

	//initialize Feature
	if (featureCfg.featureType==2) {
		vidFeature = new TCL_UView_HSColorHistogram(featureCfg);
	} else {
		vidFeature = new TCL_UView_OrdinalFeature(featureCfg);
	}
	
	if( !capture.isOpened() ){
        throw "Error when reading Video";
	}

	//check video length
	double videoLength = capture.get(CV_CAP_PROP_FRAME_COUNT);
	if (vidCfg.startFr >= videoLength)
	{
		vidCfg.startFr = (int)videoLength - 1;
	}    

	int nrFrames = (int) min(videoLength-vidCfg.startFr,vidCfg.maxNrFr);

	int featureLength = (vidFeature->getFeatureSize());	
	Mat featureValues(nrFrames,featureLength,CV_32FC1); //float;
	int progress=0,progress_old;

	if (showFrames) {
		namedWindow( "Feature Extraction", 1);
	}

	capture.set(CV_CAP_PROP_POS_FRAMES, vidCfg.startFr);
	frameRate = capture.get(CV_CAP_PROP_FPS);

    for(int i=0 ; ; i++)
    {
		//get next frame
        capture >> frame;
		if ((frame.cols > 0) && (frame.rows > 0)) {
			frameRoi = frame(cv::Range(vidCfg.uBorder,frame.rows-vidCfg.dBorder), cv::Range(vidCfg.lBorder, frame.cols-vidCfg.rBorder));
			//calculate features
			tmpFeature = vidFeature->getFeature(frameRoi);		
		} else {
			//If frame is invalid, use the previous frame..
			//tmpFeature = tmpFeature
		}
		tmpFeature.copyTo(featureValues.row(i));

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
    }
	cout << "\r100%\n";	
	destroyWindow("Feature Extraction");
	capture.release();

	delete vidFeature;
	return featureValues;
}

/*******************************************************************************************/
// saves feature values to .yaml file
bool SequentialAlignment::setClipsFeature(char * filePath, Mat &featureValues, double frameRate)
{
	
	cv::FileStorage fs(filePath, cv::FileStorage::WRITE);
	try
	{		
		fs << "frameRate" << frameRate;
		fs << "featureValues" << featureValues;
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
Mat SequentialAlignment::getClipsFeature(char * filePath, double & frameRate)
{
	Mat mat;

	FileStorage fs;
	try
	{
		if(fs.open(filePath, FileStorage::READ) == false)
		{
			printf("getClipsFeature: open file error. %s\r\n", filePath);
			return mat;
		}
		fs["frameRate"] >> frameRate;		
		fs["featureValues"] >> mat;	
	}
	catch (Exception &e)
	{
		fs.release();
		printf("setClipsFeature error. %s\r\n", e.msg.c_str());
	}
	fs.release();
	return mat;
}

/*******************************************************************************************/
//saves feature values to .csv file
void SequentialAlignment::saveArray(char * filename, float* array, int length)
{
    ofstream output(filename);
    for(int i=0;i<length;i++)
    {
        output<<array[i]<<endl;
    }
}

/*******************************************************************************************/
//public method to obtain video features without doing alignment
int SequentialAlignment::calculateVideoFeatures(char * filename1, char * resultFilename)
{
	double frameRate;

	DWORD dwTime = GetTickCount();
	printf("Start Process\r\n");

	//load Video 1
	Mat featureValuesMat1 = getVideoFeatures(filename1, videoCfg[0],frameRate);
	float* featureValues1 = convertToFloat(featureValuesMat1);
	dwTime = GetTickCount() - dwTime;
	printf("Features Video 1 extracted: %d s\r\n",dwTime/1000);
	dwTime = GetTickCount();

	//save features
	string ext = getFileExtension(resultFilename);
	if (ext.compare(".csv")==0) { //if ending is csv, save information as array in a csv file
		saveArray(resultFilename,featureValues1,featureValuesMat1.rows*featureValuesMat1.cols);
	}else {
		setClipsFeature(resultFilename,featureValuesMat1,frameRate);
	}

	return 1;
}