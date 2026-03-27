// initial_solution.cpp
#include "heuristic/initialSolution.h"
// #include "lkh3_interface.h"   // your LKH-3 subprocess wrapper
#include <cmath>
#include <iostream>
#include <algorithm>

struct Solution;

static double cost_kitchen_step1(const ProblemInstance& inst, const Solution& sol, int kitchen_idx, int school_idx) {
    double cost = 0;
    double travel_time = inst.second_echelon_matrix[inst.kitchen_e2_indexes[kitchen_idx]][inst.school_indexes[school_idx]];
    double load = inst.school_demands[school_idx];
    if (!sol.kitchens_used_flags[kitchen_idx]) {
        cost = cost + inst.kitchen_fixed_costs[kitchen_idx]; // fixed cost of opening the kitchen
    }
    cost = cost + inst.second_echelon_vehicles[0].var_cost * travel_time; // variable cost of delivering from kitchen to school
    cost += sol.kitchen_demands[kitchen_idx]; //to try and make it better to go for less used kitchens
    if (travel_time > inst.maximum_2e_lh_route_duration) {
        cost += (travel_time) * 10000000; // heavy penalty for exceeding max route duration
    }
    if (sol.kitchen_demands[kitchen_idx] + load > inst.second_echelon_vehicles[0].capacity){
        cost += load * 1000;
    }
    return cost;
}

static double cost_supplier_step2(const ProblemInstance& inst, const Solution& sol, int supplier_idx, int kitchen_idx) {
    double cost = 0;
    if (!sol.suppliers_used_flags[supplier_idx]) {
        cost = cost + inst.first_echelon_vehicles[0].fix_cost;
    }
    cost = cost + inst.first_echelon_vehicles[0].var_cost * inst.first_echelon_matrix[inst.supplier_indexes[supplier_idx]][inst.kitchen_e1_indexes[kitchen_idx]];
    //find ingredient for demand multiplier
    int ingredient_idx = inst.supplier_ingredient_type_indexes[supplier_idx];
    int ingredient_demand_multiplier = inst.ingredients[ingredient_idx].tray_to_kg_ration;

    cost = cost + (ingredient_demand_multiplier * sol.kitchen_demands[kitchen_idx]);
    cost = cost + sol.supplier_demands[supplier_idx];

    return cost;
}

static bool check_route_feasibility(const ProblemInstance& inst, const Solution& sol, const VehicleRoute& route, int school_idx) {
    //check if adding this school to the route would violate any constraints
    //for simplicity, we will just check the time window constraint and capacity constraint here
    int matrix_idx_offset = 0;
    if (route.echelon == 1) {
        matrix_idx_offset = inst.supplier_indexes.size() + inst.kitchen_e1_indexes.size(); // offset to get the correct index in the distance matrix for schools
    }
    else {
        matrix_idx_offset = inst.kitchen_e2_indexes.size(); // offset to get the correct index in the distance matrix for schools
    }

    int current = route.stops.back(); //get last stop in the route
    int new_tt = route.route_travel_time + inst.second_echelon_matrix[current][inst.school_indexes[school_idx]];
    if (new_tt > inst.maximum_2e_lh_route_duration){
        return false;
    }

    double new_load = route.route_load + inst.school_demands[school_idx];
    if (new_load > route.vehicle.capacity) {
        return false;
    }
    return true; //if it passes both checks, it's feasible
}

// ─────────────────────────────────────────────────────────────────────────────
Solution InitialSolutionBuilder::build() {
    Solution sol(inst_);

    step1_satellite_clustering(sol);
    step2_depot_clustering(sol);
    step3_construct_routes(sol);
    compute_penalty(sol);

    return sol;
}

// ─────────────────────────────────────────────────────────────────────────────
// Step 1: open satellites greedily until demand is covered, then assign
//         each school to its nearest open satellite.
// ─────────────────────────────────────────────────────────────────────────────
void InitialSolutionBuilder::step1_satellite_clustering(Solution& sol) {
    // get sorted indices of schools based on their lunch time and then demand
    std::vector<int> unassigned_schools;
    const ProblemInstance& prob = inst_;

    std::sort(unassigned_schools.begin(), unassigned_schools.end(), 
        [&prob](int idx_a, int idx_b) {
            // Criterion 1: Earliest lunch time first (Ascending)
            if (prob.school_lunch_times[idx_a] != prob.school_lunch_times[idx_b]) {
                return prob.school_lunch_times[idx_a] < prob.school_lunch_times[idx_b];
            }
            // Criterion 2: If lunch times are equal, largest demand first (Descending)
            return prob.school_demands[idx_a] > prob.school_demands[idx_b];
        }
    );

    while(!unassigned_schools.empty()) {
        int school_idx = unassigned_schools.front();
        unassigned_schools.erase(unassigned_schools.begin());
        int best_kitchen = -1;
        double best_cost = std::numeric_limits<double>::infinity();
        // for each school, we should use a formula to calculate the score of each kitchen to assign to
        for (size_t kitchen_idx = 0; kitchen_idx < prob.kitchens.size(); ++kitchen_idx) {
            double cost = cost_kitchen_step1(prob, sol, kitchen_idx, school_idx);
            if (cost < best_cost) {
                best_cost = cost;
                best_kitchen = kitchen_idx;
            }
        }
        if (!sol.kitchens_used_flags[best_kitchen]) {
            sol.kitchens_used_flags[best_kitchen] = true; // open the kitchen if it's not already open
        }
        //assign school...
        sol.school_kitchen[school_idx] = best_kitchen;
        sol.kitchen_demands[best_kitchen] += prob.school_demands[school_idx]; // reduce the remaining capacity of the kitchen
    }
}

void InitialSolutionBuilder::step2_depot_clustering(Solution& sol) {
    int n_ingredients = inst_.ingredients.size();
    std::vector<std::vector<int>> array_of_suppliers_per_ingredient(n_ingredients);
    for (int i = 0; i < n_ingredients; i++){
        std::vector<int> suppliers_for_ingredient_idx;
        for (size_t j = 0; j < inst_.suppliers.size(); j++) {
            if (inst_.suppliers[j].ingredient_type == inst_.ingredients[i].name) {
                suppliers_for_ingredient_idx.push_back(j);
            }
        }
        array_of_suppliers_per_ingredient[i] = suppliers_for_ingredient_idx;
    }

    std::vector<int> open_kitchens_idx;
    for (size_t i = 0; i < sol.kitchens_used_flags.size(); ++i) {
        if (sol.kitchens_used_flags[i]) {
            open_kitchens_idx.push_back(i);
        }
    }
    for (int i = 0; i< open_kitchens_idx.size(); i++){
        for (int j = 0; j < n_ingredients; j++){
            int best_supplier_idx = -1;
            double best_supplier_cost = std::numeric_limits<int>::max();
            for (int supplier_idx : array_of_suppliers_per_ingredient[j]) {
                double cost = cost_supplier_step2(inst_, sol, supplier_idx, open_kitchens_idx[i]);
                if (cost < best_supplier_cost) {
                    best_supplier_cost = cost;
                    best_supplier_idx = supplier_idx;
                }
            }
            sol.suppliers_used_flags[best_supplier_idx] = true;
            sol.supplier_kitchen[best_supplier_idx].push_back(open_kitchens_idx[i]);
            sol.supplier_demands[best_supplier_idx] += sol.kitchen_demands[open_kitchens_idx[i]];
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Step 3: build route sequences.
//   - First echelon:  nearest-insertion for depot routes (or LKH-3 if small).
//   - Second echelon: for each satellite partition, build a VRPTW-BH route.
//     Uses LKH-3 subprocess if available; fallback = nearest-insertion with
//     a departure-batch assignment via earliest-feasible rule.
//
// Infeasible placements are ALLOWED here — they are recorded as penalties.
// ─────────────────────────────────────────────────────────────────────────────
void InitialSolutionBuilder::step3_construct_routes(Solution& sol) {
    // first i do the second echelon
    //here i want to iterate through kitchens
    //and basically route per kitchen
    std::vector<int> used_kitchens_idx;
    for (size_t i = 0; i < sol.kitchens_used_flags.size(); ++i) {
        if (sol.kitchens_used_flags[i]) {
            used_kitchens_idx.push_back(i);
        }
    }

    //i am gonna forget about trying to update the route stats in solution and just overwrite it.
    std::vector<VehicleRoute> new_second_echelon_routes;
    std::vector<int> new_second_echelon_departure_times;
    std::vector<int> new_second_echelon_vehicle_loads;
    std::vector<int> new_second_echelon_linehaul_durations;
    std::vector<double> new_second_echelon_costs;

    
    int vehicles_iterator = 0;

    for (int i = 0; i < used_kitchens_idx.size(); i++){
        //get schools assigned to this kitchen
        std::vector<int> schools_for_kitchen;
        for (size_t j = 0; j < sol.school_kitchen.size(); j++) {
            if (sol.school_kitchen[j] == i) {
                schools_for_kitchen.push_back(j);
            }
        }
        //build route for this kitchen with the assigned schools using nearest insertion
        //group schools_for_kitchen by lunch time
        std::vector<int> school_lunch_times_for_kitchen = inst_.school_lunch_times;
        std::sort(schools_for_kitchen.begin(), schools_for_kitchen.end(), 
            [&school_lunch_times_for_kitchen](int idx_a, int idx_b) {
                return school_lunch_times_for_kitchen[idx_a] < school_lunch_times_for_kitchen[idx_b];
            }
        );
        std::vector<std::vector<int>> schools_grouped_by_lunch_time;
        std::vector<int> lunch_times_available;
        for (size_t j = 0; j < school_lunch_times_for_kitchen.size(); j++) {
            int lunch_time = school_lunch_times_for_kitchen[j];
            if (std::find(lunch_times_available.begin(), lunch_times_available.end(), lunch_time) != lunch_times_available.end()){
                //found
                //push to that lunch time's array (can be assumed last array in schools_grouped_by_lunch_time)
                int idx = lunch_times_available.size() - 1;
                schools_grouped_by_lunch_time[idx].push_back(schools_for_kitchen[j]);
            }
            else {
                lunch_times_available.push_back(lunch_time);
                schools_grouped_by_lunch_time.push_back({schools_for_kitchen[j]});
            }
        }

        //routing
        std::vector<VehicleRoute> kitchen_routes; //initialize route with satellite as start and end
        
        VehicleRoute route{used_kitchens_idx[i], -1, -1, {used_kitchens_idx[i]}, 2, 0, 0, inst_.second_echelon_vehicles[vehicles_iterator]};
        kitchen_routes.push_back(route); // Replace with actual constructor parameters based on VehicleRoute definition
        std::vector<int> kitchen_route_departure_times = {0};
        std::vector<int> kitchen_route_loads = {0};
        std::vector<int> kitchen_route_durations = {0};
        std::vector<double> kitchen_route_costs = {0};

        int current_route = 0;
        int n_routed = 0;
        int current = inst_.kitchen_e2_indexes[used_kitchens_idx[i]];
        for (size_t j = 0; j < schools_grouped_by_lunch_time.size(); j++) {
            //remember that schools_grouped_by_lunch_time is 2d
            while (!schools_grouped_by_lunch_time[j].empty()) {
                //find closest school to current
                int closest_school_idx = -1;
                double closest_school_dist = std::numeric_limits<double>::max();
                for (int school_idx : schools_grouped_by_lunch_time[j]) {
                    double dist = inst_.second_echelon_matrix[current][inst_.school_indexes[school_idx]];
                    if (dist < closest_school_dist) {
                        closest_school_dist = dist;
                        closest_school_idx = school_idx;
                    }
                }
                //do checks
                if (check_route_feasibility(inst_, sol, kitchen_routes[current_route], closest_school_idx)){
                    //if it's feasible, add to route
                    kitchen_routes[current_route].stops.push_back(closest_school_idx);
                    kitchen_routes[current_route].route_load += inst_.school_demands[closest_school_idx];
                    kitchen_routes[current_route].route_travel_time += (closest_school_dist+inst_.school_service_times[closest_school_idx]);
                    kitchen_route_loads[current_route] += inst_.school_demands[closest_school_idx];
                    kitchen_route_durations[current_route] += (closest_school_dist+inst_.school_service_times[closest_school_idx]);
                    kitchen_route_costs[current_route] += (closest_school_dist * inst_.second_echelon_vehicles[vehicles_iterator].var_cost);
                    current = inst_.school_indexes[closest_school_idx];
                    n_routed++;
                }
                else {
                    if (kitchen_routes.size() < inst_.kitchens[used_kitchens_idx[i]].n_vehicles){
                        //if it's not feasible, start a new route from the satellite
                        current_route++;
                        vehicles_iterator++;
                        VehicleRoute new_route{used_kitchens_idx[i], -1, -1, {used_kitchens_idx[i]}, 2, 0, 0, inst_.second_echelon_vehicles[vehicles_iterator]};
                        //erase inst_.second_echelon_vehicles[0]
                        kitchen_routes.push_back(new_route);
                        kitchen_route_departure_times.push_back(0);
                        kitchen_route_loads.push_back(0);
                        kitchen_route_durations.push_back(0);
                        kitchen_route_costs.push_back(0);
                        current = inst_.kitchen_e2_indexes[used_kitchens_idx[i]];
                        continue; //re-evaluate}
                    }
                    else if (kitchen_routes.size() > current_route + 1){
                        //if we have more routes available, switch to adding to other route
                        current_route++;
                        current = inst_.kitchen_e2_indexes[used_kitchens_idx[i]];
                        continue; //re-evaluate
                    }
                    else{
                        //if it's not feasible but it's more than maximum vehicles, start a new route with added cost
                        current_route++;
                        VehicleRoute new_route{used_kitchens_idx[i], -1, -1, {used_kitchens_idx[i]}, 2, 0, 0, kitchen_routes[current_route].vehicle};
                        kitchen_routes.push_back(new_route);
                        kitchen_route_departure_times.push_back(0);
                        kitchen_route_loads.push_back(0);
                        kitchen_route_durations.push_back(0);
                        kitchen_route_costs.push_back(new_route.vehicle.fix_cost);
                        current = inst_.kitchen_e2_indexes[used_kitchens_idx[i]];
                        continue; //re-evaluate}
                    }
                }
                //add closest school to route
                kitchen_routes[current_route].stops.push_back(inst_.school_indexes[closest_school_idx]);
                current = inst_.school_indexes[closest_school_idx];
                n_routed++;
                //remove closest school from schools_grouped_by_lunch_time[j]
                schools_grouped_by_lunch_time[j].erase(std::remove(schools_grouped_by_lunch_time[j].begin(), schools_grouped_by_lunch_time[j].end(), closest_school_idx), schools_grouped_by_lunch_time[j].end());
            }
        }
        new_second_echelon_routes.insert(new_second_echelon_routes.end(), kitchen_routes.begin(), kitchen_routes.end());
        new_second_echelon_vehicle_loads.insert(new_second_echelon_vehicle_loads.end(), kitchen_route_loads.begin(), kitchen_route_loads.end());
        new_second_echelon_linehaul_durations.insert(new_second_echelon_linehaul_durations.end(), kitchen_route_durations.begin(), kitchen_route_durations.end());
        new_second_echelon_costs.insert(new_second_echelon_costs.end(), kitchen_route_costs.begin(), kitchen_route_costs.end());
    }
    sol.second_echelon_vehicle_loads = new_second_echelon_vehicle_loads;
    sol.second_echelon_costs = new_second_echelon_costs;
    sol.second_echelon_linehaul_durations = new_second_echelon_linehaul_durations;
    sol.second_echelon_departure_times.assign(new_second_echelon_routes.size(), 0); //change later
    for (size_t i = 0; i < new_second_echelon_routes.size(); i++){
        new_second_echelon_routes[i].linehaul_end = new_second_echelon_routes[i].stops[new_second_echelon_routes[i].stops.size()-1];
        int to_backhaul = new_second_echelon_routes[i].stops.size()-2;
        for (size_t j = to_backhaul; j >= 0; j--){
            new_second_echelon_routes[i].stops.push_back(new_second_echelon_routes[i].stops[j]);
        }
    }

    //then the first echelon
    //i think its just to route the sol.supplier_kitchen right?
    std::vector<VehicleRoute> new_first_echelon_routes;
    std::vector<std::vector<int>> new_first_echelon_routes_per_ingredient_type;
    new_first_echelon_routes_per_ingredient_type.assign(inst_.ingredients.size(), {});
    std::vector<int> new_first_echelon_vehicle_loads;
    std::vector<int> new_first_echelon_durations;
    std::vector<double> new_first_echelon_costs;
    vehicles_iterator = 0;
    for (size_t i; i < sol.supplier_kitchen.size(); i++){
        VehicleRoute supplier_route{i, 0, -1, {inst_.supplier_indexes[i]}, 1, 0, 0, inst_.first_echelon_vehicles[vehicles_iterator]};
        double route_load = 0;
        double route_durations = 0;
        double route_costs = 0;
        //build route
        int current = inst_.supplier_indexes[i];
        for (size_t j; j < sol.supplier_kitchen[i].size(); j++){
            //find closest
            int closest_kitchen_idx = -1;
            double closest_kitchen_dist = std::numeric_limits<double>::max();
            for (int kitchen_idx : sol.supplier_kitchen[i]) {
                double dist = inst_.second_echelon_matrix[current][inst_.kitchen_e1_indexes[kitchen_idx]];
                if (dist < closest_kitchen_dist) {
                    closest_kitchen_dist = dist;
                    closest_kitchen_idx = kitchen_idx;
                }
                int demand = inst_.ingredients[inst_.supplier_ingredient_type_indexes[i]].tray_to_kg_ration * sol.kitchen_demands[closest_kitchen_idx];
                route_load += demand;
                route_durations += inst_.first_echelon_matrix[current][inst_.kitchen_e1_indexes[closest_kitchen_idx]];
                current = inst_.kitchen_e1_indexes[closest_kitchen_idx];
                route_costs += (closest_kitchen_dist * inst_.first_echelon_vehicles[0].var_cost);
                supplier_route.stops.push_back(inst_.kitchen_e1_indexes[kitchen_idx]);
                supplier_route.route_load = route_load;
                supplier_route.route_travel_time = route_durations;
            }
        }
        new_first_echelon_routes.push_back(supplier_route);
        new_first_echelon_vehicle_loads.push_back(route_load);
        new_first_echelon_durations.push_back(route_durations);
        new_first_echelon_costs.push_back(route_costs);
        new_first_echelon_routes_per_ingredient_type[inst_.supplier_ingredient_type_indexes[i]].push_back(i);
    }
    sol.first_echelon_routes = new_first_echelon_routes;
    sol.first_echelon_durations = new_first_echelon_durations;
    sol.first_echelon_vehicle_loads = new_first_echelon_vehicle_loads;
    sol.first_echelon_costs = new_first_echelon_costs;
    sol.first_echelon_routes_per_ingredient_type = new_first_echelon_routes_per_ingredient_type;
}
