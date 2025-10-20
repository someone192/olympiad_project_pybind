#include "anneal.h"

std::vector<Pos> greedy_start(const std::vector<double>& masses, double L, double W, double H, Pos target) {
    size_t n = masses.size();
    std::vector<size_t> indices(n);
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&masses](size_t i, size_t j) { return masses[i] > masses[j]; });  // Тяжелые сначала

    double nx = std::floor(L / 1.0);  // box_size=1.0
    double ny = std::floor(W / 1.0);
    double nz = std::floor(H / 1.0);
    if (n > static_cast<size_t>(nx * ny * nz)) return {};  // Ошибка: превышение объёма

    std::vector<Pos> positions(n);
    double tx = std::get<0>(target), ty = std::get<1>(target);
    size_t idx = 0;  // Индекс следующей коробки

    for (double z = 0.5; z < H; z += 1.0) {  // Слои снизу вверх (центры ячеек)
        // Генерируем все ячейки в слое
        std::vector<Pos> layer_cells;
        for (double x = 0.5; x < L; x += 1.0) {
            for (double y = 0.5; y < W; y += 1.0) {
                layer_cells.emplace_back(x, y, z);  // (x, y, z) как Pos
            }
        }

        // Сортируем ячейки по расстоянию к (tx, ty) в XY (ascending: ближе — раньше)
        std::sort(layer_cells.begin(), layer_cells.end(), [tx, ty](const Pos& a, const Pos& b) {
            auto [ax, ay, az] = a;
            auto [bx, by, bz] = b;
            double dist_a = std::sqrt(std::pow(ax - tx, 2) + std::pow(ay - ty, 2));
            double dist_b = std::sqrt(std::pow(bx - tx, 2) + std::pow(by - ty, 2));
            return dist_a < dist_b;  // Меньшее расстояние — раньше
        });

        // Размещаем тяжелые коробки в отсортированные ячейки слоя
        for (const auto& cell : layer_cells) {
            if (idx < n) {
                positions[indices[idx]] = cell;  // Тяжёлая в ближайшей к центру
                ++idx;
            } else {
                break;  // Нет больше коробок
            }
        }
    }

    // TODO: Если нужно, проверить/скорректировать stacking (mass_above <= mass_below в колонках)
    return positions;
}

Pos calculate_com(const std::vector<double>& masses, const std::vector<Pos>& positions) {
    double total_mass = std::accumulate(masses.begin(), masses.end(), 0.0);
    double xc = 0, yc = 0, zc = 0;
    for (size_t i = 0; i < masses.size(); ++i) {
        auto [x, y, z] = positions[i];
        xc += masses[i] * x; yc += masses[i] * y; zc += masses[i] * z;
    }
    return {xc / total_mass, yc / total_mass, zc / total_mass};
}

double calculate_error(const std::vector<Pos>& positions, const std::vector<double>& masses, Pos target) {
    auto [xc, yc, zc] = calculate_com(masses, positions);
    auto [tx, ty, tz] = target;  // tz — min z, penalty за высокую zc
    double dist_xy = std::sqrt(std::pow(xc - tx, 2) + std::pow(yc - ty, 2));
    double penalty_z = zc;  // Минимизируем zc
    return dist_xy + std::pow(zc - tz, 2) + 10.0 * penalty_z;  // Weighted
}

bool valid_stacking(const std::vector<Pos>& positions, const std::vector<double>& masses, double box_size) {
    std::map<std::pair<double, double>, std::vector<std::pair<double, double>>> stacks;
    for (size_t i = 0; i < positions.size(); ++i) {
        auto [x, y, z] = positions[i];
        stacks[{x, y}].emplace_back(z, masses[i]);
    }

    for (auto& [xy, stack] : stacks) {
        std::sort(stack.begin(), stack.end());  // По z
        for (size_t i = 1; i < stack.size(); ++i) {
            if (stack[i].second > stack[i-1].second) {  // mass_above > mass_below
                return false;
            }
        }
    }
    return true;
}

std::vector<double> simulate_loads(const std::vector<Pos>& positions, const std::vector<double>& masses, const std::vector<AxlePos>& axle_positions) {
    std::vector<double> loads(axle_positions.size(), 0.0);
    double total_span_x = std::get<0>(axle_positions.back()) - std::get<0>(axle_positions.front());  // Пример для x
    for (size_t i = 0; i < axle_positions.size(); ++i) {
        auto [ax, ay] = axle_positions[i];
        for (size_t j = 0; j < masses.size(); ++j) {
            auto [bx, by, bz] = positions[j];
            double dist = std::sqrt(std::pow(bx - ax, 2) + std::pow(by - ay, 2));
            loads[i] += masses[j] * (1.0 - dist / total_span_x);  // Упрощённый момент
        }
    }
    return loads;
}

bool check_constraints(const std::vector<double>& loads, const std::vector<double>& limits, double total_limit, double total_mass) {
    if (total_mass > total_limit) return false;
    for (size_t i = 0; i < loads.size(); ++i) {
        if (loads[i] > limits[i]) return false;
    }
    return true;
}

std::vector<Pos> simulated_annealing(const std::vector<double>& masses, double L, double W, double H, Pos target, const std::vector<double>& axle_limits, const std::vector<double>& initial_loads, const std::vector<AxlePos>& axle_positions, double total_limit, double threshold, double box_size) {
    auto positions = greedy_start(masses, L, W, H, target);
    if (positions.empty()) return {{-1, -1, -1}};  // -1 как маркер

    double current_error = calculate_error(positions, masses, target);
    auto best_positions = positions;
    double best_error = current_error;

    double T = 2000.0;
    double cooling_rate = 0.995;
    auto start_time = std::chrono::steady_clock::now();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, masses.size() - 1);

    while (T > 0.01) {
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time);
        if (duration.count() >= 300) break;  // 5 мин

        // Neighbor: swap two random
        size_t i = dist(gen), j = dist(gen);
        auto new_positions = positions;
        std::swap(new_positions[i], new_positions[j]);

        if (!valid_stacking(new_positions, masses, box_size)) continue;

        double new_error = calculate_error(new_positions, masses, target);
        auto new_loads_base = simulate_loads(new_positions, masses, axle_positions);
        std::vector<double> new_loads(new_loads_base.size());
        for (size_t k = 0; k < new_loads.size(); ++k) new_loads[k] = new_loads_base[k] + initial_loads[k];

        double total_mass = std::accumulate(masses.begin(), masses.end(), 0.0);
        if (!check_constraints(new_loads, axle_limits, total_limit, total_mass)) continue;

        if (new_error < current_error || std::exp(-(new_error - current_error) / T) > (static_cast<double>(rand()) / RAND_MAX)) {
            positions = new_positions;
            current_error = new_error;
        }

        if (current_error < best_error) {
            best_positions = positions;
            best_error = current_error;
        }

        T *= cooling_rate;
    }

    if (best_error > threshold) return {{-1, -1, -1}};
    return best_positions;
}