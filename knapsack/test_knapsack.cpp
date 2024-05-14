#include "knapsack.cpp"
#include "test_emit.h"

using namespace std;


struct KnapTest: MappedFieldTest {
     string desc;
     vector<float> values;
     vector<float> weights;
     vector<int> picks;
     float n;
     float W;
     float best;

     void loadFields() {
        // GenericTest::loadFields();
        desc = fields["desc"];
        auto tof = [](const std::string & s){ return std::stof(s);};
        auto toi = [](const std::string & s){ return std::stoi(s);};
        values = splitLine<float>(fields["v"], tof);
        // weights = splitLine<float>(fields["w"], &tof);
        // picks = splitLine<int>(fields["w"], &toi);
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
       cout << "HELLO????" << endl;
        // t = *test;
        // int n = t.sz;
        // float W = t.W;

        printf("Running test %s! Items: %d, Max Weight: %5.5f Item list: \n", test.desc, test.n, test.W);
        for (const auto &x: test.values) {
            cout << x << endl;
        }
        // for (int i = 0; i < n; i++) {
        //     printf("item[%03d] v=%05.5f w=%05.5f\n", i, t.v[i], t.w[i]);
        // }
        // printf("-----------------\n");

        // HashMemo hm(n * 5);
        // solveKnapsack(n-1, W, t.v, t.w, hm);
        // hm.showCollisions();

        // auto max_vals = make_unique<float_t[]>(n);
        // printf("Knapsack memo: \n");
        // unique_ptr<MemoKey[]> keys = hm.keys();
        // int hsize = hm.size();
        // HashMemo::sortKeys(keys, 0, hsize-1);
        // for (int i = 0; i < hsize; i++) {
        //     if (hm[keys[i]] <= 0.0f) {
        //         continue;
        //     }
        // max_vals[keys[i].id] = max(max_vals[keys[i].id], hm[keys[i]]);
        // printf("[%03d, %5.5f]: %5.5f\n", keys[i].id, keys[i].f, hm[keys[i]]);
        // }


        // auto item_picks = make_unique<int32_t[]>(n);
        // int item_idx = populateItems(hm, n-1, W, t.w, item_picks.get());

        // cout << "Items Expected: ";
        // for (int i = 0; i < t.exp_count; i++) {
        //      cout << t.expected[i] << " ";
        // }
        // cout << "\n";
        // cout << "Items taken: ";
        // bool pass = (item_idx) == t.exp_count;
        // for (int i = 0; i < item_idx; i++) {
        //     cout << item_picks[i] << " ";
        //     pass &= pass & item_picks[i] == t.expected[i];

        // }
        // cout << "\n";
        // cout << "Test " << (pass? "PASSED": "FAILED") << " !! " << endl;
        break;

    }


    return 0;
}

