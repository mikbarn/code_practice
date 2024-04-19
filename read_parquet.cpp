#include <map>
#include <tuple>
#include <string>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

using namespace std;

typedef unsigned char BuffType;
#define BUFF_SIZE 8

class NotImplementedException : public std::logic_error
{
public:
    NotImplementedException() : std::logic_error("Function not yet implemented") { };
};

shared_ptr<BuffType[]> slice(const BuffType *buff, int start, int end) {
    int sz = end - start;
    auto rv = shared_ptr<BuffType[]>(new BuffType[sz]);
    int j = 0;
    for (int i = start; i < end; i++) {
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
                    delete (vector<ParseResult>*) data;
                    break;
                case TCP_STRING:
                    delete[] (BuffType*) data;
                    break;                    
                
                default:
                    break;
                }
            }
        }
        map<int, ParseResult&> children;
    };

    public:
        ParquetFooterParser();
        ~ParquetFooterParser();
        void loadBuffer(const char* bytes, int len);
        void processBuffer();

    private:
        BuffType* buffer = nullptr;
        int buffer_len = 0;

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



ParquetFooterParser::ParquetFooterParser() { }

ParquetFooterParser::~ParquetFooterParser() {
    clean();
}

void ParquetFooterParser::clean() {
    if (buffer != nullptr) {
        delete[] buffer;
    }
}

void ParquetFooterParser::loadBuffer(const char* bytes, int len) {
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
    BuffType _8;// = buffer[i];
    int last_id = 0;
    int curr_id = 0;
    int i = pos;

    while (i < buffer_len) {
        _8 = buffer[i];
        int save_i = i;
        i += 1;
        cout << "Check byte: " << _8 << endl;
        if (_8 == 0) {
            cout << "Struct over: " << endl;
            break;
        }
        int delta = (_8 & 0b11110000) >> 4;
        int type_id = 0b00001111 & _8;
        last_id = curr_id;
        if (delta == 0) {
            curr_id = last_id + delta;
        } else {
            auto some = slice(buffer, i, i+2);
            last_id = curr_id = unzag(from_bytes(some.get(), 2, true));
            i+=2;
        }

        TCP_TYPE tcpt;
        thrift_compact_processor_t processor_func;
        
        if (field_map.count(type_id) != 0) {
            tie(tcpt, processor_func) = field_map[type_id];
                cout << "Process field type " << type_id << " as " << TCP_NAMES[tcpt] << endl;
                ParseResult pr2 = ParseResult(tcpt);
               i += (this->*processor_func)(0, i, pr2);
               pr.children.emplace(type_id, pr); 
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
    pr2.data = slice(buffer, pos+offset, pos+offset+strlen).get();
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
    vector<ParseResult> *v =  new vector<ParseResult>(0);
    for (int j = 0; j < sz; j++) {
        ParseResult pr2 = ParseResult(tcpt);
        i += (this->*processor_func)(0, i, pr2);
        v->push_back(pr2);
    }
    pr.data = (void *) v;
    return i;
}
