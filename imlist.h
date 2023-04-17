#pragma once

#include <imgui.h>
#include <string>
#include <type_traits>
#include <vector>

template <typename T>
concept Update_able = requires(T t) {
                        t.frame_begin();
                        t.frame_end();
                      };

// class Drawer_base
// class Widget_base
// class Window_base
// class Action_base

class Drawer_base {
    public:
  void frame_begin() {}
  void frame_end() {}
  template <typename T> bool operator()(T &t) {
    if constexpr (std::is_convertible_v<T, std::string>)
      return ImGui::Button(static_cast<std::string &&>(t).c_str());
    else if (requires { std::to_string(t); }) {
      return ImGui::Button(std::to_string(t).c_str());
    } else {
      // static_assert(false, "please impl it.");
    }
  }
};
class Action_base {
    public:
  void frame_begin() {}
  void frame_end() {}
  template <typename T> void operator()(T &t) {
    // static_assert(false, "please override this function.");
  }
};

template <typename T, typename Action, Update_able Drawer = Drawer_base> class Im_list {
public:
  void draw() {

    if (ImGui::TreeNode(title_.c_str())) {
      for (auto &v : elems_) {
        if (draw_(v)) {
          action_(v);
        }
      }

      ImGui::TreePop();
    }
  }
  void frame_begin() {
    if constexpr (Update_able<Action>) {
      action_.frame_begin();
    }
    draw_.frame_begin();
  }
  void frame_end() {
    if constexpr (Update_able<Action>) {
      action_.frame_end();
    }
    draw_.frame_end();
  }

  Action action_;
  Drawer draw_;
  std::string title_;
  std::vector<T> elems_;
};