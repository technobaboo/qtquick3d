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

#include <QtDemonRender/qdemonrenderbackendgl4.h>
#include <QtDemonRender/qdemonrenderbackendinputassemblergl.h>
#include <QtDemonRender/qdemonrenderbackendshaderprogramgl.h>

QT_BEGIN_NAMESPACE

#define NVRENDER_BACKEND_UNUSED(arg) (void)arg;

#ifdef RENDER_BACKEND_LOG_GL_ERRORS
#define RENDER_LOG_ERROR_PARAMS(x) checkGLError(#x, __FILE__, __LINE__)
#else
#define RENDER_LOG_ERROR_PARAMS(x) checkGLError()
#endif

#define GL_CALL_EXTRA_FUNCTION(x) m_glExtraFunctions->x; RENDER_LOG_ERROR_PARAMS(x);

#if defined(QT_OPENGL_ES)
#define GL_CALL_NVPATH_EXT(x) m_qdemonExtensions->x; RENDER_LOG_ERROR_PARAMS(x);
#define GL_CALL_QDEMON_EXT(x) m_qdemonExtensions->x; RENDER_LOG_ERROR_PARAMS(x);
#else
#define GL_CALL_NVPATH_EXT(x) m_nvPathRendering->x; RENDER_LOG_ERROR_PARAMS(x);
#define GL_CALL_DIRECTSTATE_EXT(x) m_directStateAccess->x; RENDER_LOG_ERROR_PARAMS(x);
#define GL_CALL_QDEMON_EXT(x) m_qdemonExtensions->x; RENDER_LOG_ERROR_PARAMS(x);
#endif

#ifndef GL_GEOMETRY_SHADER_EXT
#define GL_GEOMETRY_SHADER_EXT 0x8DD9
#endif

namespace QDemonGlExtStrings
{
QByteArray extTess() { return QByteArrayLiteral("GL_ARB_tessellation_shader"); }
QByteArray extGeometry() { return QByteArrayLiteral("GL_EXT_geometry_shader4"); }
QByteArray arbCompute() { return QByteArrayLiteral("GL_ARB_compute_shader"); }
QByteArray arbStorageBuffer() { return QByteArrayLiteral("GL_ARB_shader_storage_buffer_object"); }
QByteArray arbAtomicCounterBuffer() { return QByteArrayLiteral("GL_ARB_shader_atomic_counters"); }
QByteArray arbProgInterface() { return QByteArrayLiteral("GL_ARB_program_interface_query"); }
QByteArray arbShaderImageLoadStore() { return QByteArrayLiteral("GL_ARB_shader_image_load_store"); }
QByteArray nvPathRendering() { return QByteArrayLiteral("GL_NV_path_rendering"); }
QByteArray nvBlendAdvanced() { return QByteArrayLiteral("GL_NV_blend_equation_advanced"); }
QByteArray khrBlendAdvanced() { return QByteArrayLiteral("GL_KHR_blend_equation_advanced"); }
QByteArray nvBlendAdvancedCoherent() { return QByteArrayLiteral("GL_NV_blend_equation_advanced_coherent"); }
QByteArray khrBlendAdvancedCoherent() { return QByteArrayLiteral("GL_KHR_blend_equation_advanced_coherent"); }
}


/// constructor
QDemonRenderBackendGL4Impl::QDemonRenderBackendGL4Impl(const QSurfaceFormat &format)
    : QDemonRenderBackendGL3Impl(format)
{
    const QByteArray apiVersion(getVersionString());
    qCInfo(TRACE_INFO, "GL version: %s", apiVersion.constData());

    // get extension count
    GLint numExtensions = 0;
    GL_CALL_EXTRA_FUNCTION(glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions));

    for (qint32 i = 0; i < numExtensions; i++) {
        const GLubyte *glExt = GL_CALL_EXTRA_FUNCTION(glGetStringi(GL_EXTENSIONS, GLuint(i)));
        const QByteArray extensionString(reinterpret_cast<const char *>(glExt));

        // search for extension
        if (!m_backendSupport.caps.bits.bTessellationSupported
                && QDemonGlExtStrings::extTess().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bTessellationSupported = true;
        } else if (!m_backendSupport.caps.bits.bComputeSupported
                   && QDemonGlExtStrings::arbCompute().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bComputeSupported = true;
        } else if (!m_backendSupport.caps.bits.bGeometrySupported
                   && QDemonGlExtStrings::extGeometry().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bGeometrySupported = true;
        } else if (!m_backendSupport.caps.bits.bStorageBufferSupported
                   && QDemonGlExtStrings::arbStorageBuffer().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bStorageBufferSupported = true;
        } else if (!m_backendSupport.caps.bits.bAtomicCounterBufferSupported
                   && QDemonGlExtStrings::arbAtomicCounterBuffer().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bAtomicCounterBufferSupported = true;
        } else if (!m_backendSupport.caps.bits.bProgramInterfaceSupported
                   && QDemonGlExtStrings::arbProgInterface().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bProgramInterfaceSupported = true;
        } else if (!m_backendSupport.caps.bits.bShaderImageLoadStoreSupported
                   && QDemonGlExtStrings::arbShaderImageLoadStore().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bShaderImageLoadStoreSupported = true;
        } else if (!m_backendSupport.caps.bits.bNVPathRenderingSupported
                   && QDemonGlExtStrings::nvPathRendering().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bNVPathRenderingSupported = true;
        } else if (!m_backendSupport.caps.bits.bNVAdvancedBlendSupported
                   && QDemonGlExtStrings::nvBlendAdvanced().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bNVAdvancedBlendSupported = true;
        } else if (!m_backendSupport.caps.bits.bNVBlendCoherenceSupported
                   && QDemonGlExtStrings::nvBlendAdvancedCoherent().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bNVBlendCoherenceSupported = true;
        } else if (!m_backendSupport.caps.bits.bKHRAdvancedBlendSupported
                   && QDemonGlExtStrings::khrBlendAdvanced().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bKHRAdvancedBlendSupported = true;
        } else if (!m_backendSupport.caps.bits.bKHRBlendCoherenceSupported
                   && QDemonGlExtStrings::khrBlendAdvancedCoherent().compare(extensionString) == 0) {
            m_backendSupport.caps.bits.bKHRBlendCoherenceSupported = true;
        }
    }

    // always true for GL4.1 and GLES 3.1 devices
    m_backendSupport.caps.bits.bMsTextureSupported = true;
    m_backendSupport.caps.bits.bProgramPipelineSupported = true;

    if (!isESCompatible()) {
        // TODO: investigate GL 4.0 support
        // we expect minimum GL 4.1 context anything beyond is handeled via extensions
        // Tessellation is always supported on none ES systems which support >=GL4
        m_backendSupport.caps.bits.bTessellationSupported = true;
        // geometry shader is always supported on none ES systems which support >=GL4 ( actually
        // 3.2 already )
        m_backendSupport.caps.bits.bGeometrySupported = true;
    } else {
        // always true for GLES 3.1 devices
        m_backendSupport.caps.bits.bComputeSupported = true;
        m_backendSupport.caps.bits.bProgramInterfaceSupported = true;
        m_backendSupport.caps.bits.bStorageBufferSupported = true;
        m_backendSupport.caps.bits.bAtomicCounterBufferSupported = true;
        m_backendSupport.caps.bits.bShaderImageLoadStoreSupported = true;
    }

#if !defined(QT_OPENGL_ES)
    // Initialize extensions
    m_nvPathRendering = new QOpenGLExtension_NV_path_rendering();
    m_nvPathRendering->initializeOpenGLFunctions();
    m_directStateAccess = new QOpenGLExtension_EXT_direct_state_access();
    m_directStateAccess->initializeOpenGLFunctions();
#endif
}

/// destructor
QDemonRenderBackendGL4Impl::~QDemonRenderBackendGL4Impl()
{
#if !defined(QT_OPENGL_ES)
    delete m_nvPathRendering;
    delete m_directStateAccess;
#endif
}

void QDemonRenderBackendGL4Impl::drawIndirect(QDemonRenderDrawMode::Enum drawMode, const void *indirect)
{
    GL_CALL_EXTRA_FUNCTION(
                glDrawArraysIndirect(m_conversion.fromDrawModeToGL(
                                         drawMode, m_backendSupport.caps.bits.bTessellationSupported),
                                     indirect));
}

void QDemonRenderBackendGL4Impl::drawIndexedIndirect(QDemonRenderDrawMode::Enum drawMode,
                                                     QDemonRenderComponentTypes::Enum type,
                                                     const void *indirect)
{
    GL_CALL_EXTRA_FUNCTION(glDrawElementsIndirect(
                               m_conversion.fromDrawModeToGL(drawMode,
                                                             m_backendSupport.caps.bits.bTessellationSupported),
                               m_conversion.fromIndexBufferComponentsTypesToGL(type), indirect));
}

void QDemonRenderBackendGL4Impl::createTextureStorage2D(QDemonRenderBackendTextureObject to,
                                                        QDemonRenderTextureTargetType::Enum target,
                                                        quint32 levels,
                                                        QDemonRenderTextureFormats::Enum internalFormat,
                                                        size_t width, size_t height)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GLenum glTarget = GLConversion::fromTextureTargetToGL(target);
    GL_CALL_EXTRA_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_EXTRA_FUNCTION(glBindTexture(glTarget, texID));

    // up to now compressed is not supported
    Q_ASSERT(QDemonRenderTextureFormats::isUncompressedTextureFormat(internalFormat));

    GLenum glformat = 0, glInternalFormat = 0, gltype = GL_UNSIGNED_BYTE;
    GLConversion::fromUncompressedTextureFormatToGL(getRenderContextType(), internalFormat,
                                                    glformat, gltype, glInternalFormat);

    GL_CALL_EXTRA_FUNCTION(glTexStorage2D(glTarget, levels, glInternalFormat, GLsizei(width), GLsizei(height)));

    GL_CALL_EXTRA_FUNCTION(glBindTexture(glTarget, 0));
}

void QDemonRenderBackendGL4Impl::setMultisampledTextureData2D(
        QDemonRenderBackendTextureObject to, QDemonRenderTextureTargetType::Enum target, size_t samples,
        QDemonRenderTextureFormats::Enum internalFormat, size_t width, size_t height,
        bool fixedsamplelocations)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GLenum glTarget = GLConversion::fromTextureTargetToGL(target);
    GL_CALL_EXTRA_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_EXTRA_FUNCTION(glBindTexture(glTarget, texID));

    QDemonRenderTextureSwizzleMode::Enum swizzleMode = QDemonRenderTextureSwizzleMode::NoSwizzle;
    internalFormat = GLConversion::replaceDeprecatedTextureFormat(getRenderContextType(),
                                                                 internalFormat, swizzleMode);

    GLenum glformat = 0, glInternalFormat = 0, gltype = GL_UNSIGNED_BYTE;

    if (QDemonRenderTextureFormats::isUncompressedTextureFormat(internalFormat))
        GLConversion::fromUncompressedTextureFormatToGL(getRenderContextType(), internalFormat,
                                                        glformat, gltype, glInternalFormat);
    else if (QDemonRenderTextureFormats::isDepthTextureFormat(internalFormat))
        GLConversion::fromDepthTextureFormatToGL(getRenderContextType(), internalFormat,
                                                glformat, gltype, glInternalFormat);
    GL_CALL_EXTRA_FUNCTION(glTexStorage2DMultisample(glTarget, GLsizei(samples), glInternalFormat,
                                                     GLsizei(width), GLsizei(height),
                                                     fixedsamplelocations));

    GL_CALL_EXTRA_FUNCTION(glBindTexture(glTarget, 0));
}

QDemonRenderBackend::QDemonRenderBackendTessControlShaderObject
QDemonRenderBackendGL4Impl::createTessControlShader(QDemonConstDataRef<qint8> source,
                                                    QByteArray &errorMessage, bool binary)
{
#if !defined(QT_OPENGL_ES)
    GLuint shaderID = GL_CALL_EXTRA_FUNCTION(glCreateShader(GL_TESS_CONTROL_SHADER));
#else
    GLuint shaderID = 0;
#endif
    if (shaderID && !compileSource(shaderID, source, errorMessage, binary)) {
        GL_CALL_EXTRA_FUNCTION(glDeleteShader(shaderID));
        shaderID = 0;
    }

    return reinterpret_cast<QDemonRenderBackend::QDemonRenderBackendTessControlShaderObject>(shaderID);
}

QDemonRenderBackend::QDemonRenderBackendTessEvaluationShaderObject
QDemonRenderBackendGL4Impl::createTessEvaluationShader(QDemonConstDataRef<qint8> source,
                                                       QByteArray &errorMessage, bool binary)
{
#if !defined(QT_OPENGL_ES)
    GLuint shaderID = GL_CALL_EXTRA_FUNCTION(glCreateShader(GL_TESS_EVALUATION_SHADER));
#else
    GLuint shaderID = 0;
#endif

    if (shaderID && !compileSource(shaderID, source, errorMessage, binary)) {
        GL_CALL_EXTRA_FUNCTION(glDeleteShader(shaderID));
        shaderID = 0;
    }

    return reinterpret_cast<QDemonRenderBackend::QDemonRenderBackendTessEvaluationShaderObject>(shaderID);
}

QDemonRenderBackend::QDemonRenderBackendGeometryShaderObject QDemonRenderBackendGL4Impl::createGeometryShader(QDemonConstDataRef<qint8> source,
                                                                                                              QByteArray &errorMessage,
                                                                                                              bool binary)
{
#if defined(QT_OPENGL_ES)
    GLuint shaderID = GL_CALL_EXTRA_FUNCTION(glCreateShader(GL_GEOMETRY_SHADER_EXT));
#else
    GLuint shaderID = GL_CALL_EXTRA_FUNCTION(glCreateShader(GL_GEOMETRY_SHADER));
#endif
    if (shaderID && !compileSource(shaderID, source, errorMessage, binary)) {
        GL_CALL_EXTRA_FUNCTION(glDeleteShader(shaderID));
        shaderID = 0;
    }

    return reinterpret_cast<QDemonRenderBackend::QDemonRenderBackendGeometryShaderObject>(shaderID);
}

void QDemonRenderBackendGL4Impl::setPatchVertexCount(QDemonRenderBackendInputAssemblerObject iao,
                                                     quint32 count)
{
    Q_ASSERT(iao);
    Q_ASSERT(count);
    QDemonRenderBackendInputAssemblerGL *inputAssembler = reinterpret_cast<QDemonRenderBackendInputAssemblerGL *>(iao);
    inputAssembler->m_patchVertexCount = count;
}

void QDemonRenderBackendGL4Impl::setMemoryBarrier(QDemonRenderBufferBarrierFlags barriers)
{
    GL_CALL_EXTRA_FUNCTION(glMemoryBarrier(m_conversion.fromMemoryBarrierFlagsToGL(barriers)));
}

void QDemonRenderBackendGL4Impl::bindImageTexture(QDemonRenderBackendTextureObject to, quint32 unit,
                                                  qint32 level, bool layered, qint32 layer,
                                                  QDemonRenderImageAccessType::Enum access,
                                                  QDemonRenderTextureFormats::Enum format)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);

    GL_CALL_EXTRA_FUNCTION(glBindImageTexture(unit, texID, level, layered, layer,
                                              m_conversion.fromImageAccessToGL(access),
                                              m_conversion.fromImageFormatToGL(format)));
}

qint32 QDemonRenderBackendGL4Impl::getStorageBufferCount(QDemonRenderBackendShaderProgramObject po)
{
    GLint numStorageBuffers = 0;
    Q_ASSERT(po);
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_programID);

#if defined(GL_VERSION_4_3) || defined (QT_OPENGL_ES_3_1)
    if (m_backendSupport.caps.bits.bProgramInterfaceSupported)
        GL_CALL_EXTRA_FUNCTION(glGetProgramInterfaceiv(programID, GL_SHADER_STORAGE_BLOCK,
                                                       GL_ACTIVE_RESOURCES, &numStorageBuffers));
#endif
    return numStorageBuffers;
}

qint32
QDemonRenderBackendGL4Impl::getStorageBufferInfoByID(QDemonRenderBackendShaderProgramObject po,
                                                     quint32 id,
                                                     quint32 nameBufSize,
                                                     qint32 *paramCount,
                                                     qint32 *bufferSize,
                                                     qint32 *length,
                                                     char *nameBuf)
{
    GLint bufferIndex = GL_INVALID_INDEX;

    Q_ASSERT(po);
    Q_ASSERT(length);
    Q_ASSERT(nameBuf);
    Q_ASSERT(bufferSize);
    Q_ASSERT(paramCount);

    QDemonRenderBackendShaderProgramGL *pProgram = reinterpret_cast<QDemonRenderBackendShaderProgramGL *>(po);
    GLuint programID = static_cast<GLuint>(pProgram->m_programID);
#if defined(GL_VERSION_4_3) || defined (QT_OPENGL_ES_3_1)
    if (m_backendSupport.caps.bits.bProgramInterfaceSupported) {
        GL_CALL_EXTRA_FUNCTION(glGetProgramResourceName(programID, GL_SHADER_STORAGE_BLOCK, id, nameBufSize,
                                                        length, nameBuf));

        if (*length > 0) {
#define QUERY_COUNT 3
            GLsizei actualCount;
            GLenum props[QUERY_COUNT] = { GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE,
                                          GL_NUM_ACTIVE_VARIABLES };
            GLint params[QUERY_COUNT];
            GL_CALL_EXTRA_FUNCTION(glGetProgramResourceiv(programID, GL_SHADER_STORAGE_BLOCK, id,
                                                          QUERY_COUNT, props, QUERY_COUNT, &actualCount,
                                                          params));

            Q_ASSERT(actualCount == QUERY_COUNT);

            bufferIndex = params[0];
            *bufferSize = params[1];
            *paramCount = params[2];
        }
    }
#endif
    return bufferIndex;
}

void QDemonRenderBackendGL4Impl::programSetStorageBuffer(quint32 index,
                                                         QDemonRenderBackendBufferObject bo)
{
#if defined(GL_VERSION_4_3) || defined (QT_OPENGL_ES_3_1)
    GL_CALL_EXTRA_FUNCTION(
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, HandleToID_cast(GLuint, size_t, bo)));
#endif
}

qint32 QDemonRenderBackendGL4Impl::getAtomicCounterBufferCount(QDemonRenderBackendShaderProgramObject po)
{
    GLint numAtomicCounterBuffers = 0;
    Q_ASSERT(po);
    QDemonRenderBackendShaderProgramGL *pProgram = reinterpret_cast<QDemonRenderBackendShaderProgramGL *>(po);
    GLuint programID = static_cast<GLuint>(pProgram->m_programID);
#if defined(GL_VERSION_4_3) || defined (QT_OPENGL_ES_3_1)
    if (m_backendSupport.caps.bits.bProgramInterfaceSupported)
        GL_CALL_EXTRA_FUNCTION(glGetProgramInterfaceiv(programID, GL_ATOMIC_COUNTER_BUFFER,
                                                       GL_ACTIVE_RESOURCES, &numAtomicCounterBuffers));
#endif
    return numAtomicCounterBuffers;
}

qint32
QDemonRenderBackendGL4Impl::getAtomicCounterBufferInfoByID(QDemonRenderBackendShaderProgramObject po,
                                                           quint32 id,
                                                           quint32 nameBufSize,
                                                           qint32 *paramCount,
                                                           qint32 *bufferSize,
                                                           qint32 *length,
                                                           char *nameBuf)
{
    GLint bufferIndex = GL_INVALID_INDEX;

    Q_ASSERT(po);
    Q_ASSERT(length);
    Q_ASSERT(nameBuf);
    Q_ASSERT(bufferSize);
    Q_ASSERT(paramCount);

    QDemonRenderBackendShaderProgramGL *pProgram = reinterpret_cast<QDemonRenderBackendShaderProgramGL *>(po);
    GLuint programID = static_cast<GLuint>(pProgram->m_programID);
#if defined(GL_VERSION_4_3) || defined (QT_OPENGL_ES_3_1)
    if (m_backendSupport.caps.bits.bProgramInterfaceSupported) {
        {
#define QUERY_COUNT 3
            GLsizei actualCount;
            GLenum props[QUERY_COUNT] = { GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE,
                                          GL_NUM_ACTIVE_VARIABLES };
            GLint params[QUERY_COUNT];
            GL_CALL_EXTRA_FUNCTION(glGetProgramResourceiv(programID, GL_ATOMIC_COUNTER_BUFFER, id,
                                                          QUERY_COUNT, props, QUERY_COUNT, &actualCount,
                                                          params));

            Q_ASSERT(actualCount == QUERY_COUNT);

            bufferIndex = params[0];
            *bufferSize = params[1];
            *paramCount = params[2];

            GLenum props1[1] = { GL_ATOMIC_COUNTER_BUFFER_INDEX };
            GL_CALL_EXTRA_FUNCTION(glGetProgramResourceiv(programID, GL_UNIFORM, id, 1, props1, 1,
                                                          &actualCount, params));

            Q_ASSERT(actualCount == 1);

            *nameBuf = '\0';
            GL_CALL_EXTRA_FUNCTION(glGetProgramResourceName(programID, GL_UNIFORM, params[0], nameBufSize,
                                   length, nameBuf));
        }
    }
#endif
    return bufferIndex;
}

void QDemonRenderBackendGL4Impl::programSetAtomicCounterBuffer(quint32 index,
                                                               QDemonRenderBackendBufferObject bo)
{
#if defined(GL_VERSION_4_3) || defined (QT_OPENGL_ES_3_1)
    GL_CALL_EXTRA_FUNCTION(
                glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, index, HandleToID_cast(GLuint, size_t, bo)));
#endif
}

void QDemonRenderBackendGL4Impl::setConstantValue(QDemonRenderBackendShaderProgramObject po,
                                                  quint32 id,
                                                  QDemonRenderShaderDataTypes::Enum type,
                                                  qint32 count,
                                                  const void *value,
                                                  bool transpose)
{
    QDemonRenderBackendShaderProgramGL *pProgram = reinterpret_cast<QDemonRenderBackendShaderProgramGL *>(po);
    GLuint programID = static_cast<GLuint>(pProgram->m_programID);

    GLenum glType = GLConversion::fromPropertyDataTypesToShaderGL(type);

    switch (glType) {
    case GL_FLOAT:
        GL_CALL_EXTRA_FUNCTION(glProgramUniform1fv(programID, id, count, (GLfloat *)value));
        break;
    case GL_FLOAT_VEC2:
        GL_CALL_EXTRA_FUNCTION(glProgramUniform2fv(programID, id, count, (GLfloat *)value));
        break;
    case GL_FLOAT_VEC3:
        GL_CALL_EXTRA_FUNCTION(glProgramUniform3fv(programID, id, count, (GLfloat *)value));
        break;
    case GL_FLOAT_VEC4:
        GL_CALL_EXTRA_FUNCTION(glProgramUniform4fv(programID, id, count, (GLfloat *)value));
        break;
    case GL_INT:
        GL_CALL_EXTRA_FUNCTION(glProgramUniform1iv(programID, id, count, (GLint *)value));
        break;
    case GL_BOOL:
    {
        // Cast int value to be 0 or 1, matching to bool
        GLint *boolValue = (GLint *)value;
        *boolValue = *(GLboolean *)value;
        GL_CALL_EXTRA_FUNCTION(glProgramUniform1iv(programID, id, count, boolValue));
    }
        break;
    case GL_INT_VEC2:
    case GL_BOOL_VEC2:
        GL_CALL_EXTRA_FUNCTION(glProgramUniform2iv(programID, id, count, (GLint *)value));
        break;
    case GL_INT_VEC3:
    case GL_BOOL_VEC3:
        GL_CALL_EXTRA_FUNCTION(glProgramUniform3iv(programID, id, count, (GLint *)value));
        break;
    case GL_INT_VEC4:
    case GL_BOOL_VEC4:
        GL_CALL_EXTRA_FUNCTION(glProgramUniform4iv(programID, id, count, (GLint *)value));
        break;
    case GL_FLOAT_MAT3:
        GL_CALL_EXTRA_FUNCTION(
                    glProgramUniformMatrix3fv(programID, id, count, transpose, (GLfloat *)value));
        break;
    case GL_FLOAT_MAT4:
        GL_CALL_EXTRA_FUNCTION(
                    glProgramUniformMatrix4fv(programID, id, count, transpose, (GLfloat *)value));
        break;
    case GL_IMAGE_2D:
    case GL_SAMPLER_2D:
    case GL_SAMPLER_2D_ARRAY:
    case GL_SAMPLER_2D_SHADOW:
    case GL_SAMPLER_CUBE: {
        if (count <= 1) {
            GLint sampler = *(GLint *)value;
            GL_CALL_EXTRA_FUNCTION(glProgramUniform1i(programID, id, sampler));
        } else {
            GLint *sampler = (GLint *)value;
            GL_CALL_EXTRA_FUNCTION(glProgramUniform1iv(programID, id, count, sampler));
        }
    } break;
    default:
        qCCritical(INTERNAL_ERROR, "Unknown shader type format %d", type);
        Q_ASSERT(false);
        break;
    }
}

QDemonRenderBackend::QDemonRenderBackendComputeShaderObject
QDemonRenderBackendGL4Impl::createComputeShader(QDemonConstDataRef<qint8> source,
                                                QByteArray &errorMessage, bool binary)
{
    GLuint shaderID = 0;
#if defined(GL_COMPUTE_SHADER)
    shaderID = m_glExtraFunctions->glCreateShader(GL_COMPUTE_SHADER);

    if (shaderID && !compileSource(shaderID, source, errorMessage, binary)) {
        GL_CALL_EXTRA_FUNCTION(glDeleteShader(shaderID));
        shaderID = 0;
    }
#endif
    return reinterpret_cast<QDemonRenderBackend::QDemonRenderBackendComputeShaderObject>(shaderID);
}

void QDemonRenderBackendGL4Impl::dispatchCompute(QDemonRenderBackendShaderProgramObject,
                                                 quint32 numGroupsX,
                                                 quint32 numGroupsY,
                                                 quint32 numGroupsZ)
{
    GL_CALL_EXTRA_FUNCTION(glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ));
}

QDemonRenderBackend::QDemonRenderBackendProgramPipeline QDemonRenderBackendGL4Impl::createProgramPipeline()
{
    GLuint pipeline;
    GL_CALL_EXTRA_FUNCTION(glGenProgramPipelines(1, &pipeline));

    return reinterpret_cast<QDemonRenderBackend::QDemonRenderBackendProgramPipeline>(pipeline);
}

void QDemonRenderBackendGL4Impl::releaseProgramPipeline(QDemonRenderBackendProgramPipeline ppo)
{
    GLuint pipeline = HandleToID_cast(GLuint, size_t, ppo);
    GL_CALL_EXTRA_FUNCTION(glDeleteProgramPipelines(1, &pipeline));
}

void QDemonRenderBackendGL4Impl::setActiveProgramPipeline(QDemonRenderBackendProgramPipeline ppo)
{
    GLuint pipeline = HandleToID_cast(GLuint, size_t, ppo);

    GL_CALL_EXTRA_FUNCTION(glBindProgramPipeline(pipeline));
}

void QDemonRenderBackendGL4Impl::setProgramStages(QDemonRenderBackendProgramPipeline ppo,
                                                  QDemonRenderShaderTypeFlags flags,
                                                  QDemonRenderBackendShaderProgramObject po)
{
    GLuint pipeline = HandleToID_cast(GLuint, size_t, ppo);
    GLuint programID = 0;

    if (po) {
        QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
        programID = static_cast<GLuint>(pProgram->m_programID);
    }

    GL_CALL_EXTRA_FUNCTION(
                glUseProgramStages(pipeline, m_conversion.fromShaderTypeFlagsToGL(flags), programID));
}

void QDemonRenderBackendGL4Impl::setBlendEquation(const QDemonRenderBlendEquationArgument &pBlendEquArg)
{
    if (m_backendSupport.caps.bits.bNVAdvancedBlendSupported ||
            m_backendSupport.caps.bits.bKHRAdvancedBlendSupported)
        GL_CALL_EXTRA_FUNCTION(glBlendEquation(m_conversion.fromBlendEquationToGL(
                                                   pBlendEquArg.m_rgbEquation, m_backendSupport.caps.bits.bNVAdvancedBlendSupported,
                                                   m_backendSupport.caps.bits.bKHRAdvancedBlendSupported)));
}

void QDemonRenderBackendGL4Impl::setBlendBarrier(void)
{
    if (m_backendSupport.caps.bits.bNVAdvancedBlendSupported)
        GL_CALL_QDEMON_EXT(glBlendBarrierNV());
}

QDemonRenderBackend::QDemonRenderBackendPathObject
QDemonRenderBackendGL4Impl::createPathNVObject(size_t range)
{
    GLuint pathID = GL_CALL_NVPATH_EXT(glGenPathsNV(GLsizei(range)));

    return QDemonRenderBackend::QDemonRenderBackendPathObject(pathID);
}
void QDemonRenderBackendGL4Impl::setPathSpecification(QDemonRenderBackendPathObject inPathObject,
                                                      QDemonConstDataRef<quint8> inPathCommands,
                                                      QDemonConstDataRef<float> inPathCoords)
{
    GLuint pathID = HandleToID_cast(GLuint, size_t, inPathObject);
    GL_CALL_NVPATH_EXT(glPathCommandsNV(pathID, inPathCommands.size(), inPathCommands.begin(),
                                        inPathCoords.size(), GL_FLOAT, inPathCoords.begin()));
}

QDemonBounds3
QDemonRenderBackendGL4Impl::getPathObjectBoundingBox(QDemonRenderBackendPathObject inPathObject)
{
    float data[4];
#if defined(GL_NV_path_rendering)
    GL_CALL_NVPATH_EXT(glGetPathParameterfvNV(
                           HandleToID_cast(GLuint, size_t, inPathObject),
                           GL_PATH_OBJECT_BOUNDING_BOX_NV, data));
#endif
    return QDemonBounds3(QVector3D(data[0], data[1], 0.0f), QVector3D(data[2], data[3], 0.0f));
}

QDemonBounds3 QDemonRenderBackendGL4Impl::getPathObjectFillBox(QDemonRenderBackendPathObject inPathObject)
{
    float data[4];
#if defined(GL_NV_path_rendering)
    GL_CALL_NVPATH_EXT(glGetPathParameterfvNV(
                           HandleToID_cast(GLuint, size_t, inPathObject),
                           GL_PATH_FILL_BOUNDING_BOX_NV, data));
#endif
    return QDemonBounds3(QVector3D(data[0], data[1], 0.0f), QVector3D(data[2], data[3], 0.0f));
}

QDemonBounds3 QDemonRenderBackendGL4Impl::getPathObjectStrokeBox(QDemonRenderBackendPathObject inPathObject)
{
    float data[4];
#if defined(GL_NV_path_rendering)
    GL_CALL_NVPATH_EXT(glGetPathParameterfvNV(
                           HandleToID_cast(GLuint, size_t, inPathObject),
                           GL_PATH_STROKE_BOUNDING_BOX_NV, data));
#endif
    return QDemonBounds3(QVector3D(data[0], data[1], 0.0f), QVector3D(data[2], data[3], 0.0f));
}

void QDemonRenderBackendGL4Impl::setStrokeWidth(QDemonRenderBackendPathObject inPathObject,
                                                float inStrokeWidth)
{
#if defined(GL_NV_path_rendering)
    GL_CALL_NVPATH_EXT(glPathParameterfNV(HandleToID_cast(GLuint, size_t, inPathObject),
                                          GL_PATH_STROKE_WIDTH_NV, inStrokeWidth));
#endif
}

void QDemonRenderBackendGL4Impl::setPathProjectionMatrix(const QMatrix4x4 inPathProjection)
{
#if defined(QT_OPENGL_ES)
    NVRENDER_BACKEND_UNUSED(inPathProjection);
#else
    GL_CALL_DIRECTSTATE_EXT(glMatrixLoadfEXT(GL_PROJECTION, inPathProjection.constData()));
#endif
}

void QDemonRenderBackendGL4Impl::setPathModelViewMatrix(const QMatrix4x4 inPathModelview)
{
#if defined(QT_OPENGL_ES)
    NVRENDER_BACKEND_UNUSED(inPathModelview);
#else
    GL_CALL_DIRECTSTATE_EXT(glMatrixLoadfEXT(GL_MODELVIEW, inPathModelview.constData()));
#endif
}

void QDemonRenderBackendGL4Impl::setPathStencilDepthOffset(float inSlope, float inBias)
{
    GL_CALL_NVPATH_EXT(glPathStencilDepthOffsetNV(inSlope, inBias));
}

void QDemonRenderBackendGL4Impl::setPathCoverDepthFunc(QDemonRenderBoolOp::Enum inDepthFunction)
{
    GL_CALL_NVPATH_EXT(glPathCoverDepthFuncNV(m_conversion.fromBoolOpToGL(inDepthFunction)));
}

void QDemonRenderBackendGL4Impl::stencilStrokePath(QDemonRenderBackendPathObject inPathObject)
{
    GL_CALL_NVPATH_EXT(glStencilStrokePathNV(HandleToID_cast(GLuint, size_t, inPathObject), 0x1, (GLuint)~0));
}

void QDemonRenderBackendGL4Impl::stencilFillPath(QDemonRenderBackendPathObject inPathObject)
{
#if defined(GL_NV_path_rendering)
    GL_CALL_NVPATH_EXT(glStencilFillPathNV(HandleToID_cast(GLuint, size_t, inPathObject),
                                           GL_COUNT_UP_NV, (GLuint)~0));
#endif
}

void QDemonRenderBackendGL4Impl::releasePathNVObject(QDemonRenderBackendPathObject po, size_t range)
{
    GLuint pathID = HandleToID_cast(GLuint, size_t, po);

    GL_CALL_NVPATH_EXT(glDeletePathsNV(pathID, (GLsizei)range));
}

void QDemonRenderBackendGL4Impl::stencilFillPathInstanced(
        QDemonRenderBackendPathObject po, size_t numPaths, QDemonRenderPathFormatType::Enum type,
        const void *charCodes, QDemonRenderPathFillMode::Enum fillMode, quint32 stencilMask,
        QDemonRenderPathTransformType::Enum transformType, const float *transformValues)
{
    GLuint pathID = HandleToID_cast(GLuint, size_t, po);

    GL_CALL_NVPATH_EXT(glStencilFillPathInstancedNV(
                           (GLsizei)numPaths, m_conversion.fromPathTypeToGL(type), charCodes, pathID,
                           m_conversion.fromPathFillModeToGL(fillMode), stencilMask,
                           m_conversion.fromPathTransformToGL(transformType), transformValues));
}

void QDemonRenderBackendGL4Impl::stencilStrokePathInstancedN(
        QDemonRenderBackendPathObject po, size_t numPaths, QDemonRenderPathFormatType::Enum type,
        const void *charCodes, qint32 stencilRef, quint32 stencilMask,
        QDemonRenderPathTransformType::Enum transformType, const float *transformValues)
{
    GLuint pathID = HandleToID_cast(GLuint, size_t, po);

    GL_CALL_NVPATH_EXT(glStencilStrokePathInstancedNV(
                           (GLsizei)numPaths, m_conversion.fromPathTypeToGL(type), charCodes, pathID, stencilRef,
                           stencilMask, m_conversion.fromPathTransformToGL(transformType), transformValues));
}

void QDemonRenderBackendGL4Impl::coverFillPathInstanced(
        QDemonRenderBackendPathObject po, size_t numPaths, QDemonRenderPathFormatType::Enum type,
        const void *charCodes, QDemonRenderPathCoverMode::Enum coverMode,
        QDemonRenderPathTransformType::Enum transformType, const float *transformValues)
{
    GLuint pathID = HandleToID_cast(GLuint, size_t, po);

    GL_CALL_NVPATH_EXT(glCoverFillPathInstancedNV(
                           (GLsizei)numPaths, m_conversion.fromPathTypeToGL(type), charCodes, pathID,
                           m_conversion.fromPathCoverModeToGL(coverMode),
                           m_conversion.fromPathTransformToGL(transformType), transformValues));
}

void QDemonRenderBackendGL4Impl::coverStrokePathInstanced(
        QDemonRenderBackendPathObject po, size_t numPaths, QDemonRenderPathFormatType::Enum type,
        const void *charCodes, QDemonRenderPathCoverMode::Enum coverMode,
        QDemonRenderPathTransformType::Enum transformType, const float *transformValues)
{
    GLuint pathID = HandleToID_cast(GLuint, size_t, po);

    GL_CALL_NVPATH_EXT(glCoverStrokePathInstancedNV(
                           (GLsizei)numPaths, m_conversion.fromPathTypeToGL(type), charCodes, pathID,
                           m_conversion.fromPathCoverModeToGL(coverMode),
                           m_conversion.fromPathTransformToGL(transformType), transformValues));
}

void QDemonRenderBackendGL4Impl::loadPathGlyphs(
        QDemonRenderBackendPathObject po, QDemonRenderPathFontTarget::Enum fontTarget, const void *fontName,
        QDemonRenderPathFontStyleFlags fontStyle, size_t numGlyphs, QDemonRenderPathFormatType::Enum type,
        const void *charCodes, QDemonRenderPathMissingGlyphs::Enum handleMissingGlyphs,
        QDemonRenderBackendPathObject pathParameterTemplate, float emScale)
{
    GLuint pathID = HandleToID_cast(GLuint, size_t, po);
    GLuint pathTemplateID = (pathParameterTemplate == nullptr)
            ? ~0
            : HandleToID_cast(GLuint, size_t, pathParameterTemplate);

    GL_CALL_NVPATH_EXT(glPathGlyphsNV(pathID, m_conversion.fromPathFontTargetToGL(fontTarget), fontName,
                                      m_conversion.fromPathFontStyleToGL(fontStyle), (GLsizei)numGlyphs,
                                      m_conversion.fromPathTypeToGL(type), charCodes,
                                      m_conversion.fromPathMissingGlyphsToGL(handleMissingGlyphs),
                                      pathTemplateID, emScale));
}

QDemonRenderPathReturnValues::Enum QDemonRenderBackendGL4Impl::loadPathGlyphsIndexed(
        QDemonRenderBackendPathObject po, QDemonRenderPathFontTarget::Enum fontTarget, const void *fontName,
        QDemonRenderPathFontStyleFlags fontStyle, quint32 firstGlyphIndex, size_t numGlyphs,
        QDemonRenderBackendPathObject pathParameterTemplate, float emScale)
{
    GLuint pathID = HandleToID_cast(GLuint, size_t, po);
    GLuint pathTemplateID = (pathParameterTemplate == nullptr)
            ? ~0
            : HandleToID_cast(GLuint, size_t, pathParameterTemplate);
    GLenum glRet = 0;

    glRet = GL_CALL_QDEMON_EXT(glPathGlyphIndexArrayNV(
                                   pathID, m_conversion.fromPathFontTargetToGL(fontTarget), fontName,
                                   m_conversion.fromPathFontStyleToGL(fontStyle), firstGlyphIndex,
                                   (GLsizei)numGlyphs, pathTemplateID, emScale));

    return GLConversion::fromGLToPathFontReturn(glRet);
}

QDemonRenderBackend::QDemonRenderBackendPathObject QDemonRenderBackendGL4Impl::loadPathGlyphsIndexedRange(
        QDemonRenderPathFontTarget::Enum fontTarget, const void *fontName,
        QDemonRenderPathFontStyleFlags fontStyle, QDemonRenderBackendPathObject pathParameterTemplate,
        float emScale, quint32 *count)
{
    GLuint pathTemplateID = (pathParameterTemplate == nullptr)
            ? ~0
            : HandleToID_cast(GLuint, size_t, pathParameterTemplate);
    GLenum glRet = 0;
    GLuint baseAndCount[2] = { 0, 0 };

    glRet = GL_CALL_QDEMON_EXT(glPathGlyphIndexRangeNV(m_conversion.fromPathFontTargetToGL(fontTarget),
                                                       fontName,
                                                       m_conversion.fromPathFontStyleToGL(fontStyle),
                                                       pathTemplateID, emScale, baseAndCount));

    if (count)
        *count = baseAndCount[1];

    return QDemonRenderBackend::QDemonRenderBackendPathObject(baseAndCount[0]);
}

void QDemonRenderBackendGL4Impl::loadPathGlyphRange(
        QDemonRenderBackendPathObject po, QDemonRenderPathFontTarget::Enum fontTarget, const void *fontName,
        QDemonRenderPathFontStyleFlags fontStyle, quint32 firstGlyph, size_t numGlyphs,
        QDemonRenderPathMissingGlyphs::Enum handleMissingGlyphs,
        QDemonRenderBackendPathObject pathParameterTemplate, float emScale)
{
    GLuint pathID = HandleToID_cast(GLuint, size_t, po);
    GLuint pathTemplateID = (pathParameterTemplate == nullptr)
            ? ~GLuint(0)
            : HandleToID_cast(GLuint, size_t, pathParameterTemplate);

    GL_CALL_NVPATH_EXT(glPathGlyphRangeNV(
                           pathID, m_conversion.fromPathFontTargetToGL(fontTarget), fontName,
                           m_conversion.fromPathFontStyleToGL(fontStyle), firstGlyph, (GLsizei)numGlyphs,
                           m_conversion.fromPathMissingGlyphsToGL(handleMissingGlyphs), pathTemplateID, emScale));
}

void QDemonRenderBackendGL4Impl::getPathMetrics(QDemonRenderBackendPathObject po, size_t numPaths,
                                                QDemonRenderPathGlyphFontMetricFlags metricQueryMask,
                                                QDemonRenderPathFormatType::Enum type,
                                                const void *charCodes, size_t stride,
                                                float *metrics)
{
    GLuint pathID = HandleToID_cast(GLuint, size_t, po);

    GL_CALL_NVPATH_EXT(glGetPathMetricsNV(m_conversion.fromPathMetricQueryFlagsToGL(metricQueryMask),
                                          (GLsizei)numPaths, m_conversion.fromPathTypeToGL(type),
                                          charCodes, pathID, (GLsizei)stride, metrics));
}

void
QDemonRenderBackendGL4Impl::getPathMetricsRange(QDemonRenderBackendPathObject po, size_t numPaths,
                                                QDemonRenderPathGlyphFontMetricFlags metricQueryMask,
                                                size_t stride, float *metrics)
{
    GLuint pathID = HandleToID_cast(GLuint, size_t, po);

    GL_CALL_NVPATH_EXT(glGetPathMetricRangeNV(m_conversion.fromPathMetricQueryFlagsToGL(metricQueryMask),
                                              pathID, (GLsizei)numPaths, (GLsizei)stride, metrics));
}

void QDemonRenderBackendGL4Impl::getPathSpacing(QDemonRenderBackendPathObject po,
                                                size_t numPaths,
                                                QDemonRenderPathListMode::Enum pathListMode,
                                                QDemonRenderPathFormatType::Enum type,
                                                const void *charCodes,
                                                float advanceScale,
                                                float kerningScale,
                                                QDemonRenderPathTransformType::Enum transformType,
                                                float *spacing)
{
    GLuint pathID = HandleToID_cast(GLuint, size_t, po);

    GL_CALL_NVPATH_EXT(glGetPathSpacingNV(m_conversion.fromPathListModeToGL(pathListMode),
                                          (GLsizei)numPaths, m_conversion.fromPathTypeToGL(type),
                                          charCodes, pathID, advanceScale, kerningScale,
                                          m_conversion.fromPathTransformToGL(transformType), spacing));
}

QT_END_NAMESPACE
