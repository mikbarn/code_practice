#pragma once

#include <memory>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <map>
#include <stdexcept>


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
                TestType & operator*() { return  *(t.curr_test.get()); };
                bool operator!=(const TestIterator other) {return !other.t.eof_reached;};      
        };

        TestIterator it = TestIterator(*this);
        
    public:
        TestIterator & begin() { readNextTest(); return it; };
        TestIterator & end() { return it; };
};


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



static const std::regex kv_regex("(\\w+)\\s*=\\s*([^\n]+)");

struct MappedFieldTest {
    std::map<std::string, std::string> fields;
    
    void loadCustomFields() {}

    std::vector<std::string> splitString(std::string s, const std::string delim=",") {
        std::vector<std::string> rval;
        size_t start = 0;
        size_t end_pos;
        while ((end_pos = s.find(delim, start)) != std::string::npos) {
            std::string nxt;
            nxt = s.substr(start, end_pos);
            start = end_pos + delim.size();
            rval.push_back(nxt);
        }
        if (start < s.size()) {
            rval.push_back(s.substr(start, s.size()));
        } else {
            rval.push_back(std::string(""));
        }
        return rval;           
    }

    template <typename T>
    std::vector<T> parseList(std::string line, T (*converter)(const std::string &)) {
        std::vector<T> rval;    
        for (const auto &tok : splitString(line)) {
            try {
                T t = (*converter)(tok);
                rval.push_back(t);
            }
            catch (const std::invalid_argument& ia) {
                std::cout << "Could not convert " << tok << "!!!\n";
                throw;
            }
        }
        return rval;
    }

    template <typename T>
    std::vector<std::vector<T>> parseNestedList(std::string line, T (*converter)(const std::string &)) {
        std::vector<std::vector<T>> rval;
        size_t start = 0;
        size_t end = 0;
        while (start < line.size()) {
            if (line[start] == '[') {
                end = start + 1;
                while (end < line.size()) {
                    if (line[end] == ']') {
                        size_t dist = (end - 1) - start;
                        if (dist > 0) {
                            std::string sub = line.substr(start+1, dist);
                            rval.push_back(parseList(sub, converter));
                        } else {
                            rval.push_back(std::vector<T>(0));
                        }
                        start = end;
                        break;
                    }
                    ++end;
                }
                if (end >= line.size()) {
                    throw std::runtime_error("Mismatched '[' in nested list");
                }
            }
            ++start;
        }
        return rval;
    }        
};

template<class X>
class MappedFieldReader: public TestCaseReader<X> {
    std::unique_ptr<X> readLines(std::vector<std::string> lines) const {
        auto tst = std::make_unique<X>();
        std::smatch matches;
        for (std::string &x: lines) {
            if(std::regex_search(x, matches, kv_regex)) {
                if (matches.size() == 3) {
                    tst->fields[matches[1]] = matches[2];
                    // std::cout << "Stored field: " << matches[0] << "\n";
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
        tst->loadCustomFields();
        return tst;
    }

};
