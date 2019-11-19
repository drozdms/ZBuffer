#ifndef __MODEL_HPP__
#define __MODEL_HPP__

#include <string>
#include <vector>
#include <Eigen/Eigen>
#include <stdint.h>
#include "tiny_obj_loader.h"
#include "Logger.hpp"

struct EigenTypes {
  typedef Eigen::Vector3d Vector3;
  typedef Eigen::Vector4d Vector4;
  typedef Eigen::Matrix3d Matrix3;
  typedef Eigen::Matrix4d Matrix4;
};

struct Pixel : public EigenTypes {
  int x, y;  /// image coordinates
  Vector3 t; /// barycentric coordinates

  Pixel(int x, int y, Vector3 t)
    : x(x), y(y), t(t)
  {}
};

struct Triangle : public EigenTypes
{
  Vector3 vertices[3];
  Vector3 normals[3];

  void raster(std::vector<Pixel> &pixels, int w, int h) const;
  float getDepth(const Pixel &p) const;
  uint32_t getColor(const Pixel &p) const;
};

class Model : public EigenTypes {
public:

public:
  Model(const char *filename);
  ~Model();

public:
  void debug() const;
  size_t numShapes() const;
  size_t vertexSize(size_t i) const;
  size_t normalSize(size_t i) const;
  size_t indexSize(size_t i) const;
  size_t trianglesNumber() const;
  void *vertexData(size_t i);
  void *normalData(size_t i);
  void *indexData(size_t i);

  void getTriangles(std::vector<Triangle> &triangles, const Matrix4 &transform);

protected:
  /** \brief Calculate normals for each vertex.
   */
  void calculate_normal(size_t idx);

protected:
  std::string m_filename;
  std::vector<tinyobj::shape_t> m_shapes;

};

#endif // __MODEL_HPP__
