#pragma once
#include <glm/ext/vector_float3.hpp>
#include <memory>
#include <vector>
#include "fixingStatistics.h"
#include "vertex.h"
#include <string>
class OMesh_I {
public:
  virtual bool read(std::string file_name) = 0;

  virtual void fill_hole() = 0;
  virtual std::tuple<std::vector<Vertex>, std::vector<glm::uint>>
  export_data_vi() = 0;
  virtual bool export_to_obj(std::string name) = 0;
  virtual void load(std::vector<Vertex> p, std::vector<glm::uint> idx) = 0;
  virtual FixingStatistics check()=0;
};

std::shared_ptr<OMesh_I> create_OMesh_I();