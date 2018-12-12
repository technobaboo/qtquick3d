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
#ifndef QDEMON_RENDER_DEFAULT_MATERIAL_SHADER_GENERATOR_H
#define QDEMON_RENDER_DEFAULT_MATERIAL_SHADER_GENERATOR_H
#include <QtDemonRuntimeRender/qdemonrendermaterialshadergenerator.h>
#include <QtDemonRuntimeRender/qdemonrenderlightconstantproperties.h>

QT_BEGIN_NAMESPACE

class QDemonRenderShadowMap;
struct SShaderGeneratorGeneratedShader;

class IDefaultMaterialVertexPipeline : public IShaderStageGenerator
{
protected:
    virtual ~IDefaultMaterialVertexPipeline() {}
public:
    // Responsible for beginning all vertex and fragment generation (void main() { etc).
    virtual void BeginVertexGeneration(quint32 displacementImageIdx,
                                       SRenderableImage *displacementImage) = 0;
    // The fragment shader expects a floating point constant, object_opacity to be defined
    // post this method.
    virtual void BeginFragmentGeneration() = 0;
    // Output variables may be mangled in some circumstances so the shader generation system
    // needs an abstraction
    // mechanism around this.
    virtual void AssignOutput(const char *inVarName, const char *inVarValueExpr) = 0;

    /**
         * @brief Generates UV coordinates in shader code
         *
         * @param[in] inUVSet		index of UV data set
         *
         * @return no return
         */
    virtual void GenerateUVCoords(quint32 inUVSet = 0) = 0;

    virtual void GenerateEnvMapReflection() = 0;
    virtual void GenerateViewVector() = 0;

    // fragment shader expects varying vertex normal
    // lighting in vertex pipeline expects world_normal
    virtual void GenerateWorldNormal() = 0; // world_normal in both vert and frag shader
    virtual void GenerateObjectNormal() = 0; // object_normal in both vert and frag shader
    virtual void
    GenerateWorldPosition() = 0; // model_world_position in both vert and frag shader
    virtual void GenerateVarTangentAndBinormal() = 0;
    virtual void GenerateVertexColor() = 0;

    virtual bool HasActiveWireframe() = 0; // varEdgeDistance is a valid entity

    // responsible for closing all vertex and fragment generation
    virtual void EndVertexGeneration() = 0;
    virtual void EndFragmentGeneration() = 0;
};

class IDefaultMaterialShaderGenerator : public IMaterialShaderGenerator
{
public:
    virtual void AddDisplacementImageUniforms(IShaderStageGenerator &inGenerator,
                                              quint32 displacementImageIdx,
                                              SRenderableImage *displacementImage) = 0;
    SImageVariableNames GetImageVariableNames(quint32 inIdx) override = 0;
    void GenerateImageUVCoordinates(IShaderStageGenerator &inVertexPipeline, quint32 idx,
                                    quint32 uvSet, SRenderableImage &image) override = 0;
    // Transforms attr_pos, attr_norm, and attr_uv0.
    virtual void AddDisplacementMappingForDepthPass(IShaderStageGenerator &inShader) = 0;

    // inPipelineName needs to be unique else the shader cache will just return shaders from
    // different pipelines.
    QDemonRenderShaderProgram *GenerateShader(
            const SGraphObject &inMaterial, SShaderDefaultMaterialKey inShaderDescription,
            IShaderStageGenerator &inVertexPipeline, TShaderFeatureSet inFeatureSet,
            QDemonDataRef<SLight *> inLights, SRenderableImage *inFirstImage, bool inHasTransparency,
            const char *inVertexPipelineName, const char *inCustomMaterialName = "") override = 0;

    // Also sets the blend function on the render context.
    virtual void
    SetMaterialProperties(QDemonRenderShaderProgram &inProgram, const SGraphObject &inMaterial,
                          const QVector2D &inCameraVec, const QMatrix4x4 &inModelViewProjection,
                          const QMatrix3x3 &inNormalMatrix, const QMatrix4x4 &inGlobalTransform,
                          SRenderableImage *inFirstImage, float inOpacity,
                          SLayerGlobalRenderProperties inRenderProperties) override = 0;

    static IDefaultMaterialShaderGenerator &
    CreateDefaultMaterialShaderGenerator(IQDemonRenderContext &inRenderContext);

    SLightConstantProperties<SShaderGeneratorGeneratedShader>
    *GetLightConstantProperties(SShaderGeneratorGeneratedShader &shader);
};
QT_END_NAMESPACE
#endif
