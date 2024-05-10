#include "knapsack.cpp"



struct Test {
    int sz = 0;
    float * w = nullptr;
    float * v = nullptr;
    int * expected = nullptr;
    int exp_count = 0;
    float expected_max = 0;
    float W;

    Test& operator=(const Test &other) {
        if (this != &other) {
            _free();
            W = other.W;
            sz = other.sz;
            w = new float[other.sz];
            v = new float[other.sz];
            exp_count = other.exp_count;
            expected = new int[exp_count];
            for (int i = 0; i < other.sz; i++) {
                w[i] = other.w[i];
                v[i] = other.v[i];
            }
            for (int i = 0; i < exp_count; i++) {
                expected[i] = other.expected[i];
            }
        }
        return *this;
    }

    ~Test() {
        _free();
    }
    void _free() {
        if (w) {
            delete w;
            delete v;
            delete expected;
        }
        w = nullptr;
        v = nullptr;
        expected = nullptr;
    }
};



const int NUM_TESTS = 5;


int main() {
    quickHashTest();

    Test tests[NUM_TESTS] =  {
            Test{
                10,
                new float[]{23.0, 26.0, 20.0, 18.0, 32.0, 27.0, 29.0, 26.0, 30.0, 27.0},
                new float[]{505.0, 352.0, 458.0, 220.0, 354.0, 414.0, 498.0, 545.0, 473.0, 543.0},
                new int[]{7,3,0},
                3,
                1270.0f,
                67.0f
            },
            Test{
                3,
                new float[]{50, 20, 30},
                new float[]{5.0f, 10.0f, 15.0f},
                new int[]{2,1},
                2,
                25.0f,
                50.0f
            },
            Test{
                3,
                new float[]{50, 51, 54},
                new float[]{5.0f, 10.0f, 15.0f},
                new int[]{0},
                1,
                5.0f,
                50.0f
            },
            Test{
                3,
                new float[]{10, 50, 42},
                new float[]{10.0f, 5.0f, 15.0f},
                new int[]{2},
                1,
                15.0f,
                50.0f
            },
            Test{
                3,
                new float[]{10, 30, 49},
                new float[]{10.0f, 5.0f, 15.0f},
                new int[]{2},
                1,
                15.0f,
                50.0f
            }
    };
    Test *test;
    Test t;
    for (int test_idx = 0; test_idx < NUM_TESTS; test_idx++) {
        test = &tests[test_idx];
        t = *test;
        int n = t.sz;
        float W = t.W;

        printf("Running test %d! Items: %d, Max Weight: %5.5f Item list: \n", test_idx, n, W);
        for (int i = 0; i < n; i++) {
            printf("item[%03d] v=%05.5f w=%05.5f\n", i, t.v[i], t.w[i]);
        }
        printf("-----------------\n");

        HashMemo hm(n * 5);
        solveKnapsack(n-1, W, t.v, t.w, hm);
        hm.showCollisions();

        auto max_vals = make_unique<float_t[]>(n);
        printf("Knapsack memo: \n");
        unique_ptr<MemoKey[]> keys = hm.keys();
        int hsize = hm.size();
        HashMemo::sortKeys(keys, 0, hsize-1);
        for (int i = 0; i < hsize; i++) {
            if (hm[keys[i]] <= 0.0f) {
                continue;
            }
        max_vals[keys[i].id] = max(max_vals[keys[i].id], hm[keys[i]]);
        printf("[%03d, %5.5f]: %5.5f\n", keys[i].id, keys[i].f, hm[keys[i]]);
        }


        auto item_picks = make_unique<int32_t[]>(n);
        int item_idx = populateItems(hm, n-1, W, t.w, item_picks.get());

        cout << "Items Expected: ";
        for (int i = 0; i < t.exp_count; i++) {
             cout << t.expected[i] << " ";
        }
        cout << "\n";
        cout << "Items taken: ";
        bool pass = (item_idx) == t.exp_count;
        for (int i = 0; i < item_idx; i++) {
            cout << item_picks[i] << " ";
            pass &= pass & item_picks[i] == t.expected[i];

        }
        cout << "\n";
        cout << "Test " << (pass? "PASSED": "FAILED") << " !! " << endl;

    }


    return 0;
}

