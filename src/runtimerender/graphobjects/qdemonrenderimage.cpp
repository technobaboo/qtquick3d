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
#include <QtDemonRuntimeRender/qdemonrenderimage.h>
#include <QtDemonRuntimeRender/qdemonrenderbuffermanager.h>
#include <QtDemonRuntimeRender/qdemonoffscreenrendermanager.h>
#include <QtDemonRuntimeRender/qdemonrenderprefiltertexture.h>
#include <qdemonoffscreenrenderkey.h>

QT_BEGIN_NAMESPACE

QDemonRenderImage::QDemonRenderImage()
    : QDemonGraphObject(QDemonGraphObjectTypes::Image)
    , m_lastFrameOffscreenRenderer(nullptr)
    , m_parent(nullptr)
    , m_scale(1, 1)
    , m_pivot(0, 0)
    , m_rotation(0)
    , m_position(0, 0)
    , m_mappingMode(ImageMappingModes::Normal)
    , m_horizontalTilingMode(QDemonRenderTextureCoordOp::ClampToEdge)
    , m_verticalTilingMode(QDemonRenderTextureCoordOp::ClampToEdge)
{
    m_flags.setActive(true);
    m_flags.setDirty(true);
    m_flags.setTransformDirty(true);
}

QDemonRenderImage::~QDemonRenderImage() = default;

static void HandleOffscreenResult(QDemonRenderImage &theImage,
                                  QDemonRenderImageTextureData &newImage,
                                  QDemonOffscreenRenderResult &theResult,
                                  bool &replaceTexture,
                                  bool &wasDirty)
{
    newImage.m_texture = theResult.texture;
    newImage.m_textureFlags.setHasTransparency(theResult.hasTransparency);
    newImage.m_textureFlags.setPreMultiplied(true);
    wasDirty = wasDirty || theResult.hasChangedSinceLastFrame;
    theImage.m_lastFrameOffscreenRenderer = theResult.renderer;
    replaceTexture = true;
}

bool QDemonRenderImage::clearDirty(QDemonBufferManager &inBufferManager,
                                   QDemonOffscreenRenderManagerInterface &inRenderManager,
                                   bool forIbl)
{

    bool wasDirty = m_flags.isDirty();
    m_flags.setDirty(false);
    QDemonRenderImageTextureData newImage;
    bool replaceTexture(false);
    if (newImage.m_texture == nullptr) {
        if (!m_offscreenRendererId.isEmpty()) {
            QDemonOffscreenRenderResult theResult = inRenderManager.getRenderedItem(m_offscreenRendererId);
            HandleOffscreenResult(*this, newImage, theResult, replaceTexture, wasDirty);
        }
    }

    if (newImage.m_texture == nullptr) {
        m_lastFrameOffscreenRenderer = nullptr;
        newImage = inBufferManager.loadRenderImage(m_imagePath, false, forIbl);
        replaceTexture = newImage.m_texture != m_textureData.m_texture;
    }

    if (replaceTexture) {
        wasDirty = true;
        m_textureData = newImage;
    }

    if (m_flags.isTransformDirty()) {
        wasDirty = true;
        calculateTextureTransform();
    }
    return wasDirty;
}

void QDemonRenderImage::calculateTextureTransform()
{
    m_flags.setTransformDirty(false);

    m_textureTransform = QMatrix4x4();

    QMatrix4x4 translation;
    QMatrix4x4 rotation;
    QMatrix4x4 scale;

    translation(3, 0) = m_position.x();
    translation(3, 1) = m_position.y();
    scale(0, 0) = m_scale.x();
    scale(1, 1) = m_scale.y();
    rotation.rotate(m_rotation, QVector3D(0, 0, 1));

    // Setup the pivot.
    m_textureTransform(3, 0) = m_pivot.x();
    m_textureTransform(3, 1) = m_pivot.y();
    m_textureTransform = m_textureTransform * rotation;
    m_textureTransform = m_textureTransform * scale;
    m_textureTransform = m_textureTransform * translation;
}

QDemonRenderImageTextureData::QDemonRenderImageTextureData() = default;
QDemonRenderImageTextureData::~QDemonRenderImageTextureData() = default;

QT_END_NAMESPACE
