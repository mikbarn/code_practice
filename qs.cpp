#include <iostream>
// #include <

using namespace std;

const int tst[] = {3, 3, 5, 9, 6, 8, 3, 2, 0, 7, 4, 1};

void dump(int c, int *arr) {
    for (int i=0; i<c; i++) {
        cout << arr[i] << ((i < c-1) ? ", ": "");
    }
    cout << endl;
}

void swap(int *arr, int a, int b) {
    int tmp = arr[a];
    arr[a] = arr[b];
    arr[b] = tmp;
}

void sort_part(int *arr, int p_idx, int start_idx, int end_idx, int depth) {
    // dump(12, arr);
    // cout << "SP: " << start_idx << "("<<arr[start_idx]<<") " << end_idx-1 << "("<<arr[end_idx-1]<<") pivot:" << p_idx << "("<<arr[p_idx]<<")" << endl;

    int p_val = arr[p_idx];
    int bk = start_idx;
    for (int i=start_idx; i<end_idx; i++) {
        if(arr[i] <= p_val) {
            if (p_idx != i) {
                swap(arr, i, bk);
                bk++;
            }
            if (bk == p_idx) {
                bk++;
            }
        }
    }
    int bound = (bk > p_idx)? bk-1: bk;
    // cout << "Before " << endl;
    // dump(12, arr);
    swap(arr, bound, p_idx);

    // cout << "After" << endl;
    // dump(12, arr);
    if (start_idx < bound-1) {
        sort_part(arr, bound-1, start_idx, bound, depth+1);
    }
    if (bound < end_idx-1) {
        sort_part(arr, end_idx-1, bound+1, end_idx, depth+1);
    }
}


int* fill(int c, const int *input) {
    int *arr = new int[c];
    for(int i=0; i<c; i++) {
        arr[i] = input[i];
    }
    return arr;
}

int main(int argc, char **args) {
    int *arr = fill(12, tst);
    dump(12, arr);
    sort_part(arr, 5, 0, 12, 0);
    dump(12, arr);
    int arr2[] = {100, -1, 3, -1, 4, 5, 6, 99, 8, 7, 2, 2, 0, 0, 1, 0, 1000, 99, 50, 49, 50, 26, 39};
    dump(23, arr2);
    sort_part(arr2, 5, 0, 23, 0);
    dump(23, arr2);
}