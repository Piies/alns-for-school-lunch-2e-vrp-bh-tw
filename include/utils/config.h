#pragma once

#include <string>
#include <unordered_map>
#include <iostream>

/**
 * @brief Configuration parameters for Adaptive Repartition or sensitivity analysis experiments.
 *
 * This structure defines all runtime and sensitivity settings used
 * when executing an experiment or optimization run. Most parameters
 * control iteration counts and scoring behavior, while others allow
 * for sensitivity analysis by modifying instance data such as vehicle
 * capacity, cost, or MRT line discount rate.
 */
struct Config {
  /* @brief Name of the instance file to be loaded. */
  std::string instanceName = "small-sch10-kch5-spl5-dmnd1032-sppg0.20-mv1.50";

  /** @brief Maximum number of iterations in the search process. */
  int maxIteration = 30000;

  /** @brief Initial temperature for the simulated annealing process. */
  double initialTemp = 0.05 * maxIteration;

  /** @brief Minimum temperature for the simulated annealing process. */
  double minimumTemp = 0.06;

  /** @brief Length of each segment for scoring evaluation. */
  int segmentLength = 100;

  /** @brief Reaction factor used in score adaptation (0 < reactionFactor ≤ 1). */
  double reactionFactor = 0.1;

  /** @brief Score weight assigned to newly found best solutions. */
  double bestFoundScore = 40;

  /** @brief Score weight assigned to accepted (non-best) solutions. */
  double acceptedScore = 20;

  /** @brief Score weight assigned to worse but still accepted solutions. */
  double worseAcceptedScore = 10;

  /** @brief Initial weight for the operators. */
  double initialWeight = 40.0;

  /** @brief Minimum percentage of customers to remove during destroy phase. */
  double minRemoved = 0.20;

  /** @brief Maximum percentage of customers to remove during destroy phase. */
  double maxRemoved = 0.40;
  
  /**
   * @brief Adjusted vehicle capacity for sensitivity analysis.
   * 
   * If > 0, overrides the original vehicle capacity from the instance file.
   * Used to test the effect of capacity changes.
   */
  int adjustedVehicleCapacity = -1;

  /**
   * @brief Adjusted vehicle cost for sensitivity analysis.
   * 
   * If > 0, overrides the original vehicle operating cost from the instance file.
   * Used to evaluate cost sensitivity.
   */
  double adjustedVehicleCost = -1;

  // ==========================================
  // MBG Logistical Constraints (From Chapter 3)
  // ==========================================

  /** @brief Driver rest time in minutes before starting backhaul. */
  int driverRestTime = 30;

  /** @brief Overtime threshold in minutes (1080 mins = 18:00). */
  int overtimeThreshold = 1080;

  /** @brief Maximum allowed time in minutes from food prep to consumption. */
  int maxFoodAge = 180;

  /** @brief Optional suffix to append to the output result directory name. */
  std::string resultDirSuffix = "";

  Config(int argc, char* argv[]) {
    std::unordered_map<std::string, std::string> args;

    // Parse arguments into key-value pairs
    for (int i = 1; i < argc - 1; i += 2) {
      std::string key = argv[i];
      if (key.rfind("--", 0) == 0) {
          args[key.substr(2)] = argv[i + 1];
      }
    }

    // Assign values if provided
    if (args.count("instance-name")) instanceName = args["instance-name"];
    if (args.count("max-iteration")) maxIteration = std::stoi(args["max-iteration"]);
    if (args.count("initial-temp")) initialTemp = std::stod(args["initial-temp"]);
    if (args.count("minimum-temp")) minimumTemp = std::stod(args["minimum-temp"]);
    if (args.count("segment-length")) segmentLength = std::stoi(args["segment-length"]);
    if (args.count("reaction-factor")) reactionFactor = std::stod(args["reaction-factor"]);
    if (args.count("adjusted-vehicle-capacity")) adjustedVehicleCapacity = std::stoi(args["adjusted-vehicle-capacity"]);
    if (args.count("adjusted-vehicle-cost")) adjustedVehicleCost = std::stod(args["adjusted-vehicle-cost"]);
    if (args.count("best-found-score")) bestFoundScore = std::stod(args["best-found-score"]);
    if (args.count("accepted-score")) acceptedScore = std::stod(args["accepted-score"]);
    if (args.count("worse-accepted-score")) worseAcceptedScore = std::stod(args["worse-accepted-score"]);
    if (args.count("initial-weight")) initialWeight = std::stod(args["initial-weight"]);
    if (args.count("min-removed")) minRemoved = std::stod(args["min-removed"]);
    if (args.count("max-removed")) maxRemoved = std::stod(args["max-removed"]);
    if (args.count("driver-rest-time")) driverRestTime = std::stoi(args["driver-rest-time"]);
    if (args.count("overtime-threshold")) overtimeThreshold = std::stoi(args["overtime-threshold"]);
    if (args.count("max-food-age")) maxFoodAge = std::stoi(args["max-food-age"]);
    if (args.count("result-dir-suffix")) resultDirSuffix = args["result-dir-suffix"];
  }

  void print() const {
      std::cout << "instanceName: " << instanceName << "\n"
                << "maxIteration: " << maxIteration << "\n"
                << "initialTemp: " << initialTemp << "\n"
                << "minimumTemp: " << minimumTemp << "\n"
                << "segmentLength: " << segmentLength << "\n"
                << "reactionFactor: " << reactionFactor << "\n"
                << "bestFoundScore: " << bestFoundScore << "\n"
                << "acceptedScore: " << acceptedScore << "\n"
                << "worseAcceptedScore: " << worseAcceptedScore << "\n"
                << "adjustedVehicleCapacity: " << adjustedVehicleCapacity <<"\n"
                << "adjustedVehicleCost: " << adjustedVehicleCost <<"\n"
                << "initialWeight: " << initialWeight << "\n"
                << "minRemoved: " << minRemoved << "\n"
                << "maxRemoved: " << maxRemoved << "\n"
                << "driverRestTime: " << driverRestTime << "\n"
                << "overtimeThreshold: " << overtimeThreshold << "\n"
                << "maxFoodAge: " << maxFoodAge << "\n"
                << "resultDirSuffix: " << resultDirSuffix << "\n";
  }
};