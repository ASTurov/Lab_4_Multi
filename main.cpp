#include "MPI\mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include "QuickSort.h"
#include "time.h"

#include <iostream>
#include <string>
#include <fstream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

int main (int argc, char *argv[])
{
    QuickSort sorter;
    vector<int> vec(100000);

    srand(time(NULL));

    cout << "Generate" << endl;

    for (int i = 0; i < vec.size(); i++)
    {
        vec[i] = rand();
    }

    double start_time = 0;
    double end_time = 0;
    double search_time = 0;


    cout << "Sort" << endl;
    cout.precision(5);
    start_time = clock();

    sorter.sort(vec);

    end_time = clock();
    search_time = (end_time - start_time)/1000.0f;
    cout << "Time MPI: " << search_time << endl << endl;

}
