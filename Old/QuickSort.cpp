#include "QuickSort.h"
#include "MPI/mpi.h"

#include <iostream>
#include <algorithm>
#include <time.h>

using namespace std;

QuickSort::QuickSort()
{

}

void QuickSort::findHighLowElements(const int pivot,
                               const vector<int> &array,
                               vector<int> &high,
                               vector<int> &low)
{
    for (int element : array)
    {
        if(element < pivot)
        {
            low.push_back(element);
        }
        if(element >= pivot)
        {
            high.push_back(element);
        }
    }
}

int QuickSort::getOpponent(const int& rank, const int& localCommSize)
{
    int tmp = (int)(log(localCommSize)/log(2));
    return rank ^ (0x1 << (tmp - 1));
}

void QuickSort::getVec(vector<int>& vec, const int& opponent, const int& message, const MPI_Comm& comm)
{
    MPI_Status status;
    MPI_Probe(opponent, opponent, comm, &status);

    int count;
    MPI_Get_count(&status, MPI_INT, &count);

    vec.resize(count);
    MPI_Recv(vec.data(), count, MPI_INT, opponent, message, comm, &status);
}

void QuickSort::sentVec(vector<int>& vec, const int& opponent, const int& message, const MPI_Comm& comm)
{
    MPI_Request request;
    MPI_Isend(vec.data(), vec.size(), MPI_INT, opponent, message, comm, &request);

    vec.clear();
}

void QuickSort::sentVecGetVec(vector<int>& vecSent, vector<int>& vecGet,const int& rank, const int& opponent, const MPI_Comm& comm)
{
    sentVec(vecSent, opponent, rank, comm);

    vector <int> input;
    getVec(input, opponent, opponent, comm);
    vecGet.insert(vecGet.end(), input.begin(), input.end());

    input.clear();
}

void QuickSort::exchange(vector<int>& low, vector<int>& high, const int& rank, int localCommSize, MPI_Comm& comm)
{
    int opponent = getOpponent(rank, localCommSize);

    if (rank < opponent)
    {
        sentVecGetVec(high, low, rank, opponent, comm);
    }
    else
    {
        sentVecGetVec(low, high, rank, opponent, comm);
    }
}

int QuickSort::calcPivot(const vector<int> &vec)
{
    if(!vec.empty())
    {
        return vec[rand() % vec.size()];
    }
    else return 0;
}

void QuickSort::updateProcArray(vector<int>& low, vector<int>& high, vector<int>& procArray)
{
    procArray.clear();

    if(!high.empty())
    {
        procArray = high;
        high.clear();
    }
    else
    {
        procArray = low;
        low.clear();
    }
}
int* calcOffsets(int columnProc, const int* counts)
{
    int* offsets = new int[columnProc];
    offsets[0] = 0;
    for (int i = 1; i< columnProc; i++)
    {
        offsets[i] = offsets[i - 1] + counts[i - 1];
    }
    return offsets;
}

vector<int> QuickSort::sort(vector <int>& mainArray)
{
    if (mainArray.empty())
    {
        cout << "ERROR: Vector is empty!" << endl;
        return mainArray;
    }

    srand(time(NULL));

    int columnProc;         // Кол-во проц.
    int rank;               // Ранг текущего проц.
    int procArraySize;      // Количество элементов на 1 проц. (требуется для начального деления массива)

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &columnProc);

    if (columnProc % 2 != 0)
    {
        cout << "ERROR: proc. column not even!" << endl;
        return mainArray;
    }

    procArraySize = mainArray.size() / columnProc;

    vector<int> procArray(procArraySize);

    // Делим исходный массив на равные части всем проц.

    MPI_Scatter(mainArray.data(), procArraySize, MPI_INT,
                procArray.data(), procArraySize, MPI_INT,
                0, MPI_COMM_WORLD);

    int pivot;              // Опорынй элемент
    vector <int> high;      // Элементы больше опорного элемента
    vector <int> low;       // Элементы меньше опорного элемента

    int iterCount = (int)(log(columnProc) / log(2));  // Кол-во итерация для сортировки
    vector<MPI_Comm> commArray(iterCount);            // Вектор коммуникаторов

    MPI_Barrier(MPI_COMM_WORLD);
    int localRank;
    int localCommSize;
    int color;

    for (int iter = 0; iter < iterCount; iter++)
    {
        color = rank / (columnProc / (pow(2, iter)));

        //Разбираем проецссы на коммуникаторы

        if (iter == 0)
        {
            MPI_Comm_rank(MPI_COMM_WORLD, &localRank);
            MPI_Comm_split(MPI_COMM_WORLD, color, localRank, &commArray[iter]);
        }
        else
        {
            MPI_Comm_rank(commArray[iter - 1], &localRank);
            MPI_Comm_split(commArray[iter - 1], color, localRank, &commArray[iter]);
        }

        MPI_Comm_rank(commArray[iter], &localRank);
        MPI_Comm_size(commArray[iter], &localCommSize);

        //Расчитали новое опорное число
        if (localRank == 0)
        {
            pivot = calcPivot(procArray);
        }

        //Отправили всем

        MPI_Bcast(&pivot, 1, MPI_INT, 0, commArray[iter]);

        MPI_Comm_rank(commArray[iter], &localRank);
        MPI_Comm_size(commArray[iter], &localCommSize);

        //Нашли элементы меньше и больше опорного элемента
        findHighLowElements(pivot, procArray, high, low);

        // Обменялись с соседними проц.
        exchange(low, high, localRank, localCommSize, commArray[iter]);

        updateProcArray(low, high, procArray);
    }
    // Высвобождаем все комутаторы

    for( int i = 0; i < commArray.size(); i++)
    {
        MPI_Comm_free(&commArray[i]);
    }

    std::sort(procArray.begin(), procArray.end());

    int* counts = new int[columnProc];
    int localCount = procArray.size();

    MPI_Gather(&localCount, 1, MPI_INT, counts, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int* offsets ;
    if (rank == 0)
    {
        offsets = calcOffsets(columnProc, counts);
    }

    // Собираем куски

    MPI_Gatherv(procArray.data(), procArray.size(), MPI_INT, mainArray.data(), counts, offsets, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Finalize();
}
