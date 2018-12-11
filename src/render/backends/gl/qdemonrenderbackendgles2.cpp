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
#define GL_CALL_TIMER_EXT(x) m_qdemonExtensions->x; RENDER_LOG_ERROR_PARAMS(x);
#define GL_CALL_TESSELATION_EXT(x) m_qdemonExtensions->x; RENDER_LOG_ERROR_PARAMS(x);
#define GL_CALL_MULTISAMPLE_EXT(x) m_qdemonExtensions->x; RENDER_LOG_ERROR_PARAMS(x);
#define GL_CALL_EXTRA_FUNCTION(x) m_glExtraFunctions->x; RENDER_LOG_ERROR_PARAMS(x);
#define GL_CALL_EXTENSION_FUNCTION(x) m_qdemonExtensions->x; RENDER_LOG_ERROR_PARAMS(x);
#else
#define GL_CALL_TIMER_EXT(x)
#define GL_CALL_TESSELATION_EXT(x)
#define GL_CALL_MULTISAMPLE_EXT(x)
#define GL_CALL_EXTRA_FUNCTION(x)  m_glExtraFunctions->x; RENDER_LOG_ERROR_PARAMS(x);
#define GL_CALL_EXTENSION_FUNCTION(x)
#endif

#ifndef GL_DEPTH_STENCIL_OES
#define GL_DEPTH_STENCIL_OES 0x84F9
#endif

/// constructor
QDemonRenderBackendGLES2Impl::QDemonRenderBackendGLES2Impl(const QSurfaceFormat &format)
    : QDemonRenderBackendGLBase(format)
{
    QString exts3tc = QStringLiteral("GL_EXT_texture_compression_s3tc");
    QString extsdxt = QStringLiteral("GL_EXT_texture_compression_dxt1");
    QString extsAniso = QStringLiteral("GL_EXT_texture_filter_anisotropic");
    QString extsTexSwizzle = QStringLiteral("GL_ARB_texture_swizzle");
    QString extsFPRenderTarget = QStringLiteral("GL_EXT_color_buffer_float");
    QString extsTimerQuery = QStringLiteral("GL_EXT_timer_query");
    QString extsGpuShader5 = QStringLiteral("EXT_gpu_shader5");
    QString extDepthTexture = QStringLiteral("GL_OES_packed_depth_stencil");
    QString extvao = QStringLiteral("GL_OES_vertex_array_object");
    QString extStdDd = QStringLiteral("GL_OES_standard_derivatives");
    QString extTexLod = QStringLiteral("GL_EXT_shader_texture_lod");

    const char *languageVersion = GetShadingLanguageVersion();
    qCInfo(TRACE_INFO, "GLSL version: %s", languageVersion);

    QString apiVersion(getVersionString());
    qCInfo(TRACE_INFO, "GL version: %s", qPrintable(apiVersion));

    QString apiVendor(getVendorString());
    qCInfo(TRACE_INFO, "HW vendor: %s", qPrintable(apiVendor));

    QString apiRenderer(getRendererString());
    qCInfo(TRACE_INFO, "Vendor renderer: %s", qPrintable(apiRenderer));

    // clear support bits
    m_backendSupport.caps.u32Values = 0;

    const char *extensions = getExtensionString();
    m_extensions = QString::fromLocal8Bit(extensions).split(" ");

    // get extension count
    GLint numExtensions = m_extensions.size();

    for (qint32 i = 0; i < numExtensions; i++) {

        const QString &extensionString = m_extensions.at(i);

        // search for extension
        if (!m_backendSupport.caps.bits.bDXTImagesSupported
            && (exts3tc.compare(extensionString) == 0 || extsdxt.compare(extensionString) == 0)) {
            m_backendSupport.caps.bits.bDXTImagesSupported = true;
        } else if (!m_backendSupport.caps.bits.bAnistropySupported
                   && extsAniso.compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bAnistropySupported = true;
        } else if (!m_backendSupport.caps.bits.bFPRenderTargetsSupported
                   && extsFPRenderTarget.compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bFPRenderTargetsSupported = true;
        } else if (!m_backendSupport.caps.bits.bTimerQuerySupported
                   && extsTimerQuery.compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bTimerQuerySupported = true;
        } else if (!m_backendSupport.caps.bits.bGPUShader5ExtensionSupported
                   && extsGpuShader5.compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bGPUShader5ExtensionSupported = true;
        } else if (!m_backendSupport.caps.bits.bTextureSwizzleSupported
                   && extsTexSwizzle.compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bTextureSwizzleSupported = true;
        } else if (!m_backendSupport.caps.bits.bDepthStencilSupported
                   && extDepthTexture.compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bDepthStencilSupported = true;
        } else if (!m_backendSupport.caps.bits.bVertexArrayObjectSupported
                   && extvao.compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bVertexArrayObjectSupported = true;
        } else if (!m_backendSupport.caps.bits.bStandardDerivativesSupported
                   && extStdDd.compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bStandardDerivativesSupported = true;
        } else if (!m_backendSupport.caps.bits.bTextureLodSupported
                   && extTexLod.compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bTextureLodSupported = true;
        }
    }

    qCInfo(TRACE_INFO, "OpenGL extensions: %s", extensions);

    // constant buffers support is always not true
    m_backendSupport.caps.bits.bConstantBufferSupported = false;

    // query hardware
    GL_CALL_EXTRA_FUNCTION(glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &m_MaxAttribCount));

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
    if (m_pCurrentMiscState)
        delete m_pCurrentMiscState;
#if defined(QT_OPENGL_ES) || defined(QT_OPENGL_ES_2_ANGLE)
    if (m_qdemonExtensions)
        delete m_qdemonExtensions;
#endif
}

void QDemonRenderBackendGLES2Impl::SetMultisampledTextureData2D(
        QDemonRenderBackendTextureObject to, QDemonRenderTextureTargetType::Enum target, size_t samples,
        QDemonRenderTextureFormats::Enum internalFormat, size_t width, size_t height,
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

void QDemonRenderBackendGLES2Impl::SetTextureData3D(
        QDemonRenderBackendTextureObject to, QDemonRenderTextureTargetType::Enum target, quint32 level,
        QDemonRenderTextureFormats::Enum internalFormat, size_t width, size_t height, size_t depth,
        qint32 border, QDemonRenderTextureFormats::Enum format, const void *hostPtr)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GLenum glTarget = m_Conversion.fromTextureTargetToGL(target);
    GL_CALL_EXTRA_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_EXTRA_FUNCTION(glBindTexture(glTarget, texID));
    bool conversionRequired = format != internalFormat;

    QDemonRenderTextureSwizzleMode::Enum swizzleMode = QDemonRenderTextureSwizzleMode::NoSwizzle;
    internalFormat = m_Conversion.replaceDeprecatedTextureFormat(GetRenderContextType(),
                                                                 internalFormat, swizzleMode);

    GLenum glformat = 0, glInternalFormat = 0, gltype = GL_UNSIGNED_BYTE;

    if (QDemonRenderTextureFormats::isUncompressedTextureFormat(internalFormat)) {
        m_Conversion.fromUncompressedTextureFormatToGL(GetRenderContextType(), internalFormat,
                                                       glformat, gltype, glInternalFormat);
    }

    if (conversionRequired) {
        GLenum dummy;
        m_Conversion.fromUncompressedTextureFormatToGL(GetRenderContextType(), format, glformat,
                                                       gltype, dummy);
    } else if (QDemonRenderTextureFormats::isCompressedTextureFormat(internalFormat)) {
        m_Conversion.fromUncompressedTextureFormatToGL(GetRenderContextType(), format, glformat,
                                                       gltype, glInternalFormat);
        glInternalFormat = m_Conversion.fromCompressedTextureFormatToGL(internalFormat);
    } else if (QDemonRenderTextureFormats::isDepthTextureFormat(format)) {
        m_Conversion.fromDepthTextureFormatToGL(GetRenderContextType(), format, glformat,
                                                gltype, glInternalFormat);
    }

    GL_CALL_EXTRA_FUNCTION(glTexImage3D(glTarget, level, glInternalFormat, (GLsizei)width,
                                        (GLsizei)height, (GLsizei)depth, border, glformat,
                                        gltype, hostPtr));

    GL_CALL_EXTRA_FUNCTION(glBindTexture(glTarget, 0));
}

void QDemonRenderBackendGLES2Impl::SetTextureData2D(
        QDemonRenderBackendTextureObject to, QDemonRenderTextureTargetType::Enum target, quint32 level,
        QDemonRenderTextureFormats::Enum internalFormat, size_t width, size_t height, qint32 border,
        QDemonRenderTextureFormats::Enum format, const void *hostPtr)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GLenum glTarget = m_Conversion.fromTextureTargetToGL(target);
    GL_CALL_EXTRA_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_EXTRA_FUNCTION(glBindTexture(glTarget, texID));
    bool conversionRequired = format != internalFormat;

    QDemonRenderTextureSwizzleMode::Enum swizzleMode = QDemonRenderTextureSwizzleMode::NoSwizzle;
    internalFormat = m_Conversion.replaceDeprecatedTextureFormat(GetRenderContextType(),
                                                                 internalFormat, swizzleMode);

    GLenum glformat = 0, glInternalFormat = 0, gltype = GL_UNSIGNED_BYTE;

    if (QDemonRenderTextureFormats::isUncompressedTextureFormat(internalFormat)) {
        m_Conversion.fromUncompressedTextureFormatToGL(GetRenderContextType(), internalFormat,
                                                       glformat, gltype, glInternalFormat);
        glInternalFormat = glformat;
    }

    if (conversionRequired) {
        GLenum dummy;
        m_Conversion.fromUncompressedTextureFormatToGL(GetRenderContextType(), format, glformat,
                                                       gltype, dummy);
    } else if (QDemonRenderTextureFormats::isCompressedTextureFormat(internalFormat)) {
        m_Conversion.fromUncompressedTextureFormatToGL(GetRenderContextType(), format, glformat,
                                                       gltype, glInternalFormat);
        glInternalFormat = m_Conversion.fromCompressedTextureFormatToGL(internalFormat);
    } else if (QDemonRenderTextureFormats::isDepthTextureFormat(format)) {
        m_Conversion.fromDepthTextureFormatToGL(GetRenderContextType(), format, glformat,
                                                gltype, glInternalFormat);
        if (format == QDemonRenderTextureFormats::Depth24Stencil8) {
            glformat = GL_DEPTH_STENCIL_OES;
            gltype = GL_UNSIGNED_INT_24_8;
        }
        glInternalFormat = glformat;
    }

    Q_ASSERT(glformat == glInternalFormat);
    GL_CALL_EXTRA_FUNCTION(glTexImage2D(glTarget, level, glInternalFormat, (GLsizei)width,
                                        (GLsizei)height, border, glformat, gltype, hostPtr));
    GL_CALL_EXTRA_FUNCTION(glBindTexture(glTarget, 0));
}

void QDemonRenderBackendGLES2Impl::UpdateSampler(
        QDemonRenderBackendSamplerObject /* so */, QDemonRenderTextureTargetType::Enum target,
        QDemonRenderTextureMinifyingOp::Enum minFilter, QDemonRenderTextureMagnifyingOp::Enum magFilter,
        QDemonRenderTextureCoordOp::Enum wrapS, QDemonRenderTextureCoordOp::Enum wrapT,
        QDemonRenderTextureCoordOp::Enum wrapR, float minLod, float maxLod, float lodBias,
        QDemonRenderTextureCompareMode::Enum compareMode, QDemonRenderTextureCompareOp::Enum compareFunc,
        float anisotropy, float *borderColor)
{

    // Satisfy the compiler
    // These are not available in GLES 3 and we don't use them right now
    Q_ASSERT(lodBias == 0.0);
    Q_ASSERT(!borderColor);
    NVRENDER_BACKEND_UNUSED(lodBias);
    NVRENDER_BACKEND_UNUSED(borderColor);
    NVRENDER_BACKEND_UNUSED(wrapR);
    NVRENDER_BACKEND_UNUSED(minLod);
    NVRENDER_BACKEND_UNUSED(maxLod);
    NVRENDER_BACKEND_UNUSED(compareMode);
    NVRENDER_BACKEND_UNUSED(compareFunc);

    GLenum glTarget = m_Conversion.fromTextureTargetToGL(target);

    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_MIN_FILTER,
                                           m_Conversion.fromTextureMinifyingOpToGL(minFilter)));
    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_MAG_FILTER,
                                           m_Conversion.fromTextureMagnifyingOpToGL(magFilter)));
    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_WRAP_S,
                                           m_Conversion.fromTextureCoordOpToGL(wrapS)));
    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_WRAP_T,
                                           m_Conversion.fromTextureCoordOpToGL(wrapT)));

    if (m_backendSupport.caps.bits.bAnistropySupported) {
        GL_CALL_EXTRA_FUNCTION(glTexParameterf(glTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                                               anisotropy));
    }
}

void QDemonRenderBackendGLES2Impl::UpdateTextureObject(QDemonRenderBackendTextureObject to,
                                                   QDemonRenderTextureTargetType::Enum target,
                                                   qint32 baseLevel, qint32 maxLevel)
{
    NVRENDER_BACKEND_UNUSED(to);

    GLenum glTarget = m_Conversion.fromTextureTargetToGL(target);

    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_BASE_LEVEL, baseLevel));
    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_MAX_LEVEL, maxLevel));
}

void QDemonRenderBackendGLES2Impl::UpdateTextureSwizzle(QDemonRenderBackendTextureObject to,
                                                    QDemonRenderTextureTargetType::Enum target,
                                                    QDemonRenderTextureSwizzleMode::Enum swizzleMode)
{
    NVRENDER_BACKEND_UNUSED(to);
    NVRENDER_BACKEND_UNUSED(target);
    NVRENDER_BACKEND_UNUSED(swizzleMode);
#if defined(QT_OPENGL_ES)
    if (m_backendSupport.caps.bits.bTextureSwizzleSupported) {
        GLint glSwizzle[4];
        GLenum glTarget = m_Conversion.fromTextureTargetToGL(target);
        m_Conversion.NVRenderConvertSwizzleModeToGL(swizzleMode, glSwizzle);

        // since ES3 spec has no GL_TEXTURE_SWIZZLE_RGBA set it separately
        GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_SWIZZLE_R, glSwizzle[0]));
        GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_SWIZZLE_G, glSwizzle[1]));
        GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_SWIZZLE_B, glSwizzle[2]));
        GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_SWIZZLE_A, glSwizzle[3]));

    }
#endif
}

quint32
QDemonRenderBackendGLES2Impl::GetDepthBits() const
{
    qint32 depthBits;
    GL_CALL_EXTRA_FUNCTION(
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                              GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depthBits));

    return depthBits;
}

quint32
QDemonRenderBackendGLES2Impl::GetStencilBits() const
{
    qint32 stencilBits;
    GL_CALL_EXTRA_FUNCTION(
                glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                                      GL_DEPTH_ATTACHMENT,
                                                      GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE,
                                                      &stencilBits));

    return stencilBits;
}

void QDemonRenderBackendGLES2Impl::GenerateMipMaps(QDemonRenderBackendTextureObject to,
                                               QDemonRenderTextureTargetType::Enum target,
                                               QDemonRenderHint::Enum /*genType*/)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GLenum glTarget = m_Conversion.fromTextureTargetToGL(target);
    GL_CALL_EXTRA_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_EXTRA_FUNCTION(glBindTexture(glTarget, texID));
    GL_CALL_EXTRA_FUNCTION(glGenerateMipmap(glTarget));
    GL_CALL_EXTRA_FUNCTION(glBindTexture(glTarget, 0));
}

bool QDemonRenderBackendGLES2Impl::SetInputAssembler(QDemonRenderBackendInputAssemblerObject iao,
                                                 QDemonRenderBackendShaderProgramObject po)
{
    if (iao == nullptr) {
        // unbind and return;
        GL_CALL_EXTENSION_FUNCTION(glBindVertexArrayOES(0));
        return true;
    }

    QDemonRenderBackendInputAssemblerGL *inputAssembler = (QDemonRenderBackendInputAssemblerGL *)iao;
    QDemonRenderBackendAttributeLayoutGL *attribLayout = inputAssembler->m_attribLayout;
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_ProgramID);
    QDemonDataRef<QDemonRenderBackendShaderInputEntryGL> shaderAttribBuffer;
    if (pProgram->m_shaderInput)
        shaderAttribBuffer = pProgram->m_shaderInput->m_ShaderInputEntries;

    if ((attribLayout->m_LayoutAttribEntries.size() < shaderAttribBuffer.size())
            || (inputAssembler->m_VertexbufferHandles.size() <= attribLayout->m_MaxInputSlot)) {
        return false;
    }

    if (inputAssembler->m_VaoID == 0) {
        // generate vao
        GL_CALL_EXTENSION_FUNCTION(glGenVertexArraysOES(1, &inputAssembler->m_VaoID));
        Q_ASSERT(inputAssembler->m_VaoID);
    }

    if (inputAssembler->m_cachedShaderHandle != programID) {
        GL_CALL_EXTENSION_FUNCTION(glBindVertexArrayOES(inputAssembler->m_VaoID));
        inputAssembler->m_cachedShaderHandle = programID;

        QDEMON_FOREACH(idx, shaderAttribBuffer.size())
        {
            const QDemonRenderBackendShaderInputEntryGL &attrib(shaderAttribBuffer[idx]);
            QDemonRenderBackendLayoutEntryGL *entry =
                    attribLayout->getEntryByName(attrib.m_AttribName);

            if (entry) {
                QDemonRenderBackendLayoutEntryGL &entryData(*entry);
                if (entryData.m_Type != attrib.m_Type
                        || entryData.m_NumComponents != attrib.m_NumComponents) {
                    qCCritical(INVALID_OPERATION, "Attrib %s dn't match vertex layout",
                               qPrintable(attrib.m_AttribName));
                    Q_ASSERT(false);
                    return false;
                } else {
                    entryData.m_AttribIndex = attrib.m_AttribLocation;
                }
            } else {
                qCWarning(WARNING, "Failed to Bind attribute %s", qPrintable(attrib.m_AttribName));
            }
        }

        // disable max possible used first
        // this is currently sufficient since we always re-arrange input attributes from 0
        for (quint32 i = 0; i < attribLayout->m_LayoutAttribEntries.size(); i++)
            GL_CALL_EXTRA_FUNCTION(glDisableVertexAttribArray(i));

        // setup all attribs
        QDEMON_FOREACH(idx, shaderAttribBuffer.size())
        {
            QDemonRenderBackendLayoutEntryGL *entry =
                    attribLayout->getEntryByName(shaderAttribBuffer[idx].m_AttribName);
            if (entry) {
                const QDemonRenderBackendLayoutEntryGL &entryData(*entry);
                GLuint id = HandleToID_cast(
                            GLuint, size_t,
                            inputAssembler->m_VertexbufferHandles.mData[entryData.m_InputSlot]);
                GL_CALL_EXTRA_FUNCTION(glBindBuffer(GL_ARRAY_BUFFER, id));
                GL_CALL_EXTRA_FUNCTION(glEnableVertexAttribArray(entryData.m_AttribIndex));
                GLuint offset = inputAssembler->m_offsets[entryData.m_InputSlot];
                GLuint stride = inputAssembler->m_strides[entryData.m_InputSlot];
                GL_CALL_EXTRA_FUNCTION(glVertexAttribPointer(
                                           entryData.m_AttribIndex, entryData.m_NumComponents, GL_FLOAT, GL_FALSE,
                                           stride, (const void *)(entryData.m_Offset + offset)));

            } else {
                GL_CALL_EXTRA_FUNCTION(glDisableVertexAttribArray(idx));
            }
        }

        // setup index buffer.
        if (inputAssembler->m_IndexbufferHandle) {
            GL_CALL_EXTRA_FUNCTION(glBindBuffer(
                                       GL_ELEMENT_ARRAY_BUFFER,
                                       HandleToID_cast(GLuint, size_t, inputAssembler->m_IndexbufferHandle)));
        } else {
            GL_CALL_EXTRA_FUNCTION(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        }
    } else {
        GL_CALL_EXTENSION_FUNCTION(glBindVertexArrayOES(inputAssembler->m_VaoID));
    }
#ifdef _DEBUG
    if (inputAssembler->m_VaoID) {
        QDEMON_FOREACH(idx, shaderAttribBuffer.size())
        {
            const QDemonRenderBackendShaderInputEntryGL &attrib(shaderAttribBuffer[idx]);
            QDemonRenderBackendLayoutEntryGL *entry =
                    attribLayout->getEntryByName(attrib.m_AttribName);

            if (entry) {
                QDemonRenderBackendLayoutEntryGL &entryData(*entry);
                if (entryData.m_Type != attrib.m_Type
                        || entryData.m_NumComponents != attrib.m_NumComponents
                        || entryData.m_AttribIndex != attrib.m_AttribLocation) {
                    qCCritical(INVALID_OPERATION, "Attrib %s dn't match vertex layout",
                               qPrintable(attrib.m_AttribName));
                    Q_ASSERT(false);
                }
            } else {
                qCWarning(WARNING, "Failed to Bind attribute %s", qPrintable(attrib.m_AttribName));
            }
        }
    }
#endif // _DEBUG

    return true;
}

void QDemonRenderBackendGLES2Impl::SetDrawBuffers(QDemonRenderBackendRenderTargetObject rto,
                                              QDemonConstDataRef<qint32> inDrawBufferSet)
{
    NVRENDER_BACKEND_UNUSED(rto);

    m_DrawBuffersArray.clear();

    for (quint32 idx = 0, end = inDrawBufferSet.size(); idx < end; ++idx) {
        if (inDrawBufferSet[idx] < 0)
            m_DrawBuffersArray.push_back(GL_NONE);
        else
            m_DrawBuffersArray.push_back(GL_COLOR_ATTACHMENT0 + inDrawBufferSet[idx]);
    }

    GL_CALL_EXTRA_FUNCTION(glDrawBuffers((int)m_DrawBuffersArray.size(),
                                         m_DrawBuffersArray.data()));
}

void QDemonRenderBackendGLES2Impl::SetReadBuffer(QDemonRenderBackendRenderTargetObject rto,
                                             QDemonReadFaces::Enum inReadFace)
{
    NVRENDER_BACKEND_UNUSED(rto);
    NVRENDER_BACKEND_UNUSED(inReadFace);
}

void QDemonRenderBackendGLES2Impl::RenderTargetAttach(QDemonRenderBackendRenderTargetObject,
                                                  QDemonRenderFrameBufferAttachments::Enum attachment,
                                                  QDemonRenderBackendTextureObject to, qint32 level,
                                                  qint32 layer)
{
    NVRENDER_BACKEND_UNUSED(attachment);
    NVRENDER_BACKEND_UNUSED(to);
    NVRENDER_BACKEND_UNUSED(level);
    NVRENDER_BACKEND_UNUSED(layer);
    Q_ASSERT(false);
}

void QDemonRenderBackendGLES2Impl::BlitFramebuffer(qint32 srcX0, qint32 srcY0, qint32 srcX1,
                                               qint32 srcY1, qint32 dstX0, qint32 dstY0,
                                               qint32 dstX1, qint32 dstY1,
                                               QDemonRenderClearFlags flags,
                                               QDemonRenderTextureMagnifyingOp::Enum filter)
{
    GL_CALL_EXTRA_FUNCTION(glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1,
                                             dstY1,
                                             m_Conversion.fromClearFlagsToGL(flags),
                                             m_Conversion.fromTextureMagnifyingOpToGL(filter)));
}


QDemonRenderBackend::QDemonRenderBackendRenderTargetObject QDemonRenderBackendGLES2Impl::CreateRenderTarget()
{
    GLuint fboID = 0;
    GL_CALL_EXTRA_FUNCTION(glGenFramebuffers(1, &fboID));
    return (QDemonRenderBackend::QDemonRenderBackendRenderTargetObject)fboID;
}

void QDemonRenderBackendGLES2Impl::ReleaseRenderTarget(QDemonRenderBackendRenderTargetObject rto)
{
    GLuint fboID = HandleToID_cast(GLuint, size_t, rto);

    if (fboID)
        GL_CALL_EXTRA_FUNCTION(glDeleteFramebuffers(1, &fboID));
}

void QDemonRenderBackendGLES2Impl::RenderTargetAttach(QDemonRenderBackendRenderTargetObject /* rto */,
                                                  QDemonRenderFrameBufferAttachments::Enum attachment,
                                                  QDemonRenderBackendRenderbufferObject rbo)
{
    // rto must be the current render target
    GLuint rbID = HandleToID_cast(GLuint, size_t, rbo);

    GLenum glAttach = GLConversion::fromFramebufferAttachmentsToGL(attachment);

    GL_CALL_EXTRA_FUNCTION(glFramebufferRenderbuffer(GL_FRAMEBUFFER, glAttach, GL_RENDERBUFFER,
                                                     rbID));
}

void QDemonRenderBackendGLES2Impl::RenderTargetAttach(QDemonRenderBackendRenderTargetObject /* rto */,
                                                  QDemonRenderFrameBufferAttachments::Enum attachment,
                                                  QDemonRenderBackendTextureObject to,
                                                  QDemonRenderTextureTargetType::Enum target)
{
    // rto must be the current render target
    GLuint texID = HandleToID_cast(GLuint, size_t, to);

    Q_ASSERT(target == QDemonRenderTextureTargetType::Texture2D
                 || m_backendSupport.caps.bits.bMsTextureSupported);

    GLenum glAttach = GLConversion::fromFramebufferAttachmentsToGL(attachment);
    GLenum glTarget = m_Conversion.fromTextureTargetToGL(target);

    if (attachment == QDemonRenderFrameBufferAttachments::DepthStencil) {
        GL_CALL_EXTRA_FUNCTION(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                                      glTarget, texID, 0));
        GL_CALL_EXTRA_FUNCTION(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                                      glTarget, texID, 0));
    } else {
        GL_CALL_EXTRA_FUNCTION(glFramebufferTexture2D(GL_FRAMEBUFFER, glAttach, glTarget, texID,
                                                      0));
    }
}

void QDemonRenderBackendGLES2Impl::SetRenderTarget(QDemonRenderBackendRenderTargetObject rto)
{
    GLuint fboID = HandleToID_cast(GLuint, size_t, rto);

    GL_CALL_EXTRA_FUNCTION(glBindFramebuffer(GL_FRAMEBUFFER, fboID));
}

void QDemonRenderBackendGLES2Impl::SetReadTarget(QDemonRenderBackendRenderTargetObject rto)
{
    GLuint fboID = HandleToID_cast(GLuint, size_t, rto);

    GL_CALL_EXTRA_FUNCTION(glBindFramebuffer(GL_READ_FRAMEBUFFER, fboID));
}

bool QDemonRenderBackendGLES2Impl::RenderTargetIsValid(QDemonRenderBackendRenderTargetObject /* rto */)
{
    GLenum completeStatus = GL_CALL_EXTRA_FUNCTION(glCheckFramebufferStatus(GL_FRAMEBUFFER));
    switch (completeStatus) {
#define HANDLE_INCOMPLETE_STATUS(x)                                                                \
    case x:                                                                                        \
    qCCritical(INTERNAL_ERROR, "Framebuffer is not complete: %s", #x);           \
    return false;
    HANDLE_INCOMPLETE_STATUS(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
            HANDLE_INCOMPLETE_STATUS(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS)
            HANDLE_INCOMPLETE_STATUS(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
            HANDLE_INCOMPLETE_STATUS(GL_FRAMEBUFFER_UNSUPPORTED)
        #undef HANDLE_INCOMPLETE_STATUS
    }
    return true;
}

QDemonRenderBackend::QDemonRenderBackendRenderbufferObject
QDemonRenderBackendGLES2Impl::CreateRenderbuffer(QDemonRenderRenderBufferFormats::Enum storageFormat,
                                             size_t width, size_t height)
{
    GLuint bufID = 0;

    GL_CALL_EXTRA_FUNCTION(glGenRenderbuffers(1, &bufID));
    GL_CALL_EXTRA_FUNCTION(glBindRenderbuffer(GL_RENDERBUFFER, bufID));
    GL_CALL_EXTRA_FUNCTION(glRenderbufferStorage(GL_RENDERBUFFER,
                                                 GLConversion::fromRenderBufferFormatsToRenderBufferGL(storageFormat),
                                                 (GLsizei)width, (GLsizei)height));

    // check for error
    GLenum error = m_glFunctions->glGetError();
    if (error != GL_NO_ERROR) {
        qCCritical(GL_ERROR, GLConversion::processGLError(error));
        Q_ASSERT(false);
        GL_CALL_EXTRA_FUNCTION(glDeleteRenderbuffers(1, &bufID));
        bufID = 0;
    }

    GL_CALL_EXTRA_FUNCTION(glBindRenderbuffer(GL_RENDERBUFFER, 0));

    return (QDemonRenderBackend::QDemonRenderBackendRenderbufferObject)bufID;
}

void QDemonRenderBackendGLES2Impl::ReleaseRenderbuffer(QDemonRenderBackendRenderbufferObject rbo)
{
    GLuint bufID = HandleToID_cast(GLuint, size_t, rbo);

    if (bufID)
        GL_CALL_EXTRA_FUNCTION(glDeleteRenderbuffers(1, &bufID));
}

bool QDemonRenderBackendGLES2Impl::ResizeRenderbuffer(QDemonRenderBackendRenderbufferObject rbo,
                                                  QDemonRenderRenderBufferFormats::Enum storageFormat,
                                                  size_t width, size_t height)
{
    bool success = true;
    GLuint bufID = HandleToID_cast(GLuint, size_t, rbo);

    Q_ASSERT(bufID);

    GL_CALL_EXTRA_FUNCTION(glBindRenderbuffer(GL_RENDERBUFFER, bufID));
    GL_CALL_EXTRA_FUNCTION(glRenderbufferStorage(GL_RENDERBUFFER,
                                                 GLConversion::fromRenderBufferFormatsToRenderBufferGL(storageFormat),
                                                 (GLsizei)width, (GLsizei)height));

    // check for error
    GLenum error = m_glFunctions->glGetError();
    if (error != GL_NO_ERROR) {
        qCCritical(GL_ERROR, GLConversion::processGLError(error));
        Q_ASSERT(false);
        success = false;
    }

    return success;
}

void *QDemonRenderBackendGLES2Impl::MapBuffer(QDemonRenderBackendBufferObject,
                                          QDemonRenderBufferBindFlags bindFlags, size_t offset,
                                          size_t length, QDemonRenderBufferAccessFlags accessFlags)
{
    void *ret = nullptr;
    ret = GL_CALL_EXTRA_FUNCTION(
                glMapBufferRange(m_Conversion.fromBindBufferFlagsToGL(bindFlags), offset,
                                 length, m_Conversion.fromBufferAccessBitToGL(accessFlags)));

    return ret;
}

bool QDemonRenderBackendGLES2Impl::UnmapBuffer(QDemonRenderBackendBufferObject,
                                           QDemonRenderBufferBindFlags bindFlags)
{
    GLboolean ret;

    ret = GL_CALL_EXTRA_FUNCTION(glUnmapBuffer(m_Conversion.fromBindBufferFlagsToGL(bindFlags)));

    return (ret) ? true : false;
}

qint32 QDemonRenderBackendGLES2Impl::GetConstantBufferCount(QDemonRenderBackendShaderProgramObject po)
{
    Q_ASSERT(po);
    GLint numUniformBuffers = 0;
    if (GetRenderBackendCap(QDemonRenderBackendCaps::ConstantBuffer)) {
        QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
        GLuint programID = static_cast<GLuint>(pProgram->m_ProgramID);

        GL_CALL_EXTRA_FUNCTION(glGetProgramiv(programID, GL_ACTIVE_UNIFORM_BLOCKS,
                                              &numUniformBuffers));
    }
    return numUniformBuffers;
}

qint32 QDemonRenderBackendGLES2Impl::GetConstantBufferInfoByID(
                                                        QDemonRenderBackendShaderProgramObject po,
                                                        quint32 id, quint32 nameBufSize,
                                                        qint32 *paramCount, qint32 *bufferSize,
                                                        qint32 *length, char *nameBuf)
{
    Q_ASSERT(po);
    Q_ASSERT(length);
    Q_ASSERT(nameBuf);

    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_ProgramID);
    GLuint blockIndex = GL_INVALID_INDEX;

    GL_CALL_EXTRA_FUNCTION(glGetActiveUniformBlockName(programID, id, nameBufSize, length,
                                                       nameBuf));

    if (*length > 0) {
        blockIndex = GL_CALL_EXTRA_FUNCTION(glGetUniformBlockIndex(programID, nameBuf));
        if (blockIndex != GL_INVALID_INDEX) {
            GL_CALL_EXTRA_FUNCTION(glGetActiveUniformBlockiv(programID, blockIndex,
                                                             GL_UNIFORM_BLOCK_DATA_SIZE, bufferSize));
            GL_CALL_EXTRA_FUNCTION(glGetActiveUniformBlockiv(programID, blockIndex,
                                                             GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, paramCount));
        }
    }

    return blockIndex;
}

void QDemonRenderBackendGLES2Impl::GetConstantBufferParamIndices(QDemonRenderBackendShaderProgramObject po,
                                                             quint32 id, qint32 *indices)
{
    Q_ASSERT(po);
    Q_ASSERT(indices);

    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_ProgramID);

    if (indices) {
        GL_CALL_EXTRA_FUNCTION(glGetActiveUniformBlockiv(programID, id,
                                                         GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES,
                                                         indices));
    }
}

void QDemonRenderBackendGLES2Impl::GetConstantBufferParamInfoByIndices(
        QDemonRenderBackendShaderProgramObject po, quint32 count, quint32 *indices, qint32 *type,
        qint32 *size, qint32 *offset)
{
    Q_ASSERT(po);
    Q_ASSERT(count);
    Q_ASSERT(indices);

    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_ProgramID);

    if (count && indices) {
        if (type) {
            GL_CALL_EXTRA_FUNCTION(glGetActiveUniformsiv(programID, count, indices,
                                                         GL_UNIFORM_TYPE, type));
            // convert to UIC types
            QDEMON_FOREACH(idx, count)
            {
                type[idx] = m_Conversion.fromShaderGLToPropertyDataTypes(type[idx]);
            }
        }
        if (size) {
            GL_CALL_EXTRA_FUNCTION(glGetActiveUniformsiv(programID, count, indices,
                                                         GL_UNIFORM_SIZE, size));
        }
        if (offset) {
            GL_CALL_EXTRA_FUNCTION(
                        glGetActiveUniformsiv(programID, count, indices, GL_UNIFORM_OFFSET, offset));
        }
    }
}

void QDemonRenderBackendGLES2Impl::ProgramSetConstantBlock(QDemonRenderBackendShaderProgramObject po,
                                                       quint32 blockIndex, quint32 binding)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_ProgramID);

    GL_CALL_EXTRA_FUNCTION(glUniformBlockBinding(programID, blockIndex, binding));
}

void QDemonRenderBackendGLES2Impl::ProgramSetConstantBuffer(quint32 index,
                                                        QDemonRenderBackendBufferObject bo)
{
    Q_ASSERT(bo);

    GLuint bufID = HandleToID_cast(GLuint, size_t, bo);
    GL_CALL_EXTRA_FUNCTION(glBindBufferBase(GL_UNIFORM_BUFFER, index, bufID));
}

QDemonRenderBackend::QDemonRenderBackendQueryObject QDemonRenderBackendGLES2Impl::CreateQuery()
{
    quint32 glQueryID = 0;

    return (QDemonRenderBackendQueryObject)glQueryID;
}

void QDemonRenderBackendGLES2Impl::ReleaseQuery(QDemonRenderBackendQueryObject)
{

}

void QDemonRenderBackendGLES2Impl::BeginQuery(QDemonRenderBackendQueryObject,
                                          QDemonRenderQueryType::Enum)
{

}

void QDemonRenderBackendGLES2Impl::EndQuery(QDemonRenderBackendQueryObject, QDemonRenderQueryType::Enum)
{

}

void QDemonRenderBackendGLES2Impl::GetQueryResult(QDemonRenderBackendQueryObject,
                                              QDemonRenderQueryResultType::Enum,
                                              quint32 *)
{

}

void QDemonRenderBackendGLES2Impl::GetQueryResult(QDemonRenderBackendQueryObject,
                                              QDemonRenderQueryResultType::Enum,
                                              quint64 *)
{

}

void QDemonRenderBackendGLES2Impl::SetQueryTimer(QDemonRenderBackendQueryObject)
{

}

QDemonRenderBackend::QDemonRenderBackendSyncObject
QDemonRenderBackendGLES2Impl::CreateSync(QDemonRenderSyncType::Enum, QDemonRenderSyncFlags)
{
    GLsync syncID = 0;
    return QDemonRenderBackendSyncObject(syncID);
}

void QDemonRenderBackendGLES2Impl::ReleaseSync(QDemonRenderBackendSyncObject)
{

}

void QDemonRenderBackendGLES2Impl::WaitSync(QDemonRenderBackendSyncObject, QDemonRenderCommandFlushFlags,
                                        quint64)
{

}

QT_END_NAMESPACE
