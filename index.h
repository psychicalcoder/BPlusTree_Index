#ifndef BPINDEX
#define BPINDEX

#include <vector>
using std::vector;
using std::pair;

struct Node {
  int *keys;
  int *vals;
  int size;
  bool is_leaf;
  Node **children;
  Node *prev;
  Node *next;
  Node(int keys_size);
  ~Node();
};

class BPTree {
 private:
  int horder;
  Node *root;
  Node *create_tree();
  Node *create_node();
  Node *create_leaf();
  int split_child(Node *parent, int pos, Node *child);
  void insert_notfull(Node *node, int key, int val);
  Node *insert(Node *rt, int key, int val);
 public:
  BPTree(int horder);
  ~BPTree();
  void insert(int key, int val);
  int rmq(int left, int right);
  int search(int key);
};

class Index {
private:
  BPTree *bptree;
public:
  Index(int num_rows, vector<int> &keys, vector<int> &vals);
  void key_query(vector<int> &querys);
  void range_query(vector<pair<int, int> > &querys);
  void clear_index();
};
#endif
