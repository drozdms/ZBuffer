#include "glew.h"
#include "GLWidget.hpp"
#include "Logger.hpp"
#include "vec.h"

//#define DEBUG_NORMAL

GLWidget::GLWidget(Model *model, QWidget * parent)
  : QGLViewer(parent),
    m_model(model),
    m_numShapes(0)
{
  memset(m_indexBuffer, 0, sizeof(GLuint)*MAX_SHAPES);
  memset(m_vertexBuffer, 0, sizeof(GLuint)*MAX_SHAPES);
}

GLWidget::~GLWidget()
{
  glDeleteBuffers(m_numShapes, m_indexBuffer);
  glDeleteBuffers(m_numShapes, m_vertexBuffer);
}

void GLWidget::init()
{
  ASSERT_MSG(GLEW_OK==glewInit(), "GLWidget: GLEW failed to initialize!");
  ASSERT_MSG(m_model, "GLWidget: model failed to load!");

  m_model->debug();

  m_numShapes = m_model->numShapes();
  if ( m_numShapes > MAX_SHAPES )
    m_numShapes = MAX_SHAPES;

  glEnable(GL_MULTISAMPLE);

  // Generate buffers
  glGenBuffers(m_numShapes, m_indexBuffer);
  glGenBuffers(m_numShapes, m_vertexBuffer);

  for ( size_t i=0; i < m_numShapes; i++ )
  {
    // Bind buffers
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer[i]);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer[i]);

    // Transfer data
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*(m_model->indexSize(i)), m_model->indexData(i), GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*(m_model->vertexSize(i)+m_model->normalSize(i)), 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*(m_model->vertexSize(i)), m_model->vertexData(i));
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*(m_model->vertexSize(i)), sizeof(float)*(m_model->normalSize(i)), m_model->normalData(i));

    // Unbind buffers
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
#ifndef DEBUG_NORMAL
  float shininess = 15.0f;
  float diffuseColor[3] = {0.929524f, 0.796542f, 0.178823f};
  float specularColor[4] = {1.00000f, 0.980392f, 0.549020f, 1.0f};

  // set specular and shiniess using glMaterial (gold-yellow)
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess); // range 0 ~ 128
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularColor);

  // set ambient and diffuse color using glColorMaterial (gold-yellow)
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);
  glColor3fv(diffuseColor);

  // set light
  float light_ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
  float light_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
  float light_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
  float light_position[] = {0.0f, 5.0f, 0.0f, 1.0f};
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
#else
  glDisable(GL_LIGHTING);
  INFO("GL_NORMALIZE: %s", GL_FALSE==glIsEnabled(GL_NORMALIZE) ? "false" : "true");
  INFO("GL_RESCALE_NORMAL: %s", GL_FALSE==glIsEnabled(GL_RESCALE_NORMAL) ? "false" : "true");
#endif
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

  qglviewer::Vec pos = camera()->position();
  pos.z += 1.3f;
  camera()->setPosition(pos);
}

void GLWidget::draw()
{
  for ( size_t i=0; i < m_numShapes; i++ )
  {
    // Bind buffers
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer[i]);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer[i]);

    // Draw
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, 0);
#ifndef DEBUG_NORMAL
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, (GLvoid*)(sizeof(float)*(m_model->vertexSize(i))));
#else
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(3, GL_FLOAT, 0, (GLvoid*)(sizeof(float)*(m_model->vertexSize(i))));
#endif
    glDrawElements(GL_TRIANGLES, m_model->indexSize(i), GL_UNSIGNED_INT, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    // Unbind buffers
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
}
