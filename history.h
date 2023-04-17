#pragma once
#include "imlist.h"
#include <functional>
#include <iostream>
#include <optional>
#include <sqlite_modern_cpp.h>
#include <string>
#include <vector>

class History {
public:
  History();
  std::vector<std::string> get();
  void add(std::string path);
  void draw();
  std::optional<std::string> last_loaded();
  Im_list<std::string, std::function<void(std::string)>> drawer;

  std::optional<std::string> query(const std::string &key);
  void insert(const std::string &key, const std::string &value);

private:
  sqlite::database db;
};