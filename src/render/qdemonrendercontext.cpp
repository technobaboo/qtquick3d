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
    , m_dirtyFlags(0)
    , m_defaultOffscreenRenderTarget((QDemonRenderBackend::QDemonRenderBackendRenderTargetObject)nullptr)
    , m_dephBits(16)
    , m_stencilBits(8)
    , m_nextTextureUnit(1)
    , m_nextConstantBufferUnit(1)
{
    m_maxTextureUnits = m_backend->getMaxCombinedTextureUnits();
    m_maxConstantBufferUnits = 16; // need backend query

    // get initial state
    memZero(&m_hardwarePropertyContext, sizeof(m_hardwarePropertyContext));

    // get default blending functions
    m_backend->getBlendFunc(&m_hardwarePropertyContext.m_blendFunction);
    // set default blend euqation
    m_hardwarePropertyContext.m_blendEquation.m_rgbEquation = QDemonRenderBlendEquation::Add;
    m_hardwarePropertyContext.m_blendEquation.m_alphaEquation = QDemonRenderBlendEquation::Add;
    // default state
    m_hardwarePropertyContext.m_cullingEnabled =
            m_backend->getRenderState(QDemonRenderState::CullFace);
    m_hardwarePropertyContext.m_depthFunction = m_backend->getDepthFunc();
    m_hardwarePropertyContext.m_blendingEnabled =
            m_backend->getRenderState(QDemonRenderState::Blend);
    m_hardwarePropertyContext.m_depthWriteEnabled = m_backend->getDepthWrite();
    m_hardwarePropertyContext.m_depthTestEnabled =
            m_backend->getRenderState(QDemonRenderState::DepthTest);
    m_hardwarePropertyContext.m_scissorTestEnabled =
            m_backend->getRenderState(QDemonRenderState::ScissorTest);
    m_backend->getScissorRect(&m_hardwarePropertyContext.m_scissorRect);
    m_backend->getViewportRect(&m_hardwarePropertyContext.m_viewport);

    doSetClearColor(m_hardwarePropertyContext.m_clearColor);
}

QDemonRenderContextImpl::~QDemonRenderContextImpl()
{
    Q_ASSERT(m_vertToImpMap.size() == 0);
    m_vertToImpMap.clear();
    Q_ASSERT(m_indexToImpMap.size() == 0);
    m_indexToImpMap.clear();
    Q_ASSERT(m_constantToImpMap.size() == 0);
    m_constantToImpMap.clear();
    Q_ASSERT(m_storageToImpMap.size() == 0);
    m_storageToImpMap.clear();
    Q_ASSERT(m_depthStencilStateToImpMap.size() == 0);
    m_depthStencilStateToImpMap.clear();
    Q_ASSERT(m_rasterizerStateToImpMap.size() == 0);
    m_rasterizerStateToImpMap.clear();
    Q_ASSERT(m_pathFontSpecToImpMap.size() == 0);
    m_pathFontSpecToImpMap.clear();
    Q_ASSERT(m_tex2DToImpMap.size() == 0);
    m_tex2DToImpMap.clear();
    Q_ASSERT(m_tex2DArrayToImpMap.size() == 0);
    m_tex2DArrayToImpMap.clear();
    Q_ASSERT(m_image2DtoImpMap.size() == 0);
    m_image2DtoImpMap.clear();
    Q_ASSERT(m_shaderToImpMap.size() == 0);
    m_shaderToImpMap.clear();
    Q_ASSERT(m_renderBufferToImpMap.size() == 0);
    m_renderBufferToImpMap.clear();
    Q_ASSERT(m_frameBufferToImpMap.size() == 0);
    m_frameBufferToImpMap.clear();

    m_backend = nullptr;
}

void QDemonRenderContextImpl::getMaxTextureSize(quint32 &oWidth, quint32 &oHeight)
{
    qint32 theMaxTextureSize = 0;
    m_backend->getRenderBackendValue(QDemonRenderBackend::QDemonRenderBackendQuery::MaxTextureSize,
                                     &theMaxTextureSize);

    oWidth = (quint32)theMaxTextureSize;
    oHeight = (quint32)theMaxTextureSize;
}

QSharedPointer<QDemonRenderDepthStencilState> QDemonRenderContextImpl::createDepthStencilState(
        bool enableDepth, bool depthMask, QDemonRenderBoolOp::Enum depthFunc, bool enableStencil,
        QDemonRenderStencilFunctionArgument &stencilFuncFront,
        QDemonRenderStencilFunctionArgument &stencilFuncBack,
        QDemonRenderStencilOperationArgument &depthStencilOpFront,
        QDemonRenderStencilOperationArgument &depthStencilOpBack)
{
    QSharedPointer<QDemonRenderDepthStencilState> state = QDemonRenderDepthStencilState::create(
                sharedFromThis(), enableDepth, depthMask, depthFunc, enableStencil, stencilFuncFront,
                stencilFuncBack, depthStencilOpFront, depthStencilOpBack);
    if (state)
        m_depthStencilStateToImpMap.insert(state->getDepthStencilObjectHandle(), state.data());

    return state;
}

void QDemonRenderContextImpl::setDepthStencilState(QSharedPointer<QDemonRenderDepthStencilState> inDepthStencilState)
{
    if (inDepthStencilState) {
        m_backend->setDepthStencilState(inDepthStencilState->getDepthStencilObjectHandle());
        // currently we have a mixture therefore we need to update the context state
        setDepthFunction(inDepthStencilState->getDepthFunc());
        setDepthWriteEnabled(inDepthStencilState->getDepthMask());
        setDepthTestEnabled(inDepthStencilState->getDepthEnabled());
        setStencilTestEnabled(inDepthStencilState->getStencilEnabled());
    }
}

void QDemonRenderContextImpl::stateDestroyed(QDemonRenderDepthStencilState *state)
{
    m_depthStencilStateToImpMap.remove(state->getDepthStencilObjectHandle());
}

QSharedPointer<QDemonRenderRasterizerState>
QDemonRenderContextImpl::createRasterizerState(float depthBias, float depthScale,
                                               QDemonRenderFaces::Enum cullFace)
{
    QSharedPointer<QDemonRenderRasterizerState> state = QDemonRenderRasterizerState::create(sharedFromThis(), depthBias, depthScale, cullFace);
    if (state)
        m_rasterizerStateToImpMap.insert(state->GetRasterizerObjectHandle(), state.data());

    return state;
}

void QDemonRenderContextImpl::setRasterizerState(QSharedPointer<QDemonRenderRasterizerState> inRasterizerState)
{
    if (inRasterizerState)
        m_backend->setRasterizerState(inRasterizerState->GetRasterizerObjectHandle());
}

void QDemonRenderContextImpl::stateDestroyed(QDemonRenderRasterizerState *state)
{
    m_rasterizerStateToImpMap.remove(state->GetRasterizerObjectHandle());
}

QSharedPointer<QDemonRenderVertexBuffer> QDemonRenderContextImpl::createVertexBuffer(QDemonRenderBufferUsageType::Enum usageType, size_t size,
                                            quint32 stride, QDemonConstDataRef<quint8> bufferData)
{
    QSharedPointer<QDemonRenderVertexBuffer> buffer =
            QDemonRenderVertexBuffer::create(sharedFromThis(), usageType, size, stride, bufferData);
    if (buffer)
        m_vertToImpMap.insert(buffer->getImplementationHandle(), buffer.data());
    return buffer;
}

QSharedPointer<QDemonRenderVertexBuffer> QDemonRenderContextImpl::getVertexBuffer(const void *implementationHandle)
{
    QHash<const void *, QDemonRenderVertexBuffer *>::const_iterator entry = m_vertToImpMap.find(implementationHandle);
    if (entry != m_vertToImpMap.end())
        return entry.value()->sharedFromThis();
    return nullptr;
}

void QDemonRenderContextImpl::bufferDestroyed(QDemonRenderVertexBuffer *buffer)
{
    m_vertToImpMap.remove(buffer->getImplementationHandle());
}

QSharedPointer<QDemonRenderIndexBuffer> QDemonRenderContextImpl::createIndexBuffer(QDemonRenderBufferUsageType::Enum usageType,
                                           QDemonRenderComponentTypes::Enum componentType,
                                           size_t size, QDemonConstDataRef<quint8> bufferData)
{
    QSharedPointer<QDemonRenderIndexBuffer> buffer = QDemonRenderIndexBuffer::create(sharedFromThis(), usageType, componentType, size, bufferData);

    if (buffer) {
        m_indexToImpMap.insert(buffer->getImplementationHandle(), buffer.data());
    }

    return buffer;
}

QSharedPointer<QDemonRenderIndexBuffer> QDemonRenderContextImpl::getIndexBuffer(const void *implementationHandle)
{
    const QHash<const void *, QDemonRenderIndexBuffer *>::iterator entry = m_indexToImpMap.find(implementationHandle);
    if (entry != m_indexToImpMap.end())
        return entry.value()->sharedFromThis();
    return nullptr;
}

void QDemonRenderContextImpl::bufferDestroyed(QDemonRenderIndexBuffer *buffer)
{
    m_indexToImpMap.remove(buffer->getImplementationHandle());
}

QSharedPointer<QDemonRenderConstantBuffer>
QDemonRenderContextImpl::createConstantBuffer(const char *bufferName,
                                              QDemonRenderBufferUsageType::Enum usageType,
                                              size_t size, QDemonConstDataRef<quint8> bufferData)
{
    QSharedPointer<QDemonRenderConstantBuffer> buffer =
            QDemonRenderConstantBuffer::create(sharedFromThis(), bufferName, usageType, size, bufferData);

    if (buffer) {
        m_constantToImpMap.insert(buffer->GetBufferName(), buffer.data());
    }

    return buffer;
}

QSharedPointer<QDemonRenderConstantBuffer> QDemonRenderContextImpl::getConstantBuffer(const QString &bufferName)
{
    TContextConstantBufferMap::iterator entry = m_constantToImpMap.find(bufferName);
    if (entry != m_constantToImpMap.end())
        return entry.value()->sharedFromThis();
    return nullptr;
}

void QDemonRenderContextImpl::bufferDestroyed(QDemonRenderConstantBuffer *buffer)
{
    m_constantToImpMap.remove(buffer->GetBufferName());
}

quint32 QDemonRenderContextImpl::getNextConstantBufferUnit()
{
    quint32 retval = m_nextConstantBufferUnit;
    ++m_nextConstantBufferUnit;
    // Too many texture units for a single draw call.
    if (retval >= m_maxConstantBufferUnits) {
        Q_ASSERT(false);
        retval = retval % m_maxConstantBufferUnits;
    }
    return retval;
}

QSharedPointer<QDemonRenderStorageBuffer> QDemonRenderContextImpl::createStorageBuffer(const char *bufferName,
                                                                                       QDemonRenderBufferUsageType::Enum usageType,
                                                                                       size_t size,
                                                                                       QDemonConstDataRef<quint8> bufferData,
                                                                                       QDemonRenderDataBuffer *pBuffer)
{
    QSharedPointer<QDemonRenderStorageBuffer> buffer = QDemonRenderStorageBuffer::create(sharedFromThis(), bufferName, usageType, size, bufferData, pBuffer);

    if (buffer) {
        m_storageToImpMap.insert(buffer->getBufferName(), buffer.data());
    }

    return buffer;
}

QSharedPointer<QDemonRenderStorageBuffer> QDemonRenderContextImpl::getStorageBuffer(const QString &bufferName)
{
    TContextStorageBufferMap::iterator entry = m_storageToImpMap.find(bufferName);
    if (entry != m_storageToImpMap.end())
        return entry.value()->sharedFromThis();
    return nullptr;
}

void QDemonRenderContextImpl::bufferDestroyed(QDemonRenderStorageBuffer *buffer)
{
    m_storageToImpMap.remove(buffer->getBufferName());
}

QSharedPointer<QDemonRenderAtomicCounterBuffer> QDemonRenderContextImpl::createAtomicCounterBuffer(
        const char *bufferName, QDemonRenderBufferUsageType::Enum usageType, size_t size,
        QDemonConstDataRef<quint8> bufferData)
{
    QSharedPointer<QDemonRenderAtomicCounterBuffer> buffer =
            QDemonRenderAtomicCounterBuffer::create(sharedFromThis(), bufferName, usageType, size, bufferData);

    if (buffer) {
        m_atomicCounterToImpMap.insert(buffer->getBufferName(), buffer.data());
    }

    return buffer;
}

QSharedPointer<QDemonRenderAtomicCounterBuffer> QDemonRenderContextImpl::getAtomicCounterBuffer(const QString &bufferName)
{
    TContextAtomicCounterBufferMap::iterator entry = m_atomicCounterToImpMap.find(bufferName);
    if (entry != m_atomicCounterToImpMap.end())
        return entry.value()->sharedFromThis();
    return nullptr;
}

QSharedPointer<QDemonRenderAtomicCounterBuffer> QDemonRenderContextImpl::getAtomicCounterBufferByParam(const QString &paramName)
{
    // iterate through all atomic counter buffers
    for (TContextAtomicCounterBufferMap::iterator iter = m_atomicCounterToImpMap.begin(),
         end = m_atomicCounterToImpMap.end();
         iter != end; ++iter) {
        if (iter.value() && iter.value()->containsParam(paramName))
            return iter.value()->sharedFromThis();
    }

    return nullptr;
}

void QDemonRenderContextImpl::bufferDestroyed(QDemonRenderAtomicCounterBuffer *buffer)
{
    m_atomicCounterToImpMap.remove(buffer->getBufferName());
}

QSharedPointer<QDemonRenderDrawIndirectBuffer> QDemonRenderContextImpl::createDrawIndirectBuffer(QDemonRenderBufferUsageType::Enum usageType,
                                                                                                 size_t size,
                                                                                                 QDemonConstDataRef<quint8> bufferData)
{
    QSharedPointer<QDemonRenderDrawIndirectBuffer> buffer = QDemonRenderDrawIndirectBuffer::create(sharedFromThis(), usageType, size, bufferData);

    if (buffer)
        m_drawIndirectToImpMap.insert(buffer->getBuffertHandle(), buffer.data());

    return buffer;
}

QSharedPointer<QDemonRenderDrawIndirectBuffer> QDemonRenderContextImpl::getDrawIndirectBuffer(QDemonRenderBackend::QDemonRenderBackendBufferObject implementationHandle)
{
    TContextDrawIndirectBufferMap::iterator entry = m_drawIndirectToImpMap.find(implementationHandle);
    if (entry != m_drawIndirectToImpMap.end())
        return entry.value()->sharedFromThis();
    return nullptr;
}

void QDemonRenderContextImpl::bufferDestroyed(QDemonRenderDrawIndirectBuffer *buffer)
{
    m_drawIndirectToImpMap.remove(buffer->getBuffertHandle());
}

void QDemonRenderContextImpl::setMemoryBarrier(QDemonRenderBufferBarrierFlags barriers)
{
    m_backend->setMemoryBarrier(barriers);
}

QSharedPointer<QDemonRenderOcclusionQuery> QDemonRenderContextImpl::createOcclusionQuery()
{
    return QDemonRenderOcclusionQuery::create(sharedFromThis());
}

QSharedPointer<QDemonRenderTimerQuery> QDemonRenderContextImpl::createTimerQuery()
{
    return QDemonRenderTimerQuery::create(sharedFromThis());
}

QSharedPointer<QDemonRenderSync> QDemonRenderContextImpl::createSync()
{
    return QDemonRenderSync::create(sharedFromThis());
}

QSharedPointer<QDemonRenderTexture2D> QDemonRenderContextImpl::createTexture2D()
{
    QSharedPointer<QDemonRenderTexture2D> retval(QDemonRenderTexture2D::create(sharedFromThis()));
    if (retval)
        m_tex2DToImpMap.insert(retval->getImplementationHandle(), retval.data());
    return retval;
}

QSharedPointer<QDemonRenderTexture2DArray> QDemonRenderContextImpl::createTexture2DArray()
{
    QSharedPointer<QDemonRenderTexture2DArray> retval(QDemonRenderTexture2DArray::create(sharedFromThis()));
    if (retval)
        m_tex2DArrayToImpMap.insert(retval->getTextureObjectHandle(), retval.data());

    return retval;
}

QSharedPointer<QDemonRenderTextureCube> QDemonRenderContextImpl::createTextureCube()
{
    QSharedPointer<QDemonRenderTextureCube> retval(QDemonRenderTextureCube::create(sharedFromThis()));
    if (retval)
        m_texCubeToImpMap.insert(retval->getTextureObjectHandle(), retval.data());

    return retval;
}

QSharedPointer<QDemonRenderTexture2D> QDemonRenderContextImpl::getTexture2D(const void *implementationHandle)
{
    const QHash<const void *, QDemonRenderTexture2D *>::iterator entry = m_tex2DToImpMap.find(implementationHandle);
    if (entry != m_tex2DToImpMap.end())
        return entry.value()->sharedFromThis();
    return nullptr;
}

void QDemonRenderContextImpl::textureDestroyed(QDemonRenderTexture2D *buffer)
{
    m_tex2DToImpMap.remove(buffer->getImplementationHandle());
    // We would like to find and catch any situations where this texture is being used
    // but that would require some real work that we don't want to do right now.
}

void QDemonRenderContextImpl::textureDestroyed(QDemonRenderTexture2DArray *buffer)
{
    m_tex2DArrayToImpMap.remove(buffer->getTextureObjectHandle());
}

void QDemonRenderContextImpl::textureDestroyed(QDemonRenderTextureCube *buffer)
{
    m_texCubeToImpMap.remove(buffer->getTextureObjectHandle());
}

QSharedPointer<QDemonRenderImage2D> QDemonRenderContextImpl::createImage2D(QSharedPointer<QDemonRenderTexture2D> inTexture,
                                                            QDemonRenderImageAccessType::Enum inAccess)
{
    QSharedPointer<QDemonRenderImage2D> retval = QDemonRenderImage2D::create(sharedFromThis(), inTexture, inAccess);
    if (retval)
        m_image2DtoImpMap.insert(retval->getTextureObjectHandle(), retval.data());

    return retval;
}

void QDemonRenderContextImpl::imageDestroyed(QDemonRenderImage2D *image)
{
    m_image2DtoImpMap.remove(image->getTextureObjectHandle());
}

// IF this texture isn't on a texture unit, put it on one.
// If it is on a texture unit, mark it as the most recently used texture.
quint32 QDemonRenderContextImpl::getNextTextureUnit()
{
    quint32 retval = m_nextTextureUnit;
    ++m_nextTextureUnit;
    // Too many texture units for a single draw call.
    if (retval >= m_maxTextureUnits) {
        Q_ASSERT(false);
        retval = retval % m_maxTextureUnits;
    }
    return retval;
}

QSharedPointer<QDemonRenderRenderBuffer>
QDemonRenderContextImpl::createRenderBuffer(QDemonRenderRenderBufferFormats::Enum bufferFormat,
                                            quint32 width, quint32 height)
{
    QSharedPointer<QDemonRenderRenderBuffer> retval =
            QDemonRenderRenderBuffer::create(sharedFromThis(), bufferFormat, width, height);
    if (retval != nullptr)
        m_renderBufferToImpMap.insert(retval->getImplementationHandle(), retval.data());
    return retval;
}

QSharedPointer<QDemonRenderRenderBuffer> QDemonRenderContextImpl::getRenderBuffer(const void *implementationHandle)
{
    const QHash<const void *, QDemonRenderRenderBuffer *>::iterator entry = m_renderBufferToImpMap.find(implementationHandle);
    if (entry != m_renderBufferToImpMap.end())
        return entry.value()->sharedFromThis();
    return nullptr;
}

void QDemonRenderContextImpl::renderBufferDestroyed(QDemonRenderRenderBuffer *buffer)
{
    m_renderBufferToImpMap.remove(buffer->getImplementationHandle());
}

QSharedPointer<QDemonRenderFrameBuffer> QDemonRenderContextImpl::createFrameBuffer()
{
    QSharedPointer<QDemonRenderFrameBuffer> retval = QDemonRenderFrameBuffer::create(sharedFromThis());
    if (retval != nullptr)
        m_frameBufferToImpMap.insert(retval->getImplementationHandle(), retval.data());
    return retval;
}

QSharedPointer<QDemonRenderFrameBuffer> QDemonRenderContextImpl::getFrameBuffer(const void *implementationHandle)
{
    const QHash<const void *, QDemonRenderFrameBuffer *>::iterator entry = m_frameBufferToImpMap.find(implementationHandle);
    if (entry != m_frameBufferToImpMap.end())
        return entry.value()->sharedFromThis();
    return nullptr;
}

void QDemonRenderContextImpl::frameBufferDestroyed(QDemonRenderFrameBuffer *fb)
{
    m_frameBufferToImpMap.remove(fb->getImplementationHandle());
    if (m_hardwarePropertyContext.m_frameBuffer == fb)
        m_hardwarePropertyContext.m_frameBuffer = nullptr;
}

QSharedPointer<QDemonRenderAttribLayout> QDemonRenderContextImpl::createAttributeLayout(QDemonConstDataRef<QDemonRenderVertexBufferEntry> attribs)
{
    return QSharedPointer<QDemonRenderAttribLayout>(new QDemonRenderAttribLayout(this->sharedFromThis(), attribs));
}

QSharedPointer<QDemonRenderInputAssembler> QDemonRenderContextImpl::createInputAssembler(QSharedPointer<QDemonRenderAttribLayout> attribLayout, QDemonConstDataRef<QSharedPointer<QDemonRenderVertexBuffer>> buffers,
        const QSharedPointer<QDemonRenderIndexBuffer> indexBuffer, QDemonConstDataRef<quint32> strides,
        QDemonConstDataRef<quint32> offsets, QDemonRenderDrawMode::Enum primType, quint32 patchVertexCount)
{
    return QSharedPointer<QDemonRenderInputAssembler>(new QDemonRenderInputAssembler(sharedFromThis(), attribLayout, buffers, indexBuffer, strides,
                                                  offsets, primType, patchVertexCount));
}

void QDemonRenderContextImpl::setInputAssembler(QSharedPointer<QDemonRenderInputAssembler> inputAssembler)
{
    if (m_hardwarePropertyContext.m_inputAssembler != inputAssembler) {
        doSetInputAssembler(inputAssembler);
    }
}

QDemonRenderVertFragCompilationResult QDemonRenderContextImpl::compileSource(
        const char *shaderName, QDemonConstDataRef<qint8> vertShader, QDemonConstDataRef<qint8> fragShader,
        QDemonConstDataRef<qint8> tessControlShaderSource,
        QDemonConstDataRef<qint8> tessEvaluationShaderSource, QDemonConstDataRef<qint8> geometryShaderSource,
        bool separateProgram, QDemonRenderShaderProgramBinaryType::Enum type, bool binaryProgram)
{
    QDemonRenderVertFragCompilationResult result = QDemonRenderShaderProgram::create(
                sharedFromThis(), shaderName, vertShader, fragShader, tessControlShaderSource,
                tessEvaluationShaderSource, geometryShaderSource, separateProgram, type, binaryProgram);

    if (result.m_shader != nullptr)
        m_shaderToImpMap.insert(result.m_shader->getShaderProgramHandle(), result.m_shader.data());

    return result;
}

QDemonRenderVertFragCompilationResult QDemonRenderContextImpl::compileBinary(
        const char *shaderName, QDemonRenderShaderProgramBinaryType::Enum type,
        QDemonDataRef<qint8> vertShader, QDemonDataRef<qint8> fragShader,
        QDemonDataRef<qint8> tessControlShaderSource, QDemonDataRef<qint8> tessEvaluationShaderSource,
        QDemonConstDataRef<qint8> geometryShaderSource)
{
#ifndef _MACOSX
    QDemonRenderVertFragCompilationResult result = QDemonRenderShaderProgram::create(
                sharedFromThis(), shaderName, vertShader, fragShader, tessControlShaderSource,
                tessEvaluationShaderSource, geometryShaderSource, false, type, true);

    if (result.m_shader != nullptr)
        m_shaderToImpMap.insert(result.m_shader->getShaderProgramHandle(), result.m_shader.data());

    return result;
#else
    Q_ASSERT(false);
    return QDemonRenderVertFragCompilationResult();
#endif
}

QDemonRenderVertFragCompilationResult
QDemonRenderContextImpl::compileComputeSource(const char *shaderName,
                                              QDemonConstDataRef<qint8> computeShaderSource)
{
    QDemonRenderVertFragCompilationResult result =
            QDemonRenderShaderProgram::createCompute(sharedFromThis(), shaderName, computeShaderSource);

    if (result.m_shader != nullptr)
        m_shaderToImpMap.insert(result.m_shader->getShaderProgramHandle(), result.m_shader.data());

    return result;
}

QSharedPointer<QDemonRenderShaderProgram> QDemonRenderContextImpl::getShaderProgram(const void *implementationHandle)
{
    const QHash<const void *, QDemonRenderShaderProgram *>::iterator entry = m_shaderToImpMap.find(implementationHandle);
    if (entry != m_shaderToImpMap.end())
        return QSharedPointer<QDemonRenderShaderProgram>(entry.value());
    return nullptr;
}

void QDemonRenderContextImpl::shaderDestroyed(QDemonRenderShaderProgram *shader)
{
    m_shaderToImpMap.remove(shader->getShaderProgramHandle());
    if (m_hardwarePropertyContext.m_activeShader.data() == shader)
        setActiveShader(nullptr);
}

QSharedPointer<QDemonRenderProgramPipeline> QDemonRenderContextImpl::createProgramPipeline()
{
    return QSharedPointer<QDemonRenderProgramPipeline>(new QDemonRenderProgramPipeline(sharedFromThis()));
}

QSharedPointer<QDemonRenderPathSpecification> QDemonRenderContextImpl::createPathSpecification()
{
    return QDemonRenderPathSpecification::createPathSpecification(sharedFromThis());
}

QSharedPointer<QDemonRenderPathRender> QDemonRenderContextImpl::createPathRender(size_t range)
{
    return QDemonRenderPathRender::create(sharedFromThis(), range);
}

void QDemonRenderContextImpl::setPathProjectionMatrix(const QMatrix4x4 inPathProjection)
{
    m_backend->setPathProjectionMatrix(inPathProjection);
}

void QDemonRenderContextImpl::setPathModelViewMatrix(const QMatrix4x4 inPathModelview)
{
    m_backend->setPathModelViewMatrix(inPathModelview);
}

void QDemonRenderContextImpl::setPathStencilDepthOffset(float inSlope, float inBias)
{
    m_backend->setPathStencilDepthOffset(inSlope, inBias);
}
void QDemonRenderContextImpl::setPathCoverDepthFunc(QDemonRenderBoolOp::Enum inFunc)
{
    m_backend->setPathCoverDepthFunc(inFunc);
}

QSharedPointer<QDemonRenderPathFontSpecification>
QDemonRenderContextImpl::createPathFontSpecification(const QString &fontName)
{
    // first check if it already exists
    QHash<QString, QDemonRenderPathFontSpecification *>::const_iterator entry = m_pathFontSpecToImpMap.find(fontName);
    if (entry != m_pathFontSpecToImpMap.end())
        return entry.value()->sharedFromThis();

    // if not create new one
    QSharedPointer<QDemonRenderPathFontSpecification> pPathFontSpec =
            QDemonRenderPathFontSpecification::createPathFontSpecification(sharedFromThis(), fontName);

    if (pPathFontSpec)
        m_pathFontSpecToImpMap.insert(fontName, pPathFontSpec.data());

    return pPathFontSpec;
}

void
QDemonRenderContextImpl::releasePathFontSpecification(QDemonRenderPathFontSpecification *inPathSpec)
{
    m_pathFontSpecToImpMap.remove(inPathSpec->getFontName());
}

QSharedPointer<QDemonRenderPathFontItem> QDemonRenderContextImpl::createPathFontItem()
{
    // if not create new one
    return QDemonRenderPathFontItem::createPathFontItem(sharedFromThis());
}

void QDemonRenderContextImpl::setClearColor(QVector4D inClearColor)
{
    if (m_hardwarePropertyContext.m_clearColor != inClearColor)
        doSetClearColor(inClearColor);
}

void QDemonRenderContextImpl::setBlendFunction(QDemonRenderBlendFunctionArgument inFunctions)
{
    if (memcmp(&inFunctions, &m_hardwarePropertyContext.m_blendFunction,
               sizeof(QDemonRenderBlendFunctionArgument))) {
        doSetBlendFunction(inFunctions);
    }
}

void QDemonRenderContextImpl::setBlendEquation(QDemonRenderBlendEquationArgument inEquations)
{
    if (memcmp(&inEquations, &m_hardwarePropertyContext.m_blendEquation,
               sizeof(QDemonRenderBlendEquationArgument))) {
        doSetBlendEquation(inEquations);
    }
}

void QDemonRenderContextImpl::setCullingEnabled(bool inEnabled)
{
    if (inEnabled != m_hardwarePropertyContext.m_cullingEnabled) {
        doSetCullingEnabled(inEnabled);
    }
}

void QDemonRenderContextImpl::setDepthFunction(QDemonRenderBoolOp::Enum inFunction)
{
    if (inFunction != m_hardwarePropertyContext.m_depthFunction) {
        doSetDepthFunction(inFunction);
    }
}

void QDemonRenderContextImpl::setBlendingEnabled(bool inEnabled)
{
    if (inEnabled != m_hardwarePropertyContext.m_blendingEnabled) {
        doSetBlendingEnabled(inEnabled);
    }
}

void QDemonRenderContextImpl::setColorWritesEnabled(bool inEnabled)
{
    if (inEnabled != m_hardwarePropertyContext.m_colorWritesEnabled) {
        doSetColorWritesEnabled(inEnabled);
    }
}

void QDemonRenderContextImpl::setDepthWriteEnabled(bool inEnabled)
{
    if (inEnabled != m_hardwarePropertyContext.m_depthWriteEnabled) {
        m_hardwarePropertyContext.m_depthWriteEnabled = inEnabled;
        m_backend->setRenderState(inEnabled, QDemonRenderState::DepthWrite);
    }
}

void QDemonRenderContextImpl::setDepthTestEnabled(bool inEnabled)
{
    if (inEnabled != m_hardwarePropertyContext.m_depthTestEnabled) {
        doSetDepthTestEnabled(inEnabled);
    }
}

void QDemonRenderContextImpl::setMultisampleEnabled(bool inEnabled)
{
    if (inEnabled != m_hardwarePropertyContext.m_multisampleEnabled) {
        doSetMultisampleEnabled(inEnabled);
    }
}

void QDemonRenderContextImpl::setStencilTestEnabled(bool inEnabled)
{
    if (inEnabled != m_hardwarePropertyContext.m_stencilTestEnabled) {
        doSetStencilTestEnabled(inEnabled);
    }
}

void QDemonRenderContextImpl::setScissorTestEnabled(bool inEnabled)
{
    if (inEnabled != m_hardwarePropertyContext.m_scissorTestEnabled) {
        doSetScissorTestEnabled(inEnabled);
    }
}

void QDemonRenderContextImpl::setScissorRect(QDemonRenderRect inRect)
{
    if (memcmp(&inRect, &m_hardwarePropertyContext.m_scissorRect, sizeof(QDemonRenderRect))) {
        doSetScissorRect(inRect);
    }
}

void QDemonRenderContextImpl::setViewport(QDemonRenderRect inViewport)
{
    if (memcmp(&inViewport, &m_hardwarePropertyContext.m_viewport, sizeof(QDemonRenderRect))) {
        doSetViewport(inViewport);
    }
}

void QDemonRenderContextImpl::setActiveShader(QSharedPointer<QDemonRenderShaderProgram> inShader)
{
    if (inShader != m_hardwarePropertyContext.m_activeShader)
        doSetActiveShader(inShader);
}

void QDemonRenderContextImpl::setActiveProgramPipeline(QSharedPointer<QDemonRenderProgramPipeline> inProgramPipeline)
{
    if (inProgramPipeline != m_hardwarePropertyContext.m_activeProgramPipeline)
        doSetActiveProgramPipeline(inProgramPipeline);
}

void QDemonRenderContextImpl::dispatchCompute(QSharedPointer<QDemonRenderShaderProgram> inShader, quint32 numGroupsX,
                                              quint32 numGroupsY, quint32 numGroupsZ)
{
    Q_ASSERT(inShader);

    if (inShader != m_hardwarePropertyContext.m_activeShader)
        doSetActiveShader(inShader);

    m_backend->dispatchCompute(inShader->getShaderProgramHandle(), numGroupsX, numGroupsY,
                               numGroupsZ);

    onPostDraw();
}

void QDemonRenderContextImpl::setDrawBuffers(QDemonConstDataRef<qint32> inDrawBufferSet)
{
    m_backend->setDrawBuffers((m_hardwarePropertyContext.m_frameBuffer) ? m_hardwarePropertyContext.m_frameBuffer->getFrameBuffertHandle()
                                                                        : nullptr,
                              inDrawBufferSet);
}

void QDemonRenderContextImpl::setReadBuffer(QDemonReadFaces::Enum inReadFace)
{
    // currently nullptr which means the read target must be set with setReadTarget
    m_backend->setReadBuffer(nullptr, inReadFace);
}

void QDemonRenderContextImpl::readPixels(QDemonRenderRect inRect,
                                         QDemonRenderReadPixelFormats::Enum inFormat,
                                         QDemonDataRef<quint8> inWriteBuffer)
{
    // nullptr means read from current render target
    m_backend->readPixel(nullptr, inRect.m_x, inRect.m_y, inRect.m_width, inRect.m_height,
                         inFormat, (void *)inWriteBuffer.begin());
}

void QDemonRenderContextImpl::setRenderTarget(QSharedPointer<QDemonRenderFrameBuffer> inBuffer)
{
    if (inBuffer != m_hardwarePropertyContext.m_frameBuffer) {
        doSetRenderTarget(inBuffer);
    }
}

void QDemonRenderContextImpl::setReadTarget(QSharedPointer<QDemonRenderFrameBuffer> inBuffer)
{
    if (inBuffer != m_hardwarePropertyContext.m_frameBuffer) {
        doSetReadTarget(inBuffer);
    }
}

void QDemonRenderContextImpl::resetBlendState()
{
    qint32_4 values;

    m_backend->setRenderState(m_hardwarePropertyContext.m_blendingEnabled,
                              QDemonRenderState::Blend);
    const QDemonRenderBlendFunctionArgument &theBlendArg(m_hardwarePropertyContext.m_blendFunction);
    m_backend->setBlendFunc(theBlendArg);
}

// Pop the entire set of properties, potentially forcing the values
// to opengl.
void QDemonRenderContextImpl::popPropertySet(bool inForceSetProperties)
{
    if (!m_propertyStack.empty()) {
        QDemonGLHardPropertyContext &theTopContext(m_propertyStack.back());
        if (inForceSetProperties) {
#define HANDLE_CONTEXT_HARDWARE_PROPERTY(setterName, propName)                                     \
    doSet##setterName(theTopContext.m_##propName);

            ITERATE_HARDWARE_CONTEXT_PROPERTIES

        #undef HANDLE_CONTEXT_HARDWARE_PROPERTY
        } else {
#define HANDLE_CONTEXT_HARDWARE_PROPERTY(setterName, propName)                                     \
    set##setterName(theTopContext.m_##propName);

            ITERATE_HARDWARE_CONTEXT_PROPERTIES

        #undef HANDLE_CONTEXT_HARDWARE_PROPERTY
        }
        m_propertyStack.pop_back();
    }
}

void QDemonRenderContextImpl::clear(QDemonRenderClearFlags flags)
{
    if ((flags & QDemonRenderClearValues::Depth)
            && m_hardwarePropertyContext.m_depthWriteEnabled == false) {
        Q_ASSERT(false);
        setDepthWriteEnabled(true);
    }
    m_backend->clear(flags);
}

void QDemonRenderContextImpl::clear(QSharedPointer<QDemonRenderFrameBuffer> fb, QDemonRenderClearFlags flags)
{
    QSharedPointer<QDemonRenderFrameBuffer> previous = m_hardwarePropertyContext.m_frameBuffer;
    if (previous != fb)
        setRenderTarget(fb);

    clear(flags);

    if (previous != fb)
        setRenderTarget(previous);
}

void QDemonRenderContextImpl::blitFramebuffer(qint32 srcX0, qint32 srcY0, qint32 srcX1, qint32 srcY1,
                                              qint32 dstX0, qint32 dstY0, qint32 dstX1, qint32 dstY1,
                                              QDemonRenderClearFlags flags,
                                              QDemonRenderTextureMagnifyingOp::Enum filter)
{
    m_backend->blitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, flags, filter);
}

bool
QDemonRenderContextImpl::bindShaderToInputAssembler(const QSharedPointer<QDemonRenderInputAssembler> inputAssembler,
                                                    QSharedPointer<QDemonRenderShaderProgram> shader)
{
    // setup the input assembler object
    return m_backend->setInputAssembler(inputAssembler->getInputAssemblerHandle(),
                                        shader->getShaderProgramHandle());
}

bool QDemonRenderContextImpl::applyPreDrawProperties()
{
    // Get the currently bound vertex and shader
    QSharedPointer<QDemonRenderInputAssembler> inputAssembler = sharedFromThis()->m_hardwarePropertyContext.m_inputAssembler;
    QSharedPointer<QDemonRenderShaderProgram> shader(sharedFromThis()->m_hardwarePropertyContext.m_activeShader);

    // we could render through a program pipline
    if (shader == nullptr && sharedFromThis()->m_hardwarePropertyContext.m_activeProgramPipeline)
        shader = sharedFromThis()->m_hardwarePropertyContext.m_activeProgramPipeline->getVertexStage();

    if (inputAssembler == nullptr || shader == nullptr) {
        qCCritical(INVALID_OPERATION,
                   "Attempting to render no valid shader or input assembler setup");
        Q_ASSERT(false);
        return false;
    }

    return bindShaderToInputAssembler(inputAssembler, shader);
}

void QDemonRenderContextImpl::onPostDraw()
{
    // reset input assembler binding
    m_backend->setInputAssembler(nullptr, 0);
    // Texture unit 0 is used for setting up and loading textures.
    // Bugs happen if we load a texture then setup the sampler.
    // Then we load another texture.  Because when loading we use texture unit 0,
    // the render bindings for the first texture are blown away.
    // Again, for this reason, texture unit 0 is reserved for loading textures.
    m_nextTextureUnit = 1;
    m_nextConstantBufferUnit = 0;
}

void QDemonRenderContextImpl::draw(QDemonRenderDrawMode::Enum drawMode, quint32 count, quint32 offset)
{
    if (!applyPreDrawProperties())
        return;

    QSharedPointer<QDemonRenderIndexBuffer> theIndexBuffer = m_hardwarePropertyContext.m_inputAssembler->getIndexBuffer();
    if (theIndexBuffer == nullptr)
        m_backend->draw(drawMode, offset, count);
    else
        theIndexBuffer->draw(drawMode, count, offset);

    onPostDraw();
}

void QDemonRenderContextImpl::drawIndirect(QDemonRenderDrawMode::Enum drawMode, quint32 offset)
{
    if (!applyPreDrawProperties())
        return;

    QSharedPointer<QDemonRenderIndexBuffer> theIndexBuffer = m_hardwarePropertyContext.m_inputAssembler->getIndexBuffer();
    if (theIndexBuffer == nullptr)
        m_backend->drawIndirect(drawMode, (const void *)offset);
    else
        theIndexBuffer->drawIndirect(drawMode, offset);

    onPostDraw();
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
    if (theVirtualViewport.m_width == 0 || theVirtualViewport.m_height == 0
            || theViewport.m_width == 0 || theViewport.m_height == 0) {
        Q_ASSERT(false);
        return inProjection;
    }
    QMatrix4x4 theScaleTransMat;
    float theHeightDiff = theViewport.m_height - theVirtualViewport.m_height;
    float theViewportOffY = theVirtualViewport.m_y - theViewport.m_y;
    QVector2D theCameraOffsets = QVector2D(theVirtualViewport.m_width - theViewport.m_width
                                           + (theVirtualViewport.m_x - theViewport.m_x) * 2.0f,
                                           theHeightDiff + (theViewportOffY - theHeightDiff) * 2.0f);
    QVector2D theCameraScale = QVector2D(theVirtualViewport.m_width / theViewport.m_width,
                                         theVirtualViewport.m_height / theViewport.m_height);

    QVector3D theTranslation(theCameraOffsets.x() / theViewport.m_width,
                             theCameraOffsets.y() / theViewport.m_height, 0);
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

QDemonRenderVertFragCompilationResult QDemonRenderContext::compileSource(const char *shaderName,
                                                                         const char *vertShader,
                                                                         quint32 inVertLen,
                                                                         const char *fragShader,
                                                                         quint32 inFragLen,
                                                                         const char *tessControlShaderSource,
                                                                         quint32 inTCLen,
                                                                         const char *tessEvaluationShaderSource,
                                                                         quint32 inTELen,
                                                                         const char *geometryShaderSource,
                                                                         quint32 inGSLen,
                                                                         bool separableProgram)
{
    return compileSource(shaderName, QDemonConstDataRef<qint8>((const qint8 *)vertShader, inVertLen),
                         QDemonConstDataRef<qint8>((const qint8 *)fragShader, inFragLen),
                         QDemonConstDataRef<qint8>((const qint8 *)tessControlShaderSource, inTCLen),
                         QDemonConstDataRef<qint8>((const qint8 *)tessEvaluationShaderSource, inTELen),
                         QDemonConstDataRef<qint8>((const qint8 *)geometryShaderSource, inGSLen), separableProgram);
}

void QDemonRenderContextImpl::doSetActiveShader(QSharedPointer<QDemonRenderShaderProgram> inShader)
{
    m_hardwarePropertyContext.m_activeShader = nullptr;
    if (!m_backend)
        return;

    if (inShader)
        m_backend->setActiveProgram(inShader->getShaderProgramHandle());
    else
        m_backend->setActiveProgram(nullptr);

    m_hardwarePropertyContext.m_activeShader = inShader;
}

void QDemonRenderContextImpl::doSetActiveProgramPipeline(QSharedPointer<QDemonRenderProgramPipeline> inProgramPipeline)
{
    if (inProgramPipeline) {
        // invalid any bound shader
        doSetActiveShader(nullptr);
        inProgramPipeline->bind();
    } else {
        m_backend->setActiveProgramPipeline(nullptr);
    }

    m_hardwarePropertyContext.m_activeProgramPipeline = inProgramPipeline;
}

QSharedPointer<QDemonRenderContext> QDemonRenderContext::createNull()
{
    QSharedPointer<QDemonRenderContext> retval;

    // create backend
    QSharedPointer<QDemonRenderContextImpl> impl(new QDemonRenderContextImpl(QDemonRenderBackendNULL::createBackend()));
    retval = impl;
    return retval;
}
QT_END_NAMESPACE
