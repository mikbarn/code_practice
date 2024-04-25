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

string slice(const BuffType *buff, int start, int end, int arr_sz) {
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

    auto rv = new char[sz];
    int j = 0;
    for (int i = start; i != end; i+=inc) {
        rv[j++] = buff[i];
    }
    string rval = string(rv, sz);
    delete[] rv;
    return rval;
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

int from_bytes(const char* buff, int len, bool little) {
    return from_bytes((BuffType*) buff, len, little);
}


void dump_arr(const BuffType *buff, int sz) {
    cout << "[";
    for (int i = 0; i < sz; i++) {
        cout << buff[i] << (i < sz - 1? ",": "");
    }
    cout << "]" << endl;
}

enum TCP_TYPE { // thrift compact protocol
    TCP_INT8,
    TCP_INT16,
    TCP_INT32,
    TCP_INT64,
    TCP_TRUE,
    TCP_FALSE,
    TCP_LIST,
    TCP_STRUCT,
    TCP_MAP,
    TCP_SET,
    TCP_BINARY,
    TCP_BOOL,
    TCP_UUID,
    TCP_DOUBLE
};

const string TCP_NAMES[] = {
    "TCP_INT8",
    "TCP_INT16",
    "TCP_INT32",
    "TCP_INT64",
    "TCP_TRUE",
    "TCP_FALSE",
    "TCP_LIST",
    "TCP_STRUCT",
    "TCP_MAP",
    "TCP_SET",
    "TCP_BINARY",
    "TCP_BOOL",
    "TCP_UUID",
    "TCP_DOUBLE"
};


struct ParseResult {
    static constexpr char SPACE[] = "   ";
    const TCP_TYPE tcp_type;
    union {
        void* data = nullptr;
        int int_value;
        long long_value;
    };
    ParseResult(TCP_TYPE t): tcp_type(t)  {
    }

    ~ParseResult() {
        if (data != nullptr) {
            switch (tcp_type)
            {
                case TCP_LIST:
                case TCP_STRUCT:
                    {
                    auto ptr = getChildren();
                    if (ptr == nullptr) {
                        break;
                    }
                    for (int i = 0; i < ptr->size(); i++) {
                        delete (*ptr)[i];
                    }
                    delete ptr;
                    }
                    break;
                case TCP_BINARY:
                    delete (string*) data;
                    break;

                default:
                    break;
            }
        }
    }
    void fmtData(stringstream &ss, int indent, string margin) {
        auto ptr = (map<int, ParseResult*>*) data;
        for (const auto& [key, value]: *ptr) {
            ss << margin << key << ": " << value->str(indent+1) << endl;
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
                case TCP_TRUE:
                case TCP_FALSE:
                    ss << name << ": " << int_value;
                    return  ss.str();
                case TCP_INT64:
                    ss << name << ": " << long_value;
                    return ss.str();
                case TCP_BINARY:
                    ss << name << ": " << *((string*)data);
                    return ss.str();
                case TCP_LIST:
                {
                    ss << endl << margin << "[" << endl;
                    fmtData(ss, indent, margin);
                    ss << margin << "]" << endl;
                    return ss.str();
                }
                case TCP_STRUCT:
                {
                    if (data == nullptr) {
                        return string("Empty Struct");
                    }
                    ss << endl << margin << "{" << endl;
                    fmtData(ss, indent, margin);
                    ss << margin << "}" <<  endl;
                    return ss.str();
                }
                default:
                    return "Unsupported: " + name;
            }
    }
    map<int, ParseResult*>* getChildren() {
        if (data == nullptr) {
           data = new map<int, ParseResult*>();
        }
        return (map<int, ParseResult*>*)data;
    }

    void addChildSeq(ParseResult *pr) {
        auto ptr = getChildren();
        ptr->emplace((*ptr).size()+1, pr);
    }

    void addChildId(ParseResult *pr, int id) {
        auto ptr = getChildren();
        ptr->emplace(id, pr);
    }
};

class ParquetFooterParser {
    static const int ZIGZAG = 1;

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
        int pos = 0;
        ParseResult final_pr = ParseResult(TCP_STRUCT);

        void decodeInt(int options, ParseResult *pr);
        void decodeLong(int options, ParseResult *pr);
        void decodeList(int options, ParseResult *pr);
        void decodeStruct(int options, ParseResult *pr);
        void decodeTrue(int options, ParseResult *pr);
        void decodeFalse(int options, ParseResult *pr);
        void decodeBin(int options, ParseResult *pr);
        void decodeMap(int options, ParseResult *pr);
        template <typename T> T decodeIntegerType(int);

        ParseResult * callDecoderNewField(int type_id, ParseResult *parent);

        inline int unzag(int i) { return (i >> 1) ^ -(i & 1); };
        inline long unzag(long i) { return (i >> 1) ^ - (i & 1); };
        void clean();
        typedef void (ParquetFooterParser::*thrift_compact_processor_t)(int, ParseResult *);
        map<TCP_TYPE, thrift_compact_processor_t> decoder_map = {
            {TCP_TRUE, &ParquetFooterParser::decodeTrue},
            {TCP_FALSE, &ParquetFooterParser::decodeFalse},
            {TCP_INT16, &ParquetFooterParser::decodeInt},
            {TCP_INT32, &ParquetFooterParser::decodeInt},
            {TCP_INT64, &ParquetFooterParser::decodeLong},
            {TCP_BINARY, &ParquetFooterParser::decodeBin},
            {TCP_LIST, &ParquetFooterParser::decodeList},
            {TCP_MAP, &ParquetFooterParser::decodeMap},
            {TCP_STRUCT, &ParquetFooterParser::decodeStruct}
        };

    TCP_TYPE resolveType(int type_id, bool is_struct);

};

TCP_TYPE ParquetFooterParser::resolveType(int type_id, bool is_struct) {
    switch (type_id) {
        case 1:
            return TCP_TRUE;
        case 2:
            return (is_struct? TCP_FALSE : TCP_BOOL);
        case 3:
            return TCP_INT8;
        case 4:
            return TCP_INT16;
        case 5:
            return TCP_INT32;
        case 6:
            return TCP_INT64;
        case 7:
            return TCP_DOUBLE;
        case 8:
            return TCP_BINARY;
        case 9:
            return TCP_LIST;
        case 10:
            return TCP_SET;
        case 11:
            return TCP_MAP;
        case 12:
            return TCP_STRUCT;
        case 13:
            return TCP_UUID;
        default:
            throw NotImplementedException();
    }
}

int ParquetFooterParser::readFooterLength(BuffType *bytes, int len) {
    if (len < 8) {
        return -1;
    }
    string junk = slice(bytes, -4, len, len);

    if (junk != string("PAR1")) {
        cout << "Unexpected value of last 4: " << junk << endl;
    }
    string bits = slice(bytes, -8, -4, len);
    int footer_len = from_bytes(bits.c_str(), 4, true);
    return footer_len;
}

void ParquetFooterParser::dumpParseResult() {
    cout << "Printing final parse result ... " << endl;
    cout << final_pr.str(0) << endl;
}

void ParquetFooterParser::processBuffer() {
    decodeStruct(0, &final_pr);
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


void ParquetFooterParser::decodeMap(int options, ParseResult *pr) {
    throw NotImplementedException();
}

ParseResult* ParquetFooterParser::callDecoderNewField(int type_id, ParseResult *parent) {
    TCP_TYPE tcpt = resolveType(type_id, (parent->tcp_type == TCP_STRUCT));
    thrift_compact_processor_t processor_func = decoder_map[tcpt];
    cout << "Process field type " << type_id << " as " << TCP_NAMES[tcpt] << endl;
    ParseResult *pr2 = new ParseResult(tcpt);
    (this->*processor_func)(0, pr2);
    cout << "Read value as " << pr2->str(0) << endl;
    return pr2;
}

void ParquetFooterParser::decodeStruct(int options, ParseResult *pr) {
    int _8;
    int last_id = 0;
    int curr_field_id = 0;

    while (pos < buffer_len) {
        _8 = (int)buffer[pos];
        pos += 1;
        cout << "Check byte: " << hex << _8 <<  endl;
        if (_8 == 0) {
            cout << "Struct over: " << endl;
            break;
        }

        int delta = (_8 & 0b11110000) >> 4;
        int type_id = 0b00001111 & _8;

        last_id = curr_field_id;
        if (delta != 0) {
            curr_field_id = last_id + delta;
        } else {
            string some = slice(buffer, pos, pos + 2, buffer_len);
            curr_field_id = unzag(from_bytes(some.c_str(), 2, true));
            last_id = curr_field_id;
            pos += 2;
        }

        cout << "Delta: " << delta << " Curr Field ID: " << curr_field_id << endl;

        if (type_id == 0) {
            cout << "Done" << endl;
            break;
        } else {
            ParseResult *pr_child = callDecoderNewField(type_id, pr);
            pr->addChildId(pr_child, curr_field_id);
            cout << "Child " << curr_field_id << " added to struct" << endl;
        }

    }
}

template <typename T> T ParquetFooterParser::decodeIntegerType(int options) {
    T buf = 0;
    int s = 0;
    bool has_more = true;
    while (has_more) {
        BuffType n = buffer[pos];
        has_more = ((0b10000000 & n) == 0b10000000);
        buf |= ((n & 0b01111111) << s);
        s += 7;
        pos += 1;
    }
    if (options & ZIGZAG != 0) {
        buf = unzag(buf);
    }
    return buf;
}

// void ParquetFooterParser::decodeInt(int options, ParseResult *pr) {
//     int buf = 0;
//     int s = 0;
//     bool has_more = true;
//     while (has_more) {
//         BuffType n = buffer[pos];
//         has_more = ((0b10000000 & n) == 0b10000000);
//         buf |= ((n & 0b01111111) << s);
//         s += 7;
//         pos += 1;
//     }

//     if (options & ZIGZAG != 0) {
//         buf = unzag(buf);
//     }
//     pr->int_value = buf;
// }

void ParquetFooterParser::decodeInt(int options, ParseResult *pr) {
    int buf = decodeIntegerType<int>(options);
    pr->int_value = buf;
}

void ParquetFooterParser::decodeLong(int options, ParseResult *pr) {
    long buf = decodeIntegerType<long>(options);
    pr->long_value = buf;
}

void ParquetFooterParser::decodeTrue(int options, ParseResult *pr) {
    pr->int_value = 1;
}

void ParquetFooterParser::decodeFalse(int options, ParseResult *pr) {
    pr->int_value = 0;
}

void ParquetFooterParser::decodeBin(int options, ParseResult *pr) {
    ParseResult pr_temp = ParseResult(TCP_INT32);
    decodeInt(0, &pr_temp);
    int strlen = pr_temp.int_value;
    cout << "Read string length of " << strlen << " from " << hex << (int) buffer[pos-1] <<  "?" << endl;
    string val = slice(buffer, pos, pos + strlen, buffer_len);
    pos += strlen;
    pr->data = new string(val);
    cout << "Str is " << pr->str(0) << endl;
}

void ParquetFooterParser::decodeList(int options, ParseResult *pr) {
    BuffType _8 = buffer[pos];
    pos += 1;
    int sz = (_8 & 0b11110000) >> 4;
    int type_id = 0b00001111 & _8;
    if (sz <= 14) {
        cout << "Skip <= 14" << endl;
    } else {
        ParseResult pr_temp = ParseResult(TCP_INT32);
        decodeInt(0, &pr_temp);
        sz = pr_temp.int_value;
    }

    cout << "Generate list of " << sz << " " << TCP_NAMES[resolveType(type_id, false)] << " ID=" << dec << type_id <<  endl;
    for (int i = 0; i < sz; i++) {
        ParseResult *pr_child = callDecoderNewField(type_id, pr);
        pr->addChildSeq(pr_child);
        cout << "Added child " << i+1 << " to list" << endl;
    }
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
    string buff = slice(arr, -(footer_len+8), length-8, length);
    pfp.loadBuffer((BuffType*)buff.c_str(), footer_len);


    delete[] arr;

    pfp.processBuffer();

    cout << "Dumping results: " << endl;
    pfp.dumpParseResult();

    return 0;
}
