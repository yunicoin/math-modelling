/**
 * @file methods/main.cpp
 * @author Mikhail Lozhnikov
 *
 * Файл с функией main() для серверной части программы.
 */

#include <httplib.h>
#include <iostream>
#include <cstdio>
#include <string>
#include <nlohmann/json.hpp>
#include "tasks_queue.hpp"
#include "methods.hpp"

using json = nlohmann::json;

mm::TasksQueue tasksQueue;

int main(int argc, char* argv[]) {
  // Порт по-умолчанию.
  int port = 8080;

  if (argc >= 2) {
    // Меняем порт по умолчанию, если предоставлен соответствующий
    // аргумент командной строки.
    if (std::sscanf(argv[1], "%d", &port) != 1)
      return -1;
  }

  std::cerr << "Listening on port " << port << "..." << std::endl;

  httplib::Server svr;


  // Обработчик для GET запроса по адресу /stop. Этот обработчик
  // останавливает сервер.
  svr.Get("/stop", [&](const httplib::Request&, httplib::Response&) {
    svr.stop();
  });


  svr.Post("/CheckTaskStatus", [&](const httplib::Request& req,
                                        httplib::Response& res) {
    /*
    Поле body структуры httplib::Request содержит текст запроса.
    Функция nlohmann::json::parse() используется для того,
    чтобы преобразовать текст в объект типа nlohmann::json.
    */
    nlohmann::json input = nlohmann::json::parse(req.body);
    nlohmann::json output;

    int taskId = input["id"];

    output["id"] = taskId;

    if (tasksQueue.IsTaskFinished(taskId)) {
      // Задача завершена можно скачивать данные.
      output["status"] = "finished";
    } else {
      /* Задача либо не была добавлена, либо она ещё не досчиталась,
       * либо она уже посчитана, и данные уже скачаны и удалены с сервера. */
      output["status"] = "unknown";
    }

    /*
    Метод nlohmann::json::dump() используется для сериализации
    объекта типа nlohmann::json в строку. Метод set_content()
    позволяет задать содержимое ответа на запрос. Если передаются
    JSON данные, то MIME тип следует выставить application/json.
    */
    res.set_content(output.dump(), "application/json");
  });


  svr.Post("/DownloadTaskData", [&](const httplib::Request& req,
                                        httplib::Response& res) {
    /*
    Поле body структуры httplib::Request содержит текст запроса.
    Функция nlohmann::json::parse() используется для того,
    чтобы преобразовать текст в объект типа nlohmann::json.
    */
    nlohmann::json input = nlohmann::json::parse(req.body);
    nlohmann::json output;

    int taskId = input["id"];

    if (tasksQueue.IsTaskFinished(taskId)) {
      // Задача завершена можно скачивать данные.
      output = tasksQueue.GetFinishedTaskData(taskId);
    } else {
      /* Задача либо не была добавлена, либо она ещё не досчиталась,
       * либо она уже посчитана, и данные уже скачаны и удалены с сервера. */
      output["status"] = "unknown";
    }

    output["id"] = taskId;

    /*
    Метод nlohmann::json::dump() используется для сериализации
    объекта типа nlohmann::json в строку. Метод set_content()
    позволяет задать содержимое ответа на запрос. Если передаются
    JSON данные, то MIME тип следует выставить application/json.
    */
    res.set_content(output.dump(), "application/json");
  });




  /* Сюда нужно вставить обработчик post запроса для алгоритма. */

svr.Post("/HeatEquation", [&](const httplib::Request& req,
                            httplib::Response& res) {
  nlohmann::json input = nlohmann::json::parse(req.body);
  nlohmann::json output;
  int taskId = mm::HeatEquationMethod(input, &output);
  output["id"] = taskId;
  res.set_content(output.dump(), "application/json");
});


  /* Конец вставки. */

  // Эта функция запускает сервер на указанном порту. Программа не завершится
  // до тех пор, пока сервер не будет остановлен.
  svr.listen("0.0.0.0", port);

  return 0;
}
