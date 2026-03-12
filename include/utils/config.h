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
  std::string instanceName = "small-sch10-kch5-spl5-dmnd1032-sppg0.2-mv1.5.json";

  /** @brief Maximum number of iterations in the search process. */
  int maxIteration = 30000;

  /** @brief Scoring decay factor for operator performance evaluation. */
  int etaScore = 10;

  /** @brief Number of iterations before resetting the current best solution score. */
  int etaResetBest = 200;

  /** @brief Number of iterations before performing a full restart of the search. */
  int etaRestart = 3000;

  /** @brief Smoothing factor used in score adaptation (0 < smoothingFactor ≤ 1). */
  double smoothingFactor = 2./3.;

  /** @brief Score weight assigned to newly found best solutions. */
  double bestFoundScore = 15;

  /** @brief Score weight assigned to accepted (non-best) solutions. */
  double acceptedScore = 4;

  /** @brief Score weight assigned to worse but still accepted solutions. */
  double worseAcceptedScore = 1;

  /**
   * @brief MRT line cost discount rate (0 means disabled).
   * 
   * If nonzero, the MRT line transportation cost is multiplied by (1 − mrtLineDiscountRate)
   * to simulate discounted fares in sensitivity analysis.
   */
  double mrtLineDiscountRate = 0;

  /**
   * @brief Early-stopping patience for L1 operators.
   * 
   * Number of iterations allowed without improvement before switching or intensifying L1 operators.
   */
  int patienceL1 = 20;

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
    if (args.count("eta-score")) etaScore = std::stoi(args["eta-score"]);
    if (args.count("eta-reset-best")) etaResetBest = std::stoi(args["eta-reset-best"]);
    if (args.count("eta-restart")) etaRestart = std::stoi(args["eta-restart"]);
    if (args.count("smoothing-factor")) smoothingFactor = std::stod(args["smoothing-factor"]);
    if (args.count("mrt-line-discount-rate")) mrtLineDiscountRate = std::stod(args["mrt-line-discount-rate"]);
    if (args.count("adjusted-vehicle-capacity")) adjustedVehicleCapacity = std::stoi(args["adjusted-vehicle-capacity"]);
    if (args.count("adjusted-vehicle-cost")) adjustedVehicleCost = std::stod(args["adjusted-vehicle-cost"]);
    if (args.count("best-found-score")) bestFoundScore = std::stod(args["best-found-score"]);
    if (args.count("accepted-score")) acceptedScore = std::stod(args["accepted-score"]);
    if (args.count("worse-accepted-score")) worseAcceptedScore = std::stod(args["worse-accepted-score"]);
    if (args.count("patience-l1")) patienceL1 = std::stoi(args["patience-l1"]);
    if (args.count("result-dir-suffix")) resultDirSuffix = args["result-dir-suffix"];
  }

  void print() const {
      std::cout << "instanceName: " << instanceName << "\n"
                << "etaScore: " << etaScore << "\n"
                << "etaResetBest: " << etaResetBest << "\n"
                << "etaRestart: " << etaRestart << "\n"
                << "smoothingFactor: " << smoothingFactor << "\n"
                << "bestFoundScore: " << bestFoundScore << "\n"
                << "acceptedScore: " << acceptedScore << "\n"
                << "worseAcceptedScore: " << worseAcceptedScore << "\n"
                << "patienceL1: " << patienceL1 << "\n"
                << "mrtLineDiscountRate" << mrtLineDiscountRate <<"\n"
                << "adjustedVehicleCapacity" << adjustedVehicleCapacity <<"\n"
                << "adjustedVehicleCost" << adjustedVehicleCost <<"\n"
                << "resultDirSuffix: " << resultDirSuffix << "\n";
  }
};