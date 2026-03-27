#pragma once

#include <fmt/core.h>
#include <optional>
#include <vector>

struct ProblemInstance;

struct VehicleRoute {
    int satellite_idx;
    int departure_batch;          // minutes from midnight: 300, 420, or 540
    int linehaul_end;             // index into `stops` where backhaul begins
    std::vector<int> stops;       // stops are saved in their dist_matrix indices
    int echelon;
    double route_load;
    double route_travel_time;
    Vehicle vehicle;
    // backhaul can be derived: stops[linehaul_end..end]
};


struct Solution {
	//sesuai sempro
  std::vector<VehicleRoute> first_echelon_routes;
  std::vector<VehicleRoute> second_echelon_routes;

  std::vector<int> second_echelon_departure_times;

  std::vector<bool> kitchens_used_flags;
  std::vector<bool> suppliers_used_flags;

  //list of routes per ingredient type, for use
  std::vector<std::vector<int>> first_echelon_routes_per_ingredient_type;

  //to keep track
  std::vector<int> first_echelon_vehicle_loads;
  std::vector<int> second_echelon_vehicle_loads;

  std::vector<int> first_echelon_durations;
  std::vector<int> second_echelon_linehaul_durations;

  std::vector<int> kitchen_demands;
  std::vector<int> supplier_demands;
  std::vector<int> school_kitchen;  // school_idx(index) -> kitchen_idx(value)
  std::vector<std::vector<int>> supplier_kitchen; // supplier_idx(index) -> kitchen_idx(values)

	//stats for solution
  double total_cost = 0;
  std::vector<double> first_echelon_costs;
	std::vector<double> second_echelon_costs;

  double penalty = 0;

  //truke complementary data for guardrails, optional
  std::vector<std::vector<std::string>>first_echelon_routes_ids;
  std::vector<std::vector<std::string>>second_echelon_routes_ids;

  Solution(const ProblemInstance& problem);
  Solution(const Solution&) = default;
  Solution& operator=(const Solution&) = default;
  Solution(Solution&&) noexcept = default;
  Solution& operator=(Solution&&) noexcept = default;
  Solution() = default;
  ~Solution();

  bool isFeasible(const ProblemInstance& problem) const;
  
  // void insertCustomer(const Cvrpptpl& problem, int custIdx, int vehicleIdx, int pos);
  // double computeNodeInsertionDistanceCost(const Cvrpptpl& problem, int nodeIdx, int vehicleIdx, int pos) const;
  // double computeCustomerInsertionDCost(const Cvrpptpl& problem, int custIdx, int vehicleIdx, int pos) const;
  // void assignCustomerToDestination(const Cvrpptpl& problem, int custIdx, int destinationIdx);
  // double computeAssignCustomerToDestinationPotentialDCost(const Cvrpptpl& problem, int custIdx, 
  //   int destinationIdx, std::optional<std::vector<double>> bestInsertionCost=std::nullopt) const;
  // double computeLockerInsertionDCost(const Cvrpptpl& problem, int lockerIdx, int vehicleIdx, 
  //                                     int pos, bool viaMrt=false) const;
  // void insertLocker(const Cvrpptpl& problem, int lockerIdx, int vehicleIdx, int pos, bool viaMrt=false);
  // void insertNode(const Cvrpptpl& problem, int nodeIdx, int vehicleIdx, int pos);
  // void insertSegment(const Cvrpptpl& problem, const std::vector<int> segment, int vehicleIdx, int pos);
  // void removeAllLockers(const Cvrpptpl& problem);
  // void removeAllLockersFromVehicle(const Cvrpptpl& problem, int vehicleIdx);
  // void removeNodeFromRouteByNodeIdx(const Cvrpptpl& problem, int vehicleIdx, int nodeIdx);
  // void removeNodeFromRouteBySeq(const Cvrpptpl& problem, int vehicleIdx, int i);
  // double computeNodeRemovalFromRouteBySeqDCost(const Cvrpptpl& problem, int vehicleIdx, int i) const;
  // void removeNodesFromRouteBySeqs(const Cvrpptpl& problem, int vehicleIdx, std::vector<int>& positions);
  // double computeNodeRemovalFromRouteDCost(const Cvrpptpl& problem, int vehicleIdx, int nodeIdx) const;
  // void completeRemoveCustomer(const Cvrpptpl& problem, int custIdx);
  // double computeCustomerCompleteRemovalDCost(const Cvrpptpl& problem, int custIdx) const;
  // void completeRemoveLocker(const Cvrpptpl& problem, int lockerIdx);
  // double computeLockerCompleteRemovalDCost(const Cvrpptpl& problem, int lockerIdx) const;
  // void removeEmptyDestination(const Cvrpptpl& problem);  
  // void swapNodesIntraRoute(const Cvrpptpl& problem, int vehicleIdx, int posA, int posB);
  // void relocateNodeIntraRoute(const Cvrpptpl& problem, int vehicleIdx, int origPos, int newPos);
  // void relocateSegmentIntraRoute(const Cvrpptpl& problem, int vehicleIdx, const std::vector<int>& segment, int newPos);
  // double computeRelocateSegmentIntraRouteDCost(const Cvrpptpl& problem, int vehicleIdx, const std::vector<int>& posSegment, int newPos) const;

  // int numMrtUsed() const;
  // int numVehicleUsed() const;
  // double getTotalVehicleCharge() const;
  // double getTotalMrtCharge() const;

  // bool isFeasible(const Cvrpptpl& problem) const;
  // void assertFeasible(const Cvrpptpl& problem) const;
  // void print(const Cvrpptpl& problem) const;
};
