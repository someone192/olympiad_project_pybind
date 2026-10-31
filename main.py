import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import my_module

# Формируем masses (исправлено на 10.0)
masses_1 = [2000.0] * 2341
masses_2 = [10.0] * 22657  # Исправлено с 100.0 на 10.0
masses_3 = [100.0] * 1545
masses_4 = [1.0] * 3546
masses_5 = [43.6] * 15000
masses = masses_1 + masses_2 + masses_3 + masses_4 + masses_5

# Параметры грузовика
L, W, H = 100.0, 25.0, 20.0
target = (50.0, 12.5, 0)
axle_limits = [100000000000000.0] * 4
initial_loads = [0.0] * 4
axle_positions = [(0.0, 0.0), (100.0, 0.0), (0.0, 25), (10.0, 25)]
total_limit = 100000000000000000000.0
threshold = 100.0
box_size = 1.0

# Вызов
try:
    result = my_module.anneal_3d(
        masses, L, W, H, target, 
        axle_limits, initial_loads, axle_positions, 
        total_limit, threshold, box_size
    )
    print("Результат вызова (первые 10 позиций):")
    if len(result) == 1 and result[0] == (-1.0, -1.0, -1.0):
        print("Ошибка: решение не найдено.")
    else:
        for i, pos in enumerate(result[:10]):
            print(f"Коробка {i+1}: {pos}")
        # Взвешенный центр (исправлено)
        total_mass = sum(masses)
        xc = sum(masses[i] * result[i][0] for i in range(len(result))) / total_mass
        yc = sum(masses[i] * result[i][1] for i in range(len(result))) / total_mass
        zc = sum(masses[i] * result[i][2] for i in range(len(result))) / total_mass
        print(f"\nЦентр тяжести: ({xc:.2f}, {yc:.2f}, {zc:.2f})")
        print(f"Целевая точка: {target}")
        print(f"Общая масса: {sum(masses)} кг (в пределах {total_limit} кг)")
except Exception as e:
    print(f"Ошибка при вызове: {e}")
