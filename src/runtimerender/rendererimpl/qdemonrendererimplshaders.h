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
#ifndef QDEMON_RENDERER_IMPL_SHADERS_H
#define QDEMON_RENDERER_IMPL_SHADERS_H

#include <QtDemonRender/qdemonrendershaderprogram.h>
#include <QtDemonRender/qdemonrenderprogrampipeline.h>
#include <QtDemonRuntimeRender/qdemonrenderableobjects.h>

QT_BEGIN_NAMESPACE

/**
     *	Cached tessellation property lookups this is on a per mesh base
     */
struct SShaderTessellationProperties
{
    QDemonRenderCachedShaderProperty<float> m_EdgeTessLevel; ///< tesselation value for the edges
    QDemonRenderCachedShaderProperty<float> m_InsideTessLevel; ///< tesselation value for the inside
    QDemonRenderCachedShaderProperty<float> m_PhongBlend; ///< blending between linear and phong component
    QDemonRenderCachedShaderProperty<QVector2D> m_DistanceRange; ///< distance range for min and max tess level
    QDemonRenderCachedShaderProperty<float> m_DisableCulling; ///< if set to 1.0 this disables
    ///backface culling optimization in
    ///the tess shader

    SShaderTessellationProperties() {}
    SShaderTessellationProperties(QSharedPointer<QDemonRenderShaderProgram> inShader)
        : m_EdgeTessLevel("tessLevelOuter", inShader)
        , m_InsideTessLevel("tessLevelInner", inShader)
        , m_PhongBlend("phongBlend", inShader)
        , m_DistanceRange("distanceRange", inShader)
        , m_DisableCulling("disableCulling", inShader)
    {
    }
};

/**
     *	The results of generating a shader.  Caches all possible variable names into
     *	typesafe objects.
     */
struct SShaderGeneratorGeneratedShader
{
    quint32 m_LayerSetIndex;
    QString m_QueryString;
    QSharedPointer<QDemonRenderShaderProgram> m_Shader;
    QDemonRenderCachedShaderProperty<QMatrix4x4> m_ViewportMatrix;
    SShaderTessellationProperties m_Tessellation;

    SShaderGeneratorGeneratedShader(QString inQueryString,
                                    QSharedPointer<QDemonRenderShaderProgram> inShader)
        : m_LayerSetIndex(std::numeric_limits<quint32>::max())
        , m_QueryString(inQueryString)
        , m_Shader(inShader)
        , m_ViewportMatrix("viewport_matrix", inShader)
        , m_Tessellation(inShader)
    {
    }
    ~SShaderGeneratorGeneratedShader() 
    { 
    }
    static quint32 GetLayerIndex(const SShaderGeneratorGeneratedShader &inShader)
    {
        return inShader.m_LayerSetIndex;
    }
    static void SetLayerIndex(SShaderGeneratorGeneratedShader &inShader, quint32 idx)
    {
        inShader.m_LayerSetIndex = idx;
    }
};

struct SDefaultMaterialRenderableDepthShader
{
    QSharedPointer<QDemonRenderShaderProgram> m_Shader;
    QDemonRenderCachedShaderProperty<QMatrix4x4> m_MVP;

    SDefaultMaterialRenderableDepthShader(QSharedPointer<QDemonRenderShaderProgram> inShader,
                                          QDemonRenderContext &inContext)
        : m_Shader(inShader)
        , m_MVP("model_view_projection", inShader)
    {
        Q_UNUSED(inContext)
    }

    ~SDefaultMaterialRenderableDepthShader() {
    }

};

/**
     *	Cached texture property lookups, used one per texture so a shader generator for N
     *	textures will have an array of N of these lookup objects.
     */
struct SShaderTextureProperties
{
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> m_Sampler;
    QDemonRenderCachedShaderProperty<QVector3D> m_Offsets;
    QDemonRenderCachedShaderProperty<QVector4D> m_Rotations;
    SShaderTextureProperties(const char *sampName, const char *offName, const char *rotName,
                             QSharedPointer<QDemonRenderShaderProgram> inShader)
        : m_Sampler(sampName, inShader)
        , m_Offsets(offName, inShader)
        , m_Rotations(rotName, inShader)
    {
    }
    SShaderTextureProperties() {}
};

struct SRenderableDepthPrepassShader
{
    QSharedPointer<QDemonRenderShaderProgram> m_Shader;
    QDemonRenderCachedShaderProperty<QMatrix4x4> m_MVP;
    QDemonRenderCachedShaderProperty<QMatrix4x4> m_GlobalTransform;
    QDemonRenderCachedShaderProperty<QMatrix4x4> m_Projection;
    QDemonRenderCachedShaderProperty<QVector3D> m_CameraPosition;
    QDemonRenderCachedShaderProperty<float> m_DisplaceAmount;
    SShaderTextureProperties m_DisplacementProps;
    QDemonRenderCachedShaderProperty<QVector2D> m_CameraProperties;
    QDemonRenderCachedShaderProperty<QVector3D> m_CameraDirection;
    // QDemonRenderCachedShaderProperty<QMatrix4x4>	m_ShadowMV[6];

    // Cache the tessellation property name lookups
    SShaderTessellationProperties m_Tessellation;

    SRenderableDepthPrepassShader(QSharedPointer<QDemonRenderShaderProgram> inShader, QSharedPointer<QDemonRenderContext> inContext)
        : m_Shader(inShader)
        , m_MVP("model_view_projection", inShader)
        , m_GlobalTransform("model_matrix", inShader)
        , m_Projection("projection", inShader)
        , m_CameraPosition("camera_position", inShader)
        , m_DisplaceAmount("displaceAmount", inShader)
        , m_DisplacementProps("displacementSampler", "displacementMap_offset",
                              "displacementMap_rot", inShader)
        , m_CameraProperties("camera_properties", inShader)
        , m_CameraDirection("camera_direction", inShader)
        , m_Tessellation(inShader)
    {
        Q_UNUSED(inContext)
        /*
            m_ShadowMV[0].m_Shader = &inShader;
            m_ShadowMV[0].m_Constant = inShader.GetShaderConstant( "shadow_mv0" );
            m_ShadowMV[1].m_Shader = &inShader;
            m_ShadowMV[1].m_Constant = inShader.GetShaderConstant( "shadow_mv1" );
            m_ShadowMV[2].m_Shader = &inShader;
            m_ShadowMV[2].m_Constant = inShader.GetShaderConstant( "shadow_mv2" );
            m_ShadowMV[3].m_Shader = &inShader;
            m_ShadowMV[3].m_Constant = inShader.GetShaderConstant( "shadow_mv3" );
            m_ShadowMV[4].m_Shader = &inShader;
            m_ShadowMV[4].m_Constant = inShader.GetShaderConstant( "shadow_mv4" );
            m_ShadowMV[5].m_Shader = &inShader;
            m_ShadowMV[5].m_Constant = inShader.GetShaderConstant( "shadow_mv5" );
            */
    }

    ~SRenderableDepthPrepassShader() 
    { 
    }

};

struct SDefaultAoPassShader
{
    QSharedPointer<QDemonRenderShaderProgram> m_Shader;
    QDemonRenderCachedShaderProperty<QMatrix4x4> m_ViewMatrix;
    QDemonRenderCachedShaderProperty<QVector2D> m_CameraProperties;
    QDemonRenderCachedShaderProperty<QVector3D> m_CameraDirection;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> m_DepthTexture;
    QDemonRenderCachedShaderProperty<QDemonRenderTextureCube *> m_CubeTexture;
    QDemonRenderCachedShaderProperty<QVector2D> m_DepthSamplerSize;

    QDemonRenderCachedShaderBuffer<QDemonRenderShaderConstantBuffer> m_AoShadowParams;

    SDefaultAoPassShader(QSharedPointer<QDemonRenderShaderProgram> inShader, QSharedPointer<QDemonRenderContext> inContext)
        : m_Shader(inShader)
        , m_ViewMatrix("view_matrix", inShader)
        , m_CameraProperties("camera_properties", inShader)
        , m_CameraDirection("camera_direction", inShader)
        , m_DepthTexture("depth_sampler", inShader)
        , m_CubeTexture("depth_cube", inShader)
        , m_DepthSamplerSize("depth_sampler_size", inShader)
        , m_AoShadowParams("cbAoShadow", inShader)
    {
        Q_UNUSED(inContext)
    }
    ~SDefaultAoPassShader() 
    {
    }
};

struct STextShader
{
    QSharedPointer<QDemonRenderShaderProgram> m_Shader;

    QSharedPointer<QDemonRenderProgramPipeline> m_ProgramPipeline;

    QDemonRenderCachedShaderProperty<QMatrix4x4> m_MVP;
    // Dimensions and offsetting of the image.
    QDemonRenderCachedShaderProperty<QVector4D> m_Dimensions;
    // The fourth member of text color is the opacity
    QDemonRenderCachedShaderProperty<QVector4D> m_TextColor;
    QDemonRenderCachedShaderProperty<QVector3D> m_BackgroundColor;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> m_Sampler;
    // Dimensions and offsetting of the texture
    QDemonRenderCachedShaderProperty<QVector3D> m_TextDimensions;
    QDemonRenderCachedShaderProperty<QVector2D> m_CameraProperties;
    // Used only for onscreen text
    QDemonRenderCachedShaderProperty<QVector2D> m_VertexOffsets;

    STextShader(QSharedPointer<QDemonRenderShaderProgram> shader, QSharedPointer<QDemonRenderProgramPipeline> pipeline = nullptr)
        : m_Shader(shader)
        , m_ProgramPipeline(pipeline)
        , m_MVP("model_view_projection", shader)
        , m_Dimensions("text_dimensions", shader)
        , m_TextColor("text_textcolor", shader)
        , m_BackgroundColor("text_backgroundcolor", shader)
        , m_Sampler("text_image", shader)
        , m_TextDimensions("text_textdimensions", shader)
        , m_CameraProperties("camera_properties", shader)
        , m_VertexOffsets("vertex_offsets", shader)
    {
        if (!pipeline) {
        }
    }
    ~STextShader()
    {
    }
    void Render(QSharedPointer<QDemonRenderTexture2D> inTexture,
                const STextScaleAndOffset &inScaleAndOffset,
                const QVector4D &inTextColor,
                const QMatrix4x4 &inMVP,
                const QVector2D &inCameraVec,
                QSharedPointer<QDemonRenderContext> inRenderContext,
                QSharedPointer<QDemonRenderInputAssembler> inInputAssemblerBuffer,
                quint32 count,
                const STextTextureDetails &inTextTextureDetails,
                const QVector3D &inBackgroundColor);

    void RenderPath(QSharedPointer<QDemonRenderPathFontItem> inPathFontItem,
                    QSharedPointer<QDemonRenderPathFontSpecification> inPathFontSpec,
                    const STextScaleAndOffset &inScaleAndOffset,
                    const QVector4D &inTextColor,
                    const QMatrix4x4 &inViewProjection,
                    const QMatrix4x4 &inModel,
                    const QVector2D &inCameraVec,
                    QSharedPointer<QDemonRenderContext> inRenderContext,
                    const STextTextureDetails &inTextTextureDetails,
                    const QVector3D &inBackgroundColor);

    void Render2D(QSharedPointer<QDemonRenderTexture2D> inTexture,
                  const QVector4D &inTextColor, const QMatrix4x4 &inMVP,
                  QSharedPointer<QDemonRenderContext> inRenderContext,
                  QSharedPointer<QDemonRenderInputAssembler> inInputAssemblerBuffer,
                  quint32 count,
                  QVector2D inVertexOffsets);
};

struct STextDepthShader
{
    QSharedPointer<QDemonRenderShaderProgram> m_Shader;
    QDemonRenderCachedShaderProperty<QMatrix4x4> m_MVP;
    // Dimensions and offsetting of the image.
    QDemonRenderCachedShaderProperty<QVector4D> m_Dimensions;
    QDemonRenderCachedShaderProperty<QVector3D> m_TextDimensions;
    QDemonRenderCachedShaderProperty<QVector2D> m_CameraProperties;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> m_Sampler;
    QSharedPointer<QDemonRenderInputAssembler> m_QuadInputAssembler;

    STextDepthShader(QSharedPointer<QDemonRenderShaderProgram> prog,
                     QSharedPointer<QDemonRenderInputAssembler> assembler)
        : m_Shader(prog)
        , m_MVP("model_view_projection", prog)
        , m_Dimensions("text_dimensions", prog)
        , m_TextDimensions("text_textdimensions", prog)
        , m_CameraProperties("camera_properties", prog)
        , m_Sampler("text_image", prog)
        , m_QuadInputAssembler(assembler)
    {
    }
    ~STextDepthShader() 
    {
    }
};

struct SLayerProgAABlendShader
{
    QSharedPointer<QDemonRenderShaderProgram> m_Shader;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> m_AccumSampler;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> m_LastFrame;
    QDemonRenderCachedShaderProperty<QVector2D> m_BlendFactors;
    SLayerProgAABlendShader(QSharedPointer<QDemonRenderShaderProgram> inShader)
        : m_Shader(inShader)
        , m_AccumSampler("accumulator", inShader)
        , m_LastFrame("last_frame", inShader)
        , m_BlendFactors("blend_factors", inShader)
    {
    }
};

struct SLayerSceneShader
{
    QSharedPointer<QDemonRenderShaderProgram> m_Shader;

    QDemonRenderCachedShaderProperty<QMatrix4x4> m_MVP;
    // Dimensions and offsetting of the image.
    QDemonRenderCachedShaderProperty<QVector2D> m_Dimensions;
    // The fourth member of text color is the opacity
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> m_Sampler;

    SLayerSceneShader(QSharedPointer<QDemonRenderShaderProgram> inShader)
        : m_Shader(inShader)
        , m_MVP("model_view_projection", inShader)
        , m_Dimensions("layer_dimensions", inShader)
        , m_Sampler("layer_image", inShader)
    {
    }
    ~SLayerSceneShader() 
    {
    }
};

struct SShadowmapPreblurShader
{
    QSharedPointer<QDemonRenderShaderProgram> m_Shader;
    QDemonRenderCachedShaderProperty<QVector2D> m_CameraProperties;
    QDemonRenderCachedShaderProperty<QDemonRenderTextureCube *> m_DepthCube;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> m_DepthMap;

    SShadowmapPreblurShader(QSharedPointer<QDemonRenderShaderProgram> inShader)
        : m_Shader(inShader)
        , m_CameraProperties("camera_properties", inShader)
        , m_DepthCube("depthCube", inShader)
        , m_DepthMap("depthSrc", inShader)
    {
    }
    ~SShadowmapPreblurShader() 
    {
    }
};

#ifdef ADVANCED_BLEND_SW_FALLBACK
struct SAdvancedModeBlendShader
{
    QSharedPointer<QDemonRenderShaderProgram> m_Shader;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> m_baseLayer;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> m_blendLayer;

    SAdvancedModeBlendShader(QSharedPointer<QDemonRenderShaderProgram> inShader)
        : m_Shader(inShader)
        , m_baseLayer("base_layer", inShader)
        , m_blendLayer("blend_layer", inShader)
    {
    }
    ~SAdvancedModeBlendShader() 
    { 
    }
};
#endif

struct SGGSGet
{
    quint32 operator()(const SShaderGeneratorGeneratedShader &inShader)
    {
        return inShader.m_LayerSetIndex;
    }
};
struct SGGSSet
{
    void operator()(SShaderGeneratorGeneratedShader &inShader, quint32 idx)
    {
        inShader.m_LayerSetIndex = idx;
    }
};
QT_END_NAMESPACE
#endif
