#ifndef MB_KNAP_CPP
#define MB_KNAP_CPP
#include <iostream>
#include <ctype.h>
#include <memory>
#include <cmath>
#include <cstdlib>
#include <time.h>
#include <array>

using namespace std;

#define F_NOT_SET -1.0
#define I_NOT_SET -1


void die(const char* msg = nullptr) {
    cout << "C'mon man! " << (msg? msg: "\n");
    exit(-1);
}


struct MemoKey {
    int32_t id = I_NOT_SET;
    float_t f = F_NOT_SET;

    MemoKey() {};

    MemoKey(int32_t _id, float_t _f) {
        id = _id;
        f = _f;
    }

    MemoKey& operator=(const MemoKey & other) {
        if (this != &other) {
            this->id = other.id;
            this->f = other.f;
        }
        return *this;
    }

    bool operator==(const MemoKey &other) {
        return (id == other.id && f == other.f);
    }
    bool operator<=(const MemoKey &other) {
        if (id == other.id) {
            return f <= other.f;
        }
        return id <= other.id;
    }
};

struct ListNode {
    unique_ptr<ListNode> next = nullptr;
    float_t val = F_NOT_SET;
    unique_ptr<MemoKey> key = nullptr;
};

class HashMemo { // Giving STL vectors and maps the day off
    public:
        static constexpr uint32_t FNV_PRIME = 0x01000193;
        static constexpr uint32_t FNV_OFFSET_BASIS = 2166136261;
        static constexpr size_t KEY_BYTES_LEN = sizeof(MemoKey);
        HashMemo(uint32_t target_num_elements);
        float_t& operator[] (const MemoKey &key);
        void showCollisions();
        unique_ptr<MemoKey[]> keys();
        uint32_t size() { return num_elements;};
        static void sortKeys(unique_ptr<MemoKey[]> &keys, int low, int high);
    private:
        unique_ptr<ListNode[]> tab;
        unique_ptr<uint32_t[]> collisions;
        uint32_t num_elements;
        uint32_t capacity;
};


void HashMemo::sortKeys(unique_ptr<MemoKey[]> &keys, int low, int high) { // in place quick sort
    int pivot = low + int((high - low) / 2);
    //printf("Sorting... Low: %d Pivot: %d High: %d (%d, %f)\n", low, pivot, high, keys[pivot].id, keys[pivot].f);
    int sorted = low;
    int bound = -1;

    auto debug = [&keys, &low, &high, &pivot, &sorted, &bound](const char* msg) {
        cout << msg << "\n";
        for (int i = low; i <= high; i++) {
            cout << (i == pivot || i == bound? "[" : "(") << keys[i].id << "," << keys[i].f << (i == pivot || i == bound? "] " : ") ");
        }
        cout << endl;
    };

    //debug("Before: ");

    if (!(high > low && pivot <= high && pivot >= low)) {
        printf("Pivot: %d High: %d Low: %d\n", pivot, high, low);
        die("Bad pivot!");
    }
    auto swap = [&keys](int i, int j) {MemoKey tmp = keys[i]; keys[i] = keys[j]; keys[j] = tmp;};

    MemoKey p = keys[pivot];
    for (int i = low; i <= high; i++) {
        if (keys[i] <= p) {
            if (i != pivot) {
                swap(i, sorted);
                sorted++;
            };
            if (sorted == pivot) {
                sorted++;
            }
        }
    }

    bound = sorted > pivot? sorted - 1: sorted;
    swap(bound, pivot);

    //debug("After: ");

    int left_high = bound - 1;
    int right_low = bound + 1;
    if (left_high - low > 0) {
        HashMemo::sortKeys(keys, low, left_high);
    }
    if (high - right_low > 0) {
        HashMemo::sortKeys(keys, right_low, high);
    }
}

unique_ptr<MemoKey[]> HashMemo::keys() {
    auto rval = make_unique<MemoKey[]>(num_elements);
    int j = 0;
    for (int i = 0; i < capacity; i++) {
        int k = 0;
        ListNode * ln = &tab[i];
        while (ln != nullptr && ln->key != nullptr) {
            MemoKey m = *(ln->key);
            //printf("Adding item at [%d][%d] %d (%d, %f)\n", i, k, j, m.id, m.f);
            rval[j++] = *(ln->key);
            k++;
            ln = ln->next.get();
        }
    }
    if (j > num_elements) {
        cout << "j: " << j << " ne: " << num_elements << "\n";
        die("Uh oh");
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
    collisions = make_unique<uint32_t[]>(capacity); // list lengths really
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
            if(ln->key == nullptr) {
                collisions[idx] = (collisions[idx] > j + 1? collisions[idx]: j + 1);
                num_elements++;
            }
            ln->key = make_unique<MemoKey>(key);
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
    printf("Total entries: %d Num Elements: %d Capacity: %d\n", total, num_elements, capacity);
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

void solveKnapsack(int32_t item, float_t W, const float_t* vals, const float_t* weights, HashMemo &memo) {
    if(item < 0 || W <= 0.0) {
        memo[MemoKey{item, W}] = 0.0f;
        return;
    }

    int32_t prev = item - 1;

    if (memo[MemoKey{prev, W}] == F_NOT_SET) {
       solveKnapsack(prev, W, vals, weights, memo);
    }

    if (weights[item] > W) {
        memo[MemoKey{item, W}] = memo[{MemoKey{prev, W}}];
        return;
    }

    float_t with_me = W - weights[item];
    if (memo[MemoKey{prev, with_me}] == F_NOT_SET) {
        solveKnapsack(prev, with_me, vals, weights, memo);
    }

    memo[MemoKey{item, W}] = max(memo[MemoKey{prev, W}], memo[MemoKey{prev, with_me}] + vals[item]);

}

int32_t populateItems(HashMemo &memo, int32_t curr_item, float_t W, const float_t *weights, int32_t *items) {
    int32_t item_idx = 0;
    float_t curr_wt = W;

    while (curr_wt > 0 && curr_item > -2) {
        int32_t next_item = curr_item - 1;
        float_t without_curr = memo[MemoKey{next_item, curr_wt}];
        float_t curr_max = memo[MemoKey{curr_item, curr_wt}];
        printf("%.5f < %.5f ?\n", without_curr, curr_max);
        if(without_curr < curr_max) {
            items[item_idx++] = curr_item;
            curr_wt -= weights[curr_item];
            printf("Added %d\n", curr_item);
        } else {
            printf("Skipping %d\n", curr_item);
        }
        if (memo[MemoKey{next_item, curr_wt}] <= 0.0f) {
            curr_wt = 0.0f;
        }

        curr_item = next_item;
    }
    return item_idx;
}

#endif
