/****************************************************************************
**
** Copyright (C) 2008-2012 NVIDIA Corporation.
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt 3D Studio.
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
#ifndef QDEMON_RENDER_IMAGE_H
#define QDEMON_RENDER_IMAGE_H

#include <QtDemonRuntimeRender/qdemonrendergraphobject.h>
#include <QtDemonRuntimeRender/qdemonrendernode.h>
#include <QtDemonRuntimeRender/qdemonrenderimagetexturedata.h>
#include <QtDemonRuntimeRender/qtdemonruntimerenderglobal.h>
#include <QtDemonRender/qdemonrendertexture2d.h>

#include <QtGui/QVector2D>

QT_BEGIN_NAMESPACE
class QDemonRenderContextInterface;
class QDemonOffscreenRenderManagerInterface;
class QDemonOffscreenRendererInterface;

struct Q_DEMONRUNTIMERENDER_EXPORT QDemonRenderImage : public QDemonRenderGraphObject
{
    enum class Flag
    {
        Dirty = 1,
        TransformDirty = 1 << 1,
        Active = 1 << 2 ///< Is this exact object active
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    enum class MappingModes : quint8
    {
        Normal = 0, // UV mapping
        Environment = 1,
        LightProbe = 2,
    };

    Q_DISABLE_COPY(QDemonRenderImage)
    // Complete path to the file;
    //*not* relative to the presentation directory
    QString m_imagePath;
    QString m_imageShaderName; ///< for custom materials we don't generate the name

    // Presentation id.
    QString m_offscreenRendererId; // overrides source path if available
    QDemonRef<QDemonOffscreenRendererInterface> m_lastFrameOffscreenRenderer;
    QDemonRenderGraphObject *m_parent;

    QDemonRenderImageTextureData m_textureData;

    Flags m_flags; // only dirty, transform dirty, and active apply

    QVector2D m_scale;
    QVector2D m_pivot;
    float m_rotation; // Radians.
    QVector2D m_position;
    MappingModes m_mappingMode;
    QDemonRenderTextureCoordOp m_horizontalTilingMode;
    QDemonRenderTextureCoordOp m_verticalTilingMode;

    // Setting any of the above variables means this object is dirty.
    // Setting any of the vec2 properties means this object's transform is dirty

    QMatrix4x4 m_textureTransform;

    QDemonRenderImage();
    ~QDemonRenderImage();
    // Renders the sub presentation
    // Or finds the image.
    // and sets up the texture transform
    bool clearDirty(QDemonBufferManager &inBufferManager,
                    QDemonOffscreenRenderManagerInterface &inRenderManager,
                    /*IRenderPluginManager &pluginManager,*/
                    bool forIbl = false);

    void calculateTextureTransform();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDemonRenderImage::Flags)

QT_END_NAMESPACE

#endif
