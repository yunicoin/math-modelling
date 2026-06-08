/**
 * @file tests/equation_test.cpp
 * @author Энгельгардт М
 *
 * @brief Тесты для решателя уравнения теплопроводности.
 *
 * Проверяют:
 * - соблюдение граничных условий после одного шага,
 * - отсутствие NaN/Inf при длительном счёте,
 * - выполнение принципа максимума.
 */

#include <cmath>
#include <nlohmann/json.hpp>
#include "equation_solver.hpp"
#include "test_core.hpp"

/**
 * @brief Проверка граничных условий после одного шага.
 *
 * Запускаем решатель с маленьким tau (устойчивая схема)
 * ровно на один шаг и проверяем значения в нескольких точках.
 */
static void TestBoundaryConditions() {
  const size_t M = 10;              // 10 разбиений на единицу
  const double h = 1.0 / M;         // шаг сетки
  const double tau = h * h / 4.0;   // шаг по времени (условие устойчивости)
  const double finishTime = tau;    // остановка после первого шага

  mm::EquationSolver<double> solver(M, tau, finishTime, tau);
  nlohmann::json result;
  bool ok = solver.Solve(&result);
  REQUIRE(ok);  // Solve должен вернуть true

  // В результате Solve сохраняет данные в моменты времени,
  // кратные exportPeriod. При tau = exportPeriod будет один экспорт.
  auto grid = result["data"][0]["data"]["grid"];

  // Проверяем несколько характерных точек на границе
  REQUIRE_CLOSE(grid[0][0].get<double>(), 1.0, 1e-10);            // (0,0) низ
  REQUIRE_CLOSE(grid[0][3 * M].get<double>(), 1.0, 1e-10);        // (3,0) низ
  REQUIRE_CLOSE(grid[3 * M][0].get<double>(), 3.0, 1e-10);        // (0,3) верх
  REQUIRE_CLOSE(grid[3 * M][M].get<double>(), 3.0, 1e-10);        // (1,3) верх
  // Правая нижняя граница: x=3, y=0.5 -> u = 1 + 0.5 = 1.5
  int i_half = static_cast<int>(0.5 * M + 0.5);  // округление до ближайшего индекса
  REQUIRE_CLOSE(grid[i_half][2 * M].get<double>(), 1.5, 1e-10);
}

/**
 * @brief Проверка устойчивости: отсутствие NaN и бесконечностей.
 *
 * Считаем до 0.1 с (много шагов) и сканируем все значения.
 */
static void TestStability() {
  const size_t M = 8;
  const double h = 1.0 / M;
  const double tau = h * h / 4.0;
  const double finishTime = 0.1;

  mm::EquationSolver<double> solver(M, tau, finishTime,
                                        finishTime * 2);
  nlohmann::json result;
  bool ok = solver.Solve(&result);
  REQUIRE(ok);

  // Перебираем все сохранённые временные слои
  for (auto& frame : result["data"]) {
    auto grid = frame["data"]["grid"];
    for (auto& row : grid) {
      for (auto& val : row) {
        if (!val.is_null()) {
          double v = val.get<double>();
          REQUIRE(!std::isnan(v));   // не NaN
          REQUIRE(!std::isinf(v));   // не бесконечность
        }
      }
    }
  }
}

/**
 * @brief Проверка принципа максимума.
 *
 * При отсутствии внутренних источников температура не должна
 * выходить за пределы, заданные начальными и граничными условиями.
 * Максимальное граничное значение -- 4, минимальное -- 0.
 */
static void TestMaximumPrinciple() {
  const size_t M = 6;
  const double h = 1.0 / M;
  const double tau = h * h / 4.0;
  const double finishTime = 0.1;

  mm::HeatEquationSolver<double> solver(M, tau, finishTime,
                                        finishTime * 2);
  nlohmann::json result;
  bool ok = solver.Solve(&result);
  REQUIRE(ok);

  double max_val = -1e9;
  double min_val = 1e9;
  for (auto& frame : result["data"]) {
    auto grid = frame["data"]["grid"];
    for (auto& row : grid) {
      for (auto& val : row) {
        if (!val.is_null()) {
          double v = val.get<double>();
          if (v > max_val) max_val = v;
          if (v < min_val) min_val = v;
        }
      }
    }
  }
  // Максимальное граничное значение -- 4 (на верхней или правой вертикали),
  // минимальное -- 0 (начальное внутри). Плюс погрешность.
  REQUIRE(max_val <= 4.0 + 1e-10);
  REQUIRE(min_val >= 0.0 - 1e-10);
}

/**
 * @brief Главная тестовая функция, вызываемая из tests/main.cpp.
 */
void TestHeatEquation() {
  TestSuite suite("Heat Equation");
  RUN_TEST(suite, TestBoundaryConditions);
  RUN_TEST(suite, TestStability);
  RUN_TEST(suite, TestMaximumPrinciple);
}
