#include "characterviewer.h"

#include "animationplayer.h"
#include "charactermodel.h"
#include "sceneviewer.h"

#include <QHBoxLayout>

#include <QDebug>

CharacterViewer::CharacterViewer(QWidget* parent)
    : QWidget(parent)
{
  init();
}

CharacterViewer::CharacterViewer(const CharacterEntry& characterEntry,
                                 const MMD_File& mmd,
                                 QWidget* parent)
    : QWidget(parent)
{
  init();

  auto model = std::make_unique<CharacterModel>(characterEntry, mmd);
  m_model = model.get();

  if (mmd.animations.count() > 0)
  {
    model->setupAnimation();
  }

  m_viewer->sceneRoot().add(std::move(model));
}

void CharacterViewer::init()
{
  m_viewer = new SceneViewer(this);

  auto* layout = new QHBoxLayout(this);
  layout->addWidget(m_viewer, 1);
}

const QString& CharacterViewer::sourceDir() const
{
  return m_sourceDir;
}

void CharacterViewer::setSourceDir(const QString& sourceDir)
{
  m_sourceDir = sourceDir;
}

void CharacterViewer::reset(const CharacterEntry& characterEntry)
{
  MMD_File mmd;
  if (!mmd.open(sourceDir().toStdString(), characterEntry.index, characterEntry.filename))
  {
    return;
  }

  if (m_player)
  {
    delete m_player;
    m_player = nullptr;
  }

  m_viewer->sceneRoot().clear();

  auto model = std::make_unique<CharacterModel>(characterEntry, mmd);
  m_model = model.get();

  if (mmd.animations.count() > 0)
  {
    model->setupAnimation();
  }

  m_viewer->sceneRoot().add(std::move(model));
  m_viewer->update();
}

CharacterModel* CharacterViewer::model() const
{
  return m_model;
}

int CharacterViewer::animationCount() const
{
  return m_model ? m_model->animations.size() : 0;
}

void CharacterViewer::playAnimation(int index)
{
  if (index < 0 || index >= m_model->animations.size())
    return;

  if (!m_player)
  {
    m_player = new AnimationPlayer(*m_model, this);
    connect(m_player, &AnimationPlayer::stepped, m_viewer, qOverload<>(&SceneViewer::update));
  }

  // m_model->setupAnimation(n);
  // m_viewer->update();
  m_player->playAnimation(m_model->animations.at(index));
}
