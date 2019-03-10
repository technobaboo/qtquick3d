#ifndef QDEMONVIEW3D_H
#define QDEMONVIEW3D_H

#include <QQuickItem>
#include <QtQuick3d/qtquick3dglobal.h>

QT_BEGIN_NAMESPACE

class QDemonView3DPrivate;
class QDemonCamera;
class QDemonSceneEnvironment;
class QDemonNode;

class Q_QUICK3D_EXPORT QDemonView3D : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QObject> data READ data)
    Q_PROPERTY(QDemonCamera *camera READ camera WRITE setCamera NOTIFY cameraChanged)
    Q_PROPERTY(QDemonSceneEnvironment *environment READ environment WRITE setEnvironment NOTIFY environmentChanged)
    Q_PROPERTY(QDemonNode* scene READ scene WRITE setScene NOTIFY sceneChanged)
    Q_CLASSINFO("DefaultProperty", "data")
public:
    QDemonView3D(QQuickItem *parent = nullptr);
    ~QDemonView3D() override;

    QQmlListProperty<QObject> data();

    QDemonCamera *camera() const;
    QDemonSceneEnvironment *environment() const;
    QDemonNode *scene() const;

public Q_SLOTS:
    void setCamera(QDemonCamera *camera);
    void setEnvironment(QDemonSceneEnvironment * environment);
    void setScene(QDemonNode *sceneRoot);

Q_SIGNALS:
    void cameraChanged(QDemonCamera *camera);
    void environmentChanged(QDemonSceneEnvironment * environment);
    void sceneChanged(QDemonNode *sceneRoot);

protected:
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *) override;
    void updatePolish() override;
    void itemChange(ItemChange change, const ItemChangeData &data) override;
    void componentComplete() override;
    void classBegin() override;

private:
    Q_DISABLE_COPY(QDemonView3D)
    Q_DECLARE_PRIVATE(QDemonView3D)

    void updateLayerNode();
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDemonView3D)

#endif // QDEMONVIEW3D_H
