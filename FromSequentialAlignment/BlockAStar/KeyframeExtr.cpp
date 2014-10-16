/***********************************************************************************
 Keyframe Extraction
 
 Created:
 TCL Research America
 Armin Kappeler, 09/18/2013

 Modification:
 
 ************************************************************************************/
#include "KeyframeExtr.h"

using namespace cv;
KeyframeExtractor::KeyframeExtractor(void)
{
	KeyframeSettings cfg;
	distanceNorm = cfg.distanceNorm;
	skipFactor = cfg.skipFactor;
	tolerance = cfg.tolerance;
	saveMask = cfg.saveMask;
	if (distanceNorm==1) 
		distanceType = NORM_L1;
	else //distanceNorm==2
		distanceType = NORM_L2;
}

/***********************************************************************************/
KeyframeExtractor::KeyframeExtractor(KeyframeSettings cfg)
{
	distanceNorm = cfg.distanceNorm;
	skipFactor = cfg.skipFactor;
	tolerance = cfg.tolerance;
	saveMask = cfg.saveMask;
	if (distanceNorm==1) 
		distanceType = NORM_L1;
	else //distanceNorm==2
		distanceType = NORM_L2;

}
KeyframeExtractor::~KeyframeExtractor(void)
{
}

/***********************************************************************************/
//returns vector with keyframe indices
vector<int> KeyframeExtractor::getKeyframes(Mat seq)
{
	//initialize values
    double thresB_alpha = 2; //2
    double thresS_alpha = 1.5; //1.5
	double thresB;
	double thresS;

	int nrFrames = seq.rows;

	//calculate feature difference
	vector<double> featDiff;
	featDiff.reserve((int)floor(nrFrames/skipFactor)-1); //reserve memory

	int i_prev = 0;
	//featDiff.push_back(0);
	for (int i=skipFactor;i<nrFrames;i=i+skipFactor)
	{
		 featDiff.push_back(norm(seq.row(i),seq.row(i_prev),distanceType));
		 i_prev = i;
	}
	
	//calculate thresholds
	double median_featDiff = median(featDiff);
	thresB = median_featDiff*thresB_alpha;
	thresS = median_featDiff*thresS_alpha;

	//scan video for scenes
	int belowThresTolerance = (int)floor(2/skipFactor);
	int belowThresCounter = belowThresTolerance;
	vector<int> shotStartIdx;
	vector<int> shotEndIdx;
	shotStartIdx.push_back(0);

    int cutStart = -1;
    for (unsigned int i=1;i<featDiff.size();i++)
	{
        if (featDiff[i] >= thresS) { //transition or cut starts  
            if (cutStart == -1) {
                cutStart = i;
			}  
            belowThresCounter = belowThresTolerance;  
		}
        else if (belowThresCounter > 0) {//drop is within tolerance
            belowThresCounter = belowThresCounter - 1;
		}
        else if ((cutStart) > -1) { //diff below threshold: end of transition or cut
			if (norm(seq.row(cutStart),seq.row(i-belowThresTolerance),distanceType) > thresB) {
				shotEndIdx.push_back(cutStart*skipFactor-skipFactor);
				shotStartIdx.push_back((i-belowThresTolerance*skipFactor));
			}
            cutStart = -1;
		}
	}
	shotEndIdx.push_back(nrFrames-1);
	
#ifdef DELETE_LASTANDFIRST2KEYFRAMES
	//delete first and last keyframe pair -> unreliable
	shotEndIdx.pop_back();
	shotStartIdx.pop_back();
	shotEndIdx.erase(shotEndIdx.begin());
	shotStartIdx.erase(shotStartIdx.begin());
#endif //DELETE_LASTANDFIRST2KEYFRAMES

	//extract keyframe from each scene
	int tmpKFIdx,tmpKFIdx_prev;
	double tmp_featDiff,tmp_featDiff_prev;
	vector<int> keyFrameIdx;


	for (unsigned int i=0;i<shotStartIdx.size();i++)
	{
        //get keyframe from beginning of scene
        //move left until value is bigger than Ts
        tmpKFIdx = shotStartIdx[i]; 
        if (i>0) {
            tmp_featDiff = 0;
            while(tmp_featDiff<thresS && tmpKFIdx > 1) {
				tmpKFIdx_prev = tmpKFIdx;
                tmpKFIdx = tmpKFIdx - 1;                                 
                tmp_featDiff = norm(seq.row(tmpKFIdx),seq.row(tmpKFIdx_prev),distanceType);
			}
		}
        else {
            tmpKFIdx = 0;
            tmp_featDiff = FLT_MAX;
		}
        //move right until local minimum
        tmp_featDiff_prev = FLT_MAX;
        while((tmp_featDiff<tmp_featDiff_prev || tmp_featDiff>thresS) && tmpKFIdx < nrFrames-2 ) {            
            tmp_featDiff_prev = tmp_featDiff;
			tmpKFIdx_prev = tmpKFIdx;
            tmpKFIdx = tmpKFIdx + 1;                                  
            tmp_featDiff = norm(seq.row(tmpKFIdx),seq.row(tmpKFIdx_prev),distanceType);             
		}
		//Add keyframe to list
		if (keyFrameIdx.size() == 0) {
			keyFrameIdx.push_back(tmpKFIdx-1);
		} else if(keyFrameIdx.back() < (tmpKFIdx-1)) { //if Keyframe is already in vector, don't add again
			keyFrameIdx.push_back(tmpKFIdx-1);
		}

        //get keyframe from end of scene
        //move right until value is bigger than Ts
        tmpKFIdx = shotEndIdx[i];
        if (i<shotEndIdx.size()-1) {
            tmp_featDiff = 0;
            while(tmp_featDiff<thresS && tmpKFIdx < nrFrames-2) {
				tmpKFIdx_prev = tmpKFIdx;
                tmpKFIdx = tmpKFIdx + 1;                                
                tmp_featDiff = norm(seq.row(tmpKFIdx),seq.row(tmpKFIdx_prev),distanceType);                   
			}
		}
        else {
            tmpKFIdx = nrFrames-1;
            tmp_featDiff = FLT_MAX;                
		}
        //move left until local minimum
        tmp_featDiff_prev = FLT_MAX;
        while((tmp_featDiff<tmp_featDiff_prev || tmp_featDiff>thresS) && tmpKFIdx>1 ) {
            tmp_featDiff_prev = tmp_featDiff;
			tmpKFIdx_prev = tmpKFIdx;
            tmpKFIdx = tmpKFIdx - 1;                        
            tmp_featDiff = norm(seq.row(tmpKFIdx),seq.row(tmpKFIdx_prev),distanceType);               
		}
		//Add keyframe to list
		if (keyFrameIdx.back() < (tmpKFIdx+1)) { //if Keyframe is already in vector, don't add again
			keyFrameIdx.push_back(tmpKFIdx+1); 
		}

	}

	cout << keyFrameIdx.size() << " keyframes extracted\n";// << ", Tb = " << thresB << ", Ts = " << thresS << "\n";
	//return result
	return keyFrameIdx;

}

/***********************************************************************************/
//calculate median
double KeyframeExtractor::median(vector<double> v)
{
    size_t n = v.size() / 2;
    nth_element(v.begin(), v.begin()+n, v.end());
    return v[n];
}


/***********************************************************************************/
//calculates cost matrix D for DTW
Mat KeyframeExtractor::calcCostMatrix(Mat seq1,Mat seq2,int seq1StartIdx,int seq1EndIdx)
{
	int N = seq1EndIdx-seq1StartIdx+1; //channel
	int M = seq2.rows; //query
	Mat D(N,M,CV_64FC1);

	for (int n = 0; n<N; n++)
	{
		for (int m = 0; m<M; m++)
		{
			//calculate distance
			D.at<double>(n,m) = norm(seq1.row(seq1StartIdx+n),seq2.row(m),distanceType);
		}
	}

	return D;
}

/***********************************************************************************/
//calculates cost matrix D for DTW, but 32 Bit values
Mat KeyframeExtractor::calcCostMatrix32(Mat seq1,Mat seq2,int seq1StartIdx,int seq1EndIdx)
{
	int N = seq1EndIdx-seq1StartIdx+1; //channel
	int M = seq2.rows; //query
	Mat D(N,M,CV_32FC1);

	for (int n = 0; n<N; n++)
	{
		for (int m = 0; m<M; m++)
		{
			//calculate distance
			D.at<float>(n,m) = norm(seq1.row(seq1StartIdx+n),seq2.row(m),distanceType);
		}
	}

	return D;
}

/***********************************************************************************/
//calculates cost matrix D for DTW, but 32 Bit values
/*Mat KeyframeExtractor::calcCostMatrix32_parallel(Mat seq1,Mat seq2,int seq1StartIdx,int seq1EndIdx)
{
	int N = seq1EndIdx-seq1StartIdx+1; //channel
	int M = seq2.rows; //query
	Mat D(N,M,CV_32FC1);

	concurrency::parallel_for(size_t(0), size_t(N), [&](size_t n)
	{
		for (int m = 0; m<M; m++)
		{
			//calculate distance
			D.at<float>(n,m) = norm(seq1.row(seq1StartIdx+n),seq2.row(m),distanceType);
		}
	});

	return D;
}*/

Mat KeyframeExtractor::calcCostMatrix32_parallel(Mat seq1,Mat seq2,int seq1StartIdx,int seq1EndIdx)
{
	int N = seq1EndIdx-seq1StartIdx+1; //channel
	int M = seq2.rows; //query
	Mat D(N,M,CV_32FC1);

	// Get the number of processors in this system
	int iCPU = omp_get_num_procs();
	// Now set the number of threads
	omp_set_num_threads(iCPU-1);

	int m = 0;
	#pragma omp parallel for private(m) 
	for (int n = 0; n<N; n++)
	{
		for (m = 0; m<M; m++)
		{
			//calculate distance
			D.at<float>(n,m) = norm(seq1.row(seq1StartIdx+n),seq2.row(m),distanceType);
		}
	}

	return D;
}

/***********************************************************************************/
//calculates cost matrix D for DTW
Mat KeyframeExtractor::calcCostMatrix(Mat seq1,Mat seq2,int seq1StartIdx,int seq1EndIdx,int seq2StartIdx,int seq2EndIdx)
{
	int N = seq1EndIdx-seq1StartIdx+1; //channel
	int M = seq2EndIdx-seq2StartIdx+1; //channel
	Mat D(N,M,CV_64FC1);

	for (int n = 0; n<N; n++)
	{
		for (int m = 0; m<M; m++)
		{
			//calculate distance
			D.at<double>(n,m) = norm(seq1.row(seq1StartIdx+n),seq2.row(seq2StartIdx+m),distanceType);
		}
	}

	return D;
}

/***********************************************************************************/
//calculates cost matrix D for DTW for selected indices
Mat KeyframeExtractor::calcCostMatrix(Mat seq1,Mat seq2,vector<int> idx1, vector<int> idx2)
{
	int N = (int)idx1.size(); //channel
	int M = (int)idx2.size(); //query
	Mat D(N,M,CV_64FC1);

	for (int n = 0; n<N; n++)
	{
		for (int m = 0; m<M; m++)
		{
			//calculate distance
			D.at<double>(n,m) = norm(seq1.row(idx1[n]),seq2.row(idx2[m]),distanceType);
		}
	}
	return D;
}

/***********************************************************************************/
//calculates the constraint dtw and returns the accumulated cost and the first and the last keyframe
//Important: the second dimension (cols) has to be the Query-dimension
vector<point> KeyframeExtractor::dtw_constraint(Mat D)
{
	int N = D.rows;
	int M = D.cols;
	int minIdx;
	double minVal;
	int tmpMinIdx;
	double curVal;

	Mat G(N,M,CV_64FC1); //Matrix with accumulated cost

	//first column
	for (int n = 0; n<N; n++)
	{
		G.at<double>(n,0) = D.at<double>(n,0);
	}

	//first row
	for (int m = 1; m<M; m++)
	{
		G.at<double>(0,m) = D.at<double>(0,m) + G.at<double>(0,m-1);
	}

	//all other points
	for (int m = 1; m<M; m++)
	{
		minIdx = 0;
		minVal = G.at<double>(0,m-1);
		for (int n = 1; n<N; n++)
		{
			double curVal = G.at<double>(n,m-1);
			if (curVal < minVal) {
				minVal = curVal;
				minIdx = n;
			}
			G.at<double>(n,m) = D.at<double>(n,m) + minVal;
		}
	}

	//backtracking
	vector<point> path;
	minIdx = N-1;	

	for (int m = M-1; m>=0; m--)
	{
		tmpMinIdx = minIdx;
		minVal = FLT_MAX; // = "infinity"
		
		//find minimum
		for (int n = 0; n<=tmpMinIdx; n++)
		{
			curVal = G.at<double>(n,m);
			if (curVal < minVal) {
				minVal = curVal;
				minIdx = n;
			}
		}
		path.push_back(point(minIdx,m));
	}

	return path;

}

/***********************************************************************************/
vector<point> KeyframeExtractor::dtw_standard(Mat D)
{
	int N = D.rows;
	int M = D.cols;

	Mat G(N,M,CV_32FC1); //Matrix with accumulated cost

	//first cell
	G.at<float>(0,0) = D.at<float>(0,0);
	//first column
	for (int n = 1; n<N; n++)
	{
		G.at<float>(n,0) = D.at<float>(n,0) + G.at<float>(n-1,0);

	}

	//first row
	for (int m = 1; m<M; m++)
	{
		G.at<float>(0,m) = D.at<float>(0,m) + G.at<float>(0,m-1);

	}

	//all other points
	for (int m = 1; m<M; m++)
	{
		for (int n = 1; n<N; n++)
		{
			G.at<float>(n,m) = D.at<float>(n,m) + min(min(G.at<float>(n-1,m),G.at<float>(n-1,m-1)),G.at<float>(n,m-1));
		}
	}

	//backtracking
	vector<point> path = backtrack(G);
	return path;

}

/***********************************************************************************/
// finds the cheapest path from the end to the beginning by following the lowest cost 
//in the G matrix, starting from the corner in the last row and last column
vector<point> KeyframeExtractor::backtrack(Mat G)
{
	int N = G.rows-1;
	int M = G.cols-1;

	int n=N;
	int m=M;
	vector<point> w;
	w.push_back(point(N,M)); //last point

	while ((n+m)>0) {
		if (n==0) { 
			m=m-1;
		}
		else if (m==0) {
			n=n-1;
		}
		else {
			if ((G.at<float>(n-1,m-1) <= G.at<float>(n-1,m)) && (G.at<float>(n-1,m-1) <= G.at<float>(n,m-1))) {
				n = n-1;
				m = m-1;
			} else if (G.at<float>(n-1,m) <= G.at<float>(n,m-1)) {
				n = n-1;
			} else {
				m = m-1;
			}
		}
		w.push_back(point(n,m));
	}

	return w;
}

/***********************************************************************************/
vector<point> KeyframeExtractor::dtw_standard(Mat D, float costPerNonDiagMove)
{
	int N = D.rows;
	int M = D.cols;

	Mat G(N,M,CV_32FC1); //Matrix with accumulated cost

	//first cell
	G.at<float>(0,0) = D.at<float>(0,0);
	//first column
	for (int n = 1; n<N; n++)
	{
		G.at<float>(n,0) = costPerNonDiagMove + G.at<float>(n-1,0);
	}

	//first row
	for (int m = 1; m<M; m++)
	{
		G.at<float>(0,m) = costPerNonDiagMove + G.at<float>(0,m-1);
	}

	//all other points
	for (int m = 1; m<M; m++)
	{
		for (int n = 1; n<N; n++)
		{
			G.at<float>(n,m) = min(min(G.at<float>(n-1,m),G.at<float>(n,m-1))+costPerNonDiagMove,G.at<float>(n-1,m-1)+D.at<float>(n,m));
		}
	}

	//backtracking
	vector<point> path = backtrack(G,D,costPerNonDiagMove);
	return path;

}

/***********************************************************************************/
// finds the cheapest path from the end to the beginning by following the lowest cost 
//in the G matrix, starting from the corner in the last row and last column
vector<point> KeyframeExtractor::backtrack(Mat G, Mat D, float costPerNonDiagMove)
{
	int N = G.rows-1;
	int M = G.cols-1;

	int n=N;
	int m=M;
	vector<point> w;
	w.push_back(point(N,M)); //last point

	while ((n+m)>0) {
		if (n==0) { 
			m=m-1;
		}
		else if (m==0) {
			n=n-1;
		}
		else {
			//TODO: resolve situation where (n-1,m-1) == (n-1,m) or similar
			if ((G.at<float>(n-1,m-1)+D.at<float>(n,m) < G.at<float>(n,m-1)+costPerNonDiagMove) && (G.at<float>(n-1,m-1)+D.at<float>(n,m) < G.at<float>(n-1,m)+costPerNonDiagMove)) 
			{
				n = n-1;
				m = m-1;
			} else if (G.at<float>(n-1,m) <= G.at<float>(n,m-1)) {
				n = n-1;
			} else {
				m = m-1;
			}
		}
		w.push_back(point(n,m));
	}

	G = G - 40300;
	return w;
}

//*************************************************************************************
// Creates a Mask, that defines the region, where the true path can lie
//
// Input:
// matrixSize: original size of full matrix
// keypoints: points that have to be covered
// blockSize: corresponds to downsampling rate
// tolerance: how much tolerance in blocks around keypoints

Mat KeyframeExtractor::createMaskFromKeypoints(vector<point> keyPoints, Size matrixSize, int blockSize, float tolerance, int recursion)
{
	const int maskValue = 255;
	Size maskSize;
	maskSize.width = (int)ceil((float)matrixSize.height/blockSize);
	maskSize.height = (int)ceil((float)matrixSize.width/blockSize);

	Mat mask = Mat::zeros(maskSize,CV_8UC1); //corresponds to char
//1) find corners -> second derrivative of difference -> [1 -2 1]
	int nrKeyPoints = (int)keyPoints.size();
	vector<int> x; //contains difference between coordinates (col-row)
	x.reserve(nrKeyPoints);

	for (int i = 0; i<nrKeyPoints; i++)
	{
		x.push_back(keyPoints[i].first - keyPoints[i].second); //calculate difference
	}

	//if second derivative of x != 0 -> corner
	vector<int> corners;
	corners.push_back(0); //add start point

	for (int i = 1; i<nrKeyPoints-1; i++)
	{
		if ((x[i-1] - 2*x[i] + x[i+1]) != 0) {
			corners.push_back(i);
		}
	}
	corners.push_back(nrKeyPoints-1); //add end point

//..iterate through all corners
	point lc,rc,move,lcMask,rcMask; 
	point unitVector(1,1);
	Mat structureElement;
	int scaleSE;
	for (int i = 0; i<corners.size()-1;i++)
	{
//3) get movement direction
		lc = keyPoints[corners[i]];  //left corner
		rc = keyPoints[corners[i+1]];  //right corner
		move = rc-lc;

//3) calculate confidence (resp. size of structure element for opening)
		//double conf = ((move*unitVector)*ONE_OVER_SQRT2/move.norm());
		double conf = pow(((move*unitVector)*ONE_OVER_SQRT2/move.norm()),2.0);
		scaleSE = (int)((tolerance/conf)*2.01); //the .01 are to compensate rounding errors
//4) move along line and set mask
		int fraction = 256; //8 bits
		//calculate point coordinates
		float ptx1,pty1,ptx2,pty2;
		ptx1 = max((((float)lc.first)/blockSize-0.5f)*fraction,0.0f);
		pty1 = max((((float)lc.second)/blockSize-0.5f)*fraction,0.0f);
		Point pt_lc((int)(ptx1+0.5),(int)(pty1+0.5f)); //round

		ptx2 = max((((float)rc.first)/blockSize-0.5f)*fraction,0.0f);
		pty2 = max((((float)rc.second)/blockSize-0.5f)*fraction,0.0f);
		Point pt_rc((int)(ptx2+0.5),(int)(pty2+0.5f)); //round

		line(mask, pt_lc, pt_rc, maskValue, scaleSE, 4,8); //8 fractional bits in the point coordinates
	}


//5) connect start- and end-point to mask
	bool isMaskComplete = completeMask(mask,maskValue);
	if (!isMaskComplete) { //if mask doesn't touch the border -> increase the mask width
		if (recursion>0) { 
			recursion--;
			mask = createMaskFromKeypoints(keyPoints, matrixSize, blockSize, tolerance+1, recursion);
		} else {
			cout << "Error: couldn't complete the mask\n";
			exit(0);
		}
	}
	
//6) write mask to file
	if (saveMask==1) {
		imwrite( "Mask.bmp", mask ); //for debugging purposes
	}
	//waitKey(0);	
	
	return mask;
}

//********************************************************************************************************
//connects start-point (bottom left corner) and end-point (upper right corner) to region, where mask is set
bool KeyframeExtractor::completeMask(Mat &mask,int maskValue) 
{
	int firstRowIdx;
	int firstColIdx;

//connect start point -------------------------------------------------------------------------------
	if(mask.at<unsigned char>(0,0) != maskValue)
	{
		//find first non-zero in first column
		firstColIdx = -1;
		firstRowIdx = -1;
		for (int i=1;i<mask.rows;i++) {
			if (mask.at<unsigned char>(i,0) != 0 ) {
				firstRowIdx = i;
				break;
			}
		}

		//find first non-zero in first row
		for (int i=1;i<mask.cols;i++) {
			if (mask.at<unsigned char>(0,i) != 0 ) {
				firstColIdx = i;
				break;
			}
		}

		//no edge touches masked region
		if(firstColIdx == -1 && firstRowIdx == -1) { 
			cout << "Beginning of mask is not complete -> increase mask size\n";
			if (saveMask==1) {
				imwrite( "incomplete_Mask.bmp", mask );
			}
			return false;
		}
		//both edges touch at the same coordinates
		else if (firstColIdx==firstRowIdx) { 
			for (int i = 0;i<min(mask.cols,mask.rows);i++) {
				if (mask.at<unsigned char>(i,i) == 0) {
					mask.at<unsigned char>(i,i) = maskValue;
				} else {
					break;
				}
			}
		}
		//both edges touch at different coordinates
		else if (firstColIdx != -1 && firstRowIdx != -1) { 
			if (firstColIdx < firstRowIdx) {
				for (int i = 0;i<firstColIdx;i++) {
					mask.at<unsigned char>(0,i) = maskValue;
				}
			} else {
				for (int i = 0;i<firstRowIdx;i++) {
					mask.at<unsigned char>(i,0) = maskValue;
				}
			}
		}
		//row touches -> connect along row
		else if (firstColIdx != -1) { 
			for (int i = 0;i<firstColIdx;i++) {
				mask.at<unsigned char>(0,i) = maskValue;
			}
		}
		//col touches -> connect along col
		else if (firstRowIdx != -1) { 
			for (int i = 0;i<firstRowIdx;i++) {
				mask.at<unsigned char>(i,0) = maskValue;
			}
		}
	}
//connect end point -------------------------------------------------------------------------------
	if(mask.at<unsigned char>(mask.rows-1,mask.cols-1) != maskValue)
	{
		int lastRow = mask.rows-1;
		int lastCol = mask.cols-1;
		firstColIdx = -1;
		firstRowIdx = -1;
		//find first non-zero in last column
		for (int i=lastRow;i>=0;i--) {
			if (mask.at<unsigned char>(i,lastCol) != 0 ) {
				firstRowIdx = i;
				break;
			}
		}

		//find first non-zero in last row
		for (int i=lastCol;i>=0;i--) {
			if (mask.at<unsigned char>(lastRow,i) != 0 ) {
				firstColIdx = i;
				break;
			}
		}

		//no edge touches masked region
		if(firstColIdx == -1 && firstRowIdx == -1) { 
			cout << "End of mask is not complete -> increase mask size\n";
			if (saveMask==1) {
				imwrite( "incomplete_Mask.bmp", mask );
			}
			return false;
		}
		//both edges touch at the same coordinates
		else if (firstColIdx==firstRowIdx) { 
			for (int i = 0;i<min(mask.rows,mask.cols);i++) {
				if (mask.at<unsigned char>(lastRow-i,lastCol-i) == 0) {
					mask.at<unsigned char>(lastRow-i,lastCol-i) = maskValue;
				} else {
					break;
				}
			}
		}
		//both edges touch at different coordinates
		else if (firstColIdx != -1 && firstRowIdx != -1) { 
			if (firstColIdx > firstRowIdx) {
				for (int i = lastCol;i>firstColIdx;i--) {
					mask.at<unsigned char>(lastRow,i) = maskValue;
				}
			} else {
				for (int i = lastRow;i>firstRowIdx;i--) {
					mask.at<unsigned char>(i,lastCol) = maskValue;
				}
			}
		}
		//row touches -> connect along row
		else if (firstColIdx != -1) { 
			for (int i = lastCol;i>firstColIdx;i--) {
				mask.at<unsigned char>(lastRow,i) = maskValue;
			}
		}
		//col touches -> connect along col
		else if (firstRowIdx != -1) { 
			for (int i = lastRow;i>firstRowIdx;i--) {
				mask.at<unsigned char>(i,lastCol) = maskValue;
			}
		}	
	}

	return true;
}


//*************************************************************************************
//main method
Mat KeyframeExtractor::generateMask(Mat featuresVid1, Mat featuresVid2, int blockSize)
{

	//1) Keyframe extraction

	vector<int> kfIdx1 = getKeyframes(featuresVid1);
	vector<int> kfIdx2 = getKeyframes(featuresVid2);

	//2) dtw on Keyframes
	Mat D = calcCostMatrix(featuresVid1,featuresVid2,kfIdx1,kfIdx2);
	//imwrite( "D.bmp", D); //for debugging purposes
	

	vector<point> kfPath;
	switch (KF_DTW_TYPE) {
	case 1: //normal DTW
		kfPath = dtw_standard(D);
		break;
	case 2: //constraint DTW on shorter side
		if (D.cols<=D.rows) {
			kfPath = dtw_constraint(D);
		}
		else {
			kfPath = dtw_constraint(D.t());
			for (vector<point>::iterator it = kfPath.begin(); it != kfPath.end(); it++) {
				it->transpose();
			}
		}
	}

	//3) generate Keypoints
	vector<point> keyPoints;
	for (vector<point>::reverse_iterator  rit = kfPath.rbegin(); rit != kfPath.rend(); rit++)
	{
		int  test1 = rit->first;
		int test2 = rit->second;
		keyPoints.push_back(point(kfIdx1[rit->first],kfIdx2[rit->second]));
	}
	Size matrixSize(featuresVid2.rows,featuresVid1.rows);

	//4) generate mask from keypoints
	Mat mask = createMaskFromKeypoints(keyPoints, matrixSize, blockSize, tolerance);
	return mask;

}

//*************************************************************************************
//CVPR Comparison method
vector<point> KeyframeExtractor::doWholeDTW(Mat featuresVid1, Mat featuresVid2,bool withW)
{

	cout << "start Cost Matrix Calc\n";
	//Mat D = calcCostMatrix32(featuresVid1,featuresVid2,0,featuresVid1.rows-1);
	Mat D = calcCostMatrix32_parallel(featuresVid1,featuresVid2,0,featuresVid1.rows-1);
	featuresVid1.release();
	featuresVid2.release();

	vector<point> kfPath;
	cout << "start DTW\n";
	if (withW){
		kfPath = dtw_standard(D,1.0);
	}
	else{
		kfPath = dtw_standard(D);
	}
	cout << "end DTW\n";

	return kfPath;

}

/*int KeyframeExtractor::doWholeDTW(Mat featuresVid1, Mat featuresVid2,vector<point> &path_DTW,vector<point> &path_DTW_W)
{

	cout << "start Cost Matrix Calc\n";
	//Mat D = calcCostMatrix32(featuresVid1,featuresVid2,0,featuresVid1.rows-1);
	Mat D = calcCostMatrix32_parallel(featuresVid1,featuresVid2,0,featuresVid1.rows-1);
	featuresVid1.release();
	featuresVid2.release();

	vector<point> kfPath;
	cout << "start DTW_W\n";
	path_DTW_W = dtw_standard(D,1.0);

	cout << "start DTW\n";
	path_DTW = dtw_standard(D);

	cout << "end DTW\n";

	return 0;

}*/