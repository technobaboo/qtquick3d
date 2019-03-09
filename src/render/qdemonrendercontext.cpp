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
#include <QtDemonRender/qdemonrenderprogrampipeline.h>

#include <QtDemon/qdemonutils.h>

QT_BEGIN_NAMESPACE

QDemonRenderContext::QDemonRenderContext(const QDemonRef<QDemonRenderBackend> &inBackend)
    : m_backend(inBackend)
    , m_dirtyFlags(0)
    , m_defaultOffscreenRenderTarget((QDemonRenderBackend::QDemonRenderBackendRenderTargetObject) nullptr)
    , m_dephBits(16)
    , m_stencilBits(8)
    , m_nextTextureUnit(1)
    , m_nextConstantBufferUnit(1)
{
    m_maxTextureUnits = m_backend->getMaxCombinedTextureUnits();
    m_maxConstantBufferUnits = 16; // need backend query

    // get initial state
    memset(&m_hardwarePropertyContext, 0, sizeof(m_hardwarePropertyContext));

    // get default blending functions
    m_backend->getBlendFunc(&m_hardwarePropertyContext.m_blendFunction);
    // set default blend euqation
    m_hardwarePropertyContext.m_blendEquation.m_rgbEquation = QDemonRenderBlendEquation::Add;
    m_hardwarePropertyContext.m_blendEquation.m_alphaEquation = QDemonRenderBlendEquation::Add;
    // default state
    m_hardwarePropertyContext.m_cullingEnabled = m_backend->getRenderState(QDemonRenderState::CullFace);
    m_hardwarePropertyContext.m_depthFunction = m_backend->getDepthFunc();
    m_hardwarePropertyContext.m_blendingEnabled = m_backend->getRenderState(QDemonRenderState::Blend);
    m_hardwarePropertyContext.m_depthWriteEnabled = m_backend->getDepthWrite();
    m_hardwarePropertyContext.m_depthTestEnabled = m_backend->getRenderState(QDemonRenderState::DepthTest);
    m_hardwarePropertyContext.m_scissorTestEnabled = m_backend->getRenderState(QDemonRenderState::ScissorTest);
    m_backend->getScissorRect(&m_hardwarePropertyContext.m_scissorRect);
    m_backend->getViewportRect(&m_hardwarePropertyContext.m_viewport);

    m_backend->setClearColor(&m_hardwarePropertyContext.m_clearColor);
}

QDemonRenderContext::~QDemonRenderContext()
{
    Q_ASSERT(m_constantToImpMap.size() == 0);
    m_constantToImpMap.clear();
    Q_ASSERT(m_storageToImpMap.size() == 0);
    m_storageToImpMap.clear();
    Q_ASSERT(m_pathFontSpecToImpMap.size() == 0);
    m_pathFontSpecToImpMap.clear();

    m_backend = nullptr;
}

void QDemonRenderContext::maxTextureSize(qint32 &oWidth, qint32 &oHeight)
{
    qint32 theMaxTextureSize = 0;
    m_backend->getRenderBackendValue(QDemonRenderBackend::QDemonRenderBackendQuery::MaxTextureSize, &theMaxTextureSize);

    oWidth = theMaxTextureSize;
    oHeight = theMaxTextureSize;
}

void QDemonRenderContext::setDepthStencilState(QDemonRef<QDemonRenderDepthStencilState> inDepthStencilState)
{
    if (inDepthStencilState) {
        m_backend->setDepthStencilState(inDepthStencilState->handle());
        // currently we have a mixture therefore we need to update the context state
        setDepthFunction(inDepthStencilState->depthFunction());
        setDepthWriteEnabled(inDepthStencilState->depthMask());
        setDepthTestEnabled(inDepthStencilState->depthEnabled());
        setStencilTestEnabled(inDepthStencilState->stencilEnabled());
    }
}

void QDemonRenderContext::setRasterizerState(QDemonRef<QDemonRenderRasterizerState> inRasterizerState)
{
    if (inRasterizerState)
        m_backend->setRasterizerState(inRasterizerState->handle());
}

void QDemonRenderContext::registerConstantBuffer(QDemonRenderConstantBuffer *buffer)
{
    Q_ASSERT(buffer);
    m_constantToImpMap.insert(buffer->name(), buffer);
}

QDemonRef<QDemonRenderConstantBuffer> QDemonRenderContext::getConstantBuffer(const QByteArray &bufferName)
{
    TContextConstantBufferMap::iterator entry = m_constantToImpMap.find(bufferName);
    if (entry != m_constantToImpMap.end())
        return QDemonRef<QDemonRenderConstantBuffer>(entry.value());
    return nullptr;
}

void QDemonRenderContext::bufferDestroyed(QDemonRenderConstantBuffer *buffer)
{
    m_constantToImpMap.remove(buffer->name());
}

qint32 QDemonRenderContext::nextConstantBufferUnit()
{
    qint32 retval = m_nextConstantBufferUnit;
    ++m_nextConstantBufferUnit;
    // Too many texture units for a single draw call.
    if (retval >= m_maxConstantBufferUnits) {
        Q_ASSERT(false);
        retval = retval % m_maxConstantBufferUnits;
    }
    return retval;
}

void QDemonRenderContext::registerStorageBuffer(QDemonRenderStorageBuffer *buffer)
{
    m_storageToImpMap.insert(buffer->name(), buffer);
}

QDemonRef<QDemonRenderStorageBuffer> QDemonRenderContext::getStorageBuffer(const QByteArray &bufferName)
{
    TContextStorageBufferMap::iterator entry = m_storageToImpMap.find(bufferName);
    if (entry != m_storageToImpMap.end())
        return QDemonRef<QDemonRenderStorageBuffer>(entry.value());
    return nullptr;
}

void QDemonRenderContext::bufferDestroyed(QDemonRenderStorageBuffer *buffer)
{
    m_storageToImpMap.remove(buffer->name());
}

void QDemonRenderContext::registerAtomicCounterBuffer(QDemonRenderAtomicCounterBuffer *buffer)
{
    m_atomicCounterToImpMap.insert(buffer->bufferName(), buffer);
}

QDemonRef<QDemonRenderAtomicCounterBuffer> QDemonRenderContext::getAtomicCounterBuffer(const QByteArray &bufferName)
{
    TContextAtomicCounterBufferMap::iterator entry = m_atomicCounterToImpMap.find(bufferName);
    if (entry != m_atomicCounterToImpMap.end())
        return QDemonRef<QDemonRenderAtomicCounterBuffer>(entry.value());
    return QDemonRef<QDemonRenderAtomicCounterBuffer>();
}

QDemonRef<QDemonRenderAtomicCounterBuffer> QDemonRenderContext::getAtomicCounterBufferByParam(const QByteArray &paramName)
{
    // iterate through all atomic counter buffers
    for (TContextAtomicCounterBufferMap::iterator iter = m_atomicCounterToImpMap.begin(), end = m_atomicCounterToImpMap.end();
         iter != end;
         ++iter) {
        if (iter.value() && iter.value()->containsParam(paramName))
            return QDemonRef<QDemonRenderAtomicCounterBuffer>(iter.value());
    }

    return QDemonRef<QDemonRenderAtomicCounterBuffer>();
}

void QDemonRenderContext::bufferDestroyed(QDemonRenderAtomicCounterBuffer *buffer)
{
    m_atomicCounterToImpMap.remove(buffer->bufferName());
}

void QDemonRenderContext::setMemoryBarrier(QDemonRenderBufferBarrierFlags barriers)
{
    m_backend->setMemoryBarrier(barriers);
}

// IF this texture isn't on a texture unit, put it on one.
// If it is on a texture unit, mark it as the most recently used texture.
qint32 QDemonRenderContext::nextTextureUnit()
{
    qint32 retval = m_nextTextureUnit;
    ++m_nextTextureUnit;
    // Too many texture units for a single draw call.
    if (retval >= m_maxTextureUnits) {
        Q_ASSERT(false);
        retval = retval % m_maxTextureUnits;
    }
    return retval;
}

void QDemonRenderContext::frameBufferDestroyed(QDemonRenderFrameBuffer *fb)
{
    if (m_hardwarePropertyContext.m_frameBuffer == fb)
        m_hardwarePropertyContext.m_frameBuffer = nullptr;
}

QDemonRef<QDemonRenderAttribLayout> QDemonRenderContext::createAttributeLayout(QDemonConstDataRef<QDemonRenderVertexBufferEntry> attribs)
{
    return QDemonRef<QDemonRenderAttribLayout>(new QDemonRenderAttribLayout(this, attribs));
}

QDemonRef<QDemonRenderInputAssembler> QDemonRenderContext::createInputAssembler(
        QDemonRef<QDemonRenderAttribLayout> attribLayout,
        QDemonConstDataRef<QDemonRef<QDemonRenderVertexBuffer>> buffers,
        const QDemonRef<QDemonRenderIndexBuffer> indexBuffer,
        QDemonConstDataRef<quint32> strides,
        QDemonConstDataRef<quint32> offsets,
        QDemonRenderDrawMode primType,
        quint32 patchVertexCount)
{
    return QDemonRef<QDemonRenderInputAssembler>(
            new QDemonRenderInputAssembler(this, attribLayout, buffers, indexBuffer, strides, offsets, primType, patchVertexCount));
}

void QDemonRenderContext::setInputAssembler(QDemonRef<QDemonRenderInputAssembler> inputAssembler)
{
    if (m_hardwarePropertyContext.m_inputAssembler != inputAssembler)
        doSetInputAssembler(inputAssembler);
}

QDemonRenderVertFragCompilationResult QDemonRenderContext::compileSource(const char *shaderName,
                                                                             QDemonConstDataRef<qint8> vertShader,
                                                                             QDemonConstDataRef<qint8> fragShader,
                                                                             QDemonConstDataRef<qint8> tessControlShaderSource,
                                                                             QDemonConstDataRef<qint8> tessEvaluationShaderSource,
                                                                             QDemonConstDataRef<qint8> geometryShaderSource,
                                                                             bool separateProgram,
                                                                             QDemonRenderShaderProgramBinaryType type,
                                                                             bool binaryProgram)
{
    QDemonRenderVertFragCompilationResult result = QDemonRenderShaderProgram::create(this,
                                                                                     shaderName,
                                                                                     vertShader,
                                                                                     fragShader,
                                                                                     tessControlShaderSource,
                                                                                     tessEvaluationShaderSource,
                                                                                     geometryShaderSource,
                                                                                     separateProgram,
                                                                                     type,
                                                                                     binaryProgram);

    return result;
}

QDemonRenderVertFragCompilationResult QDemonRenderContext::compileBinary(const char *shaderName,
                                                                             QDemonRenderShaderProgramBinaryType type,
                                                                             QDemonDataRef<qint8> vertShader,
                                                                             QDemonDataRef<qint8> fragShader,
                                                                             QDemonDataRef<qint8> tessControlShaderSource,
                                                                             QDemonDataRef<qint8> tessEvaluationShaderSource,
                                                                             QDemonConstDataRef<qint8> geometryShaderSource)
{
#ifndef _MACOSX
    QDemonRenderVertFragCompilationResult result = QDemonRenderShaderProgram::create(this,
                                                                                     shaderName,
                                                                                     vertShader,
                                                                                     fragShader,
                                                                                     tessControlShaderSource,
                                                                                     tessEvaluationShaderSource,
                                                                                     geometryShaderSource,
                                                                                     false,
                                                                                     type,
                                                                                     true);

    return result;
#else
    Q_ASSERT(false);
    return QDemonRenderVertFragCompilationResult();
#endif
}

QDemonRenderVertFragCompilationResult QDemonRenderContext::compileComputeSource(const char *shaderName,
                                                                                    QDemonConstDataRef<qint8> computeShaderSource)
{
    QDemonRenderVertFragCompilationResult result = QDemonRenderShaderProgram::createCompute(this, shaderName, computeShaderSource);

    return result;
}


void QDemonRenderContext::shaderDestroyed(QDemonRenderShaderProgram *shader)
{
    if (m_hardwarePropertyContext.m_activeShader.data() == shader)
        setActiveShader(nullptr);
}

QDemonRef<QDemonRenderProgramPipeline> QDemonRenderContext::createProgramPipeline()
{
    return QDemonRef<QDemonRenderProgramPipeline>(new QDemonRenderProgramPipeline(this));
}

QDemonRef<QDemonRenderPathSpecification> QDemonRenderContext::createPathSpecification()
{
    return QDemonRenderPathSpecification::createPathSpecification(this);
}

QDemonRef<QDemonRenderPathRender> QDemonRenderContext::createPathRender(size_t range)
{
    return QDemonRenderPathRender::create(this, range);
}

void QDemonRenderContext::setPathProjectionMatrix(const QMatrix4x4 inPathProjection)
{
    m_backend->setPathProjectionMatrix(inPathProjection);
}

void QDemonRenderContext::setPathModelViewMatrix(const QMatrix4x4 inPathModelview)
{
    m_backend->setPathModelViewMatrix(inPathModelview);
}

void QDemonRenderContext::setPathStencilDepthOffset(float inSlope, float inBias)
{
    m_backend->setPathStencilDepthOffset(inSlope, inBias);
}
void QDemonRenderContext::setPathCoverDepthFunc(QDemonRenderBoolOp inFunc)
{
    m_backend->setPathCoverDepthFunc(inFunc);
}

QDemonRef<QDemonRenderPathFontSpecification> QDemonRenderContext::createPathFontSpecification(const QString &fontName)
{
    // first check if it already exists
    QHash<QString, QDemonRenderPathFontSpecification *>::const_iterator entry = m_pathFontSpecToImpMap.find(fontName);
    if (entry != m_pathFontSpecToImpMap.end())
        return QDemonRef<QDemonRenderPathFontSpecification>(entry.value());

    // if not create new one
    QDemonRef<QDemonRenderPathFontSpecification> pPathFontSpec = QDemonRenderPathFontSpecification::createPathFontSpecification(this, fontName);

    if (pPathFontSpec)
        m_pathFontSpecToImpMap.insert(fontName, pPathFontSpec.data());

    return pPathFontSpec;
}

void QDemonRenderContext::releasePathFontSpecification(QDemonRenderPathFontSpecification *inPathSpec)
{
    m_pathFontSpecToImpMap.remove(inPathSpec->getFontName());
}

QDemonRef<QDemonRenderPathFontItem> QDemonRenderContext::createPathFontItem()
{
    // if not create new one
    return QDemonRenderPathFontItem::createPathFontItem(this);
}

void QDemonRenderContext::setClearColor(QVector4D inClearColor)
{
    if (m_hardwarePropertyContext.m_clearColor != inClearColor)
        doSetClearColor(inClearColor);
}

void QDemonRenderContext::setBlendFunction(QDemonRenderBlendFunctionArgument inFunctions)
{
    if (memcmp(&inFunctions, &m_hardwarePropertyContext.m_blendFunction, sizeof(QDemonRenderBlendFunctionArgument))) {
        doSetBlendFunction(inFunctions);
    }
}

void QDemonRenderContext::setBlendEquation(QDemonRenderBlendEquationArgument inEquations)
{
    if (memcmp(&inEquations, &m_hardwarePropertyContext.m_blendEquation, sizeof(QDemonRenderBlendEquationArgument))) {
        doSetBlendEquation(inEquations);
    }
}

void QDemonRenderContext::setCullingEnabled(bool inEnabled)
{
    if (inEnabled != m_hardwarePropertyContext.m_cullingEnabled) {
        doSetCullingEnabled(inEnabled);
    }
}

void QDemonRenderContext::setDepthFunction(QDemonRenderBoolOp inFunction)
{
    if (inFunction != m_hardwarePropertyContext.m_depthFunction) {
        doSetDepthFunction(inFunction);
    }
}

void QDemonRenderContext::setBlendingEnabled(bool inEnabled)
{
    if (inEnabled != m_hardwarePropertyContext.m_blendingEnabled) {
        doSetBlendingEnabled(inEnabled);
    }
}

void QDemonRenderContext::setColorWritesEnabled(bool inEnabled)
{
    if (inEnabled != m_hardwarePropertyContext.m_colorWritesEnabled) {
        doSetColorWritesEnabled(inEnabled);
    }
}


void QDemonRenderContext::setDepthWriteEnabled(bool inEnabled)
{
    if (inEnabled != m_hardwarePropertyContext.m_depthWriteEnabled)
        doSetDepthWriteEnabled(inEnabled);
}

void QDemonRenderContext::setDepthTestEnabled(bool inEnabled)
{
    if (inEnabled != m_hardwarePropertyContext.m_depthTestEnabled) {
        doSetDepthTestEnabled(inEnabled);
    }
}

void QDemonRenderContext::setMultisampleEnabled(bool inEnabled)
{
    if (inEnabled != m_hardwarePropertyContext.m_multisampleEnabled) {
        doSetMultisampleEnabled(inEnabled);
    }
}

void QDemonRenderContext::setStencilTestEnabled(bool inEnabled)
{
    if (inEnabled != m_hardwarePropertyContext.m_stencilTestEnabled) {
        doSetStencilTestEnabled(inEnabled);
    }
}

void QDemonRenderContext::setScissorTestEnabled(bool inEnabled)
{
    if (inEnabled != m_hardwarePropertyContext.m_scissorTestEnabled) {
        doSetScissorTestEnabled(inEnabled);
    }
}

void QDemonRenderContext::setScissorRect(QRect inRect)
{
    if (memcmp(&inRect, &m_hardwarePropertyContext.m_scissorRect, sizeof(QRect))) {
        doSetScissorRect(inRect);
    }
}

void QDemonRenderContext::setViewport(QRect inViewport)
{
    if (memcmp(&inViewport, &m_hardwarePropertyContext.m_viewport, sizeof(QRect))) {
        doSetViewport(inViewport);
    }
}

void QDemonRenderContext::setActiveShader(QDemonRef<QDemonRenderShaderProgram> inShader)
{
    if (inShader != m_hardwarePropertyContext.m_activeShader)
        doSetActiveShader(inShader);
}

QDemonRef<QDemonRenderShaderProgram> QDemonRenderContext::activeShader() const
{
    return m_hardwarePropertyContext.m_activeShader;
}

void QDemonRenderContext::setActiveProgramPipeline(QDemonRef<QDemonRenderProgramPipeline> inProgramPipeline)
{
    if (inProgramPipeline != m_hardwarePropertyContext.m_activeProgramPipeline)
        doSetActiveProgramPipeline(inProgramPipeline);
}

QDemonRef<QDemonRenderProgramPipeline> QDemonRenderContext::activeProgramPipeline() const
{
    return m_hardwarePropertyContext.m_activeProgramPipeline;
}

void QDemonRenderContext::dispatchCompute(QDemonRef<QDemonRenderShaderProgram> inShader, quint32 numGroupsX, quint32 numGroupsY, quint32 numGroupsZ)
{
    Q_ASSERT(inShader);

    if (inShader != m_hardwarePropertyContext.m_activeShader)
        doSetActiveShader(inShader);

    m_backend->dispatchCompute(inShader->handle(), numGroupsX, numGroupsY, numGroupsZ);

    onPostDraw();
}

void QDemonRenderContext::setDrawBuffers(QDemonConstDataRef<qint32> inDrawBufferSet)
{
    m_backend->setDrawBuffers((m_hardwarePropertyContext.m_frameBuffer)
                                      ? m_hardwarePropertyContext.m_frameBuffer->handle()
                                      : nullptr,
                              inDrawBufferSet);
}

void QDemonRenderContext::setReadBuffer(QDemonReadFace inReadFace)
{
    // currently nullptr which means the read target must be set with setReadTarget
    m_backend->setReadBuffer(nullptr, inReadFace);
}

void QDemonRenderContext::readPixels(QRect inRect, QDemonRenderReadPixelFormat inFormat, QDemonDataRef<quint8> inWriteBuffer)
{
    // nullptr means read from current render target
    m_backend->readPixel(nullptr,
                         inRect.x(),
                         inRect.y(),
                         inRect.width(),
                         inRect.height(),
                         inFormat,
                         (void *)inWriteBuffer.begin());
}

void QDemonRenderContext::setRenderTarget(QDemonRef<QDemonRenderFrameBuffer> inBuffer)
{
    if (inBuffer != m_hardwarePropertyContext.m_frameBuffer)
        doSetRenderTarget(inBuffer);
}

void QDemonRenderContext::setReadTarget(QDemonRef<QDemonRenderFrameBuffer> inBuffer)
{
    if (inBuffer != m_hardwarePropertyContext.m_frameBuffer)
        doSetReadTarget(inBuffer);
}

void QDemonRenderContext::resetBlendState()
{
    qint32_4 values;

    m_backend->setRenderState(m_hardwarePropertyContext.m_blendingEnabled, QDemonRenderState::Blend);
    const QDemonRenderBlendFunctionArgument &theBlendArg(m_hardwarePropertyContext.m_blendFunction);
    m_backend->setBlendFunc(theBlendArg);
}

void QDemonRenderContext::pushPropertySet()
{
    m_propertyStack.push_back(m_hardwarePropertyContext);
}

// Pop the entire set of properties, potentially forcing the values
// to opengl.
void QDemonRenderContext::popPropertySet(bool inForceSetProperties)
{
    if (!m_propertyStack.empty()) {
        QDemonGLHardPropertyContext &theTopContext(m_propertyStack.back());
        if (inForceSetProperties) {
#define HANDLE_CONTEXT_HARDWARE_PROPERTY(setterName, propName) doSet##setterName(theTopContext.m_##propName);

            ITERATE_HARDWARE_CONTEXT_PROPERTIES

#undef HANDLE_CONTEXT_HARDWARE_PROPERTY
        } else {
#define HANDLE_CONTEXT_HARDWARE_PROPERTY(setterName, propName) set##setterName(theTopContext.m_##propName);

            ITERATE_HARDWARE_CONTEXT_PROPERTIES

#undef HANDLE_CONTEXT_HARDWARE_PROPERTY
        }
        m_propertyStack.pop_back();
    }
}

void QDemonRenderContext::clear(QDemonRenderClearFlags flags)
{
    if ((flags & QDemonRenderClearValues::Depth) && m_hardwarePropertyContext.m_depthWriteEnabled == false) {
        Q_ASSERT(false);
        setDepthWriteEnabled(true);
    }
    m_backend->clear(flags);
}

void QDemonRenderContext::clear(QDemonRef<QDemonRenderFrameBuffer> fb, QDemonRenderClearFlags flags)
{
    QDemonRef<QDemonRenderFrameBuffer> previous = m_hardwarePropertyContext.m_frameBuffer;
    if (previous != fb)
        setRenderTarget(fb);

    clear(flags);

    if (previous != fb)
        setRenderTarget(previous);
}

void QDemonRenderContext::blitFramebuffer(qint32 srcX0,
                                              qint32 srcY0,
                                              qint32 srcX1,
                                              qint32 srcY1,
                                              qint32 dstX0,
                                              qint32 dstY0,
                                              qint32 dstX1,
                                              qint32 dstY1,
                                              QDemonRenderClearFlags flags,
                                              QDemonRenderTextureMagnifyingOp filter)
{
    m_backend->blitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, flags, filter);
}

bool QDemonRenderContext::bindShaderToInputAssembler(const QDemonRef<QDemonRenderInputAssembler> &inputAssembler,
                                                         const QDemonRef<QDemonRenderShaderProgram> &shader)
{
    // setup the input assembler object
    return m_backend->setInputAssembler(inputAssembler->handle(), shader->handle());
}

bool QDemonRenderContext::applyPreDrawProperties()
{
    // Get the currently bound vertex and shader
    QDemonRef<QDemonRenderInputAssembler> inputAssembler = m_hardwarePropertyContext.m_inputAssembler;
    QDemonRef<QDemonRenderShaderProgram> shader(m_hardwarePropertyContext.m_activeShader);

    // we could render through a program pipline
    if (shader == nullptr && m_hardwarePropertyContext.m_activeProgramPipeline)
        shader = m_hardwarePropertyContext.m_activeProgramPipeline->vertexStage();

    if (inputAssembler == nullptr || shader == nullptr) {
        qCCritical(INVALID_OPERATION, "Attempting to render no valid shader or input assembler setup");
        Q_ASSERT(false);
        return false;
    }

    return bindShaderToInputAssembler(inputAssembler, shader);
}

void QDemonRenderContext::onPostDraw()
{
    // reset input assembler binding
    m_backend->setInputAssembler(nullptr, nullptr);
    // Texture unit 0 is used for setting up and loading textures.
    // Bugs happen if we load a texture then setup the sampler.
    // Then we load another texture.  Because when loading we use texture unit 0,
    // the render bindings for the first texture are blown away.
    // Again, for this reason, texture unit 0 is reserved for loading textures.
    m_nextTextureUnit = 1;
    m_nextConstantBufferUnit = 0;
}

void QDemonRenderContext::draw(QDemonRenderDrawMode drawMode, quint32 count, quint32 offset)
{
    if (!applyPreDrawProperties())
        return;

    QDemonRef<QDemonRenderIndexBuffer> theIndexBuffer = m_hardwarePropertyContext.m_inputAssembler->indexBuffer();
    if (theIndexBuffer == nullptr)
        m_backend->draw(drawMode, offset, count);
    else
        theIndexBuffer->draw(drawMode, count, offset);

    onPostDraw();
}

void QDemonRenderContext::drawIndirect(QDemonRenderDrawMode drawMode, quint32 offset)
{
    if (!applyPreDrawProperties())
        return;

    QDemonRef<QDemonRenderIndexBuffer> theIndexBuffer = m_hardwarePropertyContext.m_inputAssembler->indexBuffer();
    if (theIndexBuffer == nullptr)
        m_backend->drawIndirect(drawMode, (const void *)offset);
    else
        theIndexBuffer->drawIndirect(drawMode, offset);

    onPostDraw();
}

QMatrix4x4 QDemonRenderContext::applyVirtualViewportToProjectionMatrix(const QMatrix4x4 &inProjection,
                                                                       const QRectF &inViewport,
                                                                       const QRectF &inVirtualViewport)
{
    if (inVirtualViewport == inViewport)
        return inProjection;
    // Run conversion to floating point once.
    QRectF theVirtualViewport(inVirtualViewport);
    QRectF theViewport(inViewport);
    if (theVirtualViewport.width() == 0 || theVirtualViewport.height() == 0 || theViewport.width() == 0
        || theViewport.height() == 0) {
        Q_ASSERT(false);
        return inProjection;
    }
    QMatrix4x4 theScaleTransMat;
    float theHeightDiff = theViewport.height() - theVirtualViewport.height();
    float theViewportOffY = theVirtualViewport.y() - theViewport.y();
    QVector2D theCameraOffsets = QVector2D(theVirtualViewport.width() - theViewport.width()
                                                   + (theVirtualViewport.x() - theViewport.x()) * 2.0f,
                                           theHeightDiff + (theViewportOffY - theHeightDiff) * 2.0f);
    QVector2D theCameraScale = QVector2D(theVirtualViewport.width() / theViewport.width(),
                                         theVirtualViewport.height() / theViewport.height());

    QVector3D theTranslation(theCameraOffsets.x() / theViewport.width(), theCameraOffsets.y() / theViewport.height(), 0);
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
    return compileSource(shaderName,
                         QDemonConstDataRef<qint8>((const qint8 *)vertShader, inVertLen),
                         QDemonConstDataRef<qint8>((const qint8 *)fragShader, inFragLen),
                         QDemonConstDataRef<qint8>((const qint8 *)tessControlShaderSource, inTCLen),
                         QDemonConstDataRef<qint8>((const qint8 *)tessEvaluationShaderSource, inTELen),
                         QDemonConstDataRef<qint8>((const qint8 *)geometryShaderSource, inGSLen),
                         separableProgram);
}

void QDemonRenderContext::doSetActiveShader(const QDemonRef<QDemonRenderShaderProgram> &inShader)
{
    m_hardwarePropertyContext.m_activeShader = nullptr;
    if (!m_backend)
        return;

    if (inShader)
        m_backend->setActiveProgram(inShader->handle());
    else
        m_backend->setActiveProgram(nullptr);

    m_hardwarePropertyContext.m_activeShader = inShader;
}

void QDemonRenderContext::doSetActiveProgramPipeline(const QDemonRef<QDemonRenderProgramPipeline> &inProgramPipeline)
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
QDemonRef<QDemonRenderContext> QDemonRenderContext::createNull()
{
    QDemonRef<QDemonRenderContext> retval;

    // create backend
    QDemonRef<QDemonRenderContext> impl(new QDemonRenderContext(QDemonRenderBackendNULL::createBackend()));
    retval = impl;
    return retval;
}
QT_END_NAMESPACE
