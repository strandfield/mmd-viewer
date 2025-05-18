// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "timviewer.h"

class QListWidget;

class TimCollectionViewer : public QWidget
{
  Q_OBJECT
public:
  explicit TimCollectionViewer(QWidget* parent = nullptr);
  ~TimCollectionViewer();

  void addTim(const QString& name, const TimImage& img);
  const std::vector<TimImage>& tims() const { return m_tims; }

protected Q_SLOTS:
  void onCurrentRowChanged(int row);

private:
  QListWidget* m_list_widget = nullptr;
  std::vector<TimImage> m_tims;
  TimViewer* m_viewer = nullptr;
};
