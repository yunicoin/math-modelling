/**
 * @file methods/equation_method.cpp
 * @author Энгельгардт М
 *
 * @brief Серверный метод для задачи теплопроводности.
 *
 * Извлекает параметры из JSON, создаёт решалку, ставит её в очередь задач
 * и возвращает клиенту идентификатор задачи.
 */

#include <nlohmann/json.hpp>            // работа с JSON
#include "abstract_solver_wrapper.hpp"  // обёртки Float/DoubleAbstractSolverWrapper
#include "quation_solver.hpp"     // наш класс EquationSolver
#include "tasks_queue.hpp"              // класс очереди задач
#include "methods.hpp"

// Глобальная очередь, объявленная в methods/main.cpp
extern mm::TasksQueue tasksQueue;

int mm::EquationMethod(const nlohmann::json& input,
                           nlohmann::json* output) {
  // 1) Достаём параметры. Если какого-то ключа нет, метод
  // at() кинет исключение.
  size_t M = input.at("M").get<size_t>();
  double tau = input.at("tau").get<double>();
  double finishTime = input.at("finishTime").get<double>();
  double exportPeriod = input.at("exportPeriod").get<double>();

  // 2) Создаём решалку
  auto* solver = new mm::EquationSolver<double>(M, tau,
                                                    finishTime, exportPeriod);

  // 3) Оборачиваем в DoubleAbstractSolverWrapper.
  auto* wrapper = new mm::DoubleAbstractSolverWrapper(solver);

  // 4. Ставим задачу в очередь, получаем её уникальный идентификатор.
  int taskId = tasksQueue.AddTask(wrapper);

  // 5. Возвращаем id клиенту.
  (*output)["id"] = taskId;
  return taskId;
}
