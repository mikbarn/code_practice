#include <iostream>
// #include <unistd.h>
// #include <cstring>
#include <memory>
#include <cmath>

using namespace std;



// unique_ptr<float[]> readWeights() {

// }



struct ListNode {
    unique_ptr<ListNode> next = nullptr;
    float_t val = -1.0;
    int32_t id = -1;
};

struct MemoKey {
    int32_t id = -1;
    float_t f = -1;

    MemoKey(int32_t _id, float_t _f) {
        id = _id;
        f = _f;
    }
};

class HashMemo {
    public:
        static constexpr uint32_t FNV_PRIME = 0x01000193;
        static constexpr uint32_t FNV_OFFSET_BASIS = 2166136261;
        static constexpr size_t KEY_BYTES_LEN = sizeof(MemoKey);
        HashMemo(uint32_t num_elements);
        float_t& operator[] (const MemoKey &key);
        void showCollisions();
    private:
        unique_ptr<ListNode[]> tab;
        unique_ptr<uint32_t[]> collisions;
        uint32_t num_elements;
        uint32_t capacity; 
};


HashMemo::HashMemo(uint32_t num_elements) {
    num_elements = num_elements;
    capacity = 2 * num_elements;
    tab = make_unique<ListNode[]>(capacity);
    collisions = make_unique<uint32_t[]>(capacity);
}

float_t& HashMemo::operator[] (const MemoKey &key) {
    uint32_t hash = FNV_OFFSET_BASIS;
    char * nxt = (char*)&key;
    for (int i = 0; i < KEY_BYTES_LEN; i++) {
        hash *= FNV_PRIME;
        hash ^= (char)*(nxt+i);
    }
    uint32_t idx = hash % capacity;
    ListNode * ln = &tab[idx];
    
    ListNode * prev;
    int j = 0;
    while (ln != nullptr) {
        if ((ln->id == -1) || (ln->id == key.id)) {
            ln->id = key.id;
            collisions[idx] = j + 1;
            return ln->val;
        }
        j++;
        prev = ln;
        ln = ln->next.get();
    }
    prev->next = make_unique<ListNode>();
    prev->next->id = key.id;
    collisions[idx] = j + 1;
    return prev->next->val;
}

void HashMemo::showCollisions() {
    int total = 0;
    for (int i = 0; i < capacity; i++) {
        printf("[%03d]: %d\t", i, collisions[i]);
        if (i != 0 && (i+1) % 5 == 0)
            printf("\n");
        total += collisions[i];
    }
    printf("Total entries: %d\n", total);
    printf("Load Factor: %f\n", (float_t)total / capacity);
}



int main() {
    HashMemo hm(100);
   
   MemoKey z(0,0.0);
    int limit = 100;
    int id = 0;
    float f = 0.0;
    MemoKey mk(id,f);
    for (int i = 0; i < limit; i++) {
        mk.id = id;
        mk.f = f;
        float val = 1.001 * i;
        hm[mk] = val; 
        cout << "Assigned at (" << mk.id << "," << mk.f << ") val: " << val <<  " Now is: " << hm[mk] << endl;
        f+=.1;
        id++;        
    }

    id = 0;
    f = 0.0;    
    for (int i = 0; i < limit; i++) {
        mk.id = id;
        mk.f = f;
        cout << "Entry at (" << mk.id << "," << mk.f << ") stores: " << hm[mk] << endl;
        f+=.1;
        id++;        
    }
    
    hm.showCollisions();
}