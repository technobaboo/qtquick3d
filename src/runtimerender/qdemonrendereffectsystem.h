/****************************************************************************
**
** Copyright (C) 2008-2012 NVIDIA Corporation.
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Quick 3D.
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
#include <QtDemon/qdemonoption.h>
#include <QtDemonRuntimeRender/qdemonrenderdynamicobjectsystem.h>
#include <QtGui/QVector2D>

QT_BEGIN_NAMESPACE
struct QDemonRenderEffect;
struct QDemonEffectContext;
struct QDemonEffectShader;
struct QDemonEffectClass;
class QDemonEffectSystemInterface;
class QDemonResourceManager;
class QDemonRenderDepthStencilState;

namespace dynamic {
struct QDemonCommand;
struct QDemonApplyInstanceValue;
struct QDemonBindBuffer;
struct QDemonApplyValue;
struct QDemonApplyBlending;
struct QDemonAllocateBuffer;
struct QDemonBindShader;
struct QDemonAllocateImage;
struct QDemonAllocateDataBuffer;
struct QDemonApplyBufferValue;
struct QDemonApplyDepthValue;
struct QDemonApplyDataBufferValue;
struct QDemonApplyRenderState;
struct QDemonApplyImageValue;
struct QDemonDepthStencil;
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

struct QDemonEffectTextureData
{
    QDemonRef<QDemonRenderTexture2D> texture;
    bool needsAlphaMultiply = false;
    QDemonEffectTextureData(const QDemonRef<QDemonRenderTexture2D> &inTexture, bool inNeedsMultiply);
    QDemonEffectTextureData() = default;
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
class Q_DEMONRUNTIMERENDER_EXPORT QDemonEffectSystem
{
    Q_DISABLE_COPY(QDemonEffectSystem)

    typedef QHash<QString, char *> TPathDataMap;
    typedef QSet<QString> TPathSet;
    typedef QHash<QByteArray, QDemonRef<QDemonEffectClass>> TEffectClassMap;
    typedef QHash<TStrStrPair, QDemonRef<QDemonEffectShader>> TShaderMap;
    typedef QVector<QDemonRef<QDemonEffectContext>> TContextList;

    QDemonRenderContextInterface *m_context;
    QDemonRef<QDemonResourceManager> m_resourceManager;
    // Keep from dual-including headers.
    TEffectClassMap m_effectClasses;
    QVector<QByteArray> m_effectList;
    TContextList m_contexts;
    QString m_textureStringBuilder;
    QString m_textureStringBuilder2;
    TShaderMap m_shaderMap;
    QDemonRef<QDemonRenderDepthStencilState> m_defaultStencilState;
    QVector<QDemonRef<QDemonRenderDepthStencilState>> m_depthStencilStates;

public:
    QAtomicInt ref;
    QDemonEffectSystem(QDemonRenderContextInterface *inContext);

    ~QDemonEffectSystem();

    QDemonEffectContext &getEffectContext(QDemonRenderEffect &inEffect);

    const QDemonRef<QDemonEffectClass> getEffectClass(const QByteArray &inStr) const;

    bool isEffectRegistered(const QByteArray &inStr);
    QVector<QByteArray> getRegisteredEffects();

    // Set the default value.  THis is unnecessary if the default is zero as that is what it is
    // assumed to be.
    void setEffectPropertyDefaultValue(QString inName, QString inPropName, QDemonByteView inDefaultData);

    void setEffectPropertyEnumNames(QString inName, QString inPropName, QDemonDataView<QString> inNames);

    // Register an effect class that uses exactly these commands to render.
    // Effect properties cannot change after the effect is created because that would invalidate
    // existing effect instances.
    // Effect commands, which are stored on the effect class, can change.
    bool registerEffect(const QByteArray &inName /*, QDemonDataView<dynamic::QDemonPropertyDeclaration> inProperties*/);

    bool unregisterEffect(const QByteArray &inName);

    void setEffectPropertyTextureSettings(QString inName,
                                          QString inPropName,
                                          QString inPropPath,
                                          QDemonRenderTextureTypeValue inTexType,
                                          QDemonRenderTextureCoordOp inCoordOp,
                                          QDemonRenderTextureMagnifyingOp inMagFilterOp,
                                          QDemonRenderTextureMinifyingOp inMinFilterOp);

    // Setting the effect commands also sets this as if there isn't a specific "apply depth
    // value"
    // command then this effect does not require the depth texture.
    // So the setter here is completely optional.
    void setEffectRequiresDepthTexture(const QByteArray &inEffectName, bool inValue);

    bool doesEffectRequireDepthTexture(const QByteArray &inEffectName) const;

    void setEffectRequiresCompilation(const QByteArray &inEffectName, bool inValue);

    bool doesEffectRequireCompilation(const QByteArray &inEffectName) const;

    // An effect instance is just a property bag along with the name of the effect to run.
    // This instance is what is placed into the object graph.
    QDemonRenderEffect *createEffectInstance(const QByteArray &inEffectName);

    void allocateBuffer(QDemonRenderEffect &inEffect,
                        const dynamic::QDemonAllocateBuffer &inCommand,
                        quint32 inFinalWidth,
                        quint32 inFinalHeight,
                        QDemonRenderTextureFormat inSourceTextureFormat);

    void allocateImage(QDemonRenderEffect &inEffect,
                       const dynamic::QDemonAllocateImage &inCommand,
                       quint32 inFinalWidth,
                       quint32 inFinalHeight);

    void allocateDataBuffer(QDemonRenderEffect &inEffect, const dynamic::QDemonAllocateDataBuffer &inCommand);

    QDemonRef<QDemonRenderTexture2D> findTexture(QDemonRenderEffect *inEffect, const QByteArray &inName);

    QDemonRef<QDemonRenderFrameBuffer> bindBuffer(QDemonRenderEffect &inEffect,
                                                  const dynamic::QDemonBindBuffer &inCommand,
                                                  QMatrix4x4 &outMVP,
                                                  QVector2D &outDestSize);

    QDemonRef<QDemonEffectShader> bindShader(const QByteArray &inEffectId, const dynamic::QDemonBindShader &inCommand);

    void doApplyInstanceValue(QDemonRenderEffect *inEffect,
                              const QByteArray &inPropertyName,
                              const QVariant &propertyValue,
                              QDemonRenderShaderDataType inPropertyType,
                              const QDemonRef<QDemonRenderShaderProgram> &inShader);

    void applyInstanceValue(QDemonRenderEffect *inEffect,
                            const QDemonRef<QDemonEffectClass> &inClass,
                            const QDemonRef<QDemonRenderShaderProgram> &inShader,
                            const dynamic::QDemonApplyInstanceValue &inCommand);

    void applyValue(QDemonRenderEffect *inEffect,
                    const QDemonRef<QDemonEffectClass> &inClass,
                    const QDemonRef<QDemonRenderShaderProgram> &inShader,
                    const dynamic::QDemonApplyValue &inCommand);

    bool applyBlending(const dynamic::QDemonApplyBlending &inCommand);

    // This has the potential to change the source texture for the current render pass
    QDemonEffectTextureData applyBufferValue(QDemonRenderEffect *inEffect,
                                             const QDemonRef<QDemonRenderShaderProgram> &inShader,
                                             const dynamic::QDemonApplyBufferValue &inCommand,
                                             const QDemonRef<QDemonRenderTexture2D> &inSourceTexture,
                                             const QDemonEffectTextureData &inCurrentSourceTexture);

    void applyDepthValue(QDemonRenderEffect *inEffect,
                         const QDemonRef<QDemonRenderShaderProgram> &inShader,
                         const dynamic::QDemonApplyDepthValue &inCommand,
                         const QDemonRef<QDemonRenderTexture2D> &inTexture);

    void applyImageValue(QDemonRenderEffect *inEffect,
                         const QDemonRef<QDemonRenderShaderProgram> &inShader,
                         const dynamic::QDemonApplyImageValue &inCommand);

    void applyDataBufferValue(QDemonRenderEffect *inEffect,
                              const QDemonRef<QDemonRenderShaderProgram> &inShader,
                              const dynamic::QDemonApplyDataBufferValue &inCommand);

    void applyRenderStateValue(QDemonRenderFrameBuffer *inTarget,
                               const QDemonRef<QDemonRenderTexture2D> &inDepthStencilTexture,
                               const dynamic::QDemonApplyRenderState &theCommand);

    static bool compareDepthStencilState(QDemonRenderDepthStencilState &inState, dynamic::QDemonDepthStencil &inStencil);

    void renderPass(QDemonEffectShader &inShader,
                    const QMatrix4x4 &inMVP,
                    const QDemonEffectTextureData &inSourceTexture,
                    const QDemonRef<QDemonRenderFrameBuffer> &inFrameBuffer,
                    QVector2D &inDestSize,
                    const QVector2D &inCameraClipRange,
                    const QDemonRef<QDemonRenderTexture2D> &inDepthStencil,
                    QDemonOption<dynamic::QDemonDepthStencil> inDepthStencilCommand,
                    bool drawIndirect);

    void doRenderEffect(QDemonRenderEffect *inEffect,
                        const QDemonRef<QDemonEffectClass> &inClass,
                        const QDemonRef<QDemonRenderTexture2D> &inSourceTexture,
                        QMatrix4x4 &inMVP,
                        const QDemonRef<QDemonRenderFrameBuffer> &inTarget,
                        bool inEnableBlendWhenRenderToTarget,
                        const QDemonRef<QDemonRenderTexture2D> &inDepthTexture,
                        const QDemonRef<QDemonRenderTexture2D> &inDepthStencilTexture,
                        const QVector2D &inCameraClipRange);

    // Render this effect.  Returns false in the case the effect wasn't rendered and the render
    // state
    // is guaranteed to be the same as before.
    // The texture returned is allocated using the resource manager, and it is up to the caller
    // to deallocate it or return it to the temporary pool if items when necessary
    // Pass in true if you want the result image premultiplied.  Most of the functions in the
    // system
    // assume non-premultiplied color for images so probably this is false.
    QDemonRef<QDemonRenderTexture2D> renderEffect(QDemonEffectRenderArgument inRenderArgument);

    // Render the effect to the currently bound render target using this MVP and optionally
    // enabling blending when rendering to the target
    bool renderEffect(QDemonEffectRenderArgument inRenderArgument, QMatrix4x4 &inMVP, bool inEnableBlendWhenRenderToTarget);

    // Calling release effect context with no context results in no problems.
    void releaseEffectContext(QDemonEffectContext *inContext);

    // If the effect has a context you can call this to clear persistent buffers back to their
    // original value.
    void resetEffectFrameData(QDemonEffectContext &inContext);

    // Set the shader data for a given path.  Used when a path doesn't correspond to a file but
    // the data has been
    // auto-generated.  The system will look for data under this path key during the BindShader
    // effect command.
    void setShaderData(const QByteArray &path, const char *data, const char *inShaderType, const char *inShaderVersion, bool inHasGeomShader, bool inIsComputeShader);

    void init();

    QDemonRef<QDemonResourceManager> getResourceManager();
};

QT_END_NAMESPACE
#endif
