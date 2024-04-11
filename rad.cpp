#include <iostream>
#include <cstdlib>
#include <exception>
// #include <string>
#include <cstring>

using namespace std;

const int MAX_SZ = 50;

int max(int c, int *arr) {
    int m = -10000;
    for(int i=0;i<c;i++) {
        m = (arr[i] > m)? arr[i]: m;
    }
    return m;
}

void dump(int c, int *arr) {
    for (int i=0; i<c; i++) {
        cout << arr[i] << ((i < c-1) ? ", ": "");
    }
    cout << endl;
}

void cnt_sort(int c, int *arr, int *dest) {
    int m = max(c, arr);
    cout << "Max value: " << m << endl;
    if (m > MAX_SZ) {
        throw range_error("Bad range");
    }
    int cnt_arr_sz = m+1;
    int *cnt_arr = new int[cnt_arr_sz];

    memset(cnt_arr, 0, c);
    cout << "Initialied with " << cnt_arr_sz << " elements." << endl;
    for (int i = 0; i < c; i++) {
        cnt_arr[arr[i]]++;
    }
    cout << "Counts: " << endl;
    dump(cnt_arr_sz, cnt_arr);
    for (int i = 1; i < cnt_arr_sz; i++) {
        cnt_arr[i] = cnt_arr[i-1] + cnt_arr[i];
    }
    cout << "Prefix sums: " << endl;
    dump(cnt_arr_sz, cnt_arr);

    for (int i = c-1; i >=0 ; i--) {
        int val = arr[i];
        int idx = cnt_arr[val] - 1;
        dest[idx] = val;
        cnt_arr[val]--;
    }

    delete cnt_arr;
}

int main(int argc, char **args) {
    int c = 10;
    int *arr = new int[c];
    for(int i=0;i<c;i++) {
        arr[i] = rand() % MAX_SZ;
    }
    cout << "Generated array: " << endl;
    dump(c, arr);
    int* sorted = new int[c];
    cnt_sort(c, arr, sorted);
    cout << "Sorted: " << endl;
    dump(c, sorted);
    delete sorted;
    delete arr;
    return 0;
}