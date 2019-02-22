#ifndef QDEMONRENDERLOOP_P_H
#define QDEMONRENDERLOOP_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include <QObject>
#include <private/qtquick3dglobal_p.h>
#include <QtGui/QImage>
#include <QtGui/QSurface>
#include <QtCore/QSet>

QT_BEGIN_NAMESPACE

class QDemonWindow;
class QAnimationDriver;
class QDemonRenderContext;
class QDemonRenderContextInterface;
class QRunnable;

class Q_QUICK3D_PRIVATE_EXPORT QDemonRenderLoop : public QObject
{
    Q_OBJECT
public:
    enum RenderLoopFlags {
        SupportsGrabWithoutExpose = 0x01
    };

    virtual ~QDemonRenderLoop();

    virtual void show(QDemonWindow *window) = 0;
    virtual void hide(QDemonWindow *window) = 0;
    virtual void resize(QDemonWindow *) {}

    virtual void windowDestroyed(QDemonWindow *window) = 0;

    virtual void exposureChanged(QDemonWindow *window) = 0;
    virtual QImage grab(QDemonWindow *window) = 0;

    virtual void update(QDemonWindow *window) = 0;
    virtual void maybeUpdate(QDemonWindow *window) = 0;
    virtual void handleUpdateRequest(QDemonWindow *) { }

    virtual QAnimationDriver *animationDriver() const = 0;

    virtual QSharedPointer<QDemonRenderContextInterface> sceneGraphContext() const = 0;
    virtual QSharedPointer<QDemonRenderContext> renderContext() const = 0;

    virtual void releaseResources(QDemonWindow *window) = 0;
    virtual void postJob(QDemonWindow *window, QRunnable *job);

    void addWindow(QDemonWindow *win) { m_windows.insert(win); }
    void removeWindow(QDemonWindow *win) { m_windows.remove(win); }
    QSet<QDemonWindow *> windows() const { return m_windows; }

    virtual QSurface::SurfaceType windowSurfaceType() const;

    static QDemonRenderLoop *instance();
    static void setInstance(QDemonRenderLoop *instance);

    virtual bool interleaveIncubation() const { return false; }

    virtual int flags() const { return 0; }

    static void cleanup();

Q_SIGNALS:
    void timeToIncubate();

protected:
    void handleContextCreationFailure(QDemonWindow *window, bool isEs);

private:
    static QDemonRenderLoop *s_instance;

    QSet<QDemonWindow *> m_windows;
};

QT_END_NAMESPACE

#endif // QDEMONRENDERLOOP_P_H
