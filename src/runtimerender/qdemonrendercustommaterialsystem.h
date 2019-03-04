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

QT_BEGIN_NAMESPACE

namespace dynamic {
struct QDemonCommand;
}

struct QDemonCustomMaterialRenderContext;
struct QDemonRenderCustomMaterial;
class QDemonCustomMaterialSystemInterface;
struct QDemonRenderSubset;

// How to handle blend modes?
struct QDemonRenderModel;
class Q_DEMONRUNTIMERENDER_EXPORT QDemonCustomMaterialSystemInterface
{
public:
    QAtomicInt ref;
    virtual ~QDemonCustomMaterialSystemInterface() {}
    virtual bool isMaterialRegistered(QString inStr) = 0;

    virtual bool registerMaterialClass(QString inName, QDemonConstDataRef<dynamic::QDemonPropertyDeclaration> inProperties) = 0;

    virtual QDemonConstDataRef<dynamic::QDemonPropertyDefinition> getCustomMaterialProperties(QString inCustomMaterialName) const = 0;

    virtual void setCustomMaterialRefraction(QString inName, bool inHasRefraction) = 0;
    virtual void setCustomMaterialTransparency(QString inName, bool inHasTransparency) = 0;
    virtual void setCustomMaterialAlwaysDirty(QString inName, bool inIsAlwaysDirty) = 0;
    virtual void setCustomMaterialShaderKey(QString inName, quint32 inShaderKey) = 0;
    virtual void setCustomMaterialLayerCount(QString inName, quint32 inLayerCount) = 0;
    // The custom material commands are the actual commands that run for a given material
    // effect.  The tell the system exactly
    // explicitly things like bind this shader, bind this render target, apply this property,
    // run this shader
    // See UICRenderEffectCommands.h for the list of commands.
    // These commands are copied into the effect.
    virtual void setCustomMaterialCommands(QString inName, QDemonConstDataRef<dynamic::QDemonCommand *> inCommands) = 0;

    virtual void setMaterialClassShader(QString inName,
                                        const char *inShaderType,
                                        const char *inShaderVersion,
                                        const char *inShaderData,
                                        bool inHasGeomShader,
                                        bool inIsComputeShader) = 0;

    virtual QDemonRenderCustomMaterial *createCustomMaterial(QString inName) = 0;

    virtual void setPropertyEnumNames(QString inName, QString inPropName, QDemonConstDataRef<QString> inNames) = 0;

    virtual void setPropertyTextureSettings(QString inEffectName,
                                            QString inPropName,
                                            QString inPropPath,
                                            QDemonRenderTextureTypeValue::Enum inTexType,
                                            QDemonRenderTextureCoordOp::Enum inCoordOp,
                                            QDemonRenderTextureMagnifyingOp::Enum inMagFilterOp,
                                            QDemonRenderTextureMinifyingOp::Enum inMinFilterOp) = 0;

    //    virtual void Save(SWriteBuffer &ioBuffer,
    //                      const SStrRemapMap &inRemapMap,
    //                      const char *inProjectDir) const = 0;
    //    virtual void Load(QDemonDataRef<quint8> inData, CStrTableOrDataRef inStrDataBlock,
    //                      const char *inProjectDir) = 0;

    virtual QDemonRef<QDemonCustomMaterialSystemInterface> getCustomMaterialSystem(QDemonRenderContextInterface *inContext) = 0;

    static QDemonRef<QDemonCustomMaterialSystemInterface> createCustomMaterialSystem(QDemonRenderContextCoreInterface * inContext);

    // Returns true if the material is dirty and thus will produce a different render result
    // than previously.  This effects things like progressive AA.
    virtual bool prepareForRender(const QDemonRenderModel &inModel,
                                  const QDemonRenderSubset &inSubset,
                                  QDemonRenderCustomMaterial &inMaterial,
                                  bool inClearDirty) = 0;

    virtual bool renderDepthPrepass(const QMatrix4x4 &inMVP, const QDemonRenderCustomMaterial &inMaterial, const QDemonRenderSubset &inSubset) = 0;
    virtual void renderSubset(QDemonCustomMaterialRenderContext &inRenderContext, TShaderFeatureSet inFeatureSet) = 0;
    virtual void onMaterialActivationChange(const QDemonRenderCustomMaterial &inMaterial, bool inActive) = 0;

    // get shader name
    virtual QString getShaderName(const QDemonRenderCustomMaterial &inMaterial) = 0;
    // apply property values
    virtual void applyShaderPropertyValues(const QDemonRenderCustomMaterial &inMaterial,
                                           QDemonRef<QDemonRenderShaderProgram> inProgram) = 0;
    // Called by the uiccontext so this system can clear any per-frame render information.
    virtual void endFrame() = 0;
};

struct Q_DEMONRUNTIMERENDER_EXPORT QDemonCustomMaterialVertexPipeline : public QDemonVertexPipelineImpl
{
    QDemonRenderContextInterface *m_context;
    TessModeValues::Enum m_tessMode;

    QDemonCustomMaterialVertexPipeline(QDemonRenderContextInterface *inContext, TessModeValues::Enum inTessMode);
    void initializeTessControlShader();
    void initializeTessEvaluationShader();
    void finalizeTessControlShader();
    void finalizeTessEvaluationShader();

    // Responsible for beginning all vertex and fragment generation (void main() { etc).
    virtual void beginVertexGeneration(quint32 displacementImageIdx,
                                       QDemonRenderableImage *displacementImage) override;
    // The fragment shader expects a floating point constant, object_opacity to be defined
    // post this method.
    virtual void beginFragmentGeneration() override;
    // Output variables may be mangled in some circumstances so the shader generation
    // system needs an abstraction mechanism around this.
    virtual void assignOutput(const QString &inVarName, const QString &inVarValue) override;
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
    virtual void addInterpolationParameter(const QString &inName, const QString &inType) override;
    virtual void doGenerateUVCoords(quint32 inUVSet) override;
    virtual void doGenerateWorldNormal() override;
    virtual void doGenerateObjectNormal() override;
    virtual void doGenerateWorldPosition() override;
    virtual void doGenerateVarTangentAndBinormal() override;
    virtual void doGenerateVertexColor() override;
};
QT_END_NAMESPACE
#endif
