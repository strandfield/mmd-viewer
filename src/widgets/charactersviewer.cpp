#include "charactersviewer.h"

#include "charactermodel.h"
#include "characterviewer.h"

#include "timviewer.h"

#include "converters/tim2image.h"

#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QSplitter>
#include <QTextEdit>

#include <QFormLayout>
#include <QHBoxLayout>

#include <QEvent>
#include <QHelpEvent>

#include <QDebug>

#include <format>
#include <iostream>

#include <format>
#include <iostream>

static void print(QTextStream& stream, const MMD_Animation::Instruction& instruction, size_t index)
{
  stream << index << ": ";

  if (std::holds_alternative<MMD_Animation::KeyframeInstruction>(instruction))
  {
    const auto& ins = std::get<MMD_Animation::KeyframeInstruction>(instruction);
    stream << std::format("KEYFRAME (TC={})\n", ins.timecode).c_str();

    for (const MMD_Animation::KeyframeEntry& e : ins.entries)
    {
      stream << std::format("  NODE {}\n", e.affectedNode).c_str();

      for (const auto& p : e.values)
      {
        const char* axes[9] = {"SX", "SY", "SZ", "RX", "RY", "RZ", "X", "Y", "Z"};
        static_assert(static_cast<int>(MMD_Animation::Axis::SCALE_X) == 0,
                      "expects Axis to be usable as an index");
        stream << std::format("    {} {}\n", axes[static_cast<int>(p.first)], p.second).c_str();
      }
    }
  }
  else if (std::holds_alternative<MMD_Animation::LoopStartInstruction>(instruction))
  {
    const auto& ins = std::get<MMD_Animation::LoopStartInstruction>(instruction);

    stream << std::format("START LOOP ({})\n", ins.loopCount).c_str();
  }
  else if (std::holds_alternative<MMD_Animation::LoopEndInstruction>(instruction))
  {
    const auto& ins = std::get<MMD_Animation::LoopEndInstruction>(instruction);

    stream << std::format("END LOOP (TC={}, NEWTIME={})\n", ins.timecode, ins.newTime).c_str();
  }
  else if (std::holds_alternative<MMD_Animation::TextureInstruction>(instruction))
  {
    const auto& ins = std::get<MMD_Animation::TextureInstruction>(instruction);

    stream << std::format("TEXTURE (TC={})\n", ins.timecode).c_str();
    stream << std::format("  SOURCE ({},{}) {}x{}\n", ins.srcX, ins.srcY, ins.width, ins.height)
                  .c_str();
    stream << std::format("  DEST ({},{})\n", ins.destX, ins.destY).c_str();
  }
  else if (std::holds_alternative<MMD_Animation::PlaySoundInstruction>(instruction))
  {
    const auto& ins = std::get<MMD_Animation::PlaySoundInstruction>(instruction);

    stream << std::format("PLAY SOUND (TC={}, VAB={}, SOUND={})\n",
                          ins.timecode,
                          ins.vabId,
                          ins.soundId)
                  .c_str();
  }
  else
  {
    stream << "NOT IMPLEMENTED"
           << "\n";
  }
}

QString toString(const MMD_Animation& a)
{
  QString result;

  {
    QTextStream stream{&result, QTextStream::WriteOnly};

    stream << "BEGIN INSTRUCTIONS"
           << "\n";

    for (size_t i(0); i < a.instructions.size(); ++i)
    {
      print(stream, a.instructions.at(i), i);
    }

    stream << "END INSTRUCTIONS"
           << "\n";
  }

  return result;
}

class CharacterInfoGroupBox : public QGroupBox
{
  Q_OBJECT
public:
  explicit CharacterInfoGroupBox(const CharacterEntry& e, QWidget* parent = nullptr)
      : QGroupBox(parent)
  {
    setTitle("Character");

    auto* form = new QFormLayout;
    form->addRow("ID:", m_index = new QLabel());
    form->addRow("Filename:", m_filename = new QLabel());
    form->addRow("Bones:", m_boneCount = new QLabel());
    setLayout(form);

    fill(e);
  }

  void fill(const CharacterEntry& character)
  {
    m_index->setText(QString::number(character.index));
    m_filename->setText(QString::fromStdString(character.filename));
    m_boneCount->setText(QString::number(character.skeleton.size()));
  }

private:
  QLabel* m_index;
  QLabel* m_filename;
  QLabel* m_boneCount;
};

class AnimationInfoGroupBox : public QGroupBox
{
  Q_OBJECT
public:
  explicit AnimationInfoGroupBox(QWidget* parent = nullptr)
      : QGroupBox(parent)
  {
    setTitle("Animation");

    auto* layout = new QVBoxLayout;

    if (auto* form = new QFormLayout)
    {
      form->addRow("ID:", m_index = new QLabel());
      form->addRow("Frames:", m_frameCount = new QLabel());
      layout->addLayout(form);
    }

    m_instructions = new QTextEdit;
    m_instructions->setReadOnly(true);
    m_instructions->setFontFamily("Courier");
    layout->addWidget(m_instructions);

    setLayout(layout);

    setVisible(false);
  }

  void fill(const MMD_Animation& a)
  {
    m_index->setText(QString::number(a.id));
    m_frameCount->setText(QString::number(a.frameCount));
    m_instructions->setPlainText(toString(a));
  }

private:
  QLabel* m_index;
  QLabel* m_frameCount;
  QTextEdit* m_instructions;
};

class TextureViewer : public QLabel
{
  Q_OBJECT
public:
  explicit TextureViewer(QWidget* parent = nullptr)
      : QLabel(parent)
  {
    m_tooltip = std::make_unique<TimViewer>();
    m_tooltip->setWindowFlags(Qt::ToolTip);
  }

  void fill(const CharacterEntry& e)
  {
    m_image = e.texture;
    QImage img = tim2image(m_image.generateImage());
    setPixmap(QPixmap::fromImage(img));

    m_tooltip->setTimImage(m_image);
    int palnum = std::max(m_image.numberOfPalettes(), 1);
    m_tooltip->resize(m_image.width() * 2, m_image.height() * palnum / 2);
  }

public:
  bool event(QEvent* e) override
  {
    if (e->type() == QEvent::ToolTip)
    {
      // qDebug() << "tooltip requested";
      auto* ev = static_cast<QHelpEvent*>(e);
      m_tooltip->show();
      setMouseTracking(true);
      m_tooltip->move(ev->globalX() - m_tooltip->width(), ev->globalY() - m_tooltip->height());
    }

    return QLabel::event(e);
  }

  void mouseMoveEvent(QMouseEvent* ev) override
  {
    if (m_tooltip->isVisible())
    {
      setMouseTracking(false);
      m_tooltip->hide();
    }

    QLabel::mouseMoveEvent(ev);
  }

  void leaveEvent(QEvent* ev) override
  {
    if (m_tooltip->isVisible())
    {
      setMouseTracking(false);
      m_tooltip->hide();
    }

    QLabel::leaveEvent(ev);
  }

private:
  TimImage m_image;
  std::unique_ptr<TimViewer> m_tooltip;
};

CharactersViewer::CharactersViewer(const GameData& gameData, QWidget* parent)
    : QWidget(parent)
    , m_gameData(gameData)
{
  m_characterList = new QListWidget;

  m_viewer = new CharacterViewer(this);
  m_viewer->setSourceDir(QString::fromStdString(m_gameData.sourceDir.string()));
  m_viewer->reset(m_gameData.characters.front());

  auto* right_column = new QWidget;
  {
    m_animationList = new QListWidget;
    auto* layout = new QVBoxLayout(right_column);
    layout->addWidget(new CharacterInfoGroupBox(gameData.characters[0]));
    layout->addWidget(new QLabel("Animations"));
    layout->addWidget(m_animationList);
    layout->addWidget(new AnimationInfoGroupBox());
    layout->addWidget(new TextureViewer());
  }

  auto* splitter = new QSplitter(Qt::Horizontal, this);
  splitter->addWidget(m_characterList);
  splitter->addWidget(m_viewer);
  splitter->addWidget(right_column);

  auto* layout = new QHBoxLayout(this);
  layout->addWidget(splitter);

  for (int i(0); i < gameData.characters.size(); ++i)
  {
    m_characterList->addItem(QString::fromStdString(gameData.characters.at(i).filename));
  }

  fillAnimationList();

  connect(m_characterList,
          &QListWidget::currentTextChanged,
          this,
          &CharactersViewer::onSelectedCharacterChanged);

  connect(m_animationList,
          &QListWidget::currentTextChanged,
          this,
          &CharactersViewer::onSelectedAnimationChanged);

  m_characterList->setCurrentRow(0);
}

void CharactersViewer::onSelectedCharacterChanged()
{
  int n = m_characterList->currentRow();

  if (n == -1)
    return;

  const CharacterEntry& character = m_gameData.characters[n];

  m_viewer->reset(character);

  if (auto* info = findChild<CharacterInfoGroupBox*>())
  {
    info->fill(character);
  }

  if (auto* texture = findChild<TextureViewer*>())
  {
    texture->fill(character);
  }

  fillAnimationList();
}

void CharactersViewer::onSelectedAnimationChanged()
{
  const int n = m_animationList->currentRow();

  if (auto* info = findChild<AnimationInfoGroupBox*>())
  {
    info->setVisible(n != -1);
    if (n != -1)
    {
      info->fill(m_viewer->model()->animations.at(n));
    }
  }

  m_viewer->playAnimation(n);
}

void CharactersViewer::fillAnimationList()
{
  if (!m_viewer->model())
  {
    return;
  }

  const MMD_File& mmd = m_viewer->model()->mmd;
  m_animationList->clear();
  for (int i(0); i < mmd.animations.count(); ++i)
  {
    m_animationList->addItem(QString("Animation %1").arg(i + 1));
  }
}

#include "charactersviewer.moc"
