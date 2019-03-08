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

#include <QtDemonRender/qdemonrenderbackendglbase.h>
#include <QtDemonRender/qdemonrenderbackendinputassemblergl.h>
#include <QtDemonRender/qdemonrenderbackendshaderprogramgl.h>
#include <QtDemonRender/qdemonrenderbackendrenderstatesgl.h>

QT_BEGIN_NAMESPACE

#ifdef RENDER_BACKEND_LOG_GL_ERRORS
#define RENDER_LOG_ERROR_PARAMS(x) checkGLError(#x, __FILE__, __LINE__)
#else
#define RENDER_LOG_ERROR_PARAMS(x) checkGLError()
#endif

#define GL_CALL_FUNCTION(x)                                                                                            \
    m_glFunctions->x;                                                                                                  \
    RENDER_LOG_ERROR_PARAMS(x);
#define GL_CALL_EXTRA_FUNCTION(x)                                                                                      \
    m_glExtraFunctions->x;                                                                                             \
    RENDER_LOG_ERROR_PARAMS(x);

#ifndef GL_PROGRAM_SEPARABLE
#define GL_PROGRAM_SEPARABLE 0x8258
#endif

#ifndef GL_UNSIGNED_INT_IMAGE_2D
#define GL_UNSIGNED_INT_IMAGE_2D 0x9063
#endif

#ifndef GL_UNSIGNED_INT_ATOMIC_COUNTER
#define GL_UNSIGNED_INT_ATOMIC_COUNTER 0x92DB
#endif

namespace QDemonGlExtStrings {
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
}

/// constructor
QDemonRenderBackendGLBase::QDemonRenderBackendGLBase(const QSurfaceFormat &format)
    : m_conversion(), m_maxAttribCount(0), m_format(format)
{
    m_glFunctions = new QOpenGLFunctions;
    m_glFunctions->initializeOpenGLFunctions();
    m_glExtraFunctions = new QOpenGLExtraFunctions;
    m_glExtraFunctions->initializeOpenGLFunctions();

    // internal state tracker
    m_currentRasterizerState = new QDemonRenderBackendRasterizerStateGL();
    m_currentDepthStencilState = new QDemonRenderBackendDepthStencilStateGL();
}
/// destructor
QDemonRenderBackendGLBase::~QDemonRenderBackendGLBase()
{
    delete m_currentRasterizerState;
    delete m_currentDepthStencilState;
    delete m_glFunctions;
    delete m_glExtraFunctions;
}

QDemonRenderContextType QDemonRenderBackendGLBase::getRenderContextType() const
{
    if (m_format.renderableType() == QSurfaceFormat::OpenGLES) {
        if (m_format.majorVersion() == 2)
            return QDemonRenderContextType::GLES2;

        if (m_format.majorVersion() == 3) {
            if (m_format.minorVersion() >= 1)
                return QDemonRenderContextType::GLES3PLUS;
            else
                return QDemonRenderContextType::GLES3;
        }
    } else if (m_format.majorVersion() == 2) {
        return QDemonRenderContextType::GL2;
    } else if (m_format.majorVersion() == 3) {
        return QDemonRenderContextType::GL3;
    } else if (m_format.majorVersion() == 4) {
        return QDemonRenderContextType::GL4;
    }

    return QDemonRenderContextType::NullContext;
}

bool QDemonRenderBackendGLBase::isESCompatible() const
{
    return m_format.renderableType() == QSurfaceFormat::OpenGLES;
}

const char *QDemonRenderBackendGLBase::getShadingLanguageVersion()
{
    const char *retval = (const char *)GL_CALL_FUNCTION(glGetString(GL_SHADING_LANGUAGE_VERSION));
    if (retval == nullptr)
        return "";

    return retval;
}

qint32 QDemonRenderBackendGLBase::getMaxCombinedTextureUnits()
{
    qint32 maxUnits;
    GL_CALL_FUNCTION(glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxUnits));
    return maxUnits;
}

bool QDemonRenderBackendGLBase::getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps inCap) const
{
    bool bSupported = false;

    switch (inCap) {
    case QDemonRenderBackendCaps::FpRenderTarget:
        bSupported = m_backendSupport.caps.bits.bFPRenderTargetsSupported;
        break;
    case QDemonRenderBackendCaps::DepthStencilTexture:
        bSupported = m_backendSupport.caps.bits.bDepthStencilSupported;
        break;
    case QDemonRenderBackendCaps::ConstantBuffer:
        bSupported = m_backendSupport.caps.bits.bConstantBufferSupported;
        break;
    case QDemonRenderBackendCaps::DxtImages:
        bSupported = m_backendSupport.caps.bits.bDXTImagesSupported;
        break;
    case QDemonRenderBackendCaps::MsTexture:
        bSupported = m_backendSupport.caps.bits.bMsTextureSupported;
        break;
    case QDemonRenderBackendCaps::TexSwizzle:
        bSupported = m_backendSupport.caps.bits.bTextureSwizzleSupported;
        break;
    case QDemonRenderBackendCaps::FastBlits:
        bSupported = m_backendSupport.caps.bits.bFastBlitsSupported;
        break;
    case QDemonRenderBackendCaps::Tessellation:
        bSupported = m_backendSupport.caps.bits.bTessellationSupported;
        break;
    case QDemonRenderBackendCaps::Compute:
        bSupported = m_backendSupport.caps.bits.bComputeSupported;
        break;
    case QDemonRenderBackendCaps::Geometry:
        bSupported = m_backendSupport.caps.bits.bGeometrySupported;
        break;
    case QDemonRenderBackendCaps::SampleQuery: {
        // On the following context sample query is not supported
        QDemonRenderContextTypes noSamplesQuerySupportedContextFlags(QDemonRenderContextType::GL2 | QDemonRenderContextType::GLES2);
        QDemonRenderContextType ctxType = getRenderContextType();
        bSupported = !(noSamplesQuerySupportedContextFlags & ctxType);
    } break;
    case QDemonRenderBackendCaps::TimerQuery:
        bSupported = m_backendSupport.caps.bits.bTimerQuerySupported;
        break;
    case QDemonRenderBackendCaps::CommandSync: {
        // On the following context sync objects are not supported
        QDemonRenderContextTypes noSyncObjectSupportedContextFlags(QDemonRenderContextType::GL2 | QDemonRenderContextType::GLES2);
        QDemonRenderContextType ctxType = getRenderContextType();
        bSupported = !(noSyncObjectSupportedContextFlags & ctxType);
    } break;
    case QDemonRenderBackendCaps::TextureArray: {
        // On the following context texture arrays are not supported
        QDemonRenderContextTypes noTextureArraySupportedContextFlags(QDemonRenderContextType::GL2 | QDemonRenderContextType::GLES2);
        QDemonRenderContextType ctxType = getRenderContextType();
        bSupported = !(noTextureArraySupportedContextFlags& ctxType);
    } break;
    case QDemonRenderBackendCaps::StorageBuffer:
        bSupported = m_backendSupport.caps.bits.bStorageBufferSupported;
        break;
    case QDemonRenderBackendCaps::AtomicCounterBuffer:
        bSupported = m_backendSupport.caps.bits.bAtomicCounterBufferSupported;
        break;
    case QDemonRenderBackendCaps::ShaderImageLoadStore:
        bSupported = m_backendSupport.caps.bits.bShaderImageLoadStoreSupported;
        break;
    case QDemonRenderBackendCaps::ProgramPipeline:
        bSupported = m_backendSupport.caps.bits.bProgramPipelineSupported;
        break;
    case QDemonRenderBackendCaps::PathRendering:
        bSupported = m_backendSupport.caps.bits.bNVPathRenderingSupported;
        break;
    case QDemonRenderBackendCaps::AdvancedBlend:
        bSupported = m_backendSupport.caps.bits.bNVAdvancedBlendSupported | m_backendSupport.caps.bits.bKHRAdvancedBlendSupported;
        break;
    case QDemonRenderBackendCaps::AdvancedBlendKHR:
        bSupported = m_backendSupport.caps.bits.bKHRAdvancedBlendSupported;
        break;
    case QDemonRenderBackendCaps::BlendCoherency:
        bSupported = m_backendSupport.caps.bits.bNVBlendCoherenceSupported | m_backendSupport.caps.bits.bKHRBlendCoherenceSupported;
        break;
    case QDemonRenderBackendCaps::gpuShader5:
        bSupported = m_backendSupport.caps.bits.bGPUShader5ExtensionSupported;
        break;
    case QDemonRenderBackendCaps::VertexArrayObject:
        bSupported = m_backendSupport.caps.bits.bVertexArrayObjectSupported;
        break;
    case QDemonRenderBackendCaps::StandardDerivatives:
        bSupported = m_backendSupport.caps.bits.bStandardDerivativesSupported;
        break;
    case QDemonRenderBackendCaps::TextureLod:
        bSupported = m_backendSupport.caps.bits.bTextureLodSupported;
        break;
    default:
        Q_ASSERT(false);
        bSupported = false;
        break;
    }

    return bSupported;
}

void QDemonRenderBackendGLBase::getRenderBackendValue(QDemonRenderBackendQuery inQuery, qint32 *params) const
{
    if (params) {
        switch (inQuery) {
        case QDemonRenderBackendQuery::MaxTextureSize:
            GL_CALL_FUNCTION(glGetIntegerv(GL_MAX_TEXTURE_SIZE, params));
            break;
        case QDemonRenderBackendQuery::MaxTextureArrayLayers: {
            QDemonRenderContextTypes noTextureArraySupportedContextFlags(QDemonRenderContextType::GL2
                                                                        | QDemonRenderContextType::GLES2);
            QDemonRenderContextType ctxType = getRenderContextType();
            if (!(noTextureArraySupportedContextFlags & ctxType)) {
                GL_CALL_FUNCTION(glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, params));
            } else {
                *params = 0;
            }
        } break;
        case QDemonRenderBackendQuery::MaxConstantBufferSlots: {
            QDemonRenderContextTypes noConstantBufferSupportedContextFlags(QDemonRenderContextType::GL2
                                                                          | QDemonRenderContextType::GLES2);
            QDemonRenderContextType ctxType = getRenderContextType();
            if (!(noConstantBufferSupportedContextFlags & ctxType)) {
                GL_CALL_FUNCTION(glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, params));
            } else {
                *params = 0;
            }
        } break;
        case QDemonRenderBackendQuery::MaxConstantBufferBlockSize: {
            QDemonRenderContextTypes noConstantBufferSupportedContextFlags(QDemonRenderContextType::GL2
                                                                          | QDemonRenderContextType::GLES2);
            QDemonRenderContextType ctxType = getRenderContextType();
            if (!(noConstantBufferSupportedContextFlags & ctxType)) {
                GL_CALL_FUNCTION(glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, params));
            } else {
                *params = 0;
            }
        } break;
        default:
            Q_ASSERT(false);
            *params = 0;
            break;
        }
    }
}

qint32 QDemonRenderBackendGLBase::getDepthBits() const
{
    qint32 depthBits;
    GL_CALL_FUNCTION(glGetIntegerv(GL_DEPTH_BITS, &depthBits));
    return depthBits;
}

qint32 QDemonRenderBackendGLBase::getStencilBits() const
{
    qint32 stencilBits;
    GL_CALL_FUNCTION(glGetIntegerv(GL_STENCIL_BITS, &stencilBits));
    return stencilBits;
}

void QDemonRenderBackendGLBase::setMultisample(bool bEnable)
{
    Q_ASSERT(m_backendSupport.caps.bits.bMsTextureSupported || !bEnable);
    // For GL ES explicit multisample enabling is not needed
    // and does not exist
    QDemonRenderContextTypes noMsaaEnableContextFlags(QDemonRenderContextType::GLES2 | QDemonRenderContextType::GLES3
                                                     | QDemonRenderContextType::GLES3PLUS);
    QDemonRenderContextType ctxType = getRenderContextType();
    if (!(noMsaaEnableContextFlags & ctxType)) {
        setRenderState(bEnable, QDemonRenderState::Multisample);
    }
}

void QDemonRenderBackendGLBase::setRenderState(bool bEnable, const QDemonRenderState value)
{
    if (value == QDemonRenderState::DepthWrite) {
        GL_CALL_FUNCTION(glDepthMask(bEnable));
    } else {
        if (bEnable) {
            GL_CALL_FUNCTION(glEnable(m_conversion.fromRenderStateToGL(value)));
        } else {
            GL_CALL_FUNCTION(glDisable(m_conversion.fromRenderStateToGL(value)));
        }
    }
}

QDemonRenderBackend::QDemonRenderBackendDepthStencilStateObject QDemonRenderBackendGLBase::createDepthStencilState(
        bool enableDepth,
        bool depthMask,
        QDemonRenderBoolOp depthFunc,
        bool enableStencil,
        QDemonRenderStencilFunctionArgument &stencilFuncFront,
        QDemonRenderStencilFunctionArgument &stencilFuncBack,
        QDemonRenderStencilOperationArgument &depthStencilOpFront,
        QDemonRenderStencilOperationArgument &depthStencilOpBack)
{
    QDemonRenderBackendDepthStencilStateGL *retval = new QDemonRenderBackendDepthStencilStateGL(enableDepth,
                                                                                                depthMask,
                                                                                                depthFunc,
                                                                                                enableStencil,
                                                                                                stencilFuncFront,
                                                                                                stencilFuncBack,
                                                                                                depthStencilOpFront,
                                                                                                depthStencilOpBack);

    return (QDemonRenderBackend::QDemonRenderBackendDepthStencilStateObject)retval;
}

void QDemonRenderBackendGLBase::releaseDepthStencilState(QDemonRenderBackendDepthStencilStateObject inDepthStencilState)
{
    QDemonRenderBackendDepthStencilStateGL *inputState = (QDemonRenderBackendDepthStencilStateGL *)inDepthStencilState;
    delete inputState;
}

QDemonRenderBackend::QDemonRenderBackendRasterizerStateObject QDemonRenderBackendGLBase::createRasterizerState(float depthBias,
                                                                                                               float depthScale,
                                                                                                               QDemonRenderFace cullFace)
{
    QDemonRenderBackendRasterizerStateGL *retval = new QDemonRenderBackendRasterizerStateGL(depthBias, depthScale, cullFace);

    return (QDemonRenderBackend::QDemonRenderBackendRasterizerStateObject)retval;
}

void QDemonRenderBackendGLBase::releaseRasterizerState(QDemonRenderBackendRasterizerStateObject rasterizerState)
{
    delete (QDemonRenderBackendRasterizerStateGL *)rasterizerState;
}

void QDemonRenderBackendGLBase::setDepthStencilState(QDemonRenderBackendDepthStencilStateObject inDepthStencilState)
{
    QDemonRenderBackendDepthStencilStateGL *inputState = (QDemonRenderBackendDepthStencilStateGL *)inDepthStencilState;
    if (inputState && !(*m_currentDepthStencilState == *inputState)) {
        // we check on a per single state base
        if (inputState->m_depthEnable != m_currentDepthStencilState->m_depthEnable) {
            setRenderState(inputState->m_depthEnable, QDemonRenderState::DepthTest);
            m_currentDepthStencilState->m_depthEnable = inputState->m_depthEnable;
        }
        if (inputState->m_stencilEnable != m_currentDepthStencilState->m_stencilEnable) {
            setRenderState(inputState->m_stencilEnable, QDemonRenderState::StencilTest);
            m_currentDepthStencilState->m_stencilEnable = inputState->m_stencilEnable;
        }

        if (inputState->m_depthMask != m_currentDepthStencilState->m_depthMask) {
            GL_CALL_FUNCTION(glDepthMask(inputState->m_depthMask));
            m_currentDepthStencilState->m_depthMask = inputState->m_depthMask;
        }

        if (inputState->m_depthFunc != m_currentDepthStencilState->m_depthFunc) {
            GL_CALL_FUNCTION(glDepthFunc(m_conversion.fromBoolOpToGL(inputState->m_depthFunc)));
            m_currentDepthStencilState->m_depthFunc = inputState->m_depthFunc;
        }

        if (!(inputState->m_depthStencilOpFront == m_currentDepthStencilState->m_depthStencilOpFront)) {
            GL_CALL_FUNCTION(
                    glStencilOpSeparate(GL_FRONT,
                                        m_conversion.fromStencilOpToGL(inputState->m_depthStencilOpFront.m_stencilFail),
                                        m_conversion.fromStencilOpToGL(inputState->m_depthStencilOpFront.m_depthFail),
                                        m_conversion.fromStencilOpToGL(inputState->m_depthStencilOpFront.m_depthPass)));
            m_currentDepthStencilState->m_depthStencilOpFront = inputState->m_depthStencilOpFront;
        }

        if (!(inputState->m_depthStencilOpBack == m_currentDepthStencilState->m_depthStencilOpBack)) {
            GL_CALL_FUNCTION(glStencilOpSeparate(GL_BACK,
                                                 m_conversion.fromStencilOpToGL(inputState->m_depthStencilOpBack.m_stencilFail),
                                                 m_conversion.fromStencilOpToGL(inputState->m_depthStencilOpBack.m_depthFail),
                                                 m_conversion.fromStencilOpToGL(inputState->m_depthStencilOpBack.m_depthPass)));
            m_currentDepthStencilState->m_depthStencilOpBack = inputState->m_depthStencilOpBack;
        }

        if (!(inputState->m_stencilFuncFront == m_currentDepthStencilState->m_stencilFuncFront)) {
            GL_CALL_FUNCTION(glStencilFuncSeparate(GL_FRONT,
                                                   m_conversion.fromBoolOpToGL(inputState->m_stencilFuncFront.m_function),
                                                   inputState->m_stencilFuncFront.m_referenceValue,
                                                   inputState->m_stencilFuncFront.m_mask));
            m_currentDepthStencilState->m_stencilFuncFront = inputState->m_stencilFuncFront;
        }

        if (!(inputState->m_stencilFuncBack == m_currentDepthStencilState->m_stencilFuncBack)) {
            GL_CALL_FUNCTION(glStencilFuncSeparate(GL_BACK,
                                                   m_conversion.fromBoolOpToGL(inputState->m_stencilFuncBack.m_function),
                                                   inputState->m_stencilFuncBack.m_referenceValue,
                                                   inputState->m_stencilFuncBack.m_mask));
            m_currentDepthStencilState->m_stencilFuncBack = inputState->m_stencilFuncBack;
        }
    }
}

void QDemonRenderBackendGLBase::setRasterizerState(QDemonRenderBackendRasterizerStateObject rasterizerState)
{
    QDemonRenderBackendRasterizerStateGL *inputState = (QDemonRenderBackendRasterizerStateGL *)rasterizerState;
    if (inputState && !(*m_currentRasterizerState == *inputState)) {
        // store current state
        *m_currentRasterizerState = *inputState;

        if (m_currentRasterizerState->m_depthBias != 0.0 || m_currentRasterizerState->m_depthScale != 0.0) {
            GL_CALL_FUNCTION(glEnable(GL_POLYGON_OFFSET_FILL));
        } else {
            GL_CALL_FUNCTION(glDisable(GL_POLYGON_OFFSET_FILL));
        }

        GL_CALL_FUNCTION(glPolygonOffset(m_currentRasterizerState->m_depthBias, m_currentRasterizerState->m_depthScale));

        GL_CALL_FUNCTION(glCullFace(m_conversion.fromFacesToGL(m_currentRasterizerState->m_cullFace)));
    }
}

bool QDemonRenderBackendGLBase::getRenderState(const QDemonRenderState value)
{
    bool enabled = GL_CALL_FUNCTION(glIsEnabled(m_conversion.fromRenderStateToGL(value)));
    return enabled;
}

QDemonRenderBoolOp QDemonRenderBackendGLBase::getDepthFunc()
{
    qint32 value;
    GL_CALL_FUNCTION(glGetIntegerv(GL_DEPTH_FUNC, &value));
    return GLConversion::fromGLToBoolOp(value);
}

void QDemonRenderBackendGLBase::setDepthFunc(const QDemonRenderBoolOp func)
{
    GL_CALL_FUNCTION(glDepthFunc(m_conversion.fromBoolOpToGL(func)));
}

bool QDemonRenderBackendGLBase::getDepthWrite()
{
    qint32 value;
    GL_CALL_FUNCTION(glGetIntegerv(GL_DEPTH_WRITEMASK, (GLint *)&value));
    return value ? true : false;
}

void QDemonRenderBackendGLBase::setDepthWrite(bool bEnable)
{
    GL_CALL_FUNCTION(glDepthMask(bEnable));
}

void QDemonRenderBackendGLBase::setColorWrites(bool bRed, bool bGreen, bool bBlue, bool bAlpha)
{
    GL_CALL_FUNCTION(glColorMask(bRed, bGreen, bBlue, bAlpha));
}

void QDemonRenderBackendGLBase::getBlendFunc(QDemonRenderBlendFunctionArgument *pBlendFuncArg)
{
    Q_ASSERT(pBlendFuncArg);
    qint32_4 values;

    GL_CALL_FUNCTION(glGetIntegerv(GL_BLEND_SRC_RGB, (GLint *)&values.x));
    GL_CALL_FUNCTION(glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint *)&values.y));
    GL_CALL_FUNCTION(glGetIntegerv(GL_BLEND_DST_RGB, (GLint *)&values.z));
    GL_CALL_FUNCTION(glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint *)&values.w));

    pBlendFuncArg->m_srcRgb = GLConversion::fromGLToSrcBlendFunc(values.x);
    pBlendFuncArg->m_srcAlpha = GLConversion::fromGLToSrcBlendFunc(values.y);
    pBlendFuncArg->m_dstRgb = GLConversion::fromGLToDstBlendFunc(values.z);
    pBlendFuncArg->m_dstAlpha = GLConversion::fromGLToDstBlendFunc(values.w);
}

void QDemonRenderBackendGLBase::setBlendFunc(const QDemonRenderBlendFunctionArgument &blendFuncArg)
{
    qint32_4 values;

    values.x = GLConversion::fromSrcBlendFuncToGL(blendFuncArg.m_srcRgb);
    values.y = GLConversion::fromDstBlendFuncToGL(blendFuncArg.m_dstRgb);
    values.z = GLConversion::fromSrcBlendFuncToGL(blendFuncArg.m_srcAlpha);
    values.w = GLConversion::fromDstBlendFuncToGL(blendFuncArg.m_dstAlpha);

    GL_CALL_FUNCTION(glBlendFuncSeparate(values.x, values.y, values.z, values.w));
}

void QDemonRenderBackendGLBase::setBlendEquation(const QDemonRenderBlendEquationArgument &)
{
    // needs GL4 / GLES 3.1
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::setBlendBarrier()
{
    // needs GL4 / GLES 3.1
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::getScissorRect(QRect *pRect)
{
    Q_ASSERT(pRect);
    GL_CALL_FUNCTION(glGetIntegerv(GL_SCISSOR_BOX, (GLint *)pRect));
}

void QDemonRenderBackendGLBase::setScissorRect(const QRect &rect)
{
    GL_CALL_FUNCTION(glScissor(rect.x(), rect.y(), rect.width(), rect.height()));
}

void QDemonRenderBackendGLBase::getViewportRect(QRect *pRect)
{
    Q_ASSERT(pRect);
    GL_CALL_FUNCTION(glGetIntegerv(GL_VIEWPORT, (GLint *)pRect));
}

void QDemonRenderBackendGLBase::setViewportRect(const QRect &rect)
{
    GL_CALL_FUNCTION(glViewport(rect.x(), rect.y(), rect.width(), rect.height()););
}

void QDemonRenderBackendGLBase::setClearColor(const QVector4D *pClearColor)
{
    Q_ASSERT(pClearColor);

    GL_CALL_FUNCTION(glClearColor(pClearColor->x(), pClearColor->y(), pClearColor->z(), pClearColor->w()));
}

void QDemonRenderBackendGLBase::clear(QDemonRenderClearFlags flags)
{
    GL_CALL_FUNCTION(glClear(m_conversion.fromClearFlagsToGL(flags)));
}

QDemonRenderBackend::QDemonRenderBackendBufferObject QDemonRenderBackendGLBase::createBuffer(size_t size,
                                                                                             QDemonRenderBufferBindFlags bindFlags,
                                                                                             QDemonRenderBufferUsageType usage,
                                                                                             const void *hostPtr)
{
    GLuint bufID = 0;

    GL_CALL_FUNCTION(glGenBuffers(1, &bufID));

    if (bufID && size) {
        GLenum target = GLConversion::fromBindBufferFlagsToGL(bindFlags);
        if (target != GL_INVALID_ENUM) {
            GL_CALL_FUNCTION(glBindBuffer(target, bufID));
            GL_CALL_FUNCTION(glBufferData(target, size, hostPtr, m_conversion.fromBufferUsageTypeToGL(usage)));
        } else {
            GL_CALL_FUNCTION(glDeleteBuffers(1, &bufID));
            bufID = 0;
            qCCritical(GL_ERROR, "%s", GLConversion::processGLError(target));
        }
    }

    return (QDemonRenderBackend::QDemonRenderBackendBufferObject)bufID;
}

void QDemonRenderBackendGLBase::bindBuffer(QDemonRenderBackendBufferObject bo, QDemonRenderBufferBindFlags bindFlags)
{
    GLuint bufID = HandleToID_cast(GLuint, size_t, bo);
    GL_CALL_FUNCTION(glBindBuffer(m_conversion.fromBindBufferFlagsToGL(bindFlags), bufID));
}

void QDemonRenderBackendGLBase::releaseBuffer(QDemonRenderBackendBufferObject bo)
{
    GLuint bufID = HandleToID_cast(GLuint, size_t, bo);
    GL_CALL_FUNCTION(glDeleteBuffers(1, &bufID));
}

void QDemonRenderBackendGLBase::updateBuffer(QDemonRenderBackendBufferObject bo,
                                             QDemonRenderBufferBindFlags bindFlags,
                                             size_t size,
                                             QDemonRenderBufferUsageType usage,
                                             const void *data)
{
    GLuint bufID = HandleToID_cast(GLuint, size_t, bo);
    GLenum target = GLConversion::fromBindBufferFlagsToGL(bindFlags);
    GL_CALL_FUNCTION(glBindBuffer(target, bufID));
    GL_CALL_FUNCTION(glBufferData(target, size, data, m_conversion.fromBufferUsageTypeToGL(usage)));
}

void QDemonRenderBackendGLBase::updateBufferRange(QDemonRenderBackendBufferObject bo,
                                                  QDemonRenderBufferBindFlags bindFlags,
                                                  size_t offset,
                                                  size_t size,
                                                  const void *data)
{
    GLuint bufID = HandleToID_cast(GLuint, size_t, bo);
    GLenum target = GLConversion::fromBindBufferFlagsToGL(bindFlags);
    GL_CALL_FUNCTION(glBindBuffer(target, bufID));
    GL_CALL_FUNCTION(glBufferSubData(target, offset, size, data));
}

void *QDemonRenderBackendGLBase::mapBuffer(QDemonRenderBackendBufferObject, QDemonRenderBufferBindFlags, size_t, size_t, QDemonRenderBufferAccessFlags)
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return nullptr;
}

bool QDemonRenderBackendGLBase::unmapBuffer(QDemonRenderBackendBufferObject, QDemonRenderBufferBindFlags)
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return true;
}

void QDemonRenderBackendGLBase::setMemoryBarrier(QDemonRenderBufferBarrierFlags)
{
    // needs GL 4 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

QDemonRenderBackend::QDemonRenderBackendQueryObject QDemonRenderBackendGLBase::createQuery()
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return QDemonRenderBackendQueryObject(nullptr);
}

void QDemonRenderBackendGLBase::releaseQuery(QDemonRenderBackendQueryObject)
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::beginQuery(QDemonRenderBackendQueryObject, QDemonRenderQueryType)
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::endQuery(QDemonRenderBackendQueryObject, QDemonRenderQueryType)
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::getQueryResult(QDemonRenderBackendQueryObject, QDemonRenderQueryResultType, quint32 *)
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::getQueryResult(QDemonRenderBackendQueryObject, QDemonRenderQueryResultType, quint64 *)
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::setQueryTimer(QDemonRenderBackendQueryObject)
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

QDemonRenderBackend::QDemonRenderBackendSyncObject QDemonRenderBackendGLBase::createSync(QDemonRenderSyncType, QDemonRenderSyncFlags)
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return QDemonRenderBackendSyncObject(nullptr);
}

void QDemonRenderBackendGLBase::releaseSync(QDemonRenderBackendSyncObject)
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::waitSync(QDemonRenderBackendSyncObject, QDemonRenderCommandFlushFlags, quint64)
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

QDemonRenderBackend::QDemonRenderBackendRenderTargetObject QDemonRenderBackendGLBase::createRenderTarget()
{
    GLuint fboID = 0;

    GL_CALL_FUNCTION(glGenFramebuffers(1, &fboID));

    return reinterpret_cast<QDemonRenderBackend::QDemonRenderBackendRenderTargetObject>(fboID);
}

void QDemonRenderBackendGLBase::releaseRenderTarget(QDemonRenderBackendRenderTargetObject rto)
{
    GLuint fboID = HandleToID_cast(GLuint, size_t, rto);

    if (fboID) {
        GL_CALL_FUNCTION(glDeleteFramebuffers(1, &fboID));
    }
}

void QDemonRenderBackendGLBase::renderTargetAttach(QDemonRenderBackendRenderTargetObject /* rto */,
                                                   QDemonRenderFrameBufferAttachment attachment,
                                                   QDemonRenderBackendRenderbufferObject rbo)
{
    // rto must be the current render target
    GLuint rbID = HandleToID_cast(GLuint, size_t, rbo);

    GLenum glAttach = GLConversion::fromFramebufferAttachmentsToGL(attachment);

    GL_CALL_FUNCTION(glFramebufferRenderbuffer(GL_FRAMEBUFFER, glAttach, GL_RENDERBUFFER, rbID));
}

void QDemonRenderBackendGLBase::renderTargetAttach(QDemonRenderBackendRenderTargetObject /* rto */,
                                                   QDemonRenderFrameBufferAttachment attachment,
                                                   QDemonRenderBackendTextureObject to,
                                                   QDemonRenderTextureTargetType target)
{
    // rto must be the current render target
    GLuint texID = HandleToID_cast(GLuint, size_t, to);

    Q_ASSERT(target == QDemonRenderTextureTargetType::Texture2D || m_backendSupport.caps.bits.bMsTextureSupported);

    GLenum glAttach = GLConversion::fromFramebufferAttachmentsToGL(attachment);
    GLenum glTarget = GLConversion::fromTextureTargetToGL(target);

    GL_CALL_FUNCTION(glFramebufferTexture2D(GL_FRAMEBUFFER, glAttach, glTarget, texID, 0))
}

void QDemonRenderBackendGLBase::renderTargetAttach(QDemonRenderBackendRenderTargetObject,
                                                   QDemonRenderFrameBufferAttachment,
                                                   QDemonRenderBackendTextureObject,
                                                   qint32,
                                                   qint32)
{
    // Needs GL3 or GLES 3
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::setRenderTarget(QDemonRenderBackendRenderTargetObject rto)
{
    GLuint fboID = HandleToID_cast(GLuint, size_t, rto);

    GL_CALL_FUNCTION(glBindFramebuffer(GL_FRAMEBUFFER, fboID));
}

bool QDemonRenderBackendGLBase::renderTargetIsValid(QDemonRenderBackendRenderTargetObject /* rto */)
{
    // rto must be the current render target
    GLenum completeStatus = GL_CALL_FUNCTION(glCheckFramebufferStatus(GL_FRAMEBUFFER));
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

QDemonRenderBackend::QDemonRenderBackendRenderbufferObject QDemonRenderBackendGLBase::createRenderbuffer(QDemonRenderRenderBufferFormat storageFormat,
                                                                                                         qint32 width,
                                                                                                         qint32 height)
{
    GLuint bufID = 0;

    GL_CALL_FUNCTION(glGenRenderbuffers(1, &bufID));
    GL_CALL_FUNCTION(glBindRenderbuffer(GL_RENDERBUFFER, bufID));
    GL_CALL_FUNCTION(glRenderbufferStorage(GL_RENDERBUFFER,
                                           GLConversion::fromRenderBufferFormatsToRenderBufferGL(storageFormat),
                                           GLsizei(width),
                                           GLsizei(height)));

    // check for error
    GLenum error = m_glFunctions->glGetError();
    if (error != GL_NO_ERROR) {
        qCCritical(GL_ERROR, "%s", GLConversion::processGLError(error));
        Q_ASSERT(false);
        GL_CALL_FUNCTION(glDeleteRenderbuffers(1, &bufID));
        bufID = 0;
    }

    GL_CALL_FUNCTION(glBindRenderbuffer(GL_RENDERBUFFER, 0));

    return (QDemonRenderBackend::QDemonRenderBackendRenderbufferObject)bufID;
}

void QDemonRenderBackendGLBase::releaseRenderbuffer(QDemonRenderBackendRenderbufferObject rbo)
{
    GLuint bufID = HandleToID_cast(GLuint, size_t, rbo);

    if (bufID) {
        GL_CALL_FUNCTION(glDeleteRenderbuffers(1, &bufID));
    }
}

bool QDemonRenderBackendGLBase::resizeRenderbuffer(QDemonRenderBackendRenderbufferObject rbo,
                                                   QDemonRenderRenderBufferFormat storageFormat,
                                                   qint32 width,
                                                   qint32 height)
{
    bool success = true;
    GLuint bufID = HandleToID_cast(GLuint, size_t, rbo);

    Q_ASSERT(bufID);

    GL_CALL_FUNCTION(glBindRenderbuffer(GL_RENDERBUFFER, bufID));
    GL_CALL_FUNCTION(glRenderbufferStorage(GL_RENDERBUFFER,
                                           GLConversion::fromRenderBufferFormatsToRenderBufferGL(storageFormat),
                                           GLsizei(width),
                                           GLsizei(height)));

    // check for error
    GLenum error = m_glFunctions->glGetError();
    if (error != GL_NO_ERROR) {
        qCCritical(GL_ERROR, "%s", GLConversion::processGLError(error));
        Q_ASSERT(false);
        success = false;
    }

    return success;
}

QDemonRenderBackend::QDemonRenderBackendTextureObject QDemonRenderBackendGLBase::createTexture()
{
    GLuint texID = 0;

    GL_CALL_FUNCTION(glGenTextures(1, &texID));
    return (QDemonRenderBackend::QDemonRenderBackendTextureObject)texID;
}

void QDemonRenderBackendGLBase::bindTexture(QDemonRenderBackendTextureObject to,
                                            QDemonRenderTextureTargetType target,
                                            qint32 unit)
{
    Q_ASSERT(unit >= 0);
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GL_CALL_FUNCTION(glActiveTexture(GL_TEXTURE0 + GLenum(unit)));
    GL_CALL_FUNCTION(glBindTexture(m_conversion.fromTextureTargetToGL(target), texID));
}

void QDemonRenderBackendGLBase::bindImageTexture(QDemonRenderBackendTextureObject,
                                                 quint32,
                                                 qint32,
                                                 bool,
                                                 qint32,
                                                 QDemonRenderImageAccessType,
                                                 QDemonRenderTextureFormat)
{
    // needs GL 4 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::releaseTexture(QDemonRenderBackendTextureObject to)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GL_CALL_FUNCTION(glDeleteTextures(1, &texID));
}

void QDemonRenderBackendGLBase::setTextureData2D(QDemonRenderBackendTextureObject to,
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
    GL_CALL_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_FUNCTION(glBindTexture(glTarget, texID));
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

    GL_CALL_FUNCTION(glTexImage2D(glTarget, level, glInternalFormat, (GLsizei)width, (GLsizei)height, border, glformat, gltype, hostPtr));

    GL_CALL_FUNCTION(glBindTexture(glTarget, 0));
}

// This will look very SetTextureData2D, but the target for glBindTexture will be different from
// the target for
// glTexImage2D.
void QDemonRenderBackendGLBase::setTextureDataCubeFace(QDemonRenderBackendTextureObject to,
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
    GLenum glTexTarget = GLConversion::fromTextureTargetToGL(QDemonRenderTextureTargetType::TextureCube);
    GL_CALL_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_FUNCTION(glBindTexture(glTexTarget, texID));
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

    // for es2 internal format must be same as format
    if (getRenderContextType() == QDemonRenderContextType::GLES2)
        glInternalFormat = glformat;

    GL_CALL_FUNCTION(glTexImage2D(glTarget, level, glInternalFormat, GLsizei(width), GLsizei(height), border, glformat, gltype, hostPtr));

    GL_CALL_FUNCTION(glBindTexture(glTexTarget, 0));
}

void QDemonRenderBackendGLBase::createTextureStorage2D(QDemonRenderBackendTextureObject,
                                                       QDemonRenderTextureTargetType,
                                                       qint32,
                                                       QDemonRenderTextureFormat,
                                                       qint32,
                                                       qint32)
{
    // you need GL 4.2 or GLES 3.1
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::setTextureSubData2D(QDemonRenderBackendTextureObject to,
                                                    QDemonRenderTextureTargetType target,
                                                    qint32 level,
                                                    qint32 xOffset,
                                                    qint32 yOffset,
                                                    qint32 width,
                                                    qint32 height,
                                                    QDemonRenderTextureFormat format,
                                                    const void *hostPtr)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GLenum glTarget = GLConversion::fromTextureTargetToGL(target);
    GL_CALL_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_FUNCTION(glBindTexture(glTarget, texID));

    QDemonRenderTextureSwizzleMode swizzleMode = QDemonRenderTextureSwizzleMode::NoSwizzle;
    format = GLConversion::replaceDeprecatedTextureFormat(getRenderContextType(), format, swizzleMode);

    GLenum glformat = 0, glInternalFormat = 0, gltype = 0;
    GLConversion::fromUncompressedTextureFormatToGL(getRenderContextType(), format, glformat, gltype, glInternalFormat);
    GL_CALL_FUNCTION(glTexSubImage2D(glTarget, level, xOffset, yOffset, (GLsizei)width, (GLsizei)height, glformat, gltype, hostPtr));

    GL_CALL_FUNCTION(glBindTexture(glTarget, 0));
}

void QDemonRenderBackendGLBase::setCompressedTextureData2D(QDemonRenderBackendTextureObject to,
                                                           QDemonRenderTextureTargetType target,
                                                           qint32 level,
                                                           QDemonRenderTextureFormat internalFormat,
                                                           qint32 width,
                                                           qint32 height,
                                                           qint32 border,
                                                           qint32 imageSize,
                                                           const void *hostPtr)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GLenum glTarget = GLConversion::fromTextureTargetToGL(target);
    GL_CALL_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_FUNCTION(glBindTexture(glTarget, texID));

    GLenum glformat = GLConversion::fromCompressedTextureFormatToGL(internalFormat);
    GL_CALL_FUNCTION(glCompressedTexImage2D(glTarget, level, glformat, GLsizei(width), GLsizei(height), border, GLsizei(imageSize), hostPtr));

    GL_CALL_FUNCTION(glBindTexture(glTarget, 0));
}

void QDemonRenderBackendGLBase::setCompressedTextureDataCubeFace(QDemonRenderBackendTextureObject to,
                                                                 QDemonRenderTextureTargetType target,
                                                                 qint32 level,
                                                                 QDemonRenderTextureFormat internalFormat,
                                                                 qint32 width,
                                                                 qint32 height,
                                                                 qint32 border,
                                                                 qint32 imageSize,
                                                                 const void *hostPtr)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GLenum glTarget = GLConversion::fromTextureTargetToGL(target);
    GLenum glTexTarget = GLConversion::fromTextureTargetToGL(QDemonRenderTextureTargetType::TextureCube);
    GL_CALL_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_FUNCTION(glBindTexture(glTexTarget, texID));

    GLenum glformat = GLConversion::fromCompressedTextureFormatToGL(internalFormat);
    GL_CALL_FUNCTION(glCompressedTexImage2D(glTarget, level, glformat, GLsizei(width), GLsizei(height), border, GLsizei(imageSize), hostPtr));

    GL_CALL_FUNCTION(glBindTexture(glTexTarget, 0));
}

void QDemonRenderBackendGLBase::setCompressedTextureSubData2D(QDemonRenderBackendTextureObject to,
                                                              QDemonRenderTextureTargetType target,
                                                              qint32 level,
                                                              qint32 xOffset,
                                                              qint32 yOffset,
                                                              qint32 width,
                                                              qint32 height,
                                                              QDemonRenderTextureFormat format,
                                                              qint32 imageSize,
                                                              const void *hostPtr)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GLenum glTarget = GLConversion::fromTextureTargetToGL(target);
    GL_CALL_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_FUNCTION(glBindTexture(glTarget, texID));

    GLenum glformat = GLConversion::fromCompressedTextureFormatToGL(format);
    GL_CALL_FUNCTION(
            glCompressedTexSubImage2D(glTarget, level, xOffset, yOffset, GLsizei(width), GLsizei(height), glformat, GLsizei(imageSize), hostPtr));

    GL_CALL_FUNCTION(glBindTexture(glTarget, 0));
}

void QDemonRenderBackendGLBase::setTextureData3D(QDemonRenderBackendTextureObject,
                                                 QDemonRenderTextureTargetType,
                                                 qint32,
                                                 QDemonRenderTextureFormat,
                                                 qint32,
                                                 qint32,
                                                 qint32,
                                                 qint32,
                                                 QDemonRenderTextureFormat,
                                                 const void *)
{
    // needs GL3 or GLES3
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::generateMipMaps(QDemonRenderBackendTextureObject to,
                                                QDemonRenderTextureTargetType target,
                                                QDemonRenderHint genType)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GLenum glTarget = GLConversion::fromTextureTargetToGL(target);
    GL_CALL_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_FUNCTION(glBindTexture(glTarget, texID));

    GLenum glValue = GLConversion::fromHintToGL(genType);
    GL_CALL_FUNCTION(glHint(GL_GENERATE_MIPMAP_HINT, glValue));
    GL_CALL_FUNCTION(glGenerateMipmap(glTarget));

    GL_CALL_FUNCTION(glBindTexture(glTarget, 0));
}

QDemonRenderTextureSwizzleMode QDemonRenderBackendGLBase::getTextureSwizzleMode(const QDemonRenderTextureFormat inFormat) const
{
    QDemonRenderTextureSwizzleMode swizzleMode = QDemonRenderTextureSwizzleMode::NoSwizzle;
    GLConversion::replaceDeprecatedTextureFormat(getRenderContextType(), inFormat, swizzleMode);

    return swizzleMode;
}

QDemonRenderBackend::QDemonRenderBackendSamplerObject QDemonRenderBackendGLBase::createSampler(
        QDemonRenderTextureMinifyingOp minFilter,
        QDemonRenderTextureMagnifyingOp magFilter,
        QDemonRenderTextureCoordOp wrapS,
        QDemonRenderTextureCoordOp wrapT,
        QDemonRenderTextureCoordOp wrapR,
        qint32 minLod,
        qint32 maxLod,
        float lodBias,
        QDemonRenderTextureCompareMode compareMode,
        QDemonRenderTextureCompareOp compareFunc,
        float anisotropy,
        float *borderColor)
{
    // Satisfy the compiler
    // We don"t setup the state here for GL
    // but we need to pass on the variables here
    // to satisfy the interface
    Q_UNUSED(minFilter)
    Q_UNUSED(magFilter)
    Q_UNUSED(wrapS)
    Q_UNUSED(wrapT)
    Q_UNUSED(wrapR)
    Q_UNUSED(minLod)
    Q_UNUSED(maxLod)
    Q_UNUSED(lodBias)
    Q_UNUSED(compareMode)
    Q_UNUSED(compareFunc)
    Q_UNUSED(anisotropy)
    Q_UNUSED(borderColor)

    // return a dummy handle
    return (QDemonRenderBackend::QDemonRenderBackendSamplerObject)0x0001;
}

void QDemonRenderBackendGLBase::updateSampler(QDemonRenderBackendSamplerObject /* so */,
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
    // These are not available in GLES 2 and we don't use them right now
    Q_UNUSED(wrapR)
    Q_UNUSED(lodBias)
    Q_UNUSED(minLod)
    Q_UNUSED(maxLod)
    Q_UNUSED(compareMode)
    Q_UNUSED(compareFunc)
    Q_UNUSED(borderColor)

    GLenum glTarget = GLConversion::fromTextureTargetToGL(target);

    GL_CALL_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_MIN_FILTER, m_conversion.fromTextureMinifyingOpToGL(minFilter)));
    GL_CALL_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_MAG_FILTER, m_conversion.fromTextureMagnifyingOpToGL(magFilter)));
    GL_CALL_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_WRAP_S, m_conversion.fromTextureCoordOpToGL(wrapS)));
    GL_CALL_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_WRAP_T, m_conversion.fromTextureCoordOpToGL(wrapT)));
    if (m_backendSupport.caps.bits.bAnistropySupported) {
        GL_CALL_FUNCTION(glTexParameterf(glTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy));
    }
}

void QDemonRenderBackendGLBase::updateTextureObject(QDemonRenderBackendTextureObject to,
                                                    QDemonRenderTextureTargetType target,
                                                    qint32 baseLevel,
                                                    qint32 maxLevel)
{
    Q_UNUSED(to)
    Q_UNUSED(target)
    Q_UNUSED(baseLevel)
    Q_UNUSED(maxLevel)
}

void QDemonRenderBackendGLBase::updateTextureSwizzle(QDemonRenderBackendTextureObject to,
                                                     QDemonRenderTextureTargetType target,
                                                     QDemonRenderTextureSwizzleMode swizzleMode)
{
    Q_UNUSED(to)
    Q_UNUSED(target)

    // Nothing to do here still might be called
    Q_ASSERT(swizzleMode == QDemonRenderTextureSwizzleMode::NoSwizzle);

    Q_UNUSED(swizzleMode)
}

void QDemonRenderBackendGLBase::releaseSampler(QDemonRenderBackendSamplerObject so)
{
    GLuint samplerID = HandleToID_cast(GLuint, size_t, so);
    if (!samplerID)
        return;
    // otherwise nothing to do
}

QDemonRenderBackend::QDemonRenderBackendAttribLayoutObject QDemonRenderBackendGLBase::createAttribLayout(
        QDemonConstDataRef<QDemonRenderVertexBufferEntry> attribs)
{
    quint32 attribLayoutSize = sizeof(QDemonRenderBackendAttributeLayoutGL);
    quint32 entrySize = sizeof(QDemonRenderBackendLayoutEntryGL) * attribs.size();
    quint8 *newMem = static_cast<quint8 *>(::malloc(attribLayoutSize + entrySize));
    QDemonDataRef<QDemonRenderBackendLayoutEntryGL> entryRef = PtrAtOffset<QDemonRenderBackendLayoutEntryGL>(newMem, attribLayoutSize, entrySize);
    quint32 maxInputSlot = 0;

    // copy data
    for (int idx = 0; idx != attribs.size(); ++idx) {
        new (&entryRef[idx]) QDemonRenderBackendLayoutEntryGL();
        entryRef[idx].m_attribName = QString::fromLocal8Bit(attribs.mData[idx].m_name);
        entryRef[idx].m_normalize = 0;
        entryRef[idx].m_attribIndex = 0; // will be set later
        entryRef[idx].m_type = GLConversion::fromComponentTypeAndNumCompsToAttribGL(attribs.mData[idx].m_componentType,
                                                                                    attribs.mData[idx].m_numComponents);
        entryRef[idx].m_numComponents = attribs.mData[idx].m_numComponents;
        entryRef[idx].m_inputSlot = attribs.mData[idx].m_inputSlot;
        entryRef[idx].m_offset = attribs.mData[idx].m_firstItemOffset;

        if (maxInputSlot < entryRef[idx].m_inputSlot)
            maxInputSlot = entryRef[idx].m_inputSlot;
    }

    QDemonRenderBackendAttributeLayoutGL *retval = new (newMem) QDemonRenderBackendAttributeLayoutGL(entryRef, maxInputSlot);

    return (QDemonRenderBackend::QDemonRenderBackendAttribLayoutObject)retval;
}

void QDemonRenderBackendGLBase::releaseAttribLayout(QDemonRenderBackendAttribLayoutObject ao)
{
    QDemonRenderBackendAttributeLayoutGL *attribLayout = (QDemonRenderBackendAttributeLayoutGL *)ao;

    delete attribLayout;
};

QDemonRenderBackend::QDemonRenderBackendInputAssemblerObject QDemonRenderBackendGLBase::createInputAssembler(
        QDemonRenderBackendAttribLayoutObject attribLayout,
        QDemonConstDataRef<QDemonRenderBackendBufferObject> buffers,
        const QDemonRenderBackendBufferObject indexBuffer,
        QDemonConstDataRef<quint32> strides,
        QDemonConstDataRef<quint32> offsets,
        quint32 patchVertexCount)
{
    QDemonRenderBackendAttributeLayoutGL *attribLayoutGL = (QDemonRenderBackendAttributeLayoutGL *)attribLayout;

    QDemonRenderBackendInputAssemblerGL *retval = new QDemonRenderBackendInputAssemblerGL(attribLayoutGL,
                                                                                          buffers,
                                                                                          indexBuffer,
                                                                                          strides,
                                                                                          offsets,
                                                                                          patchVertexCount);

    return (QDemonRenderBackend::QDemonRenderBackendInputAssemblerObject)retval;
}

void QDemonRenderBackendGLBase::releaseInputAssembler(QDemonRenderBackendInputAssemblerObject iao)
{
    QDemonRenderBackendInputAssemblerGL *inputAssembler = (QDemonRenderBackendInputAssemblerGL *)iao;
    delete inputAssembler;
}

bool QDemonRenderBackendGLBase::compileSource(GLuint shaderID, QDemonConstDataRef<qint8> source, QByteArray &errorMessage, bool binary)
{
    GLint shaderSourceSize = static_cast<GLint>(source.size());
    const char *shaderSourceData = (const char *)source.begin();
    GLint shaderStatus = GL_TRUE;

    if (!binary) {

        GL_CALL_FUNCTION(glShaderSource(shaderID, 1, &shaderSourceData, &shaderSourceSize));
        GL_CALL_FUNCTION(glCompileShader(shaderID));

        GLint logLen;
        GL_CALL_FUNCTION(glGetShaderiv(shaderID, GL_COMPILE_STATUS, &shaderStatus));
        GL_CALL_FUNCTION(glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLen));

        // Check if some log exists. We also write warnings here
        // Should at least contain more than the null termination
        if (logLen > 2) {
            errorMessage.resize(logLen + 1);

            GLint lenWithoutNull;
            GL_CALL_FUNCTION(glGetShaderInfoLog(shaderID, logLen, &lenWithoutNull, errorMessage.data()));
        }
    } else {
        GL_CALL_FUNCTION(glShaderBinary(1, &shaderID, GL_NVIDIA_PLATFORM_BINARY_NV, shaderSourceData, shaderSourceSize));
        GLenum binaryError = m_glFunctions->glGetError();
        if (binaryError != GL_NO_ERROR) {
            errorMessage = QByteArrayLiteral("Binary shader compilation failed");
            shaderStatus = GL_FALSE;
            qCCritical(GL_ERROR, "%s", GLConversion::processGLError(binaryError));
        }
    }

    return (shaderStatus == GL_TRUE);
}

QDemonRenderBackend::QDemonRenderBackendVertexShaderObject QDemonRenderBackendGLBase::createVertexShader(QDemonConstDataRef<qint8> source,
                                                                                                         QByteArray &errorMessage,
                                                                                                         bool binary)
{
    GLuint shaderID = GL_CALL_FUNCTION(glCreateShader(GL_VERTEX_SHADER));

    if (shaderID && !compileSource(shaderID, source, errorMessage, binary)) {
        GL_CALL_FUNCTION(glDeleteShader(shaderID));
        shaderID = 0;
    }

    return (QDemonRenderBackend::QDemonRenderBackendVertexShaderObject)shaderID;
}

QDemonRenderBackend::QDemonRenderBackendFragmentShaderObject QDemonRenderBackendGLBase::createFragmentShader(QDemonConstDataRef<qint8> source,
                                                                                                             QByteArray &errorMessage,
                                                                                                             bool binary)
{
    GLuint shaderID = GL_CALL_FUNCTION(glCreateShader(GL_FRAGMENT_SHADER));

    if (shaderID && !compileSource(shaderID, source, errorMessage, binary)) {
        GL_CALL_FUNCTION(glDeleteShader(shaderID));
        shaderID = 0;
    }

    return (QDemonRenderBackend::QDemonRenderBackendFragmentShaderObject)shaderID;
}

QDemonRenderBackend::QDemonRenderBackendTessControlShaderObject QDemonRenderBackendGLBase::createTessControlShader(
        QDemonConstDataRef<qint8> source,
        QByteArray &errorMessage,
        bool binary)
{
    // needs GL 4 or GLES EXT_tessellation_shader support
    Q_UNUSED(source)
    Q_UNUSED(errorMessage)
    Q_UNUSED(binary)

    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return (QDemonRenderBackend::QDemonRenderBackendTessControlShaderObject) nullptr;
}

QDemonRenderBackend::QDemonRenderBackendTessEvaluationShaderObject QDemonRenderBackendGLBase::createTessEvaluationShader(
        QDemonConstDataRef<qint8> source,
        QByteArray &errorMessage,
        bool binary)
{
    // needs GL 4 or GLES EXT_tessellation_shader support
    Q_UNUSED(source)
    Q_UNUSED(errorMessage)
    Q_UNUSED(binary)

    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return (QDemonRenderBackend::QDemonRenderBackendTessEvaluationShaderObject) nullptr;
}

QDemonRenderBackend::QDemonRenderBackendGeometryShaderObject QDemonRenderBackendGLBase::createGeometryShader(QDemonConstDataRef<qint8> source,
                                                                                                             QByteArray &errorMessage,
                                                                                                             bool binary)
{
    // needs GL 4 or GLES EXT_geometry_shader support
    Q_UNUSED(source)
    Q_UNUSED(errorMessage)
    Q_UNUSED(binary)

    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return (QDemonRenderBackend::QDemonRenderBackendGeometryShaderObject) nullptr;
}

QDemonRenderBackend::QDemonRenderBackendComputeShaderObject QDemonRenderBackendGLBase::createComputeShader(QDemonConstDataRef<qint8> source,
                                                                                                           QByteArray &errorMessage,
                                                                                                           bool binary)
{
    // needs GL 4.3 or GLES3.1 support
    Q_UNUSED(source)
    Q_UNUSED(errorMessage)
    Q_UNUSED(binary)

    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return (QDemonRenderBackend::QDemonRenderBackendComputeShaderObject) nullptr;
}

void QDemonRenderBackendGLBase::releaseVertexShader(QDemonRenderBackendVertexShaderObject vso)
{
    GLuint shaderID = HandleToID_cast(GLuint, size_t, vso);

    GL_CALL_FUNCTION(glDeleteShader(shaderID));
}

void QDemonRenderBackendGLBase::releaseFragmentShader(QDemonRenderBackendFragmentShaderObject fso)
{
    GLuint shaderID = HandleToID_cast(GLuint, size_t, fso);

    GL_CALL_FUNCTION(glDeleteShader(shaderID));
}

void QDemonRenderBackendGLBase::releaseTessControlShader(QDemonRenderBackendTessControlShaderObject tcso)
{
    GLuint shaderID = HandleToID_cast(GLuint, size_t, tcso);

    GL_CALL_FUNCTION(glDeleteShader(shaderID));
}

void QDemonRenderBackendGLBase::releaseTessEvaluationShader(QDemonRenderBackendTessEvaluationShaderObject teso)
{
    GLuint shaderID = HandleToID_cast(GLuint, size_t, teso);

    GL_CALL_FUNCTION(glDeleteShader(shaderID));
}

void QDemonRenderBackendGLBase::releaseGeometryShader(QDemonRenderBackendGeometryShaderObject gso)
{
    GLuint shaderID = HandleToID_cast(GLuint, size_t, gso);

    GL_CALL_FUNCTION(glDeleteShader(shaderID));
}

void QDemonRenderBackendGLBase::releaseComputeShader(QDemonRenderBackendComputeShaderObject cso)
{
    GLuint shaderID = HandleToID_cast(GLuint, size_t, cso);

    GL_CALL_FUNCTION(glDeleteShader(shaderID));
}

void QDemonRenderBackendGLBase::attachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendVertexShaderObject vso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, vso);

    GL_CALL_FUNCTION(glAttachShader(static_cast<GLuint>(pProgram->m_programID), shaderID));
}

void QDemonRenderBackendGLBase::attachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendFragmentShaderObject fso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, fso);

    GL_CALL_FUNCTION(glAttachShader(static_cast<GLuint>(pProgram->m_programID), shaderID));
}

void QDemonRenderBackendGLBase::attachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendTessControlShaderObject tcso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, tcso);

    GL_CALL_FUNCTION(glAttachShader(static_cast<GLuint>(pProgram->m_programID), shaderID));
}

void QDemonRenderBackendGLBase::attachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendTessEvaluationShaderObject teso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, teso);

    GL_CALL_FUNCTION(glAttachShader(static_cast<GLuint>(pProgram->m_programID), shaderID));
}

void QDemonRenderBackendGLBase::attachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendGeometryShaderObject gso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, gso);

    GL_CALL_FUNCTION(glAttachShader(static_cast<GLuint>(pProgram->m_programID), shaderID));
}

void QDemonRenderBackendGLBase::attachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendComputeShaderObject cso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, cso);

    GL_CALL_FUNCTION(glAttachShader(static_cast<GLuint>(pProgram->m_programID), shaderID));
}

void QDemonRenderBackendGLBase::detachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendVertexShaderObject vso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, vso);

    GL_CALL_FUNCTION(glDetachShader(static_cast<GLuint>(pProgram->m_programID), shaderID));
}

void QDemonRenderBackendGLBase::detachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendFragmentShaderObject fso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, fso);

    GL_CALL_FUNCTION(glDetachShader(static_cast<GLuint>(pProgram->m_programID), shaderID));
}

void QDemonRenderBackendGLBase::detachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendTessControlShaderObject tcso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, tcso);

    GL_CALL_FUNCTION(glDetachShader(static_cast<GLuint>(pProgram->m_programID), shaderID));
}

void QDemonRenderBackendGLBase::detachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendTessEvaluationShaderObject teso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, teso);

    GL_CALL_FUNCTION(glDetachShader(static_cast<GLuint>(pProgram->m_programID), shaderID));
}

void QDemonRenderBackendGLBase::detachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendGeometryShaderObject gso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, gso);

    GL_CALL_FUNCTION(glDetachShader(static_cast<GLuint>(pProgram->m_programID), shaderID));
}

void QDemonRenderBackendGLBase::detachShader(QDemonRenderBackendShaderProgramObject po, QDemonRenderBackendComputeShaderObject cso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, cso);

    GL_CALL_FUNCTION(glDetachShader(static_cast<GLuint>(pProgram->m_programID), shaderID));
}

QDemonRenderBackend::QDemonRenderBackendShaderProgramObject QDemonRenderBackendGLBase::createShaderProgram(bool isSeparable)
{
    QDemonRenderBackendShaderProgramGL *theProgram = nullptr;
    GLuint programID = GL_CALL_FUNCTION(glCreateProgram());

    if (programID) {
        theProgram = new QDemonRenderBackendShaderProgramGL(programID);

        if (!theProgram) {
            GL_CALL_FUNCTION(glDeleteProgram(programID));
        } else if (isSeparable && m_backendSupport.caps.bits.bProgramPipelineSupported) {
            GL_CALL_EXTRA_FUNCTION(glProgramParameteri(programID, GL_PROGRAM_SEPARABLE, GL_TRUE));
        }
    }

    return (QDemonRenderBackend::QDemonRenderBackendShaderProgramObject)theProgram;
}

void QDemonRenderBackendGLBase::releaseShaderProgram(QDemonRenderBackendShaderProgramObject po)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_programID);

    GL_CALL_FUNCTION(glDeleteProgram(programID));

    if (pProgram->m_shaderInput) {
        delete pProgram->m_shaderInput;
        pProgram->m_shaderInput = nullptr;
    }

    delete pProgram;
}

bool QDemonRenderBackendGLBase::linkProgram(QDemonRenderBackendShaderProgramObject po, QByteArray &errorMessage)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_programID);

    GL_CALL_FUNCTION(glLinkProgram(programID));

    GLint linkStatus, logLen;
    GL_CALL_FUNCTION(glGetProgramiv(programID, GL_LINK_STATUS, &linkStatus));
    GL_CALL_FUNCTION(glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logLen));

    // if succesfuly linked get the attribute information
    if (linkStatus) {
        // release old stuff
        if (pProgram->m_shaderInput) {
            delete pProgram->m_shaderInput;
            pProgram->m_shaderInput = nullptr;
        }

        GLint numAttribs;
        GL_CALL_FUNCTION(glGetProgramiv(programID, GL_ACTIVE_ATTRIBUTES, &numAttribs));

        if (numAttribs) {
            QDemonRenderBackendShaderInputEntryGL *tempShaderInputEntry = static_cast<QDemonRenderBackendShaderInputEntryGL *>(
                    ::malloc(sizeof(QDemonRenderBackendShaderInputEntryGL) * m_maxAttribCount));

            GLint maxLength;
            GL_CALL_FUNCTION(glGetProgramiv(programID, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength));
            qint8 *nameBuf = static_cast<qint8 *>(::malloc(size_t(maxLength)));

            // fill in data
            quint32 count = 0;
            for (int idx = 0; idx != numAttribs; ++idx) {
                GLint size = 0;
                GLenum glType;
                QDemonRenderComponentType compType = QDemonRenderComponentType::Unknown;
                quint32 numComps = 0;

                GL_CALL_FUNCTION(glGetActiveAttrib(programID, idx, maxLength, nullptr, &size, &glType, (char *)nameBuf));
                // Skip anything named with gl_
                if (memcmp(nameBuf, "gl_", 3) == 0)
                    continue;

                GLConversion::fromAttribGLToComponentTypeAndNumComps(glType, compType, numComps);

                new (&tempShaderInputEntry[count]) QDemonRenderBackendShaderInputEntryGL();
                tempShaderInputEntry[count].m_attribName = QString::fromLocal8Bit(reinterpret_cast<char *>(nameBuf));
                tempShaderInputEntry[count].m_attribLocation = GL_CALL_FUNCTION(glGetAttribLocation(programID, (char *)nameBuf));
                tempShaderInputEntry[count].m_type = glType;
                tempShaderInputEntry[count].m_numComponents = numComps;

                ++count;
            }

            // Now allocate space for the actuall entries
            quint32 shaderInputSize = sizeof(QDemonRenderBackendShaderInputGL);
            quint32 entrySize = sizeof(QDemonRenderBackendShaderInputEntryGL) * count;
            quint8 *newMem = static_cast<quint8 *>(::malloc(shaderInputSize + entrySize));
            QDemonDataRef<QDemonRenderBackendShaderInputEntryGL> entryRef = PtrAtOffset<QDemonRenderBackendShaderInputEntryGL>(newMem, shaderInputSize, entrySize);
            // fill data
            for (int idx = 0; idx != count; ++idx) {
                new (&entryRef[idx]) QDemonRenderBackendShaderInputEntryGL();
                entryRef[idx].m_attribName = tempShaderInputEntry[idx].m_attribName;
                entryRef[idx].m_attribLocation = tempShaderInputEntry[idx].m_attribLocation;
                entryRef[idx].m_type = tempShaderInputEntry[idx].m_type;
                entryRef[idx].m_numComponents = tempShaderInputEntry[idx].m_numComponents;
            }

            // placement new
            QDemonRenderBackendShaderInputGL *shaderInput = new (newMem) QDemonRenderBackendShaderInputGL(entryRef);
            // set the pointer
            pProgram->m_shaderInput = shaderInput;

            ::free(nameBuf);
            ::free(tempShaderInputEntry);
        }
    }

    // Check if some log exists. We also write warnings here
    // Should at least contain more than the null termination
    if (logLen > 2) {
        errorMessage.resize(logLen + 1);

        GLint lenWithoutNull;
        GL_CALL_FUNCTION(glGetProgramInfoLog(programID, logLen, &lenWithoutNull, errorMessage.data()));
    }

    return (linkStatus == GL_TRUE);
}

void QDemonRenderBackendGLBase::setActiveProgram(QDemonRenderBackendShaderProgramObject po)
{
    GLuint programID = 0;

    if (po) {
        QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
        programID = static_cast<GLuint>(pProgram->m_programID);
    }

    GL_CALL_FUNCTION(glUseProgram(programID));
}

QDemonRenderBackend::QDemonRenderBackendProgramPipeline QDemonRenderBackendGLBase::createProgramPipeline()
{
    // needs GL 4 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
    return QDemonRenderBackend::QDemonRenderBackendProgramPipeline(nullptr);
}

void QDemonRenderBackendGLBase::releaseProgramPipeline(QDemonRenderBackendProgramPipeline)
{
    // needs GL 4 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::setActiveProgramPipeline(QDemonRenderBackendProgramPipeline)
{
    // needs GL 4 context
    // TODO: should be fixed?
    //        Q_ASSERT(false);
}

void QDemonRenderBackendGLBase::setProgramStages(QDemonRenderBackendProgramPipeline, QDemonRenderShaderTypeFlags, QDemonRenderBackendShaderProgramObject)
{
    // needs GL 4 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::dispatchCompute(QDemonRenderBackendShaderProgramObject, quint32, quint32, quint32)
{
    // needs GL 4 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

qint32 QDemonRenderBackendGLBase::getConstantCount(QDemonRenderBackendShaderProgramObject po)
{
    Q_ASSERT(po);
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_programID);

    GLint numUniforms;
    GL_CALL_FUNCTION(glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &numUniforms));

    return numUniforms;
}

qint32 QDemonRenderBackendGLBase::getConstantBufferCount(QDemonRenderBackendShaderProgramObject po)
{
    // needs GL3 and above
    Q_UNUSED(po)

    return 0;
}

qint32 QDemonRenderBackendGLBase::getConstantInfoByID(QDemonRenderBackendShaderProgramObject po,
                                                      quint32 id,
                                                      quint32 bufSize,
                                                      qint32 *numElem,
                                                      QDemonRenderShaderDataType *type,
                                                      qint32 *binding,
                                                      char *nameBuf)
{
    Q_ASSERT(po);
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_programID);

    GLenum glType;
    GL_CALL_FUNCTION(glGetActiveUniform(programID, id, bufSize, nullptr, numElem, &glType, nameBuf));
    *type = GLConversion::fromShaderGLToPropertyDataTypes(glType);

    qint32 uniformLoc = GL_CALL_FUNCTION(glGetUniformLocation(programID, nameBuf));

    // get unit binding point
    *binding = -1;
    if (uniformLoc != -1 && (glType == GL_IMAGE_2D || glType == GL_UNSIGNED_INT_IMAGE_2D || glType == GL_UNSIGNED_INT_ATOMIC_COUNTER)) {
        GL_CALL_FUNCTION(glGetUniformiv(programID, uniformLoc, binding));
    }

    return uniformLoc;
}

qint32 QDemonRenderBackendGLBase::getConstantBufferInfoByID(QDemonRenderBackendShaderProgramObject po,
                                                            quint32 id,
                                                            quint32 nameBufSize,
                                                            qint32 *paramCount,
                                                            qint32 *bufferSize,
                                                            qint32 *length,
                                                            char *nameBuf)
{
    // needs GL3 and above
    Q_UNUSED(po)
    Q_UNUSED(id)
    Q_UNUSED(nameBufSize)
    Q_UNUSED(paramCount)
    Q_UNUSED(bufferSize)
    Q_UNUSED(length)
    Q_UNUSED(nameBuf)

    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return -1;
}

void QDemonRenderBackendGLBase::getConstantBufferParamIndices(QDemonRenderBackendShaderProgramObject po, quint32 id, qint32 *indices)
{
    // needs GL3 and above
    Q_UNUSED(po)
    Q_UNUSED(id)
    Q_UNUSED(indices)

    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::getConstantBufferParamInfoByIndices(QDemonRenderBackendShaderProgramObject po,
                                                                    quint32 count,
                                                                    quint32 *indices,
                                                                    QDemonRenderShaderDataType *type,
                                                                    qint32 *size,
                                                                    qint32 *offset)
{
    // needs GL3 and above
    Q_UNUSED(po)
    Q_UNUSED(count)
    Q_UNUSED(indices)
    Q_UNUSED(type)
    Q_UNUSED(size)
    Q_UNUSED(offset)

    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::programSetConstantBlock(QDemonRenderBackendShaderProgramObject po, quint32 blockIndex, quint32 binding)
{
    // needs GL3 and above
    Q_UNUSED(po)
    Q_UNUSED(blockIndex)
    Q_UNUSED(binding)

    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::programSetConstantBuffer(quint32 index, QDemonRenderBackendBufferObject bo)
{
    // needs GL3 and above
    Q_UNUSED(index)
    Q_UNUSED(bo)

    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

qint32 QDemonRenderBackendGLBase::getStorageBufferCount(QDemonRenderBackendShaderProgramObject po)
{
    // needs GL4 and above
    Q_UNUSED(po)

    return 0;
}

qint32 QDemonRenderBackendGLBase::getStorageBufferInfoByID(QDemonRenderBackendShaderProgramObject po,
                                                           quint32 id,
                                                           quint32 nameBufSize,
                                                           qint32 *paramCount,
                                                           qint32 *bufferSize,
                                                           qint32 *length,
                                                           char *nameBuf)
{
    // needs GL4 and above
    Q_UNUSED(po)
    Q_UNUSED(id)
    Q_UNUSED(nameBufSize)
    Q_UNUSED(paramCount)
    Q_UNUSED(bufferSize)
    Q_UNUSED(length)
    Q_UNUSED(nameBuf)

    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return -1;
}

void QDemonRenderBackendGLBase::programSetStorageBuffer(quint32 index, QDemonRenderBackendBufferObject bo)
{
    // needs GL4 and above
    Q_UNUSED(index)
    Q_UNUSED(bo)
}

qint32 QDemonRenderBackendGLBase::getAtomicCounterBufferCount(QDemonRenderBackendShaderProgramObject po)
{
    // needs GL4 and above
    Q_UNUSED(po)

    return 0;
}

qint32 QDemonRenderBackendGLBase::getAtomicCounterBufferInfoByID(QDemonRenderBackendShaderProgramObject po,
                                                                 quint32 id,
                                                                 quint32 nameBufSize,
                                                                 qint32 *paramCount,
                                                                 qint32 *bufferSize,
                                                                 qint32 *length,
                                                                 char *nameBuf)
{
    // needs GL4 and above
    Q_UNUSED(po)
    Q_UNUSED(id)
    Q_UNUSED(nameBufSize)
    Q_UNUSED(paramCount)
    Q_UNUSED(bufferSize)
    Q_UNUSED(length)
    Q_UNUSED(nameBuf)

    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return -1;
}

void QDemonRenderBackendGLBase::programSetAtomicCounterBuffer(quint32 index, QDemonRenderBackendBufferObject bo)
{
    // needs GL4 and above
    Q_UNUSED(index)
    Q_UNUSED(bo)
}

void QDemonRenderBackendGLBase::setConstantValue(QDemonRenderBackendShaderProgramObject,
                                                 quint32 id,
                                                 QDemonRenderShaderDataType type,
                                                 qint32 count,
                                                 const void *value,
                                                 bool transpose)
{
    GLenum glType = GLConversion::fromPropertyDataTypesToShaderGL(type);

    switch (glType) {
    case GL_FLOAT:
        GL_CALL_FUNCTION(glUniform1fv(id, count, (GLfloat *)value));
        break;
    case GL_FLOAT_VEC2:
        GL_CALL_FUNCTION(glUniform2fv(id, count, (GLfloat *)value));
        break;
    case GL_FLOAT_VEC3:
        GL_CALL_FUNCTION(glUniform3fv(id, count, (GLfloat *)value));
        break;
    case GL_FLOAT_VEC4:
        GL_CALL_FUNCTION(glUniform4fv(id, count, (GLfloat *)value));
        break;
    case GL_INT:
        GL_CALL_FUNCTION(glUniform1iv(id, count, (GLint *)value));
        break;
    case GL_BOOL: {
        // Cast int value to be 0 or 1, matching to bool
        GLint *boolValue = (GLint *)value;
        *boolValue = *(GLboolean *)value;
        GL_CALL_FUNCTION(glUniform1iv(id, count, boolValue));
    } break;
    case GL_INT_VEC2:
    case GL_BOOL_VEC2:
        GL_CALL_FUNCTION(glUniform2iv(id, count, (GLint *)value));
        break;
    case GL_INT_VEC3:
    case GL_BOOL_VEC3:
        GL_CALL_FUNCTION(glUniform3iv(id, count, (GLint *)value));
        break;
    case GL_INT_VEC4:
    case GL_BOOL_VEC4:
        GL_CALL_FUNCTION(glUniform4iv(id, count, (GLint *)value));
        break;
    case GL_FLOAT_MAT3:
        GL_CALL_FUNCTION(glUniformMatrix3fv(id, count, transpose, (GLfloat *)value));
        break;
    case GL_FLOAT_MAT4:
        GL_CALL_FUNCTION(glUniformMatrix4fv(id, count, transpose, (GLfloat *)value));
        break;
    case GL_IMAGE_2D:
    case GL_SAMPLER_2D:
    case GL_SAMPLER_2D_ARRAY:
    case GL_SAMPLER_2D_SHADOW:
    case GL_SAMPLER_CUBE: {
        if (count > 1) {
            GLint *sampler = (GLint *)value;
            GL_CALL_FUNCTION(glUniform1iv(id, count, sampler));
        } else {
            GLint sampler = *(GLint *)value;
            GL_CALL_FUNCTION(glUniform1i(id, sampler));
        }
    } break;
    default:
        qCCritical(INTERNAL_ERROR, "Unknown shader type format %d", type);
        Q_ASSERT(false);
        break;
    }
}

void QDemonRenderBackendGLBase::draw(QDemonRenderDrawMode drawMode, quint32 start, quint32 count)
{
    GL_CALL_FUNCTION(glDrawArrays(m_conversion.fromDrawModeToGL(drawMode, m_backendSupport.caps.bits.bTessellationSupported), start, count));
}

void QDemonRenderBackendGLBase::drawIndirect(QDemonRenderDrawMode drawMode, const void *indirect)
{
    // needs GL4 and above
    Q_UNUSED(drawMode)
    Q_UNUSED(indirect)
}

void QDemonRenderBackendGLBase::drawIndexed(QDemonRenderDrawMode drawMode,
                                            quint32 count,
                                            QDemonRenderComponentType type,
                                            const void *indices)
{
    GL_CALL_FUNCTION(glDrawElements(m_conversion.fromDrawModeToGL(drawMode, m_backendSupport.caps.bits.bTessellationSupported),
                                    count,
                                    m_conversion.fromIndexBufferComponentsTypesToGL(type),
                                    indices));
}

void QDemonRenderBackendGLBase::drawIndexedIndirect(QDemonRenderDrawMode drawMode,
                                                    QDemonRenderComponentType type,
                                                    const void *indirect)
{
    // needs GL4 and above
    Q_UNUSED(drawMode)
    Q_UNUSED(type)
    Q_UNUSED(indirect)
}

void QDemonRenderBackendGLBase::readPixel(QDemonRenderBackendRenderTargetObject /* rto */,
                                          qint32 x,
                                          qint32 y,
                                          qint32 width,
                                          qint32 height,
                                          QDemonRenderReadPixelFormat inFormat,
                                          void *pixels)
{
    GLuint glFormat;
    GLuint glType;
    if (GLConversion::fromReadPixelsToGlFormatAndType(inFormat, &glFormat, &glType)) {
        GL_CALL_FUNCTION(glReadPixels(x, y, width, height, glFormat, glType, pixels));
    }
}

QDemonRenderBackend::QDemonRenderBackendPathObject QDemonRenderBackendGLBase::createPathNVObject(size_t)
{
    // Needs GL 4 backend
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return QDemonRenderBackend::QDemonRenderBackendPathObject(nullptr);
}

void QDemonRenderBackendGLBase::releasePathNVObject(QDemonRenderBackendPathObject, size_t)
{
    // Needs GL 4 backend
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::loadPathGlyphs(QDemonRenderBackendPathObject,
                                               QDemonRenderPathFontTarget,
                                               const void *,
                                               QDemonRenderPathFontStyleFlags,
                                               size_t,
                                               QDemonRenderPathFormatType,
                                               const void *,
                                               QDemonRenderPathMissingGlyphs,
                                               QDemonRenderBackendPathObject,
                                               float)
{
    // Needs GL 4 backend
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::loadPathGlyphRange(QDemonRenderBackendPathObject,
                                                   QDemonRenderPathFontTarget,
                                                   const void *,
                                                   QDemonRenderPathFontStyleFlags,
                                                   quint32,
                                                   size_t,
                                                   QDemonRenderPathMissingGlyphs,
                                                   QDemonRenderBackendPathObject,
                                                   float)
{
    // Needs GL 4 backend
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

QDemonRenderPathReturnValues QDemonRenderBackendGLBase::loadPathGlyphsIndexed(QDemonRenderBackendPathObject,
                                                                                    QDemonRenderPathFontTarget,
                                                                                    const void *,
                                                                                    QDemonRenderPathFontStyleFlags,
                                                                                    quint32,
                                                                                    size_t,
                                                                                    QDemonRenderBackendPathObject,
                                                                                    float)
{
    // Needs GL 4 backend
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return QDemonRenderPathReturnValues::FontUnavailable;
}

QDemonRenderBackend::QDemonRenderBackendPathObject QDemonRenderBackendGLBase::loadPathGlyphsIndexedRange(
        QDemonRenderPathFontTarget,
        const void *,
        QDemonRenderPathFontStyleFlags,
        QDemonRenderBackend::QDemonRenderBackendPathObject,
        float,
        quint32 *)
{
    return QDemonRenderBackendPathObject(nullptr);
}

void QDemonRenderBackendGLBase::getPathMetrics(QDemonRenderBackendPathObject,
                                               size_t,
                                               QDemonRenderPathGlyphFontMetricFlags,
                                               QDemonRenderPathFormatType,
                                               const void *,
                                               size_t,
                                               float *)
{
    // Needs GL 4 backend
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::getPathMetricsRange(QDemonRenderBackendPathObject, size_t, QDemonRenderPathGlyphFontMetricFlags, size_t, float *)
{
    // Needs GL 4 backend
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::getPathSpacing(QDemonRenderBackendPathObject,
                                               size_t,
                                               QDemonRenderPathListMode,
                                               QDemonRenderPathFormatType,
                                               const void *,
                                               float,
                                               float,
                                               QDemonRenderPathTransformType,
                                               float *)
{
    // Needs GL 4 backend
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::stencilFillPathInstanced(QDemonRenderBackendPathObject,
                                                         size_t,
                                                         QDemonRenderPathFormatType,
                                                         const void *,
                                                         QDemonRenderPathFillMode,
                                                         quint32,
                                                         QDemonRenderPathTransformType,
                                                         const float *)
{
    // Needs GL 4 backend
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::stencilStrokePathInstancedN(QDemonRenderBackendPathObject,
                                                            size_t,
                                                            QDemonRenderPathFormatType,
                                                            const void *,
                                                            qint32,
                                                            quint32,
                                                            QDemonRenderPathTransformType,
                                                            const float *)
{
    // Needs GL 4 backend
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::coverFillPathInstanced(QDemonRenderBackendPathObject,
                                                       size_t,
                                                       QDemonRenderPathFormatType,
                                                       const void *,
                                                       QDemonRenderPathCoverMode,
                                                       QDemonRenderPathTransformType,
                                                       const float *)
{
    // Needs GL 4 backend
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::coverStrokePathInstanced(QDemonRenderBackendPathObject,
                                                         size_t,
                                                         QDemonRenderPathFormatType,
                                                         const void *,
                                                         QDemonRenderPathCoverMode,
                                                         QDemonRenderPathTransformType,
                                                         const float *)
{
    // Needs GL 4 backend
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

///< private calls
const char *QDemonRenderBackendGLBase::getVersionString()
{
    const char *retval = (const char *)GL_CALL_FUNCTION(glGetString(GL_VERSION));
    if (retval == nullptr)
        return "";

    return retval;
}

const char *QDemonRenderBackendGLBase::getVendorString()
{
    const char *retval = (const char *)GL_CALL_FUNCTION(glGetString(GL_VENDOR));
    if (retval == nullptr)
        return "";

    return retval;
}

const char *QDemonRenderBackendGLBase::getRendererString()
{
    const char *retval = (const char *)GL_CALL_FUNCTION(glGetString(GL_RENDERER));
    if (retval == nullptr)
        return "";

    return retval;
}

const char *QDemonRenderBackendGLBase::getExtensionString()
{
    const char *retval = (const char *)GL_CALL_FUNCTION(glGetString(GL_EXTENSIONS));
    if (retval == nullptr)
        return "";

    return retval;
}

/**
 * @brief This function inspects the various strings to setup
 *		  HW capabilities of the device.
 *		  We can do a lot of smart things here based on GL version
 *		  renderer string and vendor.
 *
 * @return No return
 */
void QDemonRenderBackendGLBase::setAndInspectHardwareCaps()
{
    QByteArray apiVersion(getVersionString());
    qCInfo(TRACE_INFO, "GL version: %s", apiVersion.constData());

    // we assume all GLES versions running on mobile with shared memory
    // this means framebuffer blits are slow and should be optimized or avoided
    if (!apiVersion.contains("OpenGL ES")) {
        // no ES device
        m_backendSupport.caps.bits.bFastBlitsSupported = true;
    }
}

#ifdef RENDER_BACKEND_LOG_GL_ERRORS
void QDemonRenderBackendGLBase::checkGLError(const char *function, const char *file, const unsigned int line) const
{
    GLenum error = m_glFunctions->glGetError();
    if (error != GL_NO_ERROR) {
        qCCritical(GL_ERROR) << GLConversion::processGLError(error) << " " << function << " " << file << " " << line;
    }
}
#else
void QDemonRenderBackendGLBase::checkGLError() const
{
#if !defined(NDEBUG) || defined(_DEBUG)
    GLenum error = m_glFunctions->glGetError();
    if (error != GL_NO_ERROR)
        qCCritical(GL_ERROR) << GLConversion::processGLError(error);
#endif
}
#endif

QT_END_NAMESPACE
