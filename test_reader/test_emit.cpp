#ifndef _TEST_EMIT_CPP
#define _TEST_EMIT_CPP

#include "test_emit.h"


template<class TestType> void TestEmitter<TestType>::seekToFilePos() {
    if (!test_ifs.is_open()) {
        test_ifs.open(fname);
    }
    test_ifs.seekg(file_pos);

};

template<class TestType> void TestEmitter<TestType>::readNextTest() {
    auto test_lines = std::vector<std::string>(0);
    auto buff = std::make_unique<char[]>(BUFF_SZ);
    bool loop = true;

    if (eof_reached) {
        return;
    }

    seekToFilePos();

    curr_test = nullptr;
    while (curr_test == nullptr) {
        while (loop && !eof_reached) {
            test_ifs.getline(buff.get(), BUFF_SZ);
            eof_reached = test_ifs.eof();
            file_pos = test_ifs.tellg();
            if (test_ifs.bad() || (test_ifs.fail() && !eof_reached)) {
                printf("Failed reading line %d! \n", line_number);
                exit(-1);
            }

            auto s = std::string(buff.get(), test_ifs.gcount());
            size_t len = s.size();
            size_t new_len = len;
            auto bad = [](char & c) {return (int)c < 33;};

            while (new_len > 0 && bad(s[new_len - 1])) {
                s.pop_back();
                new_len = s.size();
            }

            printf("Read Line [Len: %d, LStrip: %d]: '%s'\n", new_len, len-new_len, s.c_str());

            if (s != TEST_START_TOKEN && new_len > 0) {
                test_lines.push_back(s);
            }

            loop = (s != TEST_START_TOKEN || line_number == 1);
            if (!eof_reached) 
                ++line_number;
        }

        if (test_lines.size() > 0) {
            curr_test = tcr.readLines(test_lines);
            if (curr_test == nullptr) {
                std::cout << "Couldn't parse test! " << std::endl;
            }
        } else {
            std::cout << "No Valid lines!" << std::endl;
            break;
        }
    }
};

#endif