
#include "test_emit.h"


struct DummyTest {
    std::string desc;
};

class DummyReader: public TestCaseReader<DummyTest> {
    std::unique_ptr<DummyTest> readLines(std::vector<std::string> lines) const {
        auto dt = std::make_unique<DummyTest>();
        dt->desc = lines[0];
        return dt;
    }
};


int main() {

    DummyReader dr;
    TestEmitter<DummyTest> tests("sample.txt",  dr);
    tests.readNextTest();
    return 0;
}