#include <stdio.h>
#include <QApplication>
#include "MainWindow.hpp"
#include <QGLFormat>

int main(int argc, char * argv[]) {

  QApplication app(argc, argv);

  QGLFormat glf = QGLFormat::defaultFormat();
  glf.setSampleBuffers(true);
  glf.setSamples(4);
  QGLFormat::setDefaultFormat(glf);
  std::vector<Model*> m_models;
  for (int i=1; i<7; ++i)
  {
      std::string s = "blue_blade/blue_blade";
      if (i<10)
          s+='0';
      char st[3];
      itoa(i,st,10);
      s+= st;
      s+=".obj";
      m_models.push_back(new Model(s.c_str()));

  }
  MainWindow window(m_models);
  window.show();

  return app.exec();
}
