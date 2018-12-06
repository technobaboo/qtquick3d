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
#pragma once
#ifndef QDEMON_RENDER_SHADOW_MAP_H
#define QDEMON_RENDER_SHADOW_MAP_H
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <Qt3DSAtomic.h>
#include <QMatrix4x4.h>
#include <QVector3D.h>
#include <Qt3DSFlags.h>
#include <StringTable.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <Qt3DSRenderTexture2D.h>
#ifdef _INTEGRITYPLATFORM
#include <Qt3DSRenderTextureCube.h>
#endif

QT_BEGIN_NAMESPACE

struct SLayerRenderData;

struct ShadowMapModes
{
    enum Enum {
        SSM, ///< standard shadow mapping
        VSM, ///< variance shadow mapping
        CUBE, ///< cubemap omnidirectional shadows
    };
};

struct ShadowFilterValues
{
    enum Enum {
        NONE = 1 << 0, ///< hard shadows
        PCF = 1 << 1, ///< Percentage close filtering
        BLUR = 1 << 2, ///< Gausian Blur
    };
};

struct SShadowMapEntry
{
    SShadowMapEntry()
        : m_LightIndex(QDEMON_MAX_U32)
        , m_ShadowMapMode(ShadowMapModes::SSM)
        , m_ShadowFilterFlags(ShadowFilterValues::NONE)
    {
    }

    SShadowMapEntry(quint32 index, ShadowMapModes::Enum mode, ShadowFilterValues::Enum filter,
                    QDemonRenderTexture2D &depthMap, QDemonRenderTexture2D &depthCopy,
                    QDemonRenderTexture2D &depthTemp)
        : m_LightIndex(index)
        , m_ShadowMapMode(mode)
        , m_ShadowFilterFlags(filter)
        , m_DepthMap(depthMap)
        , m_DepthCopy(depthCopy)
        , m_DepthCube(nullptr)
        , m_CubeCopy(nullptr)
        , m_DepthRender(depthTemp)
    {
    }

    SShadowMapEntry(quint32 index, ShadowMapModes::Enum mode, ShadowFilterValues::Enum filter,
                    QDemonRenderTextureCube &depthCube, QDemonRenderTextureCube &cubeTmp,
                    QDemonRenderTexture2D &depthTemp)
        : m_LightIndex(index)
        , m_ShadowMapMode(mode)
        , m_ShadowFilterFlags(filter)
        , m_DepthMap(nullptr)
        , m_DepthCopy(nullptr)
        , m_DepthCube(depthCube)
        , m_CubeCopy(cubeTmp)
        , m_DepthRender(depthTemp)
    {
    }

    quint32 m_LightIndex; ///< the light index it belongs to
    ShadowMapModes::Enum m_ShadowMapMode; ///< shadow map method
    ShadowFilterValues::Enum m_ShadowFilterFlags; ///< shadow filter mode

    // PKC : Adding the DepthRender buffer allows us to have a depth+stencil format when filling
    // the shadow maps (depth+stencil is necessary), but use a more compact format for the
    // actual
    // shadow map used at shade time.  See if it's worth adding.
    QDemonScopedRefCounted<QDemonRenderTexture2D> m_DepthMap; ///< shadow map texture
    QDemonScopedRefCounted<QDemonRenderTexture2D>
    m_DepthCopy; ///< shadow map buffer used during blur passes
    QDemonScopedRefCounted<QDemonRenderTextureCube> m_DepthCube; ///< shadow cube map
    QDemonScopedRefCounted<QDemonRenderTextureCube>
    m_CubeCopy; ///< cube map buffer used during the blur passes
    QDemonScopedRefCounted<QDemonRenderTexture2D>
    m_DepthRender; ///< shadow depth+stencil map used during rendering

    QMatrix4x4 m_LightVP; ///< light view projection matrix
    QMatrix4x4 m_LightCubeView[6]; ///< light cubemap view matrices
    QMatrix4x4 m_LightView; ///< light view transform
};

class Qt3DSShadowMap : public QDemonRefCounted
{
    typedef QVector<SShadowMapEntry> TShadowMapEntryList;

public:
    IQt3DSRenderContext &m_Context;
    volatile qint32 mRefCount;

public:
    Qt3DSShadowMap(IQt3DSRenderContext &inContext);
    ~Qt3DSShadowMap();

    QDEMON_IMPLEMENT_REF_COUNT_ADDREF_RELEASE(m_Context.GetAllocator())

    /*
         * @brief Add a shadow map entry
         *		  This creates a new shadow map if it does not exist or changed
         *
         * @param[in] index		shadow map entry index
         * @param[in] width		shadow map width
         * @param[in] height	shadow map height
         * @param[in] format	shadow map format
         * @param[in] samples	shadow map sample count
         * @param[in] mode		shadow map mode like SSM, VCM
         * @param[in] filter	soft shadow map mode filter like PCF
         *
         * @ return no return
         */
    void AddShadowMapEntry(quint32 index, quint32 width, quint32 height,
                           QDemonRenderTextureFormats::Enum format, quint32 samples,
                           ShadowMapModes::Enum mode, ShadowFilterValues::Enum filter);

    /*
         * @brief Get a shadow map entry
         *
         * @param[in] index		shadow map entry index
         *
         * @ return shadow map entry or nullptr
         */
    SShadowMapEntry *GetShadowMapEntry(quint32 index);

    /*
         * @brief Get shadow map entry count
         *
         * @ return count of shadow map entries
         */
    quint32 GetShadowMapEntryCount() { return m_ShadowMapList.size(); }

    static Qt3DSShadowMap *Create(IQt3DSRenderContext &inContext);

private:
    TShadowMapEntryList m_ShadowMapList; ///< List of shadow map entries
};
QT_END_NAMESPACE

#endif
