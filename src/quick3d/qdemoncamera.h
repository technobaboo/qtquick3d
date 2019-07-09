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

#ifndef QDEMONCAMERA_H
#define QDEMONCAMERA_H

#include <QtQuick3d/qdemonnode.h>

QT_BEGIN_NAMESPACE

struct QDemonRenderCamera;
class Q_QUICK3D_EXPORT QDemonCamera : public QDemonNode
{
    Q_OBJECT
    Q_PROPERTY(float clipNear READ clipNear WRITE setClipNear NOTIFY clipNearChanged)
    Q_PROPERTY(float clipFar READ clipFar WRITE setClipFar NOTIFY clipFarChanged)
    Q_PROPERTY(float fieldOfView READ fieldOfView WRITE setFieldOfView NOTIFY fieldOfViewChanged)
    Q_PROPERTY(bool isFieldOFViewHorizontal READ isFieldOFViewHorizontal WRITE setIsFieldOFViewHorizontal NOTIFY isFieldOFViewHorizontalChanged)
    Q_PROPERTY(QDemonCameraProjectionMode projectionMode READ projectionMode WRITE setProjectionMode NOTIFY projectionModeChanged)
    Q_PROPERTY(QDemonCameraScaleModes scaleMode READ scaleMode WRITE setScaleMode NOTIFY scaleModeChanged)
    Q_PROPERTY(QDemonCameraScaleAnchors scaleAnchor READ scaleAnchor WRITE setScaleAnchor NOTIFY scaleAnchorChanged)
    Q_PROPERTY(bool enableFrustumCulling READ enableFrustumCulling WRITE setEnableFrustumCulling NOTIFY enableFrustumCullingChanged)

public:
    enum QDemonCameraScaleModes {
        Fit = 0,
        SameSize,
        FitHorizontal,
        FitVertical,
    };
    Q_ENUM(QDemonCameraScaleModes)

    enum QDemonCameraScaleAnchors {
        Center = 0,
        North,
        NorthEast,
        East,
        SouthEast,
        South,
        SouthWest,
        West,
        NorthWest,
    };
    Q_ENUM(QDemonCameraScaleAnchors)

    enum QDemonCameraProjectionMode {
        Perspective,
        Orthographic
    };
    Q_ENUM(QDemonCameraProjectionMode)

    QDemonCamera();

    float clipNear() const;
    float clipFar() const;
    float fieldOfView() const;
    bool isFieldOFViewHorizontal() const;
    QDemonCameraScaleModes scaleMode() const;
    QDemonCameraScaleAnchors scaleAnchor() const;
    QDemonObject::Type type() const override;
    QDemonCameraProjectionMode projectionMode() const;
    bool enableFrustumCulling() const;

    Q_INVOKABLE QVector3D worldToViewport(const QVector3D &worldPos) const;
    Q_INVOKABLE QVector3D viewportToWorld(const QVector3D &viewportPos) const;

    QDemonRenderCamera *getCameraNode() const;

public Q_SLOTS:
    void setClipNear(float clipNear);
    void setClipFar(float clipFar);
    void setFieldOfView(float fieldOfView);
    void setIsFieldOFViewHorizontal(bool isFieldOFViewHorizontal);
    void setScaleMode(QDemonCameraScaleModes scaleMode);
    void setScaleAnchor(QDemonCameraScaleAnchors scaleAnchor);
    void setProjectionMode(QDemonCameraProjectionMode projectionMode);
    void setEnableFrustumCulling(bool enableFrustumCulling);

Q_SIGNALS:
    void clipNearChanged(float clipNear);
    void clipFarChanged(float clipFar);
    void fieldOfViewChanged(float fieldOfView);
    void isFieldOFViewHorizontalChanged(bool isFieldOFViewHorizontal);
    void scaleModeChanged(QDemonCameraScaleModes scaleMode);
    void scaleAnchorChanged(QDemonCameraScaleAnchors scaleAnchor);
    void projectionModeChanged(QDemonCameraProjectionMode projectionMode);
    void enableFrustumCullingChanged(bool enableFrustumCulling);

protected:
    QDemonRenderGraphObject *updateSpatialNode(QDemonRenderGraphObject *node) override;

private:
    float m_clipNear = 10.0f;
    float m_clipFar = 10000.0f;
    float m_fieldOfView = 60.0f;
    QDemonCameraScaleModes m_scaleMode = QDemonCameraScaleModes::Fit;
    QDemonCameraScaleAnchors m_scaleAnchor = QDemonCameraScaleAnchors::Center;
    bool m_isFieldOFViewHorizontal = false;

    QDemonRenderCamera *m_cameraNode = nullptr;
    QDemonCameraProjectionMode m_projectionMode = QDemonCameraProjectionMode::Perspective;
    bool m_enableFrustumCulling = true;
};

QT_END_NAMESPACE

#endif // QDEMONCAMERA_H
