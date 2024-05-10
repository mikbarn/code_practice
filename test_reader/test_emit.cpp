#ifndef _TEST_EMIT_CPP
#define _TEST_EMIT_CPP

#include "test_emit.h"


template<typename TestType> TestEmitter<TestType>::TestEmitter(const char * _fname, const TestCaseReader<TestType> & tcr): fname(_fname), tcr(tcr) {

}

template<typename TestType> TestEmitter<TestType>::~TestEmitter() {

}


template<typename TestType> void TestEmitter<TestType>::seekToFilePos() {
    if (!test_ifs.is_open()) {
        test_ifs.open(fname);
    }
    test_ifs.seekg(file_pos);

};

template<typename TestType> void TestEmitter<TestType>::readNextTest() {
    auto test_lines = std::vector<std::string>(5);
    auto buff = std::make_unique<char[]>(BUFF_SZ);
    bool loop = true;

    seekToFilePos();

    while (loop) {
        test_ifs.getline(buff.get(), BUFF_SZ);
        bool is_eof = test_ifs.eof();
        file_pos = test_ifs.tellg();
        if (test_ifs.bad() || (test_ifs.fail() && !is_eof)) {
            printf("Failed reading line %d! \n", line_number);
            exit(-1);
        }

        if (!is_eof)
            ++line_number;

        auto s = std::string(buff.get(), test_ifs.gcount());
        size_t len = s.size()-1;

        std::cout << "Read line: '" << s << "'" << std::endl;
        std::cout << s[len] << " " << (s.at(len) == '\n') << std::endl;
        while (len >= 0 && s[len] == '\n') {
            len = s.size() - 1;
            s.pop_back();
        }
        if (s != ">") {
            test_lines.push_back(s);
        } else {
            std::cout << "Skip token: '" << s << "'" << std::endl;
        }

        loop = (line_number != 1) && (s != ">") && (!is_eof);
    }
    curr_test = tcr.readLines(test_lines);
};

#endif