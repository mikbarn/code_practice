#include <iostream>
// #include <unistd.h>
// #include <cstring>
#include <memory>
using namespace std;



// unique_ptr<float[]> readWeights() {

// }

struct Item {
    uint32_t id;
    float_t weight;
    float_t value;

    Item(uint32_t id, float_t weight, float_t value) {
        id = id;
        weight = weight;
        value = value;
    }
};

template<typename T> struct ListNode {
    unique_ptr<ListNode<T>> next = nullptr;
    T payload;
};

struct MemoKey {
    uint32_t id;
    float_t f;
};

class HashMemo {
    public:
        static constexpr uint32_t FNV_PRIME = 0x01000193;
        static constexpr uint32_t FNV_OFFSET_BASIS = 2166136261;
        static constexpr size_t KEY_BYTES_LEN = sizeof(MemoKey);
        HashMemo(uint32_t num_elements);
        ~HashMemo();
        bool Contains(uint32_t id, float_t f);
    private:
        unique_ptr<ListNode<float_t>[]> tab;
        uint32_t num_elements;
        uint32_t capacity;

    float_t& operator[] (MemoKey &key);
};



HashMemo::HashMemo(uint32_t num_elements) {
    num_elements = num_elements;
    capacity = 2 * num_elements;
    tab = make_unique<ListNode<float_t>[]>(capacity);
}

float_t& HashMemo::operator[] (MemoKey &key) {
    uint32_t hash = FNV_OFFSET_BASIS;
    byte * nxt = (byte*)&key;
    for (int i = 0; i < KEY_BYTES_LEN; i++) {
        hash *= FNV_PRIME;
        hash ^= (uint32_t)*(nxt+i);
    }
    uint32_t idx = hash % capacity;
    ListNode<float> * ln = &tab[idx];

    unique_ptr<ListNode<float_t>> prev;
    while (ln != nullptr) {
        if (ln->payload == key.f) {
            return ln->payload;
        }
        // prev = ln;
        // ln = ln->next;
    }
    // prev->next = ListNode



}



int main() {

}