#include "stopwatch.h"
std::string to_string(Stopwatch &stopwatch) { return stopwatch.to_string(); }
std::string Stopwatch::to_string() const {
  auto end = std::chrono::high_resolution_clock::now();
  auto to_now = [&](auto start) {
    auto elapsed =
        std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(
            end - start);
    return std::to_string(elapsed.count()) + "ms";
  };
  auto s = "diff: " + to_now(prev_) + " total: " + to_now(start_);
  prev_ = end;
  return s;
}
Stopwatch::Stopwatch() : start_(std::chrono::high_resolution_clock::now()) {
  prev_ = start_;
}
std::chrono::duration<float, std::milli> Stopwatch::get_diff() const {
  auto end = std::chrono::high_resolution_clock::now();

  auto elapsed =
      std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(
          end - prev_);
  prev_ = end;
  return elapsed;
}
std::chrono::duration<float, std::milli> Stopwatch::get_total() const {
  auto end = std::chrono::high_resolution_clock::now();

  auto elapsed =
      std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(
          end - start_);
  return elapsed;
}
