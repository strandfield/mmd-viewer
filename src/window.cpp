#include "window.h"

#include "widgets/charactersviewer.h"
#include "widgets/timcollectionviewer.h"
#include "widgets/timviewer.h"
#include "widgets/tmdviewer.h"

#include "converters/tim2image.h"

#include "formats/mmd.h"
#include "gamereader.h"

#include <QFileDialog>
#include <QMessageBox>

#include <QAction>
#include <QMenu>
#include <QMenuBar>

#include <QTabWidget>

#include <QSettings>

#include <QStringList>

#include <format>
#include <fstream>

#include <iostream>

constexpr const char* PSX_EXENAME = "SLUS_010.32";

// settings key
constexpr const char* WINDOW_GEOM_KEY = "Window/geometry";
constexpr const char* LAST_OPEN_DIR_KEY = "lastOpenDir";

inline QDebug operator<<(QDebug dbg, const TMD_Code code)
{
  switch (code)
  {
  case TMD_Code_LINE:
    dbg << "line";
    break;
  case TMD_Code_POLYGON:
    dbg << "polygon";
    break;
  case TMD_Code_SPRITE:
    dbg << "sprite";
    break;
  case TMD_Code_INVALID:
  default:
    dbg << "unknown";
    break;
  }

  return dbg;
}

MainWindow::MainWindow()
{
  setWindowTitle("MMD Viewer");

  m_settings = new QSettings(QSettings::IniFormat,
                             QSettings::UserScope,
                             "Analogman Software",
                             "MMD Viewer");

  QMenu* menu = menuBar()->addMenu("File");

  {
    menu->addAction("Open...", this, &MainWindow::actOpen, QKeySequence("Ctrl+O"));
    menu->addAction("Open File...", this, &MainWindow::actOpenFile);
    menu->addSeparator();
    menu->addAction("Quit", this, &MainWindow::close, QKeySequence("Alt+F4"));
  }

  m_tab_widget = new QTabWidget;
  m_tab_widget->setTabsClosable(true);
  setCentralWidget(m_tab_widget);
  connect(m_tab_widget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

  // restore window geometry
  {
    const auto geometry = settings().value(WINDOW_GEOM_KEY, QByteArray()).toByteArray();
    if (!geometry.isEmpty())
      restoreGeometry(geometry);
  }
}

MainWindow::~MainWindow()
{

}

QSettings& MainWindow::settings() const
{
  return *m_settings;
}

void MainWindow::openDirectory(const QString& directory)
{
  QFileInfo exeinfo{directory + "/" + PSX_EXENAME};

  if (!exeinfo.exists())
  {
    QMessageBox::information(this,
                             "Error",
                             QString("The provided directory does not contain a '%1' file.")
                                 .arg(PSX_EXENAME));
    return;
  }

  GameReader reader;
  reader.readPSX_EXE(exeinfo.absoluteFilePath());

  auto* viewer = new CharactersViewer(reader.result, this);
  m_tab_widget->addTab(viewer, "Characters");
}

void MainWindow::open(const QString& filePath)
{
  const QFileInfo info{filePath};

  if (QString::compare(info.suffix(), "TIM") == 0)
  {
    std::ifstream stream{filePath.toStdString(), std::ios::in | std::ios::binary};

    struct TimEntry
    {
      size_t offset;
      TimImage image;
    };

    std::vector<TimEntry> entries;

    // read entries
    {
      TimReader reader;
      TimEntry e;
      e.offset = stream.tellg();
      while (reader.readTim(stream, e.image))
      {
        entries.push_back(e);
        reader.seekNext(stream);
        e.offset = stream.tellg();
      }
    }

    if (entries.size() > 1)
    {
      auto* viewer = new TimCollectionViewer;
      for (const TimEntry& ee : entries)
      {
        viewer->addTim("0x" + QString::number(ee.offset, 16), ee.image);
      }
      m_tab_widget->addTab(viewer, info.fileName());
    }
    else if (entries.size() == 1)
    {
      auto* viewer = new TimViewer(entries.front().image);
      m_tab_widget->addTab(viewer, info.fileName());
    }
    else
    {
      QMessageBox::information(this,
                               "Error",
                               "An error occurred while opening the file.",
                               QMessageBox::Ok);
    }

    // add TIM images to existing TMD viewers
    if (!entries.empty())
    {
      if (auto viewers = m_tab_widget->findChildren<TMD_Viewer*>(); !viewers.empty())
      {
        for (const auto& e : entries)
        {
          for (TMD_Viewer* tmdviewer : viewers)
          {
            tmdviewer->addTIM(e.image);
          }
        }
      }
    }
  }
  else if (QString::compare(info.suffix(), "TMD") == 0)
  {
    TMD_Reader reader;
    TMD_Model model;
    if (!reader.readModel(filePath.toStdString(), model))
    {
      QMessageBox::information(this, "Error", "An error occurred while opening the file.");
      return;
    }

    constexpr bool debug_tmd = false;

    if (debug_tmd)
    {
      qDebug() << "number of objects in" << filePath << ":" << model.objects().size();

      for (const TMD_Object& obj : model.objects())
      {
        qDebug() << "  object with" << obj.primitives().count() << "primitives:";
        for (int i(0); i < obj.primitives().count(); ++i)
        {
          TMD_Primitive primitive{obj.primitives().at(i)};
          qDebug() << "    " << primitive.getCode() << "(" << primitive.vertexCount() << "vertex)";
        }
      }

      qDebug() << "that's all folks!";
    }

    auto* viewer = new TMD_Viewer(model);
    m_tab_widget->addTab(viewer, info.fileName());

    if (auto* timviewer = m_tab_widget->findChild<TimViewer*>())
    {
      qDebug() << "found timviewer";
      viewer->addTIM(timviewer->getTimImage());
    }

    openAssociatedTIM(info);
  }
  else if (info.fileName() == PSX_EXENAME)
  {
    openDirectory(info.absolutePath());
  }
  else
  {
    QMessageBox::information(this,
                             "Unsupported format",
                             "The selected file is not of a format supported.");
  }
}

void MainWindow::actOpen()
{
  auto folder = settings().value(LAST_OPEN_DIR_KEY).toString();

  QString path = QFileDialog::getExistingDirectory(this, "Open game directory", folder);

  if (path.isEmpty())
  {
    return;
  }

  settings().setValue(LAST_OPEN_DIR_KEY, path);

  openDirectory(path);
}

void MainWindow::actOpenFile()
{
  auto folder = settings().value(LAST_OPEN_DIR_KEY).toString();

  const auto allformats = QStringList() << "*.tim"
                                        << "*.tmd" << PSX_EXENAME;

  auto formats = QStringList() << QString("All supported formats (%1)").arg(allformats.join(" "))
                               << "TIM file (*.tim)"
                               << "TMD file (*.tmd)" << QString("PSEXE (%1)").arg(PSX_EXENAME);

  QString path = QFileDialog::getOpenFileName(this, "Open file", folder, formats.join(";;"));

  if (path.isEmpty())
  {
    return;
  }

  settings().setValue(LAST_OPEN_DIR_KEY, QFileInfo(path).absolutePath());

  open(path);
}

void MainWindow::closeTab(int tabIndex)
{
  auto* w = m_tab_widget->widget(tabIndex);

  if (qobject_cast<TimViewer*>(w) || qobject_cast<TimCollectionViewer*>(w)
      || qobject_cast<TMD_Viewer*>(w))
  {
    m_tab_widget->removeTab(tabIndex);
    return;
  }

  if (qobject_cast<CharactersViewer*>(w))
  {
    m_tab_widget->removeTab(tabIndex);
    return;
  }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  settings().setValue(WINDOW_GEOM_KEY, saveGeometry());
  QMainWindow::closeEvent(event);
}

void MainWindow::openAssociatedTIM(const QFileInfo& tmdInfo)
{
  QFileInfo info{tmdInfo.absolutePath() + "/" + tmdInfo.baseName() + ".TIM"};

  if (!info.exists())
  {
    return;
  }

  int result = QMessageBox::question(
      this,
      "Open TIM file",
      "A TIM file with the same name as the TMD file exists in the directory.\n"
      "Do you want open it too?",
      QMessageBox::Yes,
      QMessageBox::No);

  if (result == QMessageBox::Yes)
  {
    open(info.absoluteFilePath());
  }
}
