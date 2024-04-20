#include <map>
#include <tuple>
#include <string>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

typedef unsigned char BuffType;
#define BUFF_SIZE 8

class NotImplementedException : public std::logic_error
{
public:
    NotImplementedException() : std::logic_error("Function not yet implemented") { };
};

shared_ptr<BuffType[]> slice(const BuffType *buff, int start, int end, int arr_sz) {
    if (start < 0) {
        start = arr_sz + start;
    }
    if (end < 0) {
        end = arr_sz + end;
    }
    int inc = 1;
    int sz = end - start;
    if (end < start) {
        inc = -1;
        sz = start - end;
    }

    auto rv = shared_ptr<BuffType[]>(new BuffType[sz]);
    int j = 0;
    for (int i = start; i != end; i+=inc) {
        rv[j++] = buff[i];
    }
    return rv;
}

int from_bytes(const BuffType *buff, int len, bool little) {
    int start = 0, end = len, inc = 1;
    unsigned int res = 0;
    if (!little) {
        start = len - 1;
        end = -1;
        inc = -1;
    }
    int j = 0;
    for (int i = start; i != end; i+=inc) {
        res |= (buff[j++] << (BUFF_SIZE * i));

    }
    return res;
}

void dump_arr(const BuffType *buff, int sz) {
    cout << "[";
    for (int i = 0; i < sz; i++) {
        cout << buff[i] << (i < sz - 1? ",": "");
    }
    cout << "]" << endl;
}

enum TCP_TYPE { // thrift compact protocol
    TCP_INT16,
    TCP_INT32,
    TCP_INT64,
    TCP_TRUE,
    TCP_FALSE,
    TCP_LIST,
    TCP_STRUCT,
    TCP_MAP,
    TCP_STRING
};

const string TCP_NAMES[] = {
    "TCP_INT16",
    "TCP_INT32",
    "TCP_INT64",
    "TCP_TRUE",
    "TCP_FALSE",
    "TCP_LIST",
    "TCP_STRUCT",
    "TCP_MAP",
    "TCP_STRING "
};


class ParquetFooterParser {
    static const int ZIGZAG = 1;

    struct ParseResult {
        static constexpr char SPACE[] = "   ";
        const TCP_TYPE tcp_type;
        union {
            void* data = nullptr;
            int int_value;
        };
        ParseResult(TCP_TYPE t): tcp_type(t)  {
        }

        ~ParseResult() {
            if (data != nullptr) {
                switch (tcp_type)
                {
                    case TCP_LIST:
                        delete (vector<ParseResult*>*) data;
                        break;
                    case TCP_STRING:
                        delete[] (BuffType*) data;
                        break;

                    default:
                        break;
                }
            }
        }
        string str(int indent) {
                stringstream spacer;
                for (int i = 0; i < indent; i++) {
                    spacer << SPACE;
                }
                string margin = spacer.str();
                stringstream ss;

                string name = TCP_NAMES[tcp_type];
                switch (tcp_type)
                {
                    case TCP_INT16:
                    case TCP_INT32:
                    case TCP_INT64:
                    case TCP_TRUE:
                    case TCP_FALSE:
                        ss << name << ": " << int_value;
                        return  ss.str();
                    case TCP_STRING:
                        ss << *((BuffType*)data);
                        return ss.str();
                    case TCP_LIST:
                    {
                        ss << "[";
                        auto v = *((vector<ParseResult*>*)data);
                        for (int j = 0; j < v.size(); j++) {
                            ss << v[j]->str(indent) << (j < v.size() - 1 ? ",": "");
                        }
                        ss << "]";
                        return ss.str();
                    }
                    case TCP_STRUCT:
                    {
                        ss << "{" << endl;
                        for (const auto& [key, value]: children) {
                            ss << key << ": " << value.str(indent) << endl;
                        }
                        ss << "}" <<  endl;
                        return ss.str();
                    }
                    default:
                        return "Unsupported: " + name;
                }
        }
        map<int, ParseResult&> children;
    };

    public:
        ParquetFooterParser();
        ~ParquetFooterParser();
        void loadBuffer(BuffType* bytes, int len);
        void processBuffer();
        void dumpParseResult();
        int readFooterLength(BuffType* bytes, int len);
        ParseResult parseFooter(const char* bytes, int len);

    private:
        BuffType* buffer = nullptr;
        int buffer_len = 0;
        ParseResult final_pr = ParseResult(TCP_STRUCT);

        int decodeI32(int options, int pos, ParseResult &pr);
        int decodeList(int options, int pos, ParseResult &pr);
        int decodeStruct(int options, int pos, ParseResult &pr);
        int decodeTrue(int options, int pos, ParseResult &pr);
        int decodeFalse(int options, int pos, ParseResult &pr);
        int decodeBin(int options, int pos, ParseResult &pr);
        int decodeMap(int options, int pos, ParseResult &pr);

        // shared_ptr<BuffType[]> slice(BuffType *buff, int start, int end);

        inline int unzag(int i) { return (i >> 1) ^ -(i & 1); };
        void clean();
        typedef int (ParquetFooterParser::*thrift_compact_processor_t)(int, int, ParseResult &);
        map<int, tuple<TCP_TYPE, thrift_compact_processor_t>> field_map = {
            {1, {TCP_TRUE, &ParquetFooterParser::decodeTrue}},
            {2, {TCP_FALSE, &ParquetFooterParser::decodeFalse}},
            {4, {TCP_INT16, &ParquetFooterParser::decodeI32}},
            {5, {TCP_INT32, &ParquetFooterParser::decodeI32}},
            {6, {TCP_INT64, &ParquetFooterParser::decodeI32}},
            {8, {TCP_STRING, &ParquetFooterParser::decodeBin}},
            {9, {TCP_LIST, &ParquetFooterParser::decodeList}},
            {11, {TCP_MAP, &ParquetFooterParser::decodeMap}},
            {12, {TCP_STRUCT, &ParquetFooterParser::decodeStruct}}
        };
};

int ParquetFooterParser::readFooterLength(BuffType *bytes, int len) {
    if (len < 8) {
        return -1;
    }
    auto junk = slice(bytes, -4, len, len);

    if (strcmp((char*)junk.get(), "PAR1") != 0) {
        cout << "Unexpected value of last 4: " << junk.get() << endl;
    }
    auto bits = slice(bytes, -8, -4, len);
    int footer_len = from_bytes(bits.get(), 4, true);
    return footer_len;
}

void ParquetFooterParser::dumpParseResult() {
    cout << "Printing final parse result ... " << endl;
    for (const auto& [key, value]: final_pr.children) {
        cout << "key: " << key;
    }
}

void ParquetFooterParser::processBuffer() {
    decodeStruct(0, 0, final_pr);
}

ParquetFooterParser::ParquetFooterParser() { }

ParquetFooterParser::~ParquetFooterParser() {
    clean();
}

void ParquetFooterParser::clean() {
    if (buffer != nullptr) {
        delete[] buffer;
    }
}

void ParquetFooterParser::loadBuffer(BuffType* bytes, int len) {
    clean();
    buffer = new BuffType[len];
    for (int i = 0; i < len; i++) {
        buffer[i] = (BuffType)bytes[i];
    }
    buffer_len = len;
}


int ParquetFooterParser::decodeMap(int options, int pos, ParseResult &pr) {
    throw NotImplementedException();
}

int ParquetFooterParser::decodeStruct(int options, int pos, ParseResult &pr) {
    int _8;
    int last_id = 0;
    int curr_id = 0;
    int i = pos;

    while (i < buffer_len) {
        _8 = (int)buffer[i];
        int save_i = i;
        i += 1;
        cout << "Check byte: " << hex << _8 <<  endl;
        if (_8 == 0) {
            cout << "Struct over: " << endl;
            break;
        }

        int delta = (_8 & 0b11110000) >> 4;
        int type_id = 0b00001111 & _8;
        cout << "Process type ID: " << type_id << endl;
        last_id = curr_id;
        if (delta != 0) {
            curr_id = last_id + delta;
        } else {
            auto some = slice(buffer, i, i+2, buffer_len);
            curr_id = unzag(from_bytes(some.get(), 2, true));
            last_id = curr_id;
            i+=2;
        }
        cout << "Delta " << delta << " ids: " << curr_id << ", " << last_id << endl;
        TCP_TYPE tcpt;
        thrift_compact_processor_t processor_func;

        if (field_map.count(type_id) != 0) {
            tie(tcpt, processor_func) = field_map[type_id];
                cout << "Process field type " << type_id << " as " << TCP_NAMES[tcpt] << endl;
                ParseResult pr2 = ParseResult(tcpt);
               i += (this->*processor_func)(0, i, pr2);
               cout << "Read value as " << pr2.str(0) << endl;
               pr.children.emplace(type_id, pr);
               cout << "i is " << i << endl;
        } else {
            if (type_id == 0) {
                cout << "Done" << endl;
                break;
            }
        }
    }
    return i;
}

int ParquetFooterParser::decodeI32(int options, int pos, ParseResult &pr) {
    int buf = 0;
    int s = 0;
    int i = pos;
    bool has_more = true;
    while (has_more) {
        BuffType n = buffer[i];
        has_more = (0b10000000 & n == 0b10000000);
        buf |= ((n & 0b01111111) << s);
        s += 7;
        i += 1;
    }
    if (options & ZIGZAG != 0) {
        pr.int_value = unzag(buf);
    } else {
        pr.int_value = buf;
    }
    return i;
}

int ParquetFooterParser::decodeTrue(int options, int pos, ParseResult &pr) {
    pr.int_value = 1;
    return 0;
}

int ParquetFooterParser::decodeFalse(int options, int pos, ParseResult &pr) {
    pr.int_value = 0;
    return 0;
}

int ParquetFooterParser::decodeBin(int options, int pos, ParseResult &pr) {
    ParseResult pr2 = ParseResult(TCP_INT32);
    int offset = decodeI32(0, pos, pr2);
    int strlen = pr2.int_value;
    pr2.data = slice(buffer, pos+offset, pos+offset+strlen, buffer_len).get();
    return pos + offset + strlen;
}

int ParquetFooterParser::decodeList(int options, int pos, ParseResult &pr) {
    int i = pos;
    BuffType _8 = buffer[i];
    i += 1;
    int sz = (_8 & 0b11110000) >> 4;
    int type_id = 0b00001111 & _8;
    if (sz <= 14) {
        cout << "Skip <= 14" << endl;
    } else {
        ParseResult pr2 = ParseResult(TCP_INT32);
        int offset = decodeI32(0, i, pr2);
        i += offset;
    }

    TCP_TYPE tcpt;
    thrift_compact_processor_t processor_func;
    tie(tcpt, processor_func) = field_map[type_id];
    vector<ParseResult*> *v =  new vector<ParseResult*>(0);
    for (int j = 0; j < sz; j++) {
        ParseResult pr2 = ParseResult(tcpt);
        i += (this->*processor_func)(0, i, pr2);
        v->push_back(&pr2);
    }
    pr.data = (void *) v;
    return i;
}


int main() {
    ifstream ifs("bytefooter.dat");
    if (!ifs) {
        exit(-1);
    }
    ifs.seekg(0, ifs.end);
    int length = ifs.tellg();
    ifs.seekg(0, ifs.beg);

    BuffType* arr = new BuffType[length];

    ifs.read((char*)arr, length);

    for (int i = 0; i < 10; i++) {
        cout << arr[i] << ", ";
    }
    cout << endl;

    ParquetFooterParser pfp;
    int footer_len = pfp.readFooterLength(arr, length);
    cout << "Footer length " << footer_len << endl;
    auto buff = slice(arr, -(footer_len+8), length-8, length);
    pfp.loadBuffer((BuffType*)buff.get(), footer_len);
    pfp.processBuffer();
    pfp.dumpParseResult();

    delete[] arr;

    return 0;
}
