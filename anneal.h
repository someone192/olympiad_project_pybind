#ifndef ANNEAL_H
#define ANNEAL_H

#include <vector>
#include <tuple>
#include <random>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <numeric>  // Для std::iota
#include <map>

using Pos = std::tuple<double, double, double>;
using AxlePos = std::tuple<double, double>;

std::vector<Pos> greedy_start(const std::vector<double>& masses, double L, double W, double H, Pos target);
Pos calculate_com(const std::vector<double>& masses, const std::vector<Pos>& positions);
double calculate_error(const std::vector<Pos>& positions, const std::vector<double>& masses, Pos target);
bool valid_stacking(const std::vector<Pos>& positions, const std::vector<double>& masses, double box_size);
std::vector<double> simulate_loads(const std::vector<Pos>& positions, const std::vector<double>& masses, const std::vector<AxlePos>& axle_positions);
bool check_constraints(const std::vector<double>& loads, const std::vector<double>& limits, double total_limit, double total_mass);
std::vector<Pos> simulated_annealing(const std::vector<double>& masses, double L, double W, double H, Pos target, const std::vector<double>& axle_limits, const std::vector<double>& initial_loads, const std::vector<AxlePos>& axle_positions, double total_limit, double threshold, double box_size);

#endif