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

#include <QtGui/QMatrix4x4>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>

#include <QtDemon/qdemonutils.h>

QT_BEGIN_NAMESPACE

QDemonRenderContextImpl::QDemonRenderContextImpl(QSharedPointer<QDemonRenderBackend> inBackend)
    : m_backend(inBackend)
    , m_DirtyFlags(0)
    , m_DefaultOffscreenRenderTarget((QDemonRenderBackend::QDemonRenderBackendRenderTargetObject)nullptr)
    , m_DephBits(16)
    , m_StencilBits(8)
    , m_NextTextureUnit(1)
    , m_NextConstantBufferUnit(1)
{
    m_MaxTextureUnits = m_backend->GetMaxCombinedTextureUnits();
    m_MaxConstantBufferUnits = 16; // need backend query

    // get initial state
    memZero(&m_HardwarePropertyContext, sizeof(m_HardwarePropertyContext));

    // get default blending functions
    m_backend->GetBlendFunc(&m_HardwarePropertyContext.m_BlendFunction);
    // set default blend euqation
    m_HardwarePropertyContext.m_BlendEquation.m_RGBEquation = QDemonRenderBlendEquation::Add;
    m_HardwarePropertyContext.m_BlendEquation.m_AlphaEquation = QDemonRenderBlendEquation::Add;
    // default state
    m_HardwarePropertyContext.m_CullingEnabled =
            m_backend->GetRenderState(QDemonRenderState::CullFace);
    m_HardwarePropertyContext.m_DepthFunction = m_backend->GetDepthFunc();
    m_HardwarePropertyContext.m_BlendingEnabled =
            m_backend->GetRenderState(QDemonRenderState::Blend);
    m_HardwarePropertyContext.m_DepthWriteEnabled = m_backend->GetDepthWrite();
    m_HardwarePropertyContext.m_DepthTestEnabled =
            m_backend->GetRenderState(QDemonRenderState::DepthTest);
    m_HardwarePropertyContext.m_ScissorTestEnabled =
            m_backend->GetRenderState(QDemonRenderState::ScissorTest);
    m_backend->GetScissorRect(&m_HardwarePropertyContext.m_ScissorRect);
    m_backend->GetViewportRect(&m_HardwarePropertyContext.m_Viewport);

    DoSetClearColor(m_HardwarePropertyContext.m_ClearColor);
}

QDemonRenderContextImpl::~QDemonRenderContextImpl()
{
    Q_ASSERT(m_VertToImpMap.size() == 0);
    m_VertToImpMap.clear();
    Q_ASSERT(m_IndexToImpMap.size() == 0);
    m_IndexToImpMap.clear();
    Q_ASSERT(m_ConstantToImpMap.size() == 0);
    m_ConstantToImpMap.clear();
    Q_ASSERT(m_StorageToImpMap.size() == 0);
    m_StorageToImpMap.clear();
    Q_ASSERT(m_DepthStencilStateToImpMap.size() == 0);
    m_DepthStencilStateToImpMap.clear();
    Q_ASSERT(m_RasterizerStateToImpMap.size() == 0);
    m_RasterizerStateToImpMap.clear();
    Q_ASSERT(m_PathFontSpecToImpMap.size() == 0);
    m_PathFontSpecToImpMap.clear();
    Q_ASSERT(m_Tex2DToImpMap.size() == 0);
    m_Tex2DToImpMap.clear();
    Q_ASSERT(m_Tex2DArrayToImpMap.size() == 0);
    m_Tex2DArrayToImpMap.clear();
    Q_ASSERT(m_Image2DtoImpMap.size() == 0);
    m_Image2DtoImpMap.clear();
    //Q_ASSERT(m_ShaderToImpMap.size() == 0);
    m_ShaderToImpMap.clear();
    Q_ASSERT(m_RenderBufferToImpMap.size() == 0);
    m_RenderBufferToImpMap.clear();
    //Q_ASSERT(m_FrameBufferToImpMap.size() == 0);
    m_FrameBufferToImpMap.clear();

    m_backend = nullptr;
}

void QDemonRenderContextImpl::getMaxTextureSize(quint32 &oWidth, quint32 &oHeight)
{
    qint32 theMaxTextureSize = 0;
    m_backend->GetRenderBackendValue(QDemonRenderBackend::QDemonRenderBackendQuery::MaxTextureSize,
                                     &theMaxTextureSize);

    oWidth = (quint32)theMaxTextureSize;
    oHeight = (quint32)theMaxTextureSize;
}

QDemonRenderDepthStencilState *QDemonRenderContextImpl::CreateDepthStencilState(
        bool enableDepth, bool depthMask, QDemonRenderBoolOp::Enum depthFunc, bool enableStencil,
        QDemonRenderStencilFunctionArgument &stencilFuncFront,
        QDemonRenderStencilFunctionArgument &stencilFuncBack,
        QDemonRenderStencilOperationArgument &depthStencilOpFront,
        QDemonRenderStencilOperationArgument &depthStencilOpBack)
{
    QDemonRenderDepthStencilState *state = QDemonRenderDepthStencilState::Create(
                *this, enableDepth, depthMask, depthFunc, enableStencil, stencilFuncFront,
                stencilFuncBack, depthStencilOpFront, depthStencilOpBack);
    if (state)
        m_DepthStencilStateToImpMap.insert(state->GetDepthStencilObjectHandle(), state);

    return state;
}

void QDemonRenderContextImpl::SetDepthStencilState(QDemonRenderDepthStencilState *inDepthStencilState)
{
    if (inDepthStencilState) {
        m_backend->SetDepthStencilState(inDepthStencilState->GetDepthStencilObjectHandle());
        // currently we have a mixture therefore we need to update the context state
        SetDepthFunction(inDepthStencilState->GetDepthFunc());
        SetDepthWriteEnabled(inDepthStencilState->GetDepthMask());
        SetDepthTestEnabled(inDepthStencilState->GetDepthEnabled());
        SetStencilTestEnabled(inDepthStencilState->GetStencilEnabled());
    }
}

void QDemonRenderContextImpl::StateDestroyed(QDemonRenderDepthStencilState &state)
{
    m_DepthStencilStateToImpMap.remove(state.GetDepthStencilObjectHandle());
}

QDemonRenderRasterizerState *
QDemonRenderContextImpl::CreateRasterizerState(float depthBias, float depthScale,
                                               QDemonRenderFaces::Enum cullFace)
{
    QDemonRenderRasterizerState *state =
            QDemonRenderRasterizerState::Create(*this, depthBias, depthScale, cullFace);
    if (state)
        m_RasterizerStateToImpMap.insert(state->GetRasterizerObjectHandle(), state);

    return state;
}

void QDemonRenderContextImpl::SetRasterizerState(QDemonRenderRasterizerState *inRasterizerState)
{
    if (inRasterizerState)
        m_backend->SetRasterizerState(inRasterizerState->GetRasterizerObjectHandle());
}

void QDemonRenderContextImpl::StateDestroyed(QDemonRenderRasterizerState &state)
{
    m_RasterizerStateToImpMap.remove(state.GetRasterizerObjectHandle());
}

QDemonRenderVertexBuffer *
QDemonRenderContextImpl::CreateVertexBuffer(QDemonRenderBufferUsageType::Enum usageType, size_t size,
                                            quint32 stride, QDemonConstDataRef<quint8> bufferData)
{
    QDemonRenderVertexBuffer *buffer =
            QDemonRenderVertexBuffer::Create(*this, usageType, size, stride, bufferData);
    if (buffer)
        m_VertToImpMap.insert(buffer->GetImplementationHandle(), buffer);
    return buffer;
}

QDemonRenderVertexBuffer *QDemonRenderContextImpl::GetVertexBuffer(const void *implementationHandle)
{
    QHash<const void *, QDemonRenderVertexBuffer *>::const_iterator entry = m_VertToImpMap.find(implementationHandle);
    if (entry != m_VertToImpMap.end())
        return entry.value();
    return nullptr;
}

void QDemonRenderContextImpl::BufferDestroyed(QDemonRenderVertexBuffer &buffer)
{
    m_VertToImpMap.remove(buffer.GetImplementationHandle());
}

QDemonRenderIndexBuffer *
QDemonRenderContextImpl::CreateIndexBuffer(QDemonRenderBufferUsageType::Enum usageType,
                                           QDemonRenderComponentTypes::Enum componentType,
                                           size_t size, QDemonConstDataRef<quint8> bufferData)
{
    QDemonRenderIndexBuffer *buffer =
            QDemonRenderIndexBuffer::Create(*this, usageType, componentType, size, bufferData);

    if (buffer) {
        m_IndexToImpMap.insert(buffer->GetImplementationHandle(), buffer);
    }

    return buffer;
}

QDemonRenderIndexBuffer *QDemonRenderContextImpl::GetIndexBuffer(const void *implementationHandle)
{
    const QHash<const void *, QDemonRenderIndexBuffer *>::iterator entry = m_IndexToImpMap.find(implementationHandle);
    if (entry != m_IndexToImpMap.end())
        return entry.value();
    return nullptr;
}

void QDemonRenderContextImpl::BufferDestroyed(QDemonRenderIndexBuffer &buffer)
{
    m_IndexToImpMap.remove(buffer.GetImplementationHandle());
}

QDemonRenderConstantBuffer *
QDemonRenderContextImpl::CreateConstantBuffer(const char *bufferName,
                                              QDemonRenderBufferUsageType::Enum usageType,
                                              size_t size, QDemonConstDataRef<quint8> bufferData)
{
    QDemonRenderConstantBuffer *buffer =
            QDemonRenderConstantBuffer::Create(*this, bufferName, usageType, size, bufferData);

    if (buffer) {
        m_ConstantToImpMap.insert(buffer->GetBufferName(), buffer);
    }

    return buffer;
}

QDemonRenderConstantBuffer *QDemonRenderContextImpl::GetConstantBuffer(const QString &bufferName)
{
    TContextConstantBufferMap::iterator entry = m_ConstantToImpMap.find(bufferName);
    if (entry != m_ConstantToImpMap.end())
        return entry.value();
    return nullptr;
}

void QDemonRenderContextImpl::BufferDestroyed(QDemonRenderConstantBuffer &buffer)
{
    m_ConstantToImpMap.remove(buffer.GetBufferName());
}

quint32 QDemonRenderContextImpl::GetNextConstantBufferUnit()
{
    quint32 retval = m_NextConstantBufferUnit;
    ++m_NextConstantBufferUnit;
    // Too many texture units for a single draw call.
    if (retval >= m_MaxConstantBufferUnits) {
        Q_ASSERT(false);
        retval = retval % m_MaxConstantBufferUnits;
    }
    return retval;
}

QDemonRenderStorageBuffer *QDemonRenderContextImpl::CreateStorageBuffer(
        const char *bufferName, QDemonRenderBufferUsageType::Enum usageType, size_t size,
        QDemonConstDataRef<quint8> bufferData, QDemonRenderDataBuffer *pBuffer)
{
    QDemonRenderStorageBuffer *buffer =
            QDemonRenderStorageBuffer::Create(*this, bufferName, usageType, size, bufferData, pBuffer);

    if (buffer) {
        m_StorageToImpMap.insert(buffer->GetBufferName(), buffer);
    }

    return buffer;
}

QDemonRenderStorageBuffer *QDemonRenderContextImpl::GetStorageBuffer(const QString &bufferName)
{
    TContextStorageBufferMap::iterator entry = m_StorageToImpMap.find(bufferName);
    if (entry != m_StorageToImpMap.end())
        return entry.value();
    return nullptr;
}

void QDemonRenderContextImpl::BufferDestroyed(QDemonRenderStorageBuffer &buffer)
{
    m_StorageToImpMap.remove(buffer.GetBufferName());
}

QDemonRenderAtomicCounterBuffer *QDemonRenderContextImpl::CreateAtomicCounterBuffer(
        const char *bufferName, QDemonRenderBufferUsageType::Enum usageType, size_t size,
        QDemonConstDataRef<quint8> bufferData)
{
    QDemonRenderAtomicCounterBuffer *buffer =
            QDemonRenderAtomicCounterBuffer::Create(*this, bufferName, usageType, size, bufferData);

    if (buffer) {
        m_AtomicCounterToImpMap.insert(buffer->GetBufferName(), buffer);
    }

    return buffer;
}

QDemonRenderAtomicCounterBuffer *
QDemonRenderContextImpl::GetAtomicCounterBuffer(const QString &bufferName)
{
    TContextAtomicCounterBufferMap::iterator entry = m_AtomicCounterToImpMap.find(bufferName);
    if (entry != m_AtomicCounterToImpMap.end())
        return entry.value();
    return nullptr;
}

QDemonRenderAtomicCounterBuffer *
QDemonRenderContextImpl::GetAtomicCounterBufferByParam(const QString &paramName)
{
    // iterate through all atomic counter buffers
    for (TContextAtomicCounterBufferMap::iterator iter = m_AtomicCounterToImpMap.begin(),
         end = m_AtomicCounterToImpMap.end();
         iter != end; ++iter) {
        if (iter.value() && iter.value()->ContainsParam(paramName))
            return iter.value();
    }

    return nullptr;
}

void QDemonRenderContextImpl::BufferDestroyed(QDemonRenderAtomicCounterBuffer &buffer)
{
    m_AtomicCounterToImpMap.remove(buffer.GetBufferName());
}

QDemonRenderDrawIndirectBuffer *QDemonRenderContextImpl::CreateDrawIndirectBuffer(
        QDemonRenderBufferUsageType::Enum usageType, size_t size,
        QDemonConstDataRef<quint8> bufferData)
{
    QDemonRenderDrawIndirectBuffer *buffer =
            QDemonRenderDrawIndirectBuffer::Create(*this, usageType, size, bufferData);

    if (buffer) {
        m_DrawIndirectToImpMap.insert(buffer->GetBuffertHandle(), buffer);
    }

    return buffer;
}

QDemonRenderDrawIndirectBuffer *QDemonRenderContextImpl::GetDrawIndirectBuffer(
        QDemonRenderBackend::QDemonRenderBackendBufferObject implementationHandle)
{
    TContextDrawIndirectBufferMap::iterator entry =
            m_DrawIndirectToImpMap.find(implementationHandle);
    if (entry != m_DrawIndirectToImpMap.end())
        return entry.value();
    return nullptr;
}

void QDemonRenderContextImpl::BufferDestroyed(QDemonRenderDrawIndirectBuffer &buffer)
{
    m_DrawIndirectToImpMap.remove(buffer.GetBuffertHandle());
}

void QDemonRenderContextImpl::SetMemoryBarrier(QDemonRenderBufferBarrierFlags barriers)
{
    m_backend->SetMemoryBarrier(barriers);
}

QDemonRenderOcclusionQuery *QDemonRenderContextImpl::CreateOcclusionQuery()
{
    QDemonRenderOcclusionQuery *theQuery = QDemonRenderOcclusionQuery::Create(*this);

    return theQuery;
}

QDemonRenderTimerQuery *QDemonRenderContextImpl::CreateTimerQuery()
{
    QDemonRenderTimerQuery *theQuery = QDemonRenderTimerQuery::Create(*this);

    return theQuery;
}

QDemonRenderSync *QDemonRenderContextImpl::CreateSync()
{
    QDemonRenderSync *theSync = QDemonRenderSync::Create(*this);

    return theSync;
}

QSharedPointer<QDemonRenderTexture2D> QDemonRenderContextImpl::CreateTexture2D()
{
    QSharedPointer<QDemonRenderTexture2D> retval(QDemonRenderTexture2D::Create(*this));
    if (retval)
        m_Tex2DToImpMap.insert(retval->GetImplementationHandle(), retval.data());
    return retval;
}

QSharedPointer<QDemonRenderTexture2DArray> QDemonRenderContextImpl::CreateTexture2DArray()
{
    QSharedPointer<QDemonRenderTexture2DArray> retval(QDemonRenderTexture2DArray::Create(*this));
    if (retval)
        m_Tex2DArrayToImpMap.insert(retval->GetTextureObjectHandle(), retval.data());

    return retval;
}

QDemonRenderTextureCube *QDemonRenderContextImpl::CreateTextureCube()
{
    QDemonRenderTextureCube *retval = QDemonRenderTextureCube::Create(*this);
    if (retval)
        m_TexCubeToImpMap.insert(retval->GetTextureObjectHandle(), retval);

    return retval;
}

QSharedPointer<QDemonRenderTexture2D> QDemonRenderContextImpl::GetTexture2D(const void *implementationHandle)
{
    const QHash<const void *, QDemonRenderTexture2D *>::iterator entry = m_Tex2DToImpMap.find(implementationHandle);
    if (entry != m_Tex2DToImpMap.end())
        return entry.value()->sharedFromThis();
    return nullptr;
}

void QDemonRenderContextImpl::TextureDestroyed(QDemonRenderTexture2D *buffer)
{
    m_Tex2DToImpMap.remove(buffer->GetImplementationHandle());
    // We would like to find and catch any situations where this texture is being used
    // but that would require some real work that we don't want to do right now.
}

void QDemonRenderContextImpl::TextureDestroyed(QDemonRenderTexture2DArray *buffer)
{
    m_Tex2DArrayToImpMap.remove(buffer->GetTextureObjectHandle());
}

void QDemonRenderContextImpl::TextureDestroyed(QDemonRenderTextureCube &buffer)
{
    m_TexCubeToImpMap.remove(buffer.GetTextureObjectHandle());
}

QDemonRenderImage2D *QDemonRenderContextImpl::CreateImage2D(QSharedPointer<QDemonRenderTexture2D> inTexture,
                                                            QDemonRenderImageAccessType::Enum inAccess)
{
    QDemonRenderImage2D *retval = QDemonRenderImage2D::Create(*this, inTexture, inAccess);
    if (retval)
        m_Image2DtoImpMap.insert(retval->GetTextureObjectHandle(), retval);

    return retval;
}

void QDemonRenderContextImpl::ImageDestroyed(QDemonRenderImage2D &image)
{
    m_Image2DtoImpMap.remove(image.GetTextureObjectHandle());
}

// IF this texture isn't on a texture unit, put it on one.
// If it is on a texture unit, mark it as the most recently used texture.
quint32 QDemonRenderContextImpl::GetNextTextureUnit()
{
    quint32 retval = m_NextTextureUnit;
    ++m_NextTextureUnit;
    // Too many texture units for a single draw call.
    if (retval >= m_MaxTextureUnits) {
        Q_ASSERT(false);
        retval = retval % m_MaxTextureUnits;
    }
    return retval;
}

QSharedPointer<QDemonRenderRenderBuffer>
QDemonRenderContextImpl::CreateRenderBuffer(QDemonRenderRenderBufferFormats::Enum bufferFormat,
                                            quint32 width, quint32 height)
{
    QSharedPointer<QDemonRenderRenderBuffer> retval =
            QDemonRenderRenderBuffer::Create(*this, bufferFormat, width, height);
    if (retval != nullptr)
        m_RenderBufferToImpMap.insert(retval->GetImplementationHandle(), retval.data());
    return retval;
}

QSharedPointer<QDemonRenderRenderBuffer> QDemonRenderContextImpl::GetRenderBuffer(const void *implementationHandle)
{
    const QHash<const void *, QDemonRenderRenderBuffer *>::iterator entry = m_RenderBufferToImpMap.find(implementationHandle);
    if (entry != m_RenderBufferToImpMap.end())
        return entry.value()->sharedFromThis();
    return nullptr;
}

void QDemonRenderContextImpl::RenderBufferDestroyed(QDemonRenderRenderBuffer *buffer)
{
    m_RenderBufferToImpMap.remove(buffer->GetImplementationHandle());
}

QSharedPointer<QDemonRenderFrameBuffer> QDemonRenderContextImpl::CreateFrameBuffer()
{
    QSharedPointer<QDemonRenderFrameBuffer> retval = QDemonRenderFrameBuffer::Create(*this);
    if (retval != nullptr)
        m_FrameBufferToImpMap.insert(retval->GetImplementationHandle(), retval.data());
    return retval;
}

QSharedPointer<QDemonRenderFrameBuffer> QDemonRenderContextImpl::GetFrameBuffer(const void *implementationHandle)
{
    const QHash<const void *, QDemonRenderFrameBuffer *>::iterator entry = m_FrameBufferToImpMap.find(implementationHandle);
    if (entry != m_FrameBufferToImpMap.end())
        return entry.value()->sharedFromThis();
    return nullptr;
}

void QDemonRenderContextImpl::FrameBufferDestroyed(QDemonRenderFrameBuffer *fb)
{
    m_FrameBufferToImpMap.remove(fb->GetImplementationHandle());
    if (m_HardwarePropertyContext.m_FrameBuffer == fb)
        m_HardwarePropertyContext.m_FrameBuffer = nullptr;
}

QSharedPointer<QDemonRenderAttribLayout> QDemonRenderContextImpl::CreateAttributeLayout(QDemonConstDataRef<QDemonRenderVertexBufferEntry> attribs)
{
    return QSharedPointer<QDemonRenderAttribLayout>(new QDemonRenderAttribLayout(*this, attribs));
}

QSharedPointer<QDemonRenderInputAssembler> QDemonRenderContextImpl::CreateInputAssembler(
        QSharedPointer<QDemonRenderAttribLayout> attribLayout, QDemonConstDataRef<QDemonRenderVertexBuffer *> buffers,
        const QDemonRenderIndexBuffer *indexBuffer, QDemonConstDataRef<quint32> strides,
        QDemonConstDataRef<quint32> offsets, QDemonRenderDrawMode::Enum primType, quint32 patchVertexCount)
{
    return QSharedPointer<QDemonRenderInputAssembler>(new QDemonRenderInputAssembler(*this, attribLayout, buffers, indexBuffer, strides,
                                                  offsets, primType, patchVertexCount));
}

void QDemonRenderContextImpl::SetInputAssembler(QSharedPointer<QDemonRenderInputAssembler> inputAssembler)
{
    if (m_HardwarePropertyContext.m_InputAssembler != inputAssembler) {
        DoSetInputAssembler(inputAssembler);
    }
}

QDemonRenderVertFragCompilationResult QDemonRenderContextImpl::CompileSource(
        const char *shaderName, QDemonConstDataRef<qint8> vertShader, QDemonConstDataRef<qint8> fragShader,
        QDemonConstDataRef<qint8> tessControlShaderSource,
        QDemonConstDataRef<qint8> tessEvaluationShaderSource, QDemonConstDataRef<qint8> geometryShaderSource,
        bool separateProgram, QDemonRenderShaderProgramBinaryType::Enum type, bool binaryProgram)
{
    QDemonRenderVertFragCompilationResult result = QDemonRenderShaderProgram::Create(
                *this, shaderName, vertShader, fragShader, tessControlShaderSource,
                tessEvaluationShaderSource, geometryShaderSource, separateProgram, type, binaryProgram);

    if (result.mShader != nullptr)
        m_ShaderToImpMap.insert(result.mShader->GetShaderProgramHandle(), result.mShader.data());

    return result;
}

QDemonRenderVertFragCompilationResult QDemonRenderContextImpl::CompileBinary(
        const char *shaderName, QDemonRenderShaderProgramBinaryType::Enum type,
        QDemonDataRef<qint8> vertShader, QDemonDataRef<qint8> fragShader,
        QDemonDataRef<qint8> tessControlShaderSource, QDemonDataRef<qint8> tessEvaluationShaderSource,
        QDemonConstDataRef<qint8> geometryShaderSource)
{
#ifndef _MACOSX
    QDemonRenderVertFragCompilationResult result = QDemonRenderShaderProgram::Create(
                *this, shaderName, vertShader, fragShader, tessControlShaderSource,
                tessEvaluationShaderSource, geometryShaderSource, false, type, true);

    if (result.mShader != nullptr)
        m_ShaderToImpMap.insert(result.mShader->GetShaderProgramHandle(), result.mShader.data());

    return result;
#else
    Q_ASSERT(false);
    return QDemonRenderVertFragCompilationResult();
#endif
}

QDemonRenderVertFragCompilationResult
QDemonRenderContextImpl::CompileComputeSource(const char *shaderName,
                                              QDemonConstDataRef<qint8> computeShaderSource)
{
    QDemonRenderVertFragCompilationResult result =
            QDemonRenderShaderProgram::CreateCompute(*this, shaderName, computeShaderSource);

    if (result.mShader != nullptr)
        m_ShaderToImpMap.insert(result.mShader->GetShaderProgramHandle(), result.mShader.data());

    return result;
}

QSharedPointer<QDemonRenderShaderProgram> QDemonRenderContextImpl::GetShaderProgram(const void *implementationHandle)
{
    const QHash<const void *, QDemonRenderShaderProgram *>::iterator entry = m_ShaderToImpMap.find(implementationHandle);
    if (entry != m_ShaderToImpMap.end())
        return QSharedPointer<QDemonRenderShaderProgram>(entry.value());
    return nullptr;
}

void QDemonRenderContextImpl::ShaderDestroyed(QDemonRenderShaderProgram *shader)
{
    m_ShaderToImpMap.remove(shader->GetShaderProgramHandle());
    if (m_HardwarePropertyContext.m_ActiveShader.data() == shader)
        SetActiveShader(nullptr);
}

QDemonRenderProgramPipeline *QDemonRenderContextImpl::CreateProgramPipeline()
{
    return new QDemonRenderProgramPipeline(*this);
}

QDemonRenderPathSpecification *QDemonRenderContextImpl::CreatePathSpecification()
{
    return QDemonRenderPathSpecification::CreatePathSpecification(*this);
}

QDemonRenderPathRender *QDemonRenderContextImpl::CreatePathRender(size_t range)
{
    return QDemonRenderPathRender::Create(*this, range);
}

void QDemonRenderContextImpl::SetPathProjectionMatrix(const QMatrix4x4 inPathProjection)
{
    m_backend->SetPathProjectionMatrix(inPathProjection);
}

void QDemonRenderContextImpl::SetPathModelViewMatrix(const QMatrix4x4 inPathModelview)
{
    m_backend->SetPathModelViewMatrix(inPathModelview);
}

void QDemonRenderContextImpl::SetPathStencilDepthOffset(float inSlope, float inBias)
{
    m_backend->SetPathStencilDepthOffset(inSlope, inBias);
}
void QDemonRenderContextImpl::SetPathCoverDepthFunc(QDemonRenderBoolOp::Enum inFunc)
{
    m_backend->SetPathCoverDepthFunc(inFunc);
}

QDemonRenderPathFontSpecification *
QDemonRenderContextImpl::CreatePathFontSpecification(const QString &fontName)
{
    // first check if it already exists
    QHash<QString, QDemonRenderPathFontSpecification *>::const_iterator entry = m_PathFontSpecToImpMap.find(fontName);
    if (entry != m_PathFontSpecToImpMap.end())
        return entry.value();

    // if not create new one
    QDemonRenderPathFontSpecification *pPathFontSpec =
            QDemonRenderPathFontSpecification::CreatePathFontSpecification(*this, fontName);

    if (pPathFontSpec)
        m_PathFontSpecToImpMap.insert(fontName, pPathFontSpec);

    return pPathFontSpec;
}

void
QDemonRenderContextImpl::ReleasePathFontSpecification(QDemonRenderPathFontSpecification &inPathSpec)
{
    m_PathFontSpecToImpMap.remove(inPathSpec.GetFontName());
}

QDemonRenderPathFontItem *QDemonRenderContextImpl::CreatePathFontItem()
{
    // if not create new one
    return QDemonRenderPathFontItem::CreatePathFontItem(*this);
}

void QDemonRenderContextImpl::SetClearColor(QVector4D inClearColor)
{
    if (m_HardwarePropertyContext.m_ClearColor != inClearColor)
        DoSetClearColor(inClearColor);
}

void QDemonRenderContextImpl::SetBlendFunction(QDemonRenderBlendFunctionArgument inFunctions)
{
    if (memcmp(&inFunctions, &m_HardwarePropertyContext.m_BlendFunction,
               sizeof(QDemonRenderBlendFunctionArgument))) {
        DoSetBlendFunction(inFunctions);
    }
}

void QDemonRenderContextImpl::SetBlendEquation(QDemonRenderBlendEquationArgument inEquations)
{
    if (memcmp(&inEquations, &m_HardwarePropertyContext.m_BlendEquation,
               sizeof(QDemonRenderBlendEquationArgument))) {
        DoSetBlendEquation(inEquations);
    }
}

void QDemonRenderContextImpl::SetCullingEnabled(bool inEnabled)
{
    if (inEnabled != m_HardwarePropertyContext.m_CullingEnabled) {
        DoSetCullingEnabled(inEnabled);
    }
}

void QDemonRenderContextImpl::SetDepthFunction(QDemonRenderBoolOp::Enum inFunction)
{
    if (inFunction != m_HardwarePropertyContext.m_DepthFunction) {
        DoSetDepthFunction(inFunction);
    }
}

void QDemonRenderContextImpl::SetBlendingEnabled(bool inEnabled)
{
    if (inEnabled != m_HardwarePropertyContext.m_BlendingEnabled) {
        DoSetBlendingEnabled(inEnabled);
    }
}

void QDemonRenderContextImpl::SetColorWritesEnabled(bool inEnabled)
{
    if (inEnabled != m_HardwarePropertyContext.m_ColorWritesEnabled) {
        DoSetColorWritesEnabled(inEnabled);
    }
}

void QDemonRenderContextImpl::SetDepthWriteEnabled(bool inEnabled)
{
    if (inEnabled != m_HardwarePropertyContext.m_DepthWriteEnabled) {
        m_HardwarePropertyContext.m_DepthWriteEnabled = inEnabled;
        m_backend->SetRenderState(inEnabled, QDemonRenderState::DepthWrite);
    }
}

void QDemonRenderContextImpl::SetDepthTestEnabled(bool inEnabled)
{
    if (inEnabled != m_HardwarePropertyContext.m_DepthTestEnabled) {
        DoSetDepthTestEnabled(inEnabled);
    }
}

void QDemonRenderContextImpl::SetMultisampleEnabled(bool inEnabled)
{
    if (inEnabled != m_HardwarePropertyContext.m_MultisampleEnabled) {
        DoSetMultisampleEnabled(inEnabled);
    }
}

void QDemonRenderContextImpl::SetStencilTestEnabled(bool inEnabled)
{
    if (inEnabled != m_HardwarePropertyContext.m_StencilTestEnabled) {
        DoSetStencilTestEnabled(inEnabled);
    }
}

void QDemonRenderContextImpl::SetScissorTestEnabled(bool inEnabled)
{
    if (inEnabled != m_HardwarePropertyContext.m_ScissorTestEnabled) {
        DoSetScissorTestEnabled(inEnabled);
    }
}

void QDemonRenderContextImpl::SetScissorRect(QDemonRenderRect inRect)
{
    if (memcmp(&inRect, &m_HardwarePropertyContext.m_ScissorRect, sizeof(QDemonRenderRect))) {
        DoSetScissorRect(inRect);
    }
}

void QDemonRenderContextImpl::SetViewport(QDemonRenderRect inViewport)
{
    if (memcmp(&inViewport, &m_HardwarePropertyContext.m_Viewport, sizeof(QDemonRenderRect))) {
        DoSetViewport(inViewport);
    }
}

void QDemonRenderContextImpl::SetActiveShader(QSharedPointer<QDemonRenderShaderProgram> inShader)
{
    if (inShader != m_HardwarePropertyContext.m_ActiveShader)
        DoSetActiveShader(inShader);
}

void QDemonRenderContextImpl::SetActiveProgramPipeline(QDemonRenderProgramPipeline *inProgramPipeline)
{
    if (inProgramPipeline != m_HardwarePropertyContext.m_ActiveProgramPipeline)
        DoSetActiveProgramPipeline(inProgramPipeline);
}

void QDemonRenderContextImpl::DispatchCompute(QSharedPointer<QDemonRenderShaderProgram> inShader, quint32 numGroupsX,
                                              quint32 numGroupsY, quint32 numGroupsZ)
{
    Q_ASSERT(inShader);

    if (inShader != m_HardwarePropertyContext.m_ActiveShader)
        DoSetActiveShader(inShader);

    m_backend->DispatchCompute(inShader->GetShaderProgramHandle(), numGroupsX, numGroupsY,
                               numGroupsZ);

    OnPostDraw();
}

void QDemonRenderContextImpl::SetDrawBuffers(QDemonConstDataRef<qint32> inDrawBufferSet)
{
    m_backend->SetDrawBuffers(
                (m_HardwarePropertyContext.m_FrameBuffer)
                ? m_HardwarePropertyContext.m_FrameBuffer->GetFrameBuffertHandle()
                : nullptr,
                inDrawBufferSet);
}

void QDemonRenderContextImpl::SetReadBuffer(QDemonReadFaces::Enum inReadFace)
{
    // currently nullptr which means the read target must be set with setReadTarget
    m_backend->SetReadBuffer(nullptr, inReadFace);
}

void QDemonRenderContextImpl::ReadPixels(QDemonRenderRect inRect,
                                         QDemonRenderReadPixelFormats::Enum inFormat,
                                         QDemonDataRef<quint8> inWriteBuffer)
{
    // nullptr means read from current render target
    m_backend->ReadPixel(nullptr, inRect.m_X, inRect.m_Y, inRect.m_Width, inRect.m_Height,
                         inFormat, (void *)inWriteBuffer.begin());
}

void QDemonRenderContextImpl::SetRenderTarget(QSharedPointer<QDemonRenderFrameBuffer> inBuffer)
{
    if (inBuffer != m_HardwarePropertyContext.m_FrameBuffer) {
        DoSetRenderTarget(inBuffer);
    }
}

void QDemonRenderContextImpl::SetReadTarget(QSharedPointer<QDemonRenderFrameBuffer> inBuffer)
{
    if (inBuffer != m_HardwarePropertyContext.m_FrameBuffer) {
        DoSetReadTarget(inBuffer);
    }
}

void QDemonRenderContextImpl::ResetBlendState()
{
    qint32_4 values;

    m_backend->SetRenderState(m_HardwarePropertyContext.m_BlendingEnabled,
                              QDemonRenderState::Blend);
    const QDemonRenderBlendFunctionArgument &theBlendArg(m_HardwarePropertyContext.m_BlendFunction);
    m_backend->SetBlendFunc(theBlendArg);
}

// Pop the entire set of properties, potentially forcing the values
// to opengl.
void QDemonRenderContextImpl::PopPropertySet(bool inForceSetProperties)
{
    if (!m_PropertyStack.empty()) {
        QDemonGLHardPropertyContext &theTopContext(m_PropertyStack.back());
        if (inForceSetProperties) {
#define HANDLE_CONTEXT_HARDWARE_PROPERTY(setterName, propName)                                     \
    DoSet##setterName(theTopContext.m_##propName);

            ITERATE_HARDWARE_CONTEXT_PROPERTIES

        #undef HANDLE_CONTEXT_HARDWARE_PROPERTY
        } else {
#define HANDLE_CONTEXT_HARDWARE_PROPERTY(setterName, propName)                                     \
    Set##setterName(theTopContext.m_##propName);

            ITERATE_HARDWARE_CONTEXT_PROPERTIES

        #undef HANDLE_CONTEXT_HARDWARE_PROPERTY
        }
        m_PropertyStack.pop_back();
    }
}

void QDemonRenderContextImpl::Clear(QDemonRenderClearFlags flags)
{
    if ((flags & QDemonRenderClearValues::Depth)
            && m_HardwarePropertyContext.m_DepthWriteEnabled == false) {
        Q_ASSERT(false);
        SetDepthWriteEnabled(true);
    }
    m_backend->Clear(flags);
}

void QDemonRenderContextImpl::Clear(QSharedPointer<QDemonRenderFrameBuffer> fb, QDemonRenderClearFlags flags)
{
    QSharedPointer<QDemonRenderFrameBuffer> previous = m_HardwarePropertyContext.m_FrameBuffer;
    if (previous != fb)
        SetRenderTarget(fb);

    Clear(flags);

    if (previous != fb)
        SetRenderTarget(previous);
}

void QDemonRenderContextImpl::BlitFramebuffer(qint32 srcX0, qint32 srcY0, qint32 srcX1, qint32 srcY1,
                                              qint32 dstX0, qint32 dstY0, qint32 dstX1, qint32 dstY1,
                                              QDemonRenderClearFlags flags,
                                              QDemonRenderTextureMagnifyingOp::Enum filter)
{
    m_backend->BlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, flags,
                               filter);
}

bool
QDemonRenderContextImpl::BindShaderToInputAssembler(const QSharedPointer<QDemonRenderInputAssembler> inputAssembler,
                                                    QSharedPointer<QDemonRenderShaderProgram> shader)
{
    // setup the input assembler object
    return m_backend->SetInputAssembler(inputAssembler->GetInputAssemblerHandle(),
                                        shader->GetShaderProgramHandle());
}

bool QDemonRenderContextImpl::ApplyPreDrawProperties()
{
    // Get the currently bound vertex and shader
    QSharedPointer<QDemonRenderInputAssembler> inputAssembler = this->m_HardwarePropertyContext.m_InputAssembler;
    QSharedPointer<QDemonRenderShaderProgram> shader(this->m_HardwarePropertyContext.m_ActiveShader);

    // we could render through a program pipline
    if (shader == nullptr && this->m_HardwarePropertyContext.m_ActiveProgramPipeline)
        shader = this->m_HardwarePropertyContext.m_ActiveProgramPipeline->GetVertexStage();

    if (inputAssembler == nullptr || shader == nullptr) {
        qCCritical(INVALID_OPERATION,
                   "Attempting to render no valid shader or input assembler setup");
        Q_ASSERT(false);
        return false;
    }

    return BindShaderToInputAssembler(inputAssembler, shader);
}

void QDemonRenderContextImpl::OnPostDraw()
{
    // reset input assembler binding
    m_backend->SetInputAssembler(nullptr, 0);
    // Texture unit 0 is used for setting up and loading textures.
    // Bugs happen if we load a texture then setup the sampler.
    // Then we load another texture.  Because when loading we use texture unit 0,
    // the render bindings for the first texture are blown away.
    // Again, for this reason, texture unit 0 is reserved for loading textures.
    m_NextTextureUnit = 1;
    m_NextConstantBufferUnit = 0;
}

void QDemonRenderContextImpl::Draw(QDemonRenderDrawMode::Enum drawMode, quint32 count, quint32 offset)
{
    if (!ApplyPreDrawProperties())
        return;

    QDemonRenderIndexBuffer *theIndexBuffer = const_cast<QDemonRenderIndexBuffer *>(
                m_HardwarePropertyContext.m_InputAssembler->GetIndexBuffer());
    if (theIndexBuffer == nullptr)
        m_backend->Draw(drawMode, offset, count);
    else
        theIndexBuffer->Draw(drawMode, count, offset);

    OnPostDraw();
}

void QDemonRenderContextImpl::DrawIndirect(QDemonRenderDrawMode::Enum drawMode, quint32 offset)
{
    if (!ApplyPreDrawProperties())
        return;

    QDemonRenderIndexBuffer *theIndexBuffer = const_cast<QDemonRenderIndexBuffer *>(
                m_HardwarePropertyContext.m_InputAssembler->GetIndexBuffer());
    if (theIndexBuffer == nullptr)
        m_backend->DrawIndirect(drawMode, (const void *)offset);
    else
        theIndexBuffer->DrawIndirect(drawMode, offset);

    OnPostDraw();
}

QMatrix4x4
QDemonRenderContext::ApplyVirtualViewportToProjectionMatrix(const QMatrix4x4 &inProjection,
                                                            const QDemonRenderRectF &inViewport,
                                                            const QDemonRenderRectF &inVirtualViewport)
{
    if (inVirtualViewport == inViewport)
        return inProjection;
    // Run conversion to floating point once.
    QDemonRenderRectF theVirtualViewport(inVirtualViewport);
    QDemonRenderRectF theViewport(inViewport);
    if (theVirtualViewport.m_Width == 0 || theVirtualViewport.m_Height == 0
            || theViewport.m_Width == 0 || theViewport.m_Height == 0) {
        Q_ASSERT(false);
        return inProjection;
    }
    QMatrix4x4 theScaleTransMat;
    float theHeightDiff = theViewport.m_Height - theVirtualViewport.m_Height;
    float theViewportOffY = theVirtualViewport.m_Y - theViewport.m_Y;
    QVector2D theCameraOffsets = QVector2D(theVirtualViewport.m_Width - theViewport.m_Width
                                           + (theVirtualViewport.m_X - theViewport.m_X) * 2.0f,
                                           theHeightDiff + (theViewportOffY - theHeightDiff) * 2.0f);
    QVector2D theCameraScale = QVector2D(theVirtualViewport.m_Width / theViewport.m_Width,
                                         theVirtualViewport.m_Height / theViewport.m_Height);

    QVector3D theTranslation(theCameraOffsets.x() / theViewport.m_Width,
                             theCameraOffsets.y() / theViewport.m_Height, 0);
    QVector4D column3 = theScaleTransMat.column(3);
    column3.setX(theTranslation.x());
    column3.setY(theTranslation.y());
    theScaleTransMat.setColumn(3, column3);
    QVector4D column0 = theScaleTransMat.column(0);
    column0.setX(theCameraScale.x());
    theScaleTransMat.setColumn(0, column0);
    QVector4D column1 = theScaleTransMat.column(1);
    column1.setY(theCameraScale.y());
    theScaleTransMat.setColumn(1, column1);

    return theScaleTransMat * inProjection;
}

QDemonRenderVertFragCompilationResult QDemonRenderContext::CompileSource(
        const char *shaderName, const char *vertShader, quint32 inVertLen, const char *fragShader,
        quint32 inFragLen, const char *tessControlShaderSource, quint32 inTCLen,
        const char *tessEvaluationShaderSource, quint32 inTELen, const char *geometryShaderSource,
        quint32 inGSLen, bool separableProgram)
{
    return CompileSource(
                shaderName, QDemonConstDataRef<qint8>((const qint8 *)vertShader, inVertLen),
                QDemonConstDataRef<qint8>((const qint8 *)fragShader, inFragLen),
                QDemonConstDataRef<qint8>((const qint8 *)tessControlShaderSource, inTCLen),
                QDemonConstDataRef<qint8>((const qint8 *)tessEvaluationShaderSource, inTELen),
                QDemonConstDataRef<qint8>((const qint8 *)geometryShaderSource, inGSLen), separableProgram);
}

void QDemonRenderContextImpl::DoSetActiveShader(QSharedPointer<QDemonRenderShaderProgram> inShader)
{
    m_HardwarePropertyContext.m_ActiveShader = nullptr;
    if (!m_backend)
        return;

    if (inShader)
        m_backend->SetActiveProgram(inShader->GetShaderProgramHandle());
    else {
        m_backend->SetActiveProgram(nullptr);
    }
    m_HardwarePropertyContext.m_ActiveShader = inShader;
}

void QDemonRenderContextImpl::DoSetActiveProgramPipeline(QDemonRenderProgramPipeline *inProgramPipeline)
{
    if (inProgramPipeline) {
        // invalid any bound shader
        DoSetActiveShader(nullptr);
        inProgramPipeline->Bind();
    } else
        m_backend->SetActiveProgramPipeline(nullptr);

    m_HardwarePropertyContext.m_ActiveProgramPipeline = inProgramPipeline;
}

QSharedPointer<QDemonRenderContext> QDemonRenderContext::CreateNULL()
{
    QSharedPointer<QDemonRenderContext> retval;

    // create backend
    retval.reset(new QDemonRenderContextImpl(QDemonRenderBackendNULL::CreateBackend()));
    return retval;
}
QT_END_NAMESPACE
