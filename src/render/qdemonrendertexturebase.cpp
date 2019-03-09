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
#include <QtDemonRender/qdemonrendersampler.h>
#include <QtDemonRender/qdemonrendertexturebase.h>

#include <limits>

QT_BEGIN_NAMESPACE

QDemonRenderTextureBase::QDemonRenderTextureBase(const QDemonRef<QDemonRenderContext> &context,
                                                 QDemonRenderTextureTargetType texTarget)
    : m_context(context)
    , m_backend(context->backend())
    , m_handle(nullptr)
    , m_textureUnit(std::numeric_limits<quint32>::max())
    , m_samplerParamsDirty(true)
    , m_texStateDirty(false)
    , m_sampleCount(1)
    , m_format(QDemonRenderTextureFormat::Unknown)
    , m_texTarget(texTarget)
    , m_baseLevel(0)
    , m_maxLevel(1000)
    , m_maxMipLevel(0)
    , m_immutable(false)
{
    m_handle = m_backend->createTexture();
    m_sampler = new QDemonRenderTextureSampler(context);
}

QDemonRenderTextureBase::~QDemonRenderTextureBase()
{
    if (m_sampler)
        ::free(m_sampler);
    if (m_handle)
        m_backend->releaseTexture(m_handle);
}

void QDemonRenderTextureBase::setBaseLevel(qint32 value)
{
    if (m_baseLevel != value) {
        m_baseLevel = value;
        m_texStateDirty = true;
    }
}

void QDemonRenderTextureBase::setMaxLevel(qint32 value)
{
    if (m_maxLevel != value) {
        m_maxLevel = value;
        m_texStateDirty = true;
    }
}

void QDemonRenderTextureBase::setMinFilter(QDemonRenderTextureMinifyingOp value)
{
    if (m_sampler->m_minFilter != value) {
        m_sampler->m_minFilter = value;
        m_samplerParamsDirty = true;
    }
}

void QDemonRenderTextureBase::setMagFilter(QDemonRenderTextureMagnifyingOp value)
{
    if (m_sampler->m_magFilter != value) {
        m_sampler->m_magFilter = value;
        m_samplerParamsDirty = true;
    }
}

void QDemonRenderTextureBase::setTextureWrapS(QDemonRenderTextureCoordOp value)
{
    if (m_sampler->m_wrapS != value) {
        m_sampler->m_wrapS = value;
        m_samplerParamsDirty = true;
    }
}

void QDemonRenderTextureBase::setTextureWrapT(QDemonRenderTextureCoordOp value)
{
    if (m_sampler->m_wrapT != value) {
        m_sampler->m_wrapT = value;
        m_samplerParamsDirty = true;
    }
}

void QDemonRenderTextureBase::setTextureCompareMode(QDemonRenderTextureCompareMode value)
{
    if (m_sampler->m_compareMode != value) {
        m_sampler->m_compareMode = value;
        m_samplerParamsDirty = true;
    }
}

void QDemonRenderTextureBase::setTextureCompareFunc(QDemonRenderTextureCompareOp value)
{
    if (m_sampler->m_compareOp != value) {
        m_sampler->m_compareOp = value;
        m_samplerParamsDirty = true;
    }
}

void QDemonRenderTextureBase::applyTexParams()
{
    if (m_samplerParamsDirty) {
        m_backend->updateSampler(m_sampler->GetSamplerHandle(),
                                 m_texTarget,
                                 m_sampler->m_minFilter,
                                 m_sampler->m_magFilter,
                                 m_sampler->m_wrapS,
                                 m_sampler->m_wrapT,
                                 m_sampler->m_wrapR,
                                 m_sampler->m_minLod,
                                 m_sampler->m_maxLod,
                                 m_sampler->m_lodBias,
                                 m_sampler->m_compareMode,
                                 m_sampler->m_compareOp);

        m_samplerParamsDirty = false;
    }

    if (m_texStateDirty) {
        m_backend->updateTextureObject(m_handle, m_texTarget, m_baseLevel, m_maxLevel);
        m_texStateDirty = false;
    }
}

void QDemonRenderTextureBase::applyTexSwizzle()
{
    QDemonRenderTextureSwizzleMode theSwizzleMode = m_backend->getTextureSwizzleMode(m_format);
    if (theSwizzleMode != m_sampler->m_swizzleMode) {
        m_sampler->m_swizzleMode = theSwizzleMode;
        m_backend->updateTextureSwizzle(m_handle, m_texTarget, theSwizzleMode);
    }
}
QT_END_NAMESPACE
