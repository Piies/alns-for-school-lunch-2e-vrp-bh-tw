#pragma once

#include <vector>
#include <string>

#include "problem/node.h"
#include "problem/vehicle.h"


struct Kitchen : Node{
  int capacity;
  int fix_cost, n_vehicles;
  std::string name;

  Kitchen(int idx, double x, double y, int capacity, int fix_cost, std::string name, std::vector<Vehicle> availableVehicles) : 
    Node(idx, x, y, NodeType::kitchen), capacity(capacity), fix_cost(fix_cost), name(name), availableVehicles(availableVehicles) {};
};


