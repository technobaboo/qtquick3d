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
#ifndef QDEMON_RENDER_EFFECT_SYSTEM_H
#define QDEMON_RENDER_EFFECT_SYSTEM_H

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRuntimeRender/qdemonrenderdynamicobjectsystem.h>
#include <QtGui/QVector2D>

QT_BEGIN_NAMESPACE
struct QDemonRenderEffect;
struct QDemonEffectContext;
class QDemonEffectSystemInterface;
class QDemonResourceManagerInterface;

namespace dynamic {
struct QDemonCommand; // UICRenderEffectCommands.h
}

struct QDemonEffectRenderArgument
{
    QDemonRenderEffect *m_effect;
    QDemonRef<QDemonRenderTexture2D> m_colorBuffer;
    // Some effects need the camera near and far ranges.
    QVector2D m_cameraClipRange;
    // Some effects require the depth buffer from the rendering of thelayer
    // most do not.
    QDemonRef<QDemonRenderTexture2D> m_depthTexture;
    // this is a depth preapass texture we need for some effects like bloom
    // actually we need the stencil values
    QDemonRef<QDemonRenderTexture2D> m_depthStencilBuffer;

    QDemonEffectRenderArgument(QDemonRenderEffect *inEffect,
                               const QDemonRef<QDemonRenderTexture2D> &inColorBuffer,
                               const QVector2D &inCameraClipRange,
                               const QDemonRef<QDemonRenderTexture2D> &inDepthTexture = nullptr,
                               const QDemonRef<QDemonRenderTexture2D> &inDepthBuffer = nullptr);
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
class Q_DEMONRUNTIMERENDER_EXPORT QDemonEffectSystemInterface
{
public:
    QAtomicInt ref;
    virtual ~QDemonEffectSystemInterface();
    virtual bool isEffectRegistered(QString inStr) = 0;
    virtual QVector<QString> getRegisteredEffects() = 0;
    // Register an effect class that uses exactly these commands to render.
    // Effect properties cannot change after the effect is created because that would invalidate
    // existing effect instances.
    // Effect commands, which are stored on the effect class, can change.
    virtual bool registerEffect(QString inName, QDemonConstDataRef<dynamic::QDemonPropertyDeclaration> inProperties) = 0;

    virtual bool unregisterEffect(QString inName) = 0;

    // Shorthand method that creates an effect and auto-generates the effect commands like such:
    // BindShader(inPathToEffect)
    // foreach( propdec in inProperties ) ApplyValue( propDecType )
    // ApplyShader()
    virtual bool registerGLSLEffect(QString inName,
                                    const char *inPathToEffect,
                                    QDemonConstDataRef<dynamic::QDemonPropertyDeclaration> inProperties) = 0;
    // Set the default value.  THis is unnecessary if the default is zero as that is what it is
    // assumed to be.
    virtual void setEffectPropertyDefaultValue(QString inName, QString inPropName, QDemonConstDataRef<quint8> inDefaultData) = 0;
    virtual void setEffectPropertyEnumNames(QString inName, QString inPropName, QDemonConstDataRef<QString> inNames) = 0;
    virtual QDemonConstDataRef<QString> getEffectPropertyEnumNames(QString inName, QString inPropName) const = 0;

    virtual QDemonConstDataRef<dynamic::QDemonPropertyDefinition> getEffectProperties(QString inEffectName) const = 0;

    virtual void setEffectPropertyTextureSettings(QString inEffectName,
                                                  QString inPropName,
                                                  QString inPropPath,
                                                  QDemonRenderTextureTypeValue::Enum inTexType,
                                                  QDemonRenderTextureCoordOp::Enum inCoordOp,
                                                  QDemonRenderTextureMagnifyingOp::Enum inMagFilterOp,
                                                  QDemonRenderTextureMinifyingOp::Enum inMinFilterOp) = 0;

    // Setting the effect commands also sets this as if there isn't a specific "apply depth
    // value"
    // command then this effect does not require the depth texture.
    // So the setter here is completely optional.
    virtual void setEffectRequiresDepthTexture(QString inEffectName, bool inValue) = 0;
    virtual bool doesEffectRequireDepthTexture(QString inEffectName) const = 0;

    virtual void setEffectRequiresCompilation(QString inEffectName, bool inValue) = 0;
    virtual bool doesEffectRequireCompilation(QString inEffectName) const = 0;

    // The effect commands are the actual commands that run for a given effect.  The tell the
    // system exactly
    // explicitly things like bind this shader, bind this render target, apply this property,
    // run this shader
    // See UICRenderEffectCommands.h for the list of commands.
    // These commands are copied into the effect.
    virtual void setEffectCommands(QString inEffectName, QDemonConstDataRef<dynamic::QDemonCommand *> inCommands) = 0;
    virtual QDemonConstDataRef<dynamic::QDemonCommand *> getEffectCommands(QString inEffectName) const = 0;

    // Set the shader data for a given path.  Used when a path doesn't correspond to a file but
    // the data has been
    // auto-generated.  The system will look for data under this path key during the BindShader
    // effect command.
    virtual void setShaderData(QString inPath,
                               const char *inData,
                               const char *inShaderType = nullptr,
                               const char *inShaderVersion = nullptr,
                               bool inHasGeomShader = false,
                               bool inIsComputeShader = false) = 0;

    // An effect instance is just a property bag along with the name of the effect to run.
    // This instance is what is placed into the object graph.
    virtual QDemonRenderEffect *createEffectInstance(QString inEffectName) = 0;

    //    virtual void Save(SWriteBuffer &ioBuffer,
    //                      const SStrRemapMap &inRemapMap,
    //                      const char *inProjectDir) const = 0;
    //    virtual void Load(QDemonDataRef<quint8> inData, CStrTableOrDataRef inStrDataBlock,
    //                      const char *inProjectDir) = 0;

    virtual QDemonRef<QDemonEffectSystemInterface> getEffectSystem(QDemonRenderContextInterface *context) = 0;

    virtual QDemonRef<QDemonResourceManagerInterface> getResourceManager() = 0;

    static QDemonRef<QDemonEffectSystemInterface> createEffectSystem(QDemonRenderContextCoreInterface *context);

    // Calling release effect context with no context results in no problems.
    virtual void releaseEffectContext(QDemonEffectContext *inEffect) = 0;

    // If the effect has a context you can call this to clear persistent buffers back to their
    // original value.
    virtual void resetEffectFrameData(QDemonEffectContext &inContext) = 0;

    // Render this effect.  Returns false in the case the effect wasn't rendered and the render
    // state
    // is guaranteed to be the same as before.
    // The texture returned is allocated using the resource manager, and it is up to the caller
    // to deallocate it or return it to the temporary pool if items when necessary
    // Pass in true if you want the result image premultiplied.  Most of the functions in the
    // system
    // assume non-premultiplied color for images so probably this is false.
    virtual QDemonRef<QDemonRenderTexture2D> renderEffect(QDemonEffectRenderArgument inRenderArgument) = 0;

    // Render the effect to the currently bound render target using this MVP and optionally
    // enabling blending when rendering to the target
    virtual bool renderEffect(QDemonEffectRenderArgument inRenderArgument, QMatrix4x4 &inMVP, bool inEnableBlendWhenRenderToTarget) = 0;
};

QT_END_NAMESPACE
#endif
