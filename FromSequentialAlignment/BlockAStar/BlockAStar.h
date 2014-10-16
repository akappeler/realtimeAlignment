/***********************************************************************************
 BlockAStar Path Finding Implementation

 Created:
 Armin Kappeler, 07/30/2013

 Modification:
 
************************************************************************************/
#ifndef BLOCKASTAR_H_
#define BLOCKASTAR_H_

#include <vector>

#include "MyHeap.h"
#include "AStarMatrix.h"

//#define USE_CLOSEDSET   //closed set implementation of A*
//#define DIAGONAL_AVERAGING //diagonal averaging of cost matrix -> results were not convincing, therefore deactivated
#define INF 3.4e30f
#define ONE_OVER_SQRT2 0.707106781186548f
#define PREALLOC_FACTOR 1.0f //array length for preallocated memory (1 = maximal possible memory reservation)
#define SIMPLEHEURISTIC

typedef float FLOAT;			//A* precision, has to be floating point -> change to double, if not accurate enough
typedef float FEAT_DATATYPE;	//datatype of the input feature vector, can be any type

using namespace std;

struct AlignSettings
{
	int blockSize;			 // sequences are divided in blocks of this size
	float costpMove;		 //heuristic cost per cell
	float costpNondiagMove; //"penalty for nondiagonal movement"
	int diagonalAvgLength;

	AlignSettings() { //set default values
		blockSize=128;
		costpMove=1.0f;
		costpNondiagMove=1.0f;
		diagonalAvgLength = 1; //not used
	}
};


/***********************************************************************************/
//A* class

class BlockAStar
{
private:
	//parameters
	int blockSize;
	bool useMask;
	bool initDone;

	FLOAT fCostPerMove;
	FLOAT dtwCostPerNondiagMove;
	int diagonalAvgLength;

	//variables
#ifdef DIAGONAL_AVERAGING
	costMatrix_diagAveraging<FEAT_DATATYPE> D;
#else
	costMatrix<FEAT_DATATYPE> D;//contains the cost function
#endif //DIAGONAL_AVERAGING

	int size[2];				//size of D and G (feature array lengths)

	Matrix<unsigned char> mask;
	int gridSize[2];

	static point move[3];

	//Matrix<FLOAT> G;			//contains the accumulated values
	DynamicMatrix<FLOAT> G;		//contains the accumulated values
	Matrix<int> nodeMap;		//matrix that contains the index

	Matrix<bool> * updated[3];	//update flags

	vector<point> nodes;		//matrix coordinates, row
	vector<int> cameFrom;		//previous node index
	vector<FLOAT> gScore;		//real cost
	//vector<FLOAT> fScore;		//heuristic cost -> not needed because saved in openSet

	//main process
	vector<point> startProcess();
	FLOAT getRealCost(point current,int currentIdx);
	FLOAT getHeuristicCost(point start,point goal);
	FLOAT dtw(int firstR,int firstC,int sizeR,int sizeC,point current);
	FLOAT dtw_update(int firstR,int firstC,int sizeR,int sizeC,point current,FLOAT minCost);
	vector<point> backtrack();

	inline void setUpdated(point pt) { //sets the updated flag at all neighbor blocks
		updated[0]->set(pt+move[0],1); //vertical
		updated[1]->set(pt+move[1],1); //horizonzal
		updated[2]->set(pt+move[2],1); //diagonal
		updated[0]->set(pt,0);
		updated[1]->set(pt,0);
		updated[2]->set(pt,0);
	};

	inline void setUpdated(point pt,bool v,bool h, bool d) { //sets the updated flag at all neighbor blocks
		updated[0]->set(pt+move[0],v); //vertical
		updated[1]->set(pt+move[1],h); //horizonzal
		updated[2]->set(pt+move[2],d); //diagonal
		updated[0]->set(pt,0);
		updated[1]->set(pt,0);
		updated[2]->set(pt,0);
	};

	inline bool isUpdated(point pt) { //checks, if any update flag is set
		return (updated[0]->get(pt) || updated[1]->get(pt) || updated[2]->get(pt));
	};

public:
	BlockAStar(void);
	~BlockAStar(void);

	//initialization
	void init(FEAT_DATATYPE* seq1,FEAT_DATATYPE* seq2,int sizeSeq1, int sizeSeq2, int sizeFeat, AlignSettings cfg);
	void setMask(unsigned char* maskPtr, int rows, int cols); //optional

	//find path
	vector<point> findPath();
	long LOG_totalBlocks,LOG_calcBlocks;
};

#endif /*BLOCKASTAR_H_*/