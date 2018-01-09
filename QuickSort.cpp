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

bool mycomparefunction(double  a, double  b) { return ((a) < (b)); }

static int getOpponent(int rank, int iteration) {

    return rank ^ static_cast<int>(std::pow(2, iteration));

}

static double* mergeArrays(double* a, double* b, int sizeA, int sizeB) {
    int resultSize = sizeA + sizeB;
    double* result = new double[resultSize];
    int indexA = 0;
    int indexB = 0;

    for (int i = 0; i < resultSize; i++) {

        if (indexA == sizeA) {
            result[i] = b[indexB];
            indexB += 1;
        }
        else if (indexB == sizeB) {
            result[i] = a[indexA];
            indexA += 1;
        }
        else if (a[indexA] < b[indexB]) {
            result[i] = a[indexA];
            indexA += 1;
        }
        else {
            result[i] = b[indexB];
            indexB += 1;
        }
    }
    return result;
}

static int intPower(int a, int b) {
    int result = 1;
    for (int i = 0; i<b; i++) {
        result *= a;
    }
    return result;
}


double* QuickSort::run(const Vector& vec)
{
    // Инициализация MPI

    int errCode;
    MPI_Request request, request2;
    MPI_Status status;

    if ((errCode = MPI_Init(0, 0)) != 0)
    {
        printf("errCode\n");
    }

    //Intracomm comm = MPI.COMM_WORLD;

    int threadCount, rank;

    //коммуникатор MPI_COMM_WORLD создается по умолчанию и представляет все процессы выполняемой параллельной программы ;
    //Определение количества процессов в выполняемой параллельной программе:
    MPI_Comm_size(MPI_COMM_WORLD, &threadCount);
    //ранг, получаемый при помощи функции MPI_Comm_rank, является рангом процесса, выполнившего вызов этой функции,
    //т. е. переменная ProcRank примет различные значения у разных процессов.
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Считывание списка и определение размера куска

    int sizeInput = vec.m_Size;//16;
    int* packChunkSize = new int[1];
    double* input = vec.m_Vector;//new double[sizeInput] { 0, 1, 2, 5, 7, 3, 4, 7, 3, 1, 5, 7, 3, 46, 67, 678 };

    if (rank == 0) {
        //            try {
        //                input = Files
        //                        .lines(Paths.get("numbers.txt"), StandardCharsets.UTF_8)
        //                        .mapToDouble(Double::valueOf)
        //                        .toArray();
        //            } catch (IOException e) {
        //                e.printStackTrace();
        //            }

        //            input = ListGenerator.generate(64);

        int N = sizeInput;
        packChunkSize[0] = N / threadCount;

    }

    clock_t begin = clock();

    // Рассылаем куски процессам

    //MPI_Bcast() sends the same piece of data to everyone, while MPI_Scatter() sends each process a part of the input array. MPI_Bcast() is the opposite of MPI_Reduce() and MPI_Scatter() is the opposite of MPI_Gather().
    MPI_Bcast(packChunkSize, 1, MPI_INT, 0, MPI_COMM_WORLD); //Передача от одного процесса всем процессам (широковещательная рассылка)


    int chunkSize = packChunkSize[0];

    if (rank != 0) {
        input = new double[chunkSize * threadCount];
    }

    double* local = new double[chunkSize];
    int localSize = chunkSize;

    MPI_Scatter(input, chunkSize, MPI_DOUBLE, local, chunkSize, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Локально сортируем, определяем опорный элемент
    std::sort(local, local + localSize, mycomparefunction);
    //Arrays.sort(local);

    double* packCoreElement = new double[1];

    if (rank == 0) {
        packCoreElement[0] = local[0];
    }

    //comm.Bcast(packCoreElement, 0, 1, MPI.DOUBLE, 0);
    MPI_Bcast(packCoreElement, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD); //Передача от одного процесса всем процессам (широковещательная рассылка)
    double core = packCoreElement[0];

    // Выполняем итерации обмена

    int iterCount = (int)(log(threadCount) / log(2));

    for (int iter = 0; iter < iterCount; iter++) {
        //int coreIndex = Arrays.binarySearch(local, core);
        int coreIndex = std::binary_search(local, local + localSize, core);
        if (coreIndex < 0) {
            coreIndex = coreIndex * (-1) - 1;
        }
        int opponent = getOpponent(rank, iter);


        if (rank < opponent) {
            //comm.Isend(local, coreIndex, local.length - coreIndex, MPI.DOUBLE, opponent, rank);
            //(Object buf, int offset, int count, Datatype datatype, int dest, int tag);


            MPI_Isend(local + coreIndex, localSize - coreIndex, MPI_DOUBLE, opponent, rank, MPI_COMM_WORLD, &request);

            //local = Arrays.copyOfRange(local, 0, coreIndex);
            memcpy(local, local, coreIndex);

            //Status status = comm.Probe(opponent, opponent);
            //Probe(int source, int tag)

            MPI_Status status1;
            MPI_Probe(opponent, opponent, MPI_COMM_WORLD, &status1);
            int count;
            MPI_Get_count(&status1, MPI_DOUBLE, &count);

            double* newarr = new double[count];
            //comm.Recv(newarr, 0, status.count, MPI.DOUBLE, opponent, opponent);
            //Recv(Object buf, int offset, int count, Datatype datatype, int source, int tag);

            MPI_Recv(newarr, count, MPI_DOUBLE, opponent, opponent, MPI_COMM_WORLD, &status1);
            local = mergeArrays(local, newarr, sizeInput, count);
            localSize = sizeInput + count;

        }
        else {

            //comm.Isend(local, 0, coreIndex, MPI.DOUBLE, opponent, rank);
            //comm.Isend(Object buf, int offset, int count, Datatype datatype, int dest, int tag)

            MPI_Isend(local, coreIndex, MPI_DOUBLE, opponent, rank, MPI_COMM_WORLD, &request);

            memcpy(local, local + coreIndex, localSize);
            //local = Arrays.copyOfRange(local, coreIndex, local.length);

            MPI_Status status1;
            MPI_Probe(opponent, opponent, MPI_COMM_WORLD, &status1);
            int count;
            MPI_Get_count(&status1, MPI_DOUBLE, &count);
            double* newarr = new double[count];
            //comm.Recv(newarr, 0, count, MPI.DOUBLE, opponent, opponent);
            MPI_Recv(newarr, count, MPI_DOUBLE, opponent, opponent, MPI_COMM_WORLD, &status1);
            local = mergeArrays(local, newarr, sizeInput, count);
            localSize = sizeInput + count;
        }

    }

    // Сообщаем руту конечные размеры локальных кусков и определяем отступы для вставки их в результирующий массив

    int* counts = new int[threadCount];
    int* localCount = new int [1] { localSize };
    //comm.Gather(localCount, 0, 1, MPI.INT, counts, 0, 1, MPI.INT, 0);
    //Gather(Object sendbuf, int sendoffset, int sendcount, Datatype sendtype, Object recvbuf, int recvoffset, int recvcount, Datatype recvtype, int root)

    MPI_Gather(localCount, 1, MPI_INT, counts, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int* offsets = new int[threadCount];
    if (rank == 0) {
        offsets[0] = 0;
        for (int i = 1; i< threadCount; i++) {
            offsets[i] = offsets[i - 1] + counts[i - 1];
        }
    }

    // Собираем куски

    //comm.Gatherv(local, 0, local.length, MPI.DOUBLE, input, 0, counts, offsets, MPI.DOUBLE, 0);
    //Gatherv(Object sendbuf, int sendoffset, int sendcount, Datatype sendtype, Object recvbuf, int recvoffset, int[] recvcount, int[] displs, Datatype recvtype, int root)


    MPI_Gatherv(local, localSize, MPI_DOUBLE, input, counts, offsets, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    //long stopTime = System.currentTimeMillis();
    clock_t end = clock();

    if (rank == 0) {
        double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

        std::cout << elapsed_secs;
    }

//    MPI_Wait(&request, &status);

    //comm.Barrier();
    MPI_Barrier(MPI_COMM_WORLD);
    //MPI.Finalize();
    MPI_Finalize();
    system("pause");

    return input;
}


