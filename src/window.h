// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>

class QTabWidget;

class QFileInfo;
class QSettings;

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  MainWindow();
  ~MainWindow();

  QSettings& settings() const;

  void open(const QString& path);
  void openDirectory(const QString& directory);

protected slots:
  void actOpen();
  void actOpenFile();

protected slots:
  void closeTab(int tabIndex);

protected:
  void closeEvent(QCloseEvent* event) override;

private:
  void openAssociatedTIM(const QFileInfo& tmdInfo);

private:
  QSettings* m_settings = nullptr;
  QTabWidget* m_tab_widget = nullptr;
};

#endif // WINDOW_H
