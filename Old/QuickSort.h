#ifndef QUICKSORT_H
#define QUICKSORT_H

#include <vector>
#include <MPI/mpi.h>

using namespace std;

class QuickSort
{
private:
    void findHighLowElements(const int pivot,
              const vector<int>& array,
              vector <int>& high,
              vector <int>& low);
    void exchange(vector<int>& low, vector<int>& high, const int& rank, int localCommSize, MPI_Comm& comm);
    void sentVec(vector<int>& vec, const int& opponent, const int& message, const MPI_Comm& comm);
    void getVec(vector<int>& vec, const int& opponent, const int& message, const MPI_Comm& comm);
    void sentVecGetVec(vector<int>& vecSent, vector<int>& vecGet,const int& rank, const int& opponent, const MPI_Comm& comm);
    void updateProcArray(vector<int>& low, vector<int>& high, vector<int>& procArray);

    void exchangeVec(vector<int>& input, vector<int>& sent,int rank, int opponent);

    int calcPivot(const vector<int> &vec);
    int getOpponent(const int &rank, const int &localCommSize);



public:
    QuickSort();
    vector<int> sort(vector<int> &mainArray);
};


#endif // QUICKSORT_H
