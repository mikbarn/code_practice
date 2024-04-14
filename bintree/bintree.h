#ifndef MB_RB_BINTREE_H
#define MB_RB_BINTREE_H
#include <string>
#include <list>

enum Color {RED, BLACK};
enum Dir {LEFT, RIGHT};


template <typename T> struct Node {
    T &t;
    Node *left, *right, *parent;
    Node(const T &t) {
        t = t;
        col = RED;
    }

    Color col;
};

template <typename T> struct Side {
    Dir side;
    Side(Node<T> *n, Node<T> *p) {
        if (n == p.left) {
            side = LEFT;
            return;
        }
        side = RIGHT;
    }
    Node<T>* other(Node<T> *n) {
        if (side == LEFT) {
            return n.right;
        }
        return  n.left;
    }
    static Dir oppose(Side s) {
        return s.side == LEFT? RIGHT : LEFT;
    }
};


template <typename T> class BinTree {
    public:
    Node<T> *root;
    BinTree(const T &t);

    std::list<T> Inorder();
    void insert(const T &t);
    void remove(const T &t);
    bool exists(const T &t);
    
    private:
    ~BinTree();
    void checkAndRebalance(Node<T> *n);
    void rotateDir(Node<T> *n, Node<T> *p, Dir dir); // rotate n to p position
    Color getColor(Node<T> *n);


};

#include "bintree.cpp"


#endif