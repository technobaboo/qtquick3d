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
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterialshadergenerator.h>
#include <Qt3DSAtomic.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRuntimeRender/qdemonrendershadercodegeneratorv2.h>
#include <QtDemonRuntimeRender/qdemonrenderableimage.h>
#include <QtDemonRuntimeRender/qdemonrenderimage.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRuntimeRender/qdemonrenderlight.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>
#include <QtDemonRuntimeRender/qdemonrendercamera.h>
#include <QtDemonRuntimeRender/qdemonrendershadowmap.h>
#include <QtDemonRuntimeRender/qdemonrendercustommaterial.h>
#include <QtDemonRuntimeRender/qdemonrenderdynamicobjectsystem.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>
#include <QtDemonRuntimeRender/qdemonrenderlightconstantproperties.h>


QT_BEGIN_NAMESPACE

using NVRenderCachedShaderProperty;
using NVRenderCachedShaderBuffer;

namespace {

const float MINATTENUATION = 0;
const float MAXATTENUATION = 1000;

float ClampFloat(float value, float min, float max)
{
    return value < min ? min : ((value > max) ? max : value);
}

float TranslateConstantAttenuation(float attenuation)
{
    return attenuation * .01f;
}

float TranslateLinearAttenuation(float attenuation)
{
    attenuation = ClampFloat(attenuation, MINATTENUATION, MAXATTENUATION);
    return attenuation * 0.0001f;
}

float TranslateQuadraticAttenuation(float attenuation)
{
    attenuation = ClampFloat(attenuation, MINATTENUATION, MAXATTENUATION);
    return attenuation * 0.0000001f;
}

/**
 *	Cached texture property lookups, used one per texture so a shader generator for N
 *	textures will have an array of N of these lookup objects.
 */
struct SShaderTextureProperties
{
    NVRenderCachedShaderProperty<QDemonRenderTexture2D *> m_Sampler;
    NVRenderCachedShaderProperty<QVector3D> m_Offsets;
    NVRenderCachedShaderProperty<QVector4D> m_Rotations;
    NVRenderCachedShaderProperty<QVector2D> m_Size;
    SShaderTextureProperties(const char *sampName, const char *offName, const char *rotName,
                             const char *sizeName,
                             QDemonRenderShaderProgram &inShader)
        : m_Sampler(sampName, inShader)
        , m_Offsets(offName, inShader)
        , m_Rotations(rotName, inShader)
        , m_Size(sizeName, inShader)
    {
    }
    SShaderTextureProperties() {}
};

/**
 *	Cached light property lookups, used one per light so a shader generator for N
 *	lights will have an array of N of these lookup objects.
 */
struct SShaderLightProperties
{
    // Color of the light
    QVector3D m_LightColor;
    SLightSourceShader m_LightData;

    SShaderLightProperties() {}
};

struct SShadowMapProperties
{
    NVRenderCachedShaderProperty<QDemonRenderTexture2D *> m_ShadowmapTexture; ///< shadow texture
    NVRenderCachedShaderProperty<QDemonRenderTextureCube *> m_ShadowCubeTexture; ///< shadow cubemap
    NVRenderCachedShaderProperty<QMatrix4x4>
    m_ShadowmapMatrix; ///< world to ligh space transform matrix
    NVRenderCachedShaderProperty<QVector4D> m_ShadowmapSettings; ///< shadow rendering settings

    SShadowMapProperties() {}
    SShadowMapProperties(const char *shadowmapTextureName, const char *shadowcubeTextureName,
                         const char *shadowmapMatrixName, const char *shadowmapSettingsName,
                         QDemonRenderShaderProgram &inShader)
        : m_ShadowmapTexture(shadowmapTextureName, inShader)
        , m_ShadowCubeTexture(shadowcubeTextureName, inShader)
        , m_ShadowmapMatrix(shadowmapMatrixName, inShader)
        , m_ShadowmapSettings(shadowmapSettingsName, inShader)
    {
    }
};

/**
 *	The results of generating a shader.  Caches all possible variable names into
 *	typesafe objects.
 */
struct SShaderGeneratorGeneratedShader
{
    QDemonRenderShaderProgram &m_Shader;
    // Specific properties we know the shader has to have.
    NVRenderCachedShaderProperty<QMatrix4x4> m_MVP;
    NVRenderCachedShaderProperty<QMatrix3x3> m_NormalMatrix;
    NVRenderCachedShaderProperty<QMatrix4x4> m_GlobalTransform;
    NVRenderCachedShaderProperty<QMatrix4x4> m_ViewProj;
    NVRenderCachedShaderProperty<QMatrix4x4> m_ViewMatrix;
    NVRenderCachedShaderProperty<QVector4D> m_MaterialDiffuse;
    NVRenderCachedShaderProperty<QVector4D> m_MaterialProperties;
    // tint, ior
    NVRenderCachedShaderProperty<QVector4D> m_MaterialSpecular;
    NVRenderCachedShaderProperty<float> m_BumpAmount;
    NVRenderCachedShaderProperty<float> m_DisplaceAmount;
    NVRenderCachedShaderProperty<float> m_TranslucentFalloff;
    NVRenderCachedShaderProperty<float> m_DiffuseLightWrap;
    NVRenderCachedShaderProperty<float> m_FresnelPower;
    NVRenderCachedShaderProperty<QVector3D> m_DiffuseColor;
    NVRenderCachedShaderProperty<QVector3D> m_CameraPosition;
    NVRenderCachedShaderProperty<QVector3D> m_CameraDirection;
    QVector3D m_LightAmbientTotal;
    NVRenderCachedShaderProperty<QVector3D> m_MaterialDiffuseLightAmbientTotal;
    NVRenderCachedShaderProperty<QVector2D> m_CameraProperties;

    NVRenderCachedShaderProperty<QDemonRenderTexture2D *> m_DepthTexture;
    NVRenderCachedShaderProperty<QDemonRenderTexture2D *> m_AOTexture;
    NVRenderCachedShaderProperty<QDemonRenderTexture2D *> m_LightProbe;
    NVRenderCachedShaderProperty<QVector4D> m_LightProbeProps;
    NVRenderCachedShaderProperty<QVector4D> m_LightProbeOpts;
    NVRenderCachedShaderProperty<QVector4D> m_LightProbeRot;
    NVRenderCachedShaderProperty<QVector4D> m_LightProbeOfs;
    NVRenderCachedShaderProperty<QVector2D> m_LightProbeSize;
    NVRenderCachedShaderProperty<QDemonRenderTexture2D *> m_LightProbe2;
    NVRenderCachedShaderProperty<QVector4D> m_LightProbe2Props;
    NVRenderCachedShaderProperty<QVector2D> m_LightProbe2Size;

    NVRenderCachedShaderBuffer<QDemonRenderShaderConstantBuffer *> m_AoShadowParams;
    NVRenderCachedShaderBuffer<QDemonRenderShaderConstantBuffer *> m_LightsBuffer;

    SLightConstantProperties<SShaderGeneratorGeneratedShader> *m_lightConstantProperties;

    // Cache the image property name lookups
    QVector<SShaderTextureProperties> m_Images;
    QVector<SShaderLightProperties> m_Lights;
    // Cache shadow map properties
    QVector<SShadowMapProperties> m_ShadowMaps;

    qint32 m_RefCount;

    SShaderGeneratorGeneratedShader(QDemonRenderShaderProgram &inShader, QDemonRenderContext &inContext)
        : m_Allocator(inContext.GetAllocator())
        , m_Shader(inShader)
        , m_MVP("model_view_projection", inShader)
        , m_NormalMatrix("normal_matrix", inShader)
        , m_GlobalTransform("model_matrix", inShader)
        , m_ViewProj("view_projection_matrix", inShader)
        , m_ViewMatrix("view_matrix", inShader)
        , m_MaterialDiffuse("material_diffuse", inShader)
        , m_MaterialProperties("material_properties", inShader)
        , m_MaterialSpecular("material_specular", inShader)
        , m_BumpAmount("bumpAmount", inShader)
        , m_DisplaceAmount("displaceAmount", inShader)
        , m_TranslucentFalloff("translucentFalloff", inShader)
        , m_DiffuseLightWrap("diffuseLightWrap", inShader)
        , m_FresnelPower("fresnelPower", inShader)
        , m_DiffuseColor("diffuse_color", inShader)
        , m_CameraPosition("camera_position", inShader)
        , m_CameraDirection("camera_direction", inShader)
        , m_MaterialDiffuseLightAmbientTotal("light_ambient_total", inShader)
        , m_CameraProperties("camera_properties", inShader)
        , m_DepthTexture("depth_sampler", inShader)
        , m_AOTexture("ao_sampler", inShader)
        , m_LightProbe("light_probe", inShader)
        , m_LightProbeProps("light_probe_props", inShader)
        , m_LightProbeOpts("light_probe_opts", inShader)
        , m_LightProbeRot("light_probe_rotation", inShader)
        , m_LightProbeOfs("light_probe_offset", inShader)
        , m_LightProbeSize("light_probe_size", inShader)
        , m_LightProbe2("light_probe2", inShader)
        , m_LightProbe2Props("light_probe2_props", inShader)
        , m_LightProbe2Size("light_probe2_size", inShader)
        , m_AoShadowParams("cbAoShadow", inShader)
        , m_LightsBuffer("cbBufferLights", inShader)
        , m_lightConstantProperties(nullptr)
        , m_Images(inContext.GetAllocator(), "SShaderGeneratorGeneratedShader::m_Images")
        , m_Lights(inContext.GetAllocator(), "SShaderGeneratorGeneratedShader::m_Lights")
        , m_ShadowMaps(inContext.GetAllocator(), "SShaderGeneratorGeneratedShader::m_ShadowMaps")
        , m_RefCount(0)
    {
        //m_Shader.addRef();
    }
    ~SShaderGeneratorGeneratedShader()
    {
        if (m_lightConstantProperties)
            delete m_lightConstantProperties;
        //m_Shader.release();
    }
};


#ifndef EA_PLATFORM_WINDOWS
#define _snprintf snprintf
#endif

struct SShaderGenerator : public IDefaultMaterialShaderGenerator
{
    typedef QString TStrType;
    typedef QHash<QDemonRenderShaderProgram *, QSharedPointer<SShaderGeneratorGeneratedShader>>
    TProgramToShaderMap;
    typedef QHash<QString,
    QSharedPointer<QDemonRenderConstantBuffer>>
    TStrConstanBufMap;

    IQDemonRenderContext &m_RenderContext;
    IShaderProgramGenerator &m_ProgramGenerator;

    const SDefaultMaterial *m_CurrentMaterial;
    SShaderDefaultMaterialKey *m_CurrentKey;
    QDemonRenderShadowMap *m_ShadowMapManager;
    IDefaultMaterialVertexPipeline *m_CurrentPipeline;
    TShaderFeatureSet m_CurrentFeatureSet;
    QDemonDataRef<SLight *> m_Lights;
    SRenderableImage *m_FirstImage;
    bool m_HasTransparency;
    bool m_LightsAsSeparateUniforms;

    TStrType m_ImageStem;
    TStrType m_ImageSampler;
    TStrType m_ImageOffsets;
    TStrType m_ImageRotations;
    TStrType m_ImageFragCoords;
    TStrType m_ImageTemp;
    TStrType m_ImageSamplerSize;

    TStrType m_TexCoordTemp;

    TStrType m_LightStem;
    TStrType m_LightColor;
    TStrType m_LightSpecularColor;
    TStrType m_LightAttenuation;
    TStrType m_LightConstantAttenuation;
    TStrType m_LightLinearAttenuation;
    TStrType m_LightQuadraticAttenuation;
    TStrType m_NormalizedDirection;
    TStrType m_LightDirection;
    TStrType m_LightPos;
    TStrType m_LightUp;
    TStrType m_LightRt;
    TStrType m_RelativeDistance;
    TStrType m_RelativeDirection;

    TStrType m_ShadowMapStem;
    TStrType m_ShadowCubeStem;
    TStrType m_ShadowMatrixStem;
    TStrType m_ShadowCoordStem;
    TStrType m_ShadowControlStem;

    TStrType m_TempStr;

    QString m_GeneratedShaderString;

    SShaderDefaultMaterialKeyProperties m_DefaultMaterialShaderKeyProperties;
    TProgramToShaderMap m_ProgramToShaderMap;

    TStrConstanBufMap m_ConstantBuffers; ///< store all constants buffers

    qint32 m_RefCount;

    SShaderGenerator(IQDemonRenderContext &inRc)
        : m_RenderContext(inRc)
        , m_ProgramGenerator(m_RenderContext.GetShaderProgramGenerator())
        , m_CurrentMaterial(nullptr)
        , m_CurrentKey(nullptr)
        , m_ShadowMapManager(nullptr)
        , m_CurrentPipeline(nullptr)
        , m_FirstImage(nullptr)
        , m_LightsAsSeparateUniforms(false)
    {
    }

    IShaderProgramGenerator &ProgramGenerator() { return m_ProgramGenerator; }
    IDefaultMaterialVertexPipeline &VertexGenerator() { return *m_CurrentPipeline; }
    IShaderStageGenerator &FragmentGenerator()
    {
        return *m_ProgramGenerator.GetStage(ShaderGeneratorStages::Fragment);
    }
    SShaderDefaultMaterialKey &Key() { return *m_CurrentKey; }
    const SDefaultMaterial &Material() { return *m_CurrentMaterial; }
    TShaderFeatureSet FeatureSet() { return m_CurrentFeatureSet; }
    bool HasTransparency() { return m_HasTransparency; }

    void addFunction(IShaderStageGenerator &generator, QString functionName)
    {
        generator.AddFunction(functionName);
    }

    void SetupImageVariableNames(size_t imageIdx)
    {
        m_ImageStem = "image";
        char buf[16];
        _snprintf(buf, 16, "%d", int(imageIdx));
        m_ImageStem.append(buf);
        m_ImageStem.append("_");

        m_ImageSampler = m_ImageStem;
        m_ImageSampler.append("sampler");
        m_ImageOffsets = m_ImageStem;
        m_ImageOffsets.append("offsets");
        m_ImageRotations = m_ImageStem;
        m_ImageRotations.append("rotations");
        m_ImageFragCoords = m_ImageStem;
        m_ImageFragCoords.append("uv_coords");
        m_ImageSamplerSize = m_ImageStem;
        m_ImageSamplerSize.append("size");
    }

    void SetupTexCoordVariableName(size_t uvSet)
    {
        m_TexCoordTemp = "varTexCoord";
        char buf[16];
        _snprintf(buf, 16, "%d", int(uvSet));
        m_TexCoordTemp.append(buf);
    }

    SImageVariableNames GetImageVariableNames(quint32 inIdx) override
    {
        SetupImageVariableNames(inIdx);
        SImageVariableNames retval;
        retval.m_ImageSampler = m_ImageSampler.c_str();
        retval.m_ImageFragCoords = m_ImageFragCoords.c_str();
        return retval;
    }

    void AddLocalVariable(IShaderStageGenerator &inGenerator, const char *inName,
                          const char *inType)
    {
        inGenerator << "\t" << inType << " " << inName << ";" << Endl;
    }

    void AddLocalVariable(IShaderStageGenerator &inGenerator, const TStrType &inName,
                          const char *inType)
    {
        AddLocalVariable(inGenerator, inName.c_str(), inType);
    }

    void GenerateImageUVCoordinates(IShaderStageGenerator &inVertexPipeline, quint32 idx, quint32 uvSet,
                                    SRenderableImage &image) override
    {
        IDefaultMaterialVertexPipeline &vertexShader(
                    static_cast<IDefaultMaterialVertexPipeline &>(inVertexPipeline));
        IShaderStageGenerator &fragmentShader(FragmentGenerator());
        SetupImageVariableNames(idx);
        SetupTexCoordVariableName(uvSet);
        fragmentShader.AddUniform(m_ImageSampler, "sampler2D");
        vertexShader.AddUniform(m_ImageOffsets, "vec3");
        fragmentShader.AddUniform(m_ImageOffsets, "vec3");
        vertexShader.AddUniform(m_ImageRotations, "vec4");
        fragmentShader.AddUniform(m_ImageRotations, "vec4");

        if (image.m_Image.m_MappingMode == ImageMappingModes::Normal) {
            vertexShader << "\tuTransform = vec3( " << m_ImageRotations << ".x, "
                         << m_ImageRotations << ".y, " << m_ImageOffsets << ".x );" << Endl;
            vertexShader << "\tvTransform = vec3( " << m_ImageRotations << ".z, "
                         << m_ImageRotations << ".w, " << m_ImageOffsets << ".y );" << Endl;
            vertexShader.AddOutgoing(m_ImageFragCoords, "vec2");
            addFunction(vertexShader, "getTransformedUVCoords");
            vertexShader.GenerateUVCoords(uvSet);
            m_ImageTemp = m_ImageFragCoords;
            m_ImageTemp.append("temp");
            vertexShader << "\tvec2 " << m_ImageTemp << " = getTransformedUVCoords( vec3( "
                         << m_TexCoordTemp << ", 1.0), uTransform, vTransform );" << Endl;
            if (image.m_Image.m_TextureData.m_TextureFlags.IsInvertUVCoords())
                vertexShader << "\t" << m_ImageTemp << ".y = 1.0 - " << m_ImageFragCoords << ".y;"
                             << Endl;

            vertexShader.AssignOutput(m_ImageFragCoords.c_str(), m_ImageTemp.c_str());
        } else {
            fragmentShader << "\tuTransform = vec3( " << m_ImageRotations << ".x, "
                           << m_ImageRotations << ".y, " << m_ImageOffsets << ".x );" << Endl;
            fragmentShader << "\tvTransform = vec3( " << m_ImageRotations << ".z, "
                           << m_ImageRotations << ".w, " << m_ImageOffsets << ".y );" << Endl;
            vertexShader.GenerateEnvMapReflection();
            addFunction(fragmentShader, "getTransformedUVCoords");
            fragmentShader << "\tvec2 " << m_ImageFragCoords
                           << " = getTransformedUVCoords( environment_map_reflection, uTransform, "
                              "vTransform );"
                           << Endl;
            if (image.m_Image.m_TextureData.m_TextureFlags.IsInvertUVCoords())
                fragmentShader << "\t" << m_ImageFragCoords << ".y = 1.0 - " << m_ImageFragCoords
                               << ".y;" << Endl;
        }
    }

    void GenerateImageUVCoordinates(quint32 idx, SRenderableImage &image, quint32 uvSet = 0)
    {
        GenerateImageUVCoordinates(VertexGenerator(), idx, uvSet, image);
    }

    void GenerateImageUVCoordinates(quint32 idx, SRenderableImage &image,
                                    IDefaultMaterialVertexPipeline &inShader)
    {
        if (image.m_Image.m_MappingMode == ImageMappingModes::Normal) {
            SetupImageVariableNames(idx);
            inShader.AddUniform(m_ImageSampler, "sampler2D");
            inShader.AddUniform(m_ImageOffsets, "vec3");
            inShader.AddUniform(m_ImageRotations, "vec4");

            inShader << "\tuTransform = vec3( " << m_ImageRotations << ".x, " << m_ImageRotations
                     << ".y, " << m_ImageOffsets << ".x );" << Endl;
            inShader << "\tvTransform = vec3( " << m_ImageRotations << ".z, " << m_ImageRotations
                     << ".w, " << m_ImageOffsets << ".y );" << Endl;
            inShader << "\tvec2 " << m_ImageFragCoords << ";" << Endl;
            addFunction(inShader, "getTransformedUVCoords");
            inShader.GenerateUVCoords();
            inShader
                    << "\t" << m_ImageFragCoords
                    << " = getTransformedUVCoords( vec3( varTexCoord0, 1.0), uTransform, vTransform );"
                    << Endl;
            if (image.m_Image.m_TextureData.m_TextureFlags.IsInvertUVCoords())
                inShader << "\t" << m_ImageFragCoords << ".y = 1.0 - " << m_ImageFragCoords << ".y;"
                         << Endl;
        }
    }

    void OutputSpecularEquation(DefaultMaterialSpecularModel::Enum inSpecularModel,
                                IShaderStageGenerator &fragmentShader, const char *inLightDir,
                                const char *inLightSpecColor)
    {
        switch (inSpecularModel) {
        case DefaultMaterialSpecularModel::KGGX: {
            fragmentShader.AddInclude("defaultMaterialPhysGlossyBSDF.glsllib");
            fragmentShader.AddUniform("material_specular", "vec4");
            fragmentShader << "\tglobal_specular_light.rgb += lightAttenuation * specularAmount * "
                              "specularColor * kggxGlossyDefaultMtl( "
                           << "world_normal, tangent, -" << inLightDir << ".xyz, view_vector, "
                           << inLightSpecColor
                           << ".rgb, vec3(material_specular.xyz), roughnessAmount, "
                              "roughnessAmount ).rgb;"
                           << Endl;
        } break;
        case DefaultMaterialSpecularModel::KWard: {
            fragmentShader.AddInclude("defaultMaterialPhysGlossyBSDF.glsllib");
            fragmentShader.AddUniform("material_specular", "vec4");
            fragmentShader << "\tglobal_specular_light.rgb += lightAttenuation * specularAmount * "
                              "specularColor * wardGlossyDefaultMtl( "
                           << "world_normal, tangent, -" << inLightDir << ".xyz, view_vector, "
                           << inLightSpecColor
                           << ".rgb, vec3(material_specular.xyz), roughnessAmount, "
                              "roughnessAmount ).rgb;"
                           << Endl;
        } break;
        default:
            addFunction(fragmentShader, "specularBSDF");
            fragmentShader << "\tglobal_specular_light.rgb += lightAttenuation * specularAmount * "
                              "specularColor * specularBSDF( "
                           << "world_normal, -" << inLightDir << ".xyz, view_vector, "
                           << inLightSpecColor << ".rgb, 1.0, 2.56 / (roughnessAmount + "
                                                  "0.01), vec3(1.0), scatter_reflect ).rgb;"
                           << Endl;
            break;
        }
    }

    void OutputDiffuseAreaLighting(IShaderStageGenerator &infragmentShader, const char *inPos,
                                   TStrType inLightPrefix)
    {
        m_NormalizedDirection = inLightPrefix + "_areaDir";
        AddLocalVariable(infragmentShader, m_NormalizedDirection, "vec3");
        infragmentShader << "\tlightAttenuation = calculateDiffuseAreaOld( " << m_LightDirection
                         << ".xyz, " << m_LightPos << ".xyz, " << m_LightUp << ", " << m_LightRt
                         << ", " << inPos << ", " << m_NormalizedDirection << " );" << Endl;
    }

    void OutputSpecularAreaLighting(IShaderStageGenerator &infragmentShader, const char *inPos,
                                    const char *inView, const char *inLightSpecColor)
    {
        addFunction(infragmentShader, "sampleAreaGlossyDefault");
        infragmentShader.AddUniform("material_specular", "vec4");
        infragmentShader << "global_specular_light.rgb += " << inLightSpecColor
                         << ".rgb * lightAttenuation * shadowFac * material_specular.rgb * "
                            "specularAmount * sampleAreaGlossyDefault( tanFrame, "
                         << inPos << ", " << m_NormalizedDirection << ", " << m_LightPos << ".xyz, "
                         << m_LightRt << ".w, " << m_LightUp << ".w, " << inView
                         << ", roughnessAmount, roughnessAmount ).rgb;" << Endl;
    }

    void AddTranslucencyIrradiance(IShaderStageGenerator &infragmentShader, SRenderableImage *image,
                                   TStrType inLightPrefix, bool areaLight)
    {
        if (image == nullptr)
            return;

        addFunction(infragmentShader, "diffuseReflectionWrapBSDF");
        if (areaLight) {
            infragmentShader << "\tglobal_diffuse_light.rgb += lightAttenuation * "
                                "translucent_thickness_exp * diffuseReflectionWrapBSDF( "
                                "-world_normal, "
                             << m_NormalizedDirection << ", " << m_LightColor
                             << ".rgb, diffuseLightWrap ).rgb;" << Endl;
        } else {
            infragmentShader << "\tglobal_diffuse_light.rgb += lightAttenuation * "
                                "translucent_thickness_exp * diffuseReflectionWrapBSDF( "
                                "-world_normal, "
                             << "-" << m_NormalizedDirection << ", " << m_LightColor
                             << ".rgb, diffuseLightWrap ).rgb;" << Endl;
        }
    }

    void SetupShadowMapVariableNames(size_t lightIdx)
    {
        m_ShadowMapStem = "shadowmap";
        m_ShadowCubeStem = "shadowcube";
        char buf[16];
        _snprintf(buf, 16, "%d", int(lightIdx));
        m_ShadowMapStem.append(buf);
        m_ShadowCubeStem.append(buf);
        m_ShadowMatrixStem = m_ShadowMapStem;
        m_ShadowMatrixStem.append("_matrix");
        m_ShadowCoordStem = m_ShadowMapStem;
        m_ShadowCoordStem.append("_coord");
        m_ShadowControlStem = m_ShadowMapStem;
        m_ShadowControlStem.append("_control");
    }

    void AddShadowMapContribution(IShaderStageGenerator &inLightShader, quint32 lightIndex,
                                  RenderLightTypes::Enum inType)
    {
        SetupShadowMapVariableNames(lightIndex);

        inLightShader.AddInclude("shadowMapping.glsllib");
        if (inType == RenderLightTypes::Directional) {
            inLightShader.AddUniform(m_ShadowMapStem, "sampler2D");
        } else {
            inLightShader.AddUniform(m_ShadowCubeStem, "samplerCube");
        }
        inLightShader.AddUniform(m_ShadowControlStem, "vec4");
        inLightShader.AddUniform(m_ShadowMatrixStem, "mat4");

        /*
        if ( inType == RenderLightTypes::Area )
        {
                inLightShader << "vec2 " << m_ShadowCoordStem << ";" << Endl;
                inLightShader << "\tshadow_map_occl = sampleParaboloid( " << m_ShadowMapStem << ", "
        << m_ShadowControlStem << ", "
                                                                                <<
        m_ShadowMatrixStem << ", varWorldPos, vec2(1.0, " << m_ShadowControlStem << ".z), "
                                                                                << m_ShadowCoordStem
        << " );" << Endl;
        }
        else */
        if (inType != RenderLightTypes::Directional) {
            inLightShader << "\tshadow_map_occl = sampleCubemap( " << m_ShadowCubeStem << ", "
                          << m_ShadowControlStem << ", " << m_ShadowMatrixStem << ", " << m_LightPos
                          << ".xyz, varWorldPos, vec2(1.0, " << m_ShadowControlStem << ".z) );"
                          << Endl;
        } else
            inLightShader << "\tshadow_map_occl = sampleOrthographic( " << m_ShadowMapStem << ", "
                          << m_ShadowControlStem << ", " << m_ShadowMatrixStem
                          << ", varWorldPos, vec2(1.0, " << m_ShadowControlStem << ".z) );" << Endl;
    }

    void AddDisplacementMappingForDepthPass(IShaderStageGenerator &inShader) override
    {
        inShader.AddIncoming("attr_uv0", "vec2");
        inShader.AddIncoming("attr_norm", "vec3");
        inShader.AddUniform("displacementSampler", "sampler2D");
        inShader.AddUniform("displaceAmount", "float");
        inShader.AddUniform("displacementMap_rot", "vec4");
        inShader.AddUniform("displacementMap_offset", "vec3");
        inShader.AddInclude("defaultMaterialFileDisplacementTexture.glsllib");

        inShader.Append("\tvec3 uTransform = vec3( displacementMap_rot.x, displacementMap_rot.y, "
                        "displacementMap_offset.x );");
        inShader.Append("\tvec3 vTransform = vec3( displacementMap_rot.z, displacementMap_rot.w, "
                        "displacementMap_offset.y );");
        addFunction(inShader, "getTransformedUVCoords");
        inShader.Append("\tvec2 uv_coords = attr_uv0;");
        inShader << "\tuv_coords = getTransformedUVCoords( vec3( uv_coords, 1.0), uTransform, "
                    "vTransform );\n";
        inShader << "\tvec3 displacedPos = defaultMaterialFileDisplacementTexture( "
                    "displacementSampler , displaceAmount, uv_coords , attr_norm, attr_pos );"
                 << Endl;
        inShader.Append("\tgl_Position = model_view_projection * vec4(displacedPos, 1.0);");
    }

    void AddDisplacementImageUniforms(IShaderStageGenerator &inGenerator,
                                      quint32 displacementImageIdx,
                                      SRenderableImage *displacementImage) override
    {
        if (displacementImage) {
            SetupImageVariableNames(displacementImageIdx);
            inGenerator.AddInclude("defaultMaterialFileDisplacementTexture.glsllib");
            inGenerator.AddUniform("model_matrix", "mat4");
            inGenerator.AddUniform("camera_position", "vec3");
            inGenerator.AddUniform("displaceAmount", "float");
            inGenerator.AddUniform(m_ImageSampler, "sampler2D");
        }
    }

    bool MaybeAddMaterialFresnel(IShaderStageGenerator &fragmentShader, QDemonConstDataRef<quint32> inKey,
                                 bool inFragmentHasSpecularAmount)
    {
        if (m_DefaultMaterialShaderKeyProperties.m_FresnelEnabled.GetValue(inKey)) {
            if (inFragmentHasSpecularAmount == false)
                fragmentShader << "\tfloat specularAmount = 1.0;" << Endl;
            inFragmentHasSpecularAmount = true;
            fragmentShader.AddInclude("defaultMaterialFresnel.glsllib");
            fragmentShader.AddUniform("fresnelPower", "float");
            fragmentShader.AddUniform("material_specular", "vec4");
            fragmentShader << "\tfloat fresnelRatio = defaultMaterialSimpleFresnel( world_normal, "
                              "view_vector, material_specular.w, fresnelPower );"
                           << Endl;
            fragmentShader << "\tspecularAmount *= fresnelRatio;" << Endl;
        }
        return inFragmentHasSpecularAmount;
    }
    void SetupLightVariableNames(size_t lightIdx, SLight &inLight)
    {
        if (m_LightsAsSeparateUniforms) {
            char buf[16];
            _snprintf(buf, 16, "light_%d", int(lightIdx));
            m_LightStem = buf;
            m_LightColor = m_LightStem;
            m_LightColor.append("_diffuse");
            m_LightDirection = m_LightStem;
            m_LightDirection.append("_direction");
            m_LightSpecularColor = m_LightStem;
            m_LightSpecularColor.append("_specular");
            if (inLight.m_LightType == RenderLightTypes::Point) {
                m_LightPos = m_LightStem;
                m_LightPos.append("_position");
                m_LightAttenuation = m_LightStem;
                m_LightAttenuation.append("_attenuation");
            } else if (inLight.m_LightType == RenderLightTypes::Area) {
                m_LightPos = m_LightStem;
                m_LightPos.append("_position");
                m_LightUp = m_LightStem;
                m_LightUp.append("_up");
                m_LightRt = m_LightStem;
                m_LightRt.append("_right");
            }
        } else {
            m_LightStem = "lights";
            char buf[16];
            _snprintf(buf, 16, "[%d].", int(lightIdx));
            m_LightStem.append(buf);

            m_LightColor = m_LightStem;
            m_LightColor.append("diffuse");
            m_LightDirection = m_LightStem;
            m_LightDirection.append("direction");
            m_LightSpecularColor = m_LightStem;
            m_LightSpecularColor.append("specular");
            if (inLight.m_LightType == RenderLightTypes::Point) {
                m_LightPos = m_LightStem;
                m_LightPos.append("position");
                m_LightConstantAttenuation = m_LightStem;
                m_LightConstantAttenuation.append("constantAttenuation");
                m_LightLinearAttenuation = m_LightStem;
                m_LightLinearAttenuation.append("linearAttenuation");
                m_LightQuadraticAttenuation = m_LightStem;
                m_LightQuadraticAttenuation.append("quadraticAttenuation");
            } else if (inLight.m_LightType == RenderLightTypes::Area) {
                m_LightPos = m_LightStem;
                m_LightPos.append("position");
                m_LightUp = m_LightStem;
                m_LightUp.append("up");
                m_LightRt = m_LightStem;
                m_LightRt.append("right");
            }
        }
    }

    void addDisplacementMapping(IDefaultMaterialVertexPipeline &inShader)
    {
        inShader.AddIncoming("attr_uv0", "vec2");
        inShader.AddIncoming("attr_norm", "vec3");
        inShader.AddUniform("displacementSampler", "sampler2D");
        inShader.AddUniform("displaceAmount", "float");
        inShader.AddUniform("displacementMap_rot", "vec4");
        inShader.AddUniform("displacementMap_offset", "vec3");
        inShader.AddInclude("defaultMaterialFileDisplacementTexture.glsllib");

        inShader.Append("\tvec3 uTransform = vec3( displacementMap_rot.x, displacementMap_rot.y, "
                        "displacementMap_offset.x );");
        inShader.Append("\tvec3 vTransform = vec3( displacementMap_rot.z, displacementMap_rot.w, "
                        "displacementMap_offset.y );");
        addFunction(inShader, "getTransformedUVCoords");
        inShader.GenerateUVCoords();
        inShader << "\tvarTexCoord0 = getTransformedUVCoords( vec3( varTexCoord0, 1.0), "
                    "uTransform, vTransform );\n";
        inShader << "\tvec3 displacedPos = defaultMaterialFileDisplacementTexture( "
                    "displacementSampler , displaceAmount, varTexCoord0 , attr_norm, attr_pos );"
                 << Endl;
        inShader.Append("\tgl_Position = model_view_projection * vec4(displacedPos, 1.0);");
    }

    void GenerateTextureSwizzle(QDemonRenderTextureSwizzleMode::Enum swizzleMode,
                                eastl::basic_string<char> &texSwizzle,
                                eastl::basic_string<char> &lookupSwizzle)
    {
        QDemonRenderContextType deprecatedContextFlags(QDemonRenderContextValues::GL2
                                                       | QDemonRenderContextValues::GLES2);

        if (!(m_RenderContext.GetRenderContext().GetRenderContextType() & deprecatedContextFlags)) {
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
    QDemonRenderConstantBuffer *GetLightConstantBuffer(quint32 inLightCount)
    {
        QDemonRenderContext &theContext(m_RenderContext.GetRenderContext());

        // we assume constant buffer support
        Q_ASSERT(theContext.GetConstantBufferSupport());

        // we only create if if we have lights
        if (!inLightCount || !theContext.GetConstantBufferSupport())
            return nullptr;

        QString theName = QStringLiterial("cbBufferLights");
        QDemonRenderConstantBuffer *pCB = theContext.GetConstantBuffer(theName);

        if (!pCB) {
            // create
            SLightSourceShader s[QDEMON_MAX_NUM_LIGHTS];
            QDemonDataRef<quint8> cBuffer((quint8 *)&s, (sizeof(SLightSourceShader) * QDEMON_MAX_NUM_LIGHTS)
                                          + (4 * sizeof(qint32)));
            pCB = theContext.CreateConstantBuffer(
                        theName, QDemonRenderBufferUsageType::Static,
                        (sizeof(SLightSourceShader) * QDEMON_MAX_NUM_LIGHTS) + (4 * sizeof(qint32)), cBuffer);
            if (!pCB) {
                Q_ASSERT(false);
                return nullptr;
            }
            // init first set
            memset(&s[0], 0x0, sizeof(SLightSourceShader));
            qint32 cgLights = 0;
            pCB->UpdateRaw(0, QDemonDataRef<quint8>((quint8 *)&cgLights, sizeof(qint32)));
            pCB->UpdateRaw(4 * sizeof(qint32),
                           QDemonDataRef<quint8>((quint8 *)&s[0], sizeof(SLightSourceShader)));
            pCB->Update(); // update to hardware

            m_ConstantBuffers.insert(theName, pCB);
        }

        return pCB;
    }

    void SetImageShaderVariables(SShaderGeneratorGeneratedShader &inShader,
                                 SRenderableImage &inImage, quint32 idx)
    {
        size_t numImageVariables = inShader.m_Images.size();
        for (size_t namesIdx = numImageVariables; namesIdx <= idx; ++namesIdx) {
            SetupImageVariableNames(idx);
            inShader.m_Images.push_back(
                        SShaderTextureProperties(m_ImageSampler.c_str(), m_ImageOffsets.c_str(),
                                                 m_ImageRotations.c_str(), m_ImageSamplerSize.c_str(),
                                                 inShader.m_Shader));
        }
        SShaderTextureProperties &theShaderProps = inShader.m_Images[idx];
        const QMatrix4x4 &textureTransform = inImage.m_Image.m_TextureTransform;
        // We separate rotational information from offset information so that just maybe the shader
        // will attempt to push less information to the card.
        const float *dataPtr(textureTransform.front());
        // The third member of the offsets contains a flag indicating if the texture was
        // premultiplied or not.
        // We use this to mix the texture alpha.
        QVector3D offsets(dataPtr[12], dataPtr[13],
                inImage.m_Image.m_TextureData.m_TextureFlags.IsPreMultiplied() ? 1.0f
                                                                               : 0.0f);
        // Grab just the upper 2x2 rotation matrix from the larger matrix.
        QVector4D rotations(dataPtr[0], dataPtr[4], dataPtr[1], dataPtr[5]);

        // The image horizontal and vertical tiling modes need to be set here, before we set texture
        // on the shader.
        // because setting the image on the texture forces the textue to bind and immediately apply
        // any tex params.
        QDemonRenderTexture2D *imageTexture = inImage.m_Image.m_TextureData.m_Texture;
        inImage.m_Image.m_TextureData.m_Texture->SetTextureWrapS(
                    inImage.m_Image.m_HorizontalTilingMode);
        inImage.m_Image.m_TextureData.m_Texture->SetTextureWrapT(
                    inImage.m_Image.m_VerticalTilingMode);
        theShaderProps.m_Sampler.Set(imageTexture);
        theShaderProps.m_Offsets.Set(offsets);
        theShaderProps.m_Rotations.Set(rotations);
        theShaderProps.m_Size.Set(QVector2D(imageTexture->GetTextureDetails().m_Width,
                                            imageTexture->GetTextureDetails().m_Height));
    }

    void GenerateShadowMapOcclusion(quint32 lightIdx, bool inShadowEnabled,
                                    RenderLightTypes::Enum inType)
    {
        if (inShadowEnabled) {
            VertexGenerator().GenerateWorldPosition();
            AddShadowMapContribution(FragmentGenerator(), lightIdx, inType);
            /*
            VertexGenerator().AddUniform( m_ShadowMatrixStem, "mat4" );
            VertexGenerator().AddOutgoing( m_ShadowCoordStem, "vec4" );
            VertexGenerator() << "\tvec4 local_" << m_ShadowCoordStem << " = " << m_ShadowMatrixStem
            << " * vec4(local_model_world_position, 1.0);" << Endl;
            m_TempStr.assign( "local_" );
            m_TempStr.append( m_ShadowCoordStem );
            VertexGenerator().AssignOutput( m_ShadowCoordStem.c_str(), m_TempStr.c_str() );
            */
        } else {
            FragmentGenerator() << "\tshadow_map_occl = 1.0;" << Endl;
        }
    }

    void GenerateVertexShader()
    {
        // vertex displacement
        quint32 imageIdx = 0;
        SRenderableImage *displacementImage = nullptr;
        quint32 displacementImageIdx = 0;

        for (SRenderableImage *img = m_FirstImage; img != nullptr;
             img = img->m_NextImage, ++imageIdx) {
            if (img->m_MapType == ImageMapTypes::Displacement) {
                displacementImage = img;
                displacementImageIdx = imageIdx;
                break;
            }
        }

        // the pipeline opens/closes up the shaders stages
        VertexGenerator().BeginVertexGeneration(displacementImageIdx, displacementImage);
    }

    void GenerateFragmentShader(SShaderDefaultMaterialKey &inKey)
    {
        bool specularEnabled = Material().IsSpecularEnabled();
        bool vertexColorsEnabled = Material().IsVertexColorsEnabled();

        bool hasLighting = Material().HasLighting();
        bool hasImage = m_FirstImage != nullptr;

        bool hasIblProbe = m_DefaultMaterialShaderKeyProperties.m_HasIbl.GetValue(inKey);
        bool hasSpecMap = false;
        bool hasEnvMap = false;
        bool hasEmissiveMap = false;
        bool hasLightmaps = false;
        // Pull the bump out as
        SRenderableImage *bumpImage = nullptr;
        quint32 imageIdx = 0;
        quint32 bumpImageIdx = 0;
        SRenderableImage *specularAmountImage = nullptr;
        quint32 specularAmountImageIdx = 0;
        SRenderableImage *roughnessImage = nullptr;
        quint32 roughnessImageIdx = 0;
        // normal mapping
        SRenderableImage *normalImage = nullptr;
        quint32 normalImageIdx = 0;
        // translucency map
        SRenderableImage *translucencyImage = nullptr;
        quint32 translucencyImageIdx = 0;
        // lightmaps
        SRenderableImage *lightmapIndirectImage = nullptr;
        quint32 lightmapIndirectImageIdx = 0;
        SRenderableImage *lightmapRadiosityImage = nullptr;
        quint32 lightmapRadiosityImageIdx = 0;
        SRenderableImage *lightmapShadowImage = nullptr;
        quint32 lightmapShadowImageIdx = 0;
        const bool supportStandardDerivatives
                = m_RenderContext.GetRenderContext().IsStandardDerivativesSupported();

        for (SRenderableImage *img = m_FirstImage; img != nullptr;
             img = img->m_NextImage, ++imageIdx) {
            hasSpecMap = img->m_MapType == ImageMapTypes::Specular;
            if (img->m_MapType == ImageMapTypes::Bump) {
                bumpImage = img;
                bumpImageIdx = imageIdx;
            } else if (img->m_MapType == ImageMapTypes::SpecularAmountMap) {
                specularAmountImage = img;
                specularAmountImageIdx = imageIdx;
            } else if (img->m_MapType == ImageMapTypes::Roughness) {
                roughnessImage = img;
                roughnessImageIdx = imageIdx;
            } else if (img->m_MapType == ImageMapTypes::Normal) {
                normalImage = img;
                normalImageIdx = imageIdx;
            } else if (img->m_Image.m_MappingMode == ImageMappingModes::Environment) {
                hasEnvMap = true;
            } else if (img->m_MapType == ImageMapTypes::Translucency) {
                translucencyImage = img;
                translucencyImageIdx = imageIdx;
            } else if (img->m_MapType == ImageMapTypes::Emissive) {
                hasEmissiveMap = true;
            } else if (img->m_MapType == ImageMapTypes::LightmapIndirect) {
                lightmapIndirectImage = img;
                lightmapIndirectImageIdx = imageIdx;
                hasLightmaps = true;
            } else if (img->m_MapType == ImageMapTypes::LightmapRadiosity) {
                lightmapRadiosityImage = img;
                lightmapRadiosityImageIdx = imageIdx;
                hasLightmaps = true;
            } else if (img->m_MapType == ImageMapTypes::LightmapShadow) {
                lightmapShadowImage = img;
                lightmapShadowImageIdx = imageIdx;
                hasLightmaps = true;
            }
        }

        bool enableFresnel = m_DefaultMaterialShaderKeyProperties.m_FresnelEnabled.GetValue(inKey);
        bool enableSSAO = false;
        bool enableSSDO = false;
        bool enableShadowMaps = false;
        bool enableBumpNormal = normalImage || bumpImage;

        for (quint32 idx = 0; idx < FeatureSet().size(); ++idx) {
            QString name(FeatureSet()[idx].m_Name.c_str());
            if (name == "QDEMON_ENABLE_SSAO")
                enableSSAO = FeatureSet()[idx].m_Enabled;
            else if (name == "QDEMON_ENABLE_SSDO")
                enableSSDO = FeatureSet()[idx].m_Enabled;
            else if (name == "QDEMON_ENABLE_SSM")
                enableShadowMaps = FeatureSet()[idx].m_Enabled;
        }

        bool includeSSAOSSDOVars = enableSSAO || enableSSDO || enableShadowMaps;

        VertexGenerator().BeginFragmentGeneration();
        IShaderStageGenerator &fragmentShader(FragmentGenerator());
        IDefaultMaterialVertexPipeline &vertexShader(VertexGenerator());

        // The fragment or vertex shaders may not use the material_properties or diffuse
        // uniforms in all cases but it is simpler to just add them and let the linker strip them.
        fragmentShader.AddUniform("material_diffuse", "vec4");
        fragmentShader.AddUniform("diffuse_color", "vec3");
        fragmentShader.AddUniform("material_properties", "vec4");

        // All these are needed for SSAO
        if (includeSSAOSSDOVars) {
            fragmentShader.AddInclude("SSAOCustomMaterial.glsllib");
            // fragmentShader.AddUniform( "ao_sampler", "sampler2D" );
        }

        if (hasIblProbe && hasLighting) {
            fragmentShader.AddInclude("sampleProbe.glsllib");
        }

        if (hasLighting) {
            if (!m_LightsAsSeparateUniforms)
                addFunction(fragmentShader, "sampleLightVars");
            addFunction(fragmentShader, "diffuseReflectionBSDF");
        }

        if (hasLighting && hasLightmaps) {
            fragmentShader.AddInclude("evalLightmaps.glsllib");
        }

        // view_vector, varWorldPos, world_normal are all used if there is a specular map
        // in addition to if there is specular lighting.  So they are lifted up here, always
        // generated.
        // we rely on the linker to strip out what isn't necessary instead of explicitly stripping
        // it for code simplicity.
        if (hasImage) {
            fragmentShader.Append("\tvec3 uTransform;");
            fragmentShader.Append("\tvec3 vTransform;");
        }

        if (includeSSAOSSDOVars || hasSpecMap || hasLighting || hasEnvMap || enableFresnel
                || hasIblProbe || enableBumpNormal) {
            vertexShader.GenerateViewVector();
            vertexShader.GenerateWorldNormal();
            vertexShader.GenerateWorldPosition();
        }
        if (includeSSAOSSDOVars || specularEnabled || hasIblProbe || enableBumpNormal)
            vertexShader.GenerateVarTangentAndBinormal();

        if (vertexColorsEnabled)
            vertexShader.GenerateVertexColor();
        else
            fragmentShader.Append("\tvec3 vertColor = vec3(1.0);");

        // You do bump or normal mapping but not both
        if (bumpImage != nullptr) {
            GenerateImageUVCoordinates(bumpImageIdx, *bumpImage);
            fragmentShader.AddUniform("bumpAmount", "float");

            fragmentShader.AddUniform(m_ImageSamplerSize, "vec2");
            fragmentShader.AddInclude("defaultMaterialBumpNoLod.glsllib");
            fragmentShader << "\tworld_normal = defaultMaterialBumpNoLod( " << m_ImageSampler
                           << ", bumpAmount, " << m_ImageFragCoords
                           << ", tangent, binormal, world_normal, "
                           << m_ImageSamplerSize << ");" << Endl;
            // Do gram schmidt
            fragmentShader << "\tbinormal = normalize(cross(world_normal, tangent) );\n";
            fragmentShader << "\ttangent = normalize(cross(binormal, world_normal) );\n";

        } else if (normalImage != nullptr) {
            GenerateImageUVCoordinates(normalImageIdx, *normalImage);

            fragmentShader.AddInclude("defaultMaterialFileNormalTexture.glsllib");
            fragmentShader.AddUniform("bumpAmount", "float");

            fragmentShader << "\tworld_normal = defaultMaterialFileNormalTexture( "
                           << m_ImageSampler << ", bumpAmount, " << m_ImageFragCoords
                           << ", tangent, binormal );" << Endl;
        }

        if (includeSSAOSSDOVars || specularEnabled || hasIblProbe || enableBumpNormal)
            fragmentShader << "\tmat3 tanFrame = mat3(tangent, binormal, world_normal);" << Endl;

        bool fragmentHasSpecularAmount = false;

        if (hasEmissiveMap) {
            fragmentShader.Append("\tvec3 global_emission = material_diffuse.rgb;");
        }

        if (hasLighting) {
            fragmentShader.AddUniform("light_ambient_total", "vec3");

            fragmentShader.Append(
                        "\tvec4 global_diffuse_light = vec4(light_ambient_total.xyz, 1.0);");
            fragmentShader.Append("\tvec3 global_specular_light = vec3(0.0, 0.0, 0.0);");
            fragmentShader.Append("\tfloat shadow_map_occl = 1.0;");

            if (specularEnabled) {
                vertexShader.GenerateViewVector();
                fragmentShader.AddUniform("material_properties", "vec4");
            }

            if (lightmapIndirectImage != nullptr) {
                GenerateImageUVCoordinates(lightmapIndirectImageIdx, *lightmapIndirectImage, 1);
                fragmentShader << "\tvec4 indirect_light = texture2D( " << m_ImageSampler << ", "
                               << m_ImageFragCoords << ");" << Endl;
                fragmentShader << "\tglobal_diffuse_light += indirect_light;" << Endl;
                if (specularEnabled) {
                    fragmentShader
                            << "\tglobal_specular_light += indirect_light.rgb * material_properties.x;"
                            << Endl;
                }
            }

            if (lightmapRadiosityImage != nullptr) {
                GenerateImageUVCoordinates(lightmapRadiosityImageIdx, *lightmapRadiosityImage, 1);
                fragmentShader << "\tvec4 direct_light = texture2D( " << m_ImageSampler << ", "
                               << m_ImageFragCoords << ");" << Endl;
                fragmentShader << "\tglobal_diffuse_light += direct_light;" << Endl;
                if (specularEnabled) {
                    fragmentShader
                            << "\tglobal_specular_light += direct_light.rgb * material_properties.x;"
                            << Endl;
                }
            }

            if (translucencyImage != nullptr) {
                fragmentShader.AddUniform("translucentFalloff", "float");
                fragmentShader.AddUniform("diffuseLightWrap", "float");

                GenerateImageUVCoordinates(translucencyImageIdx, *translucencyImage);

                fragmentShader << "\tvec4 translucent_depth_range = texture2D( " << m_ImageSampler
                               << ", " << m_ImageFragCoords << ");" << Endl;
                fragmentShader << "\tfloat translucent_thickness = translucent_depth_range.r * "
                                  "translucent_depth_range.r;"
                               << Endl;
                fragmentShader << "\tfloat translucent_thickness_exp = exp( translucent_thickness "
                                  "* translucentFalloff);"
                               << Endl;
            }

            fragmentShader.Append("\tfloat lightAttenuation = 1.0;");

            AddLocalVariable(fragmentShader, "aoFactor", "float");

            if (hasLighting && enableSSAO)
                fragmentShader.Append("\taoFactor = customMaterialAO();");
            else
                fragmentShader.Append("\taoFactor = 1.0;");

            AddLocalVariable(fragmentShader, "shadowFac", "float");

            if (specularEnabled) {
                fragmentShader << "\tfloat specularAmount = material_properties.x;" << Endl;
                fragmentHasSpecularAmount = true;
            }
            // Fragment lighting means we can perhaps attenuate the specular amount by a texture
            // lookup.

            fragmentShader << "\tvec3 specularColor = vec3(1.0);" << Endl;
            if (specularAmountImage) {
                if (!specularEnabled)
                    fragmentShader << "\tfloat specularAmount = 1.0;" << Endl;
                GenerateImageUVCoordinates(specularAmountImageIdx, *specularAmountImage);
                fragmentShader << "\tspecularColor = texture2D( "
                               << m_ImageSampler << ", " << m_ImageFragCoords << " ).xyz;" << Endl;
                fragmentHasSpecularAmount = true;
            }

            fragmentShader << "\tfloat roughnessAmount = material_properties.y;" << Endl;
            if (roughnessImage) {
                GenerateImageUVCoordinates(roughnessImageIdx, *roughnessImage);
                fragmentShader << "\tfloat sampledRoughness = texture2D( "
                               << m_ImageSampler << ", " << m_ImageFragCoords << " ).x;" << Endl;
                //The roughness sampled from roughness textures is Disney roughness
                //which has to be squared to get the proper value
                fragmentShader << "\troughnessAmount = roughnessAmount * "
                               << "sampledRoughness * sampledRoughness;" << Endl;
            }

            fragmentHasSpecularAmount =
                    MaybeAddMaterialFresnel(fragmentShader, inKey, fragmentHasSpecularAmount);

            // Iterate through all lights
            for (quint32 lightIdx = 0; lightIdx < m_Lights.size(); ++lightIdx) {
                SLight *lightNode = m_Lights[lightIdx];
                SetupLightVariableNames(lightIdx, *lightNode);
                bool isDirectional = lightNode->m_LightType == RenderLightTypes::Directional;
                bool isArea = lightNode->m_LightType == RenderLightTypes::Area;
                bool isShadow = enableShadowMaps && lightNode->m_CastShadow;

                fragmentShader.Append("");
                char buf[10];
                sprintf(buf, "%d", lightIdx);

                m_TempStr.assign("light");
                m_TempStr.append(buf);

                fragmentShader << "\t//Light " << buf << Endl;
                fragmentShader << "\tlightAttenuation = 1.0;" << Endl;
                if (isDirectional) {

                    if (m_LightsAsSeparateUniforms) {
                        fragmentShader.AddUniform(m_LightDirection, "vec4");
                        fragmentShader.AddUniform(m_LightColor, "vec4");
                    }

                    if (enableSSDO) {
                        fragmentShader << "\tshadowFac = customMaterialShadow( " << m_LightDirection
                                       << ".xyz, varWorldPos );" << Endl;
                    } else {
                        fragmentShader << "\tshadowFac = 1.0;" << Endl;
                    }

                    GenerateShadowMapOcclusion(lightIdx, enableShadowMaps && isShadow,
                                               lightNode->m_LightType);

                    if (specularEnabled && enableShadowMaps && isShadow)
                        fragmentShader << "\tlightAttenuation *= shadow_map_occl;" << Endl;

                    fragmentShader << "\tglobal_diffuse_light.rgb += shadowFac * shadow_map_occl * "
                                      "diffuseReflectionBSDF( world_normal, "
                                   << "-" << m_LightDirection << ".xyz, view_vector, "
                                   << m_LightColor << ".rgb, 0.0 ).rgb;" << Endl;

                    if (specularEnabled) {
                        if (m_LightsAsSeparateUniforms)
                            fragmentShader.AddUniform(m_LightSpecularColor, "vec4");
                        OutputSpecularEquation(Material().m_SpecularModel, fragmentShader,
                                               m_LightDirection.c_str(),
                                               m_LightSpecularColor.c_str());
                    }
                } else if (isArea) {
                    if (m_LightsAsSeparateUniforms) {
                        fragmentShader.AddUniform(m_LightColor, "vec4");
                        fragmentShader.AddUniform(m_LightPos, "vec4");
                        fragmentShader.AddUniform(m_LightDirection, "vec4");
                        fragmentShader.AddUniform(m_LightUp, "vec4");
                        fragmentShader.AddUniform(m_LightRt, "vec4");
                    } else {
                        addFunction(fragmentShader, "areaLightVars");
                    }
                    addFunction(fragmentShader, "calculateDiffuseAreaOld");
                    vertexShader.GenerateWorldPosition();
                    GenerateShadowMapOcclusion(lightIdx, enableShadowMaps && isShadow,
                                               lightNode->m_LightType);

                    // Debug measure to make sure paraboloid sampling was projecting to the right
                    // location
                    // fragmentShader << "\tglobal_diffuse_light.rg += " << m_ShadowCoordStem << ";"
                    // << Endl;
                    m_NormalizedDirection = m_TempStr;
                    m_NormalizedDirection.append("_Frame");

                    AddLocalVariable(fragmentShader, m_NormalizedDirection, "mat3");
                    fragmentShader << m_NormalizedDirection << " = mat3( " << m_LightRt << ".xyz, "
                                   << m_LightUp << ".xyz, -" << m_LightDirection << ".xyz );"
                                   << Endl;

                    if (enableSSDO) {
                        fragmentShader << "\tshadowFac = shadow_map_occl * customMaterialShadow( "
                                       << m_LightDirection << ".xyz, varWorldPos );" << Endl;
                    } else {
                        fragmentShader << "\tshadowFac = shadow_map_occl;" << Endl;
                    }

                    if (specularEnabled) {
                        vertexShader.GenerateViewVector();
                        if (m_LightsAsSeparateUniforms)
                            fragmentShader.AddUniform(m_LightSpecularColor, "vec4");
                        OutputSpecularAreaLighting(fragmentShader, "varWorldPos", "view_vector",
                                                   m_LightSpecularColor.c_str());
                    }

                    OutputDiffuseAreaLighting(fragmentShader, "varWorldPos", m_TempStr);
                    fragmentShader << "\tlightAttenuation *= shadowFac;" << Endl;

                    AddTranslucencyIrradiance(fragmentShader, translucencyImage, m_TempStr, true);

                    fragmentShader << "\tglobal_diffuse_light.rgb += lightAttenuation * "
                                      "diffuseReflectionBSDF( world_normal, "
                                   << m_NormalizedDirection << ", view_vector, " << m_LightColor
                                   << ".rgb, 0.0 ).rgb;" << Endl;
                } else {

                    vertexShader.GenerateWorldPosition();
                    GenerateShadowMapOcclusion(lightIdx, enableShadowMaps && isShadow,
                                               lightNode->m_LightType);

                    if (m_LightsAsSeparateUniforms) {
                        fragmentShader.AddUniform(m_LightColor, "vec4");
                        fragmentShader.AddUniform(m_LightPos, "vec4");
                    }

                    m_RelativeDirection = m_TempStr;
                    m_RelativeDirection.append("_relativeDirection");

                    m_NormalizedDirection = m_RelativeDirection;
                    m_NormalizedDirection.append("_normalized");

                    m_RelativeDistance = m_TempStr;
                    m_RelativeDistance.append("_distance");

                    fragmentShader << "\tvec3 " << m_RelativeDirection << " = varWorldPos - "
                                   << m_LightPos << ".xyz;" << Endl;
                    fragmentShader << "\tfloat " << m_RelativeDistance << " = length( "
                                   << m_RelativeDirection << " );" << Endl;
                    fragmentShader << "\tvec3 " << m_NormalizedDirection << " = "
                                   << m_RelativeDirection << " / " << m_RelativeDistance << ";"
                                   << Endl;

                    if (enableSSDO) {
                        fragmentShader << "\tshadowFac = shadow_map_occl * customMaterialShadow( "
                                       << m_NormalizedDirection << ", varWorldPos );" << Endl;
                    } else {
                        fragmentShader << "\tshadowFac = shadow_map_occl;" << Endl;
                    }

                    addFunction(fragmentShader, "calculatePointLightAttenuation");

                    if (m_LightsAsSeparateUniforms) {
                        fragmentShader.AddUniform(m_LightAttenuation, "vec3");
                        fragmentShader
                                << "\tlightAttenuation = shadowFac * calculatePointLightAttenuation("
                                << "vec3( " << m_LightAttenuation << ".x, " << m_LightAttenuation
                                << ".y, " << m_LightAttenuation << ".z), " << m_RelativeDistance
                                << ");" << Endl;
                    } else {
                        fragmentShader
                                << "\tlightAttenuation = shadowFac * calculatePointLightAttenuation("
                                << "vec3( " << m_LightConstantAttenuation << ", "
                                << m_LightLinearAttenuation << ", " << m_LightQuadraticAttenuation
                                << "), " << m_RelativeDistance << ");"
                                << Endl;
                    }



                    AddTranslucencyIrradiance(fragmentShader, translucencyImage, m_TempStr, false);

                    fragmentShader << "\tglobal_diffuse_light.rgb += lightAttenuation * "
                                      "diffuseReflectionBSDF( world_normal, "
                                   << "-" << m_NormalizedDirection << ", view_vector, "
                                   << m_LightColor << ".rgb, 0.0 ).rgb;" << Endl;

                    if (specularEnabled) {
                        if (m_LightsAsSeparateUniforms)
                            fragmentShader.AddUniform(m_LightSpecularColor, "vec4");
                        OutputSpecularEquation(Material().m_SpecularModel, fragmentShader,
                                               m_NormalizedDirection.c_str(),
                                               m_LightSpecularColor.c_str());
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
                           << Endl << "\tglobal_specular_light = vec3(global_specular_light.xyz);"
                           << Endl;
        } else // no lighting.
        {
            fragmentShader << "\tvec4 global_diffuse_light = vec4(0.0, 0.0, 0.0, object_opacity);"
                           << Endl << "\tvec3 global_specular_light = vec3(0.0, 0.0, 0.0);" << Endl;

            // We still have specular maps and such that could potentially use the fresnel variable.
            fragmentHasSpecularAmount =
                    MaybeAddMaterialFresnel(fragmentShader, inKey, fragmentHasSpecularAmount);
        }

        if (!hasEmissiveMap)
            fragmentShader
                    << "\tglobal_diffuse_light.rgb += diffuse_color.rgb * material_diffuse.rgb;"
                    << Endl;

        // since we already modulate our material diffuse color
        // into the light color we will miss it entirely if no IBL
        // or light is used
        if (hasLightmaps && !(m_Lights.size() || hasIblProbe))
            fragmentShader << "\tglobal_diffuse_light.rgb *= diffuse_color.rgb;" << Endl;

        if (hasLighting && hasIblProbe) {
            vertexShader.GenerateWorldNormal();

            fragmentShader << "\tglobal_diffuse_light.rgb += diffuse_color.rgb * aoFactor * "
                              "sampleDiffuse( tanFrame ).xyz;"
                           << Endl;

            if (specularEnabled) {

                fragmentShader.AddUniform("material_specular", "vec4");

                fragmentShader << "\tglobal_specular_light.xyz += specularAmount * specularColor * "
                                  "vec3(material_specular.xyz) * sampleGlossy( tanFrame, "
                                  "view_vector, roughnessAmount ).xyz;"
                               << Endl;
            }
        }

        if (hasImage) {
            fragmentShader.Append("\tvec4 texture_color;");
            quint32 idx = 0;
            for (SRenderableImage *image = m_FirstImage; image; image = image->m_NextImage, ++idx) {
                // Various maps are handled on a different locations
                if (image->m_MapType == ImageMapTypes::Bump
                        || image->m_MapType == ImageMapTypes::Normal
                        || image->m_MapType == ImageMapTypes::Displacement
                        || image->m_MapType == ImageMapTypes::SpecularAmountMap
                        || image->m_MapType == ImageMapTypes::Roughness
                        || image->m_MapType == ImageMapTypes::Translucency
                        || image->m_MapType == ImageMapTypes::LightmapIndirect
                        || image->m_MapType == ImageMapTypes::LightmapRadiosity) {
                    continue;
                }

                eastl::basic_string<char> texSwizzle, lookupSwizzle, texLodStr;

                GenerateImageUVCoordinates(idx, *image, 0);

                GenerateTextureSwizzle(
                            image->m_Image.m_TextureData.m_Texture->GetTextureSwizzleMode(), texSwizzle,
                            lookupSwizzle);

                if (texLodStr.empty()) {
                    fragmentShader << "\ttexture_color" << texSwizzle.c_str() << " = texture2D( "
                                   << m_ImageSampler << ", " << m_ImageFragCoords << ")"
                                   << lookupSwizzle.c_str() << ";" << Endl;
                } else {
                    fragmentShader << "\ttexture_color" << texSwizzle.c_str() << "= textureLod( "
                                   << m_ImageSampler << ", " << m_ImageFragCoords << ", "
                                   << texLodStr.c_str() << " )" << lookupSwizzle.c_str() << ";"
                                   << Endl;
                }

                if (image->m_Image.m_TextureData.m_TextureFlags.IsPreMultiplied() == true)
                    fragmentShader << "\ttexture_color.rgb = texture_color.a > 0.0 ? "
                                      "texture_color.rgb / texture_color.a : vec3( 0, 0, 0 );"
                                   << Endl;

                // These mapping types honestly don't make a whole ton of sense to me.
                switch (image->m_MapType) {
                case ImageMapTypes::Diffuse: // assume images are premultiplied.
                case ImageMapTypes::LightmapShadow:
                    // We use image offsets.z to switch between incoming premultiplied textures or
                    // not premultiplied textures.
                    // If Z is 1, then we assume the incoming texture is already premultiplied, else
                    // we just read the rgb value.
                    fragmentShader.Append("\tglobal_diffuse_light *= texture_color;");
                    break;
                case ImageMapTypes::Specular:

                    fragmentShader.AddUniform("material_specular", "vec4");
                    if (fragmentHasSpecularAmount) {
                        fragmentShader.Append("\tglobal_specular_light.xyz += specularAmount * "
                                              "specularColor * texture_color.xyz * "
                                              "material_specular.xyz;");
                    } else {
                        fragmentShader.Append("\tglobal_specular_light.xyz += texture_color.xyz * "
                                              "material_specular.xyz;");
                    }
                    fragmentShader.Append("\tglobal_diffuse_light.a *= texture_color.a;");
                    break;
                case ImageMapTypes::Opacity:
                    fragmentShader.Append("\tglobal_diffuse_light.a *= texture_color.a;");
                    break;
                case ImageMapTypes::Emissive:
                    fragmentShader.Append(
                                "\tglobal_emission *= texture_color.xyz * texture_color.a;");
                    break;
                default:
                    Q_ASSERT(false); // fallthrough intentional
                }
            }
        }

        if (hasEmissiveMap) {
            fragmentShader.Append("\tglobal_diffuse_light.rgb += global_emission.rgb;");
        }

        // Ensure the rgb colors are in range.
        fragmentShader.Append("\tfragOutput = vec4( clamp( vertColor * global_diffuse_light.xyz + "
                              "global_specular_light.xyz, 0.0, 65519.0 ), global_diffuse_light.a "
                              ");");

        if (VertexGenerator().HasActiveWireframe()) {
            fragmentShader.Append("vec3 edgeDistance = varEdgeDistance * gl_FragCoord.w;");
            fragmentShader.Append(
                        "\tfloat d = min(min(edgeDistance.x, edgeDistance.y), edgeDistance.z);");
            fragmentShader.Append("\tfloat mixVal = smoothstep(0.0, 1.0, d);"); // line width 1.0

            fragmentShader.Append(
                        "\tfragOutput = mix( vec4(0.0, 1.0, 0.0, 1.0), fragOutput, mixVal);");
        }
    }

    QDemonRenderShaderProgram *GenerateMaterialShader(const char *inShaderPrefix)
    {
        // build a string that allows us to print out the shader we are generating to the log.
        // This is time consuming but I feel like it doesn't happen all that often and is very
        // useful to users
        // looking at the log file.

        m_GeneratedShaderString.clear();
        m_GeneratedShaderString.assign(nonNull(inShaderPrefix));

        SShaderDefaultMaterialKey theKey(Key());
        theKey.ToString(m_GeneratedShaderString, m_DefaultMaterialShaderKeyProperties);

        m_LightsAsSeparateUniforms = !m_RenderContext.GetRenderContext().GetConstantBufferSupport();

        GenerateVertexShader();
        GenerateFragmentShader(theKey);

        VertexGenerator().EndVertexGeneration();
        VertexGenerator().EndFragmentGeneration();

        return ProgramGenerator().CompileGeneratedShader(m_GeneratedShaderString.c_str(),
                                                         SShaderCacheProgramFlags(), FeatureSet());
    }

    virtual QDemonRenderShaderProgram *
    GenerateShader(const SGraphObject &inMaterial, SShaderDefaultMaterialKey inShaderDescription,
                   IShaderStageGenerator &inVertexPipeline, TShaderFeatureSet inFeatureSet,
                   QDemonDataRef<SLight *> inLights, SRenderableImage *inFirstImage,
                   bool inHasTransparency, const char *inVertexPipelineName, const char *) override
    {
        Q_ASSERT(inMaterial.m_Type == GraphObjectTypes::DefaultMaterial);
        m_CurrentMaterial = static_cast<const SDefaultMaterial *>(&inMaterial);
        m_CurrentKey = &inShaderDescription;
        m_CurrentPipeline = static_cast<IDefaultMaterialVertexPipeline *>(&inVertexPipeline);
        m_CurrentFeatureSet = inFeatureSet;
        m_Lights = inLights;
        m_FirstImage = inFirstImage;
        m_HasTransparency = inHasTransparency;

        return GenerateMaterialShader(inVertexPipelineName);
    }

    SShaderGeneratorGeneratedShader &GetShaderForProgram(QDemonRenderShaderProgram &inProgram)
    {
        QPair<TProgramToShaderMap::iterator, bool> inserter =
                m_ProgramToShaderMap.insert(&inProgram, QSharedPointer<SShaderGeneratorGeneratedShader>(nullptr));
        if (inserter.second) {
            inserter.first->second = new SShaderGeneratorGeneratedShader(inProgram, m_RenderContext.GetRenderContext());
        }
        return *inserter.first->second;
    }

    void SetGlobalProperties(QDemonRenderShaderProgram &inProgram, const SLayer & /*inLayer*/
                             ,
                             SCamera &inCamera, QVector3D inCameraDirection,
                             QDemonDataRef<SLight *> inLights, QDemonDataRef<QVector3D> inLightDirections,
                             QDemonRenderShadowMap *inShadowMapManager)
    {
        SShaderGeneratorGeneratedShader &shader(GetShaderForProgram(inProgram));
        m_RenderContext.GetRenderContext().SetActiveShader(&inProgram);

        m_ShadowMapManager = inShadowMapManager;

        SCamera &theCamera(inCamera);
        shader.m_CameraPosition.Set(theCamera.GetGlobalPos());
        shader.m_CameraDirection.Set(inCameraDirection);

        QMatrix4x4 viewProj;
        if (shader.m_ViewProj.IsValid()) {
            theCamera.CalculateViewProjectionMatrix(viewProj);
            shader.m_ViewProj.Set(viewProj);
        }

        if (shader.m_ViewMatrix.IsValid()) {
            viewProj = theCamera.m_GlobalTransform.getInverse();
            shader.m_ViewMatrix.Set(viewProj);
        }

        // update the constant buffer
        shader.m_AoShadowParams.Set();
        // We can't cache light properties because they can change per object.
        QVector3D theLightAmbientTotal = QVector3D(0, 0, 0);
        size_t numShaderLights = shader.m_Lights.size();
        size_t numShadowLights = shader.m_ShadowMaps.size();
        for (quint32 lightIdx = 0, shadowMapIdx = 0, lightEnd = inLights.size();
             lightIdx < lightEnd && lightIdx < QDEMON_MAX_NUM_LIGHTS; ++lightIdx) {
            SLight *theLight(inLights[lightIdx]);
            if (lightIdx >= numShaderLights) {
                shader.m_Lights.push_back(SShaderLightProperties());
                ++numShaderLights;
            }
            if (shadowMapIdx >= numShadowLights && numShadowLights < QDEMON_MAX_NUM_SHADOWS) {
                if (theLight->m_Scope == nullptr && theLight->m_CastShadow) {
                    // PKC TODO : Fix multiple shadow issues.
                    // Need to know when the list of lights changes order, and clear shadow maps
                    // when that happens.
                    SetupShadowMapVariableNames(lightIdx);
                    shader.m_ShadowMaps.push_back(SShadowMapProperties(
                                                      m_ShadowMapStem.c_str(), m_ShadowCubeStem.c_str(),
                                                      m_ShadowMatrixStem.c_str(), m_ShadowControlStem.c_str(), inProgram));
                }
            }
            Q_ASSERT(lightIdx < numShaderLights);
            SShaderLightProperties &theLightProperties(shader.m_Lights[lightIdx]);
            float brightness = TranslateConstantAttenuation(theLight->m_Brightness);

            // setup light data
            theLightProperties.m_LightColor = theLight->m_DiffuseColor * brightness;
            theLightProperties.m_LightData.m_specular =
                    QVector4D(theLight->m_SpecularColor * brightness, 1.0);
            theLightProperties.m_LightData.m_direction = QVector4D(inLightDirections[lightIdx], 1.0);

            // TODO : This does potentially mean that we can create more shadow map entries than
            // we can actually use at once.
            if ((theLight->m_Scope == nullptr) && (theLight->m_CastShadow && inShadowMapManager)) {
                SShadowMapProperties &theShadowMapProperties(shader.m_ShadowMaps[shadowMapIdx++]);
                SShadowMapEntry *pEntry = inShadowMapManager->GetShadowMapEntry(lightIdx);
                if (pEntry) {
                    // add fixed scale bias matrix
                    QMatrix4x4 bias(QVector4D(0.5, 0.0, 0.0, 0.0), QVector4D(0.0, 0.5, 0.0, 0.0),
                                    QVector4D(0.0, 0.0, 0.5, 0.0), QVector4D(0.5, 0.5, 0.5, 1.0));

                    if (theLight->m_LightType != RenderLightTypes::Directional) {
                        theShadowMapProperties.m_ShadowCubeTexture.Set(pEntry->m_DepthCube);
                        theShadowMapProperties.m_ShadowmapMatrix.Set(pEntry->m_LightView);
                    } else {
                        theShadowMapProperties.m_ShadowmapTexture.Set(pEntry->m_DepthMap);
                        theShadowMapProperties.m_ShadowmapMatrix.Set(bias * pEntry->m_LightVP);
                    }

                    theShadowMapProperties.m_ShadowmapSettings.Set(
                                QVector4D(theLight->m_ShadowBias, theLight->m_ShadowFactor,
                                          theLight->m_ShadowMapFar, 0.0f));
                } else {
                    // if we have a light casting shadow we should find an entry
                    Q_ASSERT(false);
                }
            }

            if (theLight->m_LightType == RenderLightTypes::Point) {
                theLightProperties.m_LightData.m_position = QVector4D(theLight->GetGlobalPos(), 1.0);
                theLightProperties.m_LightData.m_constantAttenuation = 1.0;
                theLightProperties.m_LightData.m_linearAttenuation =
                        TranslateLinearAttenuation(theLight->m_LinearFade);
                theLightProperties.m_LightData.m_quadraticAttenuation =
                        TranslateQuadraticAttenuation(theLight->m_ExponentialFade);
            } else if (theLight->m_LightType == RenderLightTypes::Area) {
                theLightProperties.m_LightData.m_position = QVector4D(theLight->GetGlobalPos(), 1.0);

                QVector3D upDir = theLight->m_GlobalTransform.getUpper3x3().transform(QVector3D(0, 1, 0));
                QVector3D rtDir = theLight->m_GlobalTransform.getUpper3x3().transform(QVector3D(1, 0, 0));

                theLightProperties.m_LightData.m_up = QVector4D(upDir, theLight->m_AreaHeight);
                theLightProperties.m_LightData.m_right = QVector4D(rtDir, theLight->m_AreaWidth);
            }
            theLightAmbientTotal += theLight->m_AmbientColor;
        }
        shader.m_LightAmbientTotal = theLightAmbientTotal;
    }

    // Also sets the blend function on the render context.
    void SetMaterialProperties(QDemonRenderShaderProgram &inProgram, const SDefaultMaterial &inMaterial,
                               const QVector2D &inCameraVec, const QMatrix4x4 &inModelViewProjection,
                               const QMatrix3x3 &inNormalMatrix, const QMatrix4x4 &inGlobalTransform,
                               SRenderableImage *inFirstImage, float inOpacity,
                               QDemonRenderTexture2D *inDepthTexture, QDemonRenderTexture2D *inSSaoTexture,
                               SImage *inLightProbe, SImage *inLightProbe2, float inProbeHorizon,
                               float inProbeBright, float inProbe2Window, float inProbe2Pos,
                               float inProbe2Fade, float inProbeFOV)
    {

        QDemonRenderContext &context(m_RenderContext.GetRenderContext());
        SShaderGeneratorGeneratedShader &shader(GetShaderForProgram(inProgram));
        shader.m_MVP.Set(inModelViewProjection);
        shader.m_NormalMatrix.Set(inNormalMatrix);
        shader.m_GlobalTransform.Set(inGlobalTransform);
        shader.m_DepthTexture.Set(inDepthTexture);

        shader.m_AOTexture.Set(inSSaoTexture);

        SImage *theLightProbe = inLightProbe;
        SImage *theLightProbe2 = inLightProbe2;

        // If the material has its own IBL Override, we should use that image instead.
        if ((inMaterial.m_IblProbe) && (inMaterial.m_IblProbe->m_TextureData.m_Texture)) {
            theLightProbe = inMaterial.m_IblProbe;
        }

        if (theLightProbe) {
            if (theLightProbe->m_TextureData.m_Texture) {
                QDemonRenderTextureCoordOp::Enum theHorzLightProbeTilingMode =
                        QDemonRenderTextureCoordOp::Repeat;
                QDemonRenderTextureCoordOp::Enum theVertLightProbeTilingMode =
                        theLightProbe->m_VerticalTilingMode;
                theLightProbe->m_TextureData.m_Texture->SetTextureWrapS(
                            theHorzLightProbeTilingMode);
                theLightProbe->m_TextureData.m_Texture->SetTextureWrapT(
                            theVertLightProbeTilingMode);
                const QMatrix4x4 &textureTransform = theLightProbe->m_TextureTransform;
                // We separate rotational information from offset information so that just maybe the
                // shader
                // will attempt to push less information to the card.
                const float *dataPtr(textureTransform.front());
                // The third member of the offsets contains a flag indicating if the texture was
                // premultiplied or not.
                // We use this to mix the texture alpha.
                QVector4D offsets(dataPtr[12], dataPtr[13],
                        theLightProbe->m_TextureData.m_TextureFlags.IsPreMultiplied() ? 1.0f
                                                                                      : 0.0f,
                        (float)theLightProbe->m_TextureData.m_Texture->GetNumMipmaps());

                // Grab just the upper 2x2 rotation matrix from the larger matrix.
                QVector4D rotations(dataPtr[0], dataPtr[4], dataPtr[1], dataPtr[5]);

                shader.m_LightProbeRot.Set(rotations);
                shader.m_LightProbeOfs.Set(offsets);

                if ((!inMaterial.m_IblProbe) && (inProbeFOV < 180.f)) {
                    shader.m_LightProbeOpts.Set(
                                QVector4D(0.01745329251994329547f * inProbeFOV, 0.0f, 0.0f, 0.0f));
                }

                // Also make sure to add the secondary texture, but it should only be added if the
                // primary
                // (i.e. background) texture is also there.
                if (theLightProbe2 && theLightProbe2->m_TextureData.m_Texture) {
                    theLightProbe2->m_TextureData.m_Texture->SetTextureWrapS(
                                theHorzLightProbeTilingMode);
                    theLightProbe2->m_TextureData.m_Texture->SetTextureWrapT(
                                theVertLightProbeTilingMode);
                    shader.m_LightProbe2.Set(theLightProbe2->m_TextureData.m_Texture);
                    shader.m_LightProbe2Props.Set(
                                QVector4D(inProbe2Window, inProbe2Pos, inProbe2Fade, 1.0f));

                    const QMatrix4x4 &xform2 = theLightProbe2->m_TextureTransform;
                    const float *dataPtr(xform2.front());
                    shader.m_LightProbeProps.Set(
                                QVector4D(dataPtr[12], dataPtr[13], inProbeHorizon, inProbeBright * 0.01f));
                } else {
                    shader.m_LightProbe2Props.Set(QVector4D(0.0f, 0.0f, 0.0f, 0.0f));
                    shader.m_LightProbeProps.Set(
                                QVector4D(0.0f, 0.0f, inProbeHorizon, inProbeBright * 0.01f));
                }
                QDemonRenderTexture2D *textureImage = theLightProbe->m_TextureData.m_Texture;
                shader.m_LightProbe.Set(textureImage);
                shader.m_LightProbeSize.Set(QVector2D(textureImage->GetTextureDetails().m_Width,
                                                      textureImage->GetTextureDetails().m_Height));
            } else {
                shader.m_LightProbeProps.Set(QVector4D(0.0f, 0.0f, -1.0f, 0.0f));
                shader.m_LightProbe2Props.Set(QVector4D(0.0f, 0.0f, 0.0f, 0.0f));
            }
        } else {
            shader.m_LightProbeProps.Set(QVector4D(0.0f, 0.0f, -1.0f, 0.0f));
            shader.m_LightProbe2Props.Set(QVector4D(0.0f, 0.0f, 0.0f, 0.0f));
        }

        float emissivePower = 1.0;

        quint32 hasLighting = inMaterial.m_Lighting != DefaultMaterialLighting::NoLighting;
        if (hasLighting)
            emissivePower = inMaterial.m_EmissivePower / 100.0f;

        QVector4D material_diffuse = QVector4D(inMaterial.m_EmissiveColor[0] * emissivePower,
                inMaterial.m_EmissiveColor[1] * emissivePower,
                inMaterial.m_EmissiveColor[2] * emissivePower, inOpacity);
        shader.m_MaterialDiffuse.Set(material_diffuse);
        shader.m_DiffuseColor.Set(inMaterial.m_DiffuseColor);
        QVector4D material_specular =
                QVector4D(inMaterial.m_SpecularTint[0], inMaterial.m_SpecularTint[1],
                inMaterial.m_SpecularTint[2], inMaterial.m_IOR);
        shader.m_MaterialSpecular.Set(material_specular);
        shader.m_CameraProperties.Set(inCameraVec);
        shader.m_FresnelPower.Set(inMaterial.m_FresnelPower);

        if (context.GetConstantBufferSupport()) {
            QDemonRenderConstantBuffer *pLightCb = GetLightConstantBuffer(shader.m_Lights.size());
            // if we have lights we need a light buffer
            Q_ASSERT(shader.m_Lights.size() == 0 || pLightCb);

            for (quint32 idx = 0, end = shader.m_Lights.size(); idx < end && pLightCb; ++idx) {
                shader.m_Lights[idx].m_LightData.m_diffuse =
                        QVector4D(shader.m_Lights[idx].m_LightColor.x * inMaterial.m_DiffuseColor.x,
                                  shader.m_Lights[idx].m_LightColor.y * inMaterial.m_DiffuseColor.y,
                                  shader.m_Lights[idx].m_LightColor.z * inMaterial.m_DiffuseColor.z, 1.0);

                // this is our final change update memory
                pLightCb->UpdateRaw(idx * sizeof(SLightSourceShader) + (4 * sizeof(qint32)),
                                    QDemonDataRef<quint8>((quint8 *)&shader.m_Lights[idx].m_LightData,
                                                          sizeof(SLightSourceShader)));
            }
            // update light buffer to hardware
            if (pLightCb) {
                qint32 cgLights = shader.m_Lights.size();
                pLightCb->UpdateRaw(0, QDemonDataRef<quint8>((quint8 *)&cgLights, sizeof(qint32)));
                shader.m_LightsBuffer.Set();
            }
        } else {
            SLightConstantProperties<SShaderGeneratorGeneratedShader> *pLightConstants
                    = GetLightConstantProperties(shader);

            // if we have lights we need a light buffer
            Q_ASSERT(shader.m_Lights.size() == 0 || pLightConstants);

            for (quint32 idx = 0, end = shader.m_Lights.size();
                 idx < end && pLightConstants; ++idx) {
                shader.m_Lights[idx].m_LightData.m_diffuse =
                        QVector4D(shader.m_Lights[idx].m_LightColor.x * inMaterial.m_DiffuseColor.x,
                                  shader.m_Lights[idx].m_LightColor.y * inMaterial.m_DiffuseColor.y,
                                  shader.m_Lights[idx].m_LightColor.z * inMaterial.m_DiffuseColor.z, 1.0);
            }
            // update light buffer to hardware
            if (pLightConstants)
                pLightConstants->updateLights(shader);
        }

        shader.m_MaterialDiffuseLightAmbientTotal.Set(
                    QVector3D(shader.m_LightAmbientTotal.x * inMaterial.m_DiffuseColor[0],
                    shader.m_LightAmbientTotal.y * inMaterial.m_DiffuseColor[1],
                shader.m_LightAmbientTotal.z * inMaterial.m_DiffuseColor[2]));

        shader.m_MaterialProperties.Set(QVector4D(
                                            inMaterial.m_SpecularAmount, inMaterial.m_SpecularRoughness, emissivePower, 0.0f));
        shader.m_BumpAmount.Set(inMaterial.m_BumpAmount);
        shader.m_DisplaceAmount.Set(inMaterial.m_DisplaceAmount);
        shader.m_TranslucentFalloff.Set(inMaterial.m_TranslucentFalloff);
        shader.m_DiffuseLightWrap.Set(inMaterial.m_DiffuseLightWrap);
        quint32 imageIdx = 0;
        for (SRenderableImage *theImage = inFirstImage; theImage;
             theImage = theImage->m_NextImage, ++imageIdx)
            SetImageShaderVariables(shader, *theImage, imageIdx);

        QDemonRenderBlendFunctionArgument blendFunc;
        QDemonRenderBlendEquationArgument blendEqua(QDemonRenderBlendEquation::Add,
                                                    QDemonRenderBlendEquation::Add);
        // The blend function goes:
        // src op
        // dst op
        // src alpha op
        // dst alpha op
        // All of our shaders produce non-premultiplied values.
        switch (inMaterial.m_BlendMode) {
        case DefaultMaterialBlendMode::Screen:
            blendFunc = QDemonRenderBlendFunctionArgument(
                        QDemonRenderSrcBlendFunc::SrcAlpha, QDemonRenderDstBlendFunc::One,
                        QDemonRenderSrcBlendFunc::One, QDemonRenderDstBlendFunc::One);
            break;
        case DefaultMaterialBlendMode::Multiply:
            blendFunc = QDemonRenderBlendFunctionArgument(
                        QDemonRenderSrcBlendFunc::DstColor, QDemonRenderDstBlendFunc::Zero,
                        QDemonRenderSrcBlendFunc::One, QDemonRenderDstBlendFunc::One);
            break;
        case DefaultMaterialBlendMode::Overlay:
            // SW fallback is not using blend equation
            // note blend func is not used here anymore
            if (context.IsAdvancedBlendHwSupported() || context.IsAdvancedBlendHwSupportedKHR())
                blendEqua = QDemonRenderBlendEquationArgument(
                            QDemonRenderBlendEquation::Overlay, QDemonRenderBlendEquation::Overlay);
            break;
        case DefaultMaterialBlendMode::ColorBurn:
            // SW fallback is not using blend equation
            // note blend func is not used here anymore
            if (context.IsAdvancedBlendHwSupported() || context.IsAdvancedBlendHwSupportedKHR())
                blendEqua = QDemonRenderBlendEquationArgument(
                            QDemonRenderBlendEquation::ColorBurn, QDemonRenderBlendEquation::ColorBurn);
            break;
        case DefaultMaterialBlendMode::ColorDodge:
            // SW fallback is not using blend equation
            // note blend func is not used here anymore
            if (context.IsAdvancedBlendHwSupported() || context.IsAdvancedBlendHwSupportedKHR())
                blendEqua = QDemonRenderBlendEquationArgument(
                            QDemonRenderBlendEquation::ColorDodge, QDemonRenderBlendEquation::ColorDodge);
            break;
        default:
            blendFunc = QDemonRenderBlendFunctionArgument(
                        QDemonRenderSrcBlendFunc::SrcAlpha, QDemonRenderDstBlendFunc::OneMinusSrcAlpha,
                        QDemonRenderSrcBlendFunc::One, QDemonRenderDstBlendFunc::OneMinusSrcAlpha);
            break;
        }
        context.SetBlendFunction(blendFunc);
        context.SetBlendEquation(blendEqua);
    }
    void SetMaterialProperties(QDemonRenderShaderProgram &inProgram,
                               const SGraphObject &inMaterial, const QVector2D &inCameraVec,
                               const QMatrix4x4 &inModelViewProjection,
                               const QMatrix3x3 &inNormalMatrix,
                               const QMatrix4x4 &inGlobalTransform,
                               SRenderableImage *inFirstImage, float inOpacity,
                               SLayerGlobalRenderProperties inRenderProperties) override
    {
        const SDefaultMaterial &theMaterial(static_cast<const SDefaultMaterial &>(inMaterial));
        Q_ASSERT(inMaterial.m_Type == GraphObjectTypes::DefaultMaterial);

        SetGlobalProperties(inProgram, inRenderProperties.m_Layer, inRenderProperties.m_Camera,
                            inRenderProperties.m_CameraDirection, inRenderProperties.m_Lights,
                            inRenderProperties.m_LightDirections,
                            inRenderProperties.m_ShadowMapManager);
        SetMaterialProperties(inProgram, theMaterial, inCameraVec, inModelViewProjection,
                              inNormalMatrix, inGlobalTransform, inFirstImage, inOpacity,
                              inRenderProperties.m_DepthTexture, inRenderProperties.m_SSaoTexture,
                              inRenderProperties.m_LightProbe, inRenderProperties.m_LightProbe2,
                              inRenderProperties.m_ProbeHorizon, inRenderProperties.m_ProbeBright,
                              inRenderProperties.m_Probe2Window, inRenderProperties.m_Probe2Pos,
                              inRenderProperties.m_Probe2Fade, inRenderProperties.m_ProbeFOV);
    }

    SLightConstantProperties<SShaderGeneratorGeneratedShader> *GetLightConstantProperties(SShaderGeneratorGeneratedShader &shader)
    {
        if (!shader.m_lightConstantProperties
                || int(shader.m_Lights.size())
                > shader.m_lightConstantProperties->m_constants.size()) {
            if (shader.m_lightConstantProperties)
                delete shader.m_lightConstantProperties;
            shader.m_lightConstantProperties
                    = new SLightConstantProperties<SShaderGeneratorGeneratedShader>(
                        shader, m_LightsAsSeparateUniforms);
        }
        return shader.m_lightConstantProperties;
    }
};
}

IDefaultMaterialShaderGenerator &
IDefaultMaterialShaderGenerator::CreateDefaultMaterialShaderGenerator(IQDemonRenderContext &inRc)
{
    return *new SShaderGenerator(inRc);
}

QT_END_NAMESPACE
