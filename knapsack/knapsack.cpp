#include <iostream>
#include <ctype.h> 
#include <memory>
#include <cmath>
#include <cstdlib>

using namespace std;

#define F_NOT_SET -1.0
#define I_NOT_SET -1

void die() {
    cout << "C'mon man!" << endl;
    exit(-1);     
}

float_t randFloat(float_t min = 0.0, float_t max = 1.0) {
    if (max <= min) {
        die();
    }
    float_t range = max - min;
    return min + (((float_t)rand() / RAND_MAX) * range);
}


struct MemoKey {
    int32_t id = I_NOT_SET;
    float_t f = F_NOT_SET;

    MemoKey() {};

    MemoKey(int32_t _id, float_t _f) {
        id = _id;
        f = _f;
    }

    bool operator==(const MemoKey &other) {
        return (id == other.id && f == other.f);
    }
};

struct ListNode {
    unique_ptr<ListNode> next = nullptr;
    float_t val = F_NOT_SET;
    unique_ptr<MemoKey> key = nullptr;
};

class HashMemo {
    public:
        static constexpr uint32_t FNV_PRIME = 0x01000193;
        static constexpr uint32_t FNV_OFFSET_BASIS = 2166136261;
        static constexpr size_t KEY_BYTES_LEN = sizeof(MemoKey);
        HashMemo(uint32_t target_num_elements);
        float_t& operator[] (const MemoKey &key);
        void showCollisions();
        unique_ptr<MemoKey[]> keys();
        uint32_t size() { return num_elements;};
    private:
        unique_ptr<ListNode[]> tab;
        unique_ptr<uint32_t[]> collisions;
        uint32_t num_elements;
        uint32_t capacity; 
};

unique_ptr<MemoKey[]> HashMemo::keys() {
    auto rval = make_unique<MemoKey[]>(num_elements);
    int j = 0;
    for (int i = 0; i < capacity; i++) {
        ListNode * ln = &tab[i];
        while (ln != nullptr && ln->key != nullptr) {
            rval[j++] = *(ln->key); 
            ln = ln->next.get();   
        }
    }
    return rval;
}

HashMemo::HashMemo(uint32_t target_num_elements) {
    if (target_num_elements > 2147483000) {
        die();
    }
    num_elements = 0;
    capacity = 2 * target_num_elements;
    tab = make_unique<ListNode[]>(capacity);
    collisions = make_unique<uint32_t[]>(capacity);
}

float_t& HashMemo::operator[] (const MemoKey &key) {
    uint32_t hash = FNV_OFFSET_BASIS;
    char * nxt = (char*)&key;
    for (int i = 0; i < KEY_BYTES_LEN; i++) {
        hash ^= (char)*(nxt+i);
        hash *= FNV_PRIME;
    }
    uint32_t idx = hash % capacity;
    ListNode * ln = &tab[idx];
    
    ListNode * prev;
    int j = 0;
    while (ln != nullptr) {
        if ((ln->key == nullptr) || (*(ln->key) == key)) {
            ln->key = make_unique<MemoKey>(key);
            num_elements -= collisions[idx];
            collisions[idx] = j + 1;
            num_elements += collisions[idx];
            return ln->val;
        }
        j++;
        prev = ln;
        ln = ln->next.get();
    }
    prev->next = make_unique<ListNode>();
    prev->next->key = make_unique<MemoKey>(key);
    num_elements -= collisions[idx];
    collisions[idx] = j + 1;
    num_elements += collisions[idx];
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
    printf("Total entries: %d Num Elements: %d\n", total, num_elements);
    printf("Load Factor: %f\n", (float_t)total / capacity);
}

void quickHashTest() {
    printf("Rough-testing hash table...\n");
    HashMemo hm(100);   
    const int LIM = 1000;
    float f = 0.0;
    const float inc = (1 / 1000) / 10;
    for (int i = 0; i < LIM; i++) {
        hm[MemoKey{i,f}] = i + f - inc;
        hm[MemoKey{i,f}] = i + f;
        f += inc;
    }
    f = 0.0;   
    int matches = 0; 
    for (int i = 0; i < LIM; i++) {
        matches += (hm[MemoKey{i,f}] == i + f? 1: 0);
        f += inc;
    }
    hm.showCollisions();    
    printf("Found %d / %d successful matches\n", matches, LIM);
}

void solveKnapsack(int32_t item, float_t W, unique_ptr<float_t[]> &vals, unique_ptr<float_t[]> &weights, HashMemo &memo) {
    if(item == 0 || W <= 0.0) {
        memo[MemoKey{item, W}] = vals[item];
        return;
    }

    int32_t prev = item - 1;

    if (memo[MemoKey{prev, W}] == F_NOT_SET) {
       solveKnapsack(prev, W, vals, weights, memo);
    }

    float_t without_me = W - weights[item];
    if (memo[MemoKey{prev, without_me}] == F_NOT_SET) {
        solveKnapsack(prev, without_me, vals, weights, memo);
    }

    memo[MemoKey{item, W}] = max(memo[MemoKey{prev, W}], memo[MemoKey{prev, without_me}] + vals[item]);

}

int main() {
    quickHashTest();

    uint32_t n = 10;
    auto vals = make_unique<float_t[]>(n);
    auto weights = make_unique<float_t[]>(n);
    for (int i = 0; i < n; i++) {
        vals[i] = randFloat(0.0001, 10.0000);
        weights[i] = randFloat(0.001, 100.0000);
        printf("Adding item[%03d] v=%05.5f w=%05.5f\n", i, vals[i], weights[i]);
    }

    float_t W = 50.0;
    HashMemo hm(n);

    solveKnapsack(n-1, W, vals, weights, hm);

    printf("Knapsack memo: \n");
    unique_ptr<MemoKey[]> keys = hm.keys();
    for (int i = 0; i < hm.size(); i++) {
        printf("[%03d, %5.5f]: %5.5f\n", keys[i].id, keys[i].f, hm[keys[i]]);
    }

    return 0;
}