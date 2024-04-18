#include <map>
#include <tuple>
#include <string>
#include <functional>
#include <iostream>

using namespace std;

typedef unsigned char BuffType;
#define BUFF_SIZE 8

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


class ParquetFooterParser {
    typedef void (ParquetFooterParser::*func)(int);

    static const int ZIGZAG = 1;

    struct ParseResult {
        int offset = 0;
        int length = 0;
        string type;
        union {
            void* data = nullptr;
            int int_value;

        };
    };

    public:
        ParquetFooterParser();
        ~ParquetFooterParser();
        void loadBuffer(const char* bytes, int len);
        void processBuffer();

    private:
        BuffType* buffer = nullptr;
        int buffer_len = 0;
        int pos = 0;

        ParseResult& decodeI32(int options);
        ParseResult& decodeList(int options);
        ParseResult& decodeStruct(int options);
        ParseResult& decodeTrue(int options);
        ParseResult& decodeFalse(int options);
        ParseResult& decodeBin(int options);
        ParseResult& decodeMap(int options);
        ParseResult& decodeStruct(int options);

        // shared_ptr<BuffType[]> slice(BuffType *buff, int start, int end);

        inline int unzag(int i) { return (i >> 1) ^ -(i & 1); };
        void clean();
        map<int, tuple<string, func>> field_map = {
            {1, {"true", &decodeTrue}},
            {2, {"false", &decodeFalse}},
            {4, {"i16", &decodeI32}},
            {5, {"i32", &decodeI32}},
            {6, {"i64", &decodeI32}},
            {8, {"string", &decodeBin}},
            {9, {"list", &decodeList}},
            {11, {"map", &decodeMap}},
            {12, {"struct", &decodeStruct}}

        };

};



ParquetFooterParser::ParquetFooterParser() {

}

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


ParquetFooterParser::ParseResult& ParquetFooterParser::decodeStruct(int options) {
    char _8;// = buffer[i];
    int last_id = 0;
    int curr_id = 0;
    ParseResult res = ParseResult();

    while (pos < buffer_len) {
        _8 = buffer[pos];
        int save_i = pos;
        pos += 1;
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
            auto some = slice(buffer, pos, pos+2);
            last_id = curr_id = unzag(from_bytes(some.get(), 2, true));
            pos+=2;
        }
        // #print('Field IDs (last, current): ', last_id, curr_id)
        // #print('Field type: ', type_id)
        // if type_id in FIELDS:
        //     name, processor = FIELDS[type_id]
        //     # print('Processing as ', name)
        //     val, offset = processor(b[i:])
        //     #print(f'Read {offset} bytes as value: {val}. Raw: {b[i:i+offset]} Remain: {b[i+offset:i+offset+12]}...')
        //     i += offset
        // elif type_id == 0:
        //     print('!!Stop!!')
        //     break
        // else:
        //     raise NotImplementedError(type_id, b[save_i:])
        // res[curr_id] = val
        string name;
        func processor;
        if (field_map.count(type_id) != 0) {
            tie(name, processor) = field_map[type_id];
                cout << "Process field type " << type_id << " as " << name << endl;


        } else {
            if (type_id == 0) {
                cout << "Done" << endl;
                break;
            }
        }
        }

    return res;
    // return res, i
}

ParquetFooterParser::ParseResult& ParquetFooterParser::decodeI32(int options) {
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

    }
}

// def unzag(i):
//     return (i >> 1) ^ -(i & 1)

// def decode_i32(b: bytes, zigzag=True):
//     buf = 0
//     s = 0
//     i = 0
//     has_more = True
//     while has_more:
//         n = b[i]
//         # print('Checking next byte: ', bin(n))
//         has_more = (0b10000000 & n == 0b10000000)
//         buf |= ((n & 0b01111111) << s)
//         s += 7
//         i += 1
//     if zigzag:
//         return unzag(buf), i
//     return buf, i


// def decode_list(b: bytes):
//     i = 0
//     _8 = b[i]
//     i += 1
//     # print('Check ', bin(_8))
//     sz = (_8 & 0b11110000) >> 4
//     type_id = 0b00001111 & _8
//     if sz <= 14:
//         pass
//     else:
//         sz, offset = decode_i32(b[i:], zigzag=False)
//         i += offset

//     elements = []
//     try:
//         name, processor = FIELDS[type_id]
//         #print(f'Process list of {name}. Size: {sz}')
//     except:
//         print('Error!')
//         print(type_id)
//         print(b)
//         raise
//     # print(f'Processing  {sz} list items as {name}')
//     for idx in range(0, sz):
//         val, offset = processor(b[i:])
//         i += offset
//         #print(f'IDX {idx} - Adding Element: {val}!! ',)
//         elements.append(val)
//     #print(f'Finish list of {name}')
//     return elements, i


// def decode_bin(b: bytes):
//     strlen, offset = decode_i32(b, zigzag=False)
//     # print(f'getting string of len {strlen}')
//     return b[offset:offset+strlen], offset+strlen

// def decode_true(b: bytes):
//     return True, 0

// def decode_false(b: bytes):
//     return False, 0

// def decode_bool(b: bytes):
//     return True if b[0] == b'1' else False, 1

// def decode_map(b: bytes):
//     raise NotImplementedError('Test this')
//     _8 = b[0]
//     res = {}
//     if _8 == 0:
//         return res, 1
//     i = 0
//     sz, offset = decode_i32(b[i:])
//     i += offset
//     #print(f"Processing map of {sz} elements... {b[i:i+100]}")
//     _8 = b[i]
//     i += 1
//     k_type = (_8 & 0b11110000) >> 4
//     v_type = 0b00001111 & _8
//     name, k_handler = FIELDS[k_type]
//     v_name, v_handler = FIELDS[v_type]
//     for idx in range(0, sz):
//         _k, offset = k_handler(b[i:])
//         i += offset
//         _v, offset = k_handler(b[i:])
//         i += offset
//         res[_k] = _v
//     return res, i

// def decode_struct(b: bytes):
//     i = 0
//     _8 = b[i]
//     last_id = 0
//     curr_id = 0
//     res = {}

//     while i < len(b):
//         _8 = b[i]
//         save_i = i
//         i += 1
//         # print('Checking byte: ', bin(_8))
//         if _8 == 0:
//             #print(f'Stopping struct!?!?!? {_8}, {b[:8]} {len(b)}')
//             break
//         delta = (_8 & 0b11110000) >> 4
//         type_id = 0b00001111 & _8
//         last_id = curr_id
//         if delta != 0:
//             curr_id = last_id + delta
//         else:
//             last_id = curr_id = unzag(int.from_bytes(b[i:i+2], 'little'))
//             i += 2
//         #print('Field IDs (last, current): ', last_id, curr_id)
//         #print('Field type: ', type_id)
//         if type_id in FIELDS:
//             name, processor = FIELDS[type_id]
//             # print('Processing as ', name)
//             val, offset = processor(b[i:])
//             #print(f'Read {offset} bytes as value: {val}. Raw: {b[i:i+offset]} Remain: {b[i+offset:i+offset+12]}...')
//             i += offset
//         elif type_id == 0:
//             print('!!Stop!!')
//             break
//         else:
//             raise NotImplementedError(type_id, b[save_i:])
//         res[curr_id] = val
//     return res, i

// def get_footer_len(tail:bytes):
//     assert tail[-4:] == b'PAR1'
//     assert len(tail) >= 8
//     fl = int.from_bytes(tail[-8:-4], byteorder='little')
//     return fl

// def trim_footer(tail:bytes):
//     _l = get_footer_len(tail)
//     total = 8+_l
//     _real_len = len(tail)
//     assert _real_len >= total
//     start = _real_len-total
//     return tail[start:-8]


// class DT:
//     BOOL, INT32, INT64, INT96, FLOAT, DOUBLE, BYTES, FIXED_LEN_BYTES = 'bool', 'int32', 'int64', 'int96', 'float', 'double', 'bytes', 'fixed_len_bytes'

// DType = {
// 0: DT.BOOL,   # BOOLEAN: 1 bit boolean
// 1: DT.INT32,  # INT32: 32 bit signed ints
// 2: DT.INT64, # INT64: 64 bit signed ints
// 3: DT.INT96,  # INT96: 96 bit signed ints
// 4: DT.FLOAT,  # FLOAT: IEEE 32-bit floating point values
// 5: DT.DOUBLE, # DOUBLE: IEEE 64-bit floating point values
// 6: DT.BYTES,  # BYTE_ARRAY: arbitrarily long byte arrays
// 7: DT.FIXED_LEN_BYTES, # FIXED_LEN_BYTE_ARRAY: fixed length byte arrays
// }


// # union LogicalType {
// #   1:  StringType STRING       // use ConvertedType UTF8
// #   2:  MapType MAP             // use ConvertedType MAP
// #   3:  ListType LIST           // use ConvertedType LIST
// #   4:  EnumType ENUM           // use ConvertedType ENUM
// #   5:  DecimalType DECIMAL     // use ConvertedType DECIMAL + SchemaElement.{scale, precision}
// #   6:  DateType DATE           // use ConvertedType DATE

// #   // use ConvertedType TIME_MICROS for TIME(isAdjustedToUTC = *, unit = MICROS)
// #   // use ConvertedType TIME_MILLIS for TIME(isAdjustedToUTC = *, unit = MILLIS)
// #   7:  TimeType TIME

// #   // use ConvertedType TIMESTAMP_MICROS for TIMESTAMP(isAdjustedToUTC = *, unit = MICROS)
// #   // use ConvertedType TIMESTAMP_MILLIS for TIMESTAMP(isAdjustedToUTC = *, unit = MILLIS)
// #   8:  TimestampType TIMESTAMP

// #   // 9: reserved for INTERVAL
// #   10: IntType INTEGER         // use ConvertedType INT_* or UINT_*
// #   11: NullType UNKNOWN        // no compatible ConvertedType
// #   12: JsonType JSON           // use ConvertedType JSON
// #   13: BsonType BSON           // use ConvertedType BSON
// #   14: UUIDType UUID           // no compatible ConvertedType
// # }


// def get_file_columns_from_footer(footer):
//     res, i = decode_struct(trim_footer(footer))
//     return _get_col_spec(res)

// def _get_col_spec(_map):
//     rval = []
//     for schema_elem in _map[2][1:]: # first entry unknown format
//         dtype = DType[schema_elem[1]]
//         val = {'data_type': dtype,  'name': schema_elem[4].decode()}
//         if 10 in schema_elem:
//             logical_type = schema_elem[10]
//             if 8 in logical_type:
//                 adj_to_utc = logical_type[8][1]
//                 val['logical_type'] = 'timestamp without time zone' if not adj_to_utc else 'timestamp with time zone'
//         rval.append(val)
//     return rval

// FIELDS = {
//     1: ('true', decode_true),
//     2: ('false', decode_false),
//     4: ('i16', decode_i32),
//     5: ('i32', decode_i32),
//     6: ('i64', decode_i32),
//     9: ('list', decode_list),
//     8: ('bin/string', decode_bin),
//     11: ('map', decode_map),
//     12: ('struct', decode_struct)
// }

// # LIST_TYPE = {
// #     2: ('bool', decode_bool),
// #     3: ('bin/string', decode_bin),
// #     6: ('i16', decode_i32),
// #     8: ('i32', decode_i32),
// #     10: ('i64', decode_i32),
// #     11: ('bin/string', decode_bin),
// #     12: ('struct', decode_struct),
// #     14: ('set', decode_list),
// #     15: ('list', decode_list),
// # }

// if __name__ == '__main__':
//     import os
//     with open(os.path.expanduser('~/Desktop/output.parquet'), 'rb') as fd:
//         data = fd.read()
//     ft = trim_footer(data)
//     print(ft[:30])
//     res, i = decode_struct(ft)

//     print()
//     import pprint
//     pprint.pprint(res)
//     import pandas as pd, os
//     print(_get_col_spec(res))
//     exit(0)

// #df = pd.read_parquet(os.