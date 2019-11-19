#include <cmath>
#include <QPainter>
#include "ZBWidget.hpp"
#include "Logger.hpp"
#define M_PI 3.14159265
ZBWidget::ZBWidget(Model *model, QWidget *parent)
  : QWidget(parent),
    m_model(model),
    m_cameraAngleX(-90.0f),
    m_cameraAngleY(0.0f),
    m_cameraDistance(2.0f)
{
  connect(this, SIGNAL(repaintNeeded()), this, SLOT(update()));
}

ZBWidget::~ZBWidget()
{
}

// https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/lookat-function

ZBWidget::Matrix4 ZBWidget::lookAt(float eye_x, float eye_y, float eye_z,
                                   float center_x, float center_y, float center_z,
                                   float up_x, float up_y, float up_z)
{
  Vector3 eye(eye_x, eye_y, eye_z);
  Vector3 center(center_x, center_y, center_z);
  Vector3 up(up_x, up_y, up_z);
  Vector3 f = (center - eye).normalized();      // -forward
  Vector3 s = f.cross(up).normalized();         // right
  Vector3 u = s.cross(f).normalized();

  Matrix4 result(Matrix4::Identity());
  result(0, 0) = s.x();
  result(0, 1) = s.y();
  result(0, 2) = s.z();
  result(1, 0) = u.x();
  result(1, 1) = u.y();
  result(1, 2) = u.z();
  result(2, 0) = -f.x();
  result(2, 1) = -f.y();
  result(2, 2) = -f.z();
  result(0, 3) = -s.dot(eye);
  result(1, 3) = -u.dot(eye);
  result(2, 3) = f.dot(eye);
  return result;
}

ZBWidget::Matrix4 ZBWidget::perspective(float fov, float aspect, float near, float far)
{
  ASSERT(aspect!=0.0f);
  ASSERT(near!=far);

  // convert fov from degree to radians
  fov *= (M_PI / 180.0f);

  float tanHalfFov = std::tan(fov/2);

  Matrix4 result(Matrix4::Zero());
  result(0, 0) = 1.0f / (aspect * tanHalfFov);
  result(1, 1) = 1.0f / tanHalfFov;
  result(2, 2) = - (far + near) / (far - near);
  result(3, 2) = -1.0f;
  result(2, 3) = (-2.0f * far * near) / (far - near);
  return result;
}

ZBWidget::Matrix4 ZBWidget::rotateX(float degree)
{
  float radians = degree * (M_PI / 180.0f);
  float sinphi = std::sin(radians);
  float cosphi = std::cos(radians);

  Matrix4 result(Matrix4::Zero());
  result(0, 0) = 1.0f;
  result(1, 1) = cosphi;
  result(1, 2) = -sinphi;
  result(2, 1) = sinphi;
  result(2, 2) = cosphi;
  result(3, 3) = 1.0f;
  return result;
}

ZBWidget::Matrix4 ZBWidget::rotateY(float degree)
{
  float radians = degree * (M_PI / 180.0f);
  float sinphi = std::sin(radians);
  float cosphi = std::cos(radians);

  Matrix4 result(Matrix4::Zero());
  result(0, 0) = cosphi;
  result(0, 2) = sinphi;
  result(1, 1) = 1.0f;
  result(2, 0) = -sinphi;
  result(2, 2) = cosphi;
  result(3, 3) = 1.0f;
  return result;
}

void ZBWidget::paintEvent(QPaintEvent *event)
{
  ASSERT_MSG(m_model, "ZBWidget: failed to load model!");
  QPainter painter(this);

  int width = this->width();
  int height = this->height();

  QImage img(width, height, QImage::Format_ARGB32);
  img.fill(Qt::darkGray);

  Eigen::MatrixXd zbuffer(width, height);
  zbuffer.fill(1.0);

  std::vector<QRgb*> img_data(height);
  for ( int i=0; i < height; i++ )
  {
    img_data[i] = (QRgb*)img.scanLine(i);
  }

#if 0
  for ( int i=0; i < height; i++ )
    for ( int j=0; j < width; j++ )
    {
      double d = std::sqrt((i-height/2)*(i-height/2)+(j-width/2)*(j-width/2));
      if ( d < std::min(height/2, width/2) )
      {
        img_data[i][j] = 0xffff0000;
      }
    }
#endif

#if 1
  /* This is how our algorithms work:
   * 0. Setup model matrix
   * 1. Setup view matrix (camera)
   * 2. Setup projection matrix
   * 3. Find out all triangles in the viewing frustum
   * 4. Filter out all triangles facing backward to camera
   * 5. Rasterize each triangle into pixels
   * 6. Set each pixel of the image to its nearest triangle pixel's color
   */
  Matrix4 transform(Matrix4::Identity());
  transform *= perspective(60.0f, (float)width/height, 1.0f, 1000.0f);
  transform *= lookAt(0.0f, 0.0f, m_cameraDistance, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
  transform *= rotateX(m_cameraAngleX);
  transform *= rotateY(m_cameraAngleY);

  std::vector<Triangle> triangles;
  m_model->getTriangles(triangles, transform);

  for ( size_t i=0; i < triangles.size(); i++ )
  {
    std::vector<Pixel> pixels;
    triangles[i].raster(pixels, width, height);     // get all pixels in discrete coordinates

    for ( size_t j=0; j < pixels.size(); j++ )
    {
      const Pixel &p = pixels[j];
      if ( p.x < 0 || p.x >= width
        || p.y < 0 || p.y >= height )
        continue;

      float depth = triangles[i].getDepth(p);       // we get depth of a pixel using barycentric coordinates
      if ( depth < zbuffer(p.x, p.y) )
      {
        zbuffer(p.x, p.y) = depth;
        img_data[height-p.y-1][p.x] = triangles[i].getColor(p);
      }
    }
  }
#endif

  painter.drawImage(QPoint(), img);



}

void ZBWidget::mouseMoveEvent(QMouseEvent *event)
{
  if ( event->buttons() )
  {
    m_buttons = event->buttons();
  }
}

void ZBWidget::mousePressEvent(QMouseEvent *event)
{
  m_lastPos = event->pos();
  INFO("press pos = (%d, %d)", m_lastPos.x(), m_lastPos.y());
}

void ZBWidget::mouseReleaseEvent(QMouseEvent *event)
{
  const QPoint &pos = event->pos();
  INFO("release pos = (%d, %d)", pos.x(), pos.y());

  if ( m_buttons & Qt::LeftButton )
  {
    m_cameraAngleY += (pos.x()-m_lastPos.x());
    m_cameraAngleX += (pos.y()-m_lastPos.y());
  }
  else if ( m_buttons & Qt::RightButton )
  {
    m_cameraDistance -= (pos.y()-m_lastPos.y()) * 0.02f;
  }
  m_buttons = 0;

  if ( m_lastPos.x() == pos.x()
    && m_lastPos.y() == pos.y() )
    return;

  emit repaintNeeded();
  INFO("processing...");
}

