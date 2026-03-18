// placeholder
#include <cmath>
#include <fstream>
#include <numeric>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "problem/problem.h"
#include "problem/node.h"
#include "problem/school.h"
#include "problem/kitchen.h"
#include "problem/supplier.h"
#include "problem/vehicle.h"
#include "problem/ingredient.h"


static std::vector<std::string> split(const std::string input, const char delimiter) {
  std::vector<std::string> tokens;
  std::stringstream ss(input);
  std::string token;
  while (getline(ss, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

static std::vector<int> parsePrefLockerIdxString(std::string prefLockerIdxsString) {
  std::vector<std::string> prefLockerIdxsStringVec = split(prefLockerIdxsString, '-');
  std::vector<int> prefLockerIdxs;
  for (auto& token : prefLockerIdxsStringVec) {
    prefLockerIdxs.push_back(std::stoi(token));
  }
  return prefLockerIdxs;
}

ProblemInstance readProblemInstance(std::string instancePath) {
  std::ifstream file(instancePath);
  std::string errMessage = "Error opening, file not found at " + instancePath;
  if (!file) throw std::runtime_error(errMessage);

  std::string line;
  int index = 0;

  std::vector<School> schools;
  std::vector<std::vector<double>> distanceMatrix;

  getline(file, line);
  getline(file, line);
  while (getline(file, line)) {
    if (line == "depot") break;
    auto tokens = split(line, ',');
    int idx = std::stoi(tokens[0]);
    int capacity= std::stoi(tokens[1]);
    double cost = std::stod(tokens[2]);
    vehicles.emplace_back(idx, capacity, cost);
  }
  getline(file, line);
  getline(file, line);
  auto tokensD = split(line,',');
  int idxDepot = std::stoi(tokensD[0]);
  double depotX = std::stod(tokensD[1]);
  double depotY = std::stod(tokensD[2]);
  Node depot(idxDepot, depotX, depotY, NodeType::Depot);
  getline(file, line);
  getline(file, line);

  while (getline(file, line)) {
    if (line == "self pickup customers") break;
    auto tokens = split(line, ',');
    int idx = std::stoi(tokens[0]);
    double x = std::stod(tokens[1]);
    double y = std::stod(tokens[2]);
    int demand = std::stoi(tokens[4]);
    std::vector<int> prefLockerIdxs;
    customers.emplace_back(idx, x, y, NodeType::HomeDelivery, demand, prefLockerIdxs);
  }
  getline(file, line);
  while (getline(file, line)) {
    if (line == "flexible customers") break;
    auto tokens = split(line, ',');
    int idx = std::stoi(tokens[0]);
    double x = std::stod(tokens[1]);
    double y = std::stod(tokens[2]);
    int demand = std::stoi(tokens[4]);
    std::string prefLockerIdxsString = tokens[5];
    std::vector<int> prefLockerIdxs = parsePrefLockerIdxString(prefLockerIdxsString);
    customers.emplace_back(idx, x, y, NodeType::SelfPickup, demand, prefLockerIdxs);
  }
  getline(file, line);
  while (getline(file, line)) {
    if (line == "lockers") break;
    auto tokens = split(line, ',');
    int idx = std::stoi(tokens[0]);
    double x = std::stod(tokens[1]);
    double y = std::stod(tokens[2]);
    int demand = std::stoi(tokens[4]);
    std::string prefLockerIdxsString = tokens[5];
    std::vector<int> prefLockerIdxs = parsePrefLockerIdxString(prefLockerIdxsString);
    customers.emplace_back(idx, x, y, NodeType::Flexible, demand, prefLockerIdxs);
  }
  getline(file, line);
  while (getline(file, line)) {
    if (line == "mrt lines") break;
    auto tokens = split(line, ',');
    int idx = std::stoi(tokens[0]);
    double x = std::stod(tokens[1]);
    double y = std::stod(tokens[2]);
    int capacity = std::stoi(tokens[4]);
    lockers.emplace_back(idx, x, y, capacity);
  }
  getline(file, line);
  while (getline(file, line)) {
    if (line == "distance matrix") break;
    auto tokens = split(line, ',');
    int startStationIdx = std::stoi(tokens[0]);
    int endStationIdx = std::stoi(tokens[1]);
    int capacity = std::stoi(tokens[2]);
    double cost = std::stod(tokens[3]);
    mrtLines.emplace_back(startStationIdx, endStationIdx, cost, capacity);
  }
  getline(file, line);
  int numNodes = int(customers.size()) + int(lockers.size()) + 1;
  distanceMatrix.assign(numNodes, std::vector<double>(numNodes));
  for (int i = 0; i < numNodes; i++) {
    getline(file, line);
    auto distsString = split(line, ',');
    for (int j = 0; j < numNodes; j++) {
      distanceMatrix[i][j] = std::stod(distsString[j + 1]);
    }
  }
  ProblemInstance problem(depot, customers, lockers, mrtLines, vehicles, distanceMatrix);
  return problem;
}

ProblemInstance::ProblemInstance(Node depot, std::vector<School>& schools,
  std::vector<Kitchen>& kitchens, std::vector<Supplier>& suppliers,
  std::vector<Vehicle>& first_echelon_vehicles,
  std::vector<Vehicle>& second_echelon_vehicles,
  std::optional<std::vector<std::vector<double>>> first_echelon_matrix,
  std::optional<std::vector<std::vector<double>>> second_echelon_matrix,
  std::vector<Ind
){
  
  numVehicles = int(vehicles.size());
  vehicleCosts.reserve(numVehicles);
  vehicleIdxs.resize(numVehicles);
  std::iota(vehicleIdxs.begin(), vehicleIdxs.end(), 0);
  for (auto& vehicle : vehicles) {
    vehicleCosts.push_back(vehicle.cost);
    vehicleCapacities.push_back(vehicle.capacity);
  }
  
  numCustomers = int(customers.size());
  numLockers = int(lockers.size());
  numNodes = numCustomers + numLockers + 1;
  coords.assign(numNodes, {});
  coords[0] = { depot.x, depot.y };

  demands.resize(numNodes, 0);
  nodeTypes.resize(numNodes, NodeType::Locker);
  nodeTypes[0] = NodeType::Depot;;
  destinationAlternatives.resize(numNodes);
  for (auto& customer : customers) {
    customerIdxs.push_back(customer.idx);
    demands[customer.idx] = customer.demand;
    nodeTypes[customer.idx] = customer.nodeType;
    coords[customer.idx] = { customer.x, customer.y };
    if (customer.nodeType == NodeType::SelfPickup || customer.nodeType == NodeType::Flexible) {
      destinationAlternatives[customer.idx] = customer.preferredLockerIdxs;
    }
    if (customer.nodeType != NodeType::SelfPickup) {
      destinationAlternatives[customer.idx].push_back(customer.idx);
    }
  }
  lockerCapacities.resize(numNodes);
  for (auto& locker : lockers) {
    lockerIdxs.push_back(locker.idx);
    lockerCapacities[locker.idx] = locker.capacity;
    coords[locker.idx] = { locker.x, locker.y };
  }

  numMrtLines = int(mrtLines.size());
  incomingMrtLinesIdx.assign(numNodes, -1);
  outgoingMrtLinesIdx.assign(numNodes, -1);
  mrtLineEndStationIdxs.resize(numMrtLines);
  mrtLineCapacities.resize(numMrtLines);
  mrtLineStartStationIdxs.resize(numMrtLines);
  mrtLineCosts.resize(numMrtLines);
  for (int mrtLineIdx = 0; mrtLineIdx < numMrtLines;mrtLineIdx++) {
    auto& mrtLine = mrtLines[mrtLineIdx];
    mrtLineIdxs.push_back(mrtLineIdx);
    incomingMrtLinesIdx[mrtLine.endStationNodeIdx] = mrtLineIdx;
    outgoingMrtLinesIdx[mrtLine.startStationNodeIdx] = mrtLineIdx;
    mrtLineCapacities[mrtLineIdx] = mrtLine.freightCapacity;
    mrtLineCosts[mrtLineIdx] = mrtLine.cost;
    mrtLineStartStationIdxs[mrtLineIdx] = mrtLine.startStationNodeIdx;
    mrtLineEndStationIdxs[mrtLineIdx] = mrtLine.endStationNodeIdx;
  }
  
  if (distanceMatrix_) {
    distanceMatrix = *distanceMatrix_;
  }
  else {
    distanceMatrix.assign(numNodes, std::vector<double>(numNodes));
    for (int i = 0; i < numNodes;i++) {
      for (int j = 0; j < numNodes; j++) {
        double x2 = (coords[i][0] - coords[j][0]) * (coords[i][0] - coords[j][0]);
        double y2 = (coords[i][1] - coords[j][1]) * (coords[i][1] - coords[j][1]);
        distanceMatrix[i][j]= std::sqrt(x2 + y2);
      }
    }
  }
}
