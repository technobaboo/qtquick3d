#ifndef RENDERWINDOW_H
#define RENDERWINDOW_H

#include <QtGui/QWindow>
#include <QtCore/QElapsedTimer>
#include <QtGui/QOpenGLContext>


class IQDemonRenderContextCore;
class IQDemonRenderContext;
class QDemonRenderContext;
struct SPresentation;
struct SScene;
struct SModel;

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
    QSharedPointer<IQDemonRenderContextCore> m_contextCore;
    QSharedPointer<IQDemonRenderContext> m_context;
    QSharedPointer<QDemonRenderContext> m_renderContext;
    QSharedPointer<SPresentation> m_presentation;
    QSharedPointer<SScene> m_scene;
    SModel *m_cube;

};

#endif // RENDERWINDOW_H
