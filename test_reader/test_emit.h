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
#include "mab_logger.h"

template <class TestType>
class TestCaseReader {
    public:
        virtual std::unique_ptr<TestType> readLines(std::vector<std::string> test_lines) const = 0;
};


template<class TestType>
class TestEmitter {
    public:
        TestEmitter(const char * file_name, const TestCaseReader<TestType> & tcr): fname(file_name), tcr(tcr) {};

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
        mab::Logger log;

        void seekToFilePos();
        void readNextTest();

        class TestIterator {
            TestEmitter<TestType> * t = nullptr;
            public:
                TestIterator(TestEmitter<TestType> * _t): t(_t) {};
                TestIterator & operator++() { t->readNextTest(); return *this; };
                TestType & operator*() {  auto b = t->curr_test.get(); return *b; };
                bool operator!=(const TestIterator other) {return !(other.isDone() && this->isDone());}; // bad - not worth ranged loop
                bool isDone() const {return (t == nullptr || t->curr_test == nullptr); };
        };

    public:
        TestIterator begin() { readNextTest(); return TestIterator(this); };
        TestIterator end() { return TestIterator(nullptr); };
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
    curr_test = nullptr;

    if (eof_reached) {
        return;
    }

    seekToFilePos();

    while ((curr_test == nullptr) && (!eof_reached)) {
        test_lines.clear();
        loop = true;
        size_t starting_line = line_number;

        while (loop && !eof_reached) {
            test_ifs.getline(buff.get(), BUFF_SZ);
            eof_reached = test_ifs.eof();
            file_pos = test_ifs.tellg();
            if (test_ifs.bad() || (test_ifs.fail() && !eof_reached)) {
                log.crit("Failed reading line %d!", line_number);
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

            log.debug("Read Line [Len: %d, LStrip: %d]: '%s'", new_len, len-new_len, s.c_str());

            if (s != TEST_START_TOKEN && new_len > 0) {
                test_lines.push_back(s);
            }

            loop = (s != TEST_START_TOKEN || line_number == 1);
            if (!eof_reached)
                ++line_number;
        }

        if (test_lines.size() > 0) {
            try {
                curr_test = tcr.readLines(test_lines);
            } catch (std::invalid_argument & ia) {
                log.error("Error reading test: %s Lines %d - %d", ia.what(), starting_line, line_number - 1);
            }
            if (curr_test == nullptr) {
                log.error("Couldn't parse test! Lines %d - %d", starting_line, line_number - 1);
            }
        } else {
            log.error("No Valid lines! Lines %d - %d",  starting_line, line_number - 1);
        }
    }
};



static const std::regex kv_regex("(\\w+)\\s*=\\s*([^\n]+)");

class MappedFieldTest {
    private:
        mab::Logger log;
    public:
        std::map<std::string, std::string> fields;

        virtual void loadCustomFields() {};
        virtual ~MappedFieldTest() {};
        virtual std::vector<std::string> getValidFields() { return std::vector<std::string>(0); };

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
                    log.error("Could not convert token: '%s'", tok.c_str());
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
                        throw std::invalid_argument("Mismatched '[' in nested list");
                    }
                }
                ++start;
            }
            return rval;
    }
};

template<class TestType>
class MappedFieldReader: public TestCaseReader<TestType> {
    public:
        mab::Logger log;
        std::unique_ptr<TestType> readLines(std::vector<std::string> lines) const override {
            auto tst = std::make_unique<TestType>();
            std::smatch matches;
            for (std::string &x: lines) {
                if(std::regex_search(x, matches, kv_regex)) {
                    if (matches.size() == 3) {
                        tst->fields[matches[1]] = matches[2];
                    } else {
                        std::cout << "Got " << matches.size() << "?\n";
                        for (size_t i = 0; i < matches.size(); i++) {
                            std::cout << matches[i] << std::endl;
                        }
                    }
                } else {
                    log.debug("Pattern key=val not found for line: '%s'", x.c_str());
                }
            }
            for (const auto &_x : tst->getValidFields()) {
                if (tst->fields.count(_x) != 1) {
                    log.error("Required field %s is missing!", _x.c_str());
                    return nullptr;
                }
            }
            try {
                tst->loadCustomFields();
            } catch (std::invalid_argument & ia) {
                return nullptr;
            }
            return tst;
        }
};
