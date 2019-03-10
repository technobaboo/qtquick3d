#ifndef QDEMONVIEW3D_P_H
#define QDEMONVIEW3D_P_H

#include "qdemonview3d.h"
#include <QtQuick/private/qquickitem_p.h>

QT_BEGIN_NAMESPACE
class QDemonRenderLayer;
class QDemonView3DPrivate : public QQuickItemPrivate
{
    Q_DECLARE_PUBLIC(QDemonView3D)

public:
    QDemonView3DPrivate();
    ~QDemonView3DPrivate();

    void createRenderer();
    QSGNode *createNode();
    void sync();

    static QDemonView3DPrivate *get(QDemonView3D *item) { return item->d_func(); }

    QDemonCamera *camera = nullptr;
    QDemonSceneEnvironment *environment = nullptr;
    QDemonNode *sceneRoot = nullptr;
    QDemonNode *referencedScene = nullptr;
    QDemonRenderLayer *layerNode = nullptr;
};

QT_END_NAMESPACE

#endif // QDEMONVIEW3D_P_H
