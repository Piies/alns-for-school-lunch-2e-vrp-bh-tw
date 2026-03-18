#pragma once

#include <vector>
#include <string>

#include "problem/node.h"
#include "problem/vehicle.h"


struct Supplier : Node{
  int fix_cost;
  std::string name;

  Supplier(int idx, double x, double y, NodeType nodeType, int fix_cost, std::string name, std::vector<Vehicle> availableVehicles) : 
    Node(idx, x, y, NodeType::supplier), fix_cost(fix_cost), name(name) {};
};
