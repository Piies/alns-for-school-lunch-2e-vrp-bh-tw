//data struct for ts vrp instance
#pragma once

#include <optional>
#include <string>
#include <vector>

#include "problem/school.h"
#include "problem/kitchen.h"
#include "problem/supplier.h"
#include "problem/node.h"
#include "problem/vehicle.h"

struct School;
struct Kitchen;
struct Supplier;
struct Vehicle;


struct ProblemInstance {
  int numNodes;
  int numSchools;
  int numKitchens;
  int numSuppliers;
  
  std::vector<std::vector<double>> coords;
  std::vector<int> schoolIdxs;
  std::vector<int> kitchenIdxs;
  std::vector<int> supplierIdxs;
  std::vector<int> demands;
  std::vector<NodeType> nodeTypes;
  std::vector<int> lockerCapacities;
  std::vector<int> lockerIdxs;
  std::vector<int> vehicleCapacities;
  std::vector<double> vehicleCosts;
  std::vector<int> vehicleIdxs;

  std::vector<std::vector<double>> distanceMatrix;
  ProblemInstance(std::vector<School>& schools, std::vector<Kitchen>& kitchens, std::vector<Supplier>& suppliers,
    std::optional<std::vector<std::vector<double>>> first_echelon_matrix,
    std::optional<std::vector<std::vector<double>>> second_echelon_matrix
  );
};

ProblemInstance readProblemInstance(std::string instancePath);