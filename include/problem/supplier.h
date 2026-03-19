#pragma once

#include <vector>
#include <string>

#include "problem/node.h"
#include "problem/ingredient.h"
#include "problem/vehicle.h"


struct Supplier : Node{
  int fix_cost;
  std::string name, type, ingredient_type;

  Supplier(int idx, double x, double y, std::string name, std::string type, std::string ingredient_type) : 
    Node(idx, x, y, NodeType::supplier),  name(name), type(type), ingredient_type(ingredient_type) {};
};
