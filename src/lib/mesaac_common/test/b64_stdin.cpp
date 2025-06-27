#include <iostream>
#include "mesaac_common/b64.h"

using namespace std;
using namespace mesaac;

int main (int /*argc*/, char const ** /*argv*/)
{
    B64 codec;
    
    string unencoded;
    while (cin) {
        const int BUFFSIZE = 1024;
        char buff[BUFFSIZE];
        cin.read(buff, BUFFSIZE);
        unencoded.append(string(buff, cin.gcount()));
    }
    cout << codec.encode(unencoded) << endl;
    cerr << "Encoded string of length " << unencoded.size() << endl;
    return 0;
}
