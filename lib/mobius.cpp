#include <algorithm>
#include <array>
#include <cfloat>
#include <cmath>
#include <iostream>
#include <iterator>
#include <ranges>
#include <vector>

constexpr float u_min = 0;
constexpr float u_max = 2 * std::numbers::pi;
constexpr float v_min = -1;
constexpr float v_max = 1;

constexpr unsigned u_steps = 100;
constexpr unsigned v_steps = 20;

struct Vec2 final {
  float u;
  float v;
};

struct Vec3 final {
  float x;
  float y;
  float z;
};

struct Face final {
#ifdef TRIAG
  std::array<unsigned, 3> Points;
#else
  std::array<unsigned, 4> Points;
#endif
};

struct Model final {
  std::vector<Vec3> Vertices;
  std::vector<Face> Faces;
};

void printModel(const Model &M) {
  auto &OS = std::cout;
  OS << "ply\n";
  OS << "format ascii 1.0\n";
  OS << "element vertex " << M.Vertices.size() << '\n';
  OS << "property float x\n";
  OS << "property float y\n";
  OS << "property float z\n";
#ifndef WITHOUT_FACES
  OS << "element face " << M.Faces.size() << '\n';
  OS << "property list uchar int vertex_indices\n";
#endif
  OS << "end_header\n";
  for (auto &V : M.Vertices)
    OS << V.x << ' ' << V.y << ' ' << V.z << '\n';
#ifndef WITHOUT_FACES
  for (auto &F : M.Faces) {
    OS << F.Points.size();
    for (auto P : F.Points)
      OS << ' ' << P;
    OS << '\n';
  }
#endif
}

int main() {
  std::vector<Vec2> Params;
  for (auto v_prim : std::views::iota(0u, v_steps + 1)) {
    for (auto u_prim : std::views::iota(0u, u_steps)) {
      Params.push_back(
          {static_cast<float>(u_min + u_prim * (u_max - u_min) / (u_steps - 1)),
           static_cast<float>(v_min + v_prim * (v_max - v_min) / (v_steps))});
    }
  }
#if 0
  for (auto [u, v] : Params) {
    std::cerr << "u=" << u << ", v=" << v << '\n';
  }
#endif

  Model M;

  for (auto strip : std::views::iota(0u, v_steps / 2)) {
    for (auto col : std::views::iota(0u, u_steps - 1)) {
      auto P1 = u_steps * strip + col;
      auto P2 = u_steps * strip + col + 1;
      auto P3 = u_steps * (strip + 1) + col + 1;
      auto P4 = u_steps * (strip + 1) + col;
#ifdef TRIAG
      M.Faces.push_back({P1, P2, P3});
      M.Faces.push_back({P1, P3, P4});
#else
      M.Faces.push_back({P1, P2, P3, P4});
#endif
    }
    {
      auto P1 = u_steps * (strip + 1) - 1;
      auto P2 = u_steps * (v_steps - strip - 1);
      auto P3 = u_steps * (v_steps - strip - 2);
      auto P4 = u_steps * (strip + 2) - 1;
#ifdef TRIAG
      M.Faces.push_back({P1, P2, P3});
      M.Faces.push_back({P1, P3, P4});
#else
      M.Faces.push_back({P1, P2, P3, P4});
#endif
    }
    for (auto col : std::views::iota(0u, u_steps - 1)) {
      auto P1 = u_steps * (v_steps - strip) + col;
      auto P2 = u_steps * (v_steps - strip) + col + 1;
      auto P3 = u_steps * (v_steps - strip - 1) + col + 1;
      auto P4 = u_steps * (v_steps - strip - 1) + col;
#ifdef TRIAG
      M.Faces.push_back({P1, P2, P3});
      M.Faces.push_back({P1, P3, P4});
#else
      M.Faces.push_back({P4, P1, P2, P3});
#endif
    }
    {
      auto P1 = u_steps * (v_steps - strip) - 1;
      auto P2 = u_steps * strip;
      auto P3 = u_steps * (strip + 1);
      auto P4 = u_steps * (v_steps - strip - 1) - 1;
#ifdef TRIAG
      M.Faces.push_back({P1, P2, P3});
      M.Faces.push_back({P1, P3, P4});
#else
      M.Faces.push_back({P1, P2, P3, P4});
#endif
    }
  }

  if (v_steps % 2) {
    auto strip = v_steps / 2 + 1;
    for (auto col : std::views::iota(0u, u_steps - 1)) {
      auto P1 = u_steps * (strip) + col;
      auto P2 = u_steps * (strip) + col + 1;
      auto P3 = u_steps * (strip - 1) + col + 1;
      auto P4 = u_steps * (strip - 1) + col;
#ifdef TRIAG
      M.Faces.push_back({P1, P2, P3});
      M.Faces.push_back({P1, P3, P4});
#else
      M.Faces.push_back({P4, P1, P2, P3});
#endif
    }
    auto P1 = u_steps * (v_steps - strip + 1) - 1;
    auto P2 = u_steps * (strip + 1) - 1;
    auto P3 = u_steps * strip;
    auto P4 = u_steps * (v_steps - strip);
#ifdef TRIAG
    M.Faces.push_back({P1, P2, P3});
    M.Faces.push_back({P1, P3, P4});
#else
    M.Faces.push_back({P1, P2, P3, P4});
#endif
  }

  auto &&R = Params | std::views::transform([](Vec2 &Param) {
               auto [u, v] = Param;
               float x = (1 + (v / 2) * std::cos(u / 2)) * std::cos(u);
               float y = (1 + (v / 2) * std::cos(u / 2)) * std::sin(u);
               float z = (v / 2) * std::sin(u / 2);
               return Vec3{x, y, z};
             });
  std::ranges::copy(R, std::back_inserter(M.Vertices));

  printModel(M);
}
