#include <iostream>
#include <unistd.h>
#include <cstring>

using namespace std;


int works(float test)
{
    unsigned char result[sizeof(float)];

    memcpy(result, &test, sizeof(test));

   cout << "Print actual value at " << &result << endl;
   cout << sizeof(float) << endl;
    for (int n = 0; n < sizeof(float); ++n) {
        printf("%x ", result[n]);
    }

    return 0;
}

int main() {
    float tst = 32.5;
    char * addr = (char*)&tst;

    for (int i = 0; i < sizeof(float); i++) {

        cout << "Addr: " << (void*)(addr + i) << endl;
        printf("%x\n", *(addr + i));
    }

    cout << endl;

    works(tst);

}
