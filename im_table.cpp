#include "im_table.h"
#include <imgui.h>
#include <implot.h>
#include <math.h>
#include <string_view>
#include <unordered_map>

// utility structure for realtime plot
struct ScrollingBuffer {
  int MaxSize;
  int Offset;
  ImVector<ImVec2> Data;
  ScrollingBuffer(int max_size = 10000) {
    MaxSize = max_size;
    Offset = 0;
    Data.reserve(MaxSize);
  }
  void AddPoint(float x, float y) {
    if (Data.size() < MaxSize)
      Data.push_back(ImVec2(x, y));
    else {
      Data[Offset] = ImVec2(x, y);
      Offset = (Offset + 1) % MaxSize;
    }
  }
  void Erase() {
    if (Data.size() > 0) {
      Data.shrink(0);
      Offset = 0;
    }
  }
};

 
struct Im_real_time_plot {
  std::unordered_map<std::string_view, ScrollingBuffer> buffers_;
};
void f(Im_real_time_plot &table);
void Im_table::draw() {
  static Im_real_time_plot table;
  static float t = 0;
  t += ImGui::GetIO().DeltaTime;
  if (ImGui::TreeNode(title_.c_str())) {
    for (auto &[k, v] : values_) {
      ImGui::Text(k.c_str());
      ImGui::SameLine();
      ImGui::Text("%.3f ms", v);
      table.buffers_[k].AddPoint(t, v);
    }

    f(table);
    ImGui::TreePop();
  }
}

void f(Im_real_time_plot &table) {
  ImGui::BulletText("Move your mouse to change the data!");
  ImGui::BulletText(
      "This example assumes 60 FPS. Higher FPS requires larger buffer size.");

  static float t = 0;
  t += ImGui::GetIO().DeltaTime;
  static float history = 10.0f;
  ImGui::SliderFloat("History", &history, 1, 30, "%.1f s");

  static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;

  if (ImPlot::BeginPlot("##Scrolling", ImVec2(-1, 150))) {
    ImPlot::SetupAxes(NULL, NULL, flags, flags);
    ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always);
    ImPlot::SetupAxisLimits(ImAxis_Y1, 0, table.buffers_.begin()->second.Data[0].y+1);
    ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
    for (auto &[str, data] : table.buffers_) {
      ImPlot::PlotLine(str.data(), &data.Data[0].x, &data.Data[0].y,
                       data.Data.size(), 0, data.Offset, 2 * sizeof(float));
    }
    ImPlot::EndPlot();
  }
}

float &Im_table::operator[](const std::string &s) { return values_[s]; }
void Im_table::set_title(std::string title) { title_ = std::move(title); }
