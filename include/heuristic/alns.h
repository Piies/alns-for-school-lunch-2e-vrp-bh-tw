#pragma once

#include <memory>
#include <random>
#include <string>
#include <tbb/task_group.h>
#include <vector>

#include "heuristic/solution.h"
//#include "heuristic/summary.h"

//this is forward declaration to avoid header inclusion, just trying something new.
struct DestroyOperatorL1;
struct InsertionOperatorL1;
struct DestroyOperatorL2;
struct InsertionOperatorL2;
struct Cvrpptpl;
struct LocalSearchOperator;

struct Alns{
  int maxIteration = 100000;
  int etaScore = 50;
  int etaResetBest = 400;
  int etaRestart = 5000;
  int patienceL1 = 10;
  double smoothingFactor = 0.8;
  double bestFoundScore = 15;
  double acceptedScore = 4;
  double worseAcceptedScore = 1;
  tbb::task_group tg;

  //these ones are set after we have our initial solution
  double temperature = 1000;
  double alphaTemperature = 0.99;

  std::vector<std::unique_ptr<InsertionOperatorL1>> insertionOperatorsL1;
  std::vector <std::unique_ptr<DestroyOperatorL1>> destroyOperatorsL1;

  std::vector<std::unique_ptr<InsertionOperatorL2>> insertionOperatorsL2;
  std::vector <std::unique_ptr<DestroyOperatorL2>> destroyOperatorsL2;

  std::vector <std::unique_ptr<LocalSearchOperator>> localSearchOperators;
    
  //actual score
  std::vector<double> insertionScoresL1;
  std::vector<double> destroyScoresL1;
  std::vector<double> insertionScoresL2;
  std::vector<double> destroyScoresL2;

  //used to compute probability, periodically updated by the scores
  std::vector<double> insertionWeightsL1;
  std::vector<double> destroyWeightsL1;
  std::vector<double> insertionWeightsL2;
  std::vector<double> destroyWeightsL2;
  std::vector<int> insertionUsageCountsL1;
  std::vector<int> destroyUsageCountsL1;
  std::vector<int> destroyUsageCountsL2;
  std::vector<int> insertionUsageCountsL2;

  std::mt19937 rng = std::mt19937(42); // The member RNG

  
  int iteration = 0;
  Solution currentSolution;
  Solution bestSolution;

  auto selectDestroyOperatorL1();
  auto selectInsertionOperatorL1();
  auto selectDestroyOperatorL2();
  auto selectInsertionOperatorL2();
  void updateOperatorWeights(std::vector<double>& scores, std::vector<int>& usageCounts, std::vector<double>& weights);
  bool acceptWorseSolution(const Solution& currentSolution, const Solution& newSolution);
  void initializeTemperature(const Solution& initialSolution);
  Solution solve(const Cvrpptpl& problem);
  bool applyL1(const Cvrpptpl& problem);
  void applyL2(const Cvrpptpl& problem);
  Solution applyLocalSearch(const Cvrpptpl& problem, const Solution& candidSolution);

  Alns(int maxIteration, int etaScore, int etaResetBest, 
    int etaRestart, double smoothingFactor, int patienceL1,
    std::vector<std::unique_ptr<InsertionOperatorL1>> insertionOperatorsL1_,
    std::vector <std::unique_ptr<DestroyOperatorL1>> destroyOperatorsL1_,
    std::vector <std::unique_ptr<LocalSearchOperator>> localSearchOperators_,
    std::vector<std::unique_ptr<InsertionOperatorL2>> insertionOperatorsL2_,
    std::vector<std::unique_ptr<DestroyOperatorL2>> destroyOperatorsL2_);
  ~Alns();
};
