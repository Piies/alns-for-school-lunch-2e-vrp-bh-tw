#pragma once

struct Vehicle {
  int idx;
  int capacity;
  double var_cost;
  double fix_cost;

  Vehicle(int idx, int capacity, double var_cost, double fix_cost) : 
  idx(idx), capacity(capacity), var_cost(var_cost), fix_cost(fix_cost) {};
};
