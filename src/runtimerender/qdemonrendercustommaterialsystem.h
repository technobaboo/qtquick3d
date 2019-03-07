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
#ifndef QDEMON_RENDER_CUSTOM_MATERIAL_SYSTEM_H
#define QDEMON_RENDER_CUSTOM_MATERIAL_SYSTEM_H

#include <QtDemonRuntimeRender/qtdemonruntimerenderglobal.h>

#include <QtDemonRuntimeRender/qdemonrenderdynamicobjectsystem.h>
#include <QtDemonRuntimeRender/qdemonvertexpipelineimpl.h>

#include <QtCore/qhash.h>

QT_BEGIN_NAMESPACE

namespace dynamic {
struct QDemonCommand;
}

struct QDemonCustomMaterialRenderContext;
struct QDemonRenderCustomMaterial;
class QDemonMaterialSystem;
struct QDemonRenderSubset;
struct QDemonShaderMapKey;
struct QDemonCustomMaterialShader;
struct QDemonMaterialClass;
struct QDemonCustomMaterialTextureData;
struct QDemonCustomMaterialBuffer;
struct QDemonMaterialOrComputeShader;
namespace dynamic {
    struct QDemonBindShader;
    struct QDemonApplyInstanceValue;
    struct QDemonApplyBlending;
    struct QDemonAllocateBuffer;
    struct QDemonApplyBufferValue;
    struct QDemonBindBuffer;
    struct QDemonApplyBlitFramebuffer;
}

// How to handle blend modes?
struct QDemonRenderModel;

class Q_DEMONRUNTIMERENDER_EXPORT QDemonMaterialSystem
{
    struct Private {
        ~Private();
        QAtomicInt ref;
        typedef QHash<QDemonShaderMapKey, QDemonRef<QDemonCustomMaterialShader>> ShaderMap;
        typedef QPair<QString, QDemonRenderImage *> AllocatedImageEntry;
        typedef QHash<QString, QDemonRef<QDemonMaterialClass>> StringMaterialMap;
        typedef QPair<QString, QString> TStrStrPair;
        typedef QPair<QString, QDemonRef<QDemonCustomMaterialTextureData>> CustomMaterialTextureEntry;

        QDemonRenderContextCoreInterface *coreContext = nullptr;
        QDemonRenderContextInterface *context = nullptr;
        StringMaterialMap stringMaterialMap;
        ShaderMap shaderMap;
        QVector<CustomMaterialTextureEntry> textureEntries;
        QVector<QDemonCustomMaterialBuffer> allocatedBuffers;
        QVector<AllocatedImageEntry> allocatedImages;
        bool useFastBlits = true;
        QString shaderNameBuilder;
        quint64 lastFrameTime = 0;
        float msSinceLastFrame = 0;
    };
    QExplicitlySharedDataPointer<Private> d;

    void releaseBuffer(qint32 inIdx);
    QDemonMaterialClass *getMaterialClass(const QString &inStr);
    const QDemonMaterialClass *getMaterialClass(const QString &inStr) const;

    QDemonRef<QDemonRenderShaderProgram> getShader(QDemonCustomMaterialRenderContext &inRenderContext,
                                                   const QDemonRenderCustomMaterial &inMaterial,
                                                   const dynamic::QDemonBindShader &inCommand,
                                                   const TShaderFeatureSet &inFeatureSet,
                                                   const dynamic::QDemonDynamicShaderProgramFlags &inFlags);

    QDemonMaterialOrComputeShader bindShader(QDemonCustomMaterialRenderContext &inRenderContext,
                                             const QDemonRenderCustomMaterial &inMaterial,
                                             const dynamic::QDemonBindShader &inCommand,
                                             const TShaderFeatureSet &inFeatureSet);

    void doApplyInstanceValue(QDemonRenderCustomMaterial & /* inMaterial */,
                              quint8 *inDataPtr,
                              const QString &inPropertyName,
                              QDemonRenderShaderDataType inPropertyType,
                              const QDemonRef<QDemonRenderShaderProgram> &inShader,
                              const dynamic::QDemonPropertyDefinition &inDefinition);

    void applyInstanceValue(QDemonRenderCustomMaterial &inMaterial,
                            QDemonMaterialClass &inClass,
                            const QDemonRef<QDemonRenderShaderProgram> &inShader,
                            const dynamic::QDemonApplyInstanceValue &inCommand);

    void applyBlending(const dynamic::QDemonApplyBlending &inCommand);

    // we currently only bind a source texture
    const QDemonRef<QDemonRenderTexture2D> applyBufferValue(const QDemonRenderCustomMaterial &inMaterial,
                                                            const QDemonRef<QDemonRenderShaderProgram> &inShader,
                                                            const dynamic::QDemonApplyBufferValue &inCommand,
                                                            const QDemonRef<QDemonRenderTexture2D> inSourceTexture);

    void allocateBuffer(const dynamic::QDemonAllocateBuffer &inCommand, QDemonRenderFrameBuffer &inTarget);

    QDemonRenderFrameBuffer bindBuffer(const QDemonRenderCustomMaterial &inMaterial,
                                                  const dynamic::QDemonBindBuffer &inCommand,
                                                  bool &outClearTarget,
                                                  QVector2D &outDestSize);
    void computeScreenCoverage(QDemonCustomMaterialRenderContext &inRenderContext, qint32 *xMin, qint32 *yMin, qint32 *xMax, qint32 *yMax);
    void blitFramebuffer(QDemonCustomMaterialRenderContext &inRenderContext,
                         const dynamic::QDemonApplyBlitFramebuffer &inCommand,
                         const QDemonRenderFrameBuffer &inTarget);
    QDemonLayerGlobalRenderProperties getLayerGlobalRenderProperties(QDemonCustomMaterialRenderContext &inRenderContext);
    void renderPass(QDemonCustomMaterialRenderContext &inRenderContext,
                    const QDemonRef<QDemonCustomMaterialShader> &inShader,
                    const QDemonRef<QDemonRenderTexture2D> & /* inSourceTexture */,
                    const QDemonRenderFrameBuffer &inFrameBuffer,
                    bool inRenderTargetNeedsClear,
                    const QDemonRef<QDemonRenderInputAssembler> &inAssembler,
                    quint32 inCount,
                    quint32 inOffset);
    void doRenderCustomMaterial(QDemonCustomMaterialRenderContext &inRenderContext,
                                const QDemonRenderCustomMaterial &inMaterial,
                                QDemonMaterialClass &inClass,
                                const QDemonRenderFrameBuffer &inTarget,
                                const TShaderFeatureSet &inFeatureSet);
    void prepareTextureForRender(QDemonMaterialClass &inClass, QDemonRenderCustomMaterial &inMaterial);
    void prepareDisplacementForRender(QDemonMaterialClass &inClass, QDemonRenderCustomMaterial &inMaterial);
    void prepareMaterialForRender(QDemonMaterialClass &inClass, QDemonRenderCustomMaterial &inMaterial);

    qint32 findBuffer(const QString &inName);
    qint32 findAllocatedImage(const QString &inName);
    bool textureNeedsMips(const dynamic::QDemonPropertyDefinition *inPropDec, QDemonRenderTexture2D *inTexture);
    void setTexture(const QDemonRef<QDemonRenderShaderProgram> &inShader, const QString &inPropName,
                    const QDemonRef<QDemonRenderTexture2D> &inTexture,
                    const dynamic::QDemonPropertyDefinition *inPropDec = nullptr,
                    bool needMips = false);
public:
    QDemonMaterialSystem() = default;
    QDemonMaterialSystem(QDemonRenderContextCoreInterface *ct);

    ~QDemonMaterialSystem();
    bool isMaterialRegistered(const QString &inStr);

    bool registerMaterialClass(const QString &inName,
                                       const QDemonConstDataRef<dynamic::QDemonPropertyDeclaration> &inProperties);

    QDemonConstDataRef<dynamic::QDemonPropertyDefinition> getCustomMaterialProperties(const QString &inCustomMaterialName) const;

    void setCustomMaterialRefraction(const QString &inName, bool inHasRefraction);
    void setCustomMaterialTransparency(const QString &inName, bool inHasTransparency);
    void setCustomMaterialAlwaysDirty(const QString &inName, bool inIsAlwaysDirty);
    void setCustomMaterialShaderKey(const QString &inName, quint32 inShaderKey);
    void setCustomMaterialLayerCount(const QString &inName, quint32 inLayerCount);
    // The custom material commands are the actual commands that run for a given material
    // effect.  The tell the system exactly
    // explicitly things like bind this shader, bind this render target, apply this property,
    // run this shader
    // See UICRenderEffectCommands.h for the list of commands.
    // These commands are copied into the effect.
    void setCustomMaterialCommands(QString inName, QDemonConstDataRef<dynamic::QDemonCommand *> inCommands);

    void setMaterialClassShader(QString inName,
                                        const char *inShaderType,
                                        const char *inShaderVersion,
                                        const char *inShaderData,
                                        bool inHasGeomShader,
                                        bool inIsComputeShader);

    QDemonRenderCustomMaterial *createCustomMaterial(const QString &inName);

    void setPropertyEnumNames(const QString &inName, const QString &inPropName, const QDemonConstDataRef<QString> &inNames);

    void setPropertyTextureSettings(const QString &inEffectName,
                                            const QString &inPropName,
                                            const QString &inPropPath,
                                            QDemonRenderTextureTypeValue inTexType,
                                            QDemonRenderTextureCoordOp inCoordOp,
                                            QDemonRenderTextureMagnifyingOp inMagFilterOp,
                                            QDemonRenderTextureMinifyingOp inMinFilterOp);

    void setRenderContextInterface(QDemonRenderContextInterface *inContext);

    // Returns true if the material is dirty and thus will produce a different render result
    // than previously.  This effects things like progressive AA.
    bool prepareForRender(const QDemonRenderModel &inModel,
                                  const QDemonRenderSubset &inSubset,
                                  QDemonRenderCustomMaterial &inMaterial,
                                  bool inClearDirty);

    bool renderDepthPrepass(const QMatrix4x4 &inMVP,
                                    const QDemonRenderCustomMaterial &inMaterial,
                                    const QDemonRenderSubset &inSubset);
    void renderSubset(QDemonCustomMaterialRenderContext &inRenderContext, const TShaderFeatureSet &inFeatureSet);
    void onMaterialActivationChange(const QDemonRenderCustomMaterial &inMaterial, bool inActive);

    // get shader name
    QString getShaderName(const QDemonRenderCustomMaterial &inMaterial);
    // apply property values
    void applyShaderPropertyValues(const QDemonRenderCustomMaterial &inMaterial,
                                           const QDemonRef<QDemonRenderShaderProgram> &inProgram);
    // Called by the uiccontext so this system can clear any per-frame render information.
    void endFrame();
};

struct Q_DEMONRUNTIMERENDER_EXPORT QDemonCustomMaterialVertexPipeline : public QDemonVertexPipelineImpl
{
    QDemonRenderContextInterface *m_context;
    TessModeValues m_tessMode;

    QDemonCustomMaterialVertexPipeline(QDemonRenderContextInterface *inContext, TessModeValues inTessMode);
    void initializeTessControlShader();
    void initializeTessEvaluationShader();
    void finalizeTessControlShader();
    void finalizeTessEvaluationShader();

    // Responsible for beginning all vertex and fragment generation (void main() { etc).
    virtual void beginVertexGeneration(quint32 displacementImageIdx, QDemonRenderableImage *displacementImage) override;
    // The fragment shader expects a floating point constant, object_opacity to be defined
    // post this method.
    virtual void beginFragmentGeneration() override;
    // Output variables may be mangled in some circumstances so the shader generation
    // system needs an abstraction mechanism around this.
    virtual void assignOutput(const QByteArray &inVarName, const QByteArray &inVarValue) override;
    virtual void generateEnvMapReflection() override {}
    virtual void generateViewVector() override {}
    virtual void generateUVCoords(quint32 inUVSet) override;
    virtual void generateWorldNormal() override;
    virtual void generateObjectNormal() override;
    virtual void generateVarTangentAndBinormal() override;
    virtual void generateWorldPosition() override;
    // responsible for closing all vertex and fragment generation
    virtual void endVertexGeneration() override;
    virtual void endFragmentGeneration() override;
    virtual QDemonShaderStageGeneratorInterface &activeStage() override;
    virtual void addInterpolationParameter(const QByteArray &inName, const QByteArray &inType) override;
    virtual void doGenerateUVCoords(quint32 inUVSet) override;
    virtual void doGenerateWorldNormal() override;
    virtual void doGenerateObjectNormal() override;
    virtual void doGenerateWorldPosition() override;
    virtual void doGenerateVarTangentAndBinormal() override;
    virtual void doGenerateVertexColor() override;
};
QT_END_NAMESPACE
#endif
