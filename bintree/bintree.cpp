#include "bintree.h"
#include <iostream>

using namespace std;

template <typename T> BinTree<T>::BinTree(const T& root) {
    Node<T> t = new Node<T>(root);
    root = t;
}


template <typename T> void BinTree<T>::insert(const T &t) {
    Node<T> *n = new Node<T>(t);
    if(root == NULL) { 
        root = n;
        return;
    }
   
    Node<T> *x = root;
    Node<T> *p = NULL;
    int depth = 0;
    while (true) {
        if (n->t == x->t) {
            delete n;
            cout << "[WARNING] Duplicate insert ignored." << endl;
            return;
        }
        p = x;
        if (n->t < x->t) {
            if (x->left == NULL) {
                x->left = n;
                break;
            }
            x = x.left;
        }
        else {
            if (x->right == NULL) {
                x->right = n;
                break;
            }
            x = x->right;
        }
        x->parent = p;
        depth++;
    }
    cout << "Inserted " << *n << " at depth " << depth << endl;
    checkAndRebalance(n);
}

template <typename T> Color BinTree<T>::getColor(Node<T> *n) {
    if (n == NULL) {
        return BLACK;
    }
    return n.col;
}


template <typename T> void BinTree<T>::checkAndRebalance(Node<T> *n) {
    if (n == root) {
        return;
    }
    Node<T> *p = n->parent;
    Color p_color = getColor(p);
    if (p_color == BLACK) {
        cout << "RED has BLACK parent so nothing to do! " << endl;
        return;
    }
    while (true) {
        p = n->parent;
        Side s = Side(n, p);
        Node<T> *u = s.other(p);
        Color u_color = getColor(u);
        if (p == root && p->col == RED) {
            p->col = BLACK;
            break;
        }
        Node<T> *g = p->parent;
        if (u_color == RED) {
            u->col = BLACK;
            p->col = BLACK;
            g->col = RED;
            n = g;
            p = u = NULL;
            continue;
        } else {
            Side ps = Side(p, g);
            Dir opposite = Side<T>::oppose(s.side);
            if (s.side == opposite) {
                rotateDir(n, p, opposite);
                n = p;
                continue;
            } else {
                rotateDir(p, g, opposite);
            }
        }
    }

}

template <typename T> void BinTree<T>::rotateDir(Node<T> *n, Node<T> *p, Dir dir) {
    if (dir == LEFT) {
        Node<T> *g = p.parent;
        Node<T> *n_l_save = n.left;
        n.parent = g;
        n.left = p;
        g.left = n;
        p.parent = n;
        p.right = n_l_save;
        return;
    }
    Node<T> *g = p.parent;
    Node<T> *n_r_save = n.right;
    n.parent = g;
    n.right = p;
    g.right = n;
    p.parent = n;
    p.left = n_r_save;    

}

// template <typename T> std::list<T> BinTree<T>::Inorder() {
//     list<T> rval = list<T>();
//     return rval;
// }
