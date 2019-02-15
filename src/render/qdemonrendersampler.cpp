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

QDemonRenderTextureSampler::QDemonRenderTextureSampler(QSharedPointer<QDemonRenderContextImpl> context,
                                                       QDemonRenderTextureMinifyingOp::Enum minFilter,
                                                       QDemonRenderTextureMagnifyingOp::Enum magFilter,
                                                       QDemonRenderTextureCoordOp::Enum wrapS,
                                                       QDemonRenderTextureCoordOp::Enum wrapT,
                                                       QDemonRenderTextureCoordOp::Enum wrapR,
                                                       QDemonRenderTextureSwizzleMode::Enum swizzleMode,
                                                       float minLod,
                                                       float maxLod,
                                                       float lodBias,
                                                       QDemonRenderTextureCompareMode::Enum compareMode,
                                                       QDemonRenderTextureCompareOp::Enum compareFunc,
                                                       float anisotropy,
                                                       float *borderColor)
    : m_minFilter(minFilter)
    , m_magFilter(magFilter)
    , m_wrapS(wrapS)
    , m_wrapT(wrapT)
    , m_wrapR(wrapR)
    , m_swizzleMode(swizzleMode)
    , m_minLod(minLod)
    , m_maxLod(maxLod)
    , m_lodBias(lodBias)
    , m_compareMode(compareMode)
    , m_compareOp(compareFunc)
    , m_anisotropy(anisotropy)
    , m_context(context)
    , m_backend(context->getBackend())
    , m_samplerHandle(nullptr)
{
    // create backend handle
    m_samplerHandle = m_backend->createSampler();

    if (borderColor) {
        m_borderColor[0] = borderColor[0];
        m_borderColor[1] = borderColor[1];
        m_borderColor[2] = borderColor[2];
        m_borderColor[3] = borderColor[3];
    }
}

QDemonRenderTextureSampler::~QDemonRenderTextureSampler()
{
    if (m_samplerHandle)
        m_backend->releaseSampler(m_samplerHandle);
}

QT_END_NAMESPACE
