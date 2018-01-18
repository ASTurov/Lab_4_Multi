#include "Vector.h"

Vector::Vector()
{

}

Vector::Vector(int size)
{
    m_Size = size;
    m_Vector = new int [m_Size];
    for (int i = 0; i < m_Size; i++)
    {
        m_Vector[i] = 0;
    }
}

Vector::Vector(int size, int maxVal, int minVal) : Vector(size)
{
    fillVector(maxVal, minVal);
}

void Vector::fillVector(int maxVal, int minVal)
{
    if (maxVal == 0)
    {
        cout << "Error, min amount in Vector can't be 0" << endl;
        return;
    }

    for (int i = 0; i < m_Size; i++)
    {
        m_Vector[i] = (rand() % maxVal) + minVal;
    }
}
