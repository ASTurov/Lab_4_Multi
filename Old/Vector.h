#ifndef VECTOR_H
#define VECTOR_H

#include <iostream>
#include <fstream>

using namespace std;

struct Vector
{
public:
    Vector();
    Vector(int size);
    Vector(int size, int minVal, int maxVal);
    void fillVector(int maxVal, int minVax = 0);
    int m_Size; //Cтроки
    int* m_Vector;


    friend ostream& operator<<(ostream& fout, const Vector& vector)
    {
        {
            fout << vector.m_Size << endl;

            for (int i = 0; i < vector.m_Size; i++)
            {
                   fout << vector.m_Vector[i] << " ";
            }
            return fout;
        }
    }

    friend ifstream& operator>>(ifstream& fin, Vector& vector)
    {
        unsigned int size;
        fin >> size;

        vector = Vector(size);
        for (int i = 0; i < size; i++)
        {
               fin >> vector.m_Vector[i];
        }
        return fin;
    }
};

#endif // VECTOR_H
