#include "knapsack.cpp"
#include "test_emit.h"

using namespace std;


struct KnapTest: MappedFieldTest {
     string desc;
     vector<float> values;
     vector<float> weights;
     vector<vector<int>> solutions;
     int n;
     float W;
     float best;

     void loadCustomFields() override {
        desc = fields["desc"];
        auto tof = [](const std::string & s){ return std::stof(s);};
        auto toi = [](const std::string & s){ return std::stoi(s);};
        values = parseList<float>(fields["v"], tof);
        weights = parseList<float>(fields["w"], tof);
        solutions = parseNestedList<int>(fields["solutions"], toi);
        n = values.size();
        W = tof(fields["limit"]);
        best = tof(fields["best"]);
    }
};

int main() {
    quickHashTest();
    MappedFieldReader<KnapTest> gr;
    TestEmitter<KnapTest> test_cases("test_cases.txt",  gr);
    for (const auto &test : test_cases) {
        int n = test.n;
        float W = test.W;

        printf("Running test %s! Items: %d, Max Weight: %5.5f Item list: \n", test.desc.c_str(), test.n, test.W);

        for (int i = 0; i < n; i++) {
            printf("item[%03d] v=%05.5f w=%05.5f\n", i, test.values[i], test.weights[i]);
        }
        printf("-----------------\n");

        HashMemo hm(n * 5);
        solveKnapsack(n-1, W, (float_t *)&test.values[0], (float_t *)&test.weights[0], hm);
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
        int item_idx = populateItems(hm, n-1, W, (float_t *)&test.weights[0], item_picks.get());

        vector<int> soltuion1 = test.solutions[0];

        cout << "Items Expected: ";
        for (int i = 0; i < soltuion1.size(); i++) {
             cout << soltuion1[i] << " ";
        }
        cout << "\n";
        cout << "Items taken: ";
        bool pass = (item_idx) == soltuion1.size();
        for (int i = 0; i < item_idx; i++) {
            cout << item_picks[i] << " ";
            pass &= pass & item_picks[i] == soltuion1[i];

        }
        cout << "\n";
        cout << "Test " << (pass? "PASSED": "FAILED") << " !! " << endl;

    }


    return 0;
}

