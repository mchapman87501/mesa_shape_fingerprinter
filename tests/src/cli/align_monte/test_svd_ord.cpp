#include <iostream>
#include <cmath>
#include "svd.h"

using namespace std;

bool eq(float a, float b) {
    return (::fabs(a - b) <= 1.0e-6);
}

void printData(ap::real_2d_array& x)
{
    // Bounds are referenced starting at 1?!
    const unsigned int rowStart = x.getlowbound(1);
    const unsigned int rowEnd = x.gethighbound(1) + 1;
    const unsigned int colStart = x.getlowbound(2);
    const unsigned int colEnd = x.gethighbound(2) + 1;
    cout << "data " << endl;
    for (unsigned int row = x.getlowbound(0); row != rowEnd; row++) {
        string sep("");
        for (unsigned int col = colStart; col != colEnd; col++) {
            cout << sep << x(row, col);
            sep = "\t";
        }
        cout << endl;
    }
}

void printTransformedData(ap::real_2d_array& x, ap::real_2d_array& vt)
{
    const unsigned int rowStart = x.getlowbound(1);
    const unsigned int rowEnd = x.gethighbound(1) + 1;
    const unsigned int colStart = x.getlowbound(2);
    const unsigned int colEnd = x.gethighbound(2) + 1;
    cout << "Transformed Data: " << endl;
    for (unsigned int row = rowStart; row != rowEnd; row++) {
        string sep("");
        for (unsigned int col = colStart; col != colEnd; col++) {
            float value = 0.0;
            for (unsigned int d = colStart; d != colEnd; d++) {
                value += vt(col, d) * x(row, d);
            }
            cout << sep << value;
            sep = "\t";
        }
        cout << endl;
    }
}

void printTransposeTransformedData(ap::real_2d_array& x, ap::real_2d_array& vt)
{
    const unsigned int rowStart = x.getlowbound(1);
    const unsigned int rowEnd = x.gethighbound(1) + 1;
    const unsigned int colStart = x.getlowbound(2);
    const unsigned int colEnd = x.gethighbound(2) + 1;
    cout << "Transpose Transformed Data: " << endl;
    for (unsigned int row = rowStart; row != rowEnd; row++) {
        string sep("");
        for (unsigned int col = colStart; col != colEnd; col++) {
            float value = 0.0;
            for (unsigned int d = colStart; d != colEnd; d++) {
                value += vt(d, col) * x(row, d);
            }
            cout << sep << value;
            sep = "\t";
        }
        cout << endl;
    }
}

int main (int argc, char const *argv[])
{
    ap::real_2d_array x;
    ap::real_1d_array w;
    ap::real_2d_array u;
    ap::real_2d_array vt;
    
    cout.precision(3);
    cout.setf(ios::showpos);
    cout.setf(ios::fixed, ios::floatfield);
    cout.setf(ios::right, ios::adjustfield);

    const unsigned int numPoints(7);
    const unsigned int numVars(3); // x, y, z
    x.setbounds(0, numPoints - 1, 0, numVars - 1);
    
    const float degrees = 30.0;
    const float rads = degrees * M_PI / 180.0;
    const float s = sin(rads);
    const float c = cos(rads);
    for (unsigned int row = 0; row != numPoints; row++) {
        float xval = (float)row - (numPoints / 2);  // Mean-centered?
        x(row, 0) = xval;
        x(row, 1) = xval * tan(rads);
        x(row, 2) = 0.0;             // z == 0
    }
    printData(x);

    if (!rmatrixsvd(x, numPoints, 3, 2, 2, 2, w, u, vt)) {
        cerr << "PCA failed.  Abort" << endl;
    }
    
    cout << "VT:" << endl;
    for (unsigned int row = 0; row != numVars; row++) {
        for (unsigned int col = 0; col != numVars; col++) {
            float value(vt(row, col));
            cout << " ";
            if (eq(s, value)) {
                cout << "   sin";
            } else if (eq(-s, value)) {
                cout << "  -sin";
            } else if (eq(c, value)) {
                cout << "   cos";
            } else if (eq(-c, value)) {
                cout << "  -cos";
            } else {
                cout << value;
            }
        }
        cout << endl;
    }
    
    printTransformedData(x, vt);
    printTransposeTransformedData(x, vt);
        
    return 0;
}