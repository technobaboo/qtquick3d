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

#define GL_CALL_FUNCTION(x) m_glFunctions->x; RENDER_LOG_ERROR_PARAMS(x);
#define GL_CALL_EXTRA_FUNCTION(x) m_glExtraFunctions->x; RENDER_LOG_ERROR_PARAMS(x);

#ifndef GL_PROGRAM_SEPARABLE
#define GL_PROGRAM_SEPARABLE 0x8258
#endif

#ifndef GL_UNSIGNED_INT_IMAGE_2D
#define GL_UNSIGNED_INT_IMAGE_2D 0x9063
#endif

#ifndef GL_UNSIGNED_INT_ATOMIC_COUNTER
#define GL_UNSIGNED_INT_ATOMIC_COUNTER 0x92DB
#endif

/// constructor
QDemonRenderBackendGLBase::QDemonRenderBackendGLBase(const QSurfaceFormat &format)
    : m_Conversion()
    , m_MaxAttribCount(0)
    , m_format(format)
{
    m_glFunctions = new QOpenGLFunctions;
    m_glFunctions->initializeOpenGLFunctions();
    m_glExtraFunctions = new QOpenGLExtraFunctions;
    m_glExtraFunctions->initializeOpenGLFunctions();

    // internal state tracker
    m_pCurrentRasterizerState = new QDemonRenderBackendRasterizerStateGL();
    m_pCurrentDepthStencilState = new QDemonRenderBackendDepthStencilStateGL();
}
/// destructor
QDemonRenderBackendGLBase::~QDemonRenderBackendGLBase()
{
    if (m_pCurrentRasterizerState)
        delete m_pCurrentRasterizerState;
    if (m_pCurrentDepthStencilState)
        delete m_pCurrentDepthStencilState;
    if (m_glFunctions)
        delete m_glFunctions;
    if (m_glExtraFunctions)
        delete m_glExtraFunctions;
}

QDemonRenderContextType QDemonRenderBackendGLBase::GetRenderContextType() const
{
    if (m_format.renderableType() == QSurfaceFormat::OpenGLES) {
        if (m_format.majorVersion() == 2)
            return QDemonRenderContextValues::GLES2;

        if (m_format.majorVersion() == 3) {
            if (m_format.minorVersion() >= 1)
                return QDemonRenderContextValues::GLES3PLUS;
            else
                return QDemonRenderContextValues::GLES3;
        }
    } else if (m_format.majorVersion() == 2) {
        return QDemonRenderContextValues::GL2;
    } else if (m_format.majorVersion() == 3) {
        return QDemonRenderContextValues::GL3;
    } else if (m_format.majorVersion() == 4) {
        return QDemonRenderContextValues::GL4;
    }

    return QDemonRenderContextValues::NullContext;
}

bool QDemonRenderBackendGLBase::isESCompatible() const
{
    return m_format.renderableType() == QSurfaceFormat::OpenGLES;
}

const char *QDemonRenderBackendGLBase::GetShadingLanguageVersion()
{
    const char *retval = (const char *)GL_CALL_FUNCTION(
                glGetString(GL_SHADING_LANGUAGE_VERSION));
    if (retval == nullptr)
        return "";

    return retval;
}

quint32
QDemonRenderBackendGLBase::GetMaxCombinedTextureUnits()
{
    qint32 maxUnits;
    GL_CALL_FUNCTION(glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxUnits));
    return maxUnits;
}

bool QDemonRenderBackendGLBase::GetRenderBackendCap(
        QDemonRenderBackend::QDemonRenderBackendCaps::Enum inCap) const
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
        QDemonRenderContextType noSamplesQuerySupportedContextFlags(QDemonRenderContextValues::GL2
                                                                    | QDemonRenderContextValues::GLES2);
        QDemonRenderContextType ctxType = GetRenderContextType();
        bSupported = !(ctxType & noSamplesQuerySupportedContextFlags);
    } break;
    case QDemonRenderBackendCaps::TimerQuery:
        bSupported = m_backendSupport.caps.bits.bTimerQuerySupported;
        break;
    case QDemonRenderBackendCaps::CommandSync: {
        // On the following context sync objects are not supported
        QDemonRenderContextType noSyncObjectSupportedContextFlags(QDemonRenderContextValues::GL2
                                                                  | QDemonRenderContextValues::GLES2);
        QDemonRenderContextType ctxType = GetRenderContextType();
        bSupported = !(ctxType & noSyncObjectSupportedContextFlags);
    } break;
    case QDemonRenderBackendCaps::TextureArray: {
        // On the following context texture arrays are not supported
        QDemonRenderContextType noTextureArraySupportedContextFlags(QDemonRenderContextValues::GL2
                                                                    | QDemonRenderContextValues::GLES2);
        QDemonRenderContextType ctxType = GetRenderContextType();
        bSupported = !(ctxType & noTextureArraySupportedContextFlags);
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
        bSupported = m_backendSupport.caps.bits.bNVAdvancedBlendSupported |
                m_backendSupport.caps.bits.bKHRAdvancedBlendSupported;
        break;
    case QDemonRenderBackendCaps::AdvancedBlendKHR:
        bSupported = m_backendSupport.caps.bits.bKHRAdvancedBlendSupported;
        break;
    case QDemonRenderBackendCaps::BlendCoherency:
        bSupported = m_backendSupport.caps.bits.bNVBlendCoherenceSupported |
                m_backendSupport.caps.bits.bKHRBlendCoherenceSupported;
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

void QDemonRenderBackendGLBase::GetRenderBackendValue(QDemonRenderBackendQuery::Enum inQuery,
                                                      qint32 *params) const
{
    if (params) {
        switch (inQuery) {
        case QDemonRenderBackendQuery::MaxTextureSize:
            GL_CALL_FUNCTION(glGetIntegerv(GL_MAX_TEXTURE_SIZE, params));
            break;
        case QDemonRenderBackendQuery::MaxTextureArrayLayers: {
            QDemonRenderContextType noTextureArraySupportedContextFlags(
                        QDemonRenderContextValues::GL2 | QDemonRenderContextValues::GLES2);
            QDemonRenderContextType ctxType = GetRenderContextType();
            if (!(ctxType & noTextureArraySupportedContextFlags)) {
                GL_CALL_FUNCTION(glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, params));
            } else {
                *params = 0;
            }
        } break;
        case QDemonRenderBackendQuery::MaxConstantBufferSlots: {
            QDemonRenderContextType noConstantBufferSupportedContextFlags(
                        QDemonRenderContextValues::GL2 | QDemonRenderContextValues::GLES2);
            QDemonRenderContextType ctxType = GetRenderContextType();
            if (!(ctxType & noConstantBufferSupportedContextFlags)) {
                GL_CALL_FUNCTION(glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, params));
            } else {
                *params = 0;
            }
        } break;
        case QDemonRenderBackendQuery::MaxConstantBufferBlockSize: {
            QDemonRenderContextType noConstantBufferSupportedContextFlags(
                        QDemonRenderContextValues::GL2 | QDemonRenderContextValues::GLES2);
            QDemonRenderContextType ctxType = GetRenderContextType();
            if (!(ctxType & noConstantBufferSupportedContextFlags)) {
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

quint32
QDemonRenderBackendGLBase::GetDepthBits() const
{
    qint32 depthBits;
    GL_CALL_FUNCTION(glGetIntegerv(GL_DEPTH_BITS, &depthBits));
    return depthBits;
}

quint32
QDemonRenderBackendGLBase::GetStencilBits() const
{
    qint32 stencilBits;
    GL_CALL_FUNCTION(glGetIntegerv(GL_STENCIL_BITS, &stencilBits));
    return stencilBits;
}

void QDemonRenderBackendGLBase::SetMultisample(bool bEnable)
{
    Q_ASSERT(m_backendSupport.caps.bits.bMsTextureSupported || !bEnable);
    // For GL ES explicit multisample enabling is not needed
    // and does not exist
    QDemonRenderContextType noMsaaEnableContextFlags(QDemonRenderContextValues::GLES2
                                                     | QDemonRenderContextValues::GLES3
                                                     | QDemonRenderContextValues::GLES3PLUS);
    QDemonRenderContextType ctxType = GetRenderContextType();
    if (!(ctxType & noMsaaEnableContextFlags)) {
        SetRenderState(bEnable, QDemonRenderState::Multisample);
    }
}

void QDemonRenderBackendGLBase::SetRenderState(bool bEnable, const QDemonRenderState::Enum value)
{
    if (value == QDemonRenderState::DepthWrite) {
        GL_CALL_FUNCTION(glDepthMask(bEnable));
    } else {
        if (bEnable) {
            GL_CALL_FUNCTION(glEnable(m_Conversion.fromRenderStateToGL(value)));
        } else {
            GL_CALL_FUNCTION(glDisable(m_Conversion.fromRenderStateToGL(value)));
        }
    }
}

QDemonRenderBackend::QDemonRenderBackendDepthStencilStateObject
QDemonRenderBackendGLBase::CreateDepthStencilState(
        bool enableDepth, bool depthMask, QDemonRenderBoolOp::Enum depthFunc, bool enableStencil,
        QDemonRenderStencilFunctionArgument &stencilFuncFront,
        QDemonRenderStencilFunctionArgument &stencilFuncBack,
        QDemonRenderStencilOperationArgument &depthStencilOpFront,
        QDemonRenderStencilOperationArgument &depthStencilOpBack)
{
    QDemonRenderBackendDepthStencilStateGL *retval = new QDemonRenderBackendDepthStencilStateGL(
                enableDepth, depthMask, depthFunc, enableStencil, stencilFuncFront, stencilFuncBack,
                depthStencilOpFront, depthStencilOpBack);

    return (QDemonRenderBackend::QDemonRenderBackendDepthStencilStateObject)retval;
}

void QDemonRenderBackendGLBase::ReleaseDepthStencilState(
        QDemonRenderBackendDepthStencilStateObject inDepthStencilState)
{
    QDemonRenderBackendDepthStencilStateGL *inputState =
            (QDemonRenderBackendDepthStencilStateGL *)inDepthStencilState;
    if (inputState)
        delete inputState;
}

QDemonRenderBackend::QDemonRenderBackendRasterizerStateObject
QDemonRenderBackendGLBase::CreateRasterizerState(float depthBias, float depthScale,
                                                 QDemonRenderFaces::Enum cullFace)
{
    QDemonRenderBackendRasterizerStateGL *retval = new QDemonRenderBackendRasterizerStateGL(depthBias, depthScale, cullFace);

    return (QDemonRenderBackend::QDemonRenderBackendRasterizerStateObject)retval;
}

void QDemonRenderBackendGLBase::ReleaseRasterizerState(
        QDemonRenderBackendRasterizerStateObject rasterizerState)
{
    QDemonRenderBackendRasterizerStateGL *inputState =
            (QDemonRenderBackendRasterizerStateGL *)rasterizerState;
    if (inputState)
        delete inputState;
}

void QDemonRenderBackendGLBase::SetDepthStencilState(
        QDemonRenderBackendDepthStencilStateObject inDepthStencilState)
{
    QDemonRenderBackendDepthStencilStateGL *inputState =
            (QDemonRenderBackendDepthStencilStateGL *)inDepthStencilState;
    if (inputState && !(*m_pCurrentDepthStencilState == *inputState)) {
        // we check on a per single state base
        if (inputState->m_DepthEnable != m_pCurrentDepthStencilState->m_DepthEnable) {
            SetRenderState(inputState->m_DepthEnable, QDemonRenderState::DepthTest);
            m_pCurrentDepthStencilState->m_DepthEnable = inputState->m_DepthEnable;
        }
        if (inputState->m_StencilEnable != m_pCurrentDepthStencilState->m_StencilEnable) {
            SetRenderState(inputState->m_StencilEnable, QDemonRenderState::StencilTest);
            m_pCurrentDepthStencilState->m_StencilEnable = inputState->m_StencilEnable;
        }

        if (inputState->m_DepthMask != m_pCurrentDepthStencilState->m_DepthMask) {
            GL_CALL_FUNCTION(glDepthMask(inputState->m_DepthMask));
            m_pCurrentDepthStencilState->m_DepthMask = inputState->m_DepthMask;
        }

        if (inputState->m_DepthFunc != m_pCurrentDepthStencilState->m_DepthFunc) {
            GL_CALL_FUNCTION(glDepthFunc(m_Conversion.fromBoolOpToGL(inputState->m_DepthFunc)));
            m_pCurrentDepthStencilState->m_DepthFunc = inputState->m_DepthFunc;
        }

        if (!(inputState->m_DepthStencilOpFront
              == m_pCurrentDepthStencilState->m_DepthStencilOpFront)) {
            GL_CALL_FUNCTION(glStencilOpSeparate(
                                 GL_FRONT,
                                 m_Conversion.fromStencilOpToGL(inputState->m_DepthStencilOpFront.m_StencilFail),
                                 m_Conversion.fromStencilOpToGL(inputState->m_DepthStencilOpFront.m_DepthFail),
                                 m_Conversion.fromStencilOpToGL(inputState->m_DepthStencilOpFront.m_DepthPass)));
            m_pCurrentDepthStencilState->m_DepthStencilOpFront =
                    inputState->m_DepthStencilOpFront;
        }

        if (!(inputState->m_DepthStencilOpBack
              == m_pCurrentDepthStencilState->m_DepthStencilOpBack)) {
            GL_CALL_FUNCTION(glStencilOpSeparate(
                                 GL_BACK,
                                 m_Conversion.fromStencilOpToGL(inputState->m_DepthStencilOpBack.m_StencilFail),
                                 m_Conversion.fromStencilOpToGL(inputState->m_DepthStencilOpBack.m_DepthFail),
                                 m_Conversion.fromStencilOpToGL(inputState->m_DepthStencilOpBack.m_DepthPass)));
            m_pCurrentDepthStencilState->m_DepthStencilOpBack =
                    inputState->m_DepthStencilOpBack;
        }

        if (!(inputState->m_StencilFuncFront
              == m_pCurrentDepthStencilState->m_StencilFuncFront)) {
            GL_CALL_FUNCTION(glStencilFuncSeparate(
                                 GL_FRONT,
                                 m_Conversion.fromBoolOpToGL(inputState->m_StencilFuncFront.m_Function),
                                 inputState->m_StencilFuncFront.m_ReferenceValue,
                                 inputState->m_StencilFuncFront.m_Mask));
            m_pCurrentDepthStencilState->m_StencilFuncFront = inputState->m_StencilFuncFront;
        }

        if (!(inputState->m_StencilFuncBack
              == m_pCurrentDepthStencilState->m_StencilFuncBack)) {
            GL_CALL_FUNCTION(glStencilFuncSeparate(
                                 GL_BACK, m_Conversion.fromBoolOpToGL(inputState->m_StencilFuncBack.m_Function),
                                 inputState->m_StencilFuncBack.m_ReferenceValue,
                                 inputState->m_StencilFuncBack.m_Mask));
            m_pCurrentDepthStencilState->m_StencilFuncBack = inputState->m_StencilFuncBack;
        }
    }
}

void
QDemonRenderBackendGLBase::SetRasterizerState(QDemonRenderBackendRasterizerStateObject rasterizerState)
{
    QDemonRenderBackendRasterizerStateGL *inputState =
            (QDemonRenderBackendRasterizerStateGL *)rasterizerState;
    if (inputState && !(*m_pCurrentRasterizerState == *inputState)) {
        // store current state
        *m_pCurrentRasterizerState = *inputState;

        if (m_pCurrentRasterizerState->m_DepthBias != 0.0
                || m_pCurrentRasterizerState->m_DepthScale != 0.0) {
            GL_CALL_FUNCTION(glEnable(GL_POLYGON_OFFSET_FILL));
        } else {
            GL_CALL_FUNCTION(glDisable(GL_POLYGON_OFFSET_FILL));
        }

        GL_CALL_FUNCTION(glPolygonOffset(m_pCurrentRasterizerState->m_DepthBias,
                                         m_pCurrentRasterizerState->m_DepthScale));

        GL_CALL_FUNCTION(
                    glCullFace(m_Conversion.fromFacesToGL(m_pCurrentRasterizerState->m_CullFace)));
    }
}

bool QDemonRenderBackendGLBase::GetRenderState(const QDemonRenderState::Enum value)
{
    bool enabled = GL_CALL_FUNCTION(glIsEnabled(m_Conversion.fromRenderStateToGL(value)));
    return enabled;
}

QDemonRenderBoolOp::Enum QDemonRenderBackendGLBase::GetDepthFunc()
{
    qint32 value;
    GL_CALL_FUNCTION(glGetIntegerv(GL_DEPTH_FUNC, &value));
    return m_Conversion.fromGLToBoolOp(value);
}

void QDemonRenderBackendGLBase::SetDepthFunc(const QDemonRenderBoolOp::Enum func)
{
    GL_CALL_FUNCTION(glDepthFunc(m_Conversion.fromBoolOpToGL(func)));
}

bool QDemonRenderBackendGLBase::GetDepthWrite()
{
    qint32 value;
    GL_CALL_FUNCTION(glGetIntegerv(GL_DEPTH_WRITEMASK, (GLint *)&value));
    return value ? true : false;
}

void QDemonRenderBackendGLBase::SetDepthWrite(bool bEnable) { GL_CALL_FUNCTION(glDepthMask(bEnable)); }

void QDemonRenderBackendGLBase::SetColorWrites(bool bRed, bool bGreen, bool bBlue, bool bAlpha)
{
    GL_CALL_FUNCTION(glColorMask(bRed, bGreen, bBlue, bAlpha));
}

void QDemonRenderBackendGLBase::GetBlendFunc(QDemonRenderBlendFunctionArgument *pBlendFuncArg)
{
    Q_ASSERT(pBlendFuncArg);
    qint32_4 values;

    GL_CALL_FUNCTION(glGetIntegerv(GL_BLEND_SRC_RGB, (GLint *)&values.x));
    GL_CALL_FUNCTION(glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint *)&values.y));
    GL_CALL_FUNCTION(glGetIntegerv(GL_BLEND_DST_RGB, (GLint *)&values.z));
    GL_CALL_FUNCTION(glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint *)&values.w));

    pBlendFuncArg->m_SrcRGB = m_Conversion.fromGLToSrcBlendFunc(values.x);
    pBlendFuncArg->m_SrcAlpha = m_Conversion.fromGLToSrcBlendFunc(values.y);
    pBlendFuncArg->m_DstRGB = m_Conversion.fromGLToDstBlendFunc(values.z);
    pBlendFuncArg->m_DstAlpha = m_Conversion.fromGLToDstBlendFunc(values.w);
}

void QDemonRenderBackendGLBase::SetBlendFunc(const QDemonRenderBlendFunctionArgument &blendFuncArg)
{
    qint32_4 values;

    values.x = m_Conversion.fromSrcBlendFuncToGL(blendFuncArg.m_SrcRGB);
    values.y = m_Conversion.fromDstBlendFuncToGL(blendFuncArg.m_DstRGB);
    values.z = m_Conversion.fromSrcBlendFuncToGL(blendFuncArg.m_SrcAlpha);
    values.w = m_Conversion.fromDstBlendFuncToGL(blendFuncArg.m_DstAlpha);

    GL_CALL_FUNCTION(glBlendFuncSeparate(values.x, values.y, values.z, values.w));
}

void QDemonRenderBackendGLBase::SetBlendEquation(const QDemonRenderBlendEquationArgument &)
{
    // needs GL4 / GLES 3.1
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::SetBlendBarrier()
{
    // needs GL4 / GLES 3.1
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::GetScissorRect(QDemonRenderRect *pRect)
{
    Q_ASSERT(pRect);
    GL_CALL_FUNCTION(glGetIntegerv(GL_SCISSOR_BOX, (GLint *)pRect));
}

void QDemonRenderBackendGLBase::SetScissorRect(const QDemonRenderRect &rect)
{
    GL_CALL_FUNCTION(glScissor(rect.m_X, rect.m_Y, rect.m_Width, rect.m_Height));
}

void QDemonRenderBackendGLBase::GetViewportRect(QDemonRenderRect *pRect)
{
    Q_ASSERT(pRect);
    GL_CALL_FUNCTION(glGetIntegerv(GL_VIEWPORT, (GLint *)pRect));
}

void QDemonRenderBackendGLBase::SetViewportRect(const QDemonRenderRect &rect)
{
    GL_CALL_FUNCTION(glViewport(rect.m_X, rect.m_Y, rect.m_Width, rect.m_Height););
}

void QDemonRenderBackendGLBase::SetClearColor(const QVector4D *pClearColor)
{
    Q_ASSERT(pClearColor);

    GL_CALL_FUNCTION(glClearColor(pClearColor->x(), pClearColor->y(),
                                  pClearColor->z(), pClearColor->w()));
}

void QDemonRenderBackendGLBase::Clear(QDemonRenderClearFlags flags)
{
    GL_CALL_FUNCTION(glClear(m_Conversion.fromClearFlagsToGL(flags)));
}

QDemonRenderBackend::QDemonRenderBackendBufferObject
QDemonRenderBackendGLBase::CreateBuffer(size_t size, QDemonRenderBufferBindFlags bindFlags,
                                        QDemonRenderBufferUsageType::Enum usage, const void *hostPtr)
{
    GLuint bufID = 0;

    GL_CALL_FUNCTION(glGenBuffers(1, &bufID));

    if (bufID && size) {
        GLenum target = m_Conversion.fromBindBufferFlagsToGL(bindFlags);
        if (target != GL_INVALID_ENUM) {
            GL_CALL_FUNCTION(glBindBuffer(target, bufID));
            GL_CALL_FUNCTION(glBufferData(target, size, hostPtr,
                                          m_Conversion.fromBufferUsageTypeToGL(usage)));
        } else {
            GL_CALL_FUNCTION(glDeleteBuffers(1, &bufID));
            bufID = 0;
            qCCritical(GL_ERROR, GLConversion::processGLError(target));
        }
    }

    return (QDemonRenderBackend::QDemonRenderBackendBufferObject)bufID;
}

void QDemonRenderBackendGLBase::BindBuffer(QDemonRenderBackendBufferObject bo,
                                           QDemonRenderBufferBindFlags bindFlags)
{
    GLuint bufID = HandleToID_cast(GLuint, size_t, bo);
    GL_CALL_FUNCTION(glBindBuffer(m_Conversion.fromBindBufferFlagsToGL(bindFlags), bufID));
}

void QDemonRenderBackendGLBase::ReleaseBuffer(QDemonRenderBackendBufferObject bo)
{
    GLuint bufID = HandleToID_cast(GLuint, size_t, bo);
    GL_CALL_FUNCTION(glDeleteBuffers(1, &bufID));
}

void QDemonRenderBackendGLBase::UpdateBuffer(QDemonRenderBackendBufferObject bo,
                                             QDemonRenderBufferBindFlags bindFlags, size_t size,
                                             QDemonRenderBufferUsageType::Enum usage, const void *data)
{
    GLuint bufID = HandleToID_cast(GLuint, size_t, bo);
    GLenum target = m_Conversion.fromBindBufferFlagsToGL(bindFlags);
    GL_CALL_FUNCTION(glBindBuffer(target, bufID));
    GL_CALL_FUNCTION(glBufferData(target, size, data, m_Conversion.fromBufferUsageTypeToGL(usage)));
}

void QDemonRenderBackendGLBase::UpdateBufferRange(QDemonRenderBackendBufferObject bo,
                                                  QDemonRenderBufferBindFlags bindFlags, size_t offset,
                                                  size_t size, const void *data)
{
    GLuint bufID = HandleToID_cast(GLuint, size_t, bo);
    GLenum target = m_Conversion.fromBindBufferFlagsToGL(bindFlags);
    GL_CALL_FUNCTION(glBindBuffer(target, bufID));
    GL_CALL_FUNCTION(glBufferSubData(target, offset, size, data));
}

void *QDemonRenderBackendGLBase::MapBuffer(QDemonRenderBackendBufferObject, QDemonRenderBufferBindFlags,
                                           size_t, size_t, QDemonRenderBufferAccessFlags)
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return nullptr;
}

bool QDemonRenderBackendGLBase::UnmapBuffer(QDemonRenderBackendBufferObject, QDemonRenderBufferBindFlags)
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return true;
}

void QDemonRenderBackendGLBase::SetMemoryBarrier(QDemonRenderBufferBarrierFlags)
{
    // needs GL 4 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

QDemonRenderBackend::QDemonRenderBackendQueryObject QDemonRenderBackendGLBase::CreateQuery()
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return QDemonRenderBackendQueryObject(0);
}

void QDemonRenderBackendGLBase::ReleaseQuery(QDemonRenderBackendQueryObject)
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::BeginQuery(QDemonRenderBackendQueryObject, QDemonRenderQueryType::Enum)
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::EndQuery(QDemonRenderBackendQueryObject, QDemonRenderQueryType::Enum)
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::GetQueryResult(QDemonRenderBackendQueryObject,
                                               QDemonRenderQueryResultType::Enum, quint32 *)
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::GetQueryResult(QDemonRenderBackendQueryObject,
                                               QDemonRenderQueryResultType::Enum, quint64 *)
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::SetQueryTimer(QDemonRenderBackendQueryObject)
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

QDemonRenderBackend::QDemonRenderBackendSyncObject
QDemonRenderBackendGLBase::CreateSync(QDemonRenderSyncType::Enum, QDemonRenderSyncFlags)
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return QDemonRenderBackendSyncObject(0);
}

void QDemonRenderBackendGLBase::ReleaseSync(QDemonRenderBackendSyncObject)
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::WaitSync(QDemonRenderBackendSyncObject, QDemonRenderCommandFlushFlags,
                                         quint64)
{
    // needs GL 3 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

QDemonRenderBackend::QDemonRenderBackendRenderTargetObject QDemonRenderBackendGLBase::CreateRenderTarget()
{
    GLuint fboID = 0;

    GL_CALL_FUNCTION(glGenFramebuffers(1, &fboID));

    return reinterpret_cast<QDemonRenderBackend::QDemonRenderBackendRenderTargetObject>(fboID);
}

void QDemonRenderBackendGLBase::ReleaseRenderTarget(QDemonRenderBackendRenderTargetObject rto)
{
    GLuint fboID = HandleToID_cast(GLuint, size_t, rto);

    if (fboID) {
        GL_CALL_FUNCTION(glDeleteFramebuffers(1, &fboID));
    }
}

void QDemonRenderBackendGLBase::RenderTargetAttach(QDemonRenderBackendRenderTargetObject /* rto */,
                                                   QDemonRenderFrameBufferAttachments::Enum attachment,
                                                   QDemonRenderBackendRenderbufferObject rbo)
{
    // rto must be the current render target
    GLuint rbID = HandleToID_cast(GLuint, size_t, rbo);

    GLenum glAttach = GLConversion::fromFramebufferAttachmentsToGL(attachment);

    GL_CALL_FUNCTION(glFramebufferRenderbuffer(GL_FRAMEBUFFER, glAttach, GL_RENDERBUFFER, rbID));
}

void QDemonRenderBackendGLBase::RenderTargetAttach(QDemonRenderBackendRenderTargetObject /* rto */,
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

    GL_CALL_FUNCTION(glFramebufferTexture2D(GL_FRAMEBUFFER, glAttach, glTarget, texID, 0))
}

void QDemonRenderBackendGLBase::RenderTargetAttach(QDemonRenderBackendRenderTargetObject,
                                                   QDemonRenderFrameBufferAttachments::Enum,
                                                   QDemonRenderBackendTextureObject, qint32, qint32)
{
    // Needs GL3 or GLES 3
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::SetRenderTarget(QDemonRenderBackendRenderTargetObject rto)
{
    GLuint fboID = HandleToID_cast(GLuint, size_t, rto);

    GL_CALL_FUNCTION(glBindFramebuffer(GL_FRAMEBUFFER, fboID));
}

bool QDemonRenderBackendGLBase::RenderTargetIsValid(QDemonRenderBackendRenderTargetObject /* rto */)
{
    // rto must be the current render target
    GLenum completeStatus = GL_CALL_FUNCTION(glCheckFramebufferStatus(GL_FRAMEBUFFER));
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
QDemonRenderBackendGLBase::CreateRenderbuffer(QDemonRenderRenderBufferFormats::Enum storageFormat,
                                              size_t width, size_t height)
{
    GLuint bufID = 0;

    GL_CALL_FUNCTION(glGenRenderbuffers(1, &bufID));
    GL_CALL_FUNCTION(glBindRenderbuffer(GL_RENDERBUFFER, bufID));
    GL_CALL_FUNCTION(glRenderbufferStorage(GL_RENDERBUFFER,
                                           GLConversion::fromRenderBufferFormatsToRenderBufferGL(storageFormat),
                                           (GLsizei)width, (GLsizei)height));

    // check for error
    GLenum error = m_glFunctions->glGetError();
    if (error != GL_NO_ERROR) {
        qCCritical(GL_ERROR, GLConversion::processGLError(error));
        Q_ASSERT(false);
        GL_CALL_FUNCTION(glDeleteRenderbuffers(1, &bufID));
        bufID = 0;
    }

    GL_CALL_FUNCTION(glBindRenderbuffer(GL_RENDERBUFFER, 0));

    return (QDemonRenderBackend::QDemonRenderBackendRenderbufferObject)bufID;
}

void QDemonRenderBackendGLBase::ReleaseRenderbuffer(QDemonRenderBackendRenderbufferObject rbo)
{
    GLuint bufID = HandleToID_cast(GLuint, size_t, rbo);

    if (bufID) {
        GL_CALL_FUNCTION(glDeleteRenderbuffers(1, &bufID));
    }
}

bool QDemonRenderBackendGLBase::ResizeRenderbuffer(QDemonRenderBackendRenderbufferObject rbo,
                                                   QDemonRenderRenderBufferFormats::Enum storageFormat,
                                                   size_t width, size_t height)
{
    bool success = true;
    GLuint bufID = HandleToID_cast(GLuint, size_t, rbo);

    Q_ASSERT(bufID);

    GL_CALL_FUNCTION(glBindRenderbuffer(GL_RENDERBUFFER, bufID));
    GL_CALL_FUNCTION(glRenderbufferStorage(GL_RENDERBUFFER,
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

QDemonRenderBackend::QDemonRenderBackendTextureObject QDemonRenderBackendGLBase::CreateTexture()
{
    GLuint texID = 0;

    GL_CALL_FUNCTION(glGenTextures(1, &texID));
    return (QDemonRenderBackend::QDemonRenderBackendTextureObject)texID;
}

void QDemonRenderBackendGLBase::BindTexture(QDemonRenderBackendTextureObject to,
                                            QDemonRenderTextureTargetType::Enum target, quint32 unit)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GL_CALL_FUNCTION(glActiveTexture(GL_TEXTURE0 + unit));
    GL_CALL_FUNCTION(glBindTexture(m_Conversion.fromTextureTargetToGL(target), texID));
}

void QDemonRenderBackendGLBase::BindImageTexture(QDemonRenderBackendTextureObject, quint32, qint32, bool,
                                                 qint32, QDemonRenderImageAccessType::Enum,
                                                 QDemonRenderTextureFormats::Enum)
{
    // needs GL 4 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::ReleaseTexture(QDemonRenderBackendTextureObject to)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GL_CALL_FUNCTION(glDeleteTextures(1, &texID));
}

void QDemonRenderBackendGLBase::SetTextureData2D(
        QDemonRenderBackendTextureObject to, QDemonRenderTextureTargetType::Enum target, quint32 level,
        QDemonRenderTextureFormats::Enum internalFormat, size_t width, size_t height, qint32 border,
        QDemonRenderTextureFormats::Enum format, const void *hostPtr)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GLenum glTarget = m_Conversion.fromTextureTargetToGL(target);
    GL_CALL_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_FUNCTION(glBindTexture(glTarget, texID));
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

    GL_CALL_FUNCTION(glTexImage2D(glTarget, level, glInternalFormat, (GLsizei)width, (GLsizei)height,
                                  border, glformat, gltype, hostPtr));

    GL_CALL_FUNCTION(glBindTexture(glTarget, 0));
}

// This will look very SetTextureData2D, but the target for glBindTexture will be different from
// the target for
// glTexImage2D.
void QDemonRenderBackendGLBase::SetTextureDataCubeFace(
        QDemonRenderBackendTextureObject to, QDemonRenderTextureTargetType::Enum target, quint32 level,
        QDemonRenderTextureFormats::Enum internalFormat, size_t width, size_t height, qint32 border,
        QDemonRenderTextureFormats::Enum format, const void *hostPtr)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GLenum glTarget = m_Conversion.fromTextureTargetToGL(target);
    GLenum glTexTarget =
            m_Conversion.fromTextureTargetToGL(QDemonRenderTextureTargetType::TextureCube);
    GL_CALL_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_FUNCTION(glBindTexture(glTexTarget, texID));
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

    // for es2 internal format must be same as format
    if (GetRenderContextType() == QDemonRenderContextValues::GLES2)
        glInternalFormat = glformat;

    GL_CALL_FUNCTION(glTexImage2D(glTarget, level, glInternalFormat, (GLsizei)width, (GLsizei)height,
                                  border, glformat, gltype, hostPtr));

    GL_CALL_FUNCTION(glBindTexture(glTexTarget, 0));
}

void QDemonRenderBackendGLBase::CreateTextureStorage2D(QDemonRenderBackendTextureObject,
                                                       QDemonRenderTextureTargetType::Enum, quint32,
                                                       QDemonRenderTextureFormats::Enum, size_t, size_t)
{
    // you need GL 4.2 or GLES 3.1
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::SetTextureSubData2D(QDemonRenderBackendTextureObject to,
                                                    QDemonRenderTextureTargetType::Enum target,
                                                    quint32 level, qint32 xOffset, qint32 yOffset,
                                                    size_t width, size_t height,
                                                    QDemonRenderTextureFormats::Enum format,
                                                    const void *hostPtr)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GLenum glTarget = m_Conversion.fromTextureTargetToGL(target);
    GL_CALL_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_FUNCTION(glBindTexture(glTarget, texID));

    QDemonRenderTextureSwizzleMode::Enum swizzleMode = QDemonRenderTextureSwizzleMode::NoSwizzle;
    format = m_Conversion.replaceDeprecatedTextureFormat(GetRenderContextType(), format,
                                                         swizzleMode);

    GLenum glformat = 0, glInternalFormat = 0, gltype = 0;
    m_Conversion.fromUncompressedTextureFormatToGL(GetRenderContextType(), format, glformat,
                                                   gltype, glInternalFormat);
    GL_CALL_FUNCTION(glTexSubImage2D(glTarget, level, xOffset, yOffset, (GLsizei)width,
                                     (GLsizei)height, glformat, gltype, hostPtr));

    GL_CALL_FUNCTION(glBindTexture(glTarget, 0));
}

void QDemonRenderBackendGLBase::SetCompressedTextureData2D(
        QDemonRenderBackendTextureObject to, QDemonRenderTextureTargetType::Enum target, quint32 level,
        QDemonRenderTextureFormats::Enum internalFormat, size_t width, size_t height, qint32 border,
        size_t imageSize, const void *hostPtr)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GLenum glTarget = m_Conversion.fromTextureTargetToGL(target);
    GL_CALL_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_FUNCTION(glBindTexture(glTarget, texID));

    GLenum glformat = m_Conversion.fromCompressedTextureFormatToGL(internalFormat);
    GL_CALL_FUNCTION(glCompressedTexImage2D(glTarget, level, glformat, (GLsizei)width,
                                            (GLsizei)height, border, (GLsizei)imageSize, hostPtr));

    GL_CALL_FUNCTION(glBindTexture(glTarget, 0));
}

void QDemonRenderBackendGLBase::SetCompressedTextureDataCubeFace(
        QDemonRenderBackendTextureObject to, QDemonRenderTextureTargetType::Enum target, quint32 level,
        QDemonRenderTextureFormats::Enum internalFormat, size_t width, size_t height, qint32 border,
        size_t imageSize, const void *hostPtr)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GLenum glTarget = m_Conversion.fromTextureTargetToGL(target);
    GLenum glTexTarget =
            m_Conversion.fromTextureTargetToGL(QDemonRenderTextureTargetType::TextureCube);
    GL_CALL_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_FUNCTION(glBindTexture(glTexTarget, texID));

    GLenum glformat = m_Conversion.fromCompressedTextureFormatToGL(internalFormat);
    GL_CALL_FUNCTION(glCompressedTexImage2D(glTarget, level, glformat, (GLsizei)width,
                                            (GLsizei)height, border, (GLsizei)imageSize, hostPtr));

    GL_CALL_FUNCTION(glBindTexture(glTexTarget, 0));
}

void QDemonRenderBackendGLBase::SetCompressedTextureSubData2D(
        QDemonRenderBackendTextureObject to, QDemonRenderTextureTargetType::Enum target, quint32 level,
        qint32 xOffset, qint32 yOffset, size_t width, size_t height,
        QDemonRenderTextureFormats::Enum format, size_t imageSize, const void *hostPtr)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GLenum glTarget = m_Conversion.fromTextureTargetToGL(target);
    GL_CALL_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_FUNCTION(glBindTexture(glTarget, texID));

    GLenum glformat = m_Conversion.fromCompressedTextureFormatToGL(format);
    GL_CALL_FUNCTION(glCompressedTexSubImage2D(glTarget, level, xOffset, yOffset, (GLsizei)width,
                                               (GLsizei)height, glformat, (GLsizei)imageSize,
                                               hostPtr));

    GL_CALL_FUNCTION(glBindTexture(glTarget, 0));
}

void QDemonRenderBackendGLBase::SetTextureData3D(QDemonRenderBackendTextureObject,
                                                 QDemonRenderTextureTargetType::Enum, quint32,
                                                 QDemonRenderTextureFormats::Enum, size_t, size_t,
                                                 size_t, qint32, QDemonRenderTextureFormats::Enum,
                                                 const void *)
{
    // needs GL3 or GLES3
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::GenerateMipMaps(QDemonRenderBackendTextureObject to,
                                                QDemonRenderTextureTargetType::Enum target,
                                                QDemonRenderHint::Enum genType)
{
    GLuint texID = HandleToID_cast(GLuint, size_t, to);
    GLenum glTarget = m_Conversion.fromTextureTargetToGL(target);
    GL_CALL_FUNCTION(glActiveTexture(GL_TEXTURE0));
    GL_CALL_FUNCTION(glBindTexture(glTarget, texID));

    GLenum glValue = GLConversion::fromHintToGL(genType);
    GL_CALL_FUNCTION(glHint(GL_GENERATE_MIPMAP_HINT, glValue));
    GL_CALL_FUNCTION(glGenerateMipmap(glTarget));

    GL_CALL_FUNCTION(glBindTexture(glTarget, 0));
}

QDemonRenderTextureSwizzleMode::Enum
QDemonRenderBackendGLBase::GetTextureSwizzleMode(const QDemonRenderTextureFormats::Enum inFormat) const
{
    QDemonRenderTextureSwizzleMode::Enum swizzleMode = QDemonRenderTextureSwizzleMode::NoSwizzle;
    m_Conversion.replaceDeprecatedTextureFormat(GetRenderContextType(), inFormat, swizzleMode);

    return swizzleMode;
}

QDemonRenderBackend::QDemonRenderBackendSamplerObject QDemonRenderBackendGLBase::CreateSampler(
        QDemonRenderTextureMinifyingOp::Enum minFilter, QDemonRenderTextureMagnifyingOp::Enum magFilter,
        QDemonRenderTextureCoordOp::Enum wrapS, QDemonRenderTextureCoordOp::Enum wrapT,
        QDemonRenderTextureCoordOp::Enum wrapR, qint32 minLod, qint32 maxLod, float lodBias,
        QDemonRenderTextureCompareMode::Enum compareMode, QDemonRenderTextureCompareOp::Enum compareFunc,
        float anisotropy, float *borderColor)
{
    // Satisfy the compiler
    // We don"t setup the state here for GL
    // but we need to pass on the variables here
    // to satisfy the interface
    NVRENDER_BACKEND_UNUSED(minFilter);
    NVRENDER_BACKEND_UNUSED(magFilter);
    NVRENDER_BACKEND_UNUSED(wrapS);
    NVRENDER_BACKEND_UNUSED(wrapT);
    NVRENDER_BACKEND_UNUSED(wrapR);
    NVRENDER_BACKEND_UNUSED(minLod);
    NVRENDER_BACKEND_UNUSED(maxLod);
    NVRENDER_BACKEND_UNUSED(lodBias);
    NVRENDER_BACKEND_UNUSED(compareMode);
    NVRENDER_BACKEND_UNUSED(compareFunc);
    NVRENDER_BACKEND_UNUSED(anisotropy);
    NVRENDER_BACKEND_UNUSED(borderColor);

    // return a dummy handle
    return (QDemonRenderBackend::QDemonRenderBackendSamplerObject)0x0001;
}

void QDemonRenderBackendGLBase::UpdateSampler(
        QDemonRenderBackendSamplerObject /* so */, QDemonRenderTextureTargetType::Enum target,
        QDemonRenderTextureMinifyingOp::Enum minFilter, QDemonRenderTextureMagnifyingOp::Enum magFilter,
        QDemonRenderTextureCoordOp::Enum wrapS, QDemonRenderTextureCoordOp::Enum wrapT,
        QDemonRenderTextureCoordOp::Enum wrapR, float minLod, float maxLod, float lodBias,
        QDemonRenderTextureCompareMode::Enum compareMode, QDemonRenderTextureCompareOp::Enum compareFunc,
        float anisotropy, float *borderColor)
{
    // Satisfy the compiler
    // These are not available in GLES 2 and we don't use them right now
    NVRENDER_BACKEND_UNUSED(wrapR);
    NVRENDER_BACKEND_UNUSED(lodBias);
    NVRENDER_BACKEND_UNUSED(minLod);
    NVRENDER_BACKEND_UNUSED(maxLod);
    NVRENDER_BACKEND_UNUSED(compareMode);
    NVRENDER_BACKEND_UNUSED(compareFunc);
    NVRENDER_BACKEND_UNUSED(borderColor);

    GLenum glTarget = m_Conversion.fromTextureTargetToGL(target);

    GL_CALL_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_MIN_FILTER,
                                     m_Conversion.fromTextureMinifyingOpToGL(minFilter)));
    GL_CALL_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_MAG_FILTER,
                                     m_Conversion.fromTextureMagnifyingOpToGL(magFilter)));
    GL_CALL_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_WRAP_S,
                                     m_Conversion.fromTextureCoordOpToGL(wrapS)));
    GL_CALL_FUNCTION(glTexParameteri(glTarget, GL_TEXTURE_WRAP_T,
                                     m_Conversion.fromTextureCoordOpToGL(wrapT)));
    if (m_backendSupport.caps.bits.bAnistropySupported) {
        GL_CALL_FUNCTION(glTexParameterf(glTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy));
    }
}

void QDemonRenderBackendGLBase::UpdateTextureObject(QDemonRenderBackendTextureObject to,
                                                    QDemonRenderTextureTargetType::Enum target,
                                                    qint32 baseLevel, qint32 maxLevel)
{
    NVRENDER_BACKEND_UNUSED(to);
    NVRENDER_BACKEND_UNUSED(target);
    NVRENDER_BACKEND_UNUSED(baseLevel);
    NVRENDER_BACKEND_UNUSED(maxLevel);
}

void QDemonRenderBackendGLBase::UpdateTextureSwizzle(QDemonRenderBackendTextureObject to,
                                                     QDemonRenderTextureTargetType::Enum target,
                                                     QDemonRenderTextureSwizzleMode::Enum swizzleMode)
{
    NVRENDER_BACKEND_UNUSED(to);
    NVRENDER_BACKEND_UNUSED(target);

    // Nothing to do here still might be called
    Q_ASSERT(swizzleMode == QDemonRenderTextureSwizzleMode::NoSwizzle);

    NVRENDER_BACKEND_UNUSED(swizzleMode);
}

void QDemonRenderBackendGLBase::ReleaseSampler(QDemonRenderBackendSamplerObject so)
{
    GLuint samplerID = HandleToID_cast(GLuint, size_t, so);
    if (!samplerID)
        return;
    // otherwise nothing to do
}

QDemonRenderBackend::QDemonRenderBackendAttribLayoutObject
QDemonRenderBackendGLBase::CreateAttribLayout(QDemonConstDataRef<QDemonRenderVertexBufferEntry> attribs)
{
    quint32 attribLayoutSize = sizeof(QDemonRenderBackendAttributeLayoutGL);
    quint32 entrySize = sizeof(QDemonRenderBackendLayoutEntryGL) * attribs.size();
    quint8 *newMem = static_cast<quint8 *>(::malloc(attribLayoutSize + entrySize));
    QDemonDataRef<QDemonRenderBackendLayoutEntryGL> entryRef = PtrAtOffset<QDemonRenderBackendLayoutEntryGL>(newMem, attribLayoutSize, entrySize);
    quint32 maxInputSlot = 0;

    // copy data
    QDEMON_FOREACH(idx, attribs.size())
    {
        new (&entryRef[idx]) QDemonRenderBackendLayoutEntryGL();
        entryRef[idx].m_AttribName = QString::fromLocal8Bit(attribs.mData[idx].m_Name);
        entryRef[idx].m_Normalize = 0;
        entryRef[idx].m_AttribIndex = 0; // will be set later
        entryRef[idx].m_Type = m_Conversion.fromComponentTypeAndNumCompsToAttribGL(
                    attribs.mData[idx].m_ComponentType, attribs.mData[idx].m_NumComponents);
        entryRef[idx].m_NumComponents = attribs.mData[idx].m_NumComponents;
        entryRef[idx].m_InputSlot = attribs.mData[idx].m_InputSlot;
        entryRef[idx].m_Offset = attribs.mData[idx].m_FirstItemOffset;

        if (maxInputSlot < entryRef[idx].m_InputSlot)
            maxInputSlot = entryRef[idx].m_InputSlot;
    }

    QDemonRenderBackendAttributeLayoutGL *retval =
            new (newMem) QDemonRenderBackendAttributeLayoutGL(entryRef, maxInputSlot);

    return (QDemonRenderBackend::QDemonRenderBackendAttribLayoutObject)retval;
}

void QDemonRenderBackendGLBase::ReleaseAttribLayout(QDemonRenderBackendAttribLayoutObject ao)
{
    QDemonRenderBackendAttributeLayoutGL *attribLayout = (QDemonRenderBackendAttributeLayoutGL *)ao;

    delete attribLayout;
};

QDemonRenderBackend::QDemonRenderBackendInputAssemblerObject
QDemonRenderBackendGLBase::CreateInputAssembler(QDemonRenderBackendAttribLayoutObject attribLayout,
                                                QDemonConstDataRef<QDemonRenderBackendBufferObject> buffers,
                                                const QDemonRenderBackendBufferObject indexBuffer,
                                                QDemonConstDataRef<quint32> strides,
                                                QDemonConstDataRef<quint32> offsets,
                                                quint32 patchVertexCount)
{
    QDemonRenderBackendAttributeLayoutGL *attribLayoutGL =
            (QDemonRenderBackendAttributeLayoutGL *)attribLayout;

    QDemonRenderBackendInputAssemblerGL *retval = new QDemonRenderBackendInputAssemblerGL(attribLayoutGL, buffers, indexBuffer, strides, offsets, patchVertexCount);

    return (QDemonRenderBackend::QDemonRenderBackendInputAssemblerObject)retval;
}

void QDemonRenderBackendGLBase::ReleaseInputAssembler(QDemonRenderBackendInputAssemblerObject iao)
{
    QDemonRenderBackendInputAssemblerGL *inputAssembler = (QDemonRenderBackendInputAssemblerGL *)iao;
    delete inputAssembler;
}

bool QDemonRenderBackendGLBase::compileSource(GLuint shaderID, QDemonConstDataRef<qint8> source,
                                              QByteArray &errorMessage, bool binary)
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
        GL_CALL_FUNCTION(glShaderBinary(1, &shaderID, GL_NVIDIA_PLATFORM_BINARY_NV, shaderSourceData,
                                        shaderSourceSize));
        GLenum binaryError = m_glFunctions->glGetError();
        if (binaryError != GL_NO_ERROR) {
            shaderStatus = GL_FALSE;
            qCCritical(GL_ERROR, GLConversion::processGLError(binaryError));
        }
    }

    return (shaderStatus == GL_TRUE);
}

QDemonRenderBackend::QDemonRenderBackendVertexShaderObject
QDemonRenderBackendGLBase::CreateVertexShader(QDemonConstDataRef<qint8> source,
                                              QByteArray &errorMessage, bool binary)
{
    GLuint shaderID = GL_CALL_FUNCTION(glCreateShader(GL_VERTEX_SHADER));

    if (shaderID && !compileSource(shaderID, source, errorMessage, binary)) {
        GL_CALL_FUNCTION(glDeleteShader(shaderID));
        shaderID = 0;
    }

    return (QDemonRenderBackend::QDemonRenderBackendVertexShaderObject)shaderID;
}

QDemonRenderBackend::QDemonRenderBackendFragmentShaderObject
QDemonRenderBackendGLBase::CreateFragmentShader(QDemonConstDataRef<qint8> source,
                                                QByteArray &errorMessage, bool binary)
{
    GLuint shaderID = GL_CALL_FUNCTION(glCreateShader(GL_FRAGMENT_SHADER));

    if (shaderID && !compileSource(shaderID, source, errorMessage, binary)) {
        GL_CALL_FUNCTION(glDeleteShader(shaderID));
        shaderID = 0;
    }

    return (QDemonRenderBackend::QDemonRenderBackendFragmentShaderObject)shaderID;
}

QDemonRenderBackend::QDemonRenderBackendTessControlShaderObject
QDemonRenderBackendGLBase::CreateTessControlShader(QDemonConstDataRef<qint8> source,
                                                   QByteArray &errorMessage, bool binary)
{
    // needs GL 4 or GLES EXT_tessellation_shader support
    NVRENDER_BACKEND_UNUSED(source);
    NVRENDER_BACKEND_UNUSED(errorMessage);
    NVRENDER_BACKEND_UNUSED(binary);

    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return (QDemonRenderBackend::QDemonRenderBackendTessControlShaderObject)0;
}

QDemonRenderBackend::QDemonRenderBackendTessEvaluationShaderObject
QDemonRenderBackendGLBase::CreateTessEvaluationShader(QDemonConstDataRef<qint8> source,
                                                      QByteArray &errorMessage, bool binary)
{
    // needs GL 4 or GLES EXT_tessellation_shader support
    NVRENDER_BACKEND_UNUSED(source);
    NVRENDER_BACKEND_UNUSED(errorMessage);
    NVRENDER_BACKEND_UNUSED(binary);

    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return (QDemonRenderBackend::QDemonRenderBackendTessEvaluationShaderObject)0;
}

QDemonRenderBackend::QDemonRenderBackendGeometryShaderObject
QDemonRenderBackendGLBase::CreateGeometryShader(QDemonConstDataRef<qint8> source,
                                                QByteArray &errorMessage, bool binary)
{
    // needs GL 4 or GLES EXT_geometry_shader support
    NVRENDER_BACKEND_UNUSED(source);
    NVRENDER_BACKEND_UNUSED(errorMessage);
    NVRENDER_BACKEND_UNUSED(binary);

    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return (QDemonRenderBackend::QDemonRenderBackendGeometryShaderObject)0;
}

QDemonRenderBackend::QDemonRenderBackendComputeShaderObject
QDemonRenderBackendGLBase::CreateComputeShader(QDemonConstDataRef<qint8> source,
                                               QByteArray &errorMessage, bool binary)
{
    // needs GL 4.3 or GLES3.1 support
    NVRENDER_BACKEND_UNUSED(source);
    NVRENDER_BACKEND_UNUSED(errorMessage);
    NVRENDER_BACKEND_UNUSED(binary);

    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return (QDemonRenderBackend::QDemonRenderBackendComputeShaderObject)0;
}

void QDemonRenderBackendGLBase::ReleaseVertexShader(QDemonRenderBackendVertexShaderObject vso)
{
    GLuint shaderID = HandleToID_cast(GLuint, size_t, vso);

    GL_CALL_FUNCTION(glDeleteShader(shaderID));
}

void QDemonRenderBackendGLBase::ReleaseFragmentShader(QDemonRenderBackendFragmentShaderObject fso)
{
    GLuint shaderID = HandleToID_cast(GLuint, size_t, fso);

    GL_CALL_FUNCTION(glDeleteShader(shaderID));
}

void
QDemonRenderBackendGLBase::ReleaseTessControlShader(QDemonRenderBackendTessControlShaderObject tcso)
{
    GLuint shaderID = HandleToID_cast(GLuint, size_t, tcso);

    GL_CALL_FUNCTION(glDeleteShader(shaderID));
}

void QDemonRenderBackendGLBase::ReleaseTessEvaluationShader(
        QDemonRenderBackendTessEvaluationShaderObject teso)
{
    GLuint shaderID = HandleToID_cast(GLuint, size_t, teso);

    GL_CALL_FUNCTION(glDeleteShader(shaderID));
}

void QDemonRenderBackendGLBase::ReleaseGeometryShader(QDemonRenderBackendGeometryShaderObject gso)
{
    GLuint shaderID = HandleToID_cast(GLuint, size_t, gso);

    GL_CALL_FUNCTION(glDeleteShader(shaderID));
}

void QDemonRenderBackendGLBase::ReleaseComputeShader(QDemonRenderBackendComputeShaderObject cso)
{
    GLuint shaderID = HandleToID_cast(GLuint, size_t, cso);

    GL_CALL_FUNCTION(glDeleteShader(shaderID));
}

void QDemonRenderBackendGLBase::AttachShader(QDemonRenderBackendShaderProgramObject po,
                                             QDemonRenderBackendVertexShaderObject vso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, vso);

    GL_CALL_FUNCTION(glAttachShader(static_cast<GLuint>(pProgram->m_ProgramID), shaderID));
}

void QDemonRenderBackendGLBase::AttachShader(QDemonRenderBackendShaderProgramObject po,
                                             QDemonRenderBackendFragmentShaderObject fso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, fso);

    GL_CALL_FUNCTION(glAttachShader(static_cast<GLuint>(pProgram->m_ProgramID), shaderID));
}

void QDemonRenderBackendGLBase::AttachShader(QDemonRenderBackendShaderProgramObject po,
                                             QDemonRenderBackendTessControlShaderObject tcso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, tcso);

    GL_CALL_FUNCTION(glAttachShader(static_cast<GLuint>(pProgram->m_ProgramID), shaderID));
}

void QDemonRenderBackendGLBase::AttachShader(QDemonRenderBackendShaderProgramObject po,
                                             QDemonRenderBackendTessEvaluationShaderObject teso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, teso);

    GL_CALL_FUNCTION(glAttachShader(static_cast<GLuint>(pProgram->m_ProgramID), shaderID));
}

void QDemonRenderBackendGLBase::AttachShader(QDemonRenderBackendShaderProgramObject po,
                                             QDemonRenderBackendGeometryShaderObject gso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, gso);

    GL_CALL_FUNCTION(glAttachShader(static_cast<GLuint>(pProgram->m_ProgramID), shaderID));
}

void QDemonRenderBackendGLBase::AttachShader(QDemonRenderBackendShaderProgramObject po,
                                             QDemonRenderBackendComputeShaderObject cso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, cso);

    GL_CALL_FUNCTION(glAttachShader(static_cast<GLuint>(pProgram->m_ProgramID), shaderID));
}

void QDemonRenderBackendGLBase::DetachShader(QDemonRenderBackendShaderProgramObject po,
                                             QDemonRenderBackendVertexShaderObject vso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, vso);

    GL_CALL_FUNCTION(glDetachShader(static_cast<GLuint>(pProgram->m_ProgramID), shaderID));
}

void QDemonRenderBackendGLBase::DetachShader(QDemonRenderBackendShaderProgramObject po,
                                             QDemonRenderBackendFragmentShaderObject fso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, fso);

    GL_CALL_FUNCTION(glDetachShader(static_cast<GLuint>(pProgram->m_ProgramID), shaderID));
}

void QDemonRenderBackendGLBase::DetachShader(QDemonRenderBackendShaderProgramObject po,
                                             QDemonRenderBackendTessControlShaderObject tcso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, tcso);

    GL_CALL_FUNCTION(glDetachShader(static_cast<GLuint>(pProgram->m_ProgramID), shaderID));
}

void QDemonRenderBackendGLBase::DetachShader(QDemonRenderBackendShaderProgramObject po,
                                             QDemonRenderBackendTessEvaluationShaderObject teso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, teso);

    GL_CALL_FUNCTION(glDetachShader(static_cast<GLuint>(pProgram->m_ProgramID), shaderID));
}

void QDemonRenderBackendGLBase::DetachShader(QDemonRenderBackendShaderProgramObject po,
                                             QDemonRenderBackendGeometryShaderObject gso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, gso);

    GL_CALL_FUNCTION(glDetachShader(static_cast<GLuint>(pProgram->m_ProgramID), shaderID));
}

void QDemonRenderBackendGLBase::DetachShader(QDemonRenderBackendShaderProgramObject po,
                                             QDemonRenderBackendComputeShaderObject cso)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint shaderID = HandleToID_cast(GLuint, size_t, cso);

    GL_CALL_FUNCTION(glDetachShader(static_cast<GLuint>(pProgram->m_ProgramID), shaderID));
}

QDemonRenderBackend::QDemonRenderBackendShaderProgramObject
QDemonRenderBackendGLBase::CreateShaderProgram(bool isSeparable)
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

void QDemonRenderBackendGLBase::ReleaseShaderProgram(QDemonRenderBackendShaderProgramObject po)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_ProgramID);

    GL_CALL_FUNCTION(glDeleteProgram(programID));

    if (pProgram->m_shaderInput) {
        delete pProgram->m_shaderInput;
        pProgram->m_shaderInput = nullptr;
    }

    delete pProgram;
}

bool QDemonRenderBackendGLBase::LinkProgram(QDemonRenderBackendShaderProgramObject po,
                                            QByteArray &errorMessage)
{
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_ProgramID);

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
            QDemonRenderBackendShaderInputEntryGL *tempShaderInputEntry = static_cast<QDemonRenderBackendShaderInputEntryGL *>(::malloc(sizeof(QDemonRenderBackendShaderInputEntryGL) * m_MaxAttribCount));

            GLint maxLength;
            GL_CALL_FUNCTION(glGetProgramiv(programID, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength));
            qint8 *nameBuf = static_cast<qint8 *>(::malloc(size_t(maxLength)));

            // fill in data
            quint32 count = 0;
            QDEMON_FOREACH(idx, numAttribs)
            {
                GLint size = 0;
                GLenum glType;
                QDemonRenderComponentTypes::Enum compType = QDemonRenderComponentTypes::Unknown;
                quint32 numComps = 0;

                GL_CALL_FUNCTION(glGetActiveAttrib(programID, idx, maxLength, nullptr, &size, &glType,
                                                   (char *)nameBuf));
                // Skip anything named with gl_
                if (memcmp(nameBuf, "gl_", 3) == 0)
                    continue;

                m_Conversion.fromAttribGLToComponentTypeAndNumComps(glType, compType, numComps);

                new (&tempShaderInputEntry[count]) QDemonRenderBackendShaderInputEntryGL();
                tempShaderInputEntry[count].m_AttribName = QString::fromLocal8Bit(reinterpret_cast<char *>(nameBuf));
                tempShaderInputEntry[count].m_AttribLocation =
                        GL_CALL_FUNCTION(glGetAttribLocation(programID, (char *)nameBuf));
                tempShaderInputEntry[count].m_Type = glType;
                tempShaderInputEntry[count].m_NumComponents = numComps;

                ++count;
            }

            // Now allocate space for the actuall entries
            quint32 shaderInputSize = sizeof(QDemonRenderBackendShaderInputGL);
            quint32 entrySize = sizeof(QDemonRenderBackendShaderInputEntryGL) * count;
            quint8 *newMem = static_cast<quint8 *>(::malloc(shaderInputSize + entrySize));
            QDemonDataRef<QDemonRenderBackendShaderInputEntryGL> entryRef =
                    PtrAtOffset<QDemonRenderBackendShaderInputEntryGL>(newMem, shaderInputSize,
                                                                       entrySize);
            // fill data
            QDEMON_FOREACH(idx, count)
            {
                new (&entryRef[idx]) QDemonRenderBackendShaderInputEntryGL();
                entryRef[idx].m_AttribName = tempShaderInputEntry[idx].m_AttribName;
                entryRef[idx].m_AttribLocation = tempShaderInputEntry[idx].m_AttribLocation;
                entryRef[idx].m_Type = tempShaderInputEntry[idx].m_Type;
                entryRef[idx].m_NumComponents = tempShaderInputEntry[idx].m_NumComponents;
            }

            // placement new
            QDemonRenderBackendShaderInputGL *shaderInput =
                    new (newMem) QDemonRenderBackendShaderInputGL(entryRef);
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

void QDemonRenderBackendGLBase::SetActiveProgram(QDemonRenderBackendShaderProgramObject po)
{
    GLuint programID = 0;

    if (po) {
        QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
        programID = static_cast<GLuint>(pProgram->m_ProgramID);
    }

    GL_CALL_FUNCTION(glUseProgram(programID));
}

QDemonRenderBackend::QDemonRenderBackendProgramPipeline QDemonRenderBackendGLBase::CreateProgramPipeline()
{
    // needs GL 4 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
    return QDemonRenderBackend::QDemonRenderBackendProgramPipeline(0);
}

void QDemonRenderBackendGLBase::ReleaseProgramPipeline(QDemonRenderBackendProgramPipeline)
{
    // needs GL 4 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::SetActiveProgramPipeline(QDemonRenderBackendProgramPipeline)
{
    // needs GL 4 context
    //TODO: should be fixed?
    //        Q_ASSERT(false);
}

void QDemonRenderBackendGLBase::SetProgramStages(QDemonRenderBackendProgramPipeline,
                                                 QDemonRenderShaderTypeFlags,
                                                 QDemonRenderBackendShaderProgramObject)
{
    // needs GL 4 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::DispatchCompute(QDemonRenderBackendShaderProgramObject, quint32, quint32,
                                                quint32)
{
    // needs GL 4 context
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

qint32 QDemonRenderBackendGLBase::GetConstantCount(QDemonRenderBackendShaderProgramObject po)
{
    Q_ASSERT(po);
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_ProgramID);

    GLint numUniforms;
    GL_CALL_FUNCTION(glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &numUniforms));

    return numUniforms;
}

qint32 QDemonRenderBackendGLBase::GetConstantBufferCount(QDemonRenderBackendShaderProgramObject po)
{
    // needs GL3 and above
    NVRENDER_BACKEND_UNUSED(po);

    return 0;
}

qint32
QDemonRenderBackendGLBase::GetConstantInfoByID(QDemonRenderBackendShaderProgramObject po, quint32 id,
                                               quint32 bufSize, qint32 *numElem,
                                               QDemonRenderShaderDataTypes::Enum *type, qint32 *binding,
                                               char *nameBuf)
{
    Q_ASSERT(po);
    QDemonRenderBackendShaderProgramGL *pProgram = (QDemonRenderBackendShaderProgramGL *)po;
    GLuint programID = static_cast<GLuint>(pProgram->m_ProgramID);

    GLenum glType;
    GL_CALL_FUNCTION(glGetActiveUniform(programID, id, bufSize, nullptr, numElem, &glType, nameBuf));
    *type = m_Conversion.fromShaderGLToPropertyDataTypes(glType);

    qint32 uniformLoc = GL_CALL_FUNCTION(glGetUniformLocation(programID, nameBuf));

    // get unit binding point
    *binding = -1;
    if (uniformLoc != -1 && (glType == GL_IMAGE_2D || glType == GL_UNSIGNED_INT_IMAGE_2D
                             || glType == GL_UNSIGNED_INT_ATOMIC_COUNTER)) {
        GL_CALL_FUNCTION(glGetUniformiv(programID, uniformLoc, binding));
    }

    return uniformLoc;
}

qint32
QDemonRenderBackendGLBase::GetConstantBufferInfoByID(QDemonRenderBackendShaderProgramObject po,
                                                     quint32 id, quint32 nameBufSize, qint32 *paramCount,
                                                     qint32 *bufferSize, qint32 *length,
                                                     char *nameBuf)
{
    // needs GL3 and above
    NVRENDER_BACKEND_UNUSED(po);
    NVRENDER_BACKEND_UNUSED(id);
    NVRENDER_BACKEND_UNUSED(nameBufSize);
    NVRENDER_BACKEND_UNUSED(paramCount);
    NVRENDER_BACKEND_UNUSED(bufferSize);
    NVRENDER_BACKEND_UNUSED(length);
    NVRENDER_BACKEND_UNUSED(nameBuf);

    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return -1;
}

void QDemonRenderBackendGLBase::GetConstantBufferParamIndices(QDemonRenderBackendShaderProgramObject po,
                                                              quint32 id, qint32 *indices)
{
    // needs GL3 and above
    NVRENDER_BACKEND_UNUSED(po);
    NVRENDER_BACKEND_UNUSED(id);
    NVRENDER_BACKEND_UNUSED(indices);

    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::GetConstantBufferParamInfoByIndices(
        QDemonRenderBackendShaderProgramObject po, quint32 count, quint32 *indices, qint32 *type,
        qint32 *size, qint32 *offset)
{
    // needs GL3 and above
    NVRENDER_BACKEND_UNUSED(po);
    NVRENDER_BACKEND_UNUSED(count);
    NVRENDER_BACKEND_UNUSED(indices);
    NVRENDER_BACKEND_UNUSED(type);
    NVRENDER_BACKEND_UNUSED(size);
    NVRENDER_BACKEND_UNUSED(offset);

    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::ProgramSetConstantBlock(QDemonRenderBackendShaderProgramObject po,
                                                        quint32 blockIndex, quint32 binding)
{
    // needs GL3 and above
    NVRENDER_BACKEND_UNUSED(po);
    NVRENDER_BACKEND_UNUSED(blockIndex);
    NVRENDER_BACKEND_UNUSED(binding);

    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::ProgramSetConstantBuffer(quint32 index,
                                                         QDemonRenderBackendBufferObject bo)
{
    // needs GL3 and above
    NVRENDER_BACKEND_UNUSED(index);
    NVRENDER_BACKEND_UNUSED(bo);

    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

qint32 QDemonRenderBackendGLBase::GetStorageBufferCount(QDemonRenderBackendShaderProgramObject po)
{
    // needs GL4 and above
    NVRENDER_BACKEND_UNUSED(po);

    return 0;
}

qint32
QDemonRenderBackendGLBase::GetStorageBufferInfoByID(QDemonRenderBackendShaderProgramObject po, quint32 id,
                                                    quint32 nameBufSize, qint32 *paramCount,
                                                    qint32 *bufferSize, qint32 *length, char *nameBuf)
{
    // needs GL4 and above
    NVRENDER_BACKEND_UNUSED(po);
    NVRENDER_BACKEND_UNUSED(id);
    NVRENDER_BACKEND_UNUSED(nameBufSize);
    NVRENDER_BACKEND_UNUSED(paramCount);
    NVRENDER_BACKEND_UNUSED(bufferSize);
    NVRENDER_BACKEND_UNUSED(length);
    NVRENDER_BACKEND_UNUSED(nameBuf);

    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return -1;
}

void QDemonRenderBackendGLBase::ProgramSetStorageBuffer(quint32 index, QDemonRenderBackendBufferObject bo)
{
    // needs GL4 and above
    NVRENDER_BACKEND_UNUSED(index);
    NVRENDER_BACKEND_UNUSED(bo);
}

qint32 QDemonRenderBackendGLBase::GetAtomicCounterBufferCount(QDemonRenderBackendShaderProgramObject po)
{
    // needs GL4 and above
    NVRENDER_BACKEND_UNUSED(po);

    return 0;
}

qint32
QDemonRenderBackendGLBase::GetAtomicCounterBufferInfoByID(QDemonRenderBackendShaderProgramObject po,
                                                          quint32 id, quint32 nameBufSize,
                                                          qint32 *paramCount, qint32 *bufferSize,
                                                          qint32 *length, char *nameBuf)
{
    // needs GL4 and above
    NVRENDER_BACKEND_UNUSED(po);
    NVRENDER_BACKEND_UNUSED(id);
    NVRENDER_BACKEND_UNUSED(nameBufSize);
    NVRENDER_BACKEND_UNUSED(paramCount);
    NVRENDER_BACKEND_UNUSED(bufferSize);
    NVRENDER_BACKEND_UNUSED(length);
    NVRENDER_BACKEND_UNUSED(nameBuf);

    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return -1;
}

void QDemonRenderBackendGLBase::ProgramSetAtomicCounterBuffer(quint32 index,
                                                              QDemonRenderBackendBufferObject bo)
{
    // needs GL4 and above
    NVRENDER_BACKEND_UNUSED(index);
    NVRENDER_BACKEND_UNUSED(bo);
}

void QDemonRenderBackendGLBase::SetConstantValue(QDemonRenderBackendShaderProgramObject, quint32 id,
                                                 QDemonRenderShaderDataTypes::Enum type, qint32 count,
                                                 const void *value, bool transpose)
{
    GLenum glType = m_Conversion.fromPropertyDataTypesToShaderGL(type);

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
    case GL_BOOL:
    {
        // Cast int value to be 0 or 1, matching to bool
        GLint *boolValue = (GLint *)value;
        *boolValue = *(GLboolean *)value;
        GL_CALL_FUNCTION(glUniform1iv(id, count, boolValue));
    }
        break;
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

void QDemonRenderBackendGLBase::Draw(QDemonRenderDrawMode::Enum drawMode, quint32 start, quint32 count)
{
    GL_CALL_FUNCTION(glDrawArrays(m_Conversion.fromDrawModeToGL(
                                      drawMode, m_backendSupport.caps.bits.bTessellationSupported),
                                  start, count));
}

void QDemonRenderBackendGLBase::DrawIndirect(QDemonRenderDrawMode::Enum drawMode, const void *indirect)
{
    // needs GL4 and above
    NVRENDER_BACKEND_UNUSED(drawMode);
    NVRENDER_BACKEND_UNUSED(indirect);
}

void QDemonRenderBackendGLBase::DrawIndexed(QDemonRenderDrawMode::Enum drawMode, quint32 count,
                                            QDemonRenderComponentTypes::Enum type, const void *indices)
{
    GL_CALL_FUNCTION(glDrawElements(m_Conversion.fromDrawModeToGL(
                                        drawMode, m_backendSupport.caps.bits.bTessellationSupported),
                                    count, m_Conversion.fromIndexBufferComponentsTypesToGL(type),
                                    indices));
}

void QDemonRenderBackendGLBase::DrawIndexedIndirect(QDemonRenderDrawMode::Enum drawMode,
                                                    QDemonRenderComponentTypes::Enum type,
                                                    const void *indirect)
{
    // needs GL4 and above
    NVRENDER_BACKEND_UNUSED(drawMode);
    NVRENDER_BACKEND_UNUSED(type);
    NVRENDER_BACKEND_UNUSED(indirect);
}

void QDemonRenderBackendGLBase::ReadPixel(QDemonRenderBackendRenderTargetObject /* rto */, qint32 x,
                                          qint32 y, qint32 width, qint32 height,
                                          QDemonRenderReadPixelFormats::Enum inFormat, void *pixels)
{
    GLuint glFormat;
    GLuint glType;
    if (m_Conversion.fromReadPixelsToGlFormatAndType(inFormat, &glFormat, &glType)) {
        GL_CALL_FUNCTION(glReadPixels(x, y, width, height, glFormat, glType, pixels));
    }
}

QDemonRenderBackend::QDemonRenderBackendPathObject QDemonRenderBackendGLBase::CreatePathNVObject(size_t)
{
    // Needs GL 4 backend
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return QDemonRenderBackend::QDemonRenderBackendPathObject(0);
}

void QDemonRenderBackendGLBase::ReleasePathNVObject(QDemonRenderBackendPathObject, size_t)
{
    // Needs GL 4 backend
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::LoadPathGlyphs(QDemonRenderBackendPathObject,
                                               QDemonRenderPathFontTarget::Enum, const void *,
                                               QDemonRenderPathFontStyleFlags, size_t,
                                               QDemonRenderPathFormatType::Enum, const void *,
                                               QDemonRenderPathMissingGlyphs::Enum,
                                               QDemonRenderBackendPathObject, float)
{
    // Needs GL 4 backend
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::LoadPathGlyphRange(QDemonRenderBackendPathObject,
                                                   QDemonRenderPathFontTarget::Enum, const void *,
                                                   QDemonRenderPathFontStyleFlags, quint32, size_t,
                                                   QDemonRenderPathMissingGlyphs::Enum,
                                                   QDemonRenderBackendPathObject, float)
{
    // Needs GL 4 backend
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

QDemonRenderPathReturnValues::Enum QDemonRenderBackendGLBase::LoadPathGlyphsIndexed(
        QDemonRenderBackendPathObject, QDemonRenderPathFontTarget::Enum, const void *,
        QDemonRenderPathFontStyleFlags, quint32, size_t, QDemonRenderBackendPathObject, float)
{
    // Needs GL 4 backend
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;

    return QDemonRenderPathReturnValues::FontUnavailable;
}

QDemonRenderBackend::QDemonRenderBackendPathObject QDemonRenderBackendGLBase::LoadPathGlyphsIndexedRange(
        QDemonRenderPathFontTarget::Enum, const void *, QDemonRenderPathFontStyleFlags,
        QDemonRenderBackend::QDemonRenderBackendPathObject, float, quint32 *)
{
    return QDemonRenderBackendPathObject(0);
}

void QDemonRenderBackendGLBase::GetPathMetrics(QDemonRenderBackendPathObject, size_t,
                                               QDemonRenderPathGlyphFontMetricFlags,
                                               QDemonRenderPathFormatType::Enum, const void *, size_t,
                                               float *)
{
    // Needs GL 4 backend
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::GetPathMetricsRange(QDemonRenderBackendPathObject, size_t,
                                                    QDemonRenderPathGlyphFontMetricFlags, size_t,
                                                    float *)
{
    // Needs GL 4 backend
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::GetPathSpacing(QDemonRenderBackendPathObject, size_t,
                                               QDemonRenderPathListMode::Enum,
                                               QDemonRenderPathFormatType::Enum, const void *, float,
                                               float, QDemonRenderPathTransformType::Enum, float *)
{
    // Needs GL 4 backend
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::StencilFillPathInstanced(QDemonRenderBackendPathObject, size_t,
                                                         QDemonRenderPathFormatType::Enum, const void *,
                                                         QDemonRenderPathFillMode::Enum, quint32,
                                                         QDemonRenderPathTransformType::Enum,
                                                         const float *)
{
    // Needs GL 4 backend
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::StencilStrokePathInstancedN(QDemonRenderBackendPathObject, size_t,
                                                            QDemonRenderPathFormatType::Enum,
                                                            const void *, qint32, quint32,
                                                            QDemonRenderPathTransformType::Enum,
                                                            const float *)
{
    // Needs GL 4 backend
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::CoverFillPathInstanced(QDemonRenderBackendPathObject, size_t,
                                                       QDemonRenderPathFormatType::Enum, const void *,
                                                       QDemonRenderPathCoverMode::Enum,
                                                       QDemonRenderPathTransformType::Enum,
                                                       const float *)
{
    // Needs GL 4 backend
    qCCritical(INVALID_OPERATION) << QObject::tr("Unsupported method: ") << __FUNCTION__;
}

void QDemonRenderBackendGLBase::CoverStrokePathInstanced(QDemonRenderBackendPathObject, size_t,
                                                         QDemonRenderPathFormatType::Enum, const void *,
                                                         QDemonRenderPathCoverMode::Enum,
                                                         QDemonRenderPathTransformType::Enum,
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
    QString apiVersion(getVersionString());
    qCInfo(TRACE_INFO, "GL version: %s", qPrintable(apiVersion));

    // we assume all GLES versions running on mobile with shared memory
    // this means framebuffer blits are slow and should be optimized or avoided
    if (!apiVersion.contains("OpenGL ES")) {
        // no ES device
        m_backendSupport.caps.bits.bFastBlitsSupported = true;
    }
}

#ifdef RENDER_BACKEND_LOG_GL_ERRORS
void QDemonRenderBackendGLBase::checkGLError(const char *function, const char *file,
                                             const unsigned int line) const
{
    GLenum error = m_glFunctions->glGetError();
    if (error != GL_NO_ERROR) {
        qCCritical(GL_ERROR) << GLConversion::processGLError(error) << " "
                             << function << " " << file << " " << line;
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
