#ifndef QDEMONWINDOW_H
#define QDEMONWINDOW_H

#include <QtCore/qmetatype.h>
#include <QtGui/QWindow>

#include <QtQuick3d/qtquick3dglobal.h>
#include <QtQuick3d/QDemonObject>

#include <QtQml/qqml.h>


QT_BEGIN_NAMESPACE

class QDemonWindowPrivate;
class QQuickCloseEvent;
class QQmlIncubationController;
class QDemonWindowIncubationController;
class QRunnable;

class Q_QUICK3D_EXPORT QDemonWindow : public QWindow
{
    Q_OBJECT
    Q_PRIVATE_PROPERTY(QDemonWindow::d_func(), QQmlListProperty<QObject> data READ data DESIGNABLE false)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QDemonObject* contentItem READ contentItem CONSTANT)
    Q_PROPERTY(QDemonObject* activeFocusItem READ activeFocusItem NOTIFY activeFocusItemChanged)
    Q_CLASSINFO("DefaultProperty", "data")
    Q_DECLARE_PRIVATE(QDemonWindow)
public:
    enum RenderStage {
        BeforeSynchronizingStage,
        AfterSynchronizingStage,
        BeforeRenderingStage,
        AfterRenderingStage,
        AfterSwapStage,
        NoStage
    };

    enum SceneGraphError {
        ContextNotAvailable = 1
    };
    Q_ENUM(SceneGraphError)

    QDemonWindow(QWindow *parent = nullptr);
    ~QDemonWindow() override;

    QDemonObject *contentItem() const;

    QDemonObject *activeFocusItem() const;
    QObject *focusObject() const override;

    QImage grabWindow();

    QQmlIncubationController *incubationController() const;

    void setColor(const QColor &color);
    QColor color() const;

    void scheduleRenderJob(QRunnable *job, RenderStage stage);

    qreal effectiveDevicePixelRatio() const;

    QOpenGLContext *graphicsContext();

Q_SIGNALS:
    void frameSwapped();
    void sceneGraphInitialized();
    void sceneGraphInvalidated();
    void beforeSynchronizing();
    void afterSynchronizing();
    void beforeRendering();
    void afterRendering();
    void afterAnimating();
    void sceneGraphAboutToStop();

    void closing(QQuickCloseEvent *close);
    void colorChanged(const QColor &);
    void activeFocusItemChanged();
    void sceneGraphError(QDemonWindow::SceneGraphError error, const QString &message);


public Q_SLOTS:
    void update();
    void releaseResources();

protected:
    QDemonWindow(QDemonWindowPrivate &dd, QWindow *parent = nullptr);

    void exposeEvent(QExposeEvent *) override;
    void resizeEvent(QResizeEvent *) override;

    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;

    void focusInEvent(QFocusEvent *) override;
    void focusOutEvent(QFocusEvent *) override;

    bool event(QEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
    void keyReleaseEvent(QKeyEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
#if QT_CONFIG(wheelevent)
    void wheelEvent(QWheelEvent *) override;
#endif

private Q_SLOTS:
    void maybeUpdate();
    void cleanupSceneGraph();
    void physicalDpiChanged();
    void handleScreenChanged(QScreen *screen);
    void setTransientParent_helper(QDemonWindow *window);
    void runJobsAfterSwap();
    void handleApplicationStateChanged(Qt::ApplicationState state);

private:
    friend class QDemonObject;
    Q_DISABLE_COPY(QDemonWindow)
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QDemonWindow *)

#endif // QDEMONWINDOW_H
