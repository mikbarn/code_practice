#pragma once

#include <memory>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>


template <typename TestType>
class TestCaseReader {
    public:
        virtual std::unique_ptr<TestType> readLines(std::vector<std::string> test_lines) const;
};

template<typename TestType>
class TestEmitter {

    public:
        TestEmitter(const char * fname, const TestCaseReader<TestType> & tcr);
        ~TestEmitter();
        std::unique_ptr<TestType> curr_test = nullptr;
        void readNextTest();

    private:
        static const int BUFF_SZ = 4096;
        const char * fname;
        int file_pos = 0;
        int line_number = 1;
        std::ifstream test_ifs;
        void seekToFilePos();
        const std::string NL = std::string("\n");
        const std::string SPC = std::string(" ");

        const TestCaseReader<TestType> & tcr;

};

#include "test_emit.cpp"