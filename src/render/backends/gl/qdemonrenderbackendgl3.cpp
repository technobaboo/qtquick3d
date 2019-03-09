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

#include <QtDemonRender/qdemonrenderbackendgl3.h>
#include <QtDemonRender/qdemonrenderbackendinputassemblergl.h>
#include <QtDemonRender/qdemonrenderbackendrenderstatesgl.h>
#include <QtDemonRender/qdemonrenderbackendshaderprogramgl.h>

QT_BEGIN_NAMESPACE

#ifdef RENDER_BACKEND_LOG_GL_ERRORS
#define RENDER_LOG_ERROR_PARAMS(x) checkGLError(#x, __FILE__, __LINE__)
#else
#define RENDER_LOG_ERROR_PARAMS(x) checkGLError()
#endif

#define GL_CALL_EXTRA_FUNCTION(x)                                                                                      \
    m_glExtraFunctions->x;                                                                                             \
    RENDER_LOG_ERROR_PARAMS(x);

#if defined(QT_OPENGL_ES)
#define GL_CALL_TIMER_EXT(x)                                                                                           \
    m_qdemonExtensions->x;                                                                                             \
    RENDER_LOG_ERROR_PARAMS(x);
#define GL_CALL_TESSELATION_EXT(x)                                                                                     \
    m_qdemonExtensions->x;                                                                                             \
    RENDER_LOG_ERROR_PARAMS(x);
#else
#define GL_CALL_TIMER_EXT(x)                                                                                           \
    m_timerExtension->x;                                                                                               \
    RENDER_LOG_ERROR_PARAMS(x);
#define GL_CALL_TESSELATION_EXT(x)                                                                                     \
    m_tessellationShader->x;                                                                                           \
    RENDER_LOG_ERROR_PARAMS(x);
#define GL_CALL_MULTISAMPLE_EXT(x)                                                                                     \
    m_multiSample->x;                                                                                                  \
    RENDER_LOG_ERROR_PARAMS(x);
#endif

#ifndef GL_PATCH_VERTICES
#define GL_PATCH_VERTICES 0x8E72
#endif

namespace QDemonGlExtStrings {
QByteArray extsAstcHDR()
{
    return QByteArrayLiteral("GL_KHR_texture_compression_astc_hdr");
}
QByteArray extsAstcLDR()
{
    return QByteArrayLiteral("GL_KHR_texture_compression_astc_ldr");
}
}

/// constructor
QDemonRenderBackendGL3Impl::QDemonRenderBackendGL3Impl(const QSurfaceFormat &format) : QDemonRenderBackendGLBase(format)
{
    const char *languageVersion = getShadingLanguageVersion();
    qCInfo(TRACE_INFO, "GLSL version: %s", languageVersion);

    const QByteArray apiVersion(getVersionString());
    qCInfo(TRACE_INFO, "GL version: %s", apiVersion.constData());

    const QByteArray apiVendor(getVendorString());
    qCInfo(TRACE_INFO, "HW vendor: %s", apiVendor.constData());

    const QByteArray apiRenderer(getRendererString());
    qCInfo(TRACE_INFO, "Vendor renderer: %s", apiRenderer.constData());

    // clear support bits
    m_backendSupport.caps.u32Values = 0;

    // get extension count
    GLint numExtensions = 0;
    GL_CALL_EXTRA_FUNCTION(glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions));

    QByteArray extensionBuffer;

    for (qint32 i = 0; i < numExtensions; i++) {
        const GLubyte *glExt = GL_CALL_EXTRA_FUNCTION(glGetStringi(GL_EXTENSIONS, GLuint(i)));
        const QByteArray extensionString(reinterpret_cast<const char *>(glExt));

        m_extensions.push_back(extensionString);

        if (extensionBuffer.size())
            extensionBuffer.append(" ");
        extensionBuffer.append(extensionString);

        // search for extension
        if (!m_backendSupport.caps.bits.bDXTImagesSupported
            && (QDemonGlExtStrings::exts3tc().compare(extensionString) == 0
                || QDemonGlExtStrings::extsdxt().compare(extensionString) == 0)) {
            m_backendSupport.caps.bits.bDXTImagesSupported = true;
        } else if (!m_backendSupport.caps.bits.bAnistropySupported && QDemonGlExtStrings::extsAniso().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bAnistropySupported = true;
        } else if (!m_backendSupport.caps.bits.bFPRenderTargetsSupported
                   && QDemonGlExtStrings::extsFPRenderTarget().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bFPRenderTargetsSupported = true;
        } else if (!m_backendSupport.caps.bits.bTimerQuerySupported
                   && QDemonGlExtStrings::extsTimerQuery().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bTimerQuerySupported = true;
        } else if (!m_backendSupport.caps.bits.bGPUShader5ExtensionSupported
                   && QDemonGlExtStrings::extsGpuShader5().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bGPUShader5ExtensionSupported = true;
        }
    }

    qCInfo(TRACE_INFO, "OpenGL extensions: %s", extensionBuffer.constData());

    // texture swizzle is always true
    m_backendSupport.caps.bits.bTextureSwizzleSupported = true;
    // depthstencil renderbuffer support is always true
    m_backendSupport.caps.bits.bDepthStencilSupported = true;
    // constant buffers support is always true
    m_backendSupport.caps.bits.bConstantBufferSupported = true;
    m_backendSupport.caps.bits.bStandardDerivativesSupported = true;
    m_backendSupport.caps.bits.bVertexArrayObjectSupported = true;
    m_backendSupport.caps.bits.bTextureLodSupported = true;

    if (!isESCompatible()) {
        // render to float textures is always supported on none ES systems which support >=GL3
        m_backendSupport.caps.bits.bFPRenderTargetsSupported = true;
        // multisampled texture is always supported on none ES systems which support >=GL3
        m_backendSupport.caps.bits.bMsTextureSupported = true;
        // timer queries are always supported on none ES systems which support >=GL3
        m_backendSupport.caps.bits.bTimerQuerySupported = true;
    }

    // query hardware
    GL_CALL_EXTRA_FUNCTION(glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &m_maxAttribCount));

    // internal state tracker
    m_currentMiscState = new QDemonRenderBackendMiscStateGL();

    // finally setup caps based on device
    setAndInspectHardwareCaps();

    // Initialize extensions
#if defined(QT_OPENGL_ES_2)
    m_qdemonExtensions = new QDemonOpenGLES2Extensions;
    m_qdemonExtensions->initializeOpenGLFunctions();
#else
    m_timerExtension = new QOpenGLExtension_ARB_timer_query;
    m_timerExtension->initializeOpenGLFunctions();
    m_tessellationShader = new QOpenGLExtension_ARB_tessellation_shader;
    m_tessellationShader->initializeOpenGLFunctions();
    m_multiSample = new QOpenGLExtension_ARB_texture_multisample;
    m_multiSample->initializeOpenGLFunctions();
    m_qdemonExtensions = new QDemonOpenGLExtensions;
    m_qdemonExtensions->initializeOpenGLFunctions();
#endif
}
/// destructor
QDemonRenderBackendGL3Impl::~QDemonRenderBackendGL3Impl()
{
    delete m_currentMiscState;
#if !defined(QT_OPENGL_ES_2)
    delete m_timerExtension;
    delete m_tessellationShader;
    delete m_multiSample;
#endif
    delete m_qdemonExtensions;
}

void QDemonRenderBackendGL3Impl::setMultisampledTextureData2D(QDemonRenderBackendTextureObject to,
                                                              QDemonRenderTextureTargetType target,
                                                              qint32 samples,
                                                              QDemonRenderTextureFormat internalFormat,
                                                              qint32 width,
                                                              qint32 height,
                                                              bool fixedsamplelocations)
{
    // Not supported by ES 3 yet
#if defined(QT_OPENGL_ES)
    Q_UNUSED(to)
    Q_UNUSED(target)
    Q_UNUSED(samples)
    Q_UNUSED(internalFormat)
    Q_UNUSED(width)
    Q_UNUSED(height)
    Q_UNUSED(fixedsamplelocations)
#else
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GLenum glTarget = GLConversion::fromTextureTargetToGL(target);
    GL_CALL_EXTRA_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_EXTRA_FUNCTION(glBindTexture(glTarget, texID));

    QDemonRenderTextureSwizzleMode swizzleMode = QDemonRenderTextureSwizzleMode::NoSwizzle;
    internalFormat = GLConversion::replaceDeprecatedTextureFormat(getRenderContextType(), internalFormat, swizzleMode);

    GLenum glformat = 0, glInternalFormat = 0, gltype = GL_UNSIGNED_BYTE;

    if (internalFormat.isUncompressedTextureFormat())
        GLConversion::fromUncompressedTextureFormatToGL(getRenderContextType(), internalFormat, glformat, gltype, glInternalFormat);
    else if (internalFormat.isDepthTextureFormat())
        GLConversion::fromDepthTextureFormatToGL(getRenderContextType(), internalFormat, glformat, gltype, glInternalFormat);

    GL_CALL_MULTISAMPLE_EXT(
            glTexImage2DMultisample(glTarget, GLsizei(samples), glInternalFormat, GLsizei(width), GLsizei(height), fixedsamplelocations));

    GL_CALL_EXTRA_FUNCTION(glBindTexture(glTarget, 0));
#endif
}

void QDemonRenderBackendGL3Impl::setTextureData3D(QDemonRenderBackendTextureObject to,
                                                  QDemonRenderTextureTargetType target,
                                                  qint32 level,
                                                  QDemonRenderTextureFormat internalFormat,
                                                  qint32 width,
                                                  qint32 height,
                                                  qint32 depth,
                                                  qint32 border,
                                                  QDemonRenderTextureFormat format,
                                                  const void *hostPtr)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GLenum glTarget = GLConversion::fromTextureTargetToGL(target);
    GL_CALL_EXTRA_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_EXTRA_FUNCTION(glBindTexture(glTarget, texID));
    bool conversionRequired = format != internalFormat;

    QDemonRenderTextureSwizzleMode swizzleMode = QDemonRenderTextureSwizzleMode::NoSwizzle;
    internalFormat = GLConversion::replaceDeprecatedTextureFormat(getRenderContextType(), internalFormat, swizzleMode);

    GLenum glformat = 0, glInternalFormat = 0, gltype = GL_UNSIGNED_BYTE;

    if (internalFormat.isUncompressedTextureFormat())
        GLConversion::fromUncompressedTextureFormatToGL(getRenderContextType(), internalFormat, glformat, gltype, glInternalFormat);

    if (conversionRequired) {
        GLenum dummy;
        GLConversion::fromUncompressedTextureFormatToGL(getRenderContextType(), format, glformat, gltype, dummy);
    } else if (internalFormat.isCompressedTextureFormat()) {
        GLConversion::fromUncompressedTextureFormatToGL(getRenderContextType(), format, glformat, gltype, glInternalFormat);
        glInternalFormat = GLConversion::fromCompressedTextureFormatToGL(internalFormat);
    } else if (format.isDepthTextureFormat())
        GLConversion::fromDepthTextureFormatToGL(getRenderContextType(), format, glformat, gltype, glInternalFormat);

    GL_CALL_EXTRA_FUNCTION(
            glTexImage3D(glTarget, level, glInternalFormat, GLsizei(width), GLsizei(height), GLsizei(depth), border, glformat, gltype, hostPtr));

    GL_CALL_EXTRA_FUNCTION(glBindTexture(glTarget, 0));
}

void QDemonRenderBackendGL3Impl::updateSampler(QDemonRenderBackendSamplerObject /* so */,
                                               QDemonRenderTextureTargetType target,
                                               QDemonRenderTextureMinifyingOp minFilter,
                                               QDemonRenderTextureMagnifyingOp magFilter,
                                               QDemonRenderTextureCoordOp wrapS,
                                               QDemonRenderTextureCoordOp wrapT,
                                               QDemonRenderTextureCoordOp wrapR,
                                               float minLod,
                                               float maxLod,
                                               float lodBias,
                                               QDemonRenderTextureCompareMode compareMode,
                                               QDemonRenderTextureCompareOp compareFunc,
                                               float anisotropy,
                                               float *borderColor)
{

    // Satisfy the compiler
    // These are not available in GLES 3 and we don't use them right now
    Q_ASSERT(qFuzzyIsNull(lodBias));
    Q_ASSERT(!borderColor);
    Q_UNUSED(lodBias)
    Q_UNUSED(borderColor)

    GLenum glTarget = GLConversion::fromTextureTargetToGL(target);

    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_MIN_FILTER, m_conversion.fromTextureMinifyingOpToGL(minFilter)));
    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_MAG_FILTER, m_conversion.fromTextureMagnifyingOpToGL(magFilter)));
    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_WRAP_S, m_conversion.fromTextureCoordOpToGL(wrapS)));
    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_WRAP_T, m_conversion.fromTextureCoordOpToGL(wrapT)));
    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_WRAP_R, m_conversion.fromTextureCoordOpToGL(wrapR)));
    GL_CALL_EXTRA_FUNCTION(glTexParameterf(glTarget, GL_TEXTURE_MIN_LOD, minLod));
    GL_CALL_EXTRA_FUNCTION(glTexParameterf(glTarget, GL_TEXTURE_MAX_LOD, maxLod));
    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_COMPARE_MODE, m_conversion.fromTextureCompareModeToGL(compareMode)));
    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_COMPARE_FUNC, m_conversion.fromTextureCompareFuncToGL(compareFunc)));

    if (m_backendSupport.caps.bits.bAnistropySupported) {
        GL_CALL_EXTRA_FUNCTION(glTexParameterf(glTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy));
    }
}

void QDemonRenderBackendGL3Impl::updateTextureObject(QDemonRenderBackendTextureObject to,
                                                     QDemonRenderTextureTargetType target,
                                                     qint32 baseLevel,
                                                     qint32 maxLevel)
{
    Q_UNUSED(to)

    GLenum glTarget = GLConversion::fromTextureTargetToGL(target);

    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_BASE_LEVEL, baseLevel));
    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_MAX_LEVEL, maxLevel));
}

void QDemonRenderBackendGL3Impl::updateTextureSwizzle(QDemonRenderBackendTextureObject to,
                                                      QDemonRenderTextureTargetType target,
                                                      QDemonRenderTextureSwizzleMode swizzleMode)
{
    Q_UNUSED(to)
    if (m_backendSupport.caps.bits.bTextureSwizzleSupported) {
        GLint glSwizzle[4];
        GLenum glTarget = GLConversion::fromTextureTargetToGL(target);
        GLConversion::NVRenderConvertSwizzleModeToGL(swizzleMode, glSwizzle);
#if defined(QT_OPENGL_ES)
        // since ES3 spec has no GL_TEXTURE_SWIZZLE_RGBA set it separately
        GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_SWIZZLE_R, glSwizzle[0]));
        GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_SWIZZLE_G, glSwizzle[1]));
        GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_SWIZZLE_B, glSwizzle[2]));
        GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_SWIZZLE_A, glSwizzle[3]));
#else
        GL_CALL_EXTRA_FUNCTION(glTexParameteriv(glTarget, GL_TEXTURE_SWIZZLE_RGBA, glSwizzle));
#endif
    }
}

qint32 QDemonRenderBackendGL3Impl::getDepthBits() const
{
    qint32 depthBits;
    GL_CALL_EXTRA_FUNCTION(
            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depthBits));

    return depthBits;
}

qint32 QDemonRenderBackendGL3Impl::getStencilBits() const
{
    qint32 stencilBits;
    GL_CALL_EXTRA_FUNCTION(
            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &stencilBits));

    return stencilBits;
}

void QDemonRenderBackendGL3Impl::generateMipMaps(QDemonRenderBackendTextureObject to,
                                                 QDemonRenderTextureTargetType target,
                                                 QDemonRenderHint /*genType*/)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GLenum glTarget = GLConversion::fromTextureTargetToGL(target);
    GL_CALL_EXTRA_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_EXTRA_FUNCTION(glBindTexture(glTarget, texID));
    GL_CALL_EXTRA_FUNCTION(glGenerateMipmap(glTarget));
    GL_CALL_EXTRA_FUNCTION(glBindTexture(glTarget, 0));
}

bool QDemonRenderBackendGL3Impl::setInputAssembler(QDemonRenderBackendInputAssemblerObject iao, QDemonRenderBackendShaderProgramObject po)
{
    if (iao == nullptr) {
        // unbind and return;
        GL_CALL_EXTRA_FUNCTION(glBindVertexArray(0));
        return true;
    }

    QDemonRenderBackendInputAssemblerGL *inputAssembler = reinterpret_cast<QDemonRenderBackendInputAssemblerGL *>(iao);
    QDemonRenderBackendAttributeLayoutGL *attribLayout = inputAssembler->m_attribLayout;
    QDemonRenderBackendShaderProgramGL *pProgram = reinterpret_cast<QDemonRenderBackendShaderProgramGL *>(po);
    GLuint programID = static_cast<GLuint>(pProgram->m_programID);
    QDemonDataRef<QDemonRenderBackendShaderInputEntryGL> shaderAttribBuffer;
    if (pProgram->m_shaderInput)
        shaderAttribBuffer = pProgram->m_shaderInput->m_shaderInputEntries;

    if (attribLayout->m_layoutAttribEntries.size() < shaderAttribBuffer.size())
        return false;

    if (inputAssembler->m_vertexbufferHandles.size() <= attribLayout->m_maxInputSlot) {
        Q_ASSERT(false);
        return false;
    }

    if (inputAssembler->m_vaoID == 0) {
        // generate vao
        GL_CALL_EXTRA_FUNCTION(glGenVertexArrays(1, &inputAssembler->m_vaoID));
        Q_ASSERT(inputAssembler->m_vaoID);
    }

    // set patch parameter count if changed
    if (m_backendSupport.caps.bits.bTessellationSupported && m_currentMiscState->m_patchVertexCount != inputAssembler->m_patchVertexCount) {
        m_currentMiscState->m_patchVertexCount = inputAssembler->m_patchVertexCount;
#if defined(QT_OPENGL_ES)
        GL_CALL_TESSELATION_EXT(glPatchParameteriEXT(GL_PATCH_VERTICES, inputAssembler->m_PatchVertexCount));
#else
        GL_CALL_TESSELATION_EXT(glPatchParameteri(GL_PATCH_VERTICES, inputAssembler->m_patchVertexCount));
#endif
    }

    if (inputAssembler->m_cachedShaderHandle != programID) {
        GL_CALL_EXTRA_FUNCTION(glBindVertexArray(inputAssembler->m_vaoID));
        inputAssembler->m_cachedShaderHandle = programID;

        for (const auto &attrib : qAsConst(shaderAttribBuffer)) {
            QDemonRenderBackendLayoutEntryGL *entry = attribLayout->getEntryByName(attrib.m_attribName);

            if (entry) {
                QDemonRenderBackendLayoutEntryGL &entryData(*entry);
                if (entryData.m_type != attrib.m_type || entryData.m_numComponents != attrib.m_numComponents) {
                    qCCritical(INVALID_OPERATION, "Attrib %s doesn't match vertex layout", qPrintable(attrib.m_attribName));
                    Q_ASSERT(false);
                    return false;
                } else {
                    entryData.m_attribIndex = attrib.m_attribLocation;
                }
            } else {
                qCWarning(WARNING, "Failed to Bind attribute %s", qPrintable(attrib.m_attribName));
            }
        }

        // disable max possible used first
        // this is currently sufficient since we always re-arrange input attributes from 0
        for (quint32 i = 0; i < attribLayout->m_layoutAttribEntries.size(); i++) {
            GL_CALL_EXTRA_FUNCTION(glDisableVertexAttribArray(i));
        }

        // setup all attribs
        for (int idx = 0; idx != shaderAttribBuffer.size(); ++idx) {
            QDemonRenderBackendLayoutEntryGL *entry = attribLayout->getEntryByName(shaderAttribBuffer[idx].m_attribName);
            if (entry) {
                const QDemonRenderBackendLayoutEntryGL &entryData(*entry);
                GLuint id = HandleToID_cast(GLuint, size_t, inputAssembler->m_vertexbufferHandles.mData[entryData.m_inputSlot]);
                GL_CALL_EXTRA_FUNCTION(glBindBuffer(GL_ARRAY_BUFFER, id));
                GL_CALL_EXTRA_FUNCTION(glEnableVertexAttribArray(entryData.m_attribIndex));
                GLuint offset = inputAssembler->m_offsets[entryData.m_inputSlot];
                GLuint stride = inputAssembler->m_strides[entryData.m_inputSlot];
                GL_CALL_EXTRA_FUNCTION(glVertexAttribPointer(entryData.m_attribIndex,
                                                             entryData.m_numComponents,
                                                             GL_FLOAT,
                                                             GL_FALSE,
                                                             stride,
                                                             (const void *)(entryData.m_offset + offset)));

            } else {
                GL_CALL_EXTRA_FUNCTION(glDisableVertexAttribArray(idx));
            }
        }

        // setup index buffer.
        if (inputAssembler->m_indexbufferHandle) {
            GL_CALL_EXTRA_FUNCTION(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                                                HandleToID_cast(GLuint, size_t, inputAssembler->m_indexbufferHandle)));
        } else {
            GL_CALL_EXTRA_FUNCTION(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        }
    } else {
        GL_CALL_EXTRA_FUNCTION(glBindVertexArray(inputAssembler->m_vaoID));
    }
#ifdef _DEBUG
    if (inputAssembler->m_vaoID) {
        for (const auto &attrib : qAsConst(shaderAttribBuffer)) {
            QDemonRenderBackendLayoutEntryGL *entry = attribLayout->getEntryByName(attrib.m_attribName);

            if (entry) {
                QDemonRenderBackendLayoutEntryGL &entryData(*entry);
                if (entryData.m_type != attrib.m_type || entryData.m_numComponents != attrib.m_numComponents
                    || entryData.m_attribIndex != attrib.m_attribLocation) {
                    qCCritical(INVALID_OPERATION, "Attrib %s doesn't match vertex layout", qPrintable(attrib.m_attribName));
                    Q_ASSERT(false);
                }
            } else {
                qCWarning(WARNING, "Failed to Bind attribute %s", qPrintable(attrib.m_attribName));
            }
        }
    }
#endif // _DEBUG

    return true;
}

void QDemonRenderBackendGL3Impl::setDrawBuffers(QDemonRenderBackendRenderTargetObject rto, QDemonConstDataRef<qint32> inDrawBufferSet)
{
    Q_UNUSED(rto)

    m_drawBuffersArray.clear();

    for (quint32 idx = 0, end = inDrawBufferSet.size(); idx < end; ++idx) {
        if (inDrawBufferSet[idx] < 0)
            m_drawBuffersArray.push_back(GL_NONE);
        else
            m_drawBuffersArray.push_back(GL_COLOR_ATTACHMENT0 + inDrawBufferSet[idx]);
    }

    GL_CALL_EXTRA_FUNCTION(glDrawBuffers((int)m_drawBuffersArray.size(), m_drawBuffersArray.data()));
}

void QDemonRenderBackendGL3Impl::setReadBuffer(QDemonRenderBackendRenderTargetObject rto, QDemonReadFace inReadFace)
{
    Q_UNUSED(rto)

    GL_CALL_EXTRA_FUNCTION(glReadBuffer(m_conversion.fromReadFacesToGL(inReadFace)));
}

void QDemonRenderBackendGL3Impl::renderTargetAttach(QDemonRenderBackendRenderTargetObject,
                                                    QDemonRenderFrameBufferAttachment attachment,
                                                    QDemonRenderBackendTextureObject to,
                                                    qint32 level,
                                                    qint32 layer)
{
    // rto must be the current render target
    GLuint texID = HandleToID_cast(GLuint, size_t, to);

    GLenum glAttach = GLConversion::fromFramebufferAttachmentsToGL(attachment);

    GL_CALL_EXTRA_FUNCTION(glFramebufferTextureLayer(GL_FRAMEBUFFER, glAttach, texID, level, layer))
}

void QDemonRenderBackendGL3Impl::setReadTarget(QDemonRenderBackendRenderTargetObject rto)
{
    GLuint fboID = HandleToID_cast(GLuint, size_t, rto);

    GL_CALL_EXTRA_FUNCTION(glBindFramebuffer(GL_READ_FRAMEBUFFER, fboID));
}

void QDemonRenderBackendGL3Impl::blitFramebuffer(qint32 srcX0,
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
    GL_CALL_EXTRA_FUNCTION(glBlitFramebuffer(srcX0,
                                             srcY0,
                                             srcX1,
                                             srcY1,
                                             dstX0,
                                             dstY0,
                                             dstX1,
                                             dstY1,
                                             m_conversion.fromClearFlagsToGL(flags),
                                             m_conversion.fromTextureMagnifyingOpToGL(filter)));
}

void *QDemonRenderBackendGL3Impl::mapBuffer(QDemonRenderBackendBufferObject,
                                            QDemonRenderBufferBindType bindFlags,
                                            size_t offset,
                                            size_t length,
                                            QDemonRenderBufferAccessFlags accessFlags)
{
    void *ret = nullptr;
    ret = GL_CALL_EXTRA_FUNCTION(glMapBufferRange(m_conversion.fromBindBufferFlagsToGL(bindFlags),
                                                  offset,
                                                  length,
                                                  m_conversion.fromBufferAccessBitToGL(accessFlags)));

    return ret;
}

bool QDemonRenderBackendGL3Impl::unmapBuffer(QDemonRenderBackendBufferObject, QDemonRenderBufferBindType bindFlags)
{
    GLboolean ret;

    ret = GL_CALL_EXTRA_FUNCTION(glUnmapBuffer(m_conversion.fromBindBufferFlagsToGL(bindFlags)));

    return (ret) ? true : false;
}

qint32 QDemonRenderBackendGL3Impl::getConstantBufferCount(QDemonRenderBackendShaderProgramObject po)
{
    Q_ASSERT(po);
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_programID);

    GLint numUniformBuffers;
    GL_CALL_EXTRA_FUNCTION(glGetProgramiv(programID, GL_ACTIVE_UNIFORM_BLOCKS, &numUniformBuffers));

    return numUniformBuffers;
}

qint32 QDemonRenderBackendGL3Impl::getConstantBufferInfoByID(QDemonRenderBackendShaderProgramObject po,
                                                             quint32 id,
                                                             quint32 nameBufSize,
                                                             qint32 *paramCount,
                                                             qint32 *bufferSize,
                                                             qint32 *length,
                                                             char *nameBuf)
{
    Q_ASSERT(po);
    Q_ASSERT(length);
    Q_ASSERT(nameBuf);

    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_programID);
    GLuint blockIndex = GL_INVALID_INDEX;

    GL_CALL_EXTRA_FUNCTION(glGetActiveUniformBlockName(programID, id, nameBufSize, length, nameBuf));

    if (*length > 0) {
        blockIndex = GL_CALL_EXTRA_FUNCTION(glGetUniformBlockIndex(programID, nameBuf));
        if (blockIndex != GL_INVALID_INDEX) {
            GL_CALL_EXTRA_FUNCTION(glGetActiveUniformBlockiv(programID, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, bufferSize));
            GL_CALL_EXTRA_FUNCTION(glGetActiveUniformBlockiv(programID, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, paramCount));
        }
    }

    return blockIndex;
}

void QDemonRenderBackendGL3Impl::getConstantBufferParamIndices(QDemonRenderBackendShaderProgramObject po, quint32 id, qint32 *indices)
{
    Q_ASSERT(po);
    Q_ASSERT(indices);

    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_programID);

    if (indices) {
        GL_CALL_EXTRA_FUNCTION(glGetActiveUniformBlockiv(programID, id, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices));
    }
}

void QDemonRenderBackendGL3Impl::getConstantBufferParamInfoByIndices(QDemonRenderBackendShaderProgramObject po,
                                                                     quint32 count,
                                                                     quint32 *indices,
                                                                     QDemonRenderShaderDataType *type,
                                                                     qint32 *size,
                                                                     qint32 *offset)
{
    Q_ASSERT(po);
    Q_ASSERT(count);
    Q_ASSERT(indices);

    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_programID);

    if (count && indices) {
        if (type) {
            qint32 *glTypes = reinterpret_cast<qint32 *>(alloca(count*sizeof(qint32)));
            GL_CALL_EXTRA_FUNCTION(glGetActiveUniformsiv(programID, count, indices, GL_UNIFORM_TYPE, glTypes));
            // convert to UIC types
            for (int idx = 0; idx != count; ++idx)
                type[idx] = GLConversion::fromShaderGLToPropertyDataTypes(glTypes[idx]);
        }
        if (size) {
            GL_CALL_EXTRA_FUNCTION(glGetActiveUniformsiv(programID, count, indices, GL_UNIFORM_SIZE, size));
        }
        if (offset) {
            GL_CALL_EXTRA_FUNCTION(glGetActiveUniformsiv(programID, count, indices, GL_UNIFORM_OFFSET, offset));
        }
    }
}

void QDemonRenderBackendGL3Impl::programSetConstantBlock(QDemonRenderBackendShaderProgramObject po, quint32 blockIndex, quint32 binding)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_programID);

    GL_CALL_EXTRA_FUNCTION(glUniformBlockBinding(programID, blockIndex, binding));
}

void QDemonRenderBackendGL3Impl::programSetConstantBuffer(quint32 index, QDemonRenderBackendBufferObject bo)
{
    Q_ASSERT(bo);

    GLuint bufID = HandleToID_cast(GLuint, size_t, bo);
    GL_CALL_EXTRA_FUNCTION(glBindBufferBase(GL_UNIFORM_BUFFER, index, bufID));
}

QDemonRenderBackend::QDemonRenderBackendQueryObject QDemonRenderBackendGL3Impl::createQuery()
{
    quint32 glQueryID = 0;

    GL_CALL_EXTRA_FUNCTION(glGenQueries(1, &glQueryID));

    return (QDemonRenderBackendQueryObject)glQueryID;
}

void QDemonRenderBackendGL3Impl::releaseQuery(QDemonRenderBackendQueryObject qo)
{
    GLuint queryID = HandleToID_cast(GLuint, size_t, qo);

    GL_CALL_EXTRA_FUNCTION(glDeleteQueries(1, &queryID));
}

void QDemonRenderBackendGL3Impl::beginQuery(QDemonRenderBackendQueryObject qo, QDemonRenderQueryType type)
{
    GLuint queryID = HandleToID_cast(GLuint, size_t, qo);

    GL_CALL_EXTRA_FUNCTION(glBeginQuery(m_conversion.fromQueryTypeToGL(type), queryID));
}

void QDemonRenderBackendGL3Impl::endQuery(QDemonRenderBackendQueryObject, QDemonRenderQueryType type)
{
    GL_CALL_EXTRA_FUNCTION(glEndQuery(m_conversion.fromQueryTypeToGL(type)));
}

void QDemonRenderBackendGL3Impl::getQueryResult(QDemonRenderBackendQueryObject qo,
                                                QDemonRenderQueryResultType resultType,
                                                quint32 *params)
{
    GLuint queryID = HandleToID_cast(GLuint, size_t, qo);

    if (params)
        GL_CALL_EXTRA_FUNCTION(glGetQueryObjectuiv(queryID, m_conversion.fromQueryResultTypeToGL(resultType), params));
}

void QDemonRenderBackendGL3Impl::getQueryResult(QDemonRenderBackendQueryObject qo,
                                                QDemonRenderQueryResultType resultType,
                                                quint64 *params)
{
    // TODO: params type!
    if (m_backendSupport.caps.bits.bTimerQuerySupported) {
        GLuint queryID = HandleToID_cast(GLuint, size_t, qo);

        if (params)
#if defined(QT_OPENGL_ES)
            GL_CALL_TIMER_EXT(glGetQueryObjectui64vEXT(queryID, m_Conversion.fromQueryResultTypeToGL(resultType), params));
#else
            GL_CALL_TIMER_EXT(glGetQueryObjectui64v(queryID, m_conversion.fromQueryResultTypeToGL(resultType), (GLuint64 *)params));
#endif
    }
}

void QDemonRenderBackendGL3Impl::setQueryTimer(QDemonRenderBackendQueryObject qo)
{
    if (m_backendSupport.caps.bits.bTimerQuerySupported) {
        GLuint queryID = HandleToID_cast(GLuint, size_t, qo);
#if defined(QT_OPENGL_ES)
        GL_CALL_TIMER_EXT(glQueryCounterEXT(queryID, GL_TIMESTAMP_EXT));
#else
        GL_CALL_TIMER_EXT(glQueryCounter(queryID, GL_TIMESTAMP));
#endif
    }
}

QDemonRenderBackend::QDemonRenderBackendSyncObject QDemonRenderBackendGL3Impl::createSync(QDemonRenderSyncType syncType,
                                                                                          QDemonRenderSyncFlags)
{
    GLsync syncID = nullptr;

    syncID = GL_CALL_EXTRA_FUNCTION(glFenceSync(m_conversion.fromSyncTypeToGL(syncType), 0));

    return QDemonRenderBackendSyncObject(syncID);
}

void QDemonRenderBackendGL3Impl::releaseSync(QDemonRenderBackendSyncObject so)
{
    GLsync syncID = (GLsync)so;

    GL_CALL_EXTRA_FUNCTION(glDeleteSync(syncID));
}

void QDemonRenderBackendGL3Impl::waitSync(QDemonRenderBackendSyncObject so, QDemonRenderCommandFlushFlags, quint64)
{
    GLsync syncID = (GLsync)so;

    GL_CALL_EXTRA_FUNCTION(glWaitSync(syncID, 0, GL_TIMEOUT_IGNORED));
}

QT_BEGIN_NAMESPACE
