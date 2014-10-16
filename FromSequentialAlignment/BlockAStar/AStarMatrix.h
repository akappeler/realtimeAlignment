/***********************************************************************************
 Matrix.h : very basic matrix handling class

 Created:
 Armin Kappeler, 07/30/2013

 Modification:
 ************************************************************************************/
#ifndef ASTARMATRIX_H
#define ASTARMATRIX_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <utility>

#define INF 3.4e30f
#define LOW_MEMORY //don't save values in Matrix -> recalculate every time
#define L1_NORM

typedef float FLOAT;

using namespace std;

/***********************************************************************************/
//used for coordinates in alignment matrix
class point:public pair<int,int>
{
private:
public:
	point(int pt1,int pt2):pair(pt1,pt2)
	{
	}

	point():pair()
	{
	}

	inline point operator-=(const point &pt2) {
		return point(first-pt2.first,second-pt2.second);
	}

	inline point operator-(const point &pt2) {
		return point(first-pt2.first,second-pt2.second);
	}

	inline point operator+=(const point &pt2) {
		return point(first+pt2.first,second+pt2.second);
	}

	inline point operator+(const point &pt2)  {
		return point(first+pt2.first,second+pt2.second);
	}

	inline point operator+(const int val) {
		return point(first+val,second+val);
	}

	inline point operator-(const int val) {
		return point(first-val,second-val);
	}

	inline int operator*(const point &pt2) {
		return first*pt2.first + second*pt2.second;
	}
	inline point operator/(const int val) {
		return point(first/val,second/val);
	}

	inline double norm() {
		return sqrt(first*first + second*second);
	}

	inline void transpose() { //switch coordinates
		int tmp = first;
		first = second;
		second = tmp;	
		return;
	}
};

/***********************************************************************************/
//fast matrix implementation
template <class T> class Matrix {
private:
	T* val;
public:
	//variables
	int rows;
	int cols;
	
	//member functions
	Matrix(void)
	{
		createdInside = false;
	};

	Matrix (const int nrows,const int ncols)                  // init empty matrix
	{
		create (nrows,ncols);  
	};

	Matrix (T initValue, const int nRows,const int nCols)           // init matrix with val
	{
		create (initValue, nRows, nCols);
	};

	Matrix (T* values, const int nRows,const int nCols) // set matrix pointer to values
	{
		createdInside = false;
		rows = nRows;
		cols = nCols;
		val = values; //assign pointer, don't copy values
	};

	void create (const int nrows,const int ncols)                  // init empty matrix
	{
		createdInside = true;
		rows = nrows;
		cols = ncols;
		val = new T[rows*cols];
	};

	void create (T initValue, const int nRows,const int nCols)           // init matrix with val
	{
		createdInside = true;
		rows = nRows;
		cols = nCols;
		val = new T[nRows*nCols];
		for (int i=0; i<nRows*nCols;i++)
		{
			val[i] = initValue;
		}
	};

	void assign (T* values, const int nRows,const int nCols) // set matrix pointer to values
	{
		createdInside = false;
		rows = nRows;
		cols = nCols;
		val = values; //assign pointer, don't copy values
	};


	~Matrix()
	{
		if (createdInside) {
			delete [] val;
		}
	};

	inline T get(int row,int col )
	{
		return val[row+col*rows];
	};

	inline T* getPtr(int row,int col )
	{
		return &val[row+col*rows];
	};

	inline void set(int row,int col,T value)
	{
		val[row+col*rows] = value;
	};

	inline T get(point pt )
	{
		return val[pt.first+pt.second*rows];
	};

	inline T* getPtr(point pt )
	{
		return &val[pt.first+pt.second*rows];
	};

	inline void set(point pt ,T value)
	{
		val[pt.first+pt.second*rows] = value;
	};

protected:
	bool createdInside; //if the matrix was created in the class, free memory in deconstructor

private:

};

/***********************************************************************************/
//Same interface as matrix, but this one does only allocate as much memory as needed
template <class T>
class DynamicMatrix //similar to matrix, but reserves space dynamically, instead of alltogether
{

private:
	//variables
	int rows,cols;
	int blockSize;
	int brows,bcols; //how many blocks
	Matrix<T*> ptBlocks;
	T initVal;
	bool setInitVal;

public:
	//member functions
	DynamicMatrix()
	{
		setInitVal = false;
	};

	~DynamicMatrix()
	{
		for (int r = 0;r<ptBlocks.rows;r++)
		{
			for (int c = 0;r<ptBlocks.cols;r++)
			{
				T* ptr = ptBlocks.get(r,c);
				if (ptr != NULL)
				{
					delete [] ptr;
				}
			}
		}
	};


	void create (const int nrows,const int ncols, const int inBlockSize)                  // init empty matrix
	{
		blockSize = inBlockSize;
		rows = nrows;
		cols = ncols;
		brows = rows/blockSize+1;
		bcols = cols/blockSize+1;
		ptBlocks.create(NULL,brows,bcols); //create matrix of pointers
	};

	void create (const T initValue, const int nrows,const int ncols, const int inBlockSize)                  // init empty matrix
	{
		blockSize = inBlockSize;
		rows = nrows;
		cols = ncols;
		brows = rows/blockSize+1;
		bcols = cols/blockSize+1;
		ptBlocks.create(NULL,brows,bcols); //create matrix of pointers
		setInitVal = true;
		initVal = initValue;
	};

	inline T get(int row,int col )
	{
		int pRow = row/blockSize; //block index
		int pCol = col/blockSize;
		int bRow = row-pRow*blockSize; //block-intern index
		int bCol = col-pCol*blockSize;

		T* val = ptBlocks.get(pRow,pCol);

		if (val != NULL)
		{
			T tmp = val[bRow+bCol*blockSize];
			return val[bRow+bCol*blockSize];
		}
		else 
		{
			return INF; //value not assigned yet
		}
	};

	inline void set(int row,int col,T value)
	{
		int pRow = row/blockSize; //block index
		int pCol = col/blockSize;
		int bRow = row-pRow*blockSize; //block-intern index
		int bCol = col-pCol*blockSize;

		T* val = ptBlocks.get(pRow,pCol);	
		if (val == NULL) //allocate memory for new block
		{
			val = new T[blockSize*blockSize];
			ptBlocks.set(pRow,pCol,val);
			if (setInitVal == true) 
			{
				for (int i=0; i<blockSize*blockSize;i++)
				{
					val[i] = initVal;
				}
			}
		}
		val[bRow+bCol*blockSize] = value;
	};

};

/************************************************************************************/
//special class, that calculates the value of the cost matrix in runtime
template <class Tcm>
class costMatrix: public Matrix<float>
{
private:
	Matrix<Tcm>* seq1;
	Matrix<Tcm>* seq2;
	bool initCalled;

public:
	costMatrix() 
	{
		initCalled = false;
	};

	~costMatrix() 
	{
		if (initCalled == true) {
			delete seq1;
			delete seq2;
		}
	};

	void init(Tcm* sequence1, Tcm* sequence2,int nCols1,int nCols2,int nRows)
	{
		initCalled = true;

		seq1 = new Matrix<Tcm>(sequence1,nRows,nCols1);
		seq2 = new Matrix<Tcm>(sequence2,nRows,nCols2);

		if (seq1->rows != seq2->rows) {
			cout << "EROOR: features don't have the same dimensions!";
		}

		rows = seq1->cols;
		cols = seq2->cols;

#ifndef LOW_MEMORY
		//val = new FLOAT [rows*cols]; 
		create (-1, rows,cols);
#endif

	};

	//calculates cost values
#ifdef LOW_MEMORY
	FLOAT getCost(int idx1,int idx2) 
	{
		Tcm * ptr1 = seq1->getPtr(0,idx1);
		Tcm * ptr2 = seq2->getPtr(0,idx2);

		//calculate euclidean distance
		FLOAT norm = 0;
		for (int i=0; i < seq1->rows; i++) {
			//norm += (FLOAT)pow(ptr1[i] - ptr2[i],2);
			Tcm tmp = *ptr1 - *ptr2;
			ptr1++;
			ptr2++;		
#ifdef L1_NORM 
			norm += (FLOAT)fabs(tmp);
#else
			norm += (FLOAT)tmp*tmp;
#endif //L1_NORM
		}

		//
		return norm;
		//return sqrt(norm); 
	};
#else
	FLOAT getCost(int idx1,int idx2) 
	{
		FLOAT norm = get(idx1,idx2);
		if (norm == -1) //check, if current coordinates are already calculated
		{
			Tcm * ptr1 = seq1->getPtr(0,idx1);
			Tcm * ptr2 = seq2->getPtr(0,idx2);

			//calculate euclidean distance
			norm = 0;
			for (int i=0; i < seq1->rows; i++) {
				//norm += (FLOAT)pow(ptr1[i] - ptr2[i],2);
				Tcm tmp = *ptr1 - *ptr2;
				ptr1++;
				ptr2++;			
#ifdef L1_NORM
				norm += (FLOAT)abs(tmp);
#else
				norm += (FLOAT)tmp*tmp;
#endif //L1_NORM
			}
			set(idx1,idx2,norm);

		}	

		return norm;
		//return sqrt(norm); 
	};
#endif //LOW_MEMORY

};
/************************************************************************************/
//special class, that calculates the value of the cost matrix in runtime
//This class averages the values along the diagonal for the length defined in "diagonalAvgLength"
template <class Tcm>
class costMatrix_diagAveraging: public Matrix<float>
{
private:
	Matrix<Tcm>* seq1;
	Matrix<Tcm>* seq2;
	bool initCalled;
	int diagonalAvgLength;
	int diagOffset; // internal helper variable
	DynamicMatrix<Tcm> costValues;

public:
	costMatrix_diagAveraging() 
	{
		initCalled = false;
	};

	~costMatrix_diagAveraging() 
	{
		if (initCalled == true) {
			delete seq1;
			delete seq2;
		}
	};

	void init(Tcm* sequence1, Tcm* sequence2,int nCols1,int nCols2,int nRows,int blockSize, int diagAvgLength=1)
	{
		initCalled = true;

		seq1 = new Matrix<Tcm>(sequence1,nRows,nCols1);
		seq2 = new Matrix<Tcm>(sequence2,nRows,nCols2);

		if (seq1->rows != seq2->rows) {
			cout << "EROOR: features don't have the same dimensions!";
		}

		rows = seq1->cols;
		cols = seq2->cols;

		diagonalAvgLength = diagAvgLength;
		diagOffset = max((diagAvgLength-1)/2,0);

#ifndef LOW_MEMORY
		costValues.create(INF, rows,cols,blockSize);
#endif

	};

	//calculates cost values
#ifdef LOW_MEMORY
	FLOAT getSingleCost(int idx1,int idx2) 
	{
		Tcm * ptr1 = seq1->getPtr(0,idx1);
		Tcm * ptr2 = seq2->getPtr(0,idx2);

		//calculate euclidean distance
		FLOAT norm = 0;
		for (int i=0; i < seq1->rows; i++) {
			//norm += (FLOAT)pow(ptr1[i] - ptr2[i],2);
			Tcm tmp = *ptr1 - *ptr2;
			ptr1++;
			ptr2++;		
#ifdef L1_NORM
			norm += (FLOAT)abs(tmp);
#else
			norm += (FLOAT)tmp*tmp;
#endif //L1_NORM
		}

		//
		return norm;
	};
#else
	FLOAT getSingleCost(int idx1,int idx2) 
	{
		FLOAT norm = costValues.get(idx1,idx2);
		if (norm == INF) //check, if current coordinates are already calculated
		{
			Tcm * ptr1 = seq1->getPtr(0,idx1);
			Tcm * ptr2 = seq2->getPtr(0,idx2);

			//calculate euclidean distance
			norm = 0;
			for (int i=0; i < seq1->rows; i++) {
				//norm += (FLOAT)pow(ptr1[i] - ptr2[i],2);
				Tcm tmp = *ptr1 - *ptr2;
				ptr1++;
				ptr2++;			
#ifdef L1_NORM
				norm += (FLOAT)abs(tmp);
#else
				norm += (FLOAT)tmp*tmp;
#endif //L1_NORM
			}
			costValues.set(idx1,idx2,norm);

		}	
		
		return norm;
	};
#endif //LOW_MEMORY
	
	//calculates the average over the points along a diagonal line through point (idx1,idx2) of length 
	FLOAT getCost(int idx1,int idx2) 
	{
		int counter=0;
		FLOAT sum=0;

		for (int i = -diagOffset;i<=diagOffset;i++)
		{
			int tmpIdx1 = idx1+i;
			int tmpIdx2 = idx2+i;
			if (tmpIdx1>=0 && tmpIdx1<rows && tmpIdx2>=0 && tmpIdx2<cols) //range check
			{
				sum += getSingleCost(tmpIdx1,tmpIdx2);
				counter++;
			}
		}
		sum /= counter;
		return sum;
	};
	
};


#endif //ASTARMATRIX_H