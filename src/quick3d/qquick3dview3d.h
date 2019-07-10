/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Quick 3D.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QDEMONVIEW3D_H
#define QDEMONVIEW3D_H

#include <QtGui/QOpenGLFramebufferObject>
#include <QtQuick/QQuickItem>

#include <QtQuick3D/qtquick3dglobal.h>

#include <QtDemonRender/qdemonrenderframebuffer.h>

QT_BEGIN_NAMESPACE

class QDemonView3DPrivate;
class QQuick3DCamera;
class QQuick3DSceneEnvironment;
class QQuick3DNode;
class QQuick3DSceneRenderer;

class SGFramebufferObjectNode;
class QQuick3DSGRenderNode;
class QQuick3DSGDirectRenderer;

class Q_QUICK3D_EXPORT QQuick3DView3D : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QObject> data READ data)
    Q_PROPERTY(QQuick3DCamera *camera READ camera WRITE setCamera NOTIFY cameraChanged)
    Q_PROPERTY(QQuick3DSceneEnvironment *environment READ environment WRITE setEnvironment NOTIFY environmentChanged)
    Q_PROPERTY(QQuick3DNode* scene READ scene WRITE setScene NOTIFY sceneChanged)
    Q_PROPERTY(QQuick3DView3DRenderMode renderMode READ renderMode WRITE setRenderMode NOTIFY renderModeChanged)
    Q_CLASSINFO("DefaultProperty", "data")
public:
    enum QQuick3DView3DRenderMode {
        Texture,
        Underlay,
        Overlay,
        RenderNode
    };
    Q_ENUM(QQuick3DView3DRenderMode)

    explicit QQuick3DView3D(QQuickItem *parent = nullptr);
    ~QQuick3DView3D() override;

    QQmlListProperty<QObject> data();

    QQuick3DCamera *camera() const;
    QQuick3DSceneEnvironment *environment() const;
    QQuick3DNode *scene() const;
    QQuick3DNode *referencedScene() const;
    QQuick3DView3DRenderMode renderMode() const;

    QQuick3DSceneRenderer *createRenderer() const;

    bool isTextureProvider() const override;
    QSGTextureProvider *textureProvider() const override;
    void releaseResources() override;

    static QSurfaceFormat idealSurfaceFormat();

    Q_INVOKABLE QVector3D worldToView(const QVector3D &worldPos) const;
    Q_INVOKABLE QVector3D viewToWorld(const QVector3D &viewPos) const;

protected:
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) override;

public Q_SLOTS:
    void setCamera(QQuick3DCamera *camera);
    void setEnvironment(QQuick3DSceneEnvironment * environment);
    void setScene(QQuick3DNode *sceneRoot);
    void setRenderMode(QQuick3DView3DRenderMode renderMode);

private Q_SLOTS:
    void invalidateSceneGraph();

Q_SIGNALS:
    void cameraChanged(QQuick3DCamera *camera);
    void environmentChanged(QQuick3DSceneEnvironment * environment);
    void sceneChanged(QQuick3DNode *sceneRoot);
    void renderModeChanged(QQuick3DView3DRenderMode renderMode);

private:
    Q_DISABLE_COPY(QQuick3DView3D)

    QQuick3DCamera *m_camera = nullptr;
    QQuick3DSceneEnvironment *m_environment = nullptr;
    QQuick3DNode *m_sceneRoot = nullptr;
    QQuick3DNode *m_referencedScene = nullptr;
    mutable SGFramebufferObjectNode *m_node = nullptr;
    mutable QQuick3DSGRenderNode *m_renderNode = nullptr;
    mutable QQuick3DSGDirectRenderer *m_directRenderer = nullptr;
    bool m_renderModeDirty = false;
    QQuick3DView3DRenderMode m_renderMode = Texture;

    QHash<QObject*, QMetaObject::Connection> m_connections;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuick3DView3D)

#endif // QDEMONVIEW3D_H
