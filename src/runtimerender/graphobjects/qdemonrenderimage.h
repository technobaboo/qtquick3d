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
struct ImageMappingModes
{
    enum Enum {
        Normal = 0, // UV mapping
        Environment = 1,
        LightProbe = 2,
    };
};

struct Q_DEMONRUNTIMERENDER_EXPORT QDemonRenderImage : public QDemonGraphObject
{
    Q_DISABLE_COPY(QDemonRenderImage)
    // Complete path to the file;
    //*not* relative to the presentation directory
    QString m_imagePath;
    QString m_imageShaderName; ///< for custom materials we don't generate the name

    // Presentation id.
    QString m_offscreenRendererId; // overrides source path if available
    QDemonRef<QDemonOffscreenRendererInterface> m_lastFrameOffscreenRenderer;
    QDemonGraphObject *m_parent;

    QDemonRenderImageTextureData m_textureData;

    QDemonNodeFlags m_flags; // only dirty, transform dirty, and active apply

    QVector2D m_scale;
    QVector2D m_pivot;
    float m_rotation; // Radians.
    QVector2D m_position;
    ImageMappingModes::Enum m_mappingMode;
    QDemonRenderTextureCoordOp::Enum m_horizontalTilingMode;
    QDemonRenderTextureCoordOp::Enum m_verticalTilingMode;

    // Setting any of the above variables means this object is dirty.
    // Setting any of the vec2 properties means this object's transform is dirty

    QMatrix4x4 m_textureTransform;

    QDemonRenderImage();
    ~QDemonRenderImage();
    // Renders the sub presentation
    // Or finds the image.
    // and sets up the texture transform
    bool clearDirty(QDemonBufferManagerInterface &inBufferManager,
                    QDemonOffscreenRenderManagerInterface &inRenderManager,
                    /*IRenderPluginManager &pluginManager,*/
                    bool forIbl = false);

    void calculateTextureTransform();

    // Generic method used during serialization
    // to remap string and object pointers
    template <typename TRemapperType>
    void remap(TRemapperType &inRemapper)
    {
        QDemonGraphObject::remap(inRemapper);
        inRemapper.remap(m_imagePath);
        inRemapper.remap(m_offscreenRendererId);
        // Null out objects that should be null when loading from file.
        inRemapper.NullPtr(m_lastFrameOffscreenRenderer);
        inRemapper.NullPtr(m_textureData.m_texture);
        inRemapper.remap(m_parent);
    }
};
QT_END_NAMESPACE

#endif
