#include <iostream>

using namespace std;

struct A {
    int id;
    int * a;
    static int next_id;
    A() {
        a = new int[8];
        id = next_id++;
        cout << "New A["<<id<<"]\n";
    }

    A(const A& o) {
       cout << "Copy Ctor\n";
       id = next_id++;
       cout << "New A["<<id<<"] from A["<<o.id<<"]\n";
       a = new int[8];
    }

    A & operator=(const A &o) {
        cout << "Assignment Op\n";
        if (this != &o) {
            delete[] a;
            a = new int[8];
        }
        cout << "Copy Assign A["<<id<<"] from A["<<o.id<<"]\n";
        return *this;
    }

    ~A() {
        cout << "Destruct A["<<id<<"]\n";
        delete a;
    }
};

int A::next_id = 1;

void run() {
    A a1;
    A a2;

    cout << "A a3 = a2\n";
    A a3 = a2;

    cout << "A *ap\n";
    A *ap;

    cout << "ap = &a1\n";
    ap = &a1;
    cout << "a4 = *ap\n";
    A a4 = *ap;

    cout << "A arr[3]\n";
    A arr[3];
    cout << "arr[0] = a1\n";
    arr[0] = a1;
    cout << "arr[1] = a2\n";
    arr[1] = a2;
    cout << "arr[2] = arr[0]\n";
    arr[2] = arr[0];

    cout << "a1 = a2\n";
    a1 = a2;
}

int main() {

    run();
    cout << "Bye\n";
    return 0;
}