// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#include "window.h"

#include <QApplication>

#include <QFileInfo>

int main(int argc, char *argv[])
{
  QApplication::setOrganizationName("Analogman Software");
  //QApplication::setOrganizationDomain("mysoft.com");
  QApplication::setApplicationName("MMD Viewer");

  QApplication app{argc, argv};

  MainWindow window;
  window.show();

  if (app.arguments().size() > 1)
  {
    for (int i(1); i < app.arguments().size(); ++i)
    {
      if (QFileInfo::exists(app.arguments().at(i)))
      {
        window.open(app.arguments().at(i));
      }
    }
  }

  return app.exec();
}
