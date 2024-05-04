#include <iostream>
#include <string>
#include <vector>
#include <sstream>


using namespace std;

string next_combo(const char* vals[], int i_count, int indices[]) {
    stringstream ss;
    for (int i = 0; i < i_count; i++) {
        ss << vals[indices[i]];
    }
    return ss.str();
}


int fact(int n) {
    int answer = n;
    while(n>1) {
        answer *= (n-1);
        n--;
    }
    return answer == 0? 1 : answer;
}

void dump_arr(int* arr, int n) {
    cout << "[ ";
    for (int i = 0; i < n; i++) {
        cout << arr[i] << (i < n-1 ? ", ": "");
    }
    cout << " ]" << endl;
}

void dump_arr_mod(int* arr, int n, int mod) {
    for (int i = 0; i < n; i+=mod) {
        cout << "[ ";
        for (int j = 0; j < mod; j++) {
            cout << arr[i + j] << (j < mod-1 ? ", ": "");
        }
        cout << " ]" << endl;
    }
}


/*
123
132
213
231
312
321

0123
0132
0213
0231
0312
0321

1023
1032
1203
1230
1302
1320

2013
2031
2103
2130
2301
2310

3012
3021
3102
3120
3201
3210

01234
01243
01324
01342
01423
01432

02134
02143
02314
02341
02413
02431

03124
03142
03214
03241
03412
03421

04123
04132
04213
04231
04312
04321

10234
10243
10324
10342
10423
10432

12034
12043
12304
12340
12403
12430

*/
void swap3(int *cntr, int n, int *dest, int start) {
    int last = n - 1;
    int lmo = n - 2;
    int lmt = n - 3;

    dest[start + n - 3] = cntr[lmt];
    dest[start + n - 2] = cntr[lmo];
    dest[start + n - 1] = cntr[last];

    dest[start + (2*n) - 3] = cntr[lmt];
    dest[start + (2*n) - 2] = cntr[last];
    dest[start + (2*n) - 1] = cntr[lmo];

    dest[start + (3*n) - 3] = cntr[lmo];
    dest[start + (3*n) - 2] = cntr[lmt];
    dest[start + (3*n) - 1] = cntr[last];

    dest[start + (4*n) - 3] = cntr[lmo];
    dest[start + (4*n) - 2] = cntr[last];
    dest[start + (4*n) - 1] = cntr[lmt];

    dest[start + (5*n) - 3] = cntr[last];
    dest[start + (5*n) - 2] = cntr[lmt];
    dest[start + (5*n) - 1] = cntr[lmo];

    dest[start + (6*n) - 3] = cntr[last];
    dest[start + (6*n) - 2] = cntr[lmo];
    dest[start + (6*n) - 1] = cntr[lmt];

    for (int i = 0; i < n - 3; i++) {
        dest[start + i] = cntr[i];
        dest[start + (1*n) + i] = cntr[i];
        dest[start + (2*n) + i] = cntr[i];
        dest[start + (3*n) + i] = cntr[i];
        dest[start + (4*n) + i] = cntr[i];
        dest[start + (5*n) + i] = cntr[i];
    }
}

int* perm(int n) {
    int *cntr = new int[n];
    int* rval;
    for (int i = 0; i < n; i++) {
        cntr[i] = i;
    }
    if (n == 1) {
        rval = new int[1];
        rval[0] = 0;
    }
    else if (n == 2) {
        rval = new int[4];
        rval[0] = 0; rval[1] = 1; rval[2] = 1; rval[3] = 0;
    }
    else if (n == 3) {
        rval = new int[18];
        swap3(cntr, n, rval, 0);
    }
    else {
        int vary_len = n - 3;
        int* swaps = new int[vary_len];
        for (int i = 0; i < vary_len; i++) {
            swaps[i] = i + 3;
        }

        int swap_idx = 0;

        int iters = fact(n);
        rval = new int[iters * n];
        int q = 0;
        while (q < iters) {
            swap3(cntr, n, rval, q * n);
            int rem = swaps[swap_idx];
            //cout << swap_idx << " " << rem << endl;
            while (swap_idx < vary_len && rem <= 0) {
                swap_idx++;
                if (swap_idx < vary_len) { // TODO simplify
                    rem = swaps[swap_idx];
                }
            }
            swaps[swap_idx]--;
            if (swap_idx >= vary_len - 1 && rem <= 0) {
                break;
            } else {
                int z = swap_idx + 3;
                int* last_n = new int[z];
                for (int i = 0; i < z; i++) {
                    last_n[i] = rval[(q * n) + (n * 6) - (z-i)];
                }
                int temp_idx = n - (4 + swap_idx);
                int tmp = cntr[temp_idx];
                cntr[temp_idx] =  last_n[rem-1];
                last_n[rem-1] = tmp;

                for (int t = 0; t < z; t++) {
                    cntr[n - 1 - t] = last_n[t];
                }
                delete[] last_n;

                for(int i = 0; i < swap_idx; i++) {
                    swaps[i] = i + 3;
                }
                swap_idx = 0;
            }
            q += 6;
        }

        delete[] swaps;
    }

    delete[] cntr;
    return rval;

}


vector<string> choose(int n, const char* vals[], int r) {
    vector<string> res = vector<string>();

    int *cntr = new int[n];
    for (int i = 0; i < r; i++) {
        cntr[i] = i;
    }

    int moves = n - r;
    int adj_slot = r-1;
    bool loop = true;
    int pad = 0;

    int iters = 0;
    while (loop && iters < 50) {
        iters++;
        string nxt = next_combo(vals, r, cntr);
        cout << nxt << endl;
        res.push_back(nxt);
        int curr_idx = cntr[adj_slot];
        if (adj_slot == 0 && curr_idx == n - r) {
            break;
        }
        bool adjusted = false;
        while (curr_idx == n - pad - 1 && adj_slot >= 0) {
            adj_slot -= 1;
            pad++;
            adjusted = true;
            curr_idx = cntr[adj_slot];
        }
        if (adj_slot < 0)  {
            break;
        }

        cntr[adj_slot] = cntr[adj_slot] + 1;
        if (adjusted) {
            for (int i = adj_slot + 1; i < r; i++) {
                cntr[i] = cntr[i - 1] + 1;
            }
        }

        int next_idx = cntr[adj_slot];
        while (next_idx < n - pad - 1 && adj_slot < r - 1) {
            adj_slot += 1;
            pad--;
            next_idx = cntr[adj_slot];
        }


    }
    delete[] cntr;
    return res;
}

void test_combo(int actual, int  n, int r) {
    int expected = fact(n) / (fact(r) * fact(n-r));
    cout << "expected: " << expected << " actual: " << actual << endl;
}

void run_perm(int n, bool print) {
    int f = fact(n);
    cout << "Generating " << f << " permutations " << endl;
    int* permutations = perm(n);
    if (print) {
        dump_arr_mod(permutations, f * n, n);
        cout << endl << endl;
    }
    delete[] permutations;
}

int main(int argc, char **args) {

    int tests[] = {12, 10, 8, 7, 5, 3, 2, 1, 0};

    for (int i = 0; i < 9; i++) {
        cout << tests[i] << " factorial is " << fact(tests[i]) << endl;
    }

    const char* vals[] = {"a", "b", "c", "d", "e"};
    vector<string> results;
    results = choose(5, vals, 3);
    test_combo(results.size(), 5, 3);

    results = choose(5, vals, 5);
    test_combo(results.size(), 5, 5);

    results = choose(5, vals, 4);
    test_combo(results.size(), 5, 4);

    results = choose(5, vals, 1);
    test_combo(results.size(), 5, 1);

    run_perm(1, true);
    run_perm(2, true);
    run_perm(3, true);
    run_perm(4, true);
    run_perm(5, false);
    run_perm(6, false);
    run_perm(5, false);
    run_perm(4, false);
    run_perm(7, false);
    run_perm(9, false);
    run_perm(10, false);


}