#ifndef RENDERWINDOW_H
#define RENDERWINDOW_H

#include <QtGui/QWindow>
#include <QtCore/QElapsedTimer>
#include <QtGui/QOpenGLContext>


class QDemonRenderContextCoreInterface;
class QDemonRenderContextInterface;
class QDemonRenderContext;
struct QDemonPresentation;
struct QDemonRenderScene;
struct QDemonRenderModel;

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
    QDemonRef<QDemonRenderContextCoreInterface> m_contextCore;
    QDemonRef<QDemonRenderContextInterface> m_context;
    QDemonRef<QDemonRenderContext> m_renderContext;
    QDemonRef<QDemonPresentation> m_presentation;
    QDemonRef<QDemonRenderScene> m_scene;
    QDemonRenderModel *m_cube;

};

#endif // RENDERWINDOW_H
