#pragma once
#include <string>
#include <vector>

class ImLogger{
public:
  ImLogger();

  void reset();

  void add(std::string str);

  void draw();

  std::string title_;
  std::string buffer_;
  std::vector<int> offsets_;
  bool open_;
  bool auto_scroll_;
};

extern ImLogger logger;