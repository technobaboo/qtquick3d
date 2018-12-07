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
#pragma once
#ifndef QDEMON_RENDER_EFFECT_SYSTEM_COMMANDS_H
#define QDEMON_RENDER_EFFECT_SYSTEM_COMMANDS_H

#include <StringTable.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <Qt3DSIntrinsics.h>
#include <QtDemon/qdemonflags.h>

QT_BEGIN_NAMESPACE
namespace dynamic {
using QDemonRenderBufferBarrierFlags;

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

#define QDEMON_RENDER_EFFECTS_ITERATE_COMMAND_TYPES                                                   \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(AllocateBuffer)                                        \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(BindTarget)                                            \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(BindBuffer)                                            \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(BindShader)                                            \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(ApplyInstanceValue)                                    \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(ApplyBufferValue)                                      \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(ApplyDepthValue)                                       \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(Render)                                                \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(ApplyBlending)                                         \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(ApplyRenderState)                                      \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(ApplyBlitFramebuffer)                                  \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(ApplyValue)                                            \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(DepthStencil)                                          \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(AllocateImage)                                         \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(ApplyImageValue)                                       \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(AllocateDataBuffer)                                    \
    QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(ApplyDataBufferValue)

// All commands need at least two constructors.  One for when they are created that should
// setup all their member variables and one for when we are copying commands from an outside
// entity into the effect system.  We have to re-register strings in that case because we
// can't assume the outside entity was using the same string table we are...
struct SCommand
{
    CommandTypes::Enum m_Type;
    SCommand(CommandTypes::Enum inType)
        : m_Type(inType)
    {
    }
    SCommand()
        : m_Type(CommandTypes::Unknown)
    {
    }
    // Implemented in UICRenderEffectSystem.cpp
    static quint32 GetSizeofCommand(const SCommand &inCommand);
    static void CopyConstructCommand(quint8 *inDataBuffer, const SCommand &inCommand,
                                     IStringTable &inStrTable);
};

struct AllocateBufferFlagValues
{
    enum Enum {
        SceneLifetime = 1,
    };
};

struct SAllocateBufferFlags : public QDemonFlags<AllocateBufferFlagValues::Enum, quint32>
{
    SAllocateBufferFlags(quint32 inValues)
        : QDemonFlags<AllocateBufferFlagValues::Enum, quint32>(inValues)
    {
    }
    SAllocateBufferFlags() {}
    void SetSceneLifetime(bool inValue)
    {
        clearOrSet(inValue, AllocateBufferFlagValues::SceneLifetime);
    }
    // If isSceneLifetime is unset the buffer is assumed to be frame lifetime and will be
    // released after this render operation.
    bool IsSceneLifetime() const
    {
        return this->operator&(AllocateBufferFlagValues::SceneLifetime);
    }
};

struct SAllocateBuffer : public SCommand
{
    QString m_Name;
    QDemonRenderTextureFormats::Enum m_Format;
    QDemonRenderTextureMagnifyingOp::Enum m_FilterOp;
    QDemonRenderTextureCoordOp::Enum m_TexCoordOp;
    float m_SizeMultiplier;
    SAllocateBufferFlags m_BufferFlags;
    SAllocateBuffer()
        : SCommand(CommandTypes::AllocateBuffer)
        , m_Format(QDemonRenderTextureFormats::RGBA8)
        , m_FilterOp(QDemonRenderTextureMagnifyingOp::Linear)
        , m_TexCoordOp(QDemonRenderTextureCoordOp::ClampToEdge)
        , m_SizeMultiplier(1.0f)
    {
    }
    SAllocateBuffer(QString inName, QDemonRenderTextureFormats::Enum inFormat,
                    QDemonRenderTextureMagnifyingOp::Enum inFilterOp,
                    QDemonRenderTextureCoordOp::Enum inCoordOp, float inMultiplier,
                    SAllocateBufferFlags inFlags)
        : SCommand(CommandTypes::AllocateBuffer)
        , m_Name(inName)
        , m_Format(inFormat)
        , m_FilterOp(inFilterOp)
        , m_TexCoordOp(inCoordOp)
        , m_SizeMultiplier(inMultiplier)
        , m_BufferFlags(inFlags)
    {
    }
    SAllocateBuffer(const SAllocateBuffer &inOther, IStringTable &inStrTable)
        : SCommand(CommandTypes::AllocateBuffer)
        , m_Name(inStrTable.RegisterStr(inOther.m_Name))
        , m_Format(inOther.m_Format)
        , m_FilterOp(inOther.m_FilterOp)
        , m_TexCoordOp(inOther.m_TexCoordOp)
        , m_SizeMultiplier(inOther.m_SizeMultiplier)
        , m_BufferFlags(inOther.m_BufferFlags)
    {
    }
};

struct SAllocateImage : public SAllocateBuffer
{
    QDemonRenderImageAccessType::Enum m_Access;

    SAllocateImage()
        : SAllocateBuffer()
        , m_Access(QDemonRenderImageAccessType::ReadWrite)
    {
        m_Type = CommandTypes::AllocateImage;
    }
    SAllocateImage(QString inName, QDemonRenderTextureFormats::Enum inFormat,
                   QDemonRenderTextureMagnifyingOp::Enum inFilterOp,
                   QDemonRenderTextureCoordOp::Enum inCoordOp, float inMultiplier,
                   SAllocateBufferFlags inFlags, QDemonRenderImageAccessType::Enum inAccess)
        : SAllocateBuffer(inName, inFormat, inFilterOp, inCoordOp, inMultiplier, inFlags)
        , m_Access(inAccess)
    {
        m_Type = CommandTypes::AllocateImage;
    }

    SAllocateImage(const SAllocateImage &inOther, IStringTable &inStrTable)
        : SAllocateBuffer(inStrTable.RegisterStr(inOther.m_Name), inOther.m_Format,
                          inOther.m_FilterOp, inOther.m_TexCoordOp,
                          inOther.m_SizeMultiplier, inOther.m_BufferFlags)
        , m_Access(inOther.m_Access)
    {
        m_Type = CommandTypes::AllocateImage;
    }
};

struct SAllocateDataBuffer : public SCommand
{
    QString m_Name;
    QDemonRenderBufferBindValues::Enum m_DataBufferType;
    QString m_WrapName;
    QDemonRenderBufferBindValues::Enum m_DataBufferWrapType;
    float m_Size;
    SAllocateBufferFlags m_BufferFlags;

    SAllocateDataBuffer()
        : SCommand(CommandTypes::AllocateDataBuffer)
    {
    }

    SAllocateDataBuffer(QString inName,
                        QDemonRenderBufferBindValues::Enum inBufferType,
                        QString inWrapName,
                        QDemonRenderBufferBindValues::Enum inBufferWrapType, float inSize,
                        SAllocateBufferFlags inFlags)
        : SCommand(CommandTypes::AllocateDataBuffer)
        , m_Name(inName)
        , m_DataBufferType(inBufferType)
        , m_WrapName(inWrapName)
        , m_DataBufferWrapType(inBufferWrapType)
        , m_Size(inSize)
        , m_BufferFlags(inFlags)
    {
    }

    SAllocateDataBuffer(const SAllocateDataBuffer &inOther, IStringTable &inStrTable)
        : SCommand(CommandTypes::AllocateDataBuffer)
        , m_Name(inStrTable.RegisterStr(inOther.m_Name))
        , m_DataBufferType(inOther.m_DataBufferType)
        , m_WrapName(inStrTable.RegisterStr(inOther.m_WrapName))
        , m_DataBufferWrapType(inOther.m_DataBufferWrapType)
        , m_Size(inOther.m_Size)
        , m_BufferFlags(inOther.m_BufferFlags)
    {
    }
};

struct SBindTarget : public SCommand
{
    QDemonRenderTextureFormats::Enum m_OutputFormat;

    SBindTarget(QDemonRenderTextureFormats::Enum inFormat = QDemonRenderTextureFormats::RGBA8)
        : SCommand(CommandTypes::BindTarget)
        , m_OutputFormat(inFormat)
    {
    }
    SBindTarget(const SBindTarget &inOther, IStringTable &)
        : SCommand(CommandTypes::BindTarget)
        , m_OutputFormat(inOther.m_OutputFormat)
    {
    }
};

struct SBindBuffer : public SCommand
{
    QString m_BufferName;
    bool m_NeedsClear;
    SBindBuffer(QString inBufName, bool inNeedsClear)
        : SCommand(CommandTypes::BindBuffer)
        , m_BufferName(inBufName)
        , m_NeedsClear(inNeedsClear)
    {
    }
    SBindBuffer(const SBindBuffer &inOther, IStringTable &inTable)
        : SCommand(CommandTypes::BindBuffer)
        , m_BufferName(inTable.RegisterStr(inOther.m_BufferName))
        , m_NeedsClear(inOther.m_NeedsClear)
    {
    }
};

struct SBindShader : public SCommand
{
    QString m_ShaderPath;
    // One GLSL file can hold multiple shaders in the case of multipass effects.
    // This makes it significantly easier for authors to reason about the shader
    // but it means we need to #define a preprocessor token to indicate which
    // effect we intend to compile at this point.
    QString m_ShaderDefine;
    SBindShader(QString inShaderPath,
                QString inShaderDefine = QString())
        : SCommand(CommandTypes::BindShader)
        , m_ShaderPath(inShaderPath)
        , m_ShaderDefine(inShaderDefine)
    {
    }
    SBindShader()
        : SCommand(CommandTypes::BindShader)
    {
    }
    SBindShader(const SBindShader &inOther, IStringTable &inTable)
        : SCommand(CommandTypes::BindShader)
        , m_ShaderPath(inTable.RegisterStr(inOther.m_ShaderPath))
        , m_ShaderDefine(inTable.RegisterStr(inOther.m_ShaderDefine))
    {
    }
};

// The value sits immediately after the 'this' object
// in memory.
// If propertyName is not valid then we attempt to apply all of the effect property values
// to the shader, ignoring ones that don't match up.
struct SApplyInstanceValue : public SCommand
{
    // Name of value to apply in shader
    QString m_PropertyName;
    // type of value
    QDemonRenderShaderDataTypes::Enum m_ValueType;
    // offset in the effect data section of value.
    quint32 m_ValueOffset;
    SApplyInstanceValue(QString inName, QDemonRenderShaderDataTypes::Enum inValueType,
                        quint32 inValueOffset)
        : SCommand(CommandTypes::ApplyInstanceValue)
        , m_PropertyName(inName)
        , m_ValueType(inValueType)
        , m_ValueOffset(inValueOffset)
    {
    }
    // Default will attempt to apply all effect values to the currently bound shader
    SApplyInstanceValue()
        : SCommand(CommandTypes::ApplyInstanceValue)
        , m_ValueType(QDemonRenderShaderDataTypes::Unknown)
        , m_ValueOffset(0)
    {
    }
    SApplyInstanceValue(const SApplyInstanceValue &inOther, IStringTable &inTable)
        : SCommand(CommandTypes::ApplyInstanceValue)
        , m_PropertyName(inTable.RegisterStr(inOther.m_PropertyName))
        , m_ValueType(inOther.m_ValueType)
        , m_ValueOffset(inOther.m_ValueOffset)
    {
    }
};

struct SApplyValue : public SCommand
{
    QString m_PropertyName;
    QDemonRenderShaderDataTypes::Enum m_ValueType;
    QDemonDataRef<quint8> m_Value;
    SApplyValue(QString inName, QDemonRenderShaderDataTypes::Enum inValueType)
        : SCommand(CommandTypes::ApplyValue)
        , m_PropertyName(inName)
        , m_ValueType(inValueType)
    {
    }
    // Default will attempt to apply all effect values to the currently bound shader
    SApplyValue()
        : SCommand(CommandTypes::ApplyValue)
        , m_ValueType(QDemonRenderShaderDataTypes::Unknown)
    {
    }

    SApplyValue(const SApplyValue &inOther, IStringTable &inTable)
        : SCommand(CommandTypes::ApplyValue)
        , m_PropertyName(inTable.RegisterStr(inOther.m_PropertyName))
        , m_ValueType(inOther.m_ValueType)
        , m_Value(inOther.m_Value)
    {
    }
};

// bind a buffer to a given shader parameter.
struct SApplyBufferValue : public SCommand
{
    // If no buffer name is given then the special buffer [source]
    // is assumed.
    QString m_BufferName;
    // If no param name is given, the buffer is bound to the
    // input texture parameter (texture0).
    QString m_ParamName;

    SApplyBufferValue(QString bufferName, QString shaderParam)
        : SCommand(CommandTypes::ApplyBufferValue)
        , m_BufferName(bufferName)
        , m_ParamName(shaderParam)
    {
    }
    SApplyBufferValue(const SApplyBufferValue &inOther, IStringTable &inTable)
        : SCommand(CommandTypes::ApplyBufferValue)
        , m_BufferName(inTable.RegisterStr(inOther.m_BufferName))
        , m_ParamName(inTable.RegisterStr(inOther.m_ParamName))
    {
    }
};

// bind a buffer to a given shader parameter.
struct SApplyImageValue : public SCommand
{
    QString m_ImageName; ///< name which the image was allocated
    QString m_ParamName; ///< must match the name in the shader
    bool m_BindAsTexture; ///< bind image as texture
    bool m_NeedSync; ///< if true we add a memory barrier before usage

    SApplyImageValue(QString bufferName, QString shaderParam,
                     bool inBindAsTexture, bool inNeedSync)
        : SCommand(CommandTypes::ApplyImageValue)
        , m_ImageName(bufferName)
        , m_ParamName(shaderParam)
        , m_BindAsTexture(inBindAsTexture)
        , m_NeedSync(inNeedSync)
    {
    }
    SApplyImageValue(const SApplyImageValue &inOther, IStringTable &inTable)
        : SCommand(CommandTypes::ApplyImageValue)
        , m_ImageName(inTable.RegisterStr(inOther.m_ImageName))
        , m_ParamName(inTable.RegisterStr(inOther.m_ParamName))
        , m_BindAsTexture(inOther.m_BindAsTexture)
        , m_NeedSync(inOther.m_NeedSync)
    {
    }
};

// bind a buffer to a given shader parameter.
struct SApplyDataBufferValue : public SCommand
{
    QString m_ParamName; ///< must match the name in the shader
    QDemonRenderBufferBindValues::Enum m_BindAs; ///< to which target we bind this buffer

    SApplyDataBufferValue(QString inShaderParam,
                          QDemonRenderBufferBindValues::Enum inBufferType)
        : SCommand(CommandTypes::ApplyDataBufferValue)
        , m_ParamName(inShaderParam)
        , m_BindAs(inBufferType)
    {
    }
    SApplyDataBufferValue(const SApplyDataBufferValue &inOther, IStringTable &inTable)
        : SCommand(CommandTypes::ApplyDataBufferValue)
        , m_ParamName(inTable.RegisterStr(inOther.m_ParamName))
        , m_BindAs(inOther.m_BindAs)
    {
    }
};

struct SApplyDepthValue : public SCommand
{
    // If no param name is given, the buffer is bound to the
    // input texture parameter (texture0).
    QString m_ParamName;
    SApplyDepthValue(QString param)
        : SCommand(CommandTypes::ApplyDepthValue)
        , m_ParamName(param)
    {
    }
    SApplyDepthValue(const SApplyDepthValue &inOther, IStringTable &inTable)
        : SCommand(CommandTypes::ApplyDepthValue)
        , m_ParamName(inTable.RegisterStr(inOther.m_ParamName))
    {
    }
};

struct SRender : public SCommand
{
    bool m_DrawIndirect;
    SRender(bool inDrawIndirect)
        : SCommand(CommandTypes::Render)
        , m_DrawIndirect(inDrawIndirect)
    {
    }

    SRender(const SRender &inOther, IStringTable &)
        : SCommand(CommandTypes::Render)
        , m_DrawIndirect(inOther.m_DrawIndirect)
    {
    }
};

struct SApplyBlending : public SCommand
{
    QDemonRenderSrcBlendFunc::Enum m_SrcBlendFunc;
    QDemonRenderDstBlendFunc::Enum m_DstBlendFunc;

    SApplyBlending(QDemonRenderSrcBlendFunc::Enum inSrcBlendFunc,
                   QDemonRenderDstBlendFunc::Enum inDstBlendFunc)
        : SCommand(CommandTypes::ApplyBlending)
        , m_SrcBlendFunc(inSrcBlendFunc)
        , m_DstBlendFunc(inDstBlendFunc)
    {
    }

    SApplyBlending(const SApplyBlending &inOther, IStringTable &)
        : SCommand(CommandTypes::ApplyBlending)
        , m_SrcBlendFunc(inOther.m_SrcBlendFunc)
        , m_DstBlendFunc(inOther.m_DstBlendFunc)
    {
    }
};

struct SApplyRenderState : public SCommand
{
    QDemonRenderState::Enum m_RenderState;
    bool m_Enabled;

    SApplyRenderState(QDemonRenderState::Enum inRenderStateValue, bool inEnabled)
        : SCommand(CommandTypes::ApplyRenderState)
        , m_RenderState(inRenderStateValue)
        , m_Enabled(inEnabled)
    {
    }

    SApplyRenderState(const SApplyRenderState &inOther, IStringTable &)
        : SCommand(CommandTypes::ApplyRenderState)
        , m_RenderState(inOther.m_RenderState)
        , m_Enabled(inOther.m_Enabled)
    {
    }
};

struct SApplyBlitFramebuffer : public SCommand
{
    // If no buffer name is given then the special buffer [source]
    // is assumed. Which is the default render target
    QString m_SourceBufferName;
    // If no buffer name is given then the special buffer [dest]
    // is assumed. Which is the default render target
    QString m_DestBufferName;

    SApplyBlitFramebuffer(QString inSourceBufferName,
                          QString inDestBufferName)
        : SCommand(CommandTypes::ApplyBlitFramebuffer)
        , m_SourceBufferName(inSourceBufferName)
        , m_DestBufferName(inDestBufferName)
    {
    }

    SApplyBlitFramebuffer(const SApplyBlitFramebuffer &inOther, IStringTable &inTable)
        : SCommand(CommandTypes::ApplyBlitFramebuffer)
        , m_SourceBufferName(inTable.RegisterStr(inOther.m_SourceBufferName))
        , m_DestBufferName(inTable.RegisterStr(inOther.m_DestBufferName))
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

struct SDepthStencilFlags : public QDemonFlags<DepthStencilFlagValues::Enum>
{
    bool HasClearStencil() const { return operator&(DepthStencilFlagValues::ClearStencil); }
    void SetClearStencil(bool value)
    {
        clearOrSet(value, DepthStencilFlagValues::ClearStencil);
    }

    bool HasClearDepth() const { return operator&(DepthStencilFlagValues::ClearDepth); }
    void SetClearDepth(bool value)
    {
        clearOrSet(value, DepthStencilFlagValues::ClearDepth);
    }
};

struct SDepthStencil : public SCommand
{
    QString m_BufferName;
    SDepthStencilFlags m_Flags;
    QDemonRenderStencilOp::Enum m_StencilFailOperation;
    QDemonRenderStencilOp::Enum m_DepthPassOperation;
    QDemonRenderStencilOp::Enum m_DepthFailOperation;
    QDemonRenderBoolOp::Enum m_StencilFunction;
    quint32 m_Reference;
    quint32 m_Mask;

    SDepthStencil()
        : SCommand(CommandTypes::DepthStencil)
        , m_StencilFailOperation(QDemonRenderStencilOp::Keep)
        , m_DepthPassOperation(QDemonRenderStencilOp::Keep)
        , m_DepthFailOperation(QDemonRenderStencilOp::Keep)
        , m_StencilFunction(QDemonRenderBoolOp::Equal)
        , m_Reference(0)
        , m_Mask(QDEMON_MAX_U32)
    {
    }

    SDepthStencil(QString bufName, SDepthStencilFlags flags,
                  QDemonRenderStencilOp::Enum inStencilOp,
                  QDemonRenderStencilOp::Enum inDepthPassOp,
                  QDemonRenderStencilOp::Enum inDepthFailOp,
                  QDemonRenderBoolOp::Enum inStencilFunc, quint32 value, quint32 mask)
        : SCommand(CommandTypes::DepthStencil)
        , m_BufferName(bufName)
        , m_Flags(flags)
        , m_StencilFailOperation(inStencilOp)
        , m_DepthPassOperation(inDepthPassOp)
        , m_DepthFailOperation(inDepthFailOp)
        , m_StencilFunction(inStencilFunc)
        , m_Reference(value)
        , m_Mask(mask)
    {
    }

    SDepthStencil(const SDepthStencil &inOther, IStringTable &inTable)
        : SCommand(CommandTypes::DepthStencil)
        , m_BufferName(inTable.RegisterStr(inOther.m_BufferName))
        , m_Flags(inOther.m_Flags)
        , m_StencilFailOperation(inOther.m_StencilFailOperation)
        , m_DepthPassOperation(inOther.m_DepthPassOperation)
        , m_DepthFailOperation(inOther.m_DepthFailOperation)
        , m_StencilFunction(inOther.m_StencilFunction)
        , m_Reference(inOther.m_Reference)
        , m_Mask(inOther.m_Mask)
    {
    }
};
}
QT_END_NAMESPACE

#endif
