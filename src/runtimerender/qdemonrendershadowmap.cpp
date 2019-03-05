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

#include <QtDemonRuntimeRender/qdemonrenderlayer.h>
#include <QtDemonRuntimeRender/qdemonrendershadowmap.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcemanager.h>
#include <QtDemonRuntimeRender/qdemonrendererimpllayerrenderdata.h>
#include <QtDemonRender/qdemonrendershaderconstant.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>

QT_BEGIN_NAMESPACE

QDemonRenderShadowMap::QDemonRenderShadowMap(QDemonRenderContextInterface *inContext) : m_context(inContext) {}

QDemonRenderShadowMap::~QDemonRenderShadowMap()
{
    m_shadowMapList.clear();
}

namespace {
bool IsDepthFormat(QDemonRenderTextureFormats::Enum format)
{
    switch (format) {
    case QDemonRenderTextureFormats::Depth16:
    case QDemonRenderTextureFormats::Depth24:
    case QDemonRenderTextureFormats::Depth32:
    case QDemonRenderTextureFormats::Depth24Stencil8:
        return true;
    default:
        return false;
    }
}
}

void QDemonRenderShadowMap::addShadowMapEntry(quint32 index,
                                              quint32 width,
                                              quint32 height,
                                              QDemonRenderTextureFormats::Enum format,
                                              quint32 samples,
                                              ShadowMapModes::Enum mode,
                                              ShadowFilterValues::Enum filter)
{
    QDemonRef<QDemonResourceManagerInterface> theManager(m_context->getResourceManager());
    QDemonShadowMapEntry *pEntry = nullptr;

    if (index < m_shadowMapList.size())
        pEntry = &m_shadowMapList[index];

    if (pEntry) {
        if ((nullptr != pEntry->m_depthMap) && (mode == ShadowMapModes::CUBE)) {
            theManager->release(pEntry->m_depthMap);
            theManager->release(pEntry->m_depthCopy);
            theManager->release(pEntry->m_depthRender);
            pEntry->m_depthCube = theManager->allocateTextureCube(width, height, format, samples);
            pEntry->m_cubeCopy = theManager->allocateTextureCube(width, height, format, samples);
            pEntry->m_depthRender = theManager->allocateTexture2D(width, height, QDemonRenderTextureFormats::Depth24Stencil8, samples);
            pEntry->m_depthMap = nullptr;
            pEntry->m_depthCopy = nullptr;
        } else if ((nullptr != pEntry->m_depthCube) && (mode != ShadowMapModes::CUBE)) {
            theManager->release(pEntry->m_depthCube);
            theManager->release(pEntry->m_cubeCopy);
            theManager->release(pEntry->m_depthRender);
            pEntry->m_depthMap = theManager->allocateTexture2D(width, height, format, samples);
            pEntry->m_depthCopy = theManager->allocateTexture2D(width, height, format, samples);
            pEntry->m_depthCube = nullptr;
            pEntry->m_cubeCopy = nullptr;
            pEntry->m_depthRender = theManager->allocateTexture2D(width, height, QDemonRenderTextureFormats::Depth24Stencil8, samples);
        } else if (nullptr != pEntry->m_depthMap) {
            QDemonTextureDetails theDetails(pEntry->m_depthMap->getTextureDetails());

            // If anything differs about the map we're looking for, let's recreate it.
            if (theDetails.format != format || theDetails.width != width || theDetails.height != height
                || theDetails.sampleCount != samples) {
                // release texture
                theManager->release(pEntry->m_depthMap);
                theManager->release(pEntry->m_depthCopy);
                theManager->release(pEntry->m_depthRender);
                pEntry->m_depthMap = theManager->allocateTexture2D(width, height, format, samples);
                pEntry->m_depthCopy = theManager->allocateTexture2D(width, height, format, samples);
                pEntry->m_depthCube = nullptr;
                pEntry->m_cubeCopy = nullptr;
                pEntry->m_depthRender = theManager->allocateTexture2D(width, height, QDemonRenderTextureFormats::Depth24Stencil8, samples);
            }
        } else {
            QDemonTextureDetails theDetails(pEntry->m_depthCube->getTextureDetails());

            // If anything differs about the map we're looking for, let's recreate it.
            if (theDetails.format != format || theDetails.width != width || theDetails.height != height
                || theDetails.sampleCount != samples) {
                // release texture
                theManager->release(pEntry->m_depthCube);
                theManager->release(pEntry->m_cubeCopy);
                theManager->release(pEntry->m_depthRender);
                pEntry->m_depthCube = theManager->allocateTextureCube(width, height, format, samples);
                pEntry->m_cubeCopy = theManager->allocateTextureCube(width, height, format, samples);
                pEntry->m_depthRender = theManager->allocateTexture2D(width, height, QDemonRenderTextureFormats::Depth24Stencil8, samples);
                pEntry->m_depthMap = nullptr;
                pEntry->m_depthCopy = nullptr;
            }
        }

        pEntry->m_shadowMapMode = mode;
        pEntry->m_shadowFilterFlags = filter;
    } else if (mode == ShadowMapModes::CUBE) {
        QDemonRef<QDemonRenderTextureCube> theDepthTex = theManager->allocateTextureCube(width, height, format, samples);
        QDemonRef<QDemonRenderTextureCube> theDepthCopy = theManager->allocateTextureCube(width, height, format, samples);
        QDemonRef<QDemonRenderTexture2D> theDepthTemp = theManager->allocateTexture2D(width,
                                                                                      height,
                                                                                      QDemonRenderTextureFormats::Depth24Stencil8,
                                                                                      samples);
        m_shadowMapList.push_back(QDemonShadowMapEntry(index, mode, filter, theDepthTex, theDepthCopy, theDepthTemp));

        pEntry = &m_shadowMapList.back();
    } else {
        QDemonRef<QDemonRenderTexture2D> theDepthMap = theManager->allocateTexture2D(width, height, format, samples);
        QDemonRef<QDemonRenderTexture2D> theDepthCopy = theManager->allocateTexture2D(width, height, format, samples);
        QDemonRef<QDemonRenderTexture2D> theDepthTemp = theManager->allocateTexture2D(width,
                                                                                      height,
                                                                                      QDemonRenderTextureFormats::Depth24Stencil8,
                                                                                      samples);
        m_shadowMapList.push_back(QDemonShadowMapEntry(index, mode, filter, theDepthMap, theDepthCopy, theDepthTemp));

        pEntry = &m_shadowMapList.back();
    }

    if (pEntry) {
        // setup some texture settings
        if (pEntry->m_depthMap) {
            pEntry->m_depthMap->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
            pEntry->m_depthMap->setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
            pEntry->m_depthMap->setTextureWrapS(QDemonRenderTextureCoordOp::ClampToEdge);
            pEntry->m_depthMap->setTextureWrapT(QDemonRenderTextureCoordOp::ClampToEdge);

            pEntry->m_depthCopy->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
            pEntry->m_depthCopy->setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
            pEntry->m_depthCopy->setTextureWrapS(QDemonRenderTextureCoordOp::ClampToEdge);
            pEntry->m_depthCopy->setTextureWrapT(QDemonRenderTextureCoordOp::ClampToEdge);

            pEntry->m_depthRender->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
            pEntry->m_depthRender->setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
            pEntry->m_depthRender->setTextureWrapS(QDemonRenderTextureCoordOp::ClampToEdge);
            pEntry->m_depthRender->setTextureWrapT(QDemonRenderTextureCoordOp::ClampToEdge);
        } else {
            pEntry->m_depthCube->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
            pEntry->m_depthCube->setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
            pEntry->m_depthCube->setTextureWrapS(QDemonRenderTextureCoordOp::ClampToEdge);
            pEntry->m_depthCube->setTextureWrapT(QDemonRenderTextureCoordOp::ClampToEdge);

            pEntry->m_cubeCopy->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
            pEntry->m_cubeCopy->setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
            pEntry->m_cubeCopy->setTextureWrapS(QDemonRenderTextureCoordOp::ClampToEdge);
            pEntry->m_cubeCopy->setTextureWrapT(QDemonRenderTextureCoordOp::ClampToEdge);

            pEntry->m_depthRender->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
            pEntry->m_depthRender->setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
            pEntry->m_depthRender->setTextureWrapS(QDemonRenderTextureCoordOp::ClampToEdge);
            pEntry->m_depthRender->setTextureWrapT(QDemonRenderTextureCoordOp::ClampToEdge);
        }

        pEntry->m_lightIndex = index;
    }
}

QDemonShadowMapEntry *QDemonRenderShadowMap::getShadowMapEntry(int index)
{
    if (index < 0) {
        Q_UNREACHABLE();
        return nullptr;
    }
    QDemonShadowMapEntry *pEntry = nullptr;

    for (int i = 0; i < m_shadowMapList.size(); i++) {
        pEntry = &m_shadowMapList[i];
        if (pEntry->m_lightIndex == quint32(index))
            return pEntry;
    }

    return nullptr;
}

QDemonRef<QDemonRenderShadowMap> QDemonRenderShadowMap::create(QDemonRenderContextInterface *inContext)
{
    return QDemonRef<QDemonRenderShadowMap>(new QDemonRenderShadowMap(inContext));
}

QT_END_NAMESPACE
