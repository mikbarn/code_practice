
#include "test_emit.h"


struct DummyTest {
    std::string desc;
};

class DummyReader: public TestCaseReader<DummyTest> {
    std::unique_ptr<DummyTest> readLines(std::vector<std::string> lines) const {
        for (auto &x: lines) {
            std::cout << " -- " <<  x << std::endl;
        }
        auto dt = std::make_unique<DummyTest>();
        dt->desc = lines[0];
        return dt;
    }
};


struct FieldTest: GenericTest {
     void loadFields() {
        GenericTest::loadFields();
        std::cout << "DERIVED LOAD" << std::endl;
    }
};


int main() {

    DummyReader dr;
    TestEmitter<DummyTest> tests("sample.txt",  dr);
    for (DummyTest &x: tests) {
        std::cout << "Running test: " << x.desc << "\n";
    }

    GenericReader<FieldTest> fr;
    TestEmitter<FieldTest> field_tests("sample.txt",  fr);
    for (FieldTest &x: field_tests) {
        std::cout << "Running field test: \n";
        for (const auto &[k, v]: x.fields) {
            std::cout << "k:" << k << " v: " << v << "\n";
        }
        
    }  
    return 0;
}