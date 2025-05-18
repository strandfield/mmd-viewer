// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "formats/tim.h"

#include <QWidget>

#include <QPixmap>

#include <vector>

class TimViewer : public QWidget
{
  Q_OBJECT
public:
  explicit TimViewer(QWidget* parent = nullptr);
  explicit TimViewer(const TimImage& img, QWidget* parent = nullptr);
  ~TimViewer();

  const TimImage& getTimImage() const;
  void setTimImage(const TimImage& img);

protected:
  void paintEvent(QPaintEvent* ev) override;

private:
  TimImage m_tim;
  std::vector<QPixmap> m_pixmaps;
};
