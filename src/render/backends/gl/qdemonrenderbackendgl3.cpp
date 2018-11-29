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

#define GL_CALL_EXTRA_FUNCTION(x) m_glExtraFunctions->x; RENDER_LOG_ERROR_PARAMS(x);

#if defined(QT_OPENGL_ES)
#define GL_CALL_TIMER_EXT(x) m_qt3dsExtensions->x; RENDER_LOG_ERROR_PARAMS(x);
#define GL_CALL_TESSELATION_EXT(x) m_qt3dsExtensions->x; RENDER_LOG_ERROR_PARAMS(x);
#else
#define GL_CALL_TIMER_EXT(x) m_timerExtension->x; RENDER_LOG_ERROR_PARAMS(x);
#define GL_CALL_TESSELATION_EXT(x) m_tessellationShader->x; RENDER_LOG_ERROR_PARAMS(x);
#define GL_CALL_MULTISAMPLE_EXT(x) m_multiSample->x; RENDER_LOG_ERROR_PARAMS(x);
#endif

#ifndef GL_PATCH_VERTICES
#define GL_PATCH_VERTICES 0x8E72
#endif

/// constructor
QDemonRenderBackendGL3Impl::QDemonRenderBackendGL3Impl(const QSurfaceFormat &format)
    : QDemonRenderBackendGLBase(format)
{
    QString exts3tc("GL_EXT_texture_compression_s3tc");
    QString extsdxt("GL_EXT_texture_compression_dxt1");
    QString extsAniso("GL_EXT_texture_filter_anisotropic");
    QString extsTexSwizzle("GL_ARB_texture_swizzle");
    QString extsAstcHDR("GL_KHR_texture_compression_astc_hdr");
    QString extsAstcLDR("GL_KHR_texture_compression_astc_ldr");
    QString extsFPRenderTarget("GL_EXT_color_buffer_float");
    QString extsTimerQuery("GL_EXT_timer_query");
    QString extsGpuShader5("EXT_gpu_shader5");

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

    // get extension count
    GLint numExtensions = 0;
    GL_CALL_EXTRA_FUNCTION(glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions));

    QString extensionBuffer;

    for (qint32 i = 0; i < numExtensions; i++) {
        char *extensionString = (char *)GL_CALL_EXTRA_FUNCTION(glGetStringi(GL_EXTENSIONS, i));

        m_extensions.push_back(QString::fromLocal8Bit(extensionString));

        if (extensionBuffer.size())
            extensionBuffer.append(" ");
        extensionBuffer.append(extensionString);

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
        }

    }

    qCInfo(TRACE_INFO, "OpenGL extensions: %s", qPrintable(extensionBuffer));

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
    GL_CALL_EXTRA_FUNCTION(glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &m_MaxAttribCount));

    // internal state tracker
    m_pCurrentMiscState = new QDemonRenderBackendMiscStateGL();

    // finally setup caps based on device
    setAndInspectHardwareCaps();

    // Initialize extensions
#if defined(QT_OPENGL_ES_2)
    m_qt3dsExtensions = new QDemonOpenGLES2Extensions;
    m_qt3dsExtensions->initializeOpenGLFunctions();
#else
    m_timerExtension = new QOpenGLExtension_ARB_timer_query;
    m_timerExtension->initializeOpenGLFunctions();
    m_tessellationShader = new QOpenGLExtension_ARB_tessellation_shader;
    m_tessellationShader->initializeOpenGLFunctions();
    m_multiSample = new QOpenGLExtension_ARB_texture_multisample;
    m_multiSample->initializeOpenGLFunctions();
    m_qt3dsExtensions = new QDemonOpenGLExtensions;
    m_qt3dsExtensions->initializeOpenGLFunctions();
#endif
}
/// destructor
QDemonRenderBackendGL3Impl::~QDemonRenderBackendGL3Impl()
{
    if (m_pCurrentMiscState)
        delete m_pCurrentMiscState;
#if !defined(QT_OPENGL_ES_2)
    if (m_timerExtension)
        delete m_timerExtension;
    if (m_tessellationShader)
        delete m_tessellationShader;
    if (m_multiSample)
        delete m_multiSample;
#endif
    if (m_qt3dsExtensions)
        delete m_qt3dsExtensions;
}

void QDemonRenderBackendGL3Impl::SetMultisampledTextureData2D(
        QDemonRenderBackendTextureObject to, QDemonRenderTextureTargetType::Enum target, size_t samples,
        QDemonRenderTextureFormats::Enum internalFormat, size_t width, size_t height,
        bool fixedsamplelocations)
{
    // Not supported by ES 3 yet
#if defined(QT_OPENGL_ES)
    NVRENDER_BACKEND_UNUSED(to);
    NVRENDER_BACKEND_UNUSED(target);
    NVRENDER_BACKEND_UNUSED(samples);
    NVRENDER_BACKEND_UNUSED(internalFormat);
    NVRENDER_BACKEND_UNUSED(width);
    NVRENDER_BACKEND_UNUSED(height);
    NVRENDER_BACKEND_UNUSED(fixedsamplelocations);
#else
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GLenum glTarget = m_Conversion.fromTextureTargetToGL(target);
    GL_CALL_EXTRA_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_EXTRA_FUNCTION(glBindTexture(glTarget, texID));

    QDemonRenderTextureSwizzleMode::Enum swizzleMode = QDemonRenderTextureSwizzleMode::NoSwizzle;
    internalFormat = m_Conversion.replaceDeprecatedTextureFormat(GetRenderContextType(),
                                                                 internalFormat, swizzleMode);

    GLenum glformat = 0, glInternalFormat = 0, gltype = GL_UNSIGNED_BYTE;

    if (QDemonRenderTextureFormats::isUncompressedTextureFormat(internalFormat))
        GLConversion::fromUncompressedTextureFormatToGL(GetRenderContextType(), internalFormat,
                                                        glformat, gltype, glInternalFormat);
    else if (QDemonRenderTextureFormats::isDepthTextureFormat(internalFormat))
        m_Conversion.fromDepthTextureFormatToGL(GetRenderContextType(), internalFormat,
                                                glformat, gltype, glInternalFormat);

    GL_CALL_MULTISAMPLE_EXT(glTexImage2DMultisample(glTarget, (GLsizei)samples, glInternalFormat,
                                                    (GLsizei)width, (GLsizei)height, fixedsamplelocations));

    GL_CALL_EXTRA_FUNCTION(glBindTexture(glTarget, 0));
#endif
}

void QDemonRenderBackendGL3Impl::SetTextureData3D(
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

    if (QDemonRenderTextureFormats::isUncompressedTextureFormat(internalFormat))
        m_Conversion.fromUncompressedTextureFormatToGL(GetRenderContextType(), internalFormat,
                                                       glformat, gltype, glInternalFormat);

    if (conversionRequired) {
        GLenum dummy;
        m_Conversion.fromUncompressedTextureFormatToGL(GetRenderContextType(), format, glformat,
                                                       gltype, dummy);
    } else if (QDemonRenderTextureFormats::isCompressedTextureFormat(internalFormat)) {
        m_Conversion.fromUncompressedTextureFormatToGL(GetRenderContextType(), format, glformat,
                                                       gltype, glInternalFormat);
        glInternalFormat = m_Conversion.fromCompressedTextureFormatToGL(internalFormat);
    } else if (QDemonRenderTextureFormats::isDepthTextureFormat(format))
        m_Conversion.fromDepthTextureFormatToGL(GetRenderContextType(), format, glformat,
                                                gltype, glInternalFormat);

    GL_CALL_EXTRA_FUNCTION(glTexImage3D(glTarget, level, glInternalFormat, (GLsizei)width, (GLsizei)height,
                                        (GLsizei)depth, border, glformat, gltype, hostPtr));

    GL_CALL_EXTRA_FUNCTION(glBindTexture(glTarget, 0));
}

void QDemonRenderBackendGL3Impl::UpdateSampler(
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

    GLenum glTarget = m_Conversion.fromTextureTargetToGL(target);

    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_MIN_FILTER,
                                           m_Conversion.fromTextureMinifyingOpToGL(minFilter)));
    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_MAG_FILTER,
                                           m_Conversion.fromTextureMagnifyingOpToGL(magFilter)));
    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_WRAP_S,
                                           m_Conversion.fromTextureCoordOpToGL(wrapS)));
    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_WRAP_T,
                                           m_Conversion.fromTextureCoordOpToGL(wrapT)));
    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_WRAP_R,
                                           m_Conversion.fromTextureCoordOpToGL(wrapR)));
    GL_CALL_EXTRA_FUNCTION(glTexParameterf(glTarget, GL_TEXTURE_MIN_LOD, minLod));
    GL_CALL_EXTRA_FUNCTION(glTexParameterf(glTarget, GL_TEXTURE_MAX_LOD, maxLod));
    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_COMPARE_MODE,
                                           m_Conversion.fromTextureCompareModeToGL(compareMode)));
    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_COMPARE_FUNC,
                                           m_Conversion.fromTextureCompareFuncToGL(compareFunc)));

    if (m_backendSupport.caps.bits.bAnistropySupported) {
        GL_CALL_EXTRA_FUNCTION(glTexParameterf(glTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy));
    }
}

void QDemonRenderBackendGL3Impl::UpdateTextureObject(QDemonRenderBackendTextureObject to,
                                                     QDemonRenderTextureTargetType::Enum target,
                                                     qint32 baseLevel, qint32 maxLevel)
{
    NVRENDER_BACKEND_UNUSED(to);

    GLenum glTarget = m_Conversion.fromTextureTargetToGL(target);

    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_BASE_LEVEL, baseLevel));
    GL_CALL_EXTRA_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_MAX_LEVEL, maxLevel));
}

void QDemonRenderBackendGL3Impl::UpdateTextureSwizzle(QDemonRenderBackendTextureObject to,
                                                      QDemonRenderTextureTargetType::Enum target,
                                                      QDemonRenderTextureSwizzleMode::Enum swizzleMode)
{
    NVRENDER_BACKEND_UNUSED(to);
    if (m_backendSupport.caps.bits.bTextureSwizzleSupported) {
        GLint glSwizzle[4];
        GLenum glTarget = m_Conversion.fromTextureTargetToGL(target);
        m_Conversion.NVRenderConvertSwizzleModeToGL(swizzleMode, glSwizzle);
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

quint32
QDemonRenderBackendGL3Impl::GetDepthBits() const
{
    qint32 depthBits;
    GL_CALL_EXTRA_FUNCTION(glGetFramebufferAttachmentParameteriv(
                               GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depthBits));

    return depthBits;
}

quint32
QDemonRenderBackendGL3Impl::GetStencilBits() const
{
    qint32 stencilBits;
    GL_CALL_EXTRA_FUNCTION(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                                                 GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE,
                                                                 &stencilBits));

    return stencilBits;
}

void QDemonRenderBackendGL3Impl::GenerateMipMaps(QDemonRenderBackendTextureObject to,
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

bool QDemonRenderBackendGL3Impl::SetInputAssembler(QDemonRenderBackendInputAssemblerObject iao,
                                                   QDemonRenderBackendShaderProgramObject po)
{
    if (iao == nullptr) {
        // unbind and return;
        GL_CALL_EXTRA_FUNCTION(glBindVertexArray(0));
        return true;
    }

    QDemonRenderBackendInputAssemblerGL *inputAssembler = (QDemonRenderBackendInputAssemblerGL *)iao;
    QDemonRenderBackendAttributeLayoutGL *attribLayout = inputAssembler->m_attribLayout;
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_ProgramID);
    QDemonDataRef<QDemonRenderBackendShaderInputEntryGL> shaderAttribBuffer;
    if (pProgram->m_shaderInput)
        shaderAttribBuffer = pProgram->m_shaderInput->m_ShaderInputEntries;

    if (attribLayout->m_LayoutAttribEntries.size() < shaderAttribBuffer.size())
        return false;

    if (inputAssembler->m_VertexbufferHandles.size() <= attribLayout->m_MaxInputSlot) {
        Q_ASSERT(false);
        return false;
    }

    if (inputAssembler->m_VaoID == 0) {
        // generate vao
        GL_CALL_EXTRA_FUNCTION(glGenVertexArrays(1, &inputAssembler->m_VaoID));
        Q_ASSERT(inputAssembler->m_VaoID);
    }

    // set patch parameter count if changed
    if (m_backendSupport.caps.bits.bTessellationSupported
            && m_pCurrentMiscState->m_PatchVertexCount != inputAssembler->m_PatchVertexCount) {
        m_pCurrentMiscState->m_PatchVertexCount = inputAssembler->m_PatchVertexCount;
#if defined(QT_OPENGL_ES)
        GL_CALL_TESSELATION_EXT(glPatchParameteriEXT(GL_PATCH_VERTICES, inputAssembler->m_PatchVertexCount));
#else
        GL_CALL_TESSELATION_EXT(glPatchParameteri(GL_PATCH_VERTICES, inputAssembler->m_PatchVertexCount));
#endif
    }

    if (inputAssembler->m_cachedShaderHandle != programID) {
        GL_CALL_EXTRA_FUNCTION(glBindVertexArray(inputAssembler->m_VaoID));
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
                    qCCritical(INVALID_OPERATION, "Attrib %s doesn't match vertex layout",
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
        for (quint32 i = 0; i < attribLayout->m_LayoutAttribEntries.size(); i++) {
            GL_CALL_EXTRA_FUNCTION(glDisableVertexAttribArray(i));
        }

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
        GL_CALL_EXTRA_FUNCTION(glBindVertexArray(inputAssembler->m_VaoID));
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
                    qCCritical(INVALID_OPERATION, "Attrib %s doesn't match vertex layout",
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

void QDemonRenderBackendGL3Impl::SetDrawBuffers(QDemonRenderBackendRenderTargetObject rto,
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

    GL_CALL_EXTRA_FUNCTION(glDrawBuffers((int)m_DrawBuffersArray.size(), m_DrawBuffersArray.data()));
}

void QDemonRenderBackendGL3Impl::SetReadBuffer(QDemonRenderBackendRenderTargetObject rto,
                                               QDemonReadFaces::Enum inReadFace)
{
    NVRENDER_BACKEND_UNUSED(rto);

    GL_CALL_EXTRA_FUNCTION(glReadBuffer(m_Conversion.fromReadFacesToGL(inReadFace)));
}

void QDemonRenderBackendGL3Impl::RenderTargetAttach(QDemonRenderBackendRenderTargetObject,
                                                    QDemonRenderFrameBufferAttachments::Enum attachment,
                                                    QDemonRenderBackendTextureObject to, qint32 level,
                                                    qint32 layer)
{
    // rto must be the current render target
    GLuint texID = HandleToID_cast(GLuint, size_t, to);

    GLenum glAttach = GLConversion::fromFramebufferAttachmentsToGL(attachment);

    GL_CALL_EXTRA_FUNCTION(glFramebufferTextureLayer(GL_FRAMEBUFFER, glAttach, texID, level, layer))
}

void QDemonRenderBackendGL3Impl::SetReadTarget(QDemonRenderBackendRenderTargetObject rto)
{
    GLuint fboID = HandleToID_cast(GLuint, size_t, rto);

    GL_CALL_EXTRA_FUNCTION(glBindFramebuffer(GL_READ_FRAMEBUFFER, fboID));
}

void QDemonRenderBackendGL3Impl::BlitFramebuffer(qint32 srcX0, qint32 srcY0, qint32 srcX1, qint32 srcY1,
                                                 qint32 dstX0, qint32 dstY0, qint32 dstX1, qint32 dstY1,
                                                 QDemonRenderClearFlags flags,
                                                 QDemonRenderTextureMagnifyingOp::Enum filter)
{
    GL_CALL_EXTRA_FUNCTION(glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1,
                                             m_Conversion.fromClearFlagsToGL(flags),
                                             m_Conversion.fromTextureMagnifyingOpToGL(filter)));
}

void *QDemonRenderBackendGL3Impl::MapBuffer(QDemonRenderBackendBufferObject,
                                            QDemonRenderBufferBindFlags bindFlags, size_t offset,
                                            size_t length, QDemonRenderBufferAccessFlags accessFlags)
{
    void *ret = nullptr;
    ret = GL_CALL_EXTRA_FUNCTION(glMapBufferRange(m_Conversion.fromBindBufferFlagsToGL(bindFlags), offset,
                                                  length, m_Conversion.fromBufferAccessBitToGL(accessFlags)));

    return ret;
}

bool QDemonRenderBackendGL3Impl::UnmapBuffer(QDemonRenderBackendBufferObject,
                                             QDemonRenderBufferBindFlags bindFlags)
{
    GLboolean ret;

    ret = GL_CALL_EXTRA_FUNCTION(glUnmapBuffer(m_Conversion.fromBindBufferFlagsToGL(bindFlags)));

    return (ret) ? true : false;
}

qint32 QDemonRenderBackendGL3Impl::GetConstantBufferCount(QDemonRenderBackendShaderProgramObject po)
{
    Q_ASSERT(po);
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_ProgramID);

    GLint numUniformBuffers;
    GL_CALL_EXTRA_FUNCTION(glGetProgramiv(programID, GL_ACTIVE_UNIFORM_BLOCKS, &numUniformBuffers));

    return numUniformBuffers;
}

qint32
QDemonRenderBackendGL3Impl::GetConstantBufferInfoByID(QDemonRenderBackendShaderProgramObject po,
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

    GL_CALL_EXTRA_FUNCTION(glGetActiveUniformBlockName(programID, id, nameBufSize, length, nameBuf));

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

void
QDemonRenderBackendGL3Impl::GetConstantBufferParamIndices(QDemonRenderBackendShaderProgramObject po,
                                                          quint32 id, qint32 *indices)
{
    Q_ASSERT(po);
    Q_ASSERT(indices);

    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_ProgramID);

    if (indices) {
        GL_CALL_EXTRA_FUNCTION(glGetActiveUniformBlockiv(programID, id,
                                                         GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices));
    }
}

void QDemonRenderBackendGL3Impl::GetConstantBufferParamInfoByIndices(
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
            GL_CALL_EXTRA_FUNCTION(glGetActiveUniformsiv(programID, count, indices, GL_UNIFORM_TYPE, type));
            // convert to UIC types
            QDEMON_FOREACH(idx, count)
            {
                type[idx] = m_Conversion.fromShaderGLToPropertyDataTypes(type[idx]);
            }
        }
        if (size) {
            GL_CALL_EXTRA_FUNCTION(glGetActiveUniformsiv(programID, count, indices, GL_UNIFORM_SIZE, size));
        }
        if (offset) {
            GL_CALL_EXTRA_FUNCTION(
                        glGetActiveUniformsiv(programID, count, indices, GL_UNIFORM_OFFSET, offset));
        }
    }
}

void QDemonRenderBackendGL3Impl::ProgramSetConstantBlock(QDemonRenderBackendShaderProgramObject po,
                                                         quint32 blockIndex, quint32 binding)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_ProgramID);

    GL_CALL_EXTRA_FUNCTION(glUniformBlockBinding(programID, blockIndex, binding));
}

void QDemonRenderBackendGL3Impl::ProgramSetConstantBuffer(quint32 index,
                                                          QDemonRenderBackendBufferObject bo)
{
    Q_ASSERT(bo);

    GLuint bufID = HandleToID_cast(GLuint, size_t, bo);
    GL_CALL_EXTRA_FUNCTION(glBindBufferBase(GL_UNIFORM_BUFFER, index, bufID));
}

QDemonRenderBackend::QDemonRenderBackendQueryObject QDemonRenderBackendGL3Impl::CreateQuery()
{
    quint32 glQueryID = 0;

    GL_CALL_EXTRA_FUNCTION(glGenQueries(1, &glQueryID));

    return (QDemonRenderBackendQueryObject)glQueryID;
}

void QDemonRenderBackendGL3Impl::ReleaseQuery(QDemonRenderBackendQueryObject qo)
{
    GLuint queryID = HandleToID_cast(GLuint, size_t, qo);

    GL_CALL_EXTRA_FUNCTION(glDeleteQueries(1, &queryID));
}

void QDemonRenderBackendGL3Impl::BeginQuery(QDemonRenderBackendQueryObject qo,
                                            QDemonRenderQueryType::Enum type)
{
    GLuint queryID = HandleToID_cast(GLuint, size_t, qo);

    GL_CALL_EXTRA_FUNCTION(glBeginQuery(m_Conversion.fromQueryTypeToGL(type), queryID));
}

void QDemonRenderBackendGL3Impl::EndQuery(QDemonRenderBackendQueryObject, QDemonRenderQueryType::Enum type)
{
    GL_CALL_EXTRA_FUNCTION(glEndQuery(m_Conversion.fromQueryTypeToGL(type)));
}

void QDemonRenderBackendGL3Impl::GetQueryResult(QDemonRenderBackendQueryObject qo,
                                                QDemonRenderQueryResultType::Enum resultType,
                                                quint32 *params)
{
    GLuint queryID = HandleToID_cast(GLuint, size_t, qo);

    if (params)
        GL_CALL_EXTRA_FUNCTION(glGetQueryObjectuiv(
                                   queryID, m_Conversion.fromQueryResultTypeToGL(resultType), params));
}

void QDemonRenderBackendGL3Impl::GetQueryResult(QDemonRenderBackendQueryObject qo,
                                                QDemonRenderQueryResultType::Enum resultType,
                                                quint64 *params)
{
    if (m_backendSupport.caps.bits.bTimerQuerySupported) {
        GLuint queryID = HandleToID_cast(GLuint, size_t, qo);

        if (params)
#if defined(QT_OPENGL_ES)
            GL_CALL_TIMER_EXT(glGetQueryObjectui64vEXT(
                                  queryID, m_Conversion.fromQueryResultTypeToGL(resultType), params));
#else
            GL_CALL_TIMER_EXT(glGetQueryObjectui64v(
                                  queryID, m_Conversion.fromQueryResultTypeToGL(resultType), params));
#endif
    }
}

void QDemonRenderBackendGL3Impl::SetQueryTimer(QDemonRenderBackendQueryObject qo)
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

QDemonRenderBackend::QDemonRenderBackendSyncObject
QDemonRenderBackendGL3Impl::CreateSync(QDemonRenderSyncType::Enum syncType, QDemonRenderSyncFlags)
{
    GLsync syncID = 0;

    syncID = GL_CALL_EXTRA_FUNCTION(glFenceSync(m_Conversion.fromSyncTypeToGL(syncType), 0));

    return QDemonRenderBackendSyncObject(syncID);
}

void QDemonRenderBackendGL3Impl::ReleaseSync(QDemonRenderBackendSyncObject so)
{
    GLsync syncID = (GLsync)so;

    GL_CALL_EXTRA_FUNCTION(glDeleteSync(syncID));
}

void QDemonRenderBackendGL3Impl::WaitSync(QDemonRenderBackendSyncObject so, QDemonRenderCommandFlushFlags,
                                          quint64)
{
    GLsync syncID = (GLsync)so;

    GL_CALL_EXTRA_FUNCTION(glWaitSync(syncID, 0, GL_TIMEOUT_IGNORED));
}

QT_BEGIN_NAMESPACE
