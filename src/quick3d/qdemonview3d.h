#ifndef QDEMONVIEW3D_H
#define QDEMONVIEW3D_H

#include <QtGui/QOpenGLFramebufferObject>
#include <QtQuick/QQuickItem>

#include <QtQuick3d/qtquick3dglobal.h>

#include <QtDemonRender/qdemonrenderframebuffer.h>

QT_BEGIN_NAMESPACE

class QDemonView3DPrivate;
class QDemonCamera;
class QDemonSceneEnvironment;
class QDemonNode;
class QDemonSceneRenderer;

class SGFramebufferObjectNode;

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
    QDemonNode *referencedScene() const;

    QDemonSceneRenderer *createRenderer() const;

    bool isTextureProvider() const override;
    QSGTextureProvider *textureProvider() const override;
    void releaseResources() override;

protected:
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) override;

public Q_SLOTS:
    void setCamera(QDemonCamera *camera);
    void setEnvironment(QDemonSceneEnvironment * environment);
    void setScene(QDemonNode *sceneRoot);

private Q_SLOTS:
    void invalidateSceneGraph();

Q_SIGNALS:
    void cameraChanged(QDemonCamera *camera);
    void environmentChanged(QDemonSceneEnvironment * environment);
    void sceneChanged(QDemonNode *sceneRoot);

private:
    Q_DISABLE_COPY(QDemonView3D)

    QDemonCamera *m_camera = nullptr;
    QDemonSceneEnvironment *m_environment = nullptr;
    QDemonNode *m_sceneRoot = nullptr;
    QDemonNode *m_referencedScene = nullptr;
    mutable SGFramebufferObjectNode *m_node = nullptr;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDemonView3D)

#endif // QDEMONVIEW3D_H