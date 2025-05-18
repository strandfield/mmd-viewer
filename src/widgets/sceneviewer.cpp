#include "sceneviewer.h"

#include "rendering/scenerenderer.h"

#include "frameaxes.h"
#include "orbitalcamera.h"
#include "viewport.h"

struct SceneViewer::Data
{
  Viewport viewport;
  OrbitalCameraController cc;
  Group sceneRoot;
  std::unique_ptr<SceneRenderer> sceneRenderer;
};

SceneViewer::SceneViewer(QWidget* parent)
    : QOpenGLWidget(parent)
    , d(std::make_unique<Data>())
{
  init();
}

void SceneViewer::init()
{
  setMinimumSize(640, 480);

  d->cc.setCamera(d->viewport.camera());
  connect(d->viewport.camera(), &Camera::viewMatrixChanged, this, qOverload<>(&SceneViewer::update));
}

SceneViewer::~SceneViewer()
{
  if (d->sceneRenderer)
  {
    makeCurrent();

    d->sceneRenderer.reset();

    doneCurrent();
  }
}

Group& SceneViewer::sceneRoot() const
{
  return d->sceneRoot;
}

void SceneViewer::mousePressEvent(QMouseEvent* event)
{
  d->cc.mousePressEvent(event, &d->viewport);
}

void SceneViewer::mouseMoveEvent(QMouseEvent* event)
{
  d->cc.mouseMoveEvent(event, &d->viewport);
}

void SceneViewer::mouseReleaseEvent(QMouseEvent* event)
{
  d->cc.mouseReleaseEvent(event, &d->viewport);
}

void SceneViewer::wheelEvent(QWheelEvent* event)
{
  d->cc.wheelEvent(event, &d->viewport);
}

void SceneViewer::initializeGL()
{
  d->sceneRenderer = std::make_unique<SceneRenderer>(context());
}

void SceneViewer::resizeGL(int w, int h)
{
  d->viewport.setRect(QRect(0, 0, w, h));
}

void SceneViewer::paintGL()
{
  QOpenGLContext* ctx = context();
  QOpenGLFunctions* gl = ctx->functions();

  QColor cc{"lightcyan"};
  gl->glClearColor(cc.redF(), cc.greenF(), cc.blueF(), 1.0f);
  gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  gl->glEnable(GL_DEPTH_TEST);

  const QMatrix4x4 proj = projectionMatrix(*d->viewport.frustum());
  const QMatrix4x4 view = d->viewport.camera()->viewMatrix();

  FrameAxes axes;
  axes.drawWorldFrameAxes(gl, proj, view);

  SceneRenderer& renderer = *d->sceneRenderer;
  renderer.viewMatrix = view;
  renderer.projectionMatrix = proj;

  renderer.render(d->sceneRoot);
}
