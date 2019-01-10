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
#ifndef QDEMON_RENDER__RENDER_SAMPLER_H
#define QDEMON_RENDER__RENDER_SAMPLER_H

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrenderbackend.h>

QT_BEGIN_NAMESPACE

class QDemonRenderContextImpl;

class QDemonRenderTextureSampler
{
public:
    QDemonRenderTextureMinifyingOp::Enum m_MinFilter;
    QDemonRenderTextureMagnifyingOp::Enum m_MagFilter;
    QDemonRenderTextureCoordOp::Enum m_WrapS;
    QDemonRenderTextureCoordOp::Enum m_WrapT;
    QDemonRenderTextureCoordOp::Enum m_WrapR;
    QDemonRenderTextureSwizzleMode::Enum m_SwizzleMode;
    float m_MinLod;
    float m_MaxLod;
    float m_LodBias;
    QDemonRenderTextureCompareMode::Enum m_CompareMode;
    QDemonRenderTextureCompareOp::Enum m_CompareOp;
    float m_Anisotropy;
    float m_BorderColor[4];

    /**
         * @brief constructor
         *
         * @param[in] context		Pointer to context
         * @param[in] fnd			Pointer to foundation
         * @param[in] minFilter		Texture min filter
         * @param[in] magFilter		Texture mag filter
         * @param[in] wrapS			Texture coord generation for S
         * @param[in] wrapT			Texture coord generation for T
         * @param[in] wrapR			Texture coord generation for R
         * @param[in] swizzleMode	Texture swizzle mode
         * @param[in] minLod		Texture min level of detail
         * @param[in] maxLod		Texture max level of detail
         * @param[in] lodBias		Texture level of detail bias (unused)
         * @param[in] compareMode	Texture compare mode
         * @param[in] compareFunc	Texture compare function
         * @param[in] anisoFilter	Aniso filter value [1.0, 16.0]
         * @param[in] borderColor	Texture border color float[4] (unused)
         *
         * @return No return.
         */
    QDemonRenderTextureSampler(
            QSharedPointer<QDemonRenderContextImpl> context,
            QDemonRenderTextureMinifyingOp::Enum minFilter = QDemonRenderTextureMinifyingOp::Linear,
            QDemonRenderTextureMagnifyingOp::Enum magFilter = QDemonRenderTextureMagnifyingOp::Linear,
            QDemonRenderTextureCoordOp::Enum wrapS = QDemonRenderTextureCoordOp::ClampToEdge,
            QDemonRenderTextureCoordOp::Enum wrapT = QDemonRenderTextureCoordOp::ClampToEdge,
            QDemonRenderTextureCoordOp::Enum wrapR = QDemonRenderTextureCoordOp::ClampToEdge,
            QDemonRenderTextureSwizzleMode::Enum swizzleMode = QDemonRenderTextureSwizzleMode::NoSwizzle,
            float minLod = -1000.0, float maxLod = 1000.0, float lodBias = 0.0,
            QDemonRenderTextureCompareMode::Enum compareMode = QDemonRenderTextureCompareMode::NoCompare,
            QDemonRenderTextureCompareOp::Enum compareFunc = QDemonRenderTextureCompareOp::LessThanOrEqual,
            float anisotropy = 1.0, float *borderColor = nullptr);

    /**
         * @brief destructor
         *
         */
    virtual ~QDemonRenderTextureSampler();

    /**
         * @brief get the backend object handle
         *
         * @return the backend object handle.
         */
    QDemonRenderBackend::QDemonRenderBackendSamplerObject GetSamplerHandle() const
    {
        return m_SamplerHandle;
    }

private:
    QSharedPointer<QDemonRenderContextImpl> m_Context; ///< pointer to context
    QSharedPointer<QDemonRenderBackend> m_Backend; ///< pointer to backend
    QDemonRenderBackend::QDemonRenderBackendSamplerObject m_SamplerHandle; ///< opaque backend handle
};

QT_END_NAMESPACE

#endif
