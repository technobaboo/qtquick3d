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
#include <QtDemon/qdemonflags.h>

QT_BEGIN_NAMESPACE
namespace dynamic {

struct CommandTypes
{
    enum Enum {
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
};

#define QDEMON_RENDER_EFFECTS_ITERATE_COMMAND_TYPES                                                                    \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(AllocateBuffer)                                                         \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(BindTarget)                                                             \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(BindBuffer)                                                             \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(BindShader)                                                             \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(ApplyInstanceValue)                                                     \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(ApplyBufferValue)                                                       \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(ApplyDepthValue)                                                        \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(Render)                                                                 \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(ApplyBlending)                                                          \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(ApplyRenderState)                                                       \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(ApplyBlitFramebuffer)                                                   \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(ApplyValue)                                                             \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(DepthStencil)                                                           \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(AllocateImage)                                                          \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(ApplyImageValue)                                                        \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(AllocateDataBuffer)                                                     \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(ApplyDataBufferValue)

// All commands need at least two constructors.  One for when they are created that should
// setup all their member variables and one for when we are copying commands from an outside
// entity into the effect system.  We have to re-register strings in that case because we
// can't assume the outside entity was using the same string table we are...
struct QDemonCommand
{
    CommandTypes::Enum m_type;
    QDemonCommand(CommandTypes::Enum inType) : m_type(inType) {}
    QDemonCommand() : m_type(CommandTypes::Unknown) {}
    // Implemented in UICRenderEffectSystem.cpp
    static quint32 getSizeofCommand(const QDemonCommand &inCommand);
    static void copyConstructCommand(quint8 *inDataBuffer, const QDemonCommand &inCommand);
};

struct AllocateBufferFlagValues
{
    enum Enum {
        SceneLifetime = 1,
    };
};

struct QDemonAllocateBufferFlags : public QDemonFlags<AllocateBufferFlagValues::Enum, quint32>
{
    QDemonAllocateBufferFlags(quint32 inValues) : QDemonFlags<AllocateBufferFlagValues::Enum, quint32>(inValues) {}
    QDemonAllocateBufferFlags() {}
    void setSceneLifetime(bool inValue) { clearOrSet(inValue, AllocateBufferFlagValues::SceneLifetime); }
    // If isSceneLifetime is unset the buffer is assumed to be frame lifetime and will be
    // released after this render operation.
    bool isSceneLifetime() const { return this->operator&(AllocateBufferFlagValues::SceneLifetime); }
};

struct QDemonAllocateBuffer : public QDemonCommand
{
    QString m_name;
    QDemonRenderTextureFormats::Enum m_format = QDemonRenderTextureFormats::RGBA8;
    QDemonRenderTextureMagnifyingOp::Enum m_filterOp = QDemonRenderTextureMagnifyingOp::Linear;
    QDemonRenderTextureCoordOp::Enum m_texCoordOp = QDemonRenderTextureCoordOp::ClampToEdge;
    float m_sizeMultiplier = 1.0f;
    QDemonAllocateBufferFlags m_bufferFlags;
    QDemonAllocateBuffer() : QDemonCommand(CommandTypes::AllocateBuffer) {}
    QDemonAllocateBuffer(QString inName,
                         QDemonRenderTextureFormats::Enum inFormat,
                         QDemonRenderTextureMagnifyingOp::Enum inFilterOp,
                         QDemonRenderTextureCoordOp::Enum inCoordOp,
                         float inMultiplier,
                         QDemonAllocateBufferFlags inFlags)
        : QDemonCommand(CommandTypes::AllocateBuffer)
        , m_name(inName)
        , m_format(inFormat)
        , m_filterOp(inFilterOp)
        , m_texCoordOp(inCoordOp)
        , m_sizeMultiplier(inMultiplier)
        , m_bufferFlags(inFlags)
    {
    }
    QDemonAllocateBuffer(const QDemonAllocateBuffer &inOther)
        : QDemonCommand(CommandTypes::AllocateBuffer)
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
    QDemonRenderImageAccessType::Enum m_access = QDemonRenderImageAccessType::ReadWrite;

    QDemonAllocateImage() : QDemonAllocateBuffer() { m_type = CommandTypes::AllocateImage; }
    QDemonAllocateImage(QString inName,
                        QDemonRenderTextureFormats::Enum inFormat,
                        QDemonRenderTextureMagnifyingOp::Enum inFilterOp,
                        QDemonRenderTextureCoordOp::Enum inCoordOp,
                        float inMultiplier,
                        QDemonAllocateBufferFlags inFlags,
                        QDemonRenderImageAccessType::Enum inAccess)
        : QDemonAllocateBuffer(inName, inFormat, inFilterOp, inCoordOp, inMultiplier, inFlags), m_access(inAccess)
    {
        m_type = CommandTypes::AllocateImage;
    }

    QDemonAllocateImage(const QDemonAllocateImage &inOther)
        : QDemonAllocateBuffer(inOther.m_name, inOther.m_format, inOther.m_filterOp, inOther.m_texCoordOp, inOther.m_sizeMultiplier, inOther.m_bufferFlags)
        , m_access(inOther.m_access)
    {
        m_type = CommandTypes::AllocateImage;
    }
};

struct QDemonAllocateDataBuffer : public QDemonCommand
{
    QString m_name;
    QDemonRenderBufferBindValues::Enum m_dataBufferType;
    QString m_wrapName;
    QDemonRenderBufferBindValues::Enum m_dataBufferWrapType;
    float m_size;
    QDemonAllocateBufferFlags m_bufferFlags;

    QDemonAllocateDataBuffer() : QDemonCommand(CommandTypes::AllocateDataBuffer) {}

    QDemonAllocateDataBuffer(QString inName,
                             QDemonRenderBufferBindValues::Enum inBufferType,
                             QString inWrapName,
                             QDemonRenderBufferBindValues::Enum inBufferWrapType,
                             float inSize,
                             QDemonAllocateBufferFlags inFlags)
        : QDemonCommand(CommandTypes::AllocateDataBuffer)
        , m_name(inName)
        , m_dataBufferType(inBufferType)
        , m_wrapName(inWrapName)
        , m_dataBufferWrapType(inBufferWrapType)
        , m_size(inSize)
        , m_bufferFlags(inFlags)
    {
    }

    QDemonAllocateDataBuffer(const QDemonAllocateDataBuffer &inOther)
        : QDemonCommand(CommandTypes::AllocateDataBuffer)
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
    QDemonRenderTextureFormats::Enum m_outputFormat;

    explicit QDemonBindTarget(QDemonRenderTextureFormats::Enum inFormat = QDemonRenderTextureFormats::RGBA8)
        : QDemonCommand(CommandTypes::BindTarget), m_outputFormat(inFormat)
    {
    }
    QDemonBindTarget(const QDemonBindTarget &inOther)
        : QDemonCommand(CommandTypes::BindTarget), m_outputFormat(inOther.m_outputFormat)
    {
    }
};

struct QDemonBindBuffer : public QDemonCommand
{
    QString m_bufferName;
    bool m_needsClear;
    QDemonBindBuffer(QString inBufName, bool inNeedsClear)
        : QDemonCommand(CommandTypes::BindBuffer), m_bufferName(inBufName), m_needsClear(inNeedsClear)
    {
    }
    QDemonBindBuffer(const QDemonBindBuffer &inOther)
        : QDemonCommand(CommandTypes::BindBuffer), m_bufferName(inOther.m_bufferName), m_needsClear(inOther.m_needsClear)
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
        : QDemonCommand(CommandTypes::BindShader), m_shaderPath(inShaderPath), m_shaderDefine(inShaderDefine)
    {
    }
    QDemonBindShader() : QDemonCommand(CommandTypes::BindShader) {}
    QDemonBindShader(const QDemonBindShader &inOther)
        : QDemonCommand(CommandTypes::BindShader), m_shaderPath(inOther.m_shaderPath), m_shaderDefine(inOther.m_shaderDefine)
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
    QDemonRenderShaderDataTypes::Enum m_valueType;
    // offset in the effect data section of value.
    quint32 m_valueOffset;
    QDemonApplyInstanceValue(QString inName, QDemonRenderShaderDataTypes::Enum inValueType, quint32 inValueOffset)
        : QDemonCommand(CommandTypes::ApplyInstanceValue), m_propertyName(inName), m_valueType(inValueType), m_valueOffset(inValueOffset)
    {
    }
    // Default will attempt to apply all effect values to the currently bound shader
    QDemonApplyInstanceValue()
        : QDemonCommand(CommandTypes::ApplyInstanceValue), m_valueType(QDemonRenderShaderDataTypes::Unknown), m_valueOffset(0)
    {
    }
    QDemonApplyInstanceValue(const QDemonApplyInstanceValue &inOther)
        : QDemonCommand(CommandTypes::ApplyInstanceValue)
        , m_propertyName(inOther.m_propertyName)
        , m_valueType(inOther.m_valueType)
        , m_valueOffset(inOther.m_valueOffset)
    {
    }
};

struct QDemonApplyValue : public QDemonCommand
{
    QString m_propertyName;
    QDemonRenderShaderDataTypes::Enum m_valueType;
    QDemonDataRef<quint8> m_value;
    QDemonApplyValue(QString inName, QDemonRenderShaderDataTypes::Enum inValueType)
        : QDemonCommand(CommandTypes::ApplyValue), m_propertyName(inName), m_valueType(inValueType)
    {
    }
    // Default will attempt to apply all effect values to the currently bound shader
    QDemonApplyValue() : QDemonCommand(CommandTypes::ApplyValue), m_valueType(QDemonRenderShaderDataTypes::Unknown) {}

    QDemonApplyValue(const QDemonApplyValue &inOther)
        : QDemonCommand(CommandTypes::ApplyValue)
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
    QString m_bufferName;
    // If no param name is given, the buffer is bound to the
    // input texture parameter (texture0).
    QString m_paramName;

    QDemonApplyBufferValue(QString bufferName, QString shaderParam)
        : QDemonCommand(CommandTypes::ApplyBufferValue), m_bufferName(bufferName), m_paramName(shaderParam)
    {
    }
    QDemonApplyBufferValue(const QDemonApplyBufferValue &inOther)
        : QDemonCommand(CommandTypes::ApplyBufferValue), m_bufferName(inOther.m_bufferName), m_paramName(inOther.m_paramName)
    {
    }
};

// bind a buffer to a given shader parameter.
struct QDemonApplyImageValue : public QDemonCommand
{
    QString m_imageName; ///< name which the image was allocated
    QString m_paramName; ///< must match the name in the shader
    bool m_bindAsTexture; ///< bind image as texture
    bool m_needSync; ///< if true we add a memory barrier before usage

    QDemonApplyImageValue(QString bufferName, QString shaderParam, bool inBindAsTexture, bool inNeedSync)
        : QDemonCommand(CommandTypes::ApplyImageValue)
        , m_imageName(bufferName)
        , m_paramName(shaderParam)
        , m_bindAsTexture(inBindAsTexture)
        , m_needSync(inNeedSync)
    {
    }
    QDemonApplyImageValue(const QDemonApplyImageValue &inOther)
        : QDemonCommand(CommandTypes::ApplyImageValue)
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
    QString m_paramName; ///< must match the name in the shader
    QDemonRenderBufferBindValues::Enum m_bindAs; ///< to which target we bind this buffer

    QDemonApplyDataBufferValue(QString inShaderParam, QDemonRenderBufferBindValues::Enum inBufferType)
        : QDemonCommand(CommandTypes::ApplyDataBufferValue), m_paramName(inShaderParam), m_bindAs(inBufferType)
    {
    }
    QDemonApplyDataBufferValue(const QDemonApplyDataBufferValue &inOther)
        : QDemonCommand(CommandTypes::ApplyDataBufferValue), m_paramName(inOther.m_paramName), m_bindAs(inOther.m_bindAs)
    {
    }
};

struct QDemonApplyDepthValue : public QDemonCommand
{
    // If no param name is given, the buffer is bound to the
    // input texture parameter (texture0).
    QString m_paramName;
    QDemonApplyDepthValue(QString param) : QDemonCommand(CommandTypes::ApplyDepthValue), m_paramName(param) {}
    QDemonApplyDepthValue(const QDemonApplyDepthValue &inOther)
        : QDemonCommand(CommandTypes::ApplyDepthValue), m_paramName(inOther.m_paramName)
    {
    }
};

struct QDemonRender : public QDemonCommand
{
    bool m_drawIndirect;
    explicit QDemonRender(bool inDrawIndirect) : QDemonCommand(CommandTypes::Render), m_drawIndirect(inDrawIndirect) {}

    QDemonRender(const QDemonRender &inOther)
        : QDemonCommand(CommandTypes::Render), m_drawIndirect(inOther.m_drawIndirect)
    {
    }
};

struct QDemonApplyBlending : public QDemonCommand
{
    QDemonRenderSrcBlendFunc::Enum m_srcBlendFunc;
    QDemonRenderDstBlendFunc::Enum m_dstBlendFunc;

    QDemonApplyBlending(QDemonRenderSrcBlendFunc::Enum inSrcBlendFunc, QDemonRenderDstBlendFunc::Enum inDstBlendFunc)
        : QDemonCommand(CommandTypes::ApplyBlending), m_srcBlendFunc(inSrcBlendFunc), m_dstBlendFunc(inDstBlendFunc)
    {
    }

    QDemonApplyBlending(const QDemonApplyBlending &inOther)
        : QDemonCommand(CommandTypes::ApplyBlending), m_srcBlendFunc(inOther.m_srcBlendFunc), m_dstBlendFunc(inOther.m_dstBlendFunc)
    {
    }
};

struct QDemonApplyRenderState : public QDemonCommand
{
    QDemonRenderState::Enum m_renderState;
    bool m_enabled;

    QDemonApplyRenderState(QDemonRenderState::Enum inRenderStateValue, bool inEnabled)
        : QDemonCommand(CommandTypes::ApplyRenderState), m_renderState(inRenderStateValue), m_enabled(inEnabled)
    {
    }

    QDemonApplyRenderState(const QDemonApplyRenderState &inOther)
        : QDemonCommand(CommandTypes::ApplyRenderState), m_renderState(inOther.m_renderState), m_enabled(inOther.m_enabled)
    {
    }
};

struct QDemonApplyBlitFramebuffer : public QDemonCommand
{
    // If no buffer name is given then the special buffer [source]
    // is assumed. Which is the default render target
    QString m_sourceBufferName;
    // If no buffer name is given then the special buffer [dest]
    // is assumed. Which is the default render target
    QString m_destBufferName;

    QDemonApplyBlitFramebuffer(QString inSourceBufferName, QString inDestBufferName)
        : QDemonCommand(CommandTypes::ApplyBlitFramebuffer), m_sourceBufferName(inSourceBufferName), m_destBufferName(inDestBufferName)
    {
    }

    QDemonApplyBlitFramebuffer(const QDemonApplyBlitFramebuffer &inOther)
        : QDemonCommand(CommandTypes::ApplyBlitFramebuffer)
        , m_sourceBufferName(inOther.m_sourceBufferName)
        , m_destBufferName(inOther.m_destBufferName)
    {
    }
};

struct DepthStencilFlagValues
{
    enum Enum {
        NoFlagValue = 0,
        ClearStencil = 1 << 0,
        ClearDepth = 1 << 1,
    };
};

struct QDemonDepthStencilFlags : public QDemonFlags<DepthStencilFlagValues::Enum>
{
    bool hasClearStencil() const { return operator&(DepthStencilFlagValues::ClearStencil); }
    void setClearStencil(bool value) { clearOrSet(value, DepthStencilFlagValues::ClearStencil); }

    bool hasClearDepth() const { return operator&(DepthStencilFlagValues::ClearDepth); }
    void setClearDepth(bool value) { clearOrSet(value, DepthStencilFlagValues::ClearDepth); }
};

struct QDemonDepthStencil : public QDemonCommand
{
    QString m_bufferName;
    QDemonDepthStencilFlags m_glags;
    QDemonRenderStencilOp::Enum m_stencilFailOperation = QDemonRenderStencilOp::Keep;
    QDemonRenderStencilOp::Enum m_depthPassOperation = QDemonRenderStencilOp::Keep;
    QDemonRenderStencilOp::Enum m_depthFailOperation = QDemonRenderStencilOp::Keep;
    QDemonRenderBoolOp::Enum m_stencilFunction = QDemonRenderBoolOp::Equal;
    quint32 m_reference = 0;
    quint32 m_mask = std::numeric_limits<quint32>::max();

    QDemonDepthStencil() : QDemonCommand(CommandTypes::DepthStencil) {}

    QDemonDepthStencil(QString bufName,
                       QDemonDepthStencilFlags flags,
                       QDemonRenderStencilOp::Enum inStencilOp,
                       QDemonRenderStencilOp::Enum inDepthPassOp,
                       QDemonRenderStencilOp::Enum inDepthFailOp,
                       QDemonRenderBoolOp::Enum inStencilFunc,
                       quint32 value,
                       quint32 mask)
        : QDemonCommand(CommandTypes::DepthStencil)
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
        : QDemonCommand(CommandTypes::DepthStencil)
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
