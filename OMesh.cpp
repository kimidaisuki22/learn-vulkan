#include "OMesh.h"
#include "imLogger.h"
#include "vertex.h"
#include <fmt/format.h>
#include <glm/geometric.hpp>
#include <memory>
#include <string>
#include <vector>


bool are_points_coplanar(const std::vector<glm::vec3>& points) {
    if (points.size() < 4) {
        // Not enough points to define a plane
        return true;
    }

    // Select three non-collinear points
    glm::vec3 p1 = points[0];
    glm::vec3 p2, p3;
    for (int i = 1; i < points.size(); i++) {
        p2 = points[i];
        if (glm::length(p2 - p1) > 1e-6f) {
            break;
        }
    }
    for (int i = 1; i < points.size(); i++) {
        p3 = points[i];
        if (glm::length(p3 - p1) > 1e-6f && glm::length(p3 - p2) > 1e-6f) {
            break;
        }
    }

    // Compute normal vector of plane
    glm::vec3 n = glm::normalize(glm::cross(p2 - p1, p3 - p1));

    // Check if all other points lie on same plane
    for (int i = 0; i < points.size(); i++) {
        glm::vec3 v = points[i] - p1;
        if (glm::dot(v, n) > 1e-6f) {
            // Point does not lie on plane
            return false;
        }
    }

    // All points lie on same plane
    return true;
}
  auto local_convex (glm::vec3 p1, glm::vec3 p2, glm::vec3 p3,
                           glm::vec3 normal) {
      auto v1 = p2 - p1;
      auto v2 = p3 - p2;

      auto sin_v = dot(cross(v1, v2), normal);
      std::cout << "sinv: " << sin_v << "\n";
      return sin_v < 0;
    };

void OMesh::fill_hole() {
  struct P {
    glm::vec3 p;
    glm::vec3 n;
    OpenMesh::SmartVertexHandle o_v;
  };

  mesh_.request_face_status();
  mesh_.request_vertex_status();
  mesh_.request_vertex_texcoords2D();

  mesh_.request_vertex_normals();
  mesh_.request_face_normals();
  mesh_.request_face_colors();
  mesh_.request_face_texture_index();
  auto e1 = mesh_.halfedges_begin();

  auto points_loop =
      [this](const std::vector<OpenMesh::SmartHalfedgeHandle> &edges) {
        std::vector<P> points;
        for (auto &e : edges) {
          auto n = mesh_.calc_normal(e.opp().face());
          float *f = mesh_.point(e.to()).data();
          points.emplace_back(P{glm::vec3{f[0], f[1], f[2]},
                                glm::vec3{n[0], n[1], n[2]}, e.to()});
        }
        return points;
      };
  auto task_loop = [this](std::vector<P> points) {
    // points.push_back(points[1]);
    points.pop_back();
    std::rotate(points.begin(), points.begin() + 0, points.end());
    auto local_convex = [](glm::vec3 p1, glm::vec3 p2, glm::vec3 p3,
                           glm::vec3 normal) {
      auto v1 = p2 - p1;
      auto v2 = p3 - p2;

      auto sin_v = dot(cross(v1, v2), normal);
      std::cout << "sinv: " << sin_v << "\n";
      return sin_v < 0;
    };
    bool done = false;
    while (!done) {
      int i = 0;
      for (; i + 2 < points.size(); i++) {
        if (local_convex(points[i].p, points[i + 1].p, points[i + 2].p,
                         points[i].n)) {
          std::cout << "It's a convex!\n";
          int center = i + 1;
          // solve
          OpenMesh::SmartVertexHandle p_prev_2, p_next_2;
          if (i == 0) {
            p_prev_2 = points.back().o_v;
          } else {
            p_prev_2 = points[i - 1].o_v;
          }
          if (i + 3 >= points.size()) {
            p_next_2 = points[0].o_v;

          } else {
            p_next_2 = points[i + 3].o_v;
          }

          mesh_.add_face(p_prev_2, points[i].o_v, points[center].o_v);
          mesh_.add_face(points[center].o_v, points[center + 1].o_v, p_next_2);

          points.erase(points.begin() + center + 1);
          points.erase(points.begin() + center - 1);

          std::cout << "new loop: \n";

          for (auto p : points) {
            std::cout << p.p.x << " " << p.p.y << " " << p.p.z << "\n";
          }

          break;
        }
      }
      if (i + 3 >= points.size()) {
        done = true;
      }
    }

    for (int j = 1; j + 1 < points.size(); j++) {
      mesh_.add_face(points[0].o_v, points[j].o_v, points[j + 1].o_v);
    }
  };

  std::vector<std::vector<OpenMesh::SmartHalfedgeHandle>> edges_groups;
  std::unordered_set<int> seeked;
  for (auto e : mesh_.halfedges()) {
    if (e.is_boundary()) {
      std::vector<OpenMesh::SmartHalfedgeHandle> group;
      if (seeked.count(e.idx())) {
        continue;
      }
      std::cout << "new bound: \n";
      print_from_to(e);
      std::cout << "\n";
      auto src = e;
      seeked.insert(e.idx());
      group.push_back(e);
      e = e.next();
      seeked.insert(e.idx());
      group.push_back(e);
      while (e != src) {

        while (!e.is_boundary()) {
          e = e.opp().next();
        }
        e = e.next();
        group.push_back(e);
        seeked.insert(e.idx());
      }
      edges_groups.push_back(group);

      // if(e.next().is_boundary() && e.next().next().is_boundary()){
      //   //remove this facet.
      //   mesh.delete_face(e.face());
      //   std::cout <<"delete a face.";
      // }
    }
  }

  std::cout << "total hole group: " << edges_groups.size() << "\n";
  for (int i = 0; i < edges_groups.size(); i++) {
    std::cout << "group " << i << "\n";
    for (auto v : edges_groups[i]) {
      print_from_to(v);
    }
    std::cout << "Loop: \n";
    auto loop = points_loop(edges_groups[i]);
    for (auto p : loop) {
      std::cout << p.p.x << " " << p.p.y << " " << p.p.z << "\n";
    }
    {
      std::vector<glm::vec3> ps ;
      for(auto p:loop){
        ps.push_back(p.p);
      }
      if(are_points_coplanar(ps)){
        bool not_delete_this = true;

          // if(loop.size()>=3){
          //   auto p1 = loop[0].p;
          //   auto p2 = loop[1].p;
          //   auto p3 = loop[2].p;
          //   auto n = loop[0].n;
          //   // loop[0].o_v.faces().begin();
          // }
        if(!not_delete_this){

        for(auto e:edges_groups[i])
          mesh_.delete_edge(e.edge());

        continue;
        }
      }
    }

    task_loop(loop);
  }


  mesh_.request_face_status();
  mesh_.request_edge_status();
  mesh_.request_halfedge_status();
  mesh_.request_vertex_status();
  mesh_.garbage_collection();

  logger.add(fmt::format("fill hole finished, {}\n", stopwatch_.to_string()));
}
void OMesh::load(std::vector<Vertex> p, std::vector<glm::uint> idx) {
  reset();
  // std::vector<OpenMesh::SmartVertexHandle> vs;
  // for (auto v : p) {
  //   vs.push_back(mesh_.add_vertex({v[0], v[1], v[2]}));
  // }
  //   for (uint32_t i = 0; i + 3 <= idx.size(); i+=3) {
  //   mesh_.add_face(vs[idx[i]], vs[idx[i+1]], vs[idx[i+2]]);
  // }


 // TODO: FIX it.
  // auto vv = [&](auto i) {
  //   return mesh_.add_vertex({p[i][0], p[i][1], p[i][2]});
  // };

  // for (uint32_t i = 0; i + 3 <= idx.size(); i += 3) {
  //   mesh_.add_face(vv(idx[i]), vv(idx[i + 1]), vv(idx[i + 2]));
  // }
}
bool OMesh::read(std::string file_name) {
  reset();

  mesh_.request_face_status();
  mesh_.request_vertex_status();
  mesh_.request_vertex_texcoords2D();

  mesh_.request_vertex_normals();
  mesh_.request_face_normals();
  mesh_.request_face_colors();
  mesh_.request_face_texture_index();

  logger.add(fmt::format("start read obj: {}\n", file_name));

  if (!OpenMesh::IO::read_mesh(mesh_, file_name)) {
    std::cerr << "Failed to read mesh file: "
              << "mesh.obj" << std::endl;
    return false;
  }
  mesh_.triangulate();
  logger.add(fmt::format("read obj finished, {}\n", stopwatch_.to_string()));
  return true;
}
void OMesh::reset() {
  mesh_ = {};
  mesh_.clear();
}
std::vector<Vertex> OMesh::export_data() {
  std::vector<Vertex> points;
  auto all_face = mesh_.all_faces();

  for (auto &f : all_face) {

    auto vs = f.vertices_ccw();
    // auto n = mesh_.normal(f);
    for (auto v : vs) {
      auto &&p = mesh_.point(v);

      auto && t = mesh_.texcoord2D(v);
      Vertex vertex;
      vertex.pos_ = {p[0], p[1], p[2]};
      // vertex.normal_ = {n[0], n[1], n[2]};
      vertex.tex_coord_ = {t[0],t[1]};

      points.emplace_back(vertex);
    }
    auto& p1 = points[points.size()-3];
    auto& p2 = points[points.size()-2];
    auto& p3 = points[points.size()-1];

    auto n = glm::cross(p2.pos_-p1.pos_, p3.pos_ - p2.pos_);
    n = glm::normalize(n);
    p1.normal_ = n;
    p2.normal_ = n;
    p3.normal_ = n;

  }
  return points;
}
void OMesh::export_data(std::vector<Vertex> &points,
                        std::vector<uint> &idx) {
  assert(points.empty());
  assert(idx.empty());
  points.clear();
  idx.clear();
  // v1
  if (false) {
    uint max_idx{};

    auto p_start = mesh_.points();
    auto all_face = mesh_.all_faces();
    if (all_face.begin() == all_face.end()) {
      return;
    }
    for (auto &f : all_face) {
      auto vs = f.vertices_ccw();
      for (auto v : vs) {
        auto &&p = mesh_.point(v);
        mesh_.normal(v);
        auto id = &p - p_start;
        max_idx = std::max<uint>(max_idx, &p - p_start);
        idx.push_back(id);
      }
    }

    points.resize(max_idx + 1);
    for (uint i{}; i <= max_idx; i++) {
      // TODO: add normal.
      Vertex v;
      v.pos_ = {p_start[i][0], p_start[i][1], p_start[i][2]};

      points[i] = v;
    }
  }
  // v2
  if (true) {
    auto ps = export_data();
    for (auto p : ps) {
      auto pos = std::find(points.begin(), points.end(), p);
      uint32_t index = {};
      if (pos == points.end()) {
        index = points.size();
        points.push_back(p);
      } else {
        index = pos - points.begin();
      }
      idx.push_back(index);
    }
  }
}
std::tuple<std::vector<Vertex>, std::vector<uint>> OMesh::export_data_vi() {
  std::vector<Vertex> p;
  std::vector<uint> i;
  export_data(p, i);
  return std::make_tuple(p, i);
}
bool OMesh::export_to_obj(std::string name) {
  if (!OpenMesh::IO::write_mesh(mesh_, name)) {
    std::cerr << "Failed to write mesh file:" << name << std::endl;
    return false;
  }
  return true;
}

std::shared_ptr<OMesh_I> create_OMesh_I() { return std::make_shared<OMesh>(); }
FixingStatistics OMesh::check() {
  return {};
}
