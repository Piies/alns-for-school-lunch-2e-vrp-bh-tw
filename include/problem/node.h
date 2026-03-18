#pragma once

enum class NodeType {
  school,
  kitchen,
  supplier,
};

struct Node{
  int idx;
  double x, y;
  NodeType nodeType;
  Node(int idx, double x, double y, NodeType nodeType) : idx(idx), x(x), y(y), nodeType(nodeType) {};
};
