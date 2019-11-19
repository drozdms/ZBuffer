#ifndef __MAIN_WINDOW_HPP__
#define __MAIN_WINDOW_HPP__

#include <QMainWindow>
#include "Model.hpp"

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow {

Q_OBJECT

public:
  explicit MainWindow(std::vector<Model*> model, QWidget *parent=0);
  ~MainWindow();

private:
  std::vector<Model *> m_model;
  Ui::MainWindow *m_ui;

};

#endif //__MAIN_WINDOW_HPP__
