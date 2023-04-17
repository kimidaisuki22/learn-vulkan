#pragma once
#include <chrono>
#include <ratio>
#include <string>

class Stopwatch {
public:
  using Time_point = std::chrono::high_resolution_clock::time_point;
  Stopwatch();

  std::string to_string() const;
  std::chrono::duration<float, std::milli> get_diff() const;
  std::chrono::duration<float, std::milli> get_total() const;

private:
  const Time_point start_;
  mutable Time_point prev_;
};

std::string to_string(Stopwatch &stopwatch);