#ifndef MB_RB_BINTREE_H
#define MB_RB_BINTREE_H
#include <string>
#include <list>
#include <iostream>
#include <sstream>
#include <map>
#include <cmath>
#include <memory>
#include <array>
#include <vector>

using namespace std;

enum Color {RED, BLACK};
enum Dir {LEFT, RIGHT};


template <typename T> struct Node {
    const T t;
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
        ss << "Node '" << t << "' (" << (col == RED? "RED": "BLACK") << ") @[" << val(left) << ", " << val(parent) << ", " << val(right) << "]";
        return ss.str();
    }

    string col_str() {
        return col == RED? "R" : "B";
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
    typedef shared_ptr<string[]> dyn;
    public:
        Node<T> *root = NULL;
        BinTree();

        std::list<T> Inorder();
        void insert(const T t);
        void remove(const T t);
        bool exists(const T t);
        void printTree();
        ~BinTree();
    private:
        int print_size = 1;
        void checkAndRebalance(Node<T> *n);
        void rotateDir(Node<T> *n, Node<T> *p, Dir dir); // rotate n to p position
        void deleteNodeRecursive(Node<T> *n);
        void gridNodes(map<int, dyn> &m, Node<T> *n, int path, int depth, int &maxdepth);
        string pad(int count, const string &val);
};

template <typename T> string BinTree<T>::pad(int count, const string &val) {
    stringstream ss;
    int sz = val.size();
    ss << val;
    for (int i = 0; i < count-sz; i++) {
        ss << " ";
    }
    return ss.str();
}

template <typename T> void BinTree<T>::printTree() {
    print_size = 1;
    if (root == NULL) {
        cout << "No tree" << endl;
        return;
    }
    map<int, dyn> m = map<int, dyn>();
    int maxdepth = 0;
    gridNodes(m, root, 0, 0, maxdepth);
    cout << "Tree depth: " << maxdepth << " Print size: " << print_size << endl;

    maxdepth += 1;
    auto gaps = vector<array<int, 2>>(maxdepth);
    gaps[0][0] = 0;
    gaps[0][1] = 1;
    for (int i = 1; i < maxdepth; i++) {
        int prev = gaps[i-1][1];
        gaps[i][0] = prev + 1;
        gaps[i][1] = 2*gaps[i][0]+1;
    }

    for (const auto& [key, value]: m) {
        cout << "k: " << key << " v: [";
        for(int i = 0; i < pow(2,key); i++) {
            cout << value[i] << (i < pow(2,key) - 1? ",": "");
        }
        cout << "]" << endl;

    }

    for (int i = 0; i < maxdepth; i++) {
            dyn arr = m[i];
            int size = pow(2, i);
            string small = pad(gaps[maxdepth - 1 - i][0], "");
            string big = pad(gaps[maxdepth - 1 - i][1], "");
            // cout << "small '" << small << "' big '" << big << "' " << endl;
            //cout << small << arr[0];
            for (int j = 0; j < size; j++) {
                cout << (j==0? small: big);
                if (arr[j] == "") {
                    cout << pad(print_size, "x");
                } else {
                    cout << pad(print_size, arr[j]);
                }
            }
            cout << endl;
    }
    // if (maxdepth > 1) {
    //     exit(0);
    // }
}

template <typename T> void BinTree<T>::gridNodes(map<int, dyn> &m, Node<T> *n, int path, int depth,  int &maxdepth) {
    // cout << "Grid Nodes " << path << " " << depth << " "  << maxdepth << " " <<  n->str() << endl;
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
        int size = pow(2,depth);
        dyn a = dyn(new string[size]);
        for (int i = 0; i < size; i++) {
            a[i] = "";
        }
        m[depth] = a;
    }
    dyn arr = m[depth];
    stringstream ss;
    ss << n->t << n->col_str();
    arr[path] = ss.str();
    int len = arr[path].size();
    print_size = max(print_size, len);

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


template <typename T> void BinTree<T>::insert(const T t) {
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
    printTree();
    checkAndRebalance(n);
    cout << "Balanced tree ???" << endl;
    printTree();
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
        if (n == root ) { //&& n->col == RED
            // cout << "RED root swap to BLACK! " << endl;
            // n->col = BLACK;
            break;
        }
        p = n->parent;
        Side<T> s = Side(n, p);

        if(p == root) {
            cout << "Parent is RED root. Swap to black!" << endl;
            // Node<T> *sibling = s.other(p);
            // if (sibling != NULL) {
            //     sibling->col = BLACK;
            // }
            p->col = BLACK;
            break;
        }

        g = p->parent;
        u = s.other(g);
        Color u_color = Node<T>::getColor(u);
        cout << "Rebalance " << Node<T>::val(n) << " on " << Side<T>::val(s.side) << " side of parent " << Node<T>::val(p) << endl;
        if (u_color == RED) {
            cout << "RED parent w/ RED uncle so recolor! " << endl;
            u->col = BLACK;
            p->col = BLACK;
            g->col = RED;
            n = g;
            p = u = NULL;
            printTree();
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

}

template <typename T> void BinTree<T>::rotateDir(Node<T> *n, Node<T> *p, Dir dir) {
    cout << "Rotating " << Side<T>::val(dir) << "!" << endl;
    cout << n->str() << " --- " << p->str() << endl;
    Node<T> *g = p->parent;
    bool left = dir == LEFT;
    if (left) {
        Node<T> *n_l_save = n->left;
        n->left = p;
        p->right = n_l_save;
    } else {
        Node<T> *n_r_save = n->right;
        n->right = p;
        p->left = n_r_save;
    }

    n->parent = g;
    p->parent = n;
    if (g != NULL) {
        if (left) {
            g->right = n;
        } else {
            g->left = n;
        }
    }else {
        root = n;
    }
    cout << n->str() << " --- " << p->str() << endl;
}


#endif