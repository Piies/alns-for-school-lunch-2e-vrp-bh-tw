#pragma once

#include <vector>
#include <string>

#include "problem/node.h"


struct School : Node {
  std::string name;
  int demand;
  int lunch_time;
  School(int idx, double x, double y, std::string name, int demand, int lunch_time) :
    Node(idx, x, y, NodeType::school), name(name), demand(demand), lunch_time(lunch_time) {
  };
};

