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
#include "problem/ingredient.h"

struct School;
struct Kitchen;
struct Supplier;
struct Vehicle;
struct Ingredient;


struct ProblemInstance {
  //truke data
  std::vector<School> schools;
  std::vector<Kitchen> kitchens;
  std::vector<Supplier> suppliers;
  std::vector<Vehicle> first_echelon_vehicles;
  std::vector<Vehicle> second_echelon_vehicles;

  //truke but used
  std::vector<Ingredient> ingredients;
  std::vector<std::vector<double>> first_echelon_matrix;
  std::vector<std::vector<double>> second_echelon_matrix;

  //functional data
  std::vector<int> supplier_indexes;
  std::vector<int> kitchen_e1_indexes;
  std::vector<int> kitchen_e2_indexes;
  std::vector<int> school_indexes;

  std::vector<std::pair<double, double>> first_echelon_coords;
  std::vector<std::pair<double, double>> second_echelon_coords;

  std::vector<int> school_demands;
  std::vector<int> school_lunch_times;
  std::vector<double> school_service_times;

  std::vector<int> kitchen_capacities;
  std::vector<int> kitchen_fixed_costs;
  std::vector<int> kitchen_n_vehicles;

  std::vector<int> supplier_ingredient_type_indexes;

  ProblemInstance(
    std::vector<School>& schools, 
    std::vector<Kitchen>& kitchens, 
    std::vector<Supplier>& suppliers,
    std::vector<Ingredient>& ingredients,
    std::optional<std::vector<std::vector<double>>> first_echelon_matrix,
    std::optional<std::vector<std::vector<double>>> second_echelon_matrix,
    std::vector<Vehicle>& first_echelon_vehicles,
    std::vector<Vehicle>& second_echelon_vehicles
  );
};

ProblemInstance readProblemInstance(std::string instancePath);