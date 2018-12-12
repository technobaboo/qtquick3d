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

#include <QtDemonRender/qdemonrendercontext.h>
#include <qdemonrendersampler.h>

QT_BEGIN_NAMESPACE

QDemonRenderTextureSampler::QDemonRenderTextureSampler(
        QSharedPointer<QDemonRenderContextImpl> context,
        QDemonRenderTextureMinifyingOp::Enum minFilter, QDemonRenderTextureMagnifyingOp::Enum magFilter,
        QDemonRenderTextureCoordOp::Enum wrapS, QDemonRenderTextureCoordOp::Enum wrapT,
        QDemonRenderTextureCoordOp::Enum wrapR, QDemonRenderTextureSwizzleMode::Enum swizzleMode,
        float minLod, float maxLod, float lodBias, QDemonRenderTextureCompareMode::Enum compareMode,
        QDemonRenderTextureCompareOp::Enum compareFunc, float anisotropy, float *borderColor)
    : m_MinFilter(minFilter)
    , m_MagFilter(magFilter)
    , m_WrapS(wrapS)
    , m_WrapT(wrapT)
    , m_WrapR(wrapR)
    , m_SwizzleMode(swizzleMode)
    , m_MinLod(minLod)
    , m_MaxLod(maxLod)
    , m_LodBias(lodBias)
    , m_CompareMode(compareMode)
    , m_CompareOp(compareFunc)
    , m_Anisotropy(anisotropy)
    , m_Context(context)
    , m_Backend(context->GetBackend())
    , m_SamplerHandle(nullptr)
{
    // create backend handle
    m_SamplerHandle = m_Backend->CreateSampler();

    if (borderColor) {
        m_BorderColor[0] = borderColor[0];
        m_BorderColor[1] = borderColor[1];
        m_BorderColor[2] = borderColor[2];
        m_BorderColor[3] = borderColor[3];
    }
}

QDemonRenderTextureSampler::~QDemonRenderTextureSampler()
{
    if (m_SamplerHandle)
        m_Backend->ReleaseSampler(m_SamplerHandle);
}

QT_END_NAMESPACE
