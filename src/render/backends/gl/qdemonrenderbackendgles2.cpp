/****************************************************************************
**
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

#include <QtDemonRender/qdemonrenderbackendgles2.h>
#include <QtDemonRender/qdemonrenderbackendinputassemblergl.h>
#include <QtDemonRender/qdemonrenderbackendrenderstatesgl.h>
#include <QtDemonRender/qdemonrenderbackendshaderprogramgl.h>

QT_BEGIN_NAMESPACE

#ifdef RENDER_BACKEND_LOG_GL_ERRORS
#define RENDER_LOG_ERROR_PARAMS(x) checkGLError(#x, __FILE__, __LINE__)
#else
#define RENDER_LOG_ERROR_PARAMS(x) checkGLError()
#endif

#if defined(QT_OPENGL_ES) || defined(QT_OPENGL_ES_2_ANGLE)
#define GL_CALL_TIMER_EXT(x)                                                                                           \
    m_qdemonExtensions->x;                                                                                             \
    RENDER_LOG_ERROR_PARAMS(x);
#define GL_CALL_TESSELATION_EXT(x)                                                                                     \
    m_qdemonExtensions->x;                                                                                             \
    RENDER_LOG_ERROR_PARAMS(x);
#define GL_CALL_MULTISAMPLE_EXT(x)                                                                                     \
    m_qdemonExtensions->x;                                                                                             \
    RENDER_LOG_ERROR_PARAMS(x);
#define GL_CALL_EXTRA_FUNCTION(x)                                                                                      \
    m_glExtraFunctions->x;                                                                                             \
    RENDER_LOG_ERROR_PARAMS(x);
#define GL_CALL_EXTENSION_FUNCTION(x)                                                                                  \
    m_qdemonExtensions->x;                                                                                             \
    RENDER_LOG_ERROR_PARAMS(x);
#else
#define GL_CALL_TIMER_EXT(x)
#define GL_CALL_TESSELATION_EXT(x)
#define GL_CALL_MULTISAMPLE_EXT(x)
#define GL_CALL_EXTRA_FUNCTION(x)                                                                                      \
    m_glExtraFunctions->x;                                                                                             \
    RENDER_LOG_ERROR_PARAMS(x);
#define GL_CALL_EXTENSION_FUNCTION(x)
#endif

#ifndef GL_DEPTH_STENCIL_OES
#define GL_DEPTH_STENCIL_OES 0x84F9
#endif

QByteArray exts3tc()
{
    return QByteArrayLiteral("GL_EXT_texture_compression_s3tc");
}
QByteArray extsdxt()
{
    return QByteArrayLiteral("GL_EXT_texture_compression_dxt1");
}
QByteArray extsAniso()
{
    return QByteArrayLiteral("GL_EXT_texture_filter_anisotropic");
}
QByteArray extsTexSwizzle()
{
    return QByteArrayLiteral("GL_ARB_texture_swizzle");
}
QByteArray extsFPRenderTarget()
{
    return QByteArrayLiteral("GL_EXT_color_buffer_float");
}
QByteArray extsTimerQuery()
{
    return QByteArrayLiteral("GL_EXT_timer_query");
}
QByteArray extsGpuShader5()
{
    return QByteArrayLiteral("EXT_gpu_shader5");
}

QByteArray extDepthTexture()
{
    return QByteArrayLiteral("GL_OES_packed_depth_stencil");
}
QByteArray extvao()
{
    return QByteArrayLiteral("GL_OES_vertex_array_object");
}
QByteArray extStdDd()
{
    return QByteArrayLiteral("GL_OES_standard_derivatives");
}
QByteArray extTexLod()
{
    return QByteArrayLiteral("GL_EXT_shader_texture_lod");
}

/// constructor
QDemonRenderBackendGLES2Impl::QDemonRenderBackendGLES2Impl(const QSurfaceFormat &format)
    : QDemonRenderBackendGLBase(format)
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

    const char *extensions = getExtensionString();
    m_extensions = QByteArray(extensions).split(' ');

    // get extension count
    GLint numExtensions = m_extensions.size();

    for (qint32 i = 0; i < numExtensions; i++) {

        const QByteArray &extensionString = m_extensions.at(i);

        // search for extension
        if (!m_backendSupport.caps.bits.bDXTImagesSupported
            && (exts3tc().compare(extensionString) == 0 || extsdxt().compare(extensionString) == 0)) {
            m_backendSupport.caps.bits.bDXTImagesSupported = true;
        } else if (!m_backendSupport.caps.bits.bAnistropySupported && extsAniso().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bAnistropySupported = true;
        } else if (!m_backendSupport.caps.bits.bFPRenderTargetsSupported && extsFPRenderTarget().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bFPRenderTargetsSupported = true;
        } else if (!m_backendSupport.caps.bits.bTimerQuerySupported && extsTimerQuery().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bTimerQuerySupported = true;
        } else if (!m_backendSupport.caps.bits.bGPUShader5ExtensionSupported && extsGpuShader5().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bGPUShader5ExtensionSupported = true;
        } else if (!m_backendSupport.caps.bits.bTextureSwizzleSupported && extsTexSwizzle().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bTextureSwizzleSupported = true;
        } else if (!m_backendSupport.caps.bits.bDepthStencilSupported && extDepthTexture().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bDepthStencilSupported = true;
        } else if (!m_backendSupport.caps.bits.bVertexArrayObjectSupported && extvao().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bVertexArrayObjectSupported = true;
        } else if (!m_backendSupport.caps.bits.bStandardDerivativesSupported && extStdDd().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bStandardDerivativesSupported = true;
        } else if (!m_backendSupport.caps.bits.bTextureLodSupported && extTexLod().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bTextureLodSupported = true;
        }
    }

    qCInfo(TRACE_INFO, "OpenGL extensions: %s", extensions);

    // constant buffers support is always not true
    m_backendSupport.caps.bits.bConstantBufferSupported = false;

    // query hardware
    GL_CALL_EXTRA_FUNCTION(glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &m_maxAttribCount));

    // internal state tracker
    m_pCurrentMiscState = new QDemonRenderBackendMiscStateGL();

    // finally setup caps based on device
    setAndInspectHardwareCaps();

    // Initialize extensions
#if defined(QT_OPENGL_ES) || defined(QT_OPENGL_ES_2_ANGLE)
    m_qdemonExtensions = new QDemonOpenGLES2Extensions;
    m_qdemonExtensions->initializeOpenGLFunctions();
#endif
}
/// destructor
QDemonRenderBackendGLES2Impl::~QDemonRenderBackendGLES2Impl()
{
    delete m_pCurrentMiscState;
#if defined(QT_OPENGL_ES) || defined(QT_OPENGL_ES_2_ANGLE)
    delete m_qdemonExtensions;
#endif
}

void QDemonRenderBackendGLES2Impl::setMultisampledTextureData2D(QDemonRenderBackendTextureObject to,
                                                                QDemonRenderTextureTargetType target,
                                                                qint32 samples,
                                                                QDemonRenderTextureFormat internalFormat,
                                                                qint32 width,
                                                                qint32 height,
                                                                bool fixedsamplelocations)
{
    NVRENDER_BACKEND_UNUSED(to);
    NVRENDER_BACKEND_UNUSED(target);
    NVRENDER_BACKEND_UNUSED(samples);
    NVRENDER_BACKEND_UNUSED(internalFormat);
    NVRENDER_BACKEND_UNUSED(width);
    NVRENDER_BACKEND_UNUSED(height);
    NVRENDER_BACKEND_UNUSED(fixedsamplelocations);
}

void QDemonRenderBackendGLES2Impl::setTextureData3D(QDemonRenderBackendTextureObject to,
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

    if (internalFormat.isUncompressedTextureFormat()) {
        GLConversion::fromUncompressedTextureFormatToGL(getRenderContextType(), internalFormat, glformat, gltype, glInternalFormat);
    }

    if (conversionRequired) {
        GLenum dummy;
        GLConversion::fromUncompressedTextureFormatToGL(getRenderContextType(), format, glformat, gltype, dummy);
    } else if (internalFormat.isCompressedTextureFormat()) {
        GLConversion::fromUncompressedTextureFormatToGL(getRenderContextType(), format, glformat, gltype, glInternalFormat);
        glInternalFormat = GLConversion::fromCompressedTextureFormatToGL(internalFormat);
    } else if (format.isDepthTextureFormat()) {
        GLConversion::fromDepthTextureFormatToGL(getRenderContextType(), format, glformat, gltype, glInternalFormat);
    }

    GL_CALL_EXTRA_FUNCTION(
            glTexImage3D(glTarget, level, glInternalFormat, GLsizei(width), GLsizei(height), GLsizei(depth), border, glformat, gltype, hostPtr));

    GL_CALL_EXTRA_FUNCTION(glBindTexture(glTarget, 0));
}

void QDemonRenderBackendGLES2Impl::setTextureData2D(QDemonRenderBackendTextureObject to,
                                                    QDemonRenderTextureTargetType target,
                                                    qint32 level,
                                                    QDemonRenderTextureFormat internalFormat,
                                                    qint32 width,
                                                    qint32 height,
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

    if (internalFormat.isUncompressedTextureFormat()) {
        GLConversion::fromUncompressedTextureFormatToGL(getRenderContextType(), internalFormat, glformat, gltype, glInternalFormat);
        glInternalFormat = glformat;
    }

    if (conversionRequired) {
        GLenum dummy;
        GLConversion::fromUncompressedTextureFormatToGL(getRenderContextType(), format, glformat, gltype, dummy);
    } else if (internalFormat.isCompressedTextureFormat()) {
        GLConversion::fromUncompressedTextureFormatToGL(getRenderContextType(), format, glformat, gltype, glInternalFormat);
        glInternalFormat = GLConversion::fromCompressedTextureFormatToGL(internalFormat);
    } else if (format.isDepthTextureFormat()) {
        GLConversion::fromDepthTextureFormatToGL(getRenderContextType(), format, glformat, gltype, glInternalFormat);
        if (format == QDemonRenderTextureFormat::Depth24Stencil8) {
            glformat = GL_DEPTH_STENCIL_OES;
            gltype = GL_UNSIGNED_INT_24_8;
        }
        glInternalFormat = glformat;
    }

    Q_ASSERT(glformat == glInternalFormat);
    GL_CALL_EXTRA_FUNCTION(
            glTexImage2D(glTarget, level, glInternalFormat, GLsizei(width), GLsizei(height), border, glformat, gltype, hostPtr));
    GL_CALL_EXTRA_FUNCTION(glBindTexture(glTarget, 0));
}

void QDemonRenderBackendGLES2Impl::updateSampler(QDemonRenderBackendSamplerObject /* so */,
                                                 QDemonRenderTextureTargetType target,
                                                 QDemonRenderTextureMinifyingOp::Enum minFilter,
                                                 QDemonRenderTextureMagnifyingOp::Enum magFilter,
                                                 QDemonRenderTextureCoordOp::Enum wrapS,
                                                 QDemonRenderTextureCoordOp::Enum wrapT,
                                                 QDemonRenderTextureCoordOp::Enum wrapR,
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
    NVRENDER_BACKEND_UNUSED(lodBias);
    NVRENDER_BACKEND_UNUSED(borderColor);
    NVRENDER_BACKEND_UNUSED(wrapR);
    NVRENDER_BACKEND_UNUSED(minLod);
    NVRENDER_BACKEND_UNUSED(maxLod);
    NVRENDER_BACKEND_UNUSED(compareMode);
    NVRENDER_BACKEND_UNUSED(compareFunc);

    GLenum glTarget = GLConversion::fromTextureTargetToGL(target);

    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_MIN_FILTER, m_conversion.fromTextureMinifyingOpToGL(minFilter)));
    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_MAG_FILTER, m_conversion.fromTextureMagnifyingOpToGL(magFilter)));
    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_WRAP_S, m_conversion.fromTextureCoordOpToGL(wrapS)));
    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_WRAP_T, m_conversion.fromTextureCoordOpToGL(wrapT)));

    if (m_backendSupport.caps.bits.bAnistropySupported) {
        GL_CALL_EXTRA_FUNCTION(glTexParameterf(glTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy));
    }
}

void QDemonRenderBackendGLES2Impl::updateTextureObject(QDemonRenderBackendTextureObject to,
                                                       QDemonRenderTextureTargetType target,
                                                       qint32 baseLevel,
                                                       qint32 maxLevel)
{
    NVRENDER_BACKEND_UNUSED(to);

    GLenum glTarget = GLConversion::fromTextureTargetToGL(target);

    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_BASE_LEVEL, baseLevel));
    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_MAX_LEVEL, maxLevel));
}

void QDemonRenderBackendGLES2Impl::updateTextureSwizzle(QDemonRenderBackendTextureObject to,
                                                        QDemonRenderTextureTargetType target,
                                                        QDemonRenderTextureSwizzleMode swizzleMode)
{
    NVRENDER_BACKEND_UNUSED(to);
    NVRENDER_BACKEND_UNUSED(target);
    NVRENDER_BACKEND_UNUSED(swizzleMode);
#if defined(QT_OPENGL_ES)
    if (m_backendSupport.caps.bits.bTextureSwizzleSupported) {
        GLint glSwizzle[4];
        GLenum glTarget = m_conversion.fromTextureTargetToGL(target);
        m_conversion.NVRenderConvertSwizzleModeToGL(swizzleMode, glSwizzle);

        // since ES3 spec has no GL_TEXTURE_SWIZZLE_RGBA set it separately
        GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_SWIZZLE_R, glSwizzle[0]));
        GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_SWIZZLE_G, glSwizzle[1]));
        GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_SWIZZLE_B, glSwizzle[2]));
        GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_SWIZZLE_A, glSwizzle[3]));
    }
#endif
}

qint32 QDemonRenderBackendGLES2Impl::getDepthBits() const
{
    qint32 depthBits;
    GL_CALL_EXTRA_FUNCTION(
            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depthBits));

    return depthBits;
}

qint32 QDemonRenderBackendGLES2Impl::getStencilBits() const
{
    qint32 stencilBits;
    GL_CALL_EXTRA_FUNCTION(
            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &stencilBits));

    return stencilBits;
}

void QDemonRenderBackendGLES2Impl::generateMipMaps(QDemonRenderBackendTextureObject to,
                                                   QDemonRenderTextureTargetType target,
                                                   QDemonRenderHint::Enum /*genType*/)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GLenum glTarget = GLConversion::fromTextureTargetToGL(target);
    GL_CALL_EXTRA_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_EXTRA_FUNCTION(glBindTexture(glTarget, texID));
    GL_CALL_EXTRA_FUNCTION(glGenerateMipmap(glTarget));
    GL_CALL_EXTRA_FUNCTION(glBindTexture(glTarget, 0));
}

bool QDemonRenderBackendGLES2Impl::setInputAssembler(QDemonRenderBackendInputAssemblerObject iao, QDemonRenderBackendShaderProgramObject po)
{
    if (iao == nullptr) {
        // unbind and return;
        GL_CALL_EXTENSION_FUNCTION(glBindVertexArrayOES(0));
        return true;
    }

    QDemonRenderBackendInputAssemblerGL *inputAssembler = (QDemonRenderBackendInputAssemblerGL *)iao;
    QDemonRenderBackendAttributeLayoutGL *attribLayout = inputAssembler->m_attribLayout;
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_programID);
    QDemonDataRef<QDemonRenderBackendShaderInputEntryGL> shaderAttribBuffer;
    if (pProgram->m_shaderInput)
        shaderAttribBuffer = pProgram->m_shaderInput->m_shaderInputEntries;

    if ((attribLayout->m_layoutAttribEntries.size() < shaderAttribBuffer.size())
        || (inputAssembler->m_vertexbufferHandles.size() <= attribLayout->m_maxInputSlot)) {
        return false;
    }

    if (inputAssembler->m_vaoID == 0) {
        // generate vao
        GL_CALL_EXTENSION_FUNCTION(glGenVertexArraysOES(1, &inputAssembler->m_vaoID));
        Q_ASSERT(inputAssembler->m_vaoID);
    }

    if (inputAssembler->m_cachedShaderHandle != programID) {
        GL_CALL_EXTENSION_FUNCTION(glBindVertexArrayOES(inputAssembler->m_vaoID));
        inputAssembler->m_cachedShaderHandle = programID;

        QDEMON_FOREACH(idx, shaderAttribBuffer.size())
        {
            const QDemonRenderBackendShaderInputEntryGL &attrib(shaderAttribBuffer[idx]);
            QDemonRenderBackendLayoutEntryGL *entry = attribLayout->getEntryByName(attrib.m_attribName);

            if (entry) {
                QDemonRenderBackendLayoutEntryGL &entryData(*entry);
                if (entryData.m_type != attrib.m_type || entryData.m_numComponents != attrib.m_numComponents) {
                    qCCritical(INVALID_OPERATION, "Attrib %s dn't match vertex layout", qPrintable(attrib.m_attribName));
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
        for (quint32 i = 0; i < attribLayout->m_layoutAttribEntries.size(); i++)
            GL_CALL_EXTRA_FUNCTION(glDisableVertexAttribArray(i));

        // setup all attribs
        QDEMON_FOREACH(idx, shaderAttribBuffer.size())
        {
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
        GL_CALL_EXTENSION_FUNCTION(glBindVertexArrayOES(inputAssembler->m_vaoID));
    }
#ifdef _DEBUG
    if (inputAssembler->m_vaoID) {
        QDEMON_FOREACH(idx, shaderAttribBuffer.size())
        {
            const QDemonRenderBackendShaderInputEntryGL &attrib(shaderAttribBuffer[idx]);
            QDemonRenderBackendLayoutEntryGL *entry = attribLayout->getEntryByName(attrib.m_attribName);

            if (entry) {
                QDemonRenderBackendLayoutEntryGL &entryData(*entry);
                if (entryData.m_type != attrib.m_type || entryData.m_numComponents != attrib.m_numComponents
                    || entryData.m_attribIndex != attrib.m_attribLocation) {
                    qCCritical(INVALID_OPERATION, "Attrib %s dn't match vertex layout", qPrintable(attrib.m_attribName));
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

void QDemonRenderBackendGLES2Impl::setDrawBuffers(QDemonRenderBackendRenderTargetObject rto, QDemonConstDataRef<qint32> inDrawBufferSet)
{
    NVRENDER_BACKEND_UNUSED(rto);

    m_drawBuffersArray.clear();

    for (quint32 idx = 0, end = inDrawBufferSet.size(); idx < end; ++idx) {
        if (inDrawBufferSet[idx] < 0)
            m_drawBuffersArray.push_back(GL_NONE);
        else
            m_drawBuffersArray.push_back(GL_COLOR_ATTACHMENT0 + inDrawBufferSet[idx]);
    }

    GL_CALL_EXTRA_FUNCTION(glDrawBuffers((int)m_drawBuffersArray.size(), m_drawBuffersArray.data()));
}

void QDemonRenderBackendGLES2Impl::setReadBuffer(QDemonRenderBackendRenderTargetObject rto, QDemonReadFace inReadFace)
{
    NVRENDER_BACKEND_UNUSED(rto);
    NVRENDER_BACKEND_UNUSED(inReadFace);
}

void QDemonRenderBackendGLES2Impl::renderTargetAttach(QDemonRenderBackendRenderTargetObject,
                                                      QDemonRenderFrameBufferAttachment attachment,
                                                      QDemonRenderBackendTextureObject to,
                                                      qint32 level,
                                                      qint32 layer)
{
    NVRENDER_BACKEND_UNUSED(attachment);
    NVRENDER_BACKEND_UNUSED(to);
    NVRENDER_BACKEND_UNUSED(level);
    NVRENDER_BACKEND_UNUSED(layer);
    Q_ASSERT(false);
}

void QDemonRenderBackendGLES2Impl::blitFramebuffer(qint32 srcX0,
                                                   qint32 srcY0,
                                                   qint32 srcX1,
                                                   qint32 srcY1,
                                                   qint32 dstX0,
                                                   qint32 dstY0,
                                                   qint32 dstX1,
                                                   qint32 dstY1,
                                                   QDemonRenderClearFlags flags,
                                                   QDemonRenderTextureMagnifyingOp::Enum filter)
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

QDemonRenderBackend::QDemonRenderBackendRenderTargetObject QDemonRenderBackendGLES2Impl::createRenderTarget()
{
    GLuint fboID = 0;
    GL_CALL_EXTRA_FUNCTION(glGenFramebuffers(1, &fboID));
    return (QDemonRenderBackend::QDemonRenderBackendRenderTargetObject)fboID;
}

void QDemonRenderBackendGLES2Impl::releaseRenderTarget(QDemonRenderBackendRenderTargetObject rto)
{
    GLuint fboID = HandleToID_cast(GLuint, size_t, rto);

    if (fboID)
        GL_CALL_EXTRA_FUNCTION(glDeleteFramebuffers(1, &fboID));
}

void QDemonRenderBackendGLES2Impl::renderTargetAttach(QDemonRenderBackendRenderTargetObject /* rto */,
                                                      QDemonRenderFrameBufferAttachment attachment,
                                                      QDemonRenderBackendRenderbufferObject rbo)
{
    // rto must be the current render target
    GLuint rbID = HandleToID_cast(GLuint, size_t, rbo);

    GLenum glAttach = GLConversion::fromFramebufferAttachmentsToGL(attachment);

    GL_CALL_EXTRA_FUNCTION(glFramebufferRenderbuffer(GL_FRAMEBUFFER, glAttach, GL_RENDERBUFFER, rbID));
}

void QDemonRenderBackendGLES2Impl::renderTargetAttach(QDemonRenderBackendRenderTargetObject /* rto */,
                                                      QDemonRenderFrameBufferAttachment attachment,
                                                      QDemonRenderBackendTextureObject to,
                                                      QDemonRenderTextureTargetType target)
{
    // rto must be the current render target
    GLuint texID = HandleToID_cast(GLuint, size_t, to);

    Q_ASSERT(target == QDemonRenderTextureTargetType::Texture2D || m_backendSupport.caps.bits.bMsTextureSupported);

    GLenum glAttach = GLConversion::fromFramebufferAttachmentsToGL(attachment);
    GLenum glTarget = GLConversion::fromTextureTargetToGL(target);

    if (attachment == QDemonRenderFrameBufferAttachment::DepthStencil) {
        GL_CALL_EXTRA_FUNCTION(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, glTarget, texID, 0));
        GL_CALL_EXTRA_FUNCTION(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, glTarget, texID, 0));
    } else {
        GL_CALL_EXTRA_FUNCTION(glFramebufferTexture2D(GL_FRAMEBUFFER, glAttach, glTarget, texID, 0));
    }
}

void QDemonRenderBackendGLES2Impl::setRenderTarget(QDemonRenderBackendRenderTargetObject rto)
{
    GLuint fboID = HandleToID_cast(GLuint, size_t, rto);

    GL_CALL_EXTRA_FUNCTION(glBindFramebuffer(GL_FRAMEBUFFER, fboID));
}

void QDemonRenderBackendGLES2Impl::setReadTarget(QDemonRenderBackendRenderTargetObject rto)
{
    GLuint fboID = HandleToID_cast(GLuint, size_t, rto);

    GL_CALL_EXTRA_FUNCTION(glBindFramebuffer(GL_READ_FRAMEBUFFER, fboID));
}

bool QDemonRenderBackendGLES2Impl::renderTargetIsValid(QDemonRenderBackendRenderTargetObject /* rto */)
{
    GLenum completeStatus = GL_CALL_EXTRA_FUNCTION(glCheckFramebufferStatus(GL_FRAMEBUFFER));
    switch (completeStatus) {
#define HANDLE_INCOMPLETE_STATUS(x)                                                                                    \
    case x:                                                                                                            \
        qCCritical(INTERNAL_ERROR, "Framebuffer is not complete: %s", #x);                                             \
        return false;
        HANDLE_INCOMPLETE_STATUS(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
        HANDLE_INCOMPLETE_STATUS(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS)
        HANDLE_INCOMPLETE_STATUS(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
        HANDLE_INCOMPLETE_STATUS(GL_FRAMEBUFFER_UNSUPPORTED)
#undef HANDLE_INCOMPLETE_STATUS
    }
    return true;
}

QDemonRenderBackend::QDemonRenderBackendRenderbufferObject QDemonRenderBackendGLES2Impl::createRenderbuffer(QDemonRenderRenderBufferFormat storageFormat,
                                                                                                            qint32 width,
                                                                                                            qint32 height)
{
    GLuint bufID = 0;

    GL_CALL_EXTRA_FUNCTION(glGenRenderbuffers(1, &bufID));
    GL_CALL_EXTRA_FUNCTION(glBindRenderbuffer(GL_RENDERBUFFER, bufID));
    GL_CALL_EXTRA_FUNCTION(glRenderbufferStorage(GL_RENDERBUFFER,
                                                 GLConversion::fromRenderBufferFormatsToRenderBufferGL(storageFormat),
                                                 (GLsizei)width,
                                                 (GLsizei)height));

    // check for error
    GLenum error = m_glFunctions->glGetError();
    if (error != GL_NO_ERROR) {
        qCCritical(GL_ERROR, "%s", GLConversion::processGLError(error));
        Q_ASSERT(false);
        GL_CALL_EXTRA_FUNCTION(glDeleteRenderbuffers(1, &bufID));
        bufID = 0;
    }

    GL_CALL_EXTRA_FUNCTION(glBindRenderbuffer(GL_RENDERBUFFER, 0));

    return (QDemonRenderBackend::QDemonRenderBackendRenderbufferObject)bufID;
}

void QDemonRenderBackendGLES2Impl::releaseRenderbuffer(QDemonRenderBackendRenderbufferObject rbo)
{
    GLuint bufID = HandleToID_cast(GLuint, size_t, rbo);

    if (bufID)
        GL_CALL_EXTRA_FUNCTION(glDeleteRenderbuffers(1, &bufID));
}

bool QDemonRenderBackendGLES2Impl::resizeRenderbuffer(QDemonRenderBackendRenderbufferObject rbo,
                                                      QDemonRenderRenderBufferFormat storageFormat,
                                                      qint32 width,
                                                      qint32 height)
{
    bool success = true;
    GLuint bufID = HandleToID_cast(GLuint, size_t, rbo);

    Q_ASSERT(bufID);

    GL_CALL_EXTRA_FUNCTION(glBindRenderbuffer(GL_RENDERBUFFER, bufID));
    GL_CALL_EXTRA_FUNCTION(glRenderbufferStorage(GL_RENDERBUFFER,
                                                 GLConversion::fromRenderBufferFormatsToRenderBufferGL(storageFormat),
                                                 (GLsizei)width,
                                                 (GLsizei)height));

    // check for error
    GLenum error = m_glFunctions->glGetError();
    if (error != GL_NO_ERROR) {
        qCCritical(GL_ERROR, "%s", GLConversion::processGLError(error));
        Q_ASSERT(false);
        success = false;
    }

    return success;
}

void *QDemonRenderBackendGLES2Impl::mapBuffer(QDemonRenderBackendBufferObject,
                                              QDemonRenderBufferBindFlags bindFlags,
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

bool QDemonRenderBackendGLES2Impl::unmapBuffer(QDemonRenderBackendBufferObject, QDemonRenderBufferBindFlags bindFlags)
{
    GLboolean ret;

    ret = GL_CALL_EXTRA_FUNCTION(glUnmapBuffer(m_conversion.fromBindBufferFlagsToGL(bindFlags)));

    return (ret) ? true : false;
}

qint32 QDemonRenderBackendGLES2Impl::getConstantBufferCount(QDemonRenderBackendShaderProgramObject po)
{
    Q_ASSERT(po);
    GLint numUniformBuffers = 0;
    if (getRenderBackendCap(QDemonRenderBackendCaps::ConstantBuffer)) {
        QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
        GLuint programID = static_cast<GLuint>(pProgram->m_programID);

        GL_CALL_EXTRA_FUNCTION(glGetProgramiv(programID, GL_ACTIVE_UNIFORM_BLOCKS, &numUniformBuffers));
    }
    return numUniformBuffers;
}

qint32 QDemonRenderBackendGLES2Impl::getConstantBufferInfoByID(QDemonRenderBackendShaderProgramObject po,
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

void QDemonRenderBackendGLES2Impl::getConstantBufferParamIndices(QDemonRenderBackendShaderProgramObject po, quint32 id, qint32 *indices)
{
    Q_ASSERT(po);
    Q_ASSERT(indices);

    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_programID);

    if (indices) {
        GL_CALL_EXTRA_FUNCTION(glGetActiveUniformBlockiv(programID, id, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices));
    }
}

void QDemonRenderBackendGLES2Impl::getConstantBufferParamInfoByIndices(QDemonRenderBackendShaderProgramObject po,
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
            QDEMON_FOREACH(idx, count) { type[idx] = GLConversion::fromShaderGLToPropertyDataTypes(glTypes[idx]); }
        }
        if (size) {
            GL_CALL_EXTRA_FUNCTION(glGetActiveUniformsiv(programID, count, indices, GL_UNIFORM_SIZE, size));
        }
        if (offset) {
            GL_CALL_EXTRA_FUNCTION(glGetActiveUniformsiv(programID, count, indices, GL_UNIFORM_OFFSET, offset));
        }
    }
}

void QDemonRenderBackendGLES2Impl::programSetConstantBlock(QDemonRenderBackendShaderProgramObject po, quint32 blockIndex, quint32 binding)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_programID);

    GL_CALL_EXTRA_FUNCTION(glUniformBlockBinding(programID, blockIndex, binding));
}

void QDemonRenderBackendGLES2Impl::programSetConstantBuffer(quint32 index, QDemonRenderBackendBufferObject bo)
{
    Q_ASSERT(bo);

    GLuint bufID = HandleToID_cast(GLuint, size_t, bo);
    GL_CALL_EXTRA_FUNCTION(glBindBufferBase(GL_UNIFORM_BUFFER, index, bufID));
}

QDemonRenderBackend::QDemonRenderBackendQueryObject QDemonRenderBackendGLES2Impl::createQuery()
{
    quint32 glQueryID = 0;

    return (QDemonRenderBackendQueryObject)glQueryID;
}

void QDemonRenderBackendGLES2Impl::releaseQuery(QDemonRenderBackendQueryObject) {}

void QDemonRenderBackendGLES2Impl::beginQuery(QDemonRenderBackendQueryObject, QDemonRenderQueryType) {}

void QDemonRenderBackendGLES2Impl::endQuery(QDemonRenderBackendQueryObject, QDemonRenderQueryType) {}

void QDemonRenderBackendGLES2Impl::getQueryResult(QDemonRenderBackendQueryObject, QDemonRenderQueryResultType, quint32 *)
{
}

void QDemonRenderBackendGLES2Impl::getQueryResult(QDemonRenderBackendQueryObject, QDemonRenderQueryResultType, quint64 *)
{
}

void QDemonRenderBackendGLES2Impl::setQueryTimer(QDemonRenderBackendQueryObject) {}

QDemonRenderBackend::QDemonRenderBackendSyncObject QDemonRenderBackendGLES2Impl::createSync(QDemonRenderSyncType, QDemonRenderSyncFlags)
{
    GLsync syncID = nullptr;
    return QDemonRenderBackendSyncObject(syncID);
}

void QDemonRenderBackendGLES2Impl::releaseSync(QDemonRenderBackendSyncObject) {}

void QDemonRenderBackendGLES2Impl::waitSync(QDemonRenderBackendSyncObject, QDemonRenderCommandFlushFlags, quint64) {}

QT_END_NAMESPACE
