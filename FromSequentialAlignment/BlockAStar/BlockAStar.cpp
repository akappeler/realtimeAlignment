/***********************************************************************************
 Block A-Star Pathfinding Algorithm
 
 Created:
 TCL Research America
 Armin Kappeler, 07/30/2013

 Modification:
 
 ************************************************************************************/

#include "BlockAStar.h"

//Initialization
point BlockAStar::move[] = {point(1,0),point(0,1),point(1,1)}; //possible movements

/***********************************************************************************/
BlockAStar::BlockAStar(void)
{
	useMask = false;
	initDone = false;
}

/***********************************************************************************/
BlockAStar::~BlockAStar(void)
{
	delete updated[0];
	delete updated[1];
	delete updated[2];
}

/***********************************************************************************/
void BlockAStar::init(FEAT_DATATYPE* seq1,FEAT_DATATYPE* seq2,int sizeSeq1, int sizeSeq2, int sizeFeat, AlignSettings cfg)
{
	//init D
#ifdef DIAGONAL_AVERAGING
	D.init(seq1,seq2,sizeSeq1,sizeSeq2,sizeFeat,cfg.blockSize,cfg.diagonalAvgLength);
#else
	D.init(seq1,seq2,sizeSeq1,sizeSeq2,sizeFeat);
#endif //DIAGONAL_AVERAGING

	size[0] = sizeSeq1;
	size[1] = sizeSeq2;

	//set parameters	
	blockSize = cfg.blockSize;
	gridSize[0] = (int)ceil((float)sizeSeq1/blockSize); 
	gridSize[1] = (int)ceil((float)sizeSeq2/blockSize);

	fCostPerMove = cfg.costpMove*blockSize;
	dtwCostPerNondiagMove = cfg.costpNondiagMove;

	//calculate heuristic cost per move (admissible only if fCostPerMoveFactor <= 1)
	//fCostPerMove = fCostPerMoveFactor * abs(size[0]-size[1])*dtwCostPerNondiagMove/max(size[0],size[1]);

	diagonalAvgLength = cfg.diagonalAvgLength;
	initDone = true;

}

/***********************************************************************************/
void BlockAStar::setMask(unsigned char* maskPtr, int rows, int cols)
{
	mask.assign(maskPtr,rows,cols);
	useMask = true; 
}

/***********************************************************************************/
vector<point> BlockAStar::findPath()
{
	//parameter check
	if (!initDone) {
		printf("Error: BlockAStar not initialized. Call init() first!\n");
		exit(0);
	}	

	if(useMask) {
		if((mask.rows != gridSize[0]) || (mask.cols != gridSize[1])) {
			printf("Warning: mask dimension not correct. Mask will not be used!\n");
			useMask = 0;
		}
	}

	//start alignment
	return startProcess();
}  

/***********************************************************************************/
FLOAT BlockAStar::getRealCost(point current,int currentIdx)
{
	FLOAT cost;

    int BS_rows = min(blockSize, size[0]-current.first*blockSize);
    int BS_cols = min(blockSize, size[1]-current.second*blockSize);
    
    //point move = current-prev;
    if (currentIdx == -1){ //calculate current block
		cost = dtw(current.first*blockSize,current.second*blockSize,BS_rows,BS_cols,current);
	}
    else{ //update current block
        cost = dtw_update(current.first*blockSize,current.second*blockSize,BS_rows,BS_cols,current,gScore[currentIdx]);
	}

	return cost;
}
/***********************************************************************************/
FLOAT BlockAStar::dtw(int firstR,int firstC,int sizeR,int sizeC,point current)
{
	int startR = firstR;
	int startC = firstC;
	int endR = startR + sizeR - 1;
	int endC = startC + sizeC - 1;

	int n,m;

	bool updatedSide[3] = {updated[0]->get(current),updated[1]->get(current),updated[2]->get(current)};

	//update first corner (if it is not the start point, then at least one update has to be 1)
	if ((startC == 0)&&( startR == 0)) {
		G.set(startR,startC,D.getCost(0,0));
	} else {
		G.set(startR,startC,INF);
	}

	if (updatedSide[2]==1) { //diag
		G.set(startR,startC,min(G.get(startR-1,startC-1)+D.getCost(startR,startC),G.get(startR,startC)));
	}
	if (updatedSide[0]==1) { //vert
		G.set(startR,startC,min(G.get(startR-1,startC)+dtwCostPerNondiagMove,G.get(startR,startC)));
	}
	if (updatedSide[1]==1) { //horiz
		G.set(startR,startC,min(G.get(startR,startC-1)+dtwCostPerNondiagMove,G.get(startR,startC)));
	}

	//calculate edge distances
	if ((startC == 0 || updatedSide[1]==0)&&( startR == 0 || updatedSide[0]==0)) //ignore both sides
	{
		for (n=startR+1;n<=endR;n++) {
			G.set(n,0,G.get(n-1,0)+dtwCostPerNondiagMove);
		}
		for (m=startC+1;m<=endC;m++) {
			G.set(0,m,G.get(0,m-1)+dtwCostPerNondiagMove);
		}
		startC = startC+1;
		startR = startR+1;
	}
	else if (startC == 0 || updatedSide[1]==0) {//ignore horizontal side
		for (n=startR+1;n<=endR;n++) {
			G.set(n,startC,G.get(n-1,startC)+dtwCostPerNondiagMove);
		}
		startC = startC+1;
	}
	else if (startR == 0 || updatedSide[0]==0) {//ignore vertical side
		for (m=startC+1;m<=endC;m++) {
			G.set(startR,m,G.get(startR,m-1)+dtwCostPerNondiagMove);
		}
		startR = startR+1;
	}
    
	for (n=startR;n<=endR;n++) {
		for (m=startC;m<=endC;m++) {
			G.set(n,m,min(min(G.get(n-1,m)+ dtwCostPerNondiagMove,G.get(n-1,m-1)+D.getCost(n,m)),G.get(n,m-1)+dtwCostPerNondiagMove));
		}
	}

	//return minimum value
	//minDist=min(min(G(endR,startC:endC)),min(G(startR:endR,endC)));

	//return minimum value per side
#ifdef SIMPLEHEURISTIC
	FLOAT uScalar = fCostPerMove*dtwCostPerNondiagMove;
#else
	FLOAT uScalar = ONE_OVER_SQRT2 * fCostPerMove; //1 dimension of unit vector
#endif //SIMPLEHEURISTIC

	//search along the border
	FLOAT tmpDist,tmpHeur;
	FLOAT minDist = G.get(endR,endC); //corner

	for (m = 0;m<sizeC;m++) { //last row
		tmpHeur = uScalar * (sizeC - m - 1)/blockSize;
		tmpDist = G.get(endR,firstC+m) + tmpHeur;
		if (tmpDist<minDist) {
			minDist = tmpDist;
		}
	}

	for (n = 0;n<sizeR;n++) { //last column
		tmpHeur = uScalar * (sizeR - n - 1)/blockSize;
		tmpDist = G.get(firstR+n,endC) + tmpHeur;
		if (tmpDist<minDist) {
			minDist = tmpDist;
		}
	}

	//set update flags
	setUpdated(current);

	return minDist;
}
/***********************************************************************************/
FLOAT BlockAStar::dtw_update(int firstR,int firstC,int sizeR,int sizeC,point current,FLOAT minCost)
{
	int startR = firstR;
	int startC = firstC;
	int endR = startR + sizeR - 1;
	int endC = startC + sizeC - 1;

	int n,m;
    FLOAT sumLastColPrev,sumLastRowPrev,lastCellPrev;

	//check, if any of the sides are updated
	/*if (isUpdated(current)==0) { //if not, exit
		setUpdated(current,0,0,0); //no updates done
		return minCost;
	}*/


	lastCellPrev = G.get(endR,endC);

	sumLastColPrev = 0;
	for (n=startR;n<=endR;n++) {
		sumLastColPrev += G.get(n,endC);
	}
	sumLastRowPrev = 0;
	for (m=startC;m<=endC;m++) {
		sumLastRowPrev += G.get(endR,m);
	}

	for (n=startR;n<=endR;n++) {
		for (m=startC;m<=endC;m++) {
			//G.set(n,m,min(min(G.get(n-1,m)+ dtwCostPerNondiagMove,G.get(n-1,m-1)+D.getCost(n,m)),G.get(n,m-1)+dtwCostPerNondiagMove));
			G.set(n,m,min(min(G.get(n-1,m)+ dtwCostPerNondiagMove,G.get(n-1,m-1)+D.getCost(n,m)),min(G.get(n,m-1)+dtwCostPerNondiagMove,G.get(n,m))));
		}
	}

	//return minimum value
	//minDist=min(min(G(endR,startC:endC)),min(G(startR:endR,endC)));

	//return minimum value per side
#ifdef SIMPLEHEURISTIC
	FLOAT uScalar = fCostPerMove*dtwCostPerNondiagMove;
#else
	FLOAT uScalar = ONE_OVER_SQRT2 * fCostPerMove; //1 dimension of unit vector
#endif //SIMPLEHEURISTIC

	//search along the border
	FLOAT tmpDist,tmpHeur;
	FLOAT minDist = G.get(endR,endC); //corner
    FLOAT sumLastCol,sumLastRow,lastCell;

	lastCell = G.get(endR,endC);
	sumLastRow = 0;
	for (m = 0;m<sizeC;m++) { //last row
		tmpHeur = uScalar * (sizeC - m - 1)/blockSize;
		tmpDist = G.get(endR,firstC+m) + tmpHeur;
		if (tmpDist<minDist) {
			minDist = tmpDist;
		}
		sumLastRow += G.get(endR,firstC+m);
	}

	sumLastCol = 0;
	for (n = 0;n<sizeR;n++) { //last column
		tmpHeur = uScalar * (sizeR - n - 1)/blockSize;
		tmpDist = G.get(firstR+n,endC) + tmpHeur;
		if (tmpDist<minDist) {
			minDist = tmpDist;
		}
		sumLastCol += G.get(firstR+n,endC);
	}

	//set update flags
	//setUpdated(current,sumLastRow<sumLastRowPrev,sumLastCol<sumLastColPrev,lastCell<lastCellPrev);
	setUpdated(current);

	return minDist;
}
/***********************************************************************************/
//heuristic cost = distance projected to diagonal (slope) 
//by calculating inner product with unit diagonal vector: [1/sqrt(2), 1/sqrt(2)]
//-> promotes diagonal movement multiplied by costPerMove
FLOAT BlockAStar::getHeuristicCost(point start,point goal)
{
	point tmp = (goal-start);
#ifdef SIMPLEHEURISTIC
	return abs(tmp.first-tmp.second)*dtwCostPerNondiagMove*fCostPerMove;
#else
	return (tmp.first + tmp.second)*ONE_OVER_SQRT2*fCostPerMove;
#endif //SIMPLEHEURISTIC

}
/***********************************************************************************/
// finds the cheapest path from the end to the beginning by following the lowest cost 
//in the G matrix, starting from the corner in the last row and last column
vector<point> BlockAStar::backtrack()
{
	int N = size[0]-1;
	int M = size[1]-1;

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
			//ev. TODO: resolve situation where (n-1,m-1) == (n-1,m) or similar
			if ((G.get(n-1,m-1)+D.getCost(n,m) < G.get(n,m-1)+dtwCostPerNondiagMove) && (G.get(n-1,m-1)+D.getCost(n,m) < G.get(n-1,m)+dtwCostPerNondiagMove)) 
			{
				n = n-1;
				m = m-1;
			} else if (G.get(n-1,m) <= G.get(n,m-1)) {
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
/***********************************************************************************/
vector<point> BlockAStar::startProcess()
{
	////////////////////////////////////////////////////////////////////////////
	//initialize variables
	point start(0,0);
	point goal(gridSize[0]-1,gridSize[1]-1);
	int nodepointer = 0;

	int currentIdx;
	point current(0,0);

	int neighborIdx;
	point neighbor(0,0);

	int i;
	FLOAT tentativeScore; //helper variables

	int gridElements = gridSize[0]*gridSize[1];
	int allocSize = (int)floor(PREALLOC_FACTOR*gridElements);

	//G.create(size[0],size[1]);				//contains the accumulated values
	G.create(size[0],size[1],blockSize);				//contains the accumulated values
	nodeMap.create(-1,gridSize[0],gridSize[1]);	//matrix that contains the index
			
	updated[0] = new Matrix<bool>(false,gridSize[0]+1,gridSize[1]+1); //update flags
	updated[1] = new Matrix<bool>(false,gridSize[0]+1,gridSize[1]+1); //+1, so that no border check necessary
	updated[2] = new Matrix<bool>(false,gridSize[0]+1,gridSize[1]+1);

	nodes.reserve(allocSize);
	cameFrom.reserve(allocSize);
	gScore.reserve(allocSize);
	//fScore.reserve(allocSize);

	MinHeap<FLOAT> openSet((int)(allocSize/PREALLOC_FACTOR));			//openset - priority queue
	#ifdef USE_CLOSEDSET
	Matrix<bool> closedSet(gridSize[0],gridSize[1]);	///closedset - matrix
	#endif //USE_CLOSEDSET

	////////////////////////////////////////////////////////////////////////////
	//intialize start values
	nodes.push_back(start);
	cameFrom.push_back(-1);
	FLOAT tempGS = getRealCost(start,-1);
	gScore.push_back(tempGS);
	nodeMap.set(start,nodepointer);
	openSet.push(tempGS+getHeuristicCost(start,goal),nodepointer);

	////////////////////////////////////////////////////////////////////////////
	//start iterations
	while (!openSet.empty())
	{
		//get node with lowest cost
		currentIdx = openSet.pop(); 
		current = nodes[currentIdx];

		//check if we are at goal
		if (current==goal) {
			break;
		}

		#ifdef USE_CLOSEDSET
		///add to closed set
		closedSet.set(current,1);
		#endif //USE_CLOSEDSET
			
		//evaluate each neighbor
		for (i=0;i<3;i++)
		{
			neighbor = current + move[i];

			//border check
			if ((neighbor.first >= gridSize[0]) || (neighbor.second >= gridSize[1])){
				continue;
			}

			//check mask
			if (useMask) {
				if (mask.get(neighbor)==0) {
					continue;
				}
			}

			#ifdef USE_CLOSEDSET
			///check closed set
			if (closedSet.get(neighbor) == 1) {
				continue; //do nothing for this neighbor
			}
			#endif //USE_CLOSEDSET
			neighborIdx = nodeMap.get(neighbor);
 			tentativeScore = getRealCost(neighbor,neighborIdx);

			if (neighborIdx == -1) //unwalked coordinates - not in the openlist
			{
				nodepointer++,
				neighborIdx = nodepointer;
				nodeMap.set(neighbor,neighborIdx);	
				nodes.push_back(neighbor);
				cameFrom.push_back(currentIdx);                          
				gScore.push_back(tentativeScore);

				cout << "\r" << nodepointer+1 << " Blocks of maximal " <<gridElements<< " calculated";

				//add to openlist
				openSet.push(tentativeScore + getHeuristicCost(neighbor, goal),neighborIdx);
			}
			else if (tentativeScore < gScore[neighborIdx]) //new value is better than old one
			{
				nodes[neighborIdx] = neighbor;
				cameFrom[neighborIdx] = currentIdx;                          
				gScore[neighborIdx] = tentativeScore;

				//update value in openlist
				openSet.push(tentativeScore + getHeuristicCost(neighbor, goal),neighborIdx);           
			}
		}


	}
	
	cout <<  "\r" << nodepointer+1 << " Blocks of maximal " <<gridElements<< " calculated\n";
	LOG_totalBlocks = gridElements;
	LOG_calcBlocks = nodepointer+1;

	////////////////////////////////////////////////////////////////////////////
	//Backtrack

	vector<point> path = backtrack();
	return path;
};