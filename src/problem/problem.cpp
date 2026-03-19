
#include <cmath>
#include <fstream>
#include <numeric>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "problem/problem.h"
#include "problem/node.h"
#include "problem/school.h"
#include "problem/kitchen.h"
#include "problem/supplier.h"
#include "problem/vehicle.h"
#include "problem/ingredient.h"

static int findIngredientTypeIndex(std::vector<Ingredient>& ingredients, std::string ingredientType) {
  for (size_t i = 0; i < ingredients.size(); i++)
  {
    if (ingredients[i].name == ingredientType) {
      return i;
    }
  }
  throw std::runtime_error("Ingredient type " + ingredientType + " not found in ingredients list");
}

ProblemInstance readProblemInstance(std::string instancePath) {
  std::ifstream file(instancePath);
  std::string errMessage = "Error opening, file not found at " + instancePath;
  if (!file) throw std::runtime_error(errMessage);

  nlohmann::json instance = nlohmann::json::parse(file);

  std::string line;
  int index = 0;

  //assume that the file (json) is written in this order
  std::vector<School> schools;
  std::vector<Kitchen> kitchens;
  std::vector<Supplier> suppliers;
  std::vector<std::vector<double>> first_echelon_matrix;
  std::vector<std::vector<double>> second_echelon_matrix;
  std::vector<Vehicle> first_echelon_vehicles;
  std::vector<Vehicle> second_echelon_vehicles;
  std::vector<Ingredient> ingredients;

  nlohmann::json schoolsJson = instance["schools"];
  for (size_t i = 0; i < schoolsJson.size(); i++)
  {
    School school(
      schoolsJson[i]["school_id"],
      schoolsJson[i]["latitude"],
      schoolsJson[i]["longitude"],
      schoolsJson[i]["school_name"],
      schoolsJson[i]["demand"],
      schoolsJson[i]["lunch_time"],
      schoolsJson[i]["service_time"]
    );
    schools.push_back(school);
  }
  
  nlohmann::json kitchensJson = instance["kitchens"];
  for (size_t i = 0; i < kitchensJson.size(); i++)
  {
    Kitchen kitchen(
      kitchensJson[i]["kitchen_id"],
      kitchensJson[i]["latitude"],
      kitchensJson[i]["longitude"],
      kitchensJson[i]["capacity"],
      kitchensJson[i]["fixed_cost"],
      kitchensJson[i]["maximum_vehicles"],
      kitchensJson[i]["kitchen_name"]
    );
    kitchens.push_back(kitchen);
  }

  nlohmann::json ingredientsJson = instance["ingredients"];
  for (size_t i = 0; i < ingredientsJson.size(); i++)
  {
    Ingredient ingredient(
      ingredientsJson[i]["ingredient_name"],
      ingredientsJson[i]["tray_to_kg_ratio"]
    );
    ingredients.push_back(ingredient);
  }

  nlohmann::json suppliersJson = instance["suppliers"];
  for (size_t i = 0; i < suppliersJson.size(); i++)
  {
    Supplier supplier(
      suppliersJson[i]["supplier_id"],
      suppliersJson[i]["latitude"],
      suppliersJson[i]["longitude"],
      suppliersJson[i]["type"],
      suppliersJson[i]["ingredient"],
      suppliersJson[i]["supplier_name"]
    );
    suppliers.push_back(supplier);
  }
  
  nlohmann::json firstEchelonMatrixJson = instance["first_echelon_matrix"];
  for (size_t i = 0; i < firstEchelonMatrixJson.size(); i++)
  {
    std::vector<double> row;
    for (size_t j = 0; j < firstEchelonMatrixJson[i].size(); j++)
    {
      row.push_back(firstEchelonMatrixJson[i][j]);
    }
    first_echelon_matrix.push_back(row);
  }

  nlohmann::json secondEchelonMatrixJson = instance["second_echelon_matrix"];
  for (size_t i = 0; i < secondEchelonMatrixJson.size(); i++)
  {
    std::vector<double> row;
    for (size_t j = 0; j < secondEchelonMatrixJson[i].size(); j++)
    {
      row.push_back(secondEchelonMatrixJson[i][j]);
    }
    second_echelon_matrix.push_back(row);
  }

  nlohmann::json firstEchelonVehiclesJson = instance["first_echelon_vehicles"];
  for (size_t i = 0; i < firstEchelonVehiclesJson.size(); i++)
  {
    Vehicle vehicle(
      firstEchelonVehiclesJson[i]["vehicle_id"],
      firstEchelonVehiclesJson[i]["capacity"],
      firstEchelonVehiclesJson[i]["variable_cost"],
      firstEchelonVehiclesJson[i]["fixed_cost"]
    );
    first_echelon_vehicles.push_back(vehicle);
  }

  nlohmann::json secondEchelonVehiclesJson = instance["second_echelon_vehicles"];
  for (size_t i = 0; i < secondEchelonVehiclesJson.size(); i++)
  {
    Vehicle vehicle(
      secondEchelonVehiclesJson[i]["vehicle_id"],
      secondEchelonVehiclesJson[i]["capacity"],
      secondEchelonVehiclesJson[i]["variable_cost"],
      secondEchelonVehiclesJson[i]["fixed_cost"]
    );
    second_echelon_vehicles.push_back(vehicle);
  }

  ProblemInstance problem(
    schools, 
    kitchens, 
    suppliers, 
    ingredients,
    first_echelon_matrix,
    second_echelon_matrix,
    first_echelon_vehicles,
    second_echelon_vehicles
  );
  return problem;
}

ProblemInstance::ProblemInstance(
  std::vector<School>& schools, 
  std::vector<Kitchen>& kitchens, 
  std::vector<Supplier>& suppliers, 
  std::vector<Ingredient>& ingredients,
  std::optional<std::vector<std::vector<double>>> first_echelon_matrix,
  std::optional<std::vector<std::vector<double>>> second_echelon_matrix,
  std::vector<Vehicle>& first_echelon_vehicles,
  std::vector<Vehicle>& second_echelon_vehicles
){
  this->schools = schools;
  this->kitchens = kitchens;
  this->suppliers = suppliers;
  this->ingredients = ingredients;

  if (first_echelon_matrix.has_value()) {
    this->first_echelon_matrix = first_echelon_matrix.value();
  }
  else {
    this->first_echelon_matrix = std::vector<std::vector<double>>{};
  }

  if (second_echelon_matrix.has_value()) {
    this->second_echelon_matrix = second_echelon_matrix.value();
  }
  else {
    this->second_echelon_matrix = std::vector<std::vector<double>>{};
  }
  this->first_echelon_vehicles = first_echelon_vehicles;
  this->second_echelon_vehicles = second_echelon_vehicles;

  supplier_indexes.reserve(suppliers.size());
  for (size_t i = 0; i < suppliers.size(); i++){
    supplier_indexes.push_back(i);
  }
  kitchen_e1_indexes.reserve(kitchens.size());
  for (size_t i = supplier_indexes.size(); i < supplier_indexes.size() + kitchens.size(); i++)
  {
    kitchen_e1_indexes.push_back(i);
  }

  kitchen_e2_indexes.reserve(kitchens.size());
  for (size_t i = 0; i < kitchens.size(); i++){
    kitchen_e2_indexes.push_back(i);
  }
  school_indexes.reserve(schools.size());
  for (size_t i = kitchen_e2_indexes.size(); i < kitchen_e2_indexes.size() + schools.size(); i++)
  {
    school_indexes.push_back(i);
  }

  //sekalian validate
  if ((first_echelon_matrix.value().size() > 0) && (first_echelon_matrix.value().size() != (supplier_indexes.size() + kitchen_e1_indexes.size()))) {
    throw std::runtime_error("First echelon matrix row size does not match number of suppliers");
  }
  if ((second_echelon_matrix.value().size() > 0) && (second_echelon_matrix.value().size() != (kitchen_e2_indexes.size() + school_indexes.size()))) {
    throw std::runtime_error("First echelon matrix row size does not match number of suppliers");
  }
  
  //coords list for uhhh, just in case?
  for (size_t i = 0; i < suppliers.size(); i++)
  {
    first_echelon_coords.push_back({suppliers[i].x, suppliers[i].y});
    supplier_ingredient_type_indexes.push_back(findIngredientTypeIndex(ingredients, suppliers[i].ingredient_type));
  }
  for (size_t i = 0; i < kitchens.size(); i++)
  {
    first_echelon_coords.push_back({kitchens[i].x, kitchens[i].y});
    second_echelon_coords.push_back({kitchens[i].x, kitchens[i].y});
    kitchen_capacities.push_back(kitchens[i].capacity);
    kitchen_fixed_costs.push_back(kitchens[i].fix_cost);
    kitchen_n_vehicles.push_back(kitchens[i].n_vehicles);
  }
  for (size_t i = 0; i < schools.size(); i++)
  {
    second_echelon_coords.push_back({schools[i].x, schools[i].y});
    school_demands.push_back(schools[i].demand);
    school_lunch_times.push_back(schools[i].lunch_time);
    school_service_times.push_back(schools[i].service_time);
  }
}
