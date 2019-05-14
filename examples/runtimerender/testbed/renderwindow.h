#ifndef RENDERWINDOW_H
#define RENDERWINDOW_H

#include <QtGui/QWindow>
#include <QtCore/QElapsedTimer>
#include <QtGui/QOpenGLContext>
#include <QtDemon/qtdemonglobal.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>

QT_BEGIN_NAMESPACE
class QDemonRenderContext;
class QDemonRenderLayer;
struct QDemonRenderPresentation;
struct QDemonRenderScene;
struct QDemonRenderModel;
QT_END_NAMESPACE

class RenderWindow : public QWindow
{
public:
    RenderWindow(QWindow *parent = nullptr);
    ~RenderWindow() override;
    virtual void initialize();
    virtual void drawFrame(qint64 delta);

    void setAutoUpdate(bool autoUpdate) { m_autoUpdate = autoUpdate;}

public slots:
    void renderLater();
    void renderNow();

private slots:
    void updateAnimations();

protected:
    bool event(QEvent *event) override;
    void exposeEvent(QExposeEvent *event) override;

private:
    void preInit();
    void buildTestScene();

    QElapsedTimer m_frameTimer;
    bool m_autoUpdate = true;
    bool m_isIntialized = false;
    QOpenGLContext *m_glContext;
    QDemonRenderLayer *m_layer;
    QDemonRenderContextInterface::QDemonRenderContextInterfacePtr m_context;
    QDemonRef<QDemonRenderContext> m_renderContext;
    QDemonRenderModel *m_cube;

};

#endif // RENDERWINDOW_H
