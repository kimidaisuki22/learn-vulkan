#pragma once
#define _USE_MATH_DEFINES
#include <OpenMesh/Core/Mesh/PolyConnectivity.hh>
#include <OpenMesh/Core/System/config.h>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <string>
#include <tuple>
#include "OMesh_I.h"
#include "OpenMesh/Core/Mesh/Handles.hh"
#include "fixingStatistics.h"
#include "stopwatch.h"
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <functional>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "vertex.h"

typedef OpenMesh::TriMesh_ArrayKernelT<> MyMesh;

class OMesh:public OMesh_I {
public:
  bool read(std::string file_name);

  void fill_hole();
  std::tuple<std::vector<Vertex>, std::vector<uint>> export_data_vi();
  bool export_to_obj(std::string name) override;
  void load(std::vector<Vertex> p, std::vector<glm::uint> idx);

  void reset();
  void print_point(auto &p) {
    for (int i = 0; i < p.size(); i++) {
      std::cout << p.data()[i] << " ";
    }
  }
  auto print_from_to(auto &&e) {
    std::cout << "";
    std::cout << "from: ";
    print_point(mesh_.point(e.from()));
    std::cout << "\nto: ";
    print_point(mesh_.point(e.to()));
    std::cout << "\n";
  };
  virtual FixingStatistics check() override;

  std::vector<Vertex> export_data();
  void export_data(std::vector<Vertex> &points, std::vector<uint> &idx);

  auto next_triangle(auto &e) { return e.next().next().opp(); }

private:
  typedef OpenMesh::TriMesh_ArrayKernelT<> MyMesh;

  MyMesh mesh_;
  Stopwatch stopwatch_;
};