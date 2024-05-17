
#include "test_emit.h"

using namespace std;

class DummyTest {
    public:
        string desc;
};

class DummyReader: public TestCaseReader<DummyTest> {
    unique_ptr<DummyTest> readLines(vector<string> lines) const {
        for (auto &x: lines) {
            cout << " -- " <<  x << endl;
        }
        auto dt = make_unique<DummyTest>();
        dt->desc = lines[0];
        return dt;
    }
};


class FieldTest: public MappedFieldTest {
    public:
        string desc;
        vector<vector<float>> nested;
        vector<float> f_list;
        vector<string> getValidFields() {
            return vector<string>({"desc", "nested"});
        }
        void loadCustomFields() {
            auto tof = [](const string & s){ return stof(s);};
            desc = fields["desc"];
            nested = parseNestedList<float>(fields["nested"], tof);
            f_list = parseList<float>(fields["vals"], tof);
        }
};


int main() {

    // DummyReader dr;
    // TestEmitter<DummyTest> tests("sample.txt",  dr);
    // for (DummyTest &x: tests) {
    //     cout << "Running test: " << x.desc << "\n";
    // }

    MappedFieldReader<FieldTest> fr;
    TestEmitter<FieldTest> field_tests("sample.txt",  fr);
    for (FieldTest &x: field_tests) {
        cout << "Running field test: \n";
        for (const auto &[k, v]: x.fields) {
            cout << "k:" << k << " v: " << v << "\n";
        }
        for (const auto &outer: x.nested) {
            cout << "[ ";
            for (const auto &inner: outer) {
                cout << inner << " ";
            }
            cout << "]\n";
        }

    }
    return 0;
}