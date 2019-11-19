#ifndef __GL_WIDGET_HPP__
#define __GL_WIDGET_HPP__

#include <QWidget>
#include <qglviewer.h>
#include "Model.hpp"

class GLWidget : public QGLViewer {

Q_OBJECT

public:
  static const int MAX_SHAPES = 16;

public:
  GLWidget(Model *model, QWidget *parent=0);
  ~GLWidget();

protected:
  virtual void init();
  virtual void draw();

private:
  Model *m_model;
  size_t m_numShapes;
  GLuint m_indexBuffer[MAX_SHAPES];
  GLuint m_vertexBuffer[MAX_SHAPES];

};

#endif //__GL_WIDGET_HPP__
