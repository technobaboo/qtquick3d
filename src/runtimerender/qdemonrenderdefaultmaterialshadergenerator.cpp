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
#include <QtDemon/qdemonutils.h>

#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>

#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterialshadergenerator.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRuntimeRender/qdemonrendershadercodegeneratorv2.h>
#include <QtDemonRuntimeRender/qdemonrenderableimage.h>
#include <QtDemonRuntimeRender/qdemonrenderimage.h>
#include <QtDemonRuntimeRender/qdemonrenderlight.h>
#include <QtDemonRuntimeRender/qdemonrendercamera.h>
#include <QtDemonRuntimeRender/qdemonrendershadowmap.h>
#include <QtDemonRuntimeRender/qdemonrendercustommaterial.h>
#include <QtDemonRuntimeRender/qdemonrenderdynamicobjectsystem.h>
#include <QtDemonRuntimeRender/qdemonrenderlightconstantproperties.h>
#include <QtDemonRuntimeRender/qdemonrendershaderkeys.h>
#include <QtDemonRuntimeRender/qdemonrendererimplshaders.h>

QT_BEGIN_NAMESPACE

namespace {

const float MINATTENUATION = 0;
const float MAXATTENUATION = 1000;

float clampFloat(float value, float min, float max)
{
    return value < min ? min : ((value > max) ? max : value);
}

float translateConstantAttenuation(float attenuation)
{
    return attenuation * .01f;
}

float translateLinearAttenuation(float attenuation)
{
    attenuation = clampFloat(attenuation, MINATTENUATION, MAXATTENUATION);
    return attenuation * 0.0001f;
}

float translateQuadraticAttenuation(float attenuation)
{
    attenuation = clampFloat(attenuation, MINATTENUATION, MAXATTENUATION);
    return attenuation * 0.0000001f;
}

/**
 *	Cached light property lookups, used one per light so a shader generator for N
 *	lights will have an array of N of these lookup objects.
 */
struct QDemonShaderLightProperties
{
    // Color of the light
    QVector3D lightColor;
    QDemonLightSourceShader lightData;
};

struct QDemonShadowMapProperties
{
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> m_shadowmapTexture; ///< shadow texture
    QDemonRenderCachedShaderProperty<QDemonRenderTextureCube *> m_shadowCubeTexture; ///< shadow cubemap
    QDemonRenderCachedShaderProperty<QMatrix4x4> m_shadowmapMatrix; ///< world to ligh space transform matrix
    QDemonRenderCachedShaderProperty<QVector4D> m_shadowmapSettings; ///< shadow rendering settings

    QDemonShadowMapProperties() = default;
    QDemonShadowMapProperties(const QByteArray &shadowmapTextureName,
                              const QByteArray &shadowcubeTextureName,
                              const QByteArray &shadowmapMatrixName,
                              const QByteArray &shadowmapSettingsName,
                              const QDemonRef<QDemonRenderShaderProgram> &inShader)
        : m_shadowmapTexture(shadowmapTextureName, inShader)
        , m_shadowCubeTexture(shadowcubeTextureName, inShader)
        , m_shadowmapMatrix(shadowmapMatrixName, inShader)
        , m_shadowmapSettings(shadowmapSettingsName, inShader)
    {
    }
};

/**
 *	The results of generating a shader.  Caches all possible variable names into
 *	typesafe objects.
 */
struct QDemonShaderGeneratorGeneratedShader
{
    QAtomicInt ref;
    QDemonRef<QDemonRenderShaderProgram> m_shader;
    // Specific properties we know the shader has to have.
    QDemonRenderCachedShaderProperty<QMatrix4x4> m_mvp;
    QDemonRenderCachedShaderProperty<QMatrix3x3> m_normalMatrix;
    QDemonRenderCachedShaderProperty<QMatrix4x4> m_globalTransform;
    QDemonRenderCachedShaderProperty<QMatrix4x4> m_viewProj;
    QDemonRenderCachedShaderProperty<QMatrix4x4> m_viewMatrix;
    QDemonRenderCachedShaderProperty<QVector4D> m_materialDiffuse;
    QDemonRenderCachedShaderProperty<QVector4D> m_materialProperties;
    // tint, ior
    QDemonRenderCachedShaderProperty<QVector4D> m_materialSpecular;
    QDemonRenderCachedShaderProperty<float> m_bumpAmount;
    QDemonRenderCachedShaderProperty<float> m_displaceAmount;
    QDemonRenderCachedShaderProperty<float> m_translucentFalloff;
    QDemonRenderCachedShaderProperty<float> m_diffuseLightWrap;
    QDemonRenderCachedShaderProperty<float> m_fresnelPower;
    QDemonRenderCachedShaderProperty<QVector3D> m_diffuseColor;
    QDemonRenderCachedShaderProperty<QVector3D> m_cameraPosition;
    QDemonRenderCachedShaderProperty<QVector3D> m_cameraDirection;
    QVector3D m_lightAmbientTotal;
    QDemonRenderCachedShaderProperty<QVector3D> m_materialDiffuseLightAmbientTotal;
    QDemonRenderCachedShaderProperty<QVector2D> m_cameraProperties;

    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> m_depthTexture;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> m_aoTexture;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> m_lightProbe;
    QDemonRenderCachedShaderProperty<QVector4D> m_lightProbeProps;
    QDemonRenderCachedShaderProperty<QVector4D> m_lightProbeOpts;
    QDemonRenderCachedShaderProperty<QVector4D> m_lightProbeRot;
    QDemonRenderCachedShaderProperty<QVector4D> m_lightProbeOfs;
    QDemonRenderCachedShaderProperty<QVector2D> m_lightProbeSize;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> m_lightProbe2;
    QDemonRenderCachedShaderProperty<QVector4D> m_lightProbe2Props;
    QDemonRenderCachedShaderProperty<QVector2D> m_lightProbe2Size;

    QDemonRenderCachedShaderBuffer<QDemonRenderShaderConstantBuffer> m_aoShadowParams;
    QDemonRenderCachedShaderBuffer<QDemonRenderShaderConstantBuffer> m_lightsBuffer;

    QDemonLightConstantProperties<QDemonShaderGeneratorGeneratedShader> *m_lightConstantProperties = nullptr;

    // Cache the image property name lookups
    QVector<QDemonShaderTextureProperties> m_images;
    QVector<QDemonShaderLightProperties> m_lights;
    // Cache shadow map properties
    QVector<QDemonShadowMapProperties> m_shadowMaps;

    QDemonShaderGeneratorGeneratedShader(const QDemonRef<QDemonRenderShaderProgram> &inShader,
                                         const QDemonRef<QDemonRenderContext> &inContext)
        : m_shader(inShader)
        , m_mvp("model_view_projection", inShader)
        , m_normalMatrix("normal_matrix", inShader)
        , m_globalTransform("model_matrix", inShader)
        , m_viewProj("view_projection_matrix", inShader)
        , m_viewMatrix("view_matrix", inShader)
        , m_materialDiffuse("material_diffuse", inShader)
        , m_materialProperties("material_properties", inShader)
        , m_materialSpecular("material_specular", inShader)
        , m_bumpAmount("bumpAmount", inShader)
        , m_displaceAmount("displaceAmount", inShader)
        , m_translucentFalloff("translucentFalloff", inShader)
        , m_diffuseLightWrap("diffuseLightWrap", inShader)
        , m_fresnelPower("fresnelPower", inShader)
        , m_diffuseColor("diffuse_color", inShader)
        , m_cameraPosition("camera_position", inShader)
        , m_cameraDirection("camera_direction", inShader)
        , m_materialDiffuseLightAmbientTotal("light_ambient_total", inShader)
        , m_cameraProperties("camera_properties", inShader)
        , m_depthTexture("depth_sampler", inShader)
        , m_aoTexture("ao_sampler", inShader)
        , m_lightProbe("light_probe", inShader)
        , m_lightProbeProps("light_probe_props", inShader)
        , m_lightProbeOpts("light_probe_opts", inShader)
        , m_lightProbeRot("light_probe_rotation", inShader)
        , m_lightProbeOfs("light_probe_offset", inShader)
        , m_lightProbeSize("light_probe_size", inShader)
        , m_lightProbe2("light_probe2", inShader)
        , m_lightProbe2Props("light_probe2_props", inShader)
        , m_lightProbe2Size("light_probe2_size", inShader)
        , m_aoShadowParams("cbAoShadow", inShader)
        , m_lightsBuffer("cbBufferLights", inShader)
    {
    }
    ~QDemonShaderGeneratorGeneratedShader() { delete m_lightConstantProperties; }
};

struct QDemonShaderGenerator : public QDemonDefaultMaterialShaderGeneratorInterface
{
    const QDemonRenderDefaultMaterial *m_currentMaterial;

    typedef QHash<QDemonRef<QDemonRenderShaderProgram>, QDemonRef<QDemonShaderGeneratorGeneratedShader>> ProgramToShaderMap;
    ProgramToShaderMap m_programToShaderMap;

    QDemonRef<QDemonRenderShadowMap> m_shadowMapManager;
    bool m_lightsAsSeparateUniforms;

    QByteArray m_imageSampler;
    QByteArray m_imageFragCoords;
    QByteArray m_imageOffsets;
    QByteArray m_imageRotations;
    QByteArray m_imageTemp;
    QByteArray m_imageSamplerSize;

    QByteArray m_lightColor;
    QByteArray m_lightSpecularColor;
    QByteArray m_lightAttenuation;
    QByteArray m_lightConstantAttenuation;
    QByteArray m_lightLinearAttenuation;
    QByteArray m_lightQuadraticAttenuation;
    QByteArray m_normalizedDirection;
    QByteArray m_lightDirection;
    QByteArray m_lightPos;
    QByteArray m_lightUp;
    QByteArray m_lightRt;
    QByteArray m_relativeDistance;
    QByteArray m_relativeDirection;

    QByteArray m_shadowMapStem;
    QByteArray m_shadowCubeStem;
    QByteArray m_shadowMatrixStem;
    QByteArray m_shadowCoordStem;
    QByteArray m_shadowControlStem;

    QDemonShaderGenerator(QDemonRenderContextInterface *inRc)
        : QDemonDefaultMaterialShaderGeneratorInterface (inRc)
        , m_shadowMapManager(nullptr)
        , m_lightsAsSeparateUniforms(false)
    {
    }

    QDemonRef<QDemonShaderProgramGeneratorInterface> programGenerator() { return m_programGenerator; }
    QDemonDefaultMaterialVertexPipelineInterface &vertexGenerator() { return *m_currentPipeline; }
    QDemonShaderStageGeneratorInterface &fragmentGenerator()
    {
        return *m_programGenerator->getStage(QDemonShaderGeneratorStage::Fragment);
    }
    QDemonShaderDefaultMaterialKey &key() { return *m_currentKey; }
    const QDemonRenderDefaultMaterial *material() { return m_currentMaterial; }
    bool hasTransparency() { return m_hasTransparency; }

    void addFunction(QDemonShaderStageGeneratorInterface &generator, const QByteArray &functionName)
    {
        generator.addFunction(functionName);
    }

    void setupImageVariableNames(size_t imageIdx)
    {
        QByteArray imageStem = "image";
        char buf[16];
        qsnprintf(buf, 16, "%d", int(imageIdx));
        imageStem.append(buf);
        imageStem.append("_");

        m_imageSampler = imageStem;
        m_imageSampler.append("sampler");
        m_imageOffsets = imageStem;
        m_imageOffsets.append("offsets");
        m_imageRotations = imageStem;
        m_imageRotations.append("rotations");
        m_imageFragCoords = imageStem;
        m_imageFragCoords.append("uv_coords");
        m_imageSamplerSize = imageStem;
        m_imageSamplerSize.append("size");
    }

    QByteArray textureCoordVariableName(size_t uvSet)
    {
        QByteArray texCoordTemp = "varTexCoord";
        char buf[16];
        qsnprintf(buf, 16, "%d", int(uvSet));
        texCoordTemp.append(buf);
        return texCoordTemp;
    }

    ImageVariableNames getImageVariableNames(quint32 inIdx) override
    {
        setupImageVariableNames(inIdx);
        ImageVariableNames retval;
        retval.m_imageSampler = m_imageSampler;
        retval.m_imageFragCoords = m_imageFragCoords;
        return retval;
    }

    void addLocalVariable(QDemonShaderStageGeneratorInterface &inGenerator, const QByteArray &inName, const QByteArray &inType)
    {
        inGenerator << "\t" << inType << " " << inName << ";"
                    << "\n";
    }

    void generateImageUVCoordinates(QDemonShaderStageGeneratorInterface &inVertexPipeline, quint32 idx, quint32 uvSet, QDemonRenderableImage &image) override
    {
        QDemonDefaultMaterialVertexPipelineInterface &vertexShader(
                static_cast<QDemonDefaultMaterialVertexPipelineInterface &>(inVertexPipeline));
        QDemonShaderStageGeneratorInterface &fragmentShader(fragmentGenerator());
        setupImageVariableNames(idx);
        QByteArray textureCoordName = textureCoordVariableName(uvSet);
        fragmentShader.addUniform(m_imageSampler, "sampler2D");
        vertexShader.addUniform(m_imageOffsets, "vec3");
        fragmentShader.addUniform(m_imageOffsets, "vec3");
        vertexShader.addUniform(m_imageRotations, "vec4");
        fragmentShader.addUniform(m_imageRotations, "vec4");

        if (image.m_image.m_mappingMode == ImageMappingModes::Normal) {
            vertexShader << "\tuTransform = vec3( " << m_imageRotations << ".x, " << m_imageRotations << ".y, "
                         << m_imageOffsets << ".x );"
                         << "\n";
            vertexShader << "\tvTransform = vec3( " << m_imageRotations << ".z, " << m_imageRotations << ".w, "
                         << m_imageOffsets << ".y );"
                         << "\n";
            vertexShader.addOutgoing(m_imageFragCoords, "vec2");
            addFunction(vertexShader, "getTransformedUVCoords");
            vertexShader.generateUVCoords(uvSet);
            m_imageTemp = m_imageFragCoords;
            m_imageTemp.append("temp");
            vertexShader << "\tvec2 " << m_imageTemp << " = getTransformedUVCoords( vec3( " << textureCoordName
                         << ", 1.0), uTransform, vTransform );"
                         << "\n";
            if (image.m_image.m_textureData.m_textureFlags.isInvertUVCoords())
                vertexShader << "\t" << m_imageTemp << ".y = 1.0 - " << m_imageFragCoords << ".y;"
                             << "\n";

            vertexShader.assignOutput(m_imageFragCoords, m_imageTemp);
        } else {
            fragmentShader << "\tuTransform = vec3( " << m_imageRotations << ".x, " << m_imageRotations << ".y, "
                           << m_imageOffsets << ".x );"
                           << "\n";
            fragmentShader << "\tvTransform = vec3( " << m_imageRotations << ".z, " << m_imageRotations << ".w, "
                           << m_imageOffsets << ".y );"
                           << "\n";
            vertexShader.generateEnvMapReflection();
            addFunction(fragmentShader, "getTransformedUVCoords");
            fragmentShader << "\tvec2 " << m_imageFragCoords
                           << " = getTransformedUVCoords( environment_map_reflection, uTransform, "
                              "vTransform );"
                           << "\n";
            if (image.m_image.m_textureData.m_textureFlags.isInvertUVCoords())
                fragmentShader << "\t" << m_imageFragCoords << ".y = 1.0 - " << m_imageFragCoords << ".y;"
                               << "\n";
        }
    }

    void generateImageUVCoordinates(quint32 idx, QDemonRenderableImage &image, quint32 uvSet = 0)
    {
        generateImageUVCoordinates(vertexGenerator(), idx, uvSet, image);
    }

    void generateImageUVCoordinates(quint32 idx, QDemonRenderableImage &image, QDemonDefaultMaterialVertexPipelineInterface &inShader)
    {
        if (image.m_image.m_mappingMode == ImageMappingModes::Normal) {
            setupImageVariableNames(idx);
            inShader.addUniform(m_imageSampler, "sampler2D");
            inShader.addUniform(m_imageOffsets, "vec3");
            inShader.addUniform(m_imageRotations, "vec4");

            inShader << "\tuTransform = vec3( " << m_imageRotations << ".x, " << m_imageRotations << ".y, "
                     << m_imageOffsets << ".x );"
                     << "\n";
            inShader << "\tvTransform = vec3( " << m_imageRotations << ".z, " << m_imageRotations << ".w, "
                     << m_imageOffsets << ".y );"
                     << "\n";
            inShader << "\tvec2 " << m_imageFragCoords << ";"
                     << "\n";
            addFunction(inShader, "getTransformedUVCoords");
            inShader.generateUVCoords();
            inShader << "\t" << m_imageFragCoords << " = getTransformedUVCoords( vec3( varTexCoord0, 1.0), uTransform, vTransform );"
                     << "\n";
            if (image.m_image.m_textureData.m_textureFlags.isInvertUVCoords())
                inShader << "\t" << m_imageFragCoords << ".y = 1.0 - " << m_imageFragCoords << ".y;"
                         << "\n";
        }
    }

    void outputSpecularEquation(DefaultMaterialSpecularModel inSpecularModel,
                                QDemonShaderStageGeneratorInterface &fragmentShader,
                                const QString &inLightDir,
                                const QString &inLightSpecColor)
    {
        switch (inSpecularModel) {
        case DefaultMaterialSpecularModel::KGGX: {
            fragmentShader.addInclude("defaultMaterialPhysGlossyBSDF.glsllib");
            fragmentShader.addUniform("material_specular", "vec4");
            fragmentShader << "\tglobal_specular_light.rgb += lightAttenuation * specularAmount * "
                              "specularColor * kggxGlossyDefaultMtl( "
                           << "world_normal, tangent, -" << inLightDir.toUtf8() << ".xyz, view_vector, "
                           << inLightSpecColor.toUtf8()
                           << ".rgb, vec3(material_specular.xyz), roughnessAmount, "
                              "roughnessAmount ).rgb;"
                           << "\n";
        } break;
        case DefaultMaterialSpecularModel::KWard: {
            fragmentShader.addInclude("defaultMaterialPhysGlossyBSDF.glsllib");
            fragmentShader.addUniform("material_specular", "vec4");
            fragmentShader << "\tglobal_specular_light.rgb += lightAttenuation * specularAmount * "
                              "specularColor * wardGlossyDefaultMtl( "
                           << "world_normal, tangent, -" << inLightDir.toUtf8() << ".xyz, view_vector, "
                           << inLightSpecColor.toUtf8()
                           << ".rgb, vec3(material_specular.xyz), roughnessAmount, "
                              "roughnessAmount ).rgb;"
                           << "\n";
        } break;
        default:
            addFunction(fragmentShader, "specularBSDF");
            fragmentShader << "\tglobal_specular_light.rgb += lightAttenuation * specularAmount * "
                              "specularColor * specularBSDF( "
                           << "world_normal, -" << inLightDir.toUtf8() << ".xyz, view_vector, " << inLightSpecColor.toUtf8()
                           << ".rgb, 1.0, 2.56 / (roughnessAmount + "
                              "0.01), vec3(1.0), scatter_reflect ).rgb;"
                           << "\n";
            break;
        }
    }

    void OutputDiffuseAreaLighting(QDemonShaderStageGeneratorInterface &infragmentShader, const QByteArray &inPos, const QByteArray &inLightPrefix)
    {
        m_normalizedDirection = inLightPrefix + "_areaDir";
        addLocalVariable(infragmentShader, m_normalizedDirection, "vec3");
        infragmentShader << "\tlightAttenuation = calculateDiffuseAreaOld( " << m_lightDirection << ".xyz, " << m_lightPos
                         << ".xyz, " << m_lightUp << ", " << m_lightRt << ", " << inPos << ", " << m_normalizedDirection << " );"
                         << "\n";
    }

    void OutputSpecularAreaLighting(QDemonShaderStageGeneratorInterface &infragmentShader,
                                    const QByteArray &inPos,
                                    const QByteArray &inView,
                                    const QByteArray &inLightSpecColor)
    {
        addFunction(infragmentShader, "sampleAreaGlossyDefault");
        infragmentShader.addUniform("material_specular", "vec4");
        infragmentShader << "global_specular_light.rgb += " << inLightSpecColor
                         << ".rgb * lightAttenuation * shadowFac * material_specular.rgb * "
                            "specularAmount * sampleAreaGlossyDefault( tanFrame, "
                         << inPos << ", " << m_normalizedDirection << ", " << m_lightPos << ".xyz, " << m_lightRt
                         << ".w, " << m_lightUp << ".w, " << inView << ", roughnessAmount, roughnessAmount ).rgb;"
                         << "\n";
    }

    void addTranslucencyIrradiance(QDemonShaderStageGeneratorInterface &infragmentShader,
                                   QDemonRenderableImage *image,
                                   bool areaLight)
    {
        if (image == nullptr)
            return;

        addFunction(infragmentShader, "diffuseReflectionWrapBSDF");
        if (areaLight) {
            infragmentShader << "\tglobal_diffuse_light.rgb += lightAttenuation * "
                                "translucent_thickness_exp * diffuseReflectionWrapBSDF( "
                                "-world_normal, "
                             << m_normalizedDirection << ", " << m_lightColor << ".rgb, diffuseLightWrap ).rgb;"
                             << "\n";
        } else {
            infragmentShader << "\tglobal_diffuse_light.rgb += lightAttenuation * "
                                "translucent_thickness_exp * diffuseReflectionWrapBSDF( "
                                "-world_normal, "
                             << "-" << m_normalizedDirection << ", " << m_lightColor << ".rgb, diffuseLightWrap ).rgb;"
                             << "\n";
        }
    }

    void setupShadowMapVariableNames(size_t lightIdx)
    {
        m_shadowMapStem = "shadowmap";
        m_shadowCubeStem = "shadowcube";
        char buf[16];
        qsnprintf(buf, 16, "%d", int(lightIdx));
        m_shadowMapStem.append(buf);
        m_shadowCubeStem.append(buf);
        m_shadowMatrixStem = m_shadowMapStem;
        m_shadowMatrixStem.append("_matrix");
        m_shadowCoordStem = m_shadowMapStem;
        m_shadowCoordStem.append("_coord");
        m_shadowControlStem = m_shadowMapStem;
        m_shadowControlStem.append("_control");
    }

    void addShadowMapContribution(QDemonShaderStageGeneratorInterface &inLightShader, quint32 lightIndex, RenderLightTypes inType)
    {
        setupShadowMapVariableNames(lightIndex);

        inLightShader.addInclude("shadowMapping.glsllib");
        if (inType == RenderLightTypes::Directional) {
            inLightShader.addUniform(m_shadowMapStem, "sampler2D");
        } else {
            inLightShader.addUniform(m_shadowCubeStem, "samplerCube");
        }
        inLightShader.addUniform(m_shadowControlStem, "vec4");
        inLightShader.addUniform(m_shadowMatrixStem, "mat4");

        /*
        if ( inType == RenderLightTypes::Area )
        {
                inLightShader << "vec2 " << m_shadowCoordStem << ";" << "\n";
                inLightShader << "\tshadow_map_occl = sampleParaboloid( " << m_shadowMapStem << ", "
        << m_shadowControlStem << ", "
                                                                                <<
        m_shadowMatrixStem << ", varWorldPos, vec2(1.0, " << m_shadowControlStem << ".z), "
                                                                                << m_shadowCoordStem
        << " );" << "\n";
        }
        else */
        if (inType != RenderLightTypes::Directional) {
            inLightShader << "\tshadow_map_occl = sampleCubemap( " << m_shadowCubeStem << ", " << m_shadowControlStem
                          << ", " << m_shadowMatrixStem << ", " << m_lightPos << ".xyz, varWorldPos, vec2(1.0, "
                          << m_shadowControlStem << ".z) );"
                          << "\n";
        } else
            inLightShader << "\tshadow_map_occl = sampleOrthographic( " << m_shadowMapStem << ", " << m_shadowControlStem
                          << ", " << m_shadowMatrixStem << ", varWorldPos, vec2(1.0, " << m_shadowControlStem << ".z) );"
                          << "\n";
    }

    void addDisplacementMappingForDepthPass(QDemonShaderStageGeneratorInterface &inShader) override
    {
        inShader.addIncoming("attr_uv0", "vec2");
        inShader.addIncoming("attr_norm", "vec3");
        inShader.addUniform("displacementSampler", "sampler2D");
        inShader.addUniform("displaceAmount", "float");
        inShader.addUniform("displacementMap_rot", "vec4");
        inShader.addUniform("displacementMap_offset", "vec3");
        inShader.addInclude("defaultMaterialFileDisplacementTexture.glsllib");

        inShader.append("\tvec3 uTransform = vec3( displacementMap_rot.x, displacementMap_rot.y, "
                        "displacementMap_offset.x );");
        inShader.append("\tvec3 vTransform = vec3( displacementMap_rot.z, displacementMap_rot.w, "
                        "displacementMap_offset.y );");
        addFunction(inShader, "getTransformedUVCoords");
        inShader.append("\tvec2 uv_coords = attr_uv0;");
        inShader << "\tuv_coords = getTransformedUVCoords( vec3( uv_coords, 1.0), uTransform, "
                    "vTransform );\n";
        inShader << "\tvec3 displacedPos = defaultMaterialFileDisplacementTexture( "
                    "displacementSampler , displaceAmount, uv_coords , attr_norm, attr_pos );"
                 << "\n";
        inShader.append("\tgl_Position = model_view_projection * vec4(displacedPos, 1.0);");
    }

    void addDisplacementImageUniforms(QDemonShaderStageGeneratorInterface &inGenerator,
                                      quint32 displacementImageIdx,
                                      QDemonRenderableImage *displacementImage) override
    {
        if (displacementImage) {
            setupImageVariableNames(displacementImageIdx);
            inGenerator.addInclude("defaultMaterialFileDisplacementTexture.glsllib");
            inGenerator.addUniform("model_matrix", "mat4");
            inGenerator.addUniform("camera_position", "vec3");
            inGenerator.addUniform("displaceAmount", "float");
            inGenerator.addUniform(m_imageSampler, "sampler2D");
        }
    }

    bool maybeAddMaterialFresnel(QDemonShaderStageGeneratorInterface &fragmentShader, QDemonConstDataRef<quint32> inKey, bool inFragmentHasSpecularAmount)
    {
        if (m_defaultMaterialShaderKeyProperties.m_fresnelEnabled.getValue(inKey)) {
            if (inFragmentHasSpecularAmount == false)
                fragmentShader << "\tfloat specularAmount = 1.0;"
                               << "\n";
            inFragmentHasSpecularAmount = true;
            fragmentShader.addInclude("defaultMaterialFresnel.glsllib");
            fragmentShader.addUniform("fresnelPower", "float");
            fragmentShader.addUniform("material_specular", "vec4");
            fragmentShader << "\tfloat fresnelRatio = defaultMaterialSimpleFresnel( world_normal, "
                              "view_vector, material_specular.w, fresnelPower );"
                           << "\n";
            fragmentShader << "\tspecularAmount *= fresnelRatio;"
                           << "\n";
        }
        return inFragmentHasSpecularAmount;
    }
    void setupLightVariableNames(qint32 lightIdx, QDemonRenderLight &inLight)
    {
        Q_ASSERT(lightIdx > -1);
        if (m_lightsAsSeparateUniforms) {
            char buf[16];
            qsnprintf(buf, 16, "light_%d", int(lightIdx));
            QByteArray lightStem = buf;
            m_lightColor = lightStem;
            m_lightColor.append("_diffuse");
            m_lightDirection = lightStem;
            m_lightDirection.append("_direction");
            m_lightSpecularColor = lightStem;
            m_lightSpecularColor.append("_specular");
            if (inLight.m_lightType == RenderLightTypes::Point) {
                m_lightPos = lightStem;
                m_lightPos.append("_position");
                m_lightAttenuation = lightStem;
                m_lightAttenuation.append("_attenuation");
            } else if (inLight.m_lightType == RenderLightTypes::Area) {
                m_lightPos = lightStem;
                m_lightPos.append("_position");
                m_lightUp = lightStem;
                m_lightUp.append("_up");
                m_lightRt = lightStem;
                m_lightRt.append("_right");
            }
        } else {
            QByteArray lightStem = "lights";
            char buf[16];
            qsnprintf(buf, 16, "[%d].", int(lightIdx));
            lightStem.append(buf);

            m_lightColor = lightStem;
            m_lightColor.append("diffuse");
            m_lightDirection = lightStem;
            m_lightDirection.append("direction");
            m_lightSpecularColor = lightStem;
            m_lightSpecularColor.append("specular");
            if (inLight.m_lightType == RenderLightTypes::Point) {
                m_lightPos = lightStem;
                m_lightPos.append("position");
                m_lightConstantAttenuation = lightStem;
                m_lightConstantAttenuation.append("constantAttenuation");
                m_lightLinearAttenuation = lightStem;
                m_lightLinearAttenuation.append("linearAttenuation");
                m_lightQuadraticAttenuation = lightStem;
                m_lightQuadraticAttenuation.append("quadraticAttenuation");
            } else if (inLight.m_lightType == RenderLightTypes::Area) {
                m_lightPos = lightStem;
                m_lightPos.append("position");
                m_lightUp = lightStem;
                m_lightUp.append("up");
                m_lightRt = lightStem;
                m_lightRt.append("right");
            }
        }
    }

    void addDisplacementMapping(QDemonDefaultMaterialVertexPipelineInterface &inShader)
    {
        inShader.addIncoming("attr_uv0", "vec2");
        inShader.addIncoming("attr_norm", "vec3");
        inShader.addUniform("displacementSampler", "sampler2D");
        inShader.addUniform("displaceAmount", "float");
        inShader.addUniform("displacementMap_rot", "vec4");
        inShader.addUniform("displacementMap_offset", "vec3");
        inShader.addInclude("defaultMaterialFileDisplacementTexture.glsllib");

        inShader.append("\tvec3 uTransform = vec3( displacementMap_rot.x, displacementMap_rot.y, "
                        "displacementMap_offset.x );");
        inShader.append("\tvec3 vTransform = vec3( displacementMap_rot.z, displacementMap_rot.w, "
                        "displacementMap_offset.y );");
        addFunction(inShader, "getTransformedUVCoords");
        inShader.generateUVCoords();
        inShader << "\tvarTexCoord0 = getTransformedUVCoords( vec3( varTexCoord0, 1.0), "
                    "uTransform, vTransform );\n";
        inShader << "\tvec3 displacedPos = defaultMaterialFileDisplacementTexture( "
                    "displacementSampler , displaceAmount, varTexCoord0 , attr_norm, attr_pos );"
                 << "\n";
        inShader.append("\tgl_Position = model_view_projection * vec4(displacedPos, 1.0);");
    }

    void generateTextureSwizzle(QDemonRenderTextureSwizzleMode swizzleMode, QByteArray &texSwizzle, QByteArray &lookupSwizzle)
    {
        QDemonRenderContextTypes deprecatedContextFlags(QDemonRenderContextType::GL2 | QDemonRenderContextType::GLES2);

        if (!(deprecatedContextFlags & m_renderContext->getRenderContext()->renderContextType())) {
            switch (swizzleMode) {
            case QDemonRenderTextureSwizzleMode::L8toR8:
            case QDemonRenderTextureSwizzleMode::L16toR16:
                texSwizzle.append(".rgb");
                lookupSwizzle.append(".rrr");
                break;
            case QDemonRenderTextureSwizzleMode::L8A8toRG8:
                texSwizzle.append(".rgba");
                lookupSwizzle.append(".rrrg");
                break;
            case QDemonRenderTextureSwizzleMode::A8toR8:
                texSwizzle.append(".a");
                lookupSwizzle.append(".r");
                break;
            default:
                break;
            }
        }
    }

    ///< get the light constant buffer and generate if necessary
    QDemonRef<QDemonRenderConstantBuffer> getLightConstantBuffer(quint32 inLightCount)
    {
        QDemonRef<QDemonRenderContext> theContext(m_renderContext->getRenderContext());

        // we assume constant buffer support
        Q_ASSERT(theContext->supportsConstantBuffer());

        // we only create if if we have lights
        if (!inLightCount || !theContext->supportsConstantBuffer())
            return nullptr;

        const char *theName = "cbBufferLights";
        QDemonRef<QDemonRenderConstantBuffer> pCB = theContext->getConstantBuffer(theName);

        if (!pCB) {
            // create
            const size_t size = sizeof(QDemonLightSourceShader) * QDEMON_MAX_NUM_LIGHTS + (4 * sizeof(qint32));
            quint8 stackData[size];
            QDemonLightSourceShader *s = new (stackData) QDemonLightSourceShader[QDEMON_MAX_NUM_LIGHTS];
            QDemonDataRef<quint8> cBuffer(stackData, size);
            pCB = new QDemonRenderConstantBuffer(theContext, theName, QDemonRenderBufferUsageType::Static, size, cBuffer);
            if (!pCB) {
                Q_ASSERT(false);
                return nullptr;
            }
            // init first set
            memset(&s[0], 0x0, sizeof(QDemonLightSourceShader));
            qint32 cgLights = 0;
            pCB->updateRaw(0, QDemonDataRef<quint8>((quint8 *)&cgLights, sizeof(qint32)));
            pCB->updateRaw(4 * sizeof(qint32), QDemonDataRef<quint8>((quint8 *)&s[0], sizeof(QDemonLightSourceShader)));
            pCB->update(); // update to hardware

            m_constantBuffers.insert(theName, pCB);
        }

        return pCB;
    }

    void setImageShaderVariables(const QDemonRef<QDemonShaderGeneratorGeneratedShader> &inShader, QDemonRenderableImage &inImage, quint32 idx)
    {
        size_t numImageVariables = inShader->m_images.size();
        for (size_t namesIdx = numImageVariables; namesIdx <= idx; ++namesIdx) {
            setupImageVariableNames(idx);
            inShader->m_images.push_back(
                    QDemonShaderTextureProperties(inShader->m_shader, m_imageSampler, m_imageOffsets, m_imageRotations, m_imageSamplerSize));
        }
        QDemonShaderTextureProperties &theShaderProps = inShader->m_images[idx];
        const QMatrix4x4 &textureTransform = inImage.m_image.m_textureTransform;
        // We separate rotational information from offset information so that just maybe the shader
        // will attempt to push less information to the card.
        const float *dataPtr(textureTransform.constData());
        // The third member of the offsets contains a flag indicating if the texture was
        // premultiplied or not.
        // We use this to mix the texture alpha.
        QVector3D offsets(dataPtr[12], dataPtr[13], inImage.m_image.m_textureData.m_textureFlags.isPreMultiplied() ? 1.0f : 0.0f);
        // Grab just the upper 2x2 rotation matrix from the larger matrix.
        QVector4D rotations(dataPtr[0], dataPtr[4], dataPtr[1], dataPtr[5]);

        // The image horizontal and vertical tiling modes need to be set here, before we set texture
        // on the shader.
        // because setting the image on the texture forces the textue to bind and immediately apply
        // any tex params.
        QDemonRef<QDemonRenderTexture2D> imageTexture = inImage.m_image.m_textureData.m_texture;
        inImage.m_image.m_textureData.m_texture->setTextureWrapS(inImage.m_image.m_horizontalTilingMode);
        inImage.m_image.m_textureData.m_texture->setTextureWrapT(inImage.m_image.m_verticalTilingMode);
        theShaderProps.sampler.set(imageTexture.data());
        theShaderProps.offsets.set(offsets);
        theShaderProps.rotations.set(rotations);
        theShaderProps.size.set(QVector2D(imageTexture->textureDetails().width, imageTexture->textureDetails().height));
    }

    void generateShadowMapOcclusion(quint32 lightIdx, bool inShadowEnabled, RenderLightTypes inType)
    {
        if (inShadowEnabled) {
            vertexGenerator().generateWorldPosition();
            addShadowMapContribution(fragmentGenerator(), lightIdx, inType);
            /*
            VertexGenerator().AddUniform( m_ShadowMatrixStem, "mat4" );
            VertexGenerator().AddOutgoing( m_ShadowCoordStem, "vec4" );
            VertexGenerator() << "\tvec4 local_" << m_ShadowCoordStem << " = " << m_ShadowMatrixStem
            << " * vec4(local_model_world_position, 1.0);" << "\n";
            m_TempStr.assign( "local_" );
            m_TempStr.append( m_ShadowCoordStem );
            VertexGenerator().AssignOutput( m_ShadowCoordStem, m_TempStr );
            */
        } else {
            fragmentGenerator() << "\tshadow_map_occl = 1.0;"
                                << "\n";
        }
    }

    void generateVertexShader()
    {
        // vertex displacement
        quint32 imageIdx = 0;
        QDemonRenderableImage *displacementImage = nullptr;
        quint32 displacementImageIdx = 0;

        for (QDemonRenderableImage *img = m_firstImage; img != nullptr; img = img->m_nextImage, ++imageIdx) {
            if (img->m_mapType == QDemonImageMapTypes::Displacement) {
                displacementImage = img;
                displacementImageIdx = imageIdx;
                break;
            }
        }

        // the pipeline opens/closes up the shaders stages
        vertexGenerator().beginVertexGeneration(displacementImageIdx, displacementImage);
    }

    void generateFragmentShader(QDemonShaderDefaultMaterialKey &inKey)
    {
        bool specularEnabled = material()->isSpecularEnabled();
        bool vertexColorsEnabled = material()->isVertexColorsEnabled();

        bool hasLighting = material()->hasLighting();
        bool hasImage = m_firstImage != nullptr;

        bool hasIblProbe = m_defaultMaterialShaderKeyProperties.m_hasIbl.getValue(inKey);
        bool hasSpecMap = false;
        bool hasEnvMap = false;
        bool hasEmissiveMap = false;
        bool hasLightmaps = false;
        // Pull the bump out as
        QDemonRenderableImage *bumpImage = nullptr;
        quint32 imageIdx = 0;
        quint32 bumpImageIdx = 0;
        QDemonRenderableImage *specularAmountImage = nullptr;
        quint32 specularAmountImageIdx = 0;
        QDemonRenderableImage *roughnessImage = nullptr;
        quint32 roughnessImageIdx = 0;
        // normal mapping
        QDemonRenderableImage *normalImage = nullptr;
        quint32 normalImageIdx = 0;
        // translucency map
        QDemonRenderableImage *translucencyImage = nullptr;
        quint32 translucencyImageIdx = 0;
        // lightmaps
        QDemonRenderableImage *lightmapIndirectImage = nullptr;
        quint32 lightmapIndirectImageIdx = 0;
        QDemonRenderableImage *lightmapRadiosityImage = nullptr;
        quint32 lightmapRadiosityImageIdx = 0;
        QDemonRenderableImage *lightmapShadowImage = nullptr;
        quint32 lightmapShadowImageIdx = 0;
        const bool supportStandardDerivatives = m_renderContext->getRenderContext()->supportsStandardDerivatives();

        for (QDemonRenderableImage *img = m_firstImage; img != nullptr; img = img->m_nextImage, ++imageIdx) {
            hasSpecMap = img->m_mapType == QDemonImageMapTypes::Specular;
            if (img->m_mapType == QDemonImageMapTypes::Bump) {
                bumpImage = img;
                bumpImageIdx = imageIdx;
            } else if (img->m_mapType == QDemonImageMapTypes::SpecularAmountMap) {
                specularAmountImage = img;
                specularAmountImageIdx = imageIdx;
            } else if (img->m_mapType == QDemonImageMapTypes::Roughness) {
                roughnessImage = img;
                roughnessImageIdx = imageIdx;
            } else if (img->m_mapType == QDemonImageMapTypes::Normal) {
                normalImage = img;
                normalImageIdx = imageIdx;
            } else if (img->m_image.m_mappingMode == ImageMappingModes::Environment) {
                hasEnvMap = true;
            } else if (img->m_mapType == QDemonImageMapTypes::Translucency) {
                translucencyImage = img;
                translucencyImageIdx = imageIdx;
            } else if (img->m_mapType == QDemonImageMapTypes::Emissive) {
                hasEmissiveMap = true;
            } else if (img->m_mapType == QDemonImageMapTypes::LightmapIndirect) {
                lightmapIndirectImage = img;
                lightmapIndirectImageIdx = imageIdx;
                hasLightmaps = true;
            } else if (img->m_mapType == QDemonImageMapTypes::LightmapRadiosity) {
                lightmapRadiosityImage = img;
                lightmapRadiosityImageIdx = imageIdx;
                hasLightmaps = true;
            } else if (img->m_mapType == QDemonImageMapTypes::LightmapShadow) {
                lightmapShadowImage = img;
                lightmapShadowImageIdx = imageIdx;
                hasLightmaps = true;
            }
        }

        bool enableFresnel = m_defaultMaterialShaderKeyProperties.m_fresnelEnabled.getValue(inKey);
        bool enableSSAO = false;
        bool enableSSDO = false;
        bool enableShadowMaps = false;
        bool enableBumpNormal = normalImage || bumpImage;

        for (qint32 idx = 0; idx < m_currentFeatureSet.size(); ++idx) {
            const auto &name = m_currentFeatureSet.at(idx).name;
            if (name == QStringLiteral("QDEMON_ENABLE_SSAO"))
                enableSSAO = m_currentFeatureSet[idx].enabled;
            else if (name == QStringLiteral("QDEMON_ENABLE_SSDO"))
                enableSSDO = m_currentFeatureSet[idx].enabled;
            else if (name == QStringLiteral("QDEMON_ENABLE_SSM"))
                enableShadowMaps = m_currentFeatureSet[idx].enabled;
        }

        bool includeSSAOSSDOVars = enableSSAO || enableSSDO || enableShadowMaps;

        vertexGenerator().beginFragmentGeneration();
        QDemonShaderStageGeneratorInterface &fragmentShader(fragmentGenerator());
        QDemonDefaultMaterialVertexPipelineInterface &vertexShader(vertexGenerator());

        // The fragment or vertex shaders may not use the material_properties or diffuse
        // uniforms in all cases but it is simpler to just add them and let the linker strip them.
        fragmentShader.addUniform("material_diffuse", "vec4");
        fragmentShader.addUniform("diffuse_color", "vec3");
        fragmentShader.addUniform("material_properties", "vec4");

        // All these are needed for SSAO
        if (includeSSAOSSDOVars) {
            fragmentShader.addInclude("SSAOCustomMaterial.glsllib");
            // fragmentShader.AddUniform( "ao_sampler", "sampler2D" );
        }

        if (hasIblProbe && hasLighting) {
            fragmentShader.addInclude("sampleProbe.glsllib");
        }

        if (hasLighting) {
            if (!m_lightsAsSeparateUniforms)
                addFunction(fragmentShader, "sampleLightVars");
            addFunction(fragmentShader, "diffuseReflectionBSDF");
        }

        if (hasLighting && hasLightmaps) {
            fragmentShader.addInclude("evalLightmaps.glsllib");
        }

        // view_vector, varWorldPos, world_normal are all used if there is a specular map
        // in addition to if there is specular lighting.  So they are lifted up here, always
        // generated.
        // we rely on the linker to strip out what isn't necessary instead of explicitly stripping
        // it for code simplicity.
        if (hasImage) {
            fragmentShader.append("\tvec3 uTransform;");
            fragmentShader.append("\tvec3 vTransform;");
        }

        if (includeSSAOSSDOVars || hasSpecMap || hasLighting || hasEnvMap || enableFresnel || hasIblProbe || enableBumpNormal) {
            vertexShader.generateViewVector();
            vertexShader.generateWorldNormal();
            vertexShader.generateWorldPosition();
        }
        if (includeSSAOSSDOVars || specularEnabled || hasIblProbe || enableBumpNormal)
            vertexShader.generateVarTangentAndBinormal();

        if (vertexColorsEnabled)
            vertexShader.generateVertexColor();
        else
            fragmentShader.append("\tvec3 vertColor = vec3(1.0);");

        // You do bump or normal mapping but not both
        if (bumpImage != nullptr) {
            generateImageUVCoordinates(bumpImageIdx, *bumpImage);
            fragmentShader.addUniform("bumpAmount", "float");

            fragmentShader.addUniform(m_imageSamplerSize, "vec2");
            fragmentShader.addInclude("defaultMaterialBumpNoLod.glsllib");
            fragmentShader << "\tworld_normal = defaultMaterialBumpNoLod( " << m_imageSampler << ", bumpAmount, "
                           << m_imageFragCoords << ", tangent, binormal, world_normal, " << m_imageSamplerSize << ");"
                           << "\n";
            // Do gram schmidt
            fragmentShader << "\tbinormal = normalize(cross(world_normal, tangent) );\n";
            fragmentShader << "\ttangent = normalize(cross(binormal, world_normal) );\n";

        } else if (normalImage != nullptr) {
            generateImageUVCoordinates(normalImageIdx, *normalImage);

            fragmentShader.addInclude("defaultMaterialFileNormalTexture.glsllib");
            fragmentShader.addUniform("bumpAmount", "float");

            fragmentShader << "\tworld_normal = defaultMaterialFileNormalTexture( " << m_imageSampler
                           << ", bumpAmount, " << m_imageFragCoords << ", tangent, binormal );"
                           << "\n";
        }

        if (includeSSAOSSDOVars || specularEnabled || hasIblProbe || enableBumpNormal)
            fragmentShader << "\tmat3 tanFrame = mat3(tangent, binormal, world_normal);"
                           << "\n";

        bool fragmentHasSpecularAmount = false;

        if (hasEmissiveMap)
            fragmentShader.append("\tvec3 global_emission = material_diffuse.rgb;");

        if (hasLighting) {
            fragmentShader.addUniform("light_ambient_total", "vec3");

            fragmentShader.append("\tvec4 global_diffuse_light = vec4(light_ambient_total.xyz, 1.0);");
            fragmentShader.append("\tvec3 global_specular_light = vec3(0.0, 0.0, 0.0);");
            fragmentShader.append("\tfloat shadow_map_occl = 1.0;");

            if (specularEnabled) {
                vertexShader.generateViewVector();
                fragmentShader.addUniform("material_properties", "vec4");
            }

            if (lightmapIndirectImage != nullptr) {
                generateImageUVCoordinates(lightmapIndirectImageIdx, *lightmapIndirectImage, 1);
                fragmentShader << "\tvec4 indirect_light = texture2D( " << m_imageSampler << ", " << m_imageFragCoords << ");"
                               << "\n";
                fragmentShader << "\tglobal_diffuse_light += indirect_light;"
                               << "\n";
                if (specularEnabled) {
                    fragmentShader << "\tglobal_specular_light += indirect_light.rgb * material_properties.x;"
                                   << "\n";
                }
            }

            if (lightmapRadiosityImage != nullptr) {
                generateImageUVCoordinates(lightmapRadiosityImageIdx, *lightmapRadiosityImage, 1);
                fragmentShader << "\tvec4 direct_light = texture2D( " << m_imageSampler << ", " << m_imageFragCoords << ");"
                               << "\n";
                fragmentShader << "\tglobal_diffuse_light += direct_light;"
                               << "\n";
                if (specularEnabled) {
                    fragmentShader << "\tglobal_specular_light += direct_light.rgb * material_properties.x;"
                                   << "\n";
                }
            }

            if (translucencyImage != nullptr) {
                fragmentShader.addUniform("translucentFalloff", "float");
                fragmentShader.addUniform("diffuseLightWrap", "float");

                generateImageUVCoordinates(translucencyImageIdx, *translucencyImage);

                fragmentShader << "\tvec4 translucent_depth_range = texture2D( " << m_imageSampler << ", "
                               << m_imageFragCoords << ");"
                               << "\n";
                fragmentShader << "\tfloat translucent_thickness = translucent_depth_range.r * "
                                  "translucent_depth_range.r;"
                               << "\n";
                fragmentShader << "\tfloat translucent_thickness_exp = exp( translucent_thickness "
                                  "* translucentFalloff);"
                               << "\n";
            }

            fragmentShader.append("\tfloat lightAttenuation = 1.0;");

            addLocalVariable(fragmentShader, "aoFactor", "float");

            if (hasLighting && enableSSAO)
                fragmentShader.append("\taoFactor = customMaterialAO();");
            else
                fragmentShader.append("\taoFactor = 1.0;");

            addLocalVariable(fragmentShader, "shadowFac", "float");

            if (specularEnabled) {
                fragmentShader << "\tfloat specularAmount = material_properties.x;"
                               << "\n";
                fragmentHasSpecularAmount = true;
            }
            // Fragment lighting means we can perhaps attenuate the specular amount by a texture
            // lookup.

            fragmentShader << "\tvec3 specularColor = vec3(1.0);"
                           << "\n";
            if (specularAmountImage) {
                if (!specularEnabled)
                    fragmentShader << "\tfloat specularAmount = 1.0;"
                                   << "\n";
                generateImageUVCoordinates(specularAmountImageIdx, *specularAmountImage);
                fragmentShader << "\tspecularColor = texture2D( " << m_imageSampler << ", " << m_imageFragCoords << " ).xyz;"
                               << "\n";
                fragmentHasSpecularAmount = true;
            }

            fragmentShader << "\tfloat roughnessAmount = material_properties.y;"
                           << "\n";
            if (roughnessImage) {
                generateImageUVCoordinates(roughnessImageIdx, *roughnessImage);
                fragmentShader << "\tfloat sampledRoughness = texture2D( " << m_imageSampler << ", " << m_imageFragCoords << " ).x;"
                               << "\n";
                // The roughness sampled from roughness textures is Disney roughness
                // which has to be squared to get the proper value
                fragmentShader << "\troughnessAmount = roughnessAmount * "
                               << "sampledRoughness * sampledRoughness;"
                               << "\n";
            }

            fragmentHasSpecularAmount = maybeAddMaterialFresnel(fragmentShader, inKey, fragmentHasSpecularAmount);

            // Iterate through all lights
            for (int lightIdx = 0; lightIdx < m_lights.size(); ++lightIdx) {
                QDemonRenderLight *lightNode = m_lights[lightIdx];
                setupLightVariableNames(lightIdx, *lightNode);
                bool isDirectional = lightNode->m_lightType == RenderLightTypes::Directional;
                bool isArea = lightNode->m_lightType == RenderLightTypes::Area;
                bool isShadow = enableShadowMaps && lightNode->m_castShadow;

                fragmentShader.append("");
                char buf[10];
                sprintf(buf, "%d", lightIdx);

                QByteArray tempStr = "light";
                tempStr.append(buf);

                fragmentShader << "\t//Light " << buf << "\n";
                fragmentShader << "\tlightAttenuation = 1.0;"
                               << "\n";
                if (isDirectional) {

                    if (m_lightsAsSeparateUniforms) {
                        fragmentShader.addUniform(m_lightDirection, "vec4");
                        fragmentShader.addUniform(m_lightColor, "vec4");
                    }

                    if (enableSSDO) {
                        fragmentShader << "\tshadowFac = customMaterialShadow( " << m_lightDirection << ".xyz, varWorldPos );"
                                       << "\n";
                    } else {
                        fragmentShader << "\tshadowFac = 1.0;"
                                       << "\n";
                    }

                    generateShadowMapOcclusion(lightIdx, enableShadowMaps && isShadow, lightNode->m_lightType);

                    if (specularEnabled && enableShadowMaps && isShadow)
                        fragmentShader << "\tlightAttenuation *= shadow_map_occl;"
                                       << "\n";

                    fragmentShader << "\tglobal_diffuse_light.rgb += shadowFac * shadow_map_occl * "
                                      "diffuseReflectionBSDF( world_normal, "
                                   << "-" << m_lightDirection << ".xyz, view_vector, " << m_lightColor << ".rgb, 0.0 ).rgb;"
                                   << "\n";

                    if (specularEnabled) {
                        if (m_lightsAsSeparateUniforms)
                            fragmentShader.addUniform(m_lightSpecularColor, "vec4");
                        outputSpecularEquation(material()->specularModel, fragmentShader, m_lightDirection, m_lightSpecularColor);
                    }
                } else if (isArea) {
                    if (m_lightsAsSeparateUniforms) {
                        fragmentShader.addUniform(m_lightColor, "vec4");
                        fragmentShader.addUniform(m_lightPos, "vec4");
                        fragmentShader.addUniform(m_lightDirection, "vec4");
                        fragmentShader.addUniform(m_lightUp, "vec4");
                        fragmentShader.addUniform(m_lightRt, "vec4");
                    } else {
                        addFunction(fragmentShader, "areaLightVars");
                    }
                    addFunction(fragmentShader, "calculateDiffuseAreaOld");
                    vertexShader.generateWorldPosition();
                    generateShadowMapOcclusion(lightIdx, enableShadowMaps && isShadow, lightNode->m_lightType);

                    // Debug measure to make sure paraboloid sampling was projecting to the right
                    // location
                    // fragmentShader << "\tglobal_diffuse_light.rg += " << m_ShadowCoordStem << ";"
                    // << "\n";
                    m_normalizedDirection = tempStr;
                    m_normalizedDirection.append("_Frame");

                    addLocalVariable(fragmentShader, m_normalizedDirection, "mat3");
                    fragmentShader << m_normalizedDirection << " = mat3( " << m_lightRt << ".xyz, " << m_lightUp
                                   << ".xyz, -" << m_lightDirection << ".xyz );"
                                   << "\n";

                    if (enableSSDO) {
                        fragmentShader << "\tshadowFac = shadow_map_occl * customMaterialShadow( " << m_lightDirection
                                       << ".xyz, varWorldPos );"
                                       << "\n";
                    } else {
                        fragmentShader << "\tshadowFac = shadow_map_occl;"
                                       << "\n";
                    }

                    if (specularEnabled) {
                        vertexShader.generateViewVector();
                        if (m_lightsAsSeparateUniforms)
                            fragmentShader.addUniform(m_lightSpecularColor, "vec4");
                        OutputSpecularAreaLighting(fragmentShader, "varWorldPos", "view_vector", m_lightSpecularColor);
                    }

                    OutputDiffuseAreaLighting(fragmentShader, "varWorldPos", tempStr);
                    fragmentShader << "\tlightAttenuation *= shadowFac;"
                                   << "\n";

                    addTranslucencyIrradiance(fragmentShader, translucencyImage, true);

                    fragmentShader << "\tglobal_diffuse_light.rgb += lightAttenuation * "
                                      "diffuseReflectionBSDF( world_normal, "
                                   << m_normalizedDirection << ", view_vector, " << m_lightColor << ".rgb, 0.0 ).rgb;"
                                   << "\n";
                } else {

                    vertexShader.generateWorldPosition();
                    generateShadowMapOcclusion(lightIdx, enableShadowMaps && isShadow, lightNode->m_lightType);

                    if (m_lightsAsSeparateUniforms) {
                        fragmentShader.addUniform(m_lightColor, "vec4");
                        fragmentShader.addUniform(m_lightPos, "vec4");
                    }

                    m_relativeDirection = tempStr;
                    m_relativeDirection.append("_relativeDirection");

                    m_normalizedDirection = m_relativeDirection;
                    m_normalizedDirection.append("_normalized");

                    m_relativeDistance = tempStr;
                    m_relativeDistance.append("_distance");

                    fragmentShader << "\tvec3 " << m_relativeDirection << " = varWorldPos - " << m_lightPos << ".xyz;"
                                   << "\n";
                    fragmentShader << "\tfloat " << m_relativeDistance << " = length( " << m_relativeDirection << " );"
                                   << "\n";
                    fragmentShader << "\tvec3 " << m_normalizedDirection << " = " << m_relativeDirection << " / "
                                   << m_relativeDistance << ";"
                                   << "\n";

                    if (enableSSDO) {
                        fragmentShader << "\tshadowFac = shadow_map_occl * customMaterialShadow( "
                                       << m_normalizedDirection << ", varWorldPos );"
                                       << "\n";
                    } else {
                        fragmentShader << "\tshadowFac = shadow_map_occl;"
                                       << "\n";
                    }

                    addFunction(fragmentShader, "calculatePointLightAttenuation");

                    if (m_lightsAsSeparateUniforms) {
                        fragmentShader.addUniform(m_lightAttenuation, "vec3");
                        fragmentShader << "\tlightAttenuation = shadowFac * calculatePointLightAttenuation("
                                       << "vec3( " << m_lightAttenuation << ".x, " << m_lightAttenuation << ".y, "
                                       << m_lightAttenuation << ".z), " << m_relativeDistance << ");"
                                       << "\n";
                    } else {
                        fragmentShader << "\tlightAttenuation = shadowFac * calculatePointLightAttenuation("
                                       << "vec3( " << m_lightConstantAttenuation << ", " << m_lightLinearAttenuation
                                       << ", " << m_lightQuadraticAttenuation << "), " << m_relativeDistance << ");"
                                       << "\n";
                    }

                    addTranslucencyIrradiance(fragmentShader, translucencyImage, false);

                    fragmentShader << "\tglobal_diffuse_light.rgb += lightAttenuation * "
                                      "diffuseReflectionBSDF( world_normal, "
                                   << "-" << m_normalizedDirection << ", view_vector, " << m_lightColor << ".rgb, 0.0 ).rgb;"
                                   << "\n";

                    if (specularEnabled) {
                        if (m_lightsAsSeparateUniforms)
                            fragmentShader.addUniform(m_lightSpecularColor, "vec4");
                        outputSpecularEquation(material()->specularModel, fragmentShader, m_normalizedDirection, m_lightSpecularColor);
                    }
                }
            }

            // This may be confusing but the light colors are already modulated by the base
            // material color.
            // Thus material color is the base material color * material emissive.
            // Except material_color.a *is* the actual opacity factor.
            // Furthermore object_opacity is something that may come from the vertex pipeline or
            // somewhere else.
            // We leave it up to the vertex pipeline to figure it out.
            fragmentShader << "\tglobal_diffuse_light = vec4(global_diffuse_light.xyz * aoFactor, "
                              "object_opacity);"
                           << "\n"
                           << "\tglobal_specular_light = vec3(global_specular_light.xyz);"
                           << "\n";
        } else { // no lighting.
            fragmentShader << "\tvec4 global_diffuse_light = vec4(0.0, 0.0, 0.0, object_opacity);"
                           << "\n"
                           << "\tvec3 global_specular_light = vec3(0.0, 0.0, 0.0);"
                           << "\n";

            // We still have specular maps and such that could potentially use the fresnel variable.
            fragmentHasSpecularAmount = maybeAddMaterialFresnel(fragmentShader, inKey, fragmentHasSpecularAmount);
        }

        if (!hasEmissiveMap) {
            fragmentShader << "\tglobal_diffuse_light.rgb += diffuse_color.rgb * material_diffuse.rgb;"
                           << "\n";
        }

        // since we already modulate our material diffuse color
        // into the light color we will miss it entirely if no IBL
        // or light is used
        if (hasLightmaps && !(m_lights.size() || hasIblProbe))
            fragmentShader << "\tglobal_diffuse_light.rgb *= diffuse_color.rgb;"
                           << "\n";

        if (hasLighting && hasIblProbe) {
            vertexShader.generateWorldNormal();

            fragmentShader << "\tglobal_diffuse_light.rgb += diffuse_color.rgb * aoFactor * "
                              "sampleDiffuse( tanFrame ).xyz;"
                           << "\n";

            if (specularEnabled) {

                fragmentShader.addUniform("material_specular", "vec4");

                fragmentShader << "\tglobal_specular_light.xyz += specularAmount * specularColor * "
                                  "vec3(material_specular.xyz) * sampleGlossy( tanFrame, "
                                  "view_vector, roughnessAmount ).xyz;"
                               << "\n";
            }
        }

        if (hasImage) {
            fragmentShader.append("\tvec4 texture_color;");
            quint32 idx = 0;
            for (QDemonRenderableImage *image = m_firstImage; image; image = image->m_nextImage, ++idx) {
                // Various maps are handled on a different locations
                if (image->m_mapType == QDemonImageMapTypes::Bump || image->m_mapType == QDemonImageMapTypes::Normal
                    || image->m_mapType == QDemonImageMapTypes::Displacement || image->m_mapType == QDemonImageMapTypes::SpecularAmountMap
                    || image->m_mapType == QDemonImageMapTypes::Roughness || image->m_mapType == QDemonImageMapTypes::Translucency
                    || image->m_mapType == QDemonImageMapTypes::LightmapIndirect
                    || image->m_mapType == QDemonImageMapTypes::LightmapRadiosity) {
                    continue;
                }

                QByteArray texSwizzle;
                QByteArray lookupSwizzle;

                generateImageUVCoordinates(idx, *image, 0);

                generateTextureSwizzle(image->m_image.m_textureData.m_texture->textureSwizzleMode(), texSwizzle, lookupSwizzle);

                fragmentShader << "\ttexture_color" << texSwizzle << " = texture2D( " << m_imageSampler << ", "
                               << m_imageFragCoords << ")" << lookupSwizzle << ";"
                               << "\n";

                if (image->m_image.m_textureData.m_textureFlags.isPreMultiplied() == true)
                    fragmentShader << "\ttexture_color.rgb = texture_color.a > 0.0 ? "
                                      "texture_color.rgb / texture_color.a : vec3( 0, 0, 0 );"
                                   << "\n";

                // These mapping types honestly don't make a whole ton of sense to me.
                switch (image->m_mapType) {
                case QDemonImageMapTypes::Diffuse: // assume images are premultiplied.
                case QDemonImageMapTypes::LightmapShadow:
                    // We use image offsets.z to switch between incoming premultiplied textures or
                    // not premultiplied textures.
                    // If Z is 1, then we assume the incoming texture is already premultiplied, else
                    // we just read the rgb value.
                    fragmentShader.append("\tglobal_diffuse_light *= texture_color;");
                    break;
                case QDemonImageMapTypes::Specular:

                    fragmentShader.addUniform("material_specular", "vec4");
                    if (fragmentHasSpecularAmount) {
                        fragmentShader.append("\tglobal_specular_light.xyz += specularAmount * "
                                              "specularColor * texture_color.xyz * "
                                              "material_specular.xyz;");
                    } else {
                        fragmentShader.append("\tglobal_specular_light.xyz += texture_color.xyz * "
                                              "material_specular.xyz;");
                    }
                    fragmentShader.append("\tglobal_diffuse_light.a *= texture_color.a;");
                    break;
                case QDemonImageMapTypes::Opacity:
                    fragmentShader.append("\tglobal_diffuse_light.a *= texture_color.a;");
                    break;
                case QDemonImageMapTypes::Emissive:
                    fragmentShader.append("\tglobal_emission *= texture_color.xyz * texture_color.a;");
                    break;
                default:
                    Q_ASSERT(false); // fallthrough intentional
                }
            }
        }

        if (hasEmissiveMap)
            fragmentShader.append("\tglobal_diffuse_light.rgb += global_emission.rgb;");

        // Ensure the rgb colors are in range.
        fragmentShader.append("\tfragOutput = vec4( clamp( vertColor * global_diffuse_light.xyz + "
                              "global_specular_light.xyz, 0.0, 65519.0 ), global_diffuse_light.a "
                              ");");

        if (vertexGenerator().hasActiveWireframe()) {
            fragmentShader.append("vec3 edgeDistance = varEdgeDistance * gl_FragCoord.w;");
            fragmentShader.append("\tfloat d = min(min(edgeDistance.x, edgeDistance.y), edgeDistance.z);");
            fragmentShader.append("\tfloat mixVal = smoothstep(0.0, 1.0, d);"); // line width 1.0

            fragmentShader.append("\tfragOutput = mix( vec4(0.0, 1.0, 0.0, 1.0), fragOutput, mixVal);");
        }
    }

    QDemonRef<QDemonRenderShaderProgram> generateMaterialShader(const QByteArray &inShaderPrefix)
    {
        // build a string that allows us to print out the shader we are generating to the log.
        // This is time consuming but I feel like it doesn't happen all that often and is very
        // useful to users
        // looking at the log file.

        QByteArray generatedShaderString;
        generatedShaderString = inShaderPrefix;

        QDemonShaderDefaultMaterialKey theKey(key());
        theKey.toString(generatedShaderString, m_defaultMaterialShaderKeyProperties);

        m_lightsAsSeparateUniforms = !m_renderContext->getRenderContext()->supportsConstantBuffer();

        generateVertexShader();
        generateFragmentShader(theKey);

        vertexGenerator().endVertexGeneration();
        vertexGenerator().endFragmentGeneration();

        return programGenerator()->compileGeneratedShader(generatedShaderString, QDemonShaderCacheProgramFlags(), m_currentFeatureSet);
    }

    QDemonRef<QDemonRenderShaderProgram> generateShader(const QDemonGraphObject &inMaterial,
                                                        QDemonShaderDefaultMaterialKey inShaderDescription,
                                                        QDemonShaderStageGeneratorInterface &inVertexPipeline,
                                                        const TShaderFeatureSet &inFeatureSet,
                                                        const QVector<QDemonRenderLight *> &inLights,
                                                        QDemonRenderableImage *inFirstImage,
                                                        bool inHasTransparency,
                                                        const QByteArray &inVertexPipelineName,
                                                        const QByteArray &) override
    {
        Q_ASSERT(inMaterial.type == QDemonGraphObjectType::DefaultMaterial);
        m_currentMaterial = static_cast<const QDemonRenderDefaultMaterial *>(&inMaterial);
        m_currentKey = &inShaderDescription;
        m_currentPipeline = static_cast<QDemonDefaultMaterialVertexPipelineInterface *>(&inVertexPipeline);
        m_currentFeatureSet = inFeatureSet;
        m_lights = inLights;
        m_firstImage = inFirstImage;
        m_hasTransparency = inHasTransparency;

        return generateMaterialShader(inVertexPipelineName);
    }

    QDemonRef<QDemonShaderGeneratorGeneratedShader> getShaderForProgram(const QDemonRef<QDemonRenderShaderProgram> &inProgram)
    {
        auto inserter = m_programToShaderMap.constFind(inProgram);
        if (inserter == m_programToShaderMap.constEnd())
            inserter = m_programToShaderMap.insert(inProgram,
                                                   QDemonRef<QDemonShaderGeneratorGeneratedShader>(
                                                           new QDemonShaderGeneratorGeneratedShader(inProgram,
                                                                                                    m_renderContext->getRenderContext())));

        return inserter.value();
    }

    void setGlobalProperties(const QDemonRef<QDemonRenderShaderProgram> &inProgram,
                             const QDemonRenderLayer & /*inLayer*/,
                             QDemonRenderCamera &inCamera,
                             const QVector3D &inCameraDirection,
                             const QVector<QDemonRenderLight *> &inLights,
                             const QVector<QVector3D> &inLightDirections,
                             const QDemonRef<QDemonRenderShadowMap> &inShadowMapManager)
    {
        QDemonRef<QDemonShaderGeneratorGeneratedShader> shader(getShaderForProgram(inProgram));
        m_renderContext->getRenderContext()->setActiveShader(inProgram);

        m_shadowMapManager = inShadowMapManager;

        QDemonRenderCamera &theCamera(inCamera);
        shader->m_cameraPosition.set(theCamera.getGlobalPos());
        shader->m_cameraDirection.set(inCameraDirection);

        QMatrix4x4 viewProj;
        if (shader->m_viewProj.isValid()) {
            theCamera.calculateViewProjectionMatrix(viewProj);
            shader->m_viewProj.set(viewProj);
        }

        if (shader->m_viewMatrix.isValid()) {
            viewProj = mat44::getInverse(theCamera.globalTransform);
            shader->m_viewMatrix.set(viewProj);
        }

        // update the constant buffer
        shader->m_aoShadowParams.set();
        // We can't cache light properties because they can change per object.
        QVector3D theLightAmbientTotal = QVector3D(0, 0, 0);
        size_t numShaderLights = shader->m_lights.size();
        size_t numShadowLights = shader->m_shadowMaps.size();
        for (quint32 lightIdx = 0, shadowMapIdx = 0, lightEnd = inLights.size(); lightIdx < lightEnd && lightIdx < QDEMON_MAX_NUM_LIGHTS;
             ++lightIdx) {
            QDemonRenderLight *theLight(inLights[lightIdx]);
            if (lightIdx >= numShaderLights) {
                shader->m_lights.push_back(QDemonShaderLightProperties());
                ++numShaderLights;
            }
            if (shadowMapIdx >= numShadowLights && numShadowLights < QDEMON_MAX_NUM_SHADOWS) {
                if (theLight->m_scope == nullptr && theLight->m_castShadow) {
                    // PKC TODO : Fix multiple shadow issues.
                    // Need to know when the list of lights changes order, and clear shadow maps
                    // when that happens.
                    setupShadowMapVariableNames(lightIdx);
                    shader->m_shadowMaps.push_back(
                            QDemonShadowMapProperties(m_shadowMapStem, m_shadowCubeStem, m_shadowMatrixStem, m_shadowControlStem, inProgram));
                }
            }
            Q_ASSERT(lightIdx < numShaderLights);
            QDemonShaderLightProperties &theLightProperties(shader->m_lights[lightIdx]);
            float brightness = translateConstantAttenuation(theLight->m_brightness);

            // setup light data
            theLightProperties.lightColor = theLight->m_diffuseColor * brightness;
            theLightProperties.lightData.specular = QVector4D(theLight->m_specularColor * brightness, 1.0);
            theLightProperties.lightData.direction = QVector4D(inLightDirections[lightIdx], 1.0);

            // TODO : This does potentially mean that we can create more shadow map entries than
            // we can actually use at once.
            if ((theLight->m_scope == nullptr) && (theLight->m_castShadow && inShadowMapManager)) {
                QDemonShadowMapProperties &theShadowMapProperties(shader->m_shadowMaps[shadowMapIdx++]);
                QDemonShadowMapEntry *pEntry = inShadowMapManager->getShadowMapEntry(lightIdx);
                if (pEntry) {
                    // add fixed scale bias matrix
                    QMatrix4x4 bias = { 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0 };

                    if (theLight->m_lightType != RenderLightTypes::Directional) {
                        theShadowMapProperties.m_shadowCubeTexture.set(pEntry->m_depthCube.data());
                        theShadowMapProperties.m_shadowmapMatrix.set(pEntry->m_lightView);
                    } else {
                        theShadowMapProperties.m_shadowmapTexture.set(pEntry->m_depthMap.data());
                        theShadowMapProperties.m_shadowmapMatrix.set(bias * pEntry->m_lightVP);
                    }

                    theShadowMapProperties.m_shadowmapSettings.set(
                            QVector4D(theLight->m_shadowBias, theLight->m_shadowFactor, theLight->m_shadowMapFar, 0.0f));
                } else {
                    // if we have a light casting shadow we should find an entry
                    Q_ASSERT(false);
                }
            }

            if (theLight->m_lightType == RenderLightTypes::Point) {
                theLightProperties.lightData.position = QVector4D(theLight->getGlobalPos(), 1.0);
                theLightProperties.lightData.constantAttenuation = 1.0;
                theLightProperties.lightData.linearAttenuation = translateLinearAttenuation(theLight->m_linearFade);
                theLightProperties.lightData.quadraticAttenuation = translateQuadraticAttenuation(theLight->m_exponentialFade);
            } else if (theLight->m_lightType == RenderLightTypes::Area) {
                theLightProperties.lightData.position = QVector4D(theLight->getGlobalPos(), 1.0);

                QVector3D upDir = mat33::transform(mat44::getUpper3x3(theLight->globalTransform), QVector3D(0, 1, 0));
                QVector3D rtDir = mat33::transform(mat44::getUpper3x3(theLight->globalTransform), QVector3D(1, 0, 0));

                theLightProperties.lightData.up = QVector4D(upDir, theLight->m_areaHeight);
                theLightProperties.lightData.right = QVector4D(rtDir, theLight->m_areaWidth);
            }
            theLightAmbientTotal += theLight->m_ambientColor;
        }
        shader->m_lightAmbientTotal = theLightAmbientTotal;
    }

    // Also sets the blend function on the render context.
    void setMaterialProperties(const QDemonRef<QDemonRenderShaderProgram> &inProgram,
                               const QDemonRenderDefaultMaterial &inMaterial,
                               const QVector2D &inCameraVec,
                               const QMatrix4x4 &inModelViewProjection,
                               const QMatrix3x3 &inNormalMatrix,
                               const QMatrix4x4 &inGlobalTransform,
                               QDemonRenderableImage *inFirstImage,
                               float inOpacity,
                               const QDemonRef<QDemonRenderTexture2D> &inDepthTexture,
                               const QDemonRef<QDemonRenderTexture2D> &inSSaoTexture,
                               QDemonRenderImage *inLightProbe,
                               QDemonRenderImage *inLightProbe2,
                               float inProbeHorizon,
                               float inProbeBright,
                               float inProbe2Window,
                               float inProbe2Pos,
                               float inProbe2Fade,
                               float inProbeFOV)
    {

        QDemonRef<QDemonRenderContext> context(m_renderContext->getRenderContext());
        QDemonRef<QDemonShaderGeneratorGeneratedShader> shader(getShaderForProgram(inProgram));
        shader->m_mvp.set(inModelViewProjection);
        shader->m_normalMatrix.set(inNormalMatrix);
        shader->m_globalTransform.set(inGlobalTransform);
        shader->m_depthTexture.set(inDepthTexture.data());

        shader->m_aoTexture.set(inSSaoTexture.data());

        QDemonRenderImage *theLightProbe = inLightProbe;
        QDemonRenderImage *theLightProbe2 = inLightProbe2;

        // If the material has its own IBL Override, we should use that image instead.
        if ((inMaterial.iblProbe) && (inMaterial.iblProbe->m_textureData.m_texture)) {
            theLightProbe = inMaterial.iblProbe;
        }

        if (theLightProbe) {
            if (theLightProbe->m_textureData.m_texture) {
                QDemonRenderTextureCoordOp theHorzLightProbeTilingMode = QDemonRenderTextureCoordOp::Repeat;
                QDemonRenderTextureCoordOp theVertLightProbeTilingMode = theLightProbe->m_verticalTilingMode;
                theLightProbe->m_textureData.m_texture->setTextureWrapS(theHorzLightProbeTilingMode);
                theLightProbe->m_textureData.m_texture->setTextureWrapT(theVertLightProbeTilingMode);
                const QMatrix4x4 &textureTransform = theLightProbe->m_textureTransform;
                // We separate rotational information from offset information so that just maybe the
                // shader
                // will attempt to push less information to the card.
                const float *dataPtr(textureTransform.constData());
                // The third member of the offsets contains a flag indicating if the texture was
                // premultiplied or not.
                // We use this to mix the texture alpha.
                QVector4D offsets(dataPtr[12],
                                  dataPtr[13],
                                  theLightProbe->m_textureData.m_textureFlags.isPreMultiplied() ? 1.0f : 0.0f,
                                  (float)theLightProbe->m_textureData.m_texture->numMipmaps());

                // Grab just the upper 2x2 rotation matrix from the larger matrix.
                QVector4D rotations(dataPtr[0], dataPtr[4], dataPtr[1], dataPtr[5]);

                shader->m_lightProbeRot.set(rotations);
                shader->m_lightProbeOfs.set(offsets);

                if ((!inMaterial.iblProbe) && (inProbeFOV < 180.f)) {
                    shader->m_lightProbeOpts.set(QVector4D(0.01745329251994329547f * inProbeFOV, 0.0f, 0.0f, 0.0f));
                }

                // Also make sure to add the secondary texture, but it should only be added if the
                // primary
                // (i.e. background) texture is also there.
                if (theLightProbe2 && theLightProbe2->m_textureData.m_texture) {
                    theLightProbe2->m_textureData.m_texture->setTextureWrapS(theHorzLightProbeTilingMode);
                    theLightProbe2->m_textureData.m_texture->setTextureWrapT(theVertLightProbeTilingMode);
                    shader->m_lightProbe2.set(theLightProbe2->m_textureData.m_texture.data());
                    shader->m_lightProbe2Props.set(QVector4D(inProbe2Window, inProbe2Pos, inProbe2Fade, 1.0f));

                    const QMatrix4x4 &xform2 = theLightProbe2->m_textureTransform;
                    const float *dataPtr(xform2.constData());
                    shader->m_lightProbeProps.set(QVector4D(dataPtr[12], dataPtr[13], inProbeHorizon, inProbeBright * 0.01f));
                } else {
                    shader->m_lightProbe2Props.set(QVector4D(0.0f, 0.0f, 0.0f, 0.0f));
                    shader->m_lightProbeProps.set(QVector4D(0.0f, 0.0f, inProbeHorizon, inProbeBright * 0.01f));
                }
                QDemonRef<QDemonRenderTexture2D> textureImage = theLightProbe->m_textureData.m_texture;
                shader->m_lightProbe.set(textureImage.data());
                shader->m_lightProbeSize.set(
                        QVector2D(textureImage->textureDetails().width, textureImage->textureDetails().height));
            } else {
                shader->m_lightProbeProps.set(QVector4D(0.0f, 0.0f, -1.0f, 0.0f));
                shader->m_lightProbe2Props.set(QVector4D(0.0f, 0.0f, 0.0f, 0.0f));
            }
        } else {
            shader->m_lightProbeProps.set(QVector4D(0.0f, 0.0f, -1.0f, 0.0f));
            shader->m_lightProbe2Props.set(QVector4D(0.0f, 0.0f, 0.0f, 0.0f));
        }

        float emissivePower = 1.0;

        quint32 hasLighting = inMaterial.lighting != DefaultMaterialLighting::NoLighting;
        if (hasLighting)
            emissivePower = inMaterial.emissivePower / 100.0f;

        QVector4D material_diffuse = QVector4D(inMaterial.emissiveColor[0] * emissivePower,
                                               inMaterial.emissiveColor[1] * emissivePower,
                                               inMaterial.emissiveColor[2] * emissivePower,
                                               inOpacity);
        shader->m_materialDiffuse.set(material_diffuse);
        shader->m_diffuseColor.set(inMaterial.diffuseColor);
        QVector4D material_specular = QVector4D(inMaterial.specularTint[0],
                                                inMaterial.specularTint[1],
                                                inMaterial.specularTint[2],
                                                inMaterial.ior);
        shader->m_materialSpecular.set(material_specular);
        shader->m_cameraProperties.set(inCameraVec);
        shader->m_fresnelPower.set(inMaterial.fresnelPower);

        if (context->supportsConstantBuffer()) {
            QDemonRef<QDemonRenderConstantBuffer> pLightCb = getLightConstantBuffer(shader->m_lights.size());
            // if we have lights we need a light buffer
            Q_ASSERT(shader->m_lights.size() == 0 || pLightCb);

            for (quint32 idx = 0, end = shader->m_lights.size(); idx < end && pLightCb; ++idx) {
                shader->m_lights[idx]
                        .lightData.diffuse = QVector4D(shader->m_lights[idx].lightColor.x() * inMaterial.diffuseColor.x(),
                                                       shader->m_lights[idx].lightColor.y() * inMaterial.diffuseColor.y(),
                                                       shader->m_lights[idx].lightColor.z() * inMaterial.diffuseColor.z(),
                                                       1.0);

                // this is our final change update memory
                pLightCb->updateRaw(idx * sizeof(QDemonLightSourceShader) + (4 * sizeof(qint32)),
                                    QDemonDataRef<quint8>((quint8 *)&shader->m_lights[idx].lightData, sizeof(QDemonLightSourceShader)));
            }
            // update light buffer to hardware
            if (pLightCb) {
                qint32 cgLights = shader->m_lights.size();
                pLightCb->updateRaw(0, QDemonDataRef<quint8>((quint8 *)&cgLights, sizeof(qint32)));
                shader->m_lightsBuffer.set();
            }
        } else {
            QDemonLightConstantProperties<QDemonShaderGeneratorGeneratedShader> *pLightConstants = getLightConstantProperties(shader);

            // if we have lights we need a light buffer
            Q_ASSERT(shader->m_lights.size() == 0 || pLightConstants);

            for (quint32 idx = 0, end = shader->m_lights.size(); idx < end && pLightConstants; ++idx) {
                shader->m_lights[idx]
                        .lightData.diffuse = QVector4D(shader->m_lights[idx].lightColor.x() * inMaterial.diffuseColor.x(),
                                                       shader->m_lights[idx].lightColor.y() * inMaterial.diffuseColor.y(),
                                                       shader->m_lights[idx].lightColor.z() * inMaterial.diffuseColor.z(),
                                                       1.0);
            }
            // update light buffer to hardware
            if (pLightConstants)
                pLightConstants->updateLights(shader);
        }

        shader->m_materialDiffuseLightAmbientTotal.set(
                QVector3D(shader->m_lightAmbientTotal.x() * inMaterial.diffuseColor[0],
                          shader->m_lightAmbientTotal.y() * inMaterial.diffuseColor[1],
                          shader->m_lightAmbientTotal.z() * inMaterial.diffuseColor[2]));

        shader->m_materialProperties.set(QVector4D(inMaterial.specularAmount, inMaterial.specularRoughness, emissivePower, 0.0f));
        shader->m_bumpAmount.set(inMaterial.bumpAmount);
        shader->m_displaceAmount.set(inMaterial.displaceAmount);
        shader->m_translucentFalloff.set(inMaterial.translucentFalloff);
        shader->m_diffuseLightWrap.set(inMaterial.diffuseLightWrap);
        quint32 imageIdx = 0;
        for (QDemonRenderableImage *theImage = inFirstImage; theImage; theImage = theImage->m_nextImage, ++imageIdx)
            setImageShaderVariables(shader, *theImage, imageIdx);

        QDemonRenderBlendFunctionArgument blendFunc;
        QDemonRenderBlendEquationArgument blendEqua(QDemonRenderBlendEquation::Add, QDemonRenderBlendEquation::Add);
        // The blend function goes:
        // src op
        // dst op
        // src alpha op
        // dst alpha op
        // All of our shaders produce non-premultiplied values.
        switch (inMaterial.blendMode) {
        case DefaultMaterialBlendMode::Screen:
            blendFunc = QDemonRenderBlendFunctionArgument(QDemonRenderSrcBlendFunc::SrcAlpha,
                                                          QDemonRenderDstBlendFunc::One,
                                                          QDemonRenderSrcBlendFunc::One,
                                                          QDemonRenderDstBlendFunc::One);
            break;
        case DefaultMaterialBlendMode::Multiply:
            blendFunc = QDemonRenderBlendFunctionArgument(QDemonRenderSrcBlendFunc::DstColor,
                                                          QDemonRenderDstBlendFunc::Zero,
                                                          QDemonRenderSrcBlendFunc::One,
                                                          QDemonRenderDstBlendFunc::One);
            break;
        case DefaultMaterialBlendMode::Overlay:
            // SW fallback is not using blend equation
            // note blend func is not used here anymore
            if (context->supportsAdvancedBlendHW() || context->supportsAdvancedBlendHwKHR())
                blendEqua = QDemonRenderBlendEquationArgument(QDemonRenderBlendEquation::Overlay, QDemonRenderBlendEquation::Overlay);
            break;
        case DefaultMaterialBlendMode::ColorBurn:
            // SW fallback is not using blend equation
            // note blend func is not used here anymore
            if (context->supportsAdvancedBlendHW() || context->supportsAdvancedBlendHwKHR())
                blendEqua = QDemonRenderBlendEquationArgument(QDemonRenderBlendEquation::ColorBurn,
                                                              QDemonRenderBlendEquation::ColorBurn);
            break;
        case DefaultMaterialBlendMode::ColorDodge:
            // SW fallback is not using blend equation
            // note blend func is not used here anymore
            if (context->supportsAdvancedBlendHW() || context->supportsAdvancedBlendHwKHR())
                blendEqua = QDemonRenderBlendEquationArgument(QDemonRenderBlendEquation::ColorDodge,
                                                              QDemonRenderBlendEquation::ColorDodge);
            break;
        default:
            blendFunc = QDemonRenderBlendFunctionArgument(QDemonRenderSrcBlendFunc::SrcAlpha,
                                                          QDemonRenderDstBlendFunc::OneMinusSrcAlpha,
                                                          QDemonRenderSrcBlendFunc::One,
                                                          QDemonRenderDstBlendFunc::OneMinusSrcAlpha);
            break;
        }
        context->setBlendFunction(blendFunc);
        context->setBlendEquation(blendEqua);
    }
    void setMaterialProperties(const QDemonRef<QDemonRenderShaderProgram> &inProgram,
                               const QDemonGraphObject &inMaterial,
                               const QVector2D &inCameraVec,
                               const QMatrix4x4 &inModelViewProjection,
                               const QMatrix3x3 &inNormalMatrix,
                               const QMatrix4x4 &inGlobalTransform,
                               QDemonRenderableImage *inFirstImage,
                               float inOpacity,
                               const QDemonLayerGlobalRenderProperties &inRenderProperties) override
    {
        const QDemonRenderDefaultMaterial &theMaterial(static_cast<const QDemonRenderDefaultMaterial &>(inMaterial));
        Q_ASSERT(inMaterial.type == QDemonGraphObjectType::DefaultMaterial);

        setGlobalProperties(inProgram,
                            inRenderProperties.layer,
                            inRenderProperties.camera,
                            inRenderProperties.cameraDirection,
                            inRenderProperties.lights,
                            inRenderProperties.lightDirections,
                            inRenderProperties.shadowMapManager);
        setMaterialProperties(inProgram,
                              theMaterial,
                              inCameraVec,
                              inModelViewProjection,
                              inNormalMatrix,
                              inGlobalTransform,
                              inFirstImage,
                              inOpacity,
                              inRenderProperties.depthTexture,
                              inRenderProperties.ssaoTexture,
                              inRenderProperties.lightProbe,
                              inRenderProperties.lightProbe2,
                              inRenderProperties.probeHorizon,
                              inRenderProperties.probeBright,
                              inRenderProperties.probe2Window,
                              inRenderProperties.probe2Pos,
                              inRenderProperties.probe2Fade,
                              inRenderProperties.probeFOV);
    }

    QDemonLightConstantProperties<QDemonShaderGeneratorGeneratedShader> *getLightConstantProperties(
            const QDemonRef<QDemonShaderGeneratorGeneratedShader> &shader)
    {
        if (!shader->m_lightConstantProperties
            || int(shader->m_lights.size()) > shader->m_lightConstantProperties->m_constants.size()) {
            if (shader->m_lightConstantProperties)
                delete shader->m_lightConstantProperties;
            shader->m_lightConstantProperties = new QDemonLightConstantProperties<QDemonShaderGeneratorGeneratedShader>(shader.data(), m_lightsAsSeparateUniforms);
        }
        return shader->m_lightConstantProperties;
    }
};
}

QDemonRef<QDemonDefaultMaterialShaderGeneratorInterface> QDemonDefaultMaterialShaderGeneratorInterface::createDefaultMaterialShaderGenerator(
        QDemonRenderContextInterface *inRc)
{
    return QDemonRef<QDemonDefaultMaterialShaderGeneratorInterface>(new QDemonShaderGenerator(inRc));
}

QDemonDefaultMaterialVertexPipelineInterface::~QDemonDefaultMaterialVertexPipelineInterface() = default;

QT_END_NAMESPACE
