#include <Eigen/Dense>
#include "Model.hpp"
#include "Logger.hpp"
#include <algorithm>

Model::Model(const char *filename)
  : m_filename(filename)
{
  std::string err = tinyobj::LoadObj(m_shapes, filename);
  ASSERT_MSG(err.empty(), "%s", err.c_str());

  for ( size_t i=0; i < m_shapes.size(); i++ ) {
    if ( m_shapes[i].mesh.normals.empty() )
      calculate_normal(i);
  }
}

Model::~Model()
{
}

void Model::debug() const
{
  for ( size_t i=0; i < m_shapes.size(); i++ ) {
    const tinyobj::shape_t & shape = m_shapes[i];
    const std::vector<unsigned int> & indices = shape.mesh.indices;
    const std::vector<float> & positions = shape.mesh.positions;
    const std::vector<float> & normals = shape.mesh.normals;

    INFO("Shape %lu: %s", i, shape.name.c_str());
    ASSERT(indices.size() % 3 == 0);
    INFO("number of triangles = %lu", indices.size() / 3);
    INFO("number of vertices = %lu", positions.size() / 3);
    INFO("indices.size() = %lu", indices.size());
    INFO("positions.size() = %lu", positions.size());
    INFO("normals.size() = %lu", normals.size());
    INFO("min of index = %u", *(std::min_element(indices.begin(), indices.end())));
    INFO("max of index = %u", *(std::max_element(indices.begin(), indices.end())));
  }
}

size_t Model::numShapes() const
{
  return m_shapes.size();
}

size_t Model::vertexSize(size_t i) const
{
  return m_shapes[i].mesh.positions.size();
}

size_t Model::normalSize(size_t i) const
{
  return m_shapes[i].mesh.normals.size();
}

size_t Model::indexSize(size_t i) const
{
  return m_shapes[i].mesh.indices.size();
}

size_t Model::trianglesNumber() const
{
    size_t size = 0;
    for (int i= 0; i<m_shapes.size(); ++i)
        size += m_shapes[i].mesh.indices.size();
    return size;
}

void *Model::vertexData(size_t i)
{
  return (void*)&(m_shapes[i].mesh.positions[0]);
}

void *Model::normalData(size_t i)
{
  return (void*)&(m_shapes[i].mesh.normals[0]);
}

void *Model::indexData(size_t i)
{
  return (void*)&(m_shapes[i].mesh.indices[0]);
}

void Model::calculate_normal(size_t idx)
{
  // Index is assumed
  ASSERT(!m_shapes[idx].mesh.indices.empty());

  const std::vector<unsigned int> & indices = m_shapes[idx].mesh.indices;
  const std::vector<float> & positions = m_shapes[idx].mesh.positions;

  // To store normal for each vertex
  std::vector<Vector3> new_normals(positions.size()/3, Vector3::Zero());

  // For each face, calculate face normal, add it to each vertex of the face
  for ( size_t i=0; i < indices.size(); i += 3 )
  {
    Vector3 a(positions[3*indices[i+1]  ]-positions[3*indices[i]  ],
                      positions[3*indices[i+1]+1]-positions[3*indices[i]+1],
                      positions[3*indices[i+1]+2]-positions[3*indices[i]+2]);
    Vector3 b(positions[3*indices[i+2]  ]-positions[3*indices[i]  ],
                      positions[3*indices[i+2]+1]-positions[3*indices[i]+1],
                      positions[3*indices[i+2]+2]-positions[3*indices[i]+2]);
    Vector3 n = a.cross(b).normalized();
    new_normals[indices[i  ]] += n;
    new_normals[indices[i+1]] += n;
    new_normals[indices[i+2]] += n;
  }
  for ( size_t i=0; i < new_normals.size(); i++ )
  {
    new_normals[i].normalize();
  }

  std::vector<float> & normals = m_shapes[idx].mesh.normals;
  if ( !normals.empty() )
    WARN("Overwriting exisiting normals...");
  normals.resize(positions.size());

  for ( size_t i=0; i < indices.size(); i++ )
  {
    normals[3*indices[i]  ] = new_normals[indices[i]].x();
    normals[3*indices[i]+1] = new_normals[indices[i]].y();
    normals[3*indices[i]+2] = new_normals[indices[i]].z();
  }
}

void Model::getTriangles(std::vector<Triangle> &triangles, const Matrix4 &transform)
{
  //const Matrix4 normal_transform = (transform.transpose()*transform).inverse()*transform.transpose();
  const Matrix4 normal_transform = transform.adjoint().transpose();
  //const Matrix4 normal_transform = transform.inverse().transpose();
  //const Matrix4 &normal_transform = transform;

  float x[2] = {99999.f, -99999.f};
  float y[2] = {99999.f, -99999.f};
  float z[2] = {99999.f, -99999.f};

  size_t n_filtered = 0;
  size_t n_remained = 0;

  for ( size_t i=0; i < m_shapes.size(); i++ )
  {
    const std::vector<unsigned int> & indices = m_shapes[i].mesh.indices;
    const std::vector<float> & positions = m_shapes[i].mesh.positions;
    const std::vector<float> & normals = m_shapes[i].mesh.normals;

    for ( size_t j=0; j < indices.size(); j += 3 )
    {
      Triangle t;

      // do the transformation
      for ( size_t k=0; k < 3; k++ )
      {
        Vector4 v(positions[3*indices[j+k]], positions[3*indices[j+k]+1], positions[3*indices[j+k]+2], 1.0);
        v = transform * v;
        v /= v.w();
        t.vertices[k] = Vector3(v.x(), v.y(), v.z());
      }

      // do statistics about vertex info
      for ( size_t k=0; k < 3; k++ )
      {
        x[0] = std::min(x[0], (float)(t.vertices[k].x()));
        x[1] = std::max(x[1], (float)(t.vertices[k].x()));
        y[0] = std::min(y[0], (float)(t.vertices[k].y()));
        y[1] = std::max(y[1], (float)(t.vertices[k].y()));
        z[0] = std::min(z[0], (float)(t.vertices[k].z()));
        z[1] = std::max(z[1], (float)(t.vertices[k].z()));
      }

      // filter out this triangle if all three vertices are outside of the viewing volume
      if ( (t.vertices[0].x() < -1.0f || t.vertices[0].x() > 1.0f ||
            t.vertices[0].y() < -1.0f || t.vertices[0].y() > 1.0f ||
            t.vertices[0].z() < -1.0f || t.vertices[0].z() > 1.0f )
        && (t.vertices[1].x() < -1.0f || t.vertices[1].x() > 1.0f ||
            t.vertices[1].y() < -1.0f || t.vertices[1].y() > 1.0f ||
            t.vertices[1].z() < -1.0f || t.vertices[1].z() > 1.0f )
        && (t.vertices[2].x() < -1.0f || t.vertices[2].x() > 1.0f ||
            t.vertices[2].y() < -1.0f || t.vertices[2].y() > 1.0f ||
            t.vertices[2].z() < -1.0f || t.vertices[2].z() > 1.0f ) )
      {
        n_filtered++;
        continue;
      }

#if 1
      // transform the normals as well
      for ( size_t k=0; k < 3; k++ )
      {
        Vector4 n(normals[3*indices[j+k]], normals[3*indices[j+k]+1], normals[3*indices[j+k]+2], 1.0);
        n = normal_transform * n;
        t.normals[k] = Vector3(n.x()/n.w(), n.y()/n.w(), n.z()/n.w());
        t.normals[k].normalize();
      }
#endif

      // filter out this triangle if it's facing backward to viewer
      // TODO: find out better solution
      if (0)
      //if ( t.normals[0].z() < 0 || t.normals[1].z() < 0 || t.normals[2].z() < 0 )
      //if ( t.normals[0].z() + t.normals[1].z() + t.normals[2].z() < 0 )
      {
        n_filtered++;
        continue;
      }
      if (0)
      {
        Vector3 n = t.normals[0] + t.normals[1] + t.normals[2];
        n.normalize();

        INFO("normal = (%.2f, %.2f, %.2f)", n.x(), n.y(), n.z());
      }

      triangles.push_back(t);
      n_remained++;
    }
  }
  INFO("range of x (before clip): (%.2f, %.2f)", x[0], x[1]);
  INFO("range of y (before clip): (%.2f, %.2f)", y[0], y[1]);
  INFO("range of z (before clip): (%.2f, %.2f)", z[0], z[1]);
  INFO("filtered: %.2f%% (%lu/%lu)", 100.0f*n_filtered/(n_filtered+n_remained), n_filtered, n_filtered+n_remained);
}

void Triangle::raster(std::vector<Pixel> &pixels, int w, int h) const
{
  int x[2] = {99999, -99999};
  int y[2] = {99999, -99999};
  int xd[3];
  int yd[3];

  // find discrete coordinates of each vertex in 2D plane
  for ( size_t i=0; i < 3; i++ )
  {
    xd[i] = int((vertices[i].x() + 1.0f) / 2.0f * w);          // vertex float cooords are from -1 to 1
    yd[i] = int((vertices[i].y() + 1.0f) / 2.0f * h);
  }
  for ( size_t i=0; i < 3; i++ )
  {
    x[0] = std::min(x[0], xd[i]);
    x[1] = std::max(x[1], xd[i]);
    y[0] = std::min(y[0], yd[i]);
    y[1] = std::max(y[1], yd[i]);
  }

  // construct maxtrix A
  Matrix3 A(Matrix3::Ones());
  for ( size_t i=0; i < 3; i++ )
  {
    A(0, i) = xd[i];
    A(1, i) = yd[i];
  }
  A = A.inverse().eval();

  // find each pixel
  for ( int i=x[0]; i <= x[1]; i++ )
    for ( int j=y[0]; j <= y[1]; j++ )          // from min to max discrete coordinates
    {
      Vector3 b(i, j, 1.0);
      Vector3 t = A * b;                        //    barycentric coords

      if ( t.x() < 0 || t.y() < 0 || t.z() < 0 )
        continue;

      pixels.push_back(Pixel(i, j, t));
    }
}

float Triangle::getDepth(const Pixel &p) const
{
  const Vector3 &t = p.t;

  return t.x()*vertices[0].z()
       + t.y()*vertices[1].z()
       + t.z()*vertices[2].z();
}

uint32_t Triangle::getColor(const Pixel &p) const
{
#if 0
  int t = 255 * (0.5*getDepth(p)+0.5);
  return (0xff000000 | t << 16 | t << 8 | t);
#endif

  // define static material color
  const static float shininess = 15.0f;
  const static Vector3 diffuse(0.929524f, 0.796542f, 0.178823f);
  const static Vector3 specular(1.00000f, 0.980392f, 0.549020f);

  // calculate position and normal for p
  const Vector3 &t = p.t;
  Vector3 v = t.x()*vertices[0] + t.y()*vertices[1] + t.z()*vertices[2];
  Vector3 n = t.x()*normals[0] + t.y()*normals[1] + t.z()*normals[2];
  n.normalize();

  const Vector3 light_position = Vector3(0.0, 5.0, 0.0);

  // calculate light vector, view vector and half vector
  Vector3 li = (light_position-v).normalized();
  Vector3 vi = (-v).normalized();
  Vector3 h = (li+vi).normalized();

  // calculate color using phong model
  // https://en.wikipedia.org/wiki/Phong_reflection_model
  Vector3 color = 0.3*diffuse;
  if ( n.dot(li) > 0 )
    color += diffuse * (n.dot(li));
  if ( n.dot(h) >= 0 )
    color += specular * std::pow((double)n.dot(h), (double)shininess);
  color(0) = std::max(color(0), 0.0); color(0) = std::min(color(0), 1.0);
  color(1) = std::max(color(1), 0.0); color(1) = std::min(color(1), 1.0);
  color(2) = std::max(color(2), 0.0); color(2) = std::min(color(2), 1.0);

  int r = 255 * color.x();
  int g = 255 * color.y();
  int b = 255 * color.z();
  return (0xff000000 | r << 16 | g << 8 | b);
}
