#include "heuristic/solution.h"

#include <algorithm>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <execution>
#include <vector>

#include "problem/problem.h"
#include "problem/node.h"

Solution::~Solution() {
    fmt::print("~Solution at {}\n", (void*)this);
}


Solution::Solution(const ProblemInstance& problem){
  Vehicle default_vehicle(problem.first_echelon_vehicles[0].idx, problem.first_echelon_vehicles[0].capacity, problem.first_echelon_vehicles[0].var_cost, problem.first_echelon_vehicles[0].fix_cost);
  VehicleRoute empty_route{-2, 0, 1, {-2,-2}, 2, 0, 0, default_vehicle}; //-1 for invalid ig, so -2 for uninitialized
  first_echelon_routes.assign(problem.first_echelon_vehicles.size(), empty_route);
  second_echelon_routes.assign(problem.second_echelon_vehicles.size(), empty_route);
  //gonna overwrite these in initial solution

  first_echelon_vehicle_loads.assign(problem.first_echelon_vehicles.size(), 0);
  second_echelon_vehicle_loads.assign(problem.second_echelon_vehicles.size(), 0);

  second_echelon_linehaul_durations.assign(problem.second_echelon_vehicles.size(), 0);
  first_echelon_durations.assign(problem.first_echelon_vehicles.size(), 0);

  second_echelon_departure_times.assign(problem.second_echelon_vehicles.size(), 0);
  kitchens_used_flags.assign(problem.kitchen_e2_indexes.size(), false);
  suppliers_used_flags.assign(problem.supplier_indexes.size(), false);

  first_echelon_costs.assign(problem.first_echelon_vehicles.size(), 0);
  second_echelon_costs.assign(problem.second_echelon_vehicles.size(), 0);

  school_kitchen.assign(problem.schools.size(), -1);
  for (int i = 0; i < problem.suppliers.size(); i++){
    supplier_kitchen.push_back({});
  }
  kitchen_demands.assign(problem.kitchens.size(), 0);
  supplier_demands.assign(problem.suppliers.size(), 0);
}

bool Solution::isFeasible(const ProblemInstance& problem) const {
  //validate
  for (size_t i = 0; i < second_echelon_routes.size(); ++i) {
    int duration_counter = second_echelon_routes[i].departure_batch; // Start with departure time
    int route_load = 0;
    int sattelite_idx = second_echelon_routes[i].satellite_idx;
    // Validate School Lunch Times, Linehaul
    for (size_t j = 0; j < second_echelon_routes[i].stops.size(); ++j) {
      if (second_echelon_routes[i].stops[j] == second_echelon_routes[i].linehaul_end) {
        break; // End of linehaul, backhaul starts
      }
      int node_idx = second_echelon_routes[i].stops[j];
      int dist_matrix_idx = node_idx + problem.kitchen_e2_indexes.size(); // Adjust index for distance matrix
      route_load += problem.school_demands[node_idx];
      duration_counter += problem.second_echelon_matrix[sattelite_idx][dist_matrix_idx];
      if (duration_counter > problem.school_lunch_times[node_idx]) {
        fmt::print("Feasibility check failed: Vehicle {} exceeds school lunch time at node {}\n", i, node_idx);
        return false;
      }      
      //meaning lunch times are ok
      //check vehicle capacity?
      if (route_load > problem.second_echelon_vehicles[i].capacity) {
        fmt::print("Feasibility check failed: Vehicle {} exceeds capacity at node {}\n", i, node_idx);
        return false;
      }
      //if vehicle capacity ok, check max travel time?
      if (duration_counter > problem.maximum_2e_lh_route_duration) {
        fmt::print("Feasibility check failed: Vehicle {} exceeds maximum route duration at node {}\n", i, node_idx);
        return false;
      }
      duration_counter += problem.school_service_times[node_idx];
    }
    if (second_echelon_vehicle_loads[i] > problem.second_echelon_vehicles[i].capacity) {
      fmt::print("Feasibility check failed: Vehicle {} exceeds capacity with total load, check route and counter mismatch\n", i);
      return false;
    }
    if (second_echelon_linehaul_durations[i] > problem.maximum_2e_lh_route_duration) {
      fmt::print("Feasibility check failed: Vehicle {} exceeds maximum linehaul duration, check route and counter mismatch\n", i);
      return false;
    }
  }
  // Validate first echelon route durations and capacities
  //check if all kitchens are served by every ingredient
  for (size_t i = 0; i < first_echelon_routes_per_ingredient_type.size(); ++i) {
    std::vector<int> kitchens_served = problem.kitchen_e1_indexes;
    for (size_t j = 0; j < first_echelon_routes_per_ingredient_type[i].size(); ++j) {
      int route_idx = first_echelon_routes_per_ingredient_type[i][j];
      for (size_t k = 1; k < first_echelon_routes[route_idx].stops.size()-1; ++k) 
      { //fuckkk fr copilott, should we check all stops or just linehaul? maybe just linehaul, but for now imma check all stops to be safe
        int node_idx = first_echelon_routes[route_idx].stops[k];
        //find node_idx in kitchens served and delete it
        auto it = std::find(kitchens_served.begin(), kitchens_served.end(), node_idx);
        if (it != kitchens_served.end()) {
          kitchens_served.erase(it);
        }
      }
    }
    if (kitchens_served.size() > 0) {
      fmt::print("Feasibility check failed: Ingredient type {} is not delivered to kitchens {}\n", i, fmt::join(kitchens_served, ","));
      return false;
    }
  }
  for (size_t i = 0; i < first_echelon_routes.size(); ++i) {
    int duration_counter = first_echelon_routes[i].departure_batch; // Start with departure time
    int route_load = 0;
    // Validate route durations and capacities
    for (size_t j = 0; j < first_echelon_routes[i].stops.size(); ++j) {
      int node_idx = first_echelon_routes[i].stops[j];
      route_load += problem.school_demands[node_idx];
      duration_counter += problem.first_echelon_matrix[first_echelon_routes[i].satellite_idx][node_idx];
      if (duration_counter > problem.maximum_1e_route_duration) {
        fmt::print("Feasibility check failed: Vehicle {} exceeds maximum route duration at node {}\n", i, node_idx);
        return false;
      }
      if (route_load > problem.first_echelon_vehicles[i].capacity) {
        fmt::print("Feasibility check failed: Vehicle {} exceeds capacity at node {}\n", i, node_idx);
        return false;
      }
    }
  }
  // Additional feasibility checks can be added here (e.g., time windows, route durations)
  return true;
}