#include "QuickSort.h"
#include <vector>
#include <ctime>
#include <MPI/mpi.h>
#include <algorithm>
#include <math.h>
#include <iostream>
using namespace std;


QuickSort::QuickSort()
{
}


QuickSort::~QuickSort()
{
}

double* QuickSort::run(const Vector& vec)
{
    // ������������� MPI

    int errCode;
    MPI_Request request, request2;
    MPI_Status status;

    if ((errCode = MPI_Init(0, 0)) != 0)
    {
        printf("errCode\n");
    }

    //Intracomm comm = MPI.COMM_WORLD;

    int threadCount, rank;

    //������������ MPI_COMM_WORLD ��������� �� ��������� � ������������ ��� �������� ����������� ������������ ��������� ;
    //����������� ���������� ��������� � ����������� ������������ ���������:
    MPI_Comm_size(MPI_COMM_WORLD, &threadCount);
    //����, ���������� ��� ������ ������� MPI_Comm_rank, �������� ������ ��������, ������������ ����� ���� �������,
    //�. �. ���������� ProcRank ������ ��������� �������� � ������ ���������.
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // ���������� ������ � ����������� ������� �����

    int sizeInput = vec.m_Size;//16;
    int* packChunkSize = new int[1];
    double* input = vec.m_Vector;//new double[sizeInput] { 0, 1, 2, 5, 7, 3, 4, 7, 3, 1, 5, 7, 3, 46, 67, 678 };

    if (rank == 0)
    {
        int N = sizeInput;
        packChunkSize[0] = N / threadCount;
    }

    clock_t begin = clock();

    // ��������� ����� ���������

    MPI_Bcast(packChunkSize, 1, MPI_INT, 0, MPI_COMM_WORLD); //�������� �� ������ �������� ���� ��������� (����������������� ��������)


    int chunkSize = packChunkSize[0];

    if (rank != 0) {
        input = new double[chunkSize * threadCount];
    }

    double* local = new double[chunkSize];
    int localSize = chunkSize;

    MPI_Scatter(input, chunkSize, MPI_DOUBLE, local, chunkSize, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // �������� ���������, ���������� ������� �������
    std::sort(local, local + localSize, mycomparefunction);

    double* packCoreElement = new double[1];

    if (rank == 0) {
        packCoreElement[0] = local[0];
    }

    MPI_Bcast(packCoreElement, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD); //�������� �� ������ �������� ���� ��������� (����������������� ��������)
    double core = packCoreElement[0];

    // ��������� �������� ������

    int iterCount = (int)(log(threadCount) / log(2));

    for (int iter = 0; iter < iterCount; iter++)
    {
        int coreIndex = std::binary_search(local, local + localSize, core);
        if (coreIndex < 0)
        {
            coreIndex = coreIndex * (-1) - 1;
        }
        int opponent = getOpponent(rank, iter);


        if (rank < opponent)
        {
            MPI_Isend(local + coreIndex, localSize - coreIndex, MPI_DOUBLE, opponent, rank, MPI_COMM_WORLD, &request);
            memcpy(local, local, coreIndex);

            MPI_Status status1;
            MPI_Probe(opponent, opponent, MPI_COMM_WORLD, &status1);
            int count;
            MPI_Get_count(&status1, MPI_DOUBLE, &count);

            double* newarr = new double[count];

            MPI_Recv(newarr, count, MPI_DOUBLE, opponent, opponent, MPI_COMM_WORLD, &status1);
            local = mergeArrays(local, newarr, sizeInput, count);
            localSize = sizeInput + count;

        }
        else
        {
            MPI_Isend(local, coreIndex, MPI_DOUBLE, opponent, rank, MPI_COMM_WORLD, &request);

            memcpy(local, local + coreIndex, localSize);

            MPI_Status status1;
            MPI_Probe(opponent, opponent, MPI_COMM_WORLD, &status1);
            int count;
            MPI_Get_count(&status1, MPI_DOUBLE, &count);
            double* newarr = new double[count];
            MPI_Recv(newarr, count, MPI_DOUBLE, opponent, opponent, MPI_COMM_WORLD, &status1);
            local = mergeArrays(local, newarr, sizeInput, count);
            localSize = sizeInput + count;
        }

    }

    // �������� ���� �������� ������� ��������� ������ � ���������� ������� ��� ������� �� � �������������� ������

    int* counts = new int[threadCount];
    int* localCount = new int [1] { localSize };
    MPI_Gather(localCount, 1, MPI_INT, counts, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int* offsets = new int[threadCount];
    if (rank == 0) {
        offsets[0] = 0;
        for (int i = 1; i< threadCount; i++) {
            offsets[i] = offsets[i - 1] + counts[i - 1];
        }
    }

    // �������� �����

    MPI_Gatherv(local, localSize, MPI_DOUBLE, input, counts, offsets, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    clock_t end = clock();

    if (rank == 0) {
        double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

        std::cout << elapsed_secs;
    }

//    MPI_Wait(&request, &status);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return input;
}


