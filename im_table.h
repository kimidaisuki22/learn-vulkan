#pragma once
#include <string>
#include <unordered_map>

class Im_table {

public:
  float &operator[](const std::string &s);
  void draw();
  void set_title(std::string title);

private:
  std::string title_;
  std::unordered_map<std::string, float> values_;
};