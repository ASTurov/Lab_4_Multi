#include <QCoreApplication>
#include "Vector.h"
#include "QuickSort.h"
using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Vector vec;
    vec = Vector(1000, 10000, 1);
    cout << "VECTOR : ";
    cout << vec << endl;

    Vector out(vec.m_Size);
    QuickSort sort;
    out.m_Vector = sort.run(vec);
    cout << "FINAL: " << out;

    return a.exec();
}
