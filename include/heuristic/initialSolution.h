// initial_solution.h
#pragma once
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <limits>
#include <cassert>
#include "problem/problem.h"
#include "heuristic/solution.h"

struct Solution;

// ─────────────────────────────────────────────────────────────────────────────
//  InitialSolution builder
// ─────────────────────────────────────────────────────────────────────────────
class InitialSolutionBuilder {
public:
    explicit InitialSolutionBuilder(const ProblemInstance& inst,
                                    double infeasibility_weight = 1e5)
        : inst_(inst), W_(infeasibility_weight) {}

    Solution build();

private:
    // ── Step 1 ──────────────────────────────────────────────────────────────
    // Greedy kitchen selection + nearest-satellite school assignment.
    // Opens satellites one by one (cheapest avg. distance to unserved schools)
    // until total capacity ≥ total demand.
    void step1_satellite_clustering(Solution& sol);

    // ── Step 2 ──────────────────────────────────────────────────────────────
    // Per ingredient β, for each open satellite s:
    //   choose the depot d ∈ Dβ minimising direct travel cost c1 * dist(d, s).
    // No capacity limit on first-echelon vehicles per your proposal.
    void step2_depot_clustering(Solution& sol);

    // ── Step 3 ──────────────────────────────────────────────────────────────
    // Build feasible routes using cheapest-insertion within each partition.
    // Calls LKH3 subprocess if available; falls back to nearest-neighbour.
    // Records violations as penalty instead of hard-rejecting.
    void step3_construct_routes(Solution& sol);

    // ── Helpers ─────────────────────────────────────────────────────────────
    double sat_avg_dist_to_schools(int s,
                                   const std::vector<int>& unassigned) const;
    int    nearest_open_satellite(int school_idx, const Solution& sol) const;
    double nearest_insertion_cost(const std::vector<int>& route,
                                  int candidate,
                                  const std::vector<std::vector<double>>& dist) const;
    void   compute_penalty(Solution& sol);

    const ProblemInstance& inst_;
    double W_;  // penalty weight per violated unit
};