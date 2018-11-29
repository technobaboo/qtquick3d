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
#ifndef QDEMON_RENDER_MATERIAL_SHADER_GENERATOR_H
#define QDEMON_RENDER_MATERIAL_SHADER_GENERATOR_H
#include <Qt3DSRender.h>
#include <QDemonRefCounted>
#include <Qt3DSRenderShaderKeys.h>
#include <Qt3DSRenderShaderCache.h>
#include <Qt3DSRenderShaderCodeGeneratorV2.h>

namespace qt3ds {
namespace render {

// these are our current shader limits
#define QDEMON_MAX_NUM_LIGHTS 16
#define QDEMON_MAX_NUM_SHADOWS 8

    // note this struct must exactly match the memory layout of the
    // struct sampleLight.glsllib and sampleArea.glsllib. If you make changes here you need
    // to adjust the code in sampleLight.glsllib and sampleArea.glsllib as well
    struct SLightSourceShader
    {
        QVector4D m_position;
        QVector4D m_direction; // Specifies the light direction in world coordinates.
        QVector4D m_up;
        QVector4D m_right;
        QVector4D m_diffuse;
        QVector4D m_ambient;
        QVector4D m_specular;
        float m_spotExponent; // Specifies the intensity distribution of the light.
        float m_spotCutoff; // Specifies the maximum spread angle of the light.
        float m_constantAttenuation; // Specifies the constant light attenuation factor.
        float m_linearAttenuation; // Specifies the linear light attenuation factor.
        float m_quadraticAttenuation; // Specifies the quadratic light attenuation factor.
        float m_range; // Specifies the maximum distance of the light influence
        float m_width; // Specifies the width of the area light surface.
        float m_height; // Specifies the height of the area light surface;
        QVector4D m_shadowControls;
        QMatrix4x4 m_shadowView;
        qint32 m_shadowIdx;
        float m_padding1[3];
    };

    struct SLayerGlobalRenderProperties
    {
        const SLayer &m_Layer;
        SCamera &m_Camera;
        QVector3D m_CameraDirection;
        QDemonDataRef<SLight *> m_Lights;
        QDemonDataRef<QVector3D> m_LightDirections;
        Qt3DSShadowMap *m_ShadowMapManager;
        QDemonRenderTexture2D *m_DepthTexture;
        QDemonRenderTexture2D *m_SSaoTexture;
        SImage *m_LightProbe;
        SImage *m_LightProbe2;
        float m_ProbeHorizon;
        float m_ProbeBright;
        float m_Probe2Window;
        float m_Probe2Pos;
        float m_Probe2Fade;
        float m_ProbeFOV;

        SLayerGlobalRenderProperties(const SLayer &inLayer, SCamera &inCamera,
                                     QVector3D inCameraDirection, QDemonDataRef<SLight *> inLights,
                                     QDemonDataRef<QVector3D> inLightDirections,
                                     Qt3DSShadowMap *inShadowMapManager,
                                     QDemonRenderTexture2D *inDepthTexture,
                                     QDemonRenderTexture2D *inSSaoTexture, SImage *inLightProbe,
                                     SImage *inLightProbe2, float inProbeHorizon,
                                     float inProbeBright, float inProbe2Window, float inProbe2Pos,
                                     float inProbe2Fade, float inProbeFOV)
            : m_Layer(inLayer)
            , m_Camera(inCamera)
            , m_CameraDirection(inCameraDirection)
            , m_Lights(inLights)
            , m_LightDirections(inLightDirections)
            , m_ShadowMapManager(inShadowMapManager)
            , m_DepthTexture(inDepthTexture)
            , m_SSaoTexture(inSSaoTexture)
            , m_LightProbe(inLightProbe)
            , m_LightProbe2(inLightProbe2)
            , m_ProbeHorizon(inProbeHorizon)
            , m_ProbeBright(inProbeBright)
            , m_Probe2Window(inProbe2Window)
            , m_Probe2Pos(inProbe2Pos)
            , m_Probe2Fade(inProbe2Fade)
            , m_ProbeFOV(inProbeFOV)
        {
        }
    };

    class IMaterialShaderGenerator : public QDemonRefCounted
    {
    public:
        struct SImageVariableNames
        {
            const char8_t *m_ImageSampler;
            const char8_t *m_ImageFragCoords;
        };

        virtual SImageVariableNames GetImageVariableNames(quint32 inIdx) = 0;
        virtual void GenerateImageUVCoordinates(IShaderStageGenerator &inVertexPipeline, quint32 idx,
                                                quint32 uvSet, SRenderableImage &image) = 0;

        // inPipelineName needs to be unique else the shader cache will just return shaders from
        // different pipelines.
        virtual QDemonRenderShaderProgram *GenerateShader(
            const SGraphObject &inMaterial, SShaderDefaultMaterialKey inShaderDescription,
            IShaderStageGenerator &inVertexPipeline, TShaderFeatureSet inFeatureSet,
            QDemonDataRef<SLight *> inLights, SRenderableImage *inFirstImage, bool inHasTransparency,
            const char8_t *inVertexPipelineName, const char8_t *inCustomMaterialName = "") = 0;

        // Also sets the blend function on the render context.
        virtual void
        SetMaterialProperties(QDemonRenderShaderProgram &inProgram, const SGraphObject &inMaterial,
                              const QVector2D &inCameraVec, const QMatrix4x4 &inModelViewProjection,
                              const QMatrix3x3 &inNormalMatrix, const QMatrix4x4 &inGlobalTransform,
                              SRenderableImage *inFirstImage, float inOpacity,
                              SLayerGlobalRenderProperties inRenderProperties) = 0;
    };
}
}

#endif
