/*
  Copyright (C) 2021 Marvin Liu

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "index.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <ios>

int horder = 10;

// keys_size should be 2*horder - 1
Node::Node(int keys_size) {
  keys = new int[keys_size];
  size = 0;
  children = new Node*[keys_size + 1];
  prev = nullptr;
  next = nullptr;
  is_leaf = false;
}

int cnt = 0;

Node::~Node() {
  if (is_leaf)
    delete[] vals;
  for (int i = 0; i <= size; i++)
    delete children[i];
  delete[] children;
  delete[] keys;
}

Node* BPTree::create_node() {
  int keys_size = 2 * horder - 1;
  Node *nd = new Node(keys_size);
  for (int i = 0; i < keys_size; i++)
    nd->keys[i] = 0;
  for (int i = 0; i < keys_size + 1; i++)
    nd->children[i] = nullptr;
  return nd;
}

Node* BPTree::create_leaf() {
  Node *nd = create_node();
  nd->is_leaf = true;
  nd->vals = new int[horder << 1];
  for (int i = 0; i < 2*horder; i++) nd->vals[i] = 0;
  return nd;
}

Node *BPTree::create_tree() {
  Node *node = create_leaf();
  //node->next = node;
  //node->prev = node;
  return node;
}

int BPTree::split_child(Node *parent, int pos, Node *child) {
  Node *new_child = child->is_leaf ? create_leaf() : create_node();
  new_child->size = horder - 1;
  for (int i = 0; i < horder - 1; i++) {
    new_child->keys[i] = child->keys[i+horder];
  }

  if (!new_child->is_leaf) {
    for (int i = 0; i < horder; i++) {
      new_child->children[i] = child->children[i+horder];
    }
  } else {
    for (int i = 0; i < horder; i++) {
      new_child->vals[i] = child->vals[i+horder];
    }
  }

  child->size = horder - 1;
  if (child->is_leaf) child->size++; // if is leaf, keep the middle element, put
                                     //it in the left;

  for (int i = parent->size; i > pos; i--)
    parent->children[i+1] = parent->children[i];
  parent->children[pos+1] = new_child;

  for (int i = parent->size - 1; i >= pos; i--)
    parent->keys[i+1] = parent->keys[i];
  parent->keys[pos] = child->keys[horder-1];

  parent->size++;

  if (child->is_leaf) {
    new_child->next = child->next;
    //child->next->prev = new_child;
    //new_child->prev = child;
    child->next = new_child;
  }
  return 0;
}

void BPTree::insert_notfull(Node *node, int key, int val) {
  if (node->is_leaf) {
    int pos = node->size;
    while (pos >= 1 && key < node->keys[pos-1]) {
      node->keys[pos] = node->keys[pos-1];
      node->vals[pos] = node->vals[pos-1];
      pos--;
    }
    node->keys[pos] = key;
    node->vals[pos] = val;
    node->size++;
  } else {
    int pos = node->size;
    while (pos > 0 && key < node->keys[pos-1]) {
      pos--;
    }

    if ((2*horder-1) == node->children[pos]->size) {
      split_child(node, pos, node->children[pos]);
      if (key > node->keys[pos]) pos++;
    }
    insert_notfull(node->children[pos], key, val);
  }
}

Node* BPTree::insert(Node *rt, int key, int val) {
  if ((2*horder-1) == rt->size) {
    Node *node = create_node();
    node->children[0] = rt;
    split_child(node, 0, rt);
    insert_notfull(node, key, val);
    return node;
  } else {
    insert_notfull(rt, key, val);
    return rt;
  }
}

void BPTree::insert(int key, int val) {
  root = insert(root, key, val);
}

BPTree::BPTree(int horder) {
  this->horder = horder;
  root = create_tree();
}

BPTree::~BPTree() {
  delete root;
}

int BPTree::search(int key) {
  Node *y = this->root;
  while (!y->is_leaf) {
    int pos = std::lower_bound(y->keys, y->keys+y->size, key) - y->keys;
    y = y->children[pos];
  }
  int pos = std::lower_bound(y->keys, y->keys+y->size, key) - y->keys;
  if (y->keys[pos] != key) return -1;
  else return y->vals[pos];
}

int BPTree::rmq(int left, int right) {
  Node *y = this->root;
  while (!y->is_leaf) {
    int pos = std::lower_bound(y->keys, y->keys+y->size, left) - y->keys;
    y = y->children[pos];
  }
  int pos = std::lower_bound(y->keys, y->keys+y->size, left) - y->keys;
  if (y->keys[pos] > right) return -1;
  int m = y->vals[pos];
  //int cur_k = y->keys[pos];
  while (y->keys[pos] <= right) {
    if (y->vals[pos] > m) m = y->vals[pos];
    pos++;
    if (pos >= y->size) {
      y = y->next;
      if (y == nullptr) break;
      pos = 0;
    }
    //if (cur_k >= y->keys[pos]) break;
    //cur_k = y->keys[pos];
  }
  return m;
}

Index::Index(int num_rows, vector<int> &keys, vector<int> &vals) {
  int order = std::max(16, int(std::pow(double(num_rows), 0.333) + 0.5));
  this->bptree = new BPTree(order>>1);
  vector<int>::iterator k = keys.begin();
  vector<int>::iterator v = vals.begin();
  for (int i = 0; i < num_rows; i++) {
    this->bptree->insert(*k, *v);
    k++, v++;
  }
}

void Index::key_query(vector<int> &querys) {
  std::fstream outfile;
  outfile.open("key_query_out.txt", std::ios_base::out);
  if (outfile.is_open()) {
    for (int qs: querys) {
      outfile << this->bptree->search(qs) << '\n';
    }
    outfile.flush();
  }
  outfile.close();
}

void Index::range_query(vector<pair<int, int>> &querys) {
  std::fstream outfile;
  outfile.open("range_query_out.txt", std::ios_base::out);
  if (outfile.is_open()) {
    for (pair<int, int> qs: querys) {
      outfile << this->bptree->rmq(qs.first, qs.second) << '\n';
    }
    outfile.flush();
  }
  outfile.close();
}

void Index::clear_index() { delete this->bptree;}
