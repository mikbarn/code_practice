#include <iostream>
#include <cstdlib>

template <typename T>
struct B {
    T * val;
    size_t len;
    size_t end_idx;
    size_t idx = 0;

    B(T * v, size_t l) {
        val = v;
        len = l;
        end_idx = len;
    }


    bool operator!=(const B<T> & other) const {
        return idx != other.end_idx;
    }

    B<T> & operator++() {
        idx += 1;
        return *this;
    }

    T & operator*() const {
        return val[idx];
    }

    B<T> & begin() {
        idx = 0;
        return *this;
    }

    B<T> & end()  {
        return *this;
    }



};

std::ostream & operator<<(std::ostream & os, const B<int> &b) {
    os << "I'm B [" << b.idx << "-" << b.end_idx << "]";
    return os;
}



int main() {
    int arr[4] = {1,2,3,4};
    B<int> a(arr, 4);
    std::cout << a << std::endl;
    for (const auto &x : a) {
        std::cout << x << std::endl;
    }
    return 0;
}

