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

#include <Qt3DSRenderLayer.h>
#include <Qt3DSRenderShadowMap.h>
#include <Qt3DSRenderResourceManager.h>
#include <Qt3DSRendererImplLayerRenderData.h>
#include <Qt3DSRenderShaderConstant.h>
#include <qdemonrendershaderprogram.h>

using namespace render;
using QDemonRenderContextScopedProperty;
using NVRenderCachedShaderProperty;

Qt3DSShadowMap::Qt3DSShadowMap(IQt3DSRenderContext &inContext)
    : m_Context(inContext)
    , mRefCount(0)
    , m_ShadowMapList(inContext.GetAllocator(), "Qt3DSShadowMap::m_ShadowMapList")
{
}

Qt3DSShadowMap::~Qt3DSShadowMap()
{
    m_ShadowMapList.clear();
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

void Qt3DSShadowMap::AddShadowMapEntry(quint32 index, quint32 width, quint32 height,
                                     QDemonRenderTextureFormats::Enum format, quint32 samples,
                                     ShadowMapModes::Enum mode, ShadowFilterValues::Enum filter)
{
    IResourceManager &theManager(m_Context.GetResourceManager());
    SShadowMapEntry *pEntry = nullptr;

    if (index < m_ShadowMapList.size())
        pEntry = &m_ShadowMapList[index];

    if (pEntry) {
        if ((nullptr != pEntry->m_DepthMap) && (mode == ShadowMapModes::CUBE)) {
            theManager.Release(*pEntry->m_DepthMap);
            theManager.Release(*pEntry->m_DepthCopy);
            theManager.Release(*pEntry->m_DepthRender);
            pEntry->m_DepthCube = theManager.AllocateTextureCube(width, height, format, samples);
            pEntry->m_CubeCopy = theManager.AllocateTextureCube(width, height, format, samples);
            pEntry->m_DepthRender = theManager.AllocateTexture2D(
                width, height, QDemonRenderTextureFormats::Depth24Stencil8, samples);
            pEntry->m_DepthMap = nullptr;
            pEntry->m_DepthCopy = nullptr;
        } else if ((nullptr != pEntry->m_DepthCube) && (mode != ShadowMapModes::CUBE)) {
            theManager.Release(*pEntry->m_DepthCube);
            theManager.Release(*pEntry->m_CubeCopy);
            theManager.Release(*pEntry->m_DepthRender);
            pEntry->m_DepthMap = theManager.AllocateTexture2D(width, height, format, samples);
            pEntry->m_DepthCopy = theManager.AllocateTexture2D(width, height, format, samples);
            pEntry->m_DepthCube = nullptr;
            pEntry->m_CubeCopy = nullptr;
            pEntry->m_DepthRender = theManager.AllocateTexture2D(
                width, height, QDemonRenderTextureFormats::Depth24Stencil8, samples);
        } else if (nullptr != pEntry->m_DepthMap) {
            STextureDetails theDetails(pEntry->m_DepthMap->GetTextureDetails());

            // If anything differs about the map we're looking for, let's recreate it.
            if (theDetails.m_Format != format || theDetails.m_Width != width
                || theDetails.m_Height != height || theDetails.m_SampleCount != samples) {
                // release texture
                theManager.Release(*pEntry->m_DepthMap);
                theManager.Release(*pEntry->m_DepthCopy);
                theManager.Release(*pEntry->m_DepthRender);
                pEntry->m_DepthMap = theManager.AllocateTexture2D(width, height, format, samples);
                pEntry->m_DepthCopy = theManager.AllocateTexture2D(width, height, format, samples);
                pEntry->m_DepthCube = nullptr;
                pEntry->m_CubeCopy = nullptr;
                pEntry->m_DepthRender = theManager.AllocateTexture2D(
                    width, height, QDemonRenderTextureFormats::Depth24Stencil8, samples);
            }
        } else {
            STextureDetails theDetails(pEntry->m_DepthCube->GetTextureDetails());

            // If anything differs about the map we're looking for, let's recreate it.
            if (theDetails.m_Format != format || theDetails.m_Width != width
                || theDetails.m_Height != height || theDetails.m_SampleCount != samples) {
                // release texture
                theManager.Release(*pEntry->m_DepthCube);
                theManager.Release(*pEntry->m_CubeCopy);
                theManager.Release(*pEntry->m_DepthRender);
                pEntry->m_DepthCube =
                    theManager.AllocateTextureCube(width, height, format, samples);
                pEntry->m_CubeCopy = theManager.AllocateTextureCube(width, height, format, samples);
                pEntry->m_DepthRender = theManager.AllocateTexture2D(
                    width, height, QDemonRenderTextureFormats::Depth24Stencil8, samples);
                pEntry->m_DepthMap = nullptr;
                pEntry->m_DepthCopy = nullptr;
            }
        }

        pEntry->m_ShadowMapMode = mode;
        pEntry->m_ShadowFilterFlags = filter;
    } else if (mode == ShadowMapModes::CUBE) {
        QDemonRenderTextureCube *theDepthTex =
            theManager.AllocateTextureCube(width, height, format, samples);
        QDemonRenderTextureCube *theDepthCopy =
            theManager.AllocateTextureCube(width, height, format, samples);
        QDemonRenderTexture2D *theDepthTemp = theManager.AllocateTexture2D(
            width, height, QDemonRenderTextureFormats::Depth24Stencil8, samples);

        m_ShadowMapList.push_back(
            SShadowMapEntry(index, mode, filter, *theDepthTex, *theDepthCopy, *theDepthTemp));

        pEntry = &m_ShadowMapList.back();
    } else {
        QDemonRenderTexture2D *theDepthMap =
            theManager.AllocateTexture2D(width, height, format, samples);
        QDemonRenderTexture2D *theDepthCopy =
            theManager.AllocateTexture2D(width, height, format, samples);
        QDemonRenderTexture2D *theDepthTemp = theManager.AllocateTexture2D(
            width, height, QDemonRenderTextureFormats::Depth24Stencil8, samples);

        m_ShadowMapList.push_back(
            SShadowMapEntry(index, mode, filter, *theDepthMap, *theDepthCopy, *theDepthTemp));

        pEntry = &m_ShadowMapList.back();
    }

    if (pEntry) {
        // setup some texture settings
        if (pEntry->m_DepthMap) {
            pEntry->m_DepthMap->SetMinFilter(QDemonRenderTextureMinifyingOp::Linear);
            pEntry->m_DepthMap->SetMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
            pEntry->m_DepthMap->SetTextureWrapS(QDemonRenderTextureCoordOp::ClampToEdge);
            pEntry->m_DepthMap->SetTextureWrapT(QDemonRenderTextureCoordOp::ClampToEdge);

            pEntry->m_DepthCopy->SetMinFilter(QDemonRenderTextureMinifyingOp::Linear);
            pEntry->m_DepthCopy->SetMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
            pEntry->m_DepthCopy->SetTextureWrapS(QDemonRenderTextureCoordOp::ClampToEdge);
            pEntry->m_DepthCopy->SetTextureWrapT(QDemonRenderTextureCoordOp::ClampToEdge);

            pEntry->m_DepthRender->SetMinFilter(QDemonRenderTextureMinifyingOp::Linear);
            pEntry->m_DepthRender->SetMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
            pEntry->m_DepthRender->SetTextureWrapS(QDemonRenderTextureCoordOp::ClampToEdge);
            pEntry->m_DepthRender->SetTextureWrapT(QDemonRenderTextureCoordOp::ClampToEdge);
        } else {
            pEntry->m_DepthCube->SetMinFilter(QDemonRenderTextureMinifyingOp::Linear);
            pEntry->m_DepthCube->SetMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
            pEntry->m_DepthCube->SetTextureWrapS(QDemonRenderTextureCoordOp::ClampToEdge);
            pEntry->m_DepthCube->SetTextureWrapT(QDemonRenderTextureCoordOp::ClampToEdge);

            pEntry->m_CubeCopy->SetMinFilter(QDemonRenderTextureMinifyingOp::Linear);
            pEntry->m_CubeCopy->SetMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
            pEntry->m_CubeCopy->SetTextureWrapS(QDemonRenderTextureCoordOp::ClampToEdge);
            pEntry->m_CubeCopy->SetTextureWrapT(QDemonRenderTextureCoordOp::ClampToEdge);

            pEntry->m_DepthRender->SetMinFilter(QDemonRenderTextureMinifyingOp::Linear);
            pEntry->m_DepthRender->SetMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
            pEntry->m_DepthRender->SetTextureWrapS(QDemonRenderTextureCoordOp::ClampToEdge);
            pEntry->m_DepthRender->SetTextureWrapT(QDemonRenderTextureCoordOp::ClampToEdge);
        }

        pEntry->m_LightIndex = index;
    }
}

SShadowMapEntry *Qt3DSShadowMap::GetShadowMapEntry(quint32 index)
{
    SShadowMapEntry *pEntry = nullptr;

    for (quint32 i = 0; i < m_ShadowMapList.size(); i++) {
        pEntry = &m_ShadowMapList[i];
        if (pEntry->m_LightIndex == index)
            return pEntry;
    }

    return nullptr;
}

Qt3DSShadowMap *Qt3DSShadowMap::Create(IQt3DSRenderContext &inContext)
{
    return QDEMON_NEW(inContext.GetFoundation().getAllocator(), Qt3DSShadowMap)(inContext);
}
