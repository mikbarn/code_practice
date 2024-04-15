#include "bintree.h"
#include <iostream>

using namespace std;

int main() {
    int i = 10;
    BinTree<int> b = BinTree<int>();
    cout << "Testing ..." << endl;
    b.insert(10);
    b.insert(9);
    b.insert(8);
    b.insert(7);
    b.insert(6);
    b.insert(5);
}