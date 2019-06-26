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

#ifndef QDEMONIMAGE_H
#define QDEMONIMAGE_H

#include <QtQuick3D/QQuick3DObject>
#include <QtQuick/private/qquickitemchangelistener_p.h>
#include <QtQuick/QSGNode>
#include <QtCore/QUrl>

QT_BEGIN_NAMESPACE

class QQuickItem;
class QSGLayer;
struct QDemonRenderImage;
class Q_QUICK3D_EXPORT QQuick3DTexture : public QQuick3DObject, public QQuickItemChangeListener
{
    Q_OBJECT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QQuickItem *sourceItem READ sourceItem WRITE setSourceItem NOTIFY sourceItemChanged)
    Q_PROPERTY(float scaleU READ scaleU WRITE setScaleU NOTIFY scaleUChanged)
    Q_PROPERTY(float scaleV READ scaleV WRITE setScaleV NOTIFY scaleVChanged)
    Q_PROPERTY(MappingMode mappingMode READ mappingMode WRITE setMappingMode NOTIFY mappingModeChanged)
    Q_PROPERTY(TilingMode tilingModeHorizontal READ horizontalTiling WRITE setHorizontalTiling NOTIFY horizontalTilingChanged)
    Q_PROPERTY(TilingMode tilingModeVertical READ verticalTiling WRITE setVerticalTiling NOTIFY verticalTilingChanged)
    Q_PROPERTY(float rotationUV READ rotationUV WRITE setRotationUV NOTIFY rotationUVChanged)
    Q_PROPERTY(float positionU READ positionU WRITE setPositionU NOTIFY positionUChanged)
    Q_PROPERTY(float positionV READ positionV WRITE setPositionV NOTIFY positionVChanged)
    Q_PROPERTY(float pivotU READ pivotU WRITE setPivotU NOTIFY pivotUChanged)
    Q_PROPERTY(float pivotV READ pivotV WRITE setPivotV NOTIFY pivotVChanged)
    Q_PROPERTY(Format format READ format WRITE setFormat NOTIFY formatChanged)

public:
    enum MappingMode
    {
        Normal = 0, // UV mapping
        Environment = 1,
        LightProbe = 2,
    };
    Q_ENUM(MappingMode)

    enum TilingMode
    {
        Unknown = 0,
        ClampToEdge,
        MirroredRepeat,
        Repeat
    };
    Q_ENUM(TilingMode)

    enum Format {
        Automatic = 0,
        R8,
        R16,
        R16F,
        R32I,
        R32UI,
        R32F,
        RG8,
        RGBA8,
        RGB8,
        SRGB8,
        SRGB8A8,
        RGB565,
        RGBA5551,
        Alpha8,
        Luminance8,
        Luminance16,
        LuminanceAlpha8,
        RGBA16F,
        RG16F,
        RG32F,
        RGB32F,
        RGBA32F,
        R11G11B10,
        RGB9E5,
        RGBA_DXT1,
        RGB_DXT1,
        RGBA_DXT3,
        RGBA_DXT5,
        Depth16,
        Depth24,
        Depth32,
        Depth24Stencil8
    };
    Q_ENUM(Format)

    QQuick3DTexture();
    ~QQuick3DTexture() override;

    QUrl source() const;
    QQuickItem *sourceItem() const;
    float scaleU() const;
    float scaleV() const;
    MappingMode mappingMode() const;
    TilingMode horizontalTiling() const;
    TilingMode verticalTiling() const;
    float rotationUV() const;
    float positionU() const;
    float positionV() const;
    float pivotU() const;
    float pivotV() const;
    QQuick3DObject::Type type() const override;

    QDemonRenderImage *getRenderImage();

    Format format() const;

public Q_SLOTS:
    void setSource(const QUrl &source);
    void setSourceItem(QQuickItem *sourceItem);
    void setScaleU(float scaleU);
    void setScaleV(float scaleV);
    void setMappingMode(MappingMode mappingMode);
    void setHorizontalTiling(TilingMode tilingModeHorizontal);
    void setVerticalTiling(TilingMode tilingModeVertical);
    void setRotationUV(float rotationUV);
    void setPositionU(float positionU);
    void setPositionV(float positionV);
    void setPivotU(float pivotU);
    void setPivotV(float pivotV);
    void setFormat(Format format);

Q_SIGNALS:
    void sourceChanged(const QUrl &source);
    void sourceItemChanged(QQuickItem *sourceItem);
    void scaleUChanged(float scaleU);
    void scaleVChanged(float scaleV);
    void mappingModeChanged(MappingMode mappingMode);
    void horizontalTilingChanged(TilingMode tilingModeHorizontal);
    void verticalTilingChanged(TilingMode tilingModeVertical);
    void rotationUVChanged(float rotationUV);
    void positionUChanged(float positionU);
    void positionVChanged(float positionV);
    void pivotUChanged(float pivotU);
    void pivotVChanged(float pivotV);
    void formatChanged(Format format);

protected:
    QDemonRenderGraphObject *updateSpatialNode(QDemonRenderGraphObject *node) override;
    void itemChange(ItemChange change, const ItemChangeData &value) override;

    void itemGeometryChanged(QQuickItem *item, QQuickGeometryChange change, const QRectF &geometry) override;

private Q_SLOTS:
    void sourceItemDestroyed(QObject *item);

private:
    void ensureTexture();

    QUrl m_source;
    QQuickItem *m_sourceItem = nullptr;
    bool m_sourceItemReparented = false;
    QSGLayer *m_layer = nullptr;
    float m_scaleU = 1.0f;
    float m_scaleV = 1.0f;
    MappingMode m_mappingMode = Normal;
    TilingMode m_tilingModeHorizontal = ClampToEdge;
    TilingMode m_tilingModeVertical = ClampToEdge;
    float m_rotationUV = 0;
    float m_positionU = 0;
    float m_positionV = 0;
    float m_pivotU = 0;
    float m_pivotV = 0;
    Format m_format = Automatic;
};

QT_END_NAMESPACE

#endif // QDEMONIMAGE_H
