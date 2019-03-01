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
#ifndef QDEMON_RENDER_SHADOW_MAP_H
#define QDEMON_RENDER_SHADOW_MAP_H
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtGui/QMatrix4x4>
#include <QtGui/QVector3D>
#include <QtDemon/qdemonflags.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>

QT_BEGIN_NAMESPACE

struct QDemonLayerRenderData;

struct ShadowMapModes {
    enum Enum {
        SSM, ///< standard shadow mapping
        VSM, ///< variance shadow mapping
        CUBE, ///< cubemap omnidirectional shadows
    };
};

struct ShadowFilterValues {
    enum Enum {
        NONE = 1 << 0, ///< hard shadows
        PCF = 1 << 1, ///< Percentage close filtering
        BLUR = 1 << 2, ///< Gausian Blur
    };
};

struct QDemonShadowMapEntry {
    QDemonShadowMapEntry()
        : m_lightIndex(std::numeric_limits<quint32>::max())
        , m_shadowMapMode(ShadowMapModes::SSM)
        , m_shadowFilterFlags(ShadowFilterValues::NONE)
    {
    }

    QDemonShadowMapEntry(quint32 index,
                    ShadowMapModes::Enum mode,
                    ShadowFilterValues::Enum filter,
                    QDemonRef<QDemonRenderTexture2D> depthMap,
                    QDemonRef<QDemonRenderTexture2D> depthCopy,
                    QDemonRef<QDemonRenderTexture2D> depthTemp)
        : m_lightIndex(index)
        , m_shadowMapMode(mode)
        , m_shadowFilterFlags(filter)
        , m_depthMap(depthMap)
        , m_depthCopy(depthCopy)
        , m_depthCube(nullptr)
        , m_cubeCopy(nullptr)
        , m_depthRender(depthTemp)
    {
    }

    QDemonShadowMapEntry(quint32 index,
                    ShadowMapModes::Enum mode,
                    ShadowFilterValues::Enum filter,
                    QDemonRef<QDemonRenderTextureCube> depthCube,
                    QDemonRef<QDemonRenderTextureCube> cubeTmp,
                    QDemonRef<QDemonRenderTexture2D> depthTemp)
        : m_lightIndex(index)
        , m_shadowMapMode(mode)
        , m_shadowFilterFlags(filter)
        , m_depthMap(nullptr)
        , m_depthCopy(nullptr)
        , m_depthCube(depthCube)
        , m_cubeCopy(cubeTmp)
        , m_depthRender(depthTemp)
    {
    }

    quint32 m_lightIndex; ///< the light index it belongs to
    ShadowMapModes::Enum m_shadowMapMode; ///< shadow map method
    ShadowFilterValues::Enum m_shadowFilterFlags; ///< shadow filter mode

    // PKC : Adding the DepthRender buffer allows us to have a depth+stencil format when filling
    // the shadow maps (depth+stencil is necessary), but use a more compact format for the
    // actual
    // shadow map used at shade time.  See if it's worth adding.
    QDemonRef<QDemonRenderTexture2D> m_depthMap; ///< shadow map texture
    QDemonRef<QDemonRenderTexture2D> m_depthCopy; ///< shadow map buffer used during blur passes
    QDemonRef<QDemonRenderTextureCube> m_depthCube; ///< shadow cube map
    QDemonRef<QDemonRenderTextureCube> m_cubeCopy; ///< cube map buffer used during the blur passes
    QDemonRef<QDemonRenderTexture2D> m_depthRender; ///< shadow depth+stencil map used during rendering

    QMatrix4x4 m_lightVP; ///< light view projection matrix
    QMatrix4x4 m_lightCubeView[6]; ///< light cubemap view matrices
    QMatrix4x4 m_lightView; ///< light view transform
};

class QDemonRenderShadowMap
{
    typedef QVector<QDemonShadowMapEntry> TShadowMapEntryList;

public:
    QAtomicInt ref;
    QDemonRenderContextInterface *m_context;

    QDemonRenderShadowMap(QDemonRenderContextInterface *inContext);
    ~QDemonRenderShadowMap();

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
    void addShadowMapEntry(quint32 index,
                           quint32 width,
                           quint32 height,
                           QDemonRenderTextureFormats::Enum format,
                           quint32 samples,
                           ShadowMapModes::Enum mode,
                           ShadowFilterValues::Enum filter);

    /*
         * @brief Get a shadow map entry
         *
         * @param[in] index		shadow map entry index
         *
         * @ return shadow map entry or nullptr
         */
    QDemonShadowMapEntry *getShadowMapEntry(int index);

    /*
         * @brief Get shadow map entry count
         *
         * @ return count of shadow map entries
         */
    quint32 getShadowMapEntryCount() { return m_shadowMapList.size(); }

    static QDemonRef<QDemonRenderShadowMap> create(QDemonRenderContextInterface *inContext);

private:
    TShadowMapEntryList m_shadowMapList; ///< List of shadow map entries
};
QT_END_NAMESPACE

#endif
