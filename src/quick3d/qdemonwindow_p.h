#ifndef QDEMONWINDOW_P_H
#define QDEMONWINDOW_P_H

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

#include <private/qtquick3dglobal_p.h>
#include <private/qwindow_p.h>
#include <private/qdemonrenderloop_p.h>

#include <QtCore/QSet>
#include <QtCore/QMutex>
#include <QtCore/QList>
#include <QtCore/QRunnable>
#include <QtCore/QSharedPointer>

#include <QtDemonRuntimeRender/qdemonrenderscene.h>
#include <QtDemonRuntimeRender/qdemonrenderpresentation.h>

#include "qdemonnode.h"

#include "qdemonwindow.h"

QT_BEGIN_NAMESPACE

class QQuickAnimatorController;
class QQuickWindowIncubationController;
class QDemonLayer;

class Q_QUICK3D_PRIVATE_EXPORT QDemonWindowPrivate : public QWindowPrivate
{
public:
    Q_DECLARE_PUBLIC(QDemonWindow)

    enum CustomEvents { FullUpdateRequest = QEvent::User + 1 };

    static inline QDemonWindowPrivate *get(QDemonWindow *c) { return c->d_func(); }

    QDemonWindowPrivate();
    ~QDemonWindowPrivate() override;

    void init(QDemonWindow *);

    QDemonObject *contentItem;
    QSet<QDemonObject *> parentlessItems;
    QQmlListProperty<QObject> data();

    QDemonObject *activeFocusItem;

    void deliverKeyEvent(QKeyEvent *);

    void dirtyItem(QDemonObject *);
    void cleanup(QDemonGraphObject *);

    void polishItems();
    void forcePolish();
    void syncSceneGraph();
    void renderSceneGraph(const QSize &size);

    bool isRenderable() const;

    // QDemonNode::UpdatePaintNodeData updatePaintNodeData;

    QDemonObject *dirtySpatialNodeList;
    QDemonObject *dirtyResourceList;
    QList<QDemonGraphObject *> cleanupNodeList;

    QVector<QDemonObject *> itemsToPolish;
    QVector<QDemonObject *> hasFiltered; // during event delivery to a single receiver, the filtering parents for which childMouseEventFilter was already called
    QVector<QDemonObject *> skipDelivery; // during delivery of one event to all receivers, Items to which we know delivery is no longer necessary

    qreal devicePixelRatio;
    QMetaObject::Connection physicalDpiChangedConnection;

    void updateDirtyNodes();
    void cleanupNodes();
    void cleanupNodesOnShutdown();
    bool updateEffectiveOpacity(QDemonObject *);
    void updateEffectiveOpacityRoot(QDemonObject *, qreal);
    void updateDirtyNode(QDemonObject *);

    void updateDirtyResource(QDemonObject *resourceObject);
    void updateDirtySpatialNode(QDemonNode *spatialNode);
    void updateDirtyLayer(QDemonLayer *layerNode);

    void fireFrameSwapped() { Q_EMIT q_func()->frameSwapped(); }
    void fireAboutToStop() { Q_EMIT q_func()->sceneGraphAboutToStop(); }

    QOpenGLContext *glContext;
    QDemonRenderContextInterface *context;
    QDemonRef<QDemonPresentation> m_presentation;
    QDemonRef<QDemonRenderScene> m_scene;

    QDemonRenderLoop *windowManager;
    QQuickAnimatorController *animationController;

    QColor clearColor;

    uint clearBeforeRendering : 1;

    uint persistentGLContext : 1;
    uint persistentSceneGraph : 1;

    uint lastWheelEventAccepted : 1;
    bool componentCompleted : 1;

    bool allowChildEventFiltering : 1;
    bool allowDoubleClick : 1;

    mutable QDemonWindowIncubationController *incubationController;

    // data property
    static void data_append(QQmlListProperty<QObject> *, QObject *);
    static int data_count(QQmlListProperty<QObject> *);
    static QObject *data_at(QQmlListProperty<QObject> *, int);
    static void data_clear(QQmlListProperty<QObject> *);

    QMutex renderJobMutex;
    QList<QRunnable *> beforeSynchronizingJobs;
    QList<QRunnable *> afterSynchronizingJobs;
    QList<QRunnable *> beforeRenderingJobs;
    QList<QRunnable *> afterRenderingJobs;
    QList<QRunnable *> afterSwapJobs;

    void runAndClearJobs(QList<QRunnable *> *jobs);

private:
    static void cleanupNodesOnShutdown(QDemonObject *);
};

class QDemonWindowQObjectCleanupJob : public QRunnable
{
public:
    QDemonWindowQObjectCleanupJob(QObject *o) : object(o) {}
    void run() override { delete object; }
    QObject *object;
    static void schedule(QDemonWindow *window, QObject *object)
    {
        Q_ASSERT(window);
        Q_ASSERT(object);
        window->scheduleRenderJob(new QDemonWindowQObjectCleanupJob(object), QDemonWindow::AfterSynchronizingStage);
    }
};

QT_END_NAMESPACE

#endif // QDEMONWINDOW_P_H
