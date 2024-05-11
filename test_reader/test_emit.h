#pragma once

#include <memory>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <map>


template <class TestType>
class TestCaseReader {
    public:
        virtual std::unique_ptr<TestType> readLines(std::vector<std::string> test_lines) const;
};


template<class TestType>
class TestEmitter {

    public:
        TestEmitter(const char * _fname, const TestCaseReader<TestType> & tcr): fname(_fname), tcr(tcr) {};
       
    private:
        static const int BUFF_SZ = 4096;
        inline static const std::string TEST_START_TOKEN  = ">";
        const char * fname;
        int file_pos = 0;
        int line_number = 1;
        std::ifstream test_ifs;
        bool eof_reached = false;
        const TestCaseReader<TestType> & tcr;
        std::unique_ptr<TestType> curr_test = nullptr;

        void seekToFilePos();
        void readNextTest();        
        
        class TestIterator {
            TestEmitter<TestType> & t; 
            public:   
                TestIterator(TestEmitter<TestType> & _t): t(_t) {};
                TestIterator & operator++() { t.readNextTest(); return *this; };
                TestType & operator*() { return *(t.curr_test.get()); };
                bool operator!=(const TestIterator other) {return !other.t.eof_reached;};      
        };

        TestIterator it = TestIterator(*this);
        
    public:
        TestIterator & begin() { readNextTest(); return it; };
        TestIterator & end() { return it; };
};


static const std::regex kv_regex("(\\w+)\\s*=\\s*([^\n]+)");

struct GenericTest {
        std::map<std::string, std::string> fields;
        void loadFields() {
            for (const auto &[k, v]: fields) {
                std::cout << k << " = " << v << std::endl;
            }
        }
};

template<class X>
class GenericReader: public TestCaseReader<X> {
    std::unique_ptr<X> readLines(std::vector<std::string> lines) const {
        auto tst = std::make_unique<X>();
        std::smatch matches;
        for (std::string &x: lines) {
            if(std::regex_search(x, matches, kv_regex)) {
                if (matches.size() == 3) {
                    tst->fields[matches[1]] = matches[2];
                    std::cout << "Stored field: " << matches[0] << "\n";
                } else {
                    std::cout << "Got " << matches.size() << "?\n";
                    for (int i = 0; i < matches.size(); i++) {
                        std::cout << matches[i] << std::endl;
                    }
                    
                }
            } else {
                std::cout << "Pattern not found ?\n";
            }
        }
        tst->loadFields();
        return tst;
    }
};

#include "test_emit.cpp"