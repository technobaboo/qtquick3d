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
#ifndef QDEMON_RENDER_CUSTOM_MATERIAL_SYSTEM_H
#define QDEMON_RENDER_CUSTOM_MATERIAL_SYSTEM_H
#include <Qt3DSRender.h>
#include <QDemonRefCounted>
#include <Qt3DSRenderDynamicObjectSystem.h>
#include <Qt3DSVertexPipelineImpl.h>

namespace qt3ds {
namespace render {

    namespace dynamic {
        struct SCommand; // UICRenderEffectCommands.h
    }

    struct SCustomMaterialRenderContext;

    class ICustomMaterialSystemCore : public QDemonRefCounted
    {
    public:
        virtual bool IsMaterialRegistered(CRegisteredString inStr) = 0;

        virtual bool
        RegisterMaterialClass(CRegisteredString inName,
                              QDemonConstDataRef<dynamic::SPropertyDeclaration> inProperties) = 0;

        virtual QDemonConstDataRef<dynamic::SPropertyDefinition>
        GetCustomMaterialProperties(CRegisteredString inCustomMaterialName) const = 0;

        virtual void SetCustomMaterialRefraction(CRegisteredString inName,
                                                 bool inHasRefraction) = 0;
        virtual void SetCustomMaterialTransparency(CRegisteredString inName,
                                                   bool inHasTransparency) = 0;
        virtual void SetCustomMaterialAlwaysDirty(CRegisteredString inName,
                                                  bool inIsAlwaysDirty) = 0;
        virtual void SetCustomMaterialShaderKey(CRegisteredString inName, quint32 inShaderKey) = 0;
        virtual void SetCustomMaterialLayerCount(CRegisteredString inName, quint32 inLayerCount) = 0;
        // The custom material commands are the actual commands that run for a given material
        // effect.  The tell the system exactly
        // explicitly things like bind this shader, bind this render target, apply this property,
        // run this shader
        // See UICRenderEffectCommands.h for the list of commands.
        // These commands are copied into the effect.
        virtual void SetCustomMaterialCommands(CRegisteredString inName,
                                               QDemonConstDataRef<dynamic::SCommand *> inCommands) = 0;

        virtual void SetMaterialClassShader(CRegisteredString inName, const char8_t *inShaderType,
                                            const char8_t *inShaderVersion,
                                            const char8_t *inShaderData, bool inHasGeomShader,
                                            bool inIsComputeShader) = 0;

        virtual SCustomMaterial *
        CreateCustomMaterial(CRegisteredString inName,
                             NVAllocatorCallback &inSceneGraphAllocator) = 0;

        virtual void SetPropertyEnumNames(CRegisteredString inName, CRegisteredString inPropName,
                                          QDemonConstDataRef<CRegisteredString> inNames) = 0;

        virtual void SetPropertyTextureSettings(CRegisteredString inEffectName,
                                                CRegisteredString inPropName,
                                                CRegisteredString inPropPath,
                                                QDemonRenderTextureTypeValue::Enum inTexType,
                                                QDemonRenderTextureCoordOp::Enum inCoordOp,
                                                QDemonRenderTextureMagnifyingOp::Enum inMagFilterOp,
                                                QDemonRenderTextureMinifyingOp::Enum inMinFilterOp) = 0;

        virtual void Save(SWriteBuffer &ioBuffer,
                          const SStrRemapMap &inRemapMap,
                          const char8_t *inProjectDir) const = 0;
        virtual void Load(QDemonDataRef<quint8> inData, CStrTableOrDataRef inStrDataBlock,
                          const char8_t *inProjectDir) = 0;

        virtual ICustomMaterialSystem &GetCustomMaterialSystem(IQt3DSRenderContext &inContext) = 0;

        static ICustomMaterialSystemCore &
        CreateCustomMaterialSystemCore(IQt3DSRenderContextCore &inContext);
    };
    // How to handle blend modes?
    class ICustomMaterialSystem : public ICustomMaterialSystemCore
    {
    public:
        // Returns true if the material is dirty and thus will produce a different render result
        // than previously.  This effects things like progressive AA.
        virtual bool PrepareForRender(const SModel &inModel, const SRenderSubset &inSubset,
                                      SCustomMaterial &inMaterial, bool inClearDirty) = 0;

        virtual bool RenderDepthPrepass(const QMatrix4x4 &inMVP, const SCustomMaterial &inMaterial,
                                        const SRenderSubset &inSubset) = 0;
        virtual void RenderSubset(SCustomMaterialRenderContext &inRenderContext,
                                  TShaderFeatureSet inFeatureSet) = 0;
        virtual void OnMaterialActivationChange(const SCustomMaterial &inMaterial,
                                                bool inActive) = 0;

        // get shader name
        virtual const char *GetShaderName(const SCustomMaterial &inMaterial) = 0;
        // apply property values
        virtual void ApplyShaderPropertyValues(const SCustomMaterial &inMaterial,
                                               QDemonRenderShaderProgram &inProgram) = 0;
        // Called by the uiccontext so this system can clear any per-frame render information.
        virtual void EndFrame() = 0;
    };

    struct QDEMON_AUTOTEST_EXPORT SCustomMaterialVertexPipeline : public SVertexPipelineImpl
    {
        IQt3DSRenderContext *m_Context;
        TessModeValues::Enum m_TessMode;

        SCustomMaterialVertexPipeline(IQt3DSRenderContext *inContext, TessModeValues::Enum inTessMode);
        void InitializeTessControlShader();
        void InitializeTessEvaluationShader();
        void FinalizeTessControlShader();
        void FinalizeTessEvaluationShader();

        // Responsible for beginning all vertex and fragment generation (void main() { etc).
        virtual void BeginVertexGeneration(quint32 displacementImageIdx,
                                           SRenderableImage *displacementImage) override;
        // The fragment shader expects a floating point constant, object_opacity to be defined
        // post this method.
        virtual void BeginFragmentGeneration() override;
        // Output variables may be mangled in some circumstances so the shader generation
        // system needs an abstraction mechanism around this.
        virtual void AssignOutput(const char8_t *inVarName, const char8_t *inVarValue) override;
        virtual void GenerateEnvMapReflection() override {}
        virtual void GenerateViewVector() override {}
        virtual void GenerateUVCoords(quint32 inUVSet) override;
        virtual void GenerateWorldNormal() override;
        virtual void GenerateObjectNormal() override;
        virtual void GenerateVarTangentAndBinormal() override;
        virtual void GenerateWorldPosition() override;
        // responsible for closing all vertex and fragment generation
        virtual void EndVertexGeneration() override;
        virtual void EndFragmentGeneration() override;
        virtual IShaderStageGenerator &ActiveStage() override;
        virtual void AddInterpolationParameter(const char8_t *inName, const char8_t *inType) override;
        virtual void DoGenerateUVCoords(quint32 inUVSet) override;
        virtual void DoGenerateWorldNormal() override;
        virtual void DoGenerateObjectNormal() override;
        virtual void DoGenerateWorldPosition() override;
        virtual void DoGenerateVarTangentAndBinormal() override;
        virtual void DoGenerateVertexColor() override;
    };
}
}
#endif
