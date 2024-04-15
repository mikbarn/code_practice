#ifndef MB_RB_BINTREE_H
#define MB_RB_BINTREE_H
#include <string>
#include <list>
#include <iostream>
#include <sstream>
#include <map>
#include <cmath>
#include <vector>

using namespace std;

enum Color {RED, BLACK};
enum Dir {LEFT, RIGHT};


template <typename T> struct Node {
    const T &t;
    Node *left = NULL, *right = NULL, *parent = NULL;
    Node(const T &r):t(r) {
        col = RED;
    }

    Color col;
    static Color getColor(Node *n) {
        if (n == NULL) {
            return BLACK;
        }
        return n->col;
    }   

    static string val(Node *n) {
        if (n == NULL) {
            return "NULL";
        }
        stringstream ss;
        ss << n->t;
        return ss.str();
    }

    string str() {
        stringstream ss;
        ss << "Node '" << t << "' (" << (col == RED? "RED": "BLACK") << ") @[" << val(left) << ", " << val(right) << ", " << val(parent) << "]";
        return ss.str();
    }
    
};

template <typename T> struct Side {
    Dir side;
    Side(Node<T> *n, Node<T> *p) {
        if (n == p->left) {
            side = LEFT;
            return;
        }
        side = RIGHT;
    }
    Node<T>* other(Node<T> *n) {
        if (side == LEFT) {
            return n->right;
        }
        return  n->left;
    }
    static Dir oppose(Dir d) {
        return d == LEFT? RIGHT : LEFT;
    }

    static string val(Dir d) {
        return d == RIGHT? "RIGHT" : "LEFT";
    }
};


template <typename T> class BinTree {
    public:
        Node<T> *root;
        BinTree();

        std::list<T> Inorder();
        void insert(const T &t);
        void remove(const T &t);
        bool exists(const T &t);
        void printTree();
        ~BinTree();
    private:
        void checkAndRebalance(Node<T> *n);
        void rotateDir(Node<T> *n, Node<T> *p, Dir dir); // rotate n to p position
        void deleteNodeRecursive(Node<T> *n);
        void gridNodes(map<int, vector<T>> &m, Node<T> *n, int path, int depth, int &maxdepth);
        string space(int count);
};

template <typename T> string BinTree<T>::space(int count) {
    stringstream ss;
    for (int i = 0; i < count; i++) {
        ss << " ";
    }
    return ss.str();
}

template <typename T> void BinTree<T>::printTree() {
    map<int, vector<T>> m = map<int, vector<T>>();
    int path = 0;
    int maxdepth = 0;
    gridNodes(m, root, path, 0, maxdepth);
    vector<vector<int>> gaps = vector<vector<int>>(maxdepth+1);
    gaps[0][0] = 0;
    gaps[0][1] = 1;
    for (int i = 1; i <= maxdepth; i++) {
        int prev = gaps[i-1][1];
        gaps[i] = {prev, 2*prev+1};
    }
    for (int i = 0; i <= maxdepth ; i++) {
            vector<T> v = m[i];
            vector<int> spaces = gaps[maxdepth - i];
            cout << space(spaces[0]) << v[0];
            for (int j = 1; j < v.size(); j++) {
                cout << space(spaces[1]);
                if (v[j] != NULL) {
                    cout << v[j];
                } else {
                    cout << "X";
                }
            }
            cout << endl;
    }
}

template <typename T> void BinTree<T>::gridNodes(map<int, vector<T>> &m, Node<T> *n, int path, int depth,  int &maxdepth) {
    maxdepth = max(depth, maxdepth);
    if (n->left != NULL) {
        int p = path << 1;
        gridNodes(m, n->left, p, depth+1, maxdepth);
    }
    if (n->right != NULL) {
        int p = path << 1;
        gridNodes(m, n->right, p + 1, depth+1, maxdepth);
    }
    if (m.count(depth) == 0) {
        m[depth] = vector<T>(pow(2,depth));
    }
    vector<T> v = m[depth];
    v[path] = n->t;

}

template <typename T> BinTree<T>::BinTree() {
}

template <typename T> void BinTree<T>::deleteNodeRecursive(Node<T> *n) {
    if (n == NULL) {
        return;
    }
    if (n->left != NULL) {
        deleteNodeRecursive(n->left);
    }
    if (n->right != NULL) {
        deleteNodeRecursive(n->right);
    }
    cout << "Deleting node " << n->t << endl;
    delete n;
}

template <typename T> BinTree<T>::~BinTree() {
    deleteNodeRecursive(root);
}


template <typename T> void BinTree<T>::insert(const T &t) {
    printTree();
    Node<T> *n = new Node<T>(t);
    cout << "Inserting " << n->str() << endl;
    if(root == NULL) { 
        root = n;
        root->col = BLACK;
        cout << "Set root: " << root->str() << endl;        
        return;
    }
   
    Node<T> *x = root;
    int depth = 0;
    while (true) {
        if (n->t == x->t) {
            delete n;
            cout << "[WARNING] Duplicate insert ignored." << endl;
            return;
        }
        if (n->t < x->t) {
            if (x->left == NULL) {
                x->left = n;
                n->parent = x;
                break;
            }
            x = x->left;
        } else {
            if (x->right == NULL) {
                x->right = n;
                n->parent = x;
                break;
            }
            x = x->right;
        }
        depth++;
    }
    cout << "Inserted " << n->str() << " at depth " << depth << endl;
    checkAndRebalance(n);
}

template <typename T> void BinTree<T>::checkAndRebalance(Node<T> *n) {
    cout << "Balancing " << n->str() << endl;
    if (n == root) {
        return;
    }
    if (Node<T>::getColor(n->parent) == BLACK) {
        cout << "RED has BLACK parent so nothing to do! " << endl;
        return;
    }
    Node<T> *p, *u, *g;
    
    while (true) {
        cout << "Enter loop " << endl;
        if (n == root && n->col == RED) {
            cout << "RED root/parent swap to BLACK! " << endl;
            n->col = BLACK;
            break;
        }
        p = n->parent;
        Side<T> s = Side(n, p);
        u = s.other(p);
        Color u_color = Node<T>::getColor(u);
        g = p->parent;
        cout << "Rebalance " << Node<T>::val(n) << " on " << Side<T>::val(s.side) << " side of parent " << Node<T>::val(p) << endl;
        if (u_color == RED) {
            cout << "RED parent w/ RED uncle so recolor! " << endl;
            if (u != NULL) {
                u->col = BLACK;
            }
            p->col = BLACK;
            g->col = RED;
            n = g;
            p = u = NULL;
            continue;
        } else {
            Side ps = Side(p, g);
            Dir opposite = Side<T>::oppose(s.side);
            if (s.side == opposite) {
                cout << "RED parent w/ BLACK uncle inner case, rotate to outer! " << endl;
                rotateDir(n, p, opposite);
                n = p;
                continue;
            } else {
                cout << "RED parent w/ BLACK uncle outer case, rotate p to g! " << endl;
                rotateDir(p, g, opposite);
                p->col = BLACK;
                g->col = RED;
                break;
            }
        }
    }
    cout << "DONE??" << endl;

}

template <typename T> void BinTree<T>::rotateDir(Node<T> *n, Node<T> *p, Dir dir) {
    if (dir == LEFT) {
        Node<T> *g = p->parent;
        Node<T> *n_l_save = n->left;
        n->parent = g;
        n->left = p;
        if (g != NULL) {
            g->left = n;
        }
        
        p->parent = n;
        p->right = n_l_save;
        return;
    }
    Node<T> *g = p->parent;
    Node<T> *n_r_save = n->right;
    n->parent = g;
    n->right = p;
    if (g != NULL) {
        g->right = n;
    }
    p->parent = n;
    p->left = n_r_save;    

}


#endif