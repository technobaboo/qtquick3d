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
#ifndef QDEMON_RENDER_EFFECT_SYSTEM_COMMANDS_H
#define QDEMON_RENDER_EFFECT_SYSTEM_COMMANDS_H

#include <QtDemonRender/qdemonrenderbasetypes.h>

QT_BEGIN_NAMESPACE
namespace dynamic {

enum class CommandType
{
    Unknown = 0,
    AllocateBuffer,
    BindTarget,
    BindBuffer,
    BindShader,
    ApplyInstanceValue,
    ApplyBufferValue,
    // Apply the depth buffer as an input texture.
    ApplyDepthValue,
    Render, // Render to current FBO
    ApplyBlending,
    ApplyRenderState, // apply a render state
    ApplyBlitFramebuffer,
    ApplyValue,
    DepthStencil,
    AllocateImage,
    ApplyImageValue,
    AllocateDataBuffer,
    ApplyDataBufferValue,
};

// All commands need at least two constructors.  One for when they are created that should
// setup all their member variables and one for when we are copying commands from an outside
// entity into the effect system.  We have to re-register strings in that case because we
// can't assume the outside entity was using the same string table we are...
struct QDemonCommand
{
    CommandType m_type;
    QDemonCommand(CommandType inType) : m_type(inType) {}
    QDemonCommand() : m_type(CommandType::Unknown) {}
    // Implemented in UICRenderEffectSystem.cpp
    static quint32 getSizeofCommand(const QDemonCommand &inCommand);
    static void copyConstructCommand(quint8 *inDataBuffer, const QDemonCommand &inCommand);
};

enum class AllocateBufferFlagValues
{
    None = 0,
    SceneLifetime = 1,
};

struct QDemonAllocateBufferFlags : public QFlags<AllocateBufferFlagValues>
{
    QDemonAllocateBufferFlags(quint32 inValues) : QFlags(inValues) {}
    QDemonAllocateBufferFlags() {}
    void setSceneLifetime(bool inValue) { setFlag(AllocateBufferFlagValues::SceneLifetime, inValue); }
    // If isSceneLifetime is unset the buffer is assumed to be frame lifetime and will be
    // released after this render operation.
    bool isSceneLifetime() const { return this->operator&(AllocateBufferFlagValues::SceneLifetime); }
};

struct QDemonAllocateBuffer : public QDemonCommand
{
    QByteArray m_name;
    QDemonRenderTextureFormat m_format = QDemonRenderTextureFormat::RGBA8;
    QDemonRenderTextureMagnifyingOp m_filterOp = QDemonRenderTextureMagnifyingOp::Linear;
    QDemonRenderTextureCoordOp m_texCoordOp = QDemonRenderTextureCoordOp::ClampToEdge;
    float m_sizeMultiplier = 1.0f;
    QDemonAllocateBufferFlags m_bufferFlags;
    QDemonAllocateBuffer() : QDemonCommand(CommandType::AllocateBuffer) {}
    QDemonAllocateBuffer(const QByteArray &inName,
                         QDemonRenderTextureFormat inFormat,
                         QDemonRenderTextureMagnifyingOp inFilterOp,
                         QDemonRenderTextureCoordOp inCoordOp,
                         float inMultiplier,
                         QDemonAllocateBufferFlags inFlags)
        : QDemonCommand(CommandType::AllocateBuffer)
        , m_name(inName)
        , m_format(inFormat)
        , m_filterOp(inFilterOp)
        , m_texCoordOp(inCoordOp)
        , m_sizeMultiplier(inMultiplier)
        , m_bufferFlags(inFlags)
    {
    }
    QDemonAllocateBuffer(const QDemonAllocateBuffer &inOther)
        : QDemonCommand(CommandType::AllocateBuffer)
        , m_name(inOther.m_name)
        , m_format(inOther.m_format)
        , m_filterOp(inOther.m_filterOp)
        , m_texCoordOp(inOther.m_texCoordOp)
        , m_sizeMultiplier(inOther.m_sizeMultiplier)
        , m_bufferFlags(inOther.m_bufferFlags)
    {
    }
};

struct QDemonAllocateImage : public QDemonAllocateBuffer
{
    QDemonRenderImageAccessType m_access = QDemonRenderImageAccessType::ReadWrite;

    QDemonAllocateImage() : QDemonAllocateBuffer() { m_type = CommandType::AllocateImage; }
    QDemonAllocateImage(QByteArray &inName,
                        QDemonRenderTextureFormat inFormat,
                        QDemonRenderTextureMagnifyingOp inFilterOp,
                        QDemonRenderTextureCoordOp inCoordOp,
                        float inMultiplier,
                        QDemonAllocateBufferFlags inFlags,
                        QDemonRenderImageAccessType inAccess)
        : QDemonAllocateBuffer(inName, inFormat, inFilterOp, inCoordOp, inMultiplier, inFlags), m_access(inAccess)
    {
        m_type = CommandType::AllocateImage;
    }

    QDemonAllocateImage(const QDemonAllocateImage &inOther)
        : QDemonAllocateBuffer(inOther.m_name, inOther.m_format, inOther.m_filterOp, inOther.m_texCoordOp, inOther.m_sizeMultiplier, inOther.m_bufferFlags)
        , m_access(inOther.m_access)
    {
        m_type = CommandType::AllocateImage;
    }
};

struct QDemonAllocateDataBuffer : public QDemonCommand
{
    QByteArray m_name;
    QDemonRenderBufferType m_dataBufferType;
    QByteArray m_wrapName;
    QDemonRenderBufferType m_dataBufferWrapType;
    float m_size;
    QDemonAllocateBufferFlags m_bufferFlags;

    QDemonAllocateDataBuffer() : QDemonCommand(CommandType::AllocateDataBuffer) {}

    QDemonAllocateDataBuffer(const QByteArray &inName,
                             QDemonRenderBufferType inBufferType,
                             const QByteArray &inWrapName,
                             QDemonRenderBufferType inBufferWrapType,
                             float inSize,
                             QDemonAllocateBufferFlags inFlags)
        : QDemonCommand(CommandType::AllocateDataBuffer)
        , m_name(inName)
        , m_dataBufferType(inBufferType)
        , m_wrapName(inWrapName)
        , m_dataBufferWrapType(inBufferWrapType)
        , m_size(inSize)
        , m_bufferFlags(inFlags)
    {
    }

    QDemonAllocateDataBuffer(const QDemonAllocateDataBuffer &inOther)
        : QDemonCommand(CommandType::AllocateDataBuffer)
        , m_name(inOther.m_name)
        , m_dataBufferType(inOther.m_dataBufferType)
        , m_wrapName(inOther.m_wrapName)
        , m_dataBufferWrapType(inOther.m_dataBufferWrapType)
        , m_size(inOther.m_size)
        , m_bufferFlags(inOther.m_bufferFlags)
    {
    }
};

struct QDemonBindTarget : public QDemonCommand
{
    QDemonRenderTextureFormat m_outputFormat;

    explicit QDemonBindTarget(QDemonRenderTextureFormat inFormat = QDemonRenderTextureFormat::RGBA8)
        : QDemonCommand(CommandType::BindTarget), m_outputFormat(inFormat)
    {
    }
    QDemonBindTarget(const QDemonBindTarget &inOther)
        : QDemonCommand(CommandType::BindTarget), m_outputFormat(inOther.m_outputFormat)
    {
    }
};

struct QDemonBindBuffer : public QDemonCommand
{
    QByteArray m_bufferName;
    bool m_needsClear;
    QDemonBindBuffer(const QByteArray &inBufName, bool inNeedsClear)
        : QDemonCommand(CommandType::BindBuffer), m_bufferName(inBufName), m_needsClear(inNeedsClear)
    {
    }
    QDemonBindBuffer(const QDemonBindBuffer &inOther)
        : QDemonCommand(CommandType::BindBuffer), m_bufferName(inOther.m_bufferName), m_needsClear(inOther.m_needsClear)
    {
    }
};

struct QDemonBindShader : public QDemonCommand
{
    QString m_shaderPath;
    // One GLSL file can hold multiple shaders in the case of multipass effects.
    // This makes it significantly easier for authors to reason about the shader
    // but it means we need to #define a preprocessor token to indicate which
    // effect we intend to compile at this point.
    QString m_shaderDefine;
    QDemonBindShader(QString inShaderPath, QString inShaderDefine = QString())
        : QDemonCommand(CommandType::BindShader), m_shaderPath(inShaderPath), m_shaderDefine(inShaderDefine)
    {
    }
    QDemonBindShader() : QDemonCommand(CommandType::BindShader) {}
    QDemonBindShader(const QDemonBindShader &inOther)
        : QDemonCommand(CommandType::BindShader), m_shaderPath(inOther.m_shaderPath), m_shaderDefine(inOther.m_shaderDefine)
    {
    }
};

// The value sits immediately after the 'this' object
// in memory.
// If propertyName is not valid then we attempt to apply all of the effect property values
// to the shader, ignoring ones that don't match up.
struct QDemonApplyInstanceValue : public QDemonCommand
{
    // Name of value to apply in shader
    QString m_propertyName;
    // type of value
    QDemonRenderShaderDataType m_valueType;
    // offset in the effect data section of value.
    quint32 m_valueOffset;
    QDemonApplyInstanceValue(QString inName, QDemonRenderShaderDataType inValueType, quint32 inValueOffset)
        : QDemonCommand(CommandType::ApplyInstanceValue), m_propertyName(inName), m_valueType(inValueType), m_valueOffset(inValueOffset)
    {
    }
    // Default will attempt to apply all effect values to the currently bound shader
    QDemonApplyInstanceValue()
        : QDemonCommand(CommandType::ApplyInstanceValue), m_valueType(QDemonRenderShaderDataType::Unknown), m_valueOffset(0)
    {
    }
    QDemonApplyInstanceValue(const QDemonApplyInstanceValue &inOther)
        : QDemonCommand(CommandType::ApplyInstanceValue)
        , m_propertyName(inOther.m_propertyName)
        , m_valueType(inOther.m_valueType)
        , m_valueOffset(inOther.m_valueOffset)
    {
    }
};

struct QDemonApplyValue : public QDemonCommand
{
    QString m_propertyName;
    QDemonRenderShaderDataType m_valueType;
    QDemonByteRef m_value;
    QDemonApplyValue(QString inName, QDemonRenderShaderDataType inValueType)
        : QDemonCommand(CommandType::ApplyValue), m_propertyName(inName), m_valueType(inValueType)
    {
    }
    // Default will attempt to apply all effect values to the currently bound shader
    QDemonApplyValue() : QDemonCommand(CommandType::ApplyValue), m_valueType(QDemonRenderShaderDataType::Unknown) {}

    QDemonApplyValue(const QDemonApplyValue &inOther)
        : QDemonCommand(CommandType::ApplyValue)
        , m_propertyName(inOther.m_propertyName)
        , m_valueType(inOther.m_valueType)
        , m_value(inOther.m_value)
    {
    }
};

// bind a buffer to a given shader parameter.
struct QDemonApplyBufferValue : public QDemonCommand
{
    // If no buffer name is given then the special buffer [source]
    // is assumed.
    QByteArray m_bufferName;
    // If no param name is given, the buffer is bound to the
    // input texture parameter (texture0).
    QByteArray m_paramName;

    QDemonApplyBufferValue(const QByteArray &bufferName, const QByteArray &shaderParam)
        : QDemonCommand(CommandType::ApplyBufferValue), m_bufferName(bufferName), m_paramName(shaderParam)
    {
    }
    QDemonApplyBufferValue(const QDemonApplyBufferValue &inOther)
        : QDemonCommand(CommandType::ApplyBufferValue), m_bufferName(inOther.m_bufferName), m_paramName(inOther.m_paramName)
    {
    }
};

// bind a buffer to a given shader parameter.
struct QDemonApplyImageValue : public QDemonCommand
{
    QByteArray m_imageName; ///< name which the image was allocated
    QByteArray m_paramName; ///< must match the name in the shader
    bool m_bindAsTexture; ///< bind image as texture
    bool m_needSync; ///< if true we add a memory barrier before usage

    QDemonApplyImageValue(const QByteArray &bufferName, const QByteArray &shaderParam, bool inBindAsTexture, bool inNeedSync)
        : QDemonCommand(CommandType::ApplyImageValue)
        , m_imageName(bufferName)
        , m_paramName(shaderParam)
        , m_bindAsTexture(inBindAsTexture)
        , m_needSync(inNeedSync)
    {
    }
    QDemonApplyImageValue(const QDemonApplyImageValue &inOther)
        : QDemonCommand(CommandType::ApplyImageValue)
        , m_imageName(inOther.m_imageName)
        , m_paramName(inOther.m_paramName)
        , m_bindAsTexture(inOther.m_bindAsTexture)
        , m_needSync(inOther.m_needSync)
    {
    }
};

// bind a buffer to a given shader parameter.
struct QDemonApplyDataBufferValue : public QDemonCommand
{
    QByteArray m_paramName; ///< must match the name in the shader
    QDemonRenderBufferType m_bindAs; ///< to which target we bind this buffer

    QDemonApplyDataBufferValue(const QByteArray &inShaderParam, QDemonRenderBufferType inBufferType)
        : QDemonCommand(CommandType::ApplyDataBufferValue), m_paramName(inShaderParam), m_bindAs(inBufferType)
    {
    }
    QDemonApplyDataBufferValue(const QDemonApplyDataBufferValue &inOther)
        : QDemonCommand(CommandType::ApplyDataBufferValue), m_paramName(inOther.m_paramName), m_bindAs(inOther.m_bindAs)
    {
    }
};

struct QDemonApplyDepthValue : public QDemonCommand
{
    // If no param name is given, the buffer is bound to the
    // input texture parameter (texture0).
    QByteArray m_paramName;
    QDemonApplyDepthValue(const QByteArray &param) : QDemonCommand(CommandType::ApplyDepthValue), m_paramName(param) {}
    QDemonApplyDepthValue(const QDemonApplyDepthValue &inOther)
        : QDemonCommand(CommandType::ApplyDepthValue), m_paramName(inOther.m_paramName)
    {
    }
};

struct QDemonRender : public QDemonCommand
{
    bool m_drawIndirect;
    explicit QDemonRender(bool inDrawIndirect) : QDemonCommand(CommandType::Render), m_drawIndirect(inDrawIndirect) {}

    QDemonRender(const QDemonRender &inOther)
        : QDemonCommand(CommandType::Render), m_drawIndirect(inOther.m_drawIndirect)
    {
    }
};

struct QDemonApplyBlending : public QDemonCommand
{
    QDemonRenderSrcBlendFunc m_srcBlendFunc;
    QDemonRenderDstBlendFunc m_dstBlendFunc;

    QDemonApplyBlending(QDemonRenderSrcBlendFunc inSrcBlendFunc, QDemonRenderDstBlendFunc inDstBlendFunc)
        : QDemonCommand(CommandType::ApplyBlending), m_srcBlendFunc(inSrcBlendFunc), m_dstBlendFunc(inDstBlendFunc)
    {
    }

    QDemonApplyBlending(const QDemonApplyBlending &inOther)
        : QDemonCommand(CommandType::ApplyBlending), m_srcBlendFunc(inOther.m_srcBlendFunc), m_dstBlendFunc(inOther.m_dstBlendFunc)
    {
    }
};

struct QDemonApplyRenderState : public QDemonCommand
{
    QDemonRenderState m_renderState;
    bool m_enabled;

    QDemonApplyRenderState(QDemonRenderState inRenderStateValue, bool inEnabled)
        : QDemonCommand(CommandType::ApplyRenderState), m_renderState(inRenderStateValue), m_enabled(inEnabled)
    {
    }

    QDemonApplyRenderState(const QDemonApplyRenderState &inOther)
        : QDemonCommand(CommandType::ApplyRenderState), m_renderState(inOther.m_renderState), m_enabled(inOther.m_enabled)
    {
    }
};

struct QDemonApplyBlitFramebuffer : public QDemonCommand
{
    // If no buffer name is given then the special buffer [source]
    // is assumed. Which is the default render target
    QByteArray m_sourceBufferName;
    // If no buffer name is given then the special buffer [dest]
    // is assumed. Which is the default render target
    QByteArray m_destBufferName;

    QDemonApplyBlitFramebuffer(const QByteArray &inSourceBufferName, const QByteArray &inDestBufferName)
        : QDemonCommand(CommandType::ApplyBlitFramebuffer), m_sourceBufferName(inSourceBufferName), m_destBufferName(inDestBufferName)
    {
    }

    QDemonApplyBlitFramebuffer(const QDemonApplyBlitFramebuffer &inOther)
        : QDemonCommand(CommandType::ApplyBlitFramebuffer)
        , m_sourceBufferName(inOther.m_sourceBufferName)
        , m_destBufferName(inOther.m_destBufferName)
    {
    }
};

enum class QDemonDepthStencilFlagValue
{
    NoFlagValue = 0,
    ClearStencil = 1 << 0,
    ClearDepth = 1 << 1,
};

struct QDemonDepthStencilFlags : public QFlags<QDemonDepthStencilFlagValue>
{
    bool hasClearStencil() const { return operator&(QDemonDepthStencilFlagValue::ClearStencil); }
    void setClearStencil(bool value) { setFlag(QDemonDepthStencilFlagValue::ClearStencil, value); }

    bool hasClearDepth() const { return operator&(QDemonDepthStencilFlagValue::ClearDepth); }
    void setClearDepth(bool value) { setFlag(QDemonDepthStencilFlagValue::ClearDepth, value); }
};

struct QDemonDepthStencil : public QDemonCommand
{
    QString m_bufferName;
    QDemonDepthStencilFlags m_glags;
    QDemonRenderStencilOp m_stencilFailOperation = QDemonRenderStencilOp::Keep;
    QDemonRenderStencilOp m_depthPassOperation = QDemonRenderStencilOp::Keep;
    QDemonRenderStencilOp m_depthFailOperation = QDemonRenderStencilOp::Keep;
    QDemonRenderBoolOp m_stencilFunction = QDemonRenderBoolOp::Equal;
    quint32 m_reference = 0;
    quint32 m_mask = std::numeric_limits<quint32>::max();

    QDemonDepthStencil() : QDemonCommand(CommandType::DepthStencil) {}

    QDemonDepthStencil(QString bufName,
                       QDemonDepthStencilFlags flags,
                       QDemonRenderStencilOp inStencilOp,
                       QDemonRenderStencilOp inDepthPassOp,
                       QDemonRenderStencilOp inDepthFailOp,
                       QDemonRenderBoolOp inStencilFunc,
                       quint32 value,
                       quint32 mask)
        : QDemonCommand(CommandType::DepthStencil)
        , m_bufferName(bufName)
        , m_glags(flags)
        , m_stencilFailOperation(inStencilOp)
        , m_depthPassOperation(inDepthPassOp)
        , m_depthFailOperation(inDepthFailOp)
        , m_stencilFunction(inStencilFunc)
        , m_reference(value)
        , m_mask(mask)
    {
    }

    QDemonDepthStencil(const QDemonDepthStencil &inOther)
        : QDemonCommand(CommandType::DepthStencil)
        , m_bufferName(inOther.m_bufferName)
        , m_glags(inOther.m_glags)
        , m_stencilFailOperation(inOther.m_stencilFailOperation)
        , m_depthPassOperation(inOther.m_depthPassOperation)
        , m_depthFailOperation(inOther.m_depthFailOperation)
        , m_stencilFunction(inOther.m_stencilFunction)
        , m_reference(inOther.m_reference)
        , m_mask(inOther.m_mask)
    {
    }
};
}
QT_END_NAMESPACE

#endif
