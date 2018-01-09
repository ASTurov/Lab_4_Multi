#include <iostream>
#include <string>
#include <fstream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>

#include "CSVManager.h"
#include "Vector.h"
#include "QuickSort.h"

using namespace std;

void myQuickSort(double* arr, int left, int right) {
      int i = left, j = right;
      int tmp;
      int pivot = arr[(left + right) / 2];

      /* partition */

      while (i <= j)
      {
            while (arr[i] < pivot)
                  i++;
            while (arr[j] > pivot)
                  j--;
            if (i <= j)
            {
                  tmp = arr[i];
                  arr[i] = arr[j];
                  arr[j] = tmp;
                  i++;
                  j--;
            }
      };

      /* recursion */
      if (left < j)
            myQuickSort(arr, left, j);
      if (i < right)
            myQuickSort(arr, i, right);
}

int main(int argc, char** argv)
{
    if (argc >= 3)
    {
        string command = string(argv[1]);

        if (command == "create")
        {
            string arg1(argv[2]);
            string arg2(argv[3]);

            Vector tmp(stoi(arg1), 1000, 10);
            string nameOfFile = arg2;
            ofstream file(nameOfFile);
            file << tmp;
            cout << "Vector " << nameOfFile << " created!" << endl;
        }
        if (command == "sort")
        {
            string type(argv[2]);
            ifstream file1(argv[3]);
            string nameForResultMatrix(argv[4]);
            string nameOfOutputFile(argv[5]);

            Vector vec;

            if(file1)
            {
                file1 >> vec;
            }
            else
            {
                cout << "NO THIS FILE WITH VECTOR" << endl;
                system ("pause");
                return -1;
            }

            Vector resultVector(vec.m_Size);

            double start_time = 0;
            double end_time = 0;
            double search_time = 0;

            if (type == "std")
            {
                Vector tmpVec = vec;
                for (int i = 0; i < 10; i++)
                {
                    start_time = clock();

                    myQuickSort(vec.m_Vector, 0, vec.m_Size - 1);

                    end_time = clock();
                    search_time += (end_time - start_time)/1000.0f;
                }
                resultVector = vec;
            }
            if (type == "mpi")
            {
                start_time = clock();

                QuickSort sort;
                resultVector.m_Vector = sort.run(vec);

                end_time = clock();
                search_time += (end_time - start_time)/1000.0f;
            }

            nameOfOutputFile += ".csv";
            fstream file(nameOfOutputFile, ios::in);
            CSVManager CSV;

            file >> CSV;
            file.close();
            file.open(nameOfOutputFile, ios::out);

            CSV.addInformation(resultVector.m_Size, 1, type, search_time);
            file << CSV;
            file.close();

            ofstream fileoutput(nameForResultMatrix);
            fileoutput << resultVector;
        }
        else
        {
            cout << "No this command:" << argv[1] << endl;
        }
    }

    cout << "END" << endl;
    return 0;
}
