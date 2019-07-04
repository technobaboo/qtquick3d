#ifndef QDEMONSCENEMANAGER_P_H
#define QDEMONSCENEMANAGER_P_H

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

#include <QtCore/QObject>
#include <QtCore/QSet>

#include <QtQuick3d/private/qtquick3dglobal_p.h>

#include "qdemonobject.h"
#include "qdemonnode.h"

QT_BEGIN_NAMESPACE

class Q_QUICK3D_PRIVATE_EXPORT QDemonSceneManager : public QObject
{
    Q_OBJECT
public:
    explicit QDemonSceneManager(QObject *parent = nullptr);

    void dirtyItem(QDemonObject *item);
    void cleanup(QDemonRenderGraphObject *item);

    void polishItems();
    void forcePolish();
    void sync();

    void updateDirtyNodes();
    void updateDirtyNode(QDemonObject *object);
    void updateDirtyResource(QDemonObject *resourceObject);
    void updateDirtySpatialNode(QDemonNode *spatialNode);
    //void updateDirtyLayer(QDemonLayer *layerNode);

    void cleanupNodes();

    QDemonObject *dirtySpatialNodeList;
    QDemonObject *dirtyResourceList;
    QDemonObject *dirtyImageList;
    QList<QDemonObject *> dirtyLightList;
    QList<QDemonRenderGraphObject *> cleanupNodeList;
    QSet<QDemonObject *> parentlessItems;
    friend QDemonObject;

Q_SIGNALS:
    void needsUpdate();
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDemonSceneManager)

#endif // QDEMONSCENEMANAGER_P_H
