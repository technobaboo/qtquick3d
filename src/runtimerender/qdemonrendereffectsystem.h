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
#ifndef QDEMON_RENDER_EFFECT_SYSTEM_H
#define QDEMON_RENDER_EFFECT_SYSTEM_H
#include <qdemonrender.h>
#include <QDemonRefCounted>
#include <qdemonrenderbasetypes.h>
#include <StringTable.h>
#include <QVector2D.h>
#include <qdemonrenderdynamicobjectsystem.h>

QT_BEGIN_NAMESPACE
struct SEffect;
struct SEffectContext;
namespace dynamic {
struct SCommand; // UICRenderEffectCommands.h
}

struct SEffectRenderArgument
{
    SEffect &m_Effect;
    QDemonRenderTexture2D &m_ColorBuffer;
    // Some effects need the camera near and far ranges.
    QVector2D m_CameraClipRange;
    // Some effects require the depth buffer from the rendering of thelayer
    // most do not.
    QDemonRenderTexture2D *m_DepthTexture;
    // this is a depth preapass texture we need for some effects like bloom
    // actually we need the stencil values
    QDemonRenderTexture2D *m_DepthStencilBuffer;

    SEffectRenderArgument(SEffect &inEffect, QDemonRenderTexture2D &inColorBuffer,
                          const QVector2D &inCameraClipRange,
                          QDemonRenderTexture2D *inDepthTexture = nullptr,
                          QDemonRenderTexture2D *inDepthBuffer = nullptr)
        : m_Effect(inEffect)
        , m_ColorBuffer(inColorBuffer)
        , m_CameraClipRange(inCameraClipRange)
        , m_DepthTexture(inDepthTexture)
        , m_DepthStencilBuffer(inDepthBuffer)
    {
    }
};

class IEffectSystemCore : public QDemonRefCounted
{
public:
    virtual bool IsEffectRegistered(CRegisteredString inStr) = 0;
    virtual QDemonConstDataRef<CRegisteredString> GetRegisteredEffects() = 0;
    // Register an effect class that uses exactly these commands to render.
    // Effect properties cannot change after the effect is created because that would invalidate
    // existing effect instances.
    // Effect commands, which are stored on the effect class, can change.
    virtual bool RegisterEffect(CRegisteredString inName,
                                QDemonConstDataRef<dynamic::SPropertyDeclaration> inProperties) = 0;

    virtual bool UnregisterEffect(CRegisteredString inName) = 0;

    // Shorthand method that creates an effect and auto-generates the effect commands like such:
    // BindShader(inPathToEffect)
    // foreach( propdec in inProperties ) ApplyValue( propDecType )
    // ApplyShader()
    virtual bool
    RegisterGLSLEffect(CRegisteredString inName, const char8_t *inPathToEffect,
                       QDemonConstDataRef<dynamic::SPropertyDeclaration> inProperties) = 0;
    // Set the default value.  THis is unnecessary if the default is zero as that is what it is
    // assumed to be.
    virtual void SetEffectPropertyDefaultValue(CRegisteredString inName,
                                               CRegisteredString inPropName,
                                               QDemonConstDataRef<quint8> inDefaultData) = 0;
    virtual void SetEffectPropertyEnumNames(CRegisteredString inName,
                                            CRegisteredString inPropName,
                                            QDemonConstDataRef<CRegisteredString> inNames) = 0;
    virtual QDemonConstDataRef<CRegisteredString>
    GetEffectPropertyEnumNames(CRegisteredString inName,
                               CRegisteredString inPropName) const = 0;

    virtual QDemonConstDataRef<dynamic::SPropertyDefinition>
    GetEffectProperties(CRegisteredString inEffectName) const = 0;

    virtual void SetEffectPropertyTextureSettings(
            CRegisteredString inEffectName, CRegisteredString inPropName,
            CRegisteredString inPropPath, QDemonRenderTextureTypeValue::Enum inTexType,
            QDemonRenderTextureCoordOp::Enum inCoordOp, QDemonRenderTextureMagnifyingOp::Enum inMagFilterOp,
            QDemonRenderTextureMinifyingOp::Enum inMinFilterOp) = 0;

    // Setting the effect commands also sets this as if there isn't a specific "apply depth
    // value"
    // command then this effect does not require the depth texture.
    // So the setter here is completely optional.
    virtual void SetEffectRequiresDepthTexture(CRegisteredString inEffectName,
                                               bool inValue) = 0;
    virtual bool DoesEffectRequireDepthTexture(CRegisteredString inEffectName) const = 0;

    virtual void SetEffectRequiresCompilation(CRegisteredString inEffectName,
                                              bool inValue) = 0;
    virtual bool DoesEffectRequireCompilation(CRegisteredString inEffectName) const = 0;

    // The effect commands are the actual commands that run for a given effect.  The tell the
    // system exactly
    // explicitly things like bind this shader, bind this render target, apply this property,
    // run this shader
    // See UICRenderEffectCommands.h for the list of commands.
    // These commands are copied into the effect.
    virtual void SetEffectCommands(CRegisteredString inEffectName,
                                   QDemonConstDataRef<dynamic::SCommand *> inCommands) = 0;
    virtual QDemonConstDataRef<dynamic::SCommand *>
    GetEffectCommands(CRegisteredString inEffectName) const = 0;

    // Set the shader data for a given path.  Used when a path doesn't correspond to a file but
    // the data has been
    // auto-generated.  The system will look for data under this path key during the BindShader
    // effect command.
    virtual void SetShaderData(CRegisteredString inPath, const char8_t *inData,
                               const char8_t *inShaderType = nullptr,
                               const char8_t *inShaderVersion = nullptr,
                               bool inHasGeomShader = false,
                               bool inIsComputeShader = false) = 0;

    // An effect instance is just a property bag along with the name of the effect to run.
    // This instance is what is placed into the object graph.
    virtual SEffect *CreateEffectInstance(CRegisteredString inEffectName,
                                          NVAllocatorCallback &inSceneGraphAllocator) = 0;

    virtual void Save(SWriteBuffer &ioBuffer,
                      const SStrRemapMap &inRemapMap,
                      const char8_t *inProjectDir) const = 0;
    virtual void Load(QDemonDataRef<quint8> inData, CStrTableOrDataRef inStrDataBlock,
                      const char8_t *inProjectDir) = 0;

    virtual IEffectSystem &GetEffectSystem(IQt3DSRenderContext &context) = 0;

    virtual IResourceManager &GetResourceManager() = 0;

    static IEffectSystemCore &CreateEffectSystemCore(IQt3DSRenderContextCore &context);
};

/**
      * An effect is essentially a function that takes a image and produces a new image.  The source
      *and dest images
      *	aren't guaranteed to be the same size, the effect may enlarge or shrink the result.
      * A specialization is when you want the effect to render to the final render target instead of
      *to a separate image.
      * In this case the effect cannot enlarge or shrink the final target and it will render to the
      *destination buffer
      *	using the given MVP.
      */
class IEffectSystem : public IEffectSystemCore
{
protected:
    virtual ~IEffectSystem() {}

public:
    // Calling release effect context with no context results in no problems.
    virtual void ReleaseEffectContext(SEffectContext *inEffect) = 0;

    // If the effect has a context you can call this to clear persistent buffers back to their
    // original value.
    virtual void ResetEffectFrameData(SEffectContext &inContext) = 0;

    // Render this effect.  Returns false in the case the effect wasn't rendered and the render
    // state
    // is guaranteed to be the same as before.
    // The texture returned is allocated using the resource manager, and it is up to the caller
    // to deallocate it or return it to the temporary pool if items when necessary
    // Pass in true if you want the result image premultiplied.  Most of the functions in the
    // system
    // assume non-premultiplied color for images so probably this is false.
    virtual QDemonRenderTexture2D *RenderEffect(SEffectRenderArgument inRenderArgument) = 0;

    // Render the effect to the currently bound render target using this MVP and optionally
    // enabling blending when rendering to the target
    virtual bool RenderEffect(SEffectRenderArgument inRenderArgument, QMatrix4x4 &inMVP,
                              bool inEnableBlendWhenRenderToTarget) = 0;
};
QT_END_NAMESPACE
#endif
