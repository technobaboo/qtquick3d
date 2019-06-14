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
#ifndef QDEMON_RENDER_MATERIAL_SHADER_GENERATOR_H
#define QDEMON_RENDER_MATERIAL_SHADER_GENERATOR_H

#include <QtDemonRuntimeRender/qtdemonruntimerenderglobal.h>
#include <QtDemon/QDemonDataRef>

#include <QtGui/QVector4D>
#include <QtGui/QMatrix4x4>

#include <QtDemonRuntimeRender/qdemonrendershadercache.h>
#include <QtDemonRuntimeRender/qdemonrendershaderkeys.h>

QT_BEGIN_NAMESPACE

// these are our current shader limits
#define QDEMON_MAX_NUM_LIGHTS 16
#define QDEMON_MAX_NUM_SHADOWS 8

// note this struct must exactly match the memory layout of the
// struct sampleLight.glsllib and sampleArea.glsllib. If you make changes here you need
// to adjust the code in sampleLight.glsllib and sampleArea.glsllib as well
struct QDemonLightSourceShader
{
    QVector4D position;
    QVector4D direction; // Specifies the light direction in world coordinates.
    QVector4D up;
    QVector4D right;
    QVector4D diffuse;
    QVector4D ambient;
    QVector4D specular;
    float spotExponent; // Specifies the intensity distribution of the light.
    float spotCutoff; // Specifies the maximum spread angle of the light.
    float constantAttenuation; // Specifies the constant light attenuation factor.
    float linearAttenuation; // Specifies the linear light attenuation factor.
    float quadraticAttenuation; // Specifies the quadratic light attenuation factor.
    float range; // Specifies the maximum distance of the light influence
    float width; // Specifies the width of the area light surface.
    float height; // Specifies the height of the area light surface;
    QVector4D shadowControls;
    float shadowView[16];
    qint32 shadowIdx;
    float padding1[3];
};

struct QDemonRenderLayer;
struct QDemonRenderCamera;
struct QDemonRenderLight;
class QDemonRenderShadowMap;
class QDemonRenderTexture2D;
struct QDemonRenderImage;
class QDemonShaderStageGeneratorInterface;
struct QDemonRenderableImage;
class QDemonRenderShaderProgram;
struct QDemonRenderGraphObject;
struct QDemonShaderDefaultMaterialKey;
class QDemonRenderContextInterface;
class QDemonShaderProgramGeneratorInterface;
class QDemonDefaultMaterialVertexPipelineInterface;
struct QDemonShaderGeneratorGeneratedShader;
class QDemonRenderConstantBuffer;

struct QDemonLayerGlobalRenderProperties
{
    const QDemonRenderLayer &layer;
    QDemonRenderCamera &camera;
    QVector3D cameraDirection;
    QVector<QDemonRenderLight *> &lights;
    QVector<QVector3D> lightDirections;
    QDemonRef<QDemonRenderShadowMap> shadowMapManager;
    QDemonRef<QDemonRenderTexture2D> depthTexture;
    QDemonRef<QDemonRenderTexture2D> ssaoTexture;
    QDemonRenderImage *lightProbe;
    QDemonRenderImage *lightProbe2;
    float probeHorizon;
    float probeBright;
    float probe2Window;
    float probe2Pos;
    float probe2Fade;
    float probeFOV;
};

class QDemonMaterialShaderGeneratorInterface
{
public:
    QAtomicInt ref;

protected:
    typedef QHash<QByteArray, QDemonRef<QDemonRenderConstantBuffer>> ConstanBufferMap;

    bool m_hasTransparency = false;
    QDemonRenderContextInterface *m_renderContext;
    QDemonRef<QDemonShaderProgramGeneratorInterface> m_programGenerator;

    QDemonShaderDefaultMaterialKey *m_currentKey = nullptr;
    QDemonDefaultMaterialVertexPipelineInterface *m_currentPipeline = nullptr;
    TShaderFeatureSet m_currentFeatureSet;
    QVector<QDemonRenderLight *> m_lights;
    QDemonRenderableImage *m_firstImage = nullptr;
    QDemonShaderDefaultMaterialKeyProperties m_defaultMaterialShaderKeyProperties;
    ConstanBufferMap m_constantBuffers; ///< store all constants buffers


protected:
    QDemonMaterialShaderGeneratorInterface(QDemonRenderContextInterface *renderContext);
public:
    virtual ~QDemonMaterialShaderGeneratorInterface();
    struct ImageVariableNames
    {
        QByteArray m_imageSampler;
        QByteArray m_imageFragCoords;
    };

    virtual ImageVariableNames getImageVariableNames(quint32 inIdx) = 0;
    virtual void generateImageUVCoordinates(QDemonShaderStageGeneratorInterface &inVertexPipeline,
                                            quint32 idx,
                                            quint32 uvSet,
                                            QDemonRenderableImage &image) = 0;

    // inPipelineName needs to be unique else the shader cache will just return shaders from
    // different pipelines.
    virtual QDemonRef<QDemonRenderShaderProgram> generateShader(const QDemonRenderGraphObject &inMaterial,
                                                                QDemonShaderDefaultMaterialKey inShaderDescription,
                                                                QDemonShaderStageGeneratorInterface &inVertexPipeline,
                                                                const TShaderFeatureSet &inFeatureSet,
                                                                const QVector<QDemonRenderLight *> &inLights,
                                                                QDemonRenderableImage *inFirstImage,
                                                                bool inHasTransparency,
                                                                const QByteArray &inVertexPipelineName,
                                                                const QByteArray &inCustomMaterialName = QByteArray()) = 0;

    // Also sets the blend function on the render context.
    virtual void setMaterialProperties(const QDemonRef<QDemonRenderShaderProgram> &inProgram,
                                       const QDemonRenderGraphObject &inMaterial,
                                       const QVector2D &inCameraVec,
                                       const QMatrix4x4 &inModelViewProjection,
                                       const QMatrix3x3 &inNormalMatrix,
                                       const QMatrix4x4 &inGlobalTransform,
                                       QDemonRenderableImage *inFirstImage,
                                       float inOpacity,
                                       const QDemonLayerGlobalRenderProperties &inRenderProperties) = 0;

    static QDemonRef<QDemonMaterialShaderGeneratorInterface> createCustomMaterialShaderGenerator(QDemonRenderContextInterface *inRenderContext);
};
QT_END_NAMESPACE

#endif
