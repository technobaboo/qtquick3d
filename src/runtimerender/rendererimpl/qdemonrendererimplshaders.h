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
struct QDemonShaderTessellationProperties
{
    QDemonRenderCachedShaderProperty<float> edgeTessLevel; ///< tesselation value for the edges
    QDemonRenderCachedShaderProperty<float> insideTessLevel; ///< tesselation value for the inside
    QDemonRenderCachedShaderProperty<float> phongBlend; ///< blending between linear and phong component
    QDemonRenderCachedShaderProperty<QVector2D> distanceRange; ///< distance range for min and max tess level
    QDemonRenderCachedShaderProperty<float> disableCulling; ///< if set to 1.0 this disables
    ///backface culling optimization in
    ///the tess shader

    QDemonShaderTessellationProperties() = default;
    QDemonShaderTessellationProperties(QDemonRef<QDemonRenderShaderProgram> inShader)
        : edgeTessLevel("tessLevelOuter", inShader)
        , insideTessLevel("tessLevelInner", inShader)
        , phongBlend("phongBlend", inShader)
        , distanceRange("distanceRange", inShader)
        , disableCulling("disableCulling", inShader)
    {
    }
};

/**
     *	The results of generating a shader.  Caches all possible variable names into
     *	typesafe objects.
     */
struct QDemonShaderGeneratorGeneratedShader
{
    quint32 layerSetIndex;
    QString queryString;
    QDemonRef<QDemonRenderShaderProgram> shader;
    QDemonRenderCachedShaderProperty<QMatrix4x4> viewportMatrix;
    QDemonShaderTessellationProperties tessellation;

    QDemonShaderGeneratorGeneratedShader(QString inQueryString,
                                         QDemonRef<QDemonRenderShaderProgram> inShader)
        : layerSetIndex(std::numeric_limits<quint32>::max())
        , queryString(inQueryString)
        , shader(inShader)
        , viewportMatrix("viewport_matrix", inShader)
        , tessellation(inShader)
    {
    }

    static quint32 getLayerIndex(const QDemonShaderGeneratorGeneratedShader &inShader)
    {
        return inShader.layerSetIndex;
    }
    static void setLayerIndex(QDemonShaderGeneratorGeneratedShader &inShader, quint32 idx)
    {
        inShader.layerSetIndex = idx;
    }
};

struct QDemonDefaultMaterialRenderableDepthShader
{
    QDemonRef<QDemonRenderShaderProgram> shader;
    QDemonRenderCachedShaderProperty<QMatrix4x4> mvp;

    QDemonDefaultMaterialRenderableDepthShader(QDemonRef<QDemonRenderShaderProgram> inShader,
                                               QDemonRenderContext &inContext)
        : shader(inShader)
        , mvp("model_view_projection", inShader)
    {
        // TODO:
        Q_UNUSED(inContext)
    }
};

/**
     *	Cached texture property lookups, used one per texture so a shader generator for N
     *	textures will have an array of N of these lookup objects.
     */
struct QDemonShaderTextureProperties
{
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> sampler;
    QDemonRenderCachedShaderProperty<QVector3D> offsets;
    QDemonRenderCachedShaderProperty<QVector4D> rotations;
    QDemonShaderTextureProperties(const char *sampName,
                                  const char *offName,
                                  const char *rotName,
                                  QDemonRef<QDemonRenderShaderProgram> inShader)
        : sampler(sampName, inShader)
        , offsets(offName, inShader)
        , rotations(rotName, inShader)
    {
    }
    QDemonShaderTextureProperties() = default;
};

struct QDemonRenderableDepthPrepassShader
{
    QDemonRef<QDemonRenderShaderProgram> shader;
    QDemonRenderCachedShaderProperty<QMatrix4x4> mvp;
    QDemonRenderCachedShaderProperty<QMatrix4x4> globalTransform;
    QDemonRenderCachedShaderProperty<QMatrix4x4> projection;
    QDemonRenderCachedShaderProperty<QVector3D> cameraPosition;
    QDemonRenderCachedShaderProperty<float> displaceAmount;
    QDemonShaderTextureProperties displacementProps;
    QDemonRenderCachedShaderProperty<QVector2D> cameraProperties;
    QDemonRenderCachedShaderProperty<QVector3D> cameraDirection;
    // QDemonRenderCachedShaderProperty<QMatrix4x4> shadowMv[6];

    // Cache the tessellation property name lookups
    QDemonShaderTessellationProperties tessellation;

    QDemonRenderableDepthPrepassShader(QDemonRef<QDemonRenderShaderProgram> inShader,
                                       QDemonRef<QDemonRenderContext> inContext)
        : shader(inShader)
        , mvp("model_view_projection", inShader)
        , globalTransform("model_matrix", inShader)
        , projection("projection", inShader)
        , cameraPosition("camera_position", inShader)
        , displaceAmount("displaceAmount", inShader)
        , displacementProps("displacementSampler", "displacementMap_offset",
                              "displacementMap_rot", inShader)
        , cameraProperties("camera_properties", inShader)
        , cameraDirection("camera_direction", inShader)
        , tessellation(inShader)
    {
        Q_UNUSED(inContext)
        /*
            shadowMv[0].m_Shader = &inShader;
            shadowMv[0].m_Constant = inShader.GetShaderConstant( "shadow_mv0" );
            shadowMv[1].m_Shader = &inShader;
            shadowMv[1].m_Constant = inShader.GetShaderConstant( "shadow_mv1" );
            shadowMv[2].m_Shader = &inShader;
            shadowMv[2].m_Constant = inShader.GetShaderConstant( "shadow_mv2" );
            shadowMv[3].m_Shader = &inShader;
            shadowMv[3].m_Constant = inShader.GetShaderConstant( "shadow_mv3" );
            shadowMv[4].m_Shader = &inShader;
            shadowMv[4].m_Constant = inShader.GetShaderConstant( "shadow_mv4" );
            shadowMv[5].m_Shader = &inShader;
            shadowMv[5].m_Constant = inShader.GetShaderConstant( "shadow_mv5" );
            */
    }

    ~QDemonRenderableDepthPrepassShader()
    { 
    }

};

struct QDemonDefaultAoPassShader
{
    QDemonRef<QDemonRenderShaderProgram> shader;
    QDemonRenderCachedShaderProperty<QMatrix4x4> viewMatrix;
    QDemonRenderCachedShaderProperty<QVector2D> cameraProperties;
    QDemonRenderCachedShaderProperty<QVector3D> cameraDirection;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> depthTexture;
    QDemonRenderCachedShaderProperty<QDemonRenderTextureCube *> cubeTexture;
    QDemonRenderCachedShaderProperty<QVector2D> depthSamplerSize;

    QDemonRenderCachedShaderBuffer<QDemonRenderShaderConstantBuffer> aoShadowParams;

    QDemonDefaultAoPassShader(QDemonRef<QDemonRenderShaderProgram> inShader,
                              QDemonRef<QDemonRenderContext> inContext)
        : shader(inShader)
        , viewMatrix("view_matrix", inShader)
        , cameraProperties("camera_properties", inShader)
        , cameraDirection("camera_direction", inShader)
        , depthTexture("depth_sampler", inShader)
        , cubeTexture("depth_cube", inShader)
        , depthSamplerSize("depth_sampler_size", inShader)
        , aoShadowParams("cbAoShadow", inShader)
    {
        Q_UNUSED(inContext)
    }
    ~QDemonDefaultAoPassShader()
    {
    }
};

struct QDemonTextShader
{
    QDemonRef<QDemonRenderShaderProgram> shader;
    QDemonRef<QDemonRenderProgramPipeline> programPipeline;
    QDemonRenderCachedShaderProperty<QMatrix4x4> mvp;
    // Dimensions and offsetting of the image.
    QDemonRenderCachedShaderProperty<QVector4D> dimensions;
    // The fourth member of text color is the opacity
    QDemonRenderCachedShaderProperty<QVector4D> textColor;
    QDemonRenderCachedShaderProperty<QVector3D> backgroundColor;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> sampler;
    // Dimensions and offsetting of the texture
    QDemonRenderCachedShaderProperty<QVector3D> textDimensions;
    QDemonRenderCachedShaderProperty<QVector2D> cameraProperties;
    // Used only for onscreen text
    QDemonRenderCachedShaderProperty<QVector2D> vertexOffsets;

    QDemonTextShader(QDemonRef<QDemonRenderShaderProgram> inShader, QDemonRef<QDemonRenderProgramPipeline> pipeline = nullptr)
        : shader(inShader)
        , programPipeline(pipeline)
        , mvp("model_view_projection", inShader)
        , dimensions("text_dimensions", inShader)
        , textColor("text_textcolor", inShader)
        , backgroundColor("text_backgroundcolor", inShader)
        , sampler("text_image", inShader)
        , textDimensions("text_textdimensions", inShader)
        , cameraProperties("camera_properties", inShader)
        , vertexOffsets("vertex_offsets", inShader)
    {
        if (!pipeline) {
            // TODO: ??
        }
    }
    ~QDemonTextShader()
    {
    }
    void render(QDemonRef<QDemonRenderTexture2D> inTexture,
                const QDemonTextScaleAndOffset &inScaleAndOffset,
                const QVector4D &inTextColor,
                const QMatrix4x4 &inMVP,
                const QVector2D &inCameraVec,
                QDemonRef<QDemonRenderContext> inRenderContext,
                QDemonRef<QDemonRenderInputAssembler> inInputAssemblerBuffer,
                quint32 count,
                const QDemonTextTextureDetails &inTextTextureDetails,
                const QVector3D &inBackgroundColor);

    void renderPath(QDemonRef<QDemonRenderPathFontItem> inPathFontItem,
                    QDemonRef<QDemonRenderPathFontSpecification> inPathFontSpec,
                    const QDemonTextScaleAndOffset &inScaleAndOffset,
                    const QVector4D &inTextColor,
                    const QMatrix4x4 &inViewProjection,
                    const QMatrix4x4 &inModel,
                    const QVector2D &inCameraVec,
                    QDemonRef<QDemonRenderContext> inRenderContext,
                    const QDemonTextTextureDetails &inTextTextureDetails,
                    const QVector3D &inBackgroundColor);

    void render2D(QDemonRef<QDemonRenderTexture2D> inTexture,
                  const QVector4D &inTextColor, const QMatrix4x4 &inMVP,
                  QDemonRef<QDemonRenderContext> inRenderContext,
                  QDemonRef<QDemonRenderInputAssembler> inInputAssemblerBuffer,
                  quint32 count,
                  QVector2D inVertexOffsets);
};

struct QDemonTextDepthShader
{
    QDemonRef<QDemonRenderShaderProgram> shader;
    QDemonRenderCachedShaderProperty<QMatrix4x4> mvp;
    // Dimensions and offsetting of the image.
    QDemonRenderCachedShaderProperty<QVector4D> dimensions;
    QDemonRenderCachedShaderProperty<QVector3D> textDimensions;
    QDemonRenderCachedShaderProperty<QVector2D> cameraProperties;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> sampler;
    QDemonRef<QDemonRenderInputAssembler> quadInputAssembler;

    QDemonTextDepthShader(QDemonRef<QDemonRenderShaderProgram> prog,
                          QDemonRef<QDemonRenderInputAssembler> assembler)
        : shader(prog)
        , mvp("model_view_projection", prog)
        , dimensions("text_dimensions", prog)
        , textDimensions("text_textdimensions", prog)
        , cameraProperties("camera_properties", prog)
        , sampler("text_image", prog)
        , quadInputAssembler(assembler)
    {
    }
    ~QDemonTextDepthShader()
    {
    }
};

struct QDemonLayerProgAABlendShader
{
    QDemonRef<QDemonRenderShaderProgram> shader;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> accumSampler;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> lastFrame;
    QDemonRenderCachedShaderProperty<QVector2D> blendFactors;
    QDemonLayerProgAABlendShader(QDemonRef<QDemonRenderShaderProgram> inShader)
        : shader(inShader)
        , accumSampler("accumulator", inShader)
        , lastFrame("last_frame", inShader)
        , blendFactors("blend_factors", inShader)
    {
    }
};

struct QDemonLayerSceneShader
{
    QDemonRef<QDemonRenderShaderProgram> shader;

    QDemonRenderCachedShaderProperty<QMatrix4x4> mvp;
    // Dimensions and offsetting of the image.
    QDemonRenderCachedShaderProperty<QVector2D> dimensions;
    // The fourth member of text color is the opacity
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> sampler;

    QDemonLayerSceneShader(QDemonRef<QDemonRenderShaderProgram> inShader)
        : shader(inShader)
        , mvp("model_view_projection", inShader)
        , dimensions("layer_dimensions", inShader)
        , sampler("layer_image", inShader)
    {
    }
    ~QDemonLayerSceneShader()
    {
    }
};

struct QDemonShadowmapPreblurShader
{
    QDemonRef<QDemonRenderShaderProgram> shader;
    QDemonRenderCachedShaderProperty<QVector2D> cameraProperties;
    QDemonRenderCachedShaderProperty<QDemonRenderTextureCube *> depthCube;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> depthMap;

    QDemonShadowmapPreblurShader(QDemonRef<QDemonRenderShaderProgram> inShader)
        : shader(inShader)
        , cameraProperties("camera_properties", inShader)
        , depthCube("depthCube", inShader)
        , depthMap("depthSrc", inShader)
    {
    }
    ~QDemonShadowmapPreblurShader()
    {
    }
};

#ifdef ADVANCED_BLEND_SW_FALLBACK
struct QDemonAdvancedModeBlendShader
{
    QDemonRef<QDemonRenderShaderProgram> shader;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> baseLayer;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> blendLayer;

    QDemonAdvancedModeBlendShader(QDemonRef<QDemonRenderShaderProgram> inShader)
        : shader(inShader)
        , baseLayer("base_layer", inShader)
        , blendLayer("blend_layer", inShader)
    {
    }
    ~QDemonAdvancedModeBlendShader()
    { 
    }
};
#endif

struct QDemonGGSGet
{
    quint32 operator()(const QDemonShaderGeneratorGeneratedShader &inShader)
    {
        return inShader.layerSetIndex;
    }
};
struct QDemonGGSSet
{
    void operator()(QDemonShaderGeneratorGeneratedShader &inShader, quint32 idx)
    {
        inShader.layerSetIndex = idx;
    }
};
QT_END_NAMESPACE
#endif
