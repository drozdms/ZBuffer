#include <QLabel>
#include "MainWindow.hpp"
#include "ui_MainWindow.h"
#include "ZBWidget.hpp"

MainWindow::MainWindow(std::vector<Model*> model, QWidget *parent) :
  QMainWindow(parent),
  m_model(model),
  m_ui(new Ui::MainWindow)
{
  m_ui->setupUi(this);
  for (int i = 0; i<3; ++i)
  {
      char cstr[10];
      itoa(m_model[i]->trianglesNumber()/3,cstr,10);
    m_ui->gridLayout->addWidget(new QLabel(cstr), 0,i, Qt::AlignCenter);
    m_ui->gridLayout->addWidget(new ZBWidget(model[i], this), 1, i);
  }
  for (int i = 3; i<6; ++i)
  {
    char cstr[10];
    itoa(m_model[i]->trianglesNumber()/3,cstr,10);
    m_ui->gridLayout->addWidget(new QLabel(cstr), 2,i-3, Qt::AlignCenter);
    m_ui->gridLayout->addWidget(new ZBWidget(model[i], this), 3, i-3);
  }
  m_ui->gridLayout->setRowStretch(0, 0);
  m_ui->gridLayout->setRowStretch(1, 1);
  m_ui->gridLayout->setRowStretch(2, 0);
  m_ui->gridLayout->setRowStretch(3, 1);

  setWindowIcon(QIcon("z-buffer-net.jpg"));

}

MainWindow::~MainWindow()
{
  for (int i = 0; i< 6; ++i)
      delete m_model[i];
  delete m_ui;
}
