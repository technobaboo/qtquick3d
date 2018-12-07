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
#include <qdemonrendercustommaterialshadergenerator.h>
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
#include <qdemonrendercustommaterialsystem.h>
#include <QtDemonRuntimeRender/qdemonrenderlightconstantproperties.h>

QT_BEGIN_NAMESPACE

using NVRenderCachedShaderProperty;
using NVRenderCachedShaderBuffer;

namespace {
struct SShaderLightProperties
{
    QDemonScopedRefCounted<QDemonRenderShaderProgram> m_Shader;
    RenderLightTypes::Enum m_LightType;
    SLightSourceShader m_LightData;
    volatile qint32 mRefCount;

    SShaderLightProperties(QDemonRenderShaderProgram &inShader)
        : m_Shader(inShader)
        , m_LightType(RenderLightTypes::Directional)
        , mRefCount(0)
    {
    }

    void Set(const SLight *inLight)
    {
        QVector3D dir(0, 0, 1);
        if (inLight->m_LightType == RenderLightTypes::Directional) {
            dir = inLight->GetScalingCorrectDirection();
            // we lit in world sapce
            dir *= -1;
            m_LightData.m_position = QVector4D(dir, 0.0);
        } else if (inLight->m_LightType == RenderLightTypes::Area) {
            dir = inLight->GetScalingCorrectDirection();
            m_LightData.m_position = QVector4D(inLight->GetGlobalPos(), 1.0);
        } else {
            dir = inLight->GetGlobalPos();
            m_LightData.m_position = QVector4D(dir, 1.0);
        }

        m_LightType = inLight->m_LightType;

        m_LightData.m_direction = QVector4D(dir, 0.0);

        float normalizedBrightness = inLight->m_Brightness / 100.0f;
        m_LightData.m_diffuse = QVector4D(inLight->m_DiffuseColor * normalizedBrightness, 1.0);
        m_LightData.m_specular = QVector4D(inLight->m_SpecularColor * normalizedBrightness, 1.0);

        if (inLight->m_LightType == RenderLightTypes::Area) {
            m_LightData.m_width = inLight->m_AreaWidth;
            m_LightData.m_height = inLight->m_AreaWidth;

            QMatrix3x3 theDirMatrix(inLight->m_GlobalTransform.getUpper3x3());
            m_LightData.m_right =
                    QVector4D(theDirMatrix.transform(QVector3D(1, 0, 0)), inLight->m_AreaWidth);
            m_LightData.m_up =
                    QVector4D(theDirMatrix.transform(QVector3D(0, 1, 0)), inLight->m_AreaHeight);
        } else {
            m_LightData.m_width = 0.0;
            m_LightData.m_height = 0.0;
            m_LightData.m_right = QVector4D(0.0f);
            m_LightData.m_up = QVector4D(0.0f);

            // These components only apply to CG lights
            m_LightData.m_ambient = QVector4D(inLight->m_AmbientColor, 1.0);

            m_LightData.m_constantAttenuation = 1.0;
            m_LightData.m_linearAttenuation = inLight->m_LinearFade;
            m_LightData.m_quadraticAttenuation = inLight->m_ExponentialFade;
            m_LightData.m_spotCutoff = 180.0;
        }

        if (m_LightType == RenderLightTypes::Point) {
            m_LightData.m_shadowView = QMatrix4x4::createIdentity();
        } else {
            m_LightData.m_shadowView = inLight->m_GlobalTransform;
        }
    }

    QDEMON_IMPLEMENT_REF_COUNT_ADDREF_RELEASE(m_Shader->GetRenderContext().GetAllocator())

    static SShaderLightProperties CreateLightEntry(QDemonRenderShaderProgram &inShader)
    {
        return SShaderLightProperties(inShader);
    }
};

/**
 *	Cached texture property lookups, used one per texture so a shader generator for N
 *	textures will have an array of N of these lookup objects.
 */
struct SShaderTextureProperties
{
    NVRenderCachedShaderProperty<QDemonRenderTexture2D *> m_Sampler;
    NVRenderCachedShaderProperty<QVector3D> m_Offsets;
    NVRenderCachedShaderProperty<QVector4D> m_Rotations;
    SShaderTextureProperties(const char *sampName, const char *offName, const char *rotName,
                             QDemonRenderShaderProgram &inShader)
        : m_Sampler(sampName, inShader)
        , m_Offsets(offName, inShader)
        , m_Rotations(rotName, inShader)
    {
    }
    SShaderTextureProperties() {}
};

/* We setup some shared state on the custom material shaders */
struct SShaderGeneratorGeneratedShader
{
    typedef QHash<quint32, SShaderTextureProperties> TCustomMaterialImagMap;

    NVAllocatorCallback &m_Allocator;
    QDemonRenderShaderProgram &m_Shader;
    // Specific properties we know the shader has to have.
    NVRenderCachedShaderProperty<QMatrix4x4> m_ModelMatrix;
    NVRenderCachedShaderProperty<QMatrix4x4> m_ViewProjMatrix;
    NVRenderCachedShaderProperty<QMatrix4x4> m_ViewMatrix;
    NVRenderCachedShaderProperty<QMatrix3x3> m_NormalMatrix;
    NVRenderCachedShaderProperty<QVector3D> m_CameraPos;
    NVRenderCachedShaderProperty<QMatrix4x4> m_ProjMatrix;
    NVRenderCachedShaderProperty<QMatrix4x4> m_ViewportMatrix;
    NVRenderCachedShaderProperty<QVector2D> m_CamProperties;
    NVRenderCachedShaderProperty<QDemonRenderTexture2D *> m_DepthTexture;
    NVRenderCachedShaderProperty<QDemonRenderTexture2D *> m_AOTexture;
    NVRenderCachedShaderProperty<QDemonRenderTexture2D *> m_LightProbe;
    NVRenderCachedShaderProperty<QVector4D> m_LightProbeProps;
    NVRenderCachedShaderProperty<QVector4D> m_LightProbeOpts;
    NVRenderCachedShaderProperty<QVector4D> m_LightProbeRot;
    NVRenderCachedShaderProperty<QVector4D> m_LightProbeOfs;
    NVRenderCachedShaderProperty<QDemonRenderTexture2D *> m_LightProbe2;
    NVRenderCachedShaderProperty<QVector4D> m_LightProbe2Props;
    NVRenderCachedShaderProperty<qint32> m_LightCount;
    NVRenderCachedShaderProperty<qint32> m_AreaLightCount;
    NVRenderCachedShaderProperty<qint32> m_ShadowMapCount;
    NVRenderCachedShaderProperty<qint32> m_ShadowCubeCount;
    NVRenderCachedShaderProperty<float> m_Opacity;
    NVRenderCachedShaderBuffer<QDemonRenderShaderConstantBuffer *> m_AoShadowParams;
    NVRenderCachedShaderBuffer<QDemonRenderShaderConstantBuffer *> m_LightsBuffer;
    NVRenderCachedShaderBuffer<QDemonRenderShaderConstantBuffer *> m_AreaLightsBuffer;

    SLightConstantProperties<SShaderGeneratorGeneratedShader> *m_lightsProperties;
    SLightConstantProperties<SShaderGeneratorGeneratedShader> *m_areaLightsProperties;

    typedef NVRenderCachedShaderPropertyArray<QDemonRenderTexture2D *,
    QDEMON_MAX_NUM_SHADOWS> ShadowMapPropertyArray;
    typedef NVRenderCachedShaderPropertyArray<QDemonRenderTextureCube *,
    QDEMON_MAX_NUM_SHADOWS> ShadowCubePropertyArray;

    ShadowMapPropertyArray m_shadowMaps;
    ShadowCubePropertyArray m_shadowCubes;

    // Cache the image property name lookups
    TCustomMaterialImagMap m_Images; // Images external to custom material usage
    volatile qint32 m_RefCount;

    SShaderGeneratorGeneratedShader(QDemonRenderShaderProgram &inShader, QDemonRenderContext &inContext)
        : m_Allocator(inContext.GetAllocator())
        , m_Shader(inShader)
        , m_ModelMatrix("model_matrix", inShader)
        , m_ViewProjMatrix("model_view_projection", inShader)
        , m_ViewMatrix("view_matrix", inShader)
        , m_NormalMatrix("normal_matrix", inShader)
        , m_CameraPos("camera_position", inShader)
        , m_ProjMatrix("view_projection_matrix", inShader)
        , m_ViewportMatrix("viewport_matrix", inShader)
        , m_CamProperties("camera_properties", inShader)
        , m_DepthTexture("depth_sampler", inShader)
        , m_AOTexture("ao_sampler", inShader)
        , m_LightProbe("light_probe", inShader)
        , m_LightProbeProps("light_probe_props", inShader)
        , m_LightProbeOpts("light_probe_opts", inShader)
        , m_LightProbeRot("light_probe_rotation", inShader)
        , m_LightProbeOfs("light_probe_offset", inShader)
        , m_LightProbe2("light_probe2", inShader)
        , m_LightProbe2Props("light_probe2_props", inShader)
        , m_LightCount("uNumLights", inShader)
        , m_AreaLightCount("uNumAreaLights", inShader)
        , m_ShadowMapCount("uNumShadowMaps", inShader)
        , m_ShadowCubeCount("uNumShadowCubes", inShader)
        , m_Opacity("object_opacity", inShader)
        , m_AoShadowParams("cbAoShadow", inShader)
        , m_LightsBuffer("cbBufferLights", inShader)
        , m_AreaLightsBuffer("cbBufferAreaLights", inShader)
        , m_lightsProperties(nullptr)
        , m_areaLightsProperties(nullptr)
        , m_shadowMaps("shadowMaps[0]", inShader)
        , m_shadowCubes("shadowCubes[0]", inShader)
        , m_Images(inContext.GetAllocator(), "SShaderGeneratorGeneratedShader::m_Images")
        , m_RefCount(0)
    {
        m_Shader.addRef();
    }

    ~SShaderGeneratorGeneratedShader()
    {
        m_Shader.release();
        delete m_lightsProperties;
        delete m_areaLightsProperties;
    }

    void addRef() { ++m_RefCount; }
    void release()
    {
        --m_RefCount;
        if (m_RefCount <= 0) {
            NVAllocatorCallback &alloc(m_Allocator);
            NVDelete(alloc, this);
        }
    }

    SLightConstantProperties<SShaderGeneratorGeneratedShader> *GetLightProperties(int count)
    {
        if (!m_lightsProperties || m_areaLightsProperties->m_lightCountInt < count) {
            if (m_lightsProperties)
                delete m_lightsProperties;
            m_lightsProperties = new SLightConstantProperties<SShaderGeneratorGeneratedShader>
                    ("lights", "uNumLights", *this, false, count);
        }
        return m_lightsProperties;
    }
    SLightConstantProperties<SShaderGeneratorGeneratedShader> *GetAreaLightProperties(int count)
    {
        if (!m_areaLightsProperties || m_areaLightsProperties->m_lightCountInt < count) {
            if (m_areaLightsProperties)
                delete m_areaLightsProperties;
            m_areaLightsProperties = new SLightConstantProperties<SShaderGeneratorGeneratedShader>
                    ("areaLights", "uNumAreaLights", *this, false, count);
        }
        return m_areaLightsProperties;
    }
};

struct SShaderGenerator : public ICustomMaterialShaderGenerator
{
    typedef CRenderString TStrType;
    typedef QHash<QDemonRenderShaderProgram *, QDemonScopedRefCounted<SShaderGeneratorGeneratedShader>>
    TProgramToShaderMap;
    typedef QPair<size_t, QDemonScopedRefCounted<SShaderLightProperties>>
    TCustomMaterialLightEntry;
    typedef QPair<size_t, NVRenderCachedShaderProperty<QDemonRenderTexture2D *>> TShadowMapEntry;
    typedef QPair<size_t, NVRenderCachedShaderProperty<QDemonRenderTextureCube *>>
    TShadowCubeEntry;
    typedef QHash<QString,
    QDemonScopedRefCounted<QDemonRenderConstantBuffer>>
    TStrConstanBufMap;

    IQDemonRenderContext &m_RenderContext;
    IShaderProgramGenerator &m_ProgramGenerator;

    const SCustomMaterial *m_CurrentMaterial;
    SShaderDefaultMaterialKey *m_CurrentKey;
    IDefaultMaterialVertexPipeline *m_CurrentPipeline;
    TShaderFeatureSet m_CurrentFeatureSet;
    QDemonDataRef<SLight *> m_Lights;
    SRenderableImage *m_FirstImage;
    bool m_HasTransparency;

    TStrType m_ImageStem;
    TStrType m_ImageSampler;
    TStrType m_ImageFragCoords;
    TStrType m_ImageRotScale;
    TStrType m_ImageOffset;

    QString m_GeneratedShaderString;

    SShaderDefaultMaterialKeyProperties m_DefaultMaterialShaderKeyProperties;
    TProgramToShaderMap m_ProgramToShaderMap;

    QVector<TCustomMaterialLightEntry> m_LightEntries;

    TStrConstanBufMap m_ConstantBuffers; ///< store all constants buffers

    qint32 m_RefCount;

    SShaderGenerator(IQDemonRenderContext &inRc)
        : m_RenderContext(inRc)
        , m_ProgramGenerator(m_RenderContext.GetShaderProgramGenerator())
        , m_CurrentMaterial(nullptr)
        , m_CurrentKey(nullptr)
        , m_CurrentPipeline(nullptr)
        , m_FirstImage(nullptr)
        , m_HasTransparency(false)
        , m_ProgramToShaderMap(inRc.GetAllocator(), "m_ProgramToShaderMap")
        , m_LightEntries(inRc.GetAllocator(), "m_LightEntries")
        , m_ConstantBuffers(inRc.GetAllocator(), "m_ConstantBuffers")
        , m_RefCount(0)
    {
    }

    void addRef() override { atomicIncrement(&m_RefCount); }
    void release() override
    {
        atomicDecrement(&m_RefCount);
        if (m_RefCount <= 0) {
            m_ConstantBuffers.clear();
            NVDelete(m_RenderContext.GetAllocator(), this);
        }
    }

    IShaderProgramGenerator &ProgramGenerator() { return m_ProgramGenerator; }
    IDefaultMaterialVertexPipeline &VertexGenerator() { return *m_CurrentPipeline; }
    IShaderStageGenerator &FragmentGenerator()
    {
        return *m_ProgramGenerator.GetStage(ShaderGeneratorStages::Fragment);
    }
    SShaderDefaultMaterialKey &Key() { return *m_CurrentKey; }
    const SCustomMaterial &Material() { return *m_CurrentMaterial; }
    TShaderFeatureSet FeatureSet() { return m_CurrentFeatureSet; }
    bool HasTransparency() { return m_HasTransparency; }

    quint32
    ConvertTextureTypeValue(ImageMapTypes::Enum inType)
    {
        QDemonRenderTextureTypeValue::Enum retVal = QDemonRenderTextureTypeValue::Unknown;

        switch (inType) {
        case ImageMapTypes::LightmapIndirect:
            retVal = QDemonRenderTextureTypeValue::LightmapIndirect;
            break;
        case ImageMapTypes::LightmapRadiosity:
            retVal = QDemonRenderTextureTypeValue::LightmapRadiosity;
            break;
        case ImageMapTypes::LightmapShadow:
            retVal = QDemonRenderTextureTypeValue::LightmapShadow;
            break;
        case ImageMapTypes::Bump:
            retVal = QDemonRenderTextureTypeValue::Bump;
            break;
        case ImageMapTypes::Diffuse:
            retVal = QDemonRenderTextureTypeValue::Diffuse;
            break;
        case ImageMapTypes::Displacement:
            retVal = QDemonRenderTextureTypeValue::Displace;
            break;
        default:
            retVal = QDemonRenderTextureTypeValue::Unknown;
            break;
        }

        Q_ASSERT(retVal != QDemonRenderTextureTypeValue::Unknown);

        return (quint32)retVal;
    }

    SImageVariableNames GetImageVariableNames(quint32 imageIdx) override
    {
        // convert to QDemonRenderTextureTypeValue
        QDemonRenderTextureTypeValue::Enum texType = (QDemonRenderTextureTypeValue::Enum)imageIdx;
        m_ImageStem = QDemonRenderTextureTypeValue::toString(texType);
        m_ImageStem.append("_");
        m_ImageSampler = m_ImageStem;
        m_ImageSampler.append("sampler");
        m_ImageFragCoords = m_ImageStem;
        m_ImageFragCoords.append("uv_coords");
        m_ImageRotScale = m_ImageStem;
        m_ImageRotScale.append("rot_scale");
        m_ImageOffset = m_ImageStem;
        m_ImageOffset.append("offset");

        SImageVariableNames retVal;
        retVal.m_ImageSampler = m_ImageSampler.c_str();
        retVal.m_ImageFragCoords = m_ImageFragCoords.c_str();
        return retVal;
    }

    void SetImageShaderVariables(SShaderGeneratorGeneratedShader &inShader,
                                 SRenderableImage &inImage)
    {
        // skip displacement and emissive mask maps which are handled differently
        if (inImage.m_MapType == ImageMapTypes::Displacement
                || inImage.m_MapType == ImageMapTypes::Emissive)
            return;

        SShaderGeneratorGeneratedShader::TCustomMaterialImagMap::iterator iter =
                inShader.m_Images.find(inImage.m_MapType);
        if (iter == inShader.m_Images.end()) {
            SImageVariableNames names =
                    GetImageVariableNames(ConvertTextureTypeValue(inImage.m_MapType));
            inShader.m_Images.insert(
                                         (quint32)inImage.m_MapType,
                                         SShaderTextureProperties(names.m_ImageSampler, m_ImageOffset.c_str(),
                                                                  m_ImageRotScale.c_str(), inShader.m_Shader));
            iter = inShader.m_Images.find(inImage.m_MapType);
        }

        SShaderTextureProperties &theShaderProps = iter->second;
        const QMatrix4x4 &textureTransform = inImage.m_Image.m_TextureTransform;
        const float *dataPtr(textureTransform.front());
        QVector3D offsets(dataPtr[12], dataPtr[13], 0.0f);
        // Grab just the upper 2x2 rotation matrix from the larger matrix.
        QVector4D rotations(dataPtr[0], dataPtr[4], dataPtr[1], dataPtr[5]);

        // The image horizontal and vertical tiling modes need to be set here, before we set texture
        // on the shader.
        // because setting the image on the texture forces the textue to bind and immediately apply
        // any tex params.
        inImage.m_Image.m_TextureData.m_Texture->SetTextureWrapS(
                    inImage.m_Image.m_HorizontalTilingMode);
        inImage.m_Image.m_TextureData.m_Texture->SetTextureWrapT(
                    inImage.m_Image.m_VerticalTilingMode);

        theShaderProps.m_Sampler.Set(inImage.m_Image.m_TextureData.m_Texture);
        theShaderProps.m_Offsets.Set(offsets);
        theShaderProps.m_Rotations.Set(rotations);
    }

    void GenerateImageUVCoordinates(IShaderStageGenerator &, quint32, quint32, SRenderableImage &) override {}

    ///< get the light constant buffer and generate if necessary
    QDemonRenderConstantBuffer *GetLightConstantBuffer(const char *name, quint32 inLightCount)
    {
        QDemonRenderContext &theContext(m_RenderContext.GetRenderContext());

        // we assume constant buffer support
        Q_ASSERT(theContext.GetConstantBufferSupport());
        // we only create if if we have lights
        if (!inLightCount || !theContext.GetConstantBufferSupport())
            return nullptr;

        QString theName = theContext.GetStringTable().RegisterStr(name);
        QDemonRenderConstantBuffer *pCB = theContext.GetConstantBuffer(theName);

        if (!pCB) {
            // create with size of all structures + int for light count
            SLightSourceShader s[QDEMON_MAX_NUM_LIGHTS];
            QDemonDataRef<quint8> cBuffer((quint8 *)&s, (sizeof(SLightSourceShader) * QDEMON_MAX_NUM_LIGHTS)
                                          + (4 * sizeof(qint32)));
            pCB = theContext.CreateConstantBuffer(
                        name, QDemonRenderBufferUsageType::Static,
                        (sizeof(SLightSourceShader) * QDEMON_MAX_NUM_LIGHTS) + (4 * sizeof(qint32)), cBuffer);
            if (!pCB) {
                Q_ASSERT(false);
                return nullptr;
            }
            // init first set
            memset(&s[0], 0x0, sizeof(SLightSourceShader) * QDEMON_MAX_NUM_LIGHTS);
            qint32 cgLights[4] = {0, 0, 0, 0};
            pCB->UpdateRaw(0, QDemonDataRef<quint8>((quint8 *)&cgLights, sizeof(qint32) * 4));
            pCB->UpdateRaw(4 * sizeof(qint32),
                           QDemonDataRef<quint8>((quint8 *)&s[0],
                           sizeof(SLightSourceShader) * QDEMON_MAX_NUM_LIGHTS));
            pCB->Update(); // update to hardware

            m_ConstantBuffers.insert(theName, pCB);
        }

        return pCB;
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

    SShaderGeneratorGeneratedShader &GetShaderForProgram(QDemonRenderShaderProgram &inProgram)
    {
        QPair<TProgramToShaderMap::iterator, bool> inserter =
                m_ProgramToShaderMap.insert(
                                                &inProgram, QDemonScopedRefCounted<SShaderGeneratorGeneratedShader>(nullptr));
        if (inserter.second) {
            NVAllocatorCallback &alloc(m_RenderContext.GetRenderContext().GetAllocator());
            inserter.first->second = QDEMON_NEW(alloc, SShaderGeneratorGeneratedShader)(
                        inProgram, m_RenderContext.GetRenderContext());
        }
        return *inserter.first->second;
    }

    virtual SShaderLightProperties *SetLight(QDemonRenderShaderProgram &inShader, size_t lightIdx,
                                             size_t shadeIdx, const SLight *inLight,
                                             SShadowMapEntry *inShadow, qint32 shadowIdx,
                                             float shadowDist)
    {
        SShaderLightProperties *theLightEntry(nullptr);
        for (quint32 idx = 0, end = m_LightEntries.size(); idx < end && theLightEntry == nullptr;
             ++idx) {
            if (m_LightEntries[idx].first == lightIdx
                    && m_LightEntries[idx].second->m_Shader.mPtr == &inShader
                    && m_LightEntries[idx].second->m_LightType == inLight->m_LightType) {
                theLightEntry = m_LightEntries[idx].second;
            }
        }
        if (theLightEntry == nullptr) {
            // create a new name
            QString lightName;
            if (inLight->m_LightType == RenderLightTypes::Area)
                lightName = "arealights";
            else
                lightName = "lights";
            char buf[16];
            _snprintf(buf, 16, "[%d]", int(shadeIdx));
            lightName.append(buf);

            QDemonScopedRefCounted<SShaderLightProperties> theNewEntry =
                    QDEMON_NEW(m_RenderContext.GetAllocator(),
                               SShaderLightProperties)(SShaderLightProperties::CreateLightEntry(inShader));
            m_LightEntries.push_back(TCustomMaterialLightEntry(lightIdx, theNewEntry));
            theLightEntry = theNewEntry.mPtr;
        }
        theLightEntry->Set(inLight);
        theLightEntry->m_LightData.m_shadowControls =
                QVector4D(inLight->m_ShadowBias, inLight->m_ShadowFactor, shadowDist, 0.0);
        theLightEntry->m_LightData.m_shadowIdx = (inShadow) ? shadowIdx : -1;

        return theLightEntry;
    }

    void SetShadowMaps(QDemonRenderShaderProgram &inProgram, SShadowMapEntry *inShadow,
                       qint32 &numShadowMaps, qint32 &numShadowCubes, bool shadowMap,
                       SShaderGeneratorGeneratedShader::ShadowMapPropertyArray &shadowMaps,
                       SShaderGeneratorGeneratedShader::ShadowCubePropertyArray &shadowCubes)
    {
        Q_UNUSED(inProgram)
        if (inShadow) {
            if (shadowMap == false && inShadow->m_DepthCube
                    && (numShadowCubes < QDEMON_MAX_NUM_SHADOWS)) {
                shadowCubes.m_array[numShadowCubes] = inShadow->m_DepthCube.mPtr;
                ++numShadowCubes;
            } else if (shadowMap && inShadow->m_DepthMap
                       && (numShadowMaps < QDEMON_MAX_NUM_SHADOWS)) {
                shadowMaps.m_array[numShadowMaps] = inShadow->m_DepthMap.mPtr;
                ++numShadowMaps;
            }
        }
    }

    void SetGlobalProperties(QDemonRenderShaderProgram &inProgram, const SLayer & /*inLayer*/
                             ,
                             SCamera &inCamera, QVector3D, QDemonDataRef<SLight *> inLights,
                             QDemonDataRef<QVector3D>, Qt3DSShadowMap *inShadowMaps)
    {
        SShaderGeneratorGeneratedShader &theShader(GetShaderForProgram(inProgram));
        m_RenderContext.GetRenderContext().SetActiveShader(&inProgram);

        SCamera &theCamera(inCamera);

        QVector2D camProps(theCamera.m_ClipNear, theCamera.m_ClipFar);
        theShader.m_CamProperties.Set(camProps);
        theShader.m_CameraPos.Set(theCamera.GetGlobalPos());

        if (theShader.m_ViewMatrix.IsValid())
            theShader.m_ViewMatrix.Set(theCamera.m_GlobalTransform.getInverse());

        if (theShader.m_ProjMatrix.IsValid()) {
            QMatrix4x4 vProjMat;
            inCamera.CalculateViewProjectionMatrix(vProjMat);
            theShader.m_ProjMatrix.Set(vProjMat);
        }

        // set lights separate for area lights
        qint32 cgLights = 0, areaLights = 0;
        qint32 numShadowMaps = 0, numShadowCubes = 0;

        // this call setup the constant buffer for ambient occlusion and shadow
        theShader.m_AoShadowParams.Set();

        if (m_RenderContext.GetRenderContext().GetConstantBufferSupport()) {
            QDemonRenderConstantBuffer *pLightCb =
                    GetLightConstantBuffer("cbBufferLights", inLights.size());
            QDemonRenderConstantBuffer *pAreaLightCb =
                    GetLightConstantBuffer("cbBufferAreaLights", inLights.size());

            // Split the count between CG lights and area lights
            for (quint32 lightIdx = 0; lightIdx < inLights.size() && pLightCb; ++lightIdx) {
                SShadowMapEntry *theShadow = nullptr;
                if (inShadowMaps && inLights[lightIdx]->m_CastShadow)
                    theShadow = inShadowMaps->GetShadowMapEntry(lightIdx);

                qint32 shdwIdx = (inLights[lightIdx]->m_LightType
                                  != RenderLightTypes::Directional)
                        ? numShadowCubes
                        : numShadowMaps;
                SetShadowMaps(inProgram, theShadow, numShadowMaps, numShadowCubes,
                              inLights[lightIdx]->m_LightType == RenderLightTypes::Directional,
                              theShader.m_shadowMaps, theShader.m_shadowCubes);

                if (inLights[lightIdx]->m_LightType == RenderLightTypes::Area) {
                    SShaderLightProperties *theAreaLightEntry =
                            SetLight(inProgram, lightIdx, areaLights, inLights[lightIdx], theShadow,
                                     shdwIdx, inCamera.m_ClipFar);

                    if (theAreaLightEntry && pAreaLightCb) {
                        pAreaLightCb->UpdateRaw(
                                    areaLights * sizeof(SLightSourceShader) + (4 * sizeof(qint32)),
                                    QDemonDataRef<quint8>((quint8 *)&theAreaLightEntry->m_LightData,
                                                          sizeof(SLightSourceShader)));
                    }

                    areaLights++;
                } else {
                    SShaderLightProperties *theLightEntry =
                            SetLight(inProgram, lightIdx, cgLights, inLights[lightIdx], theShadow,
                                     shdwIdx, inCamera.m_ClipFar);

                    if (theLightEntry && pLightCb) {
                        pLightCb->UpdateRaw(
                                    cgLights * sizeof(SLightSourceShader) + (4 * sizeof(qint32)),
                                    QDemonDataRef<quint8>((quint8 *)&theLightEntry->m_LightData,
                                                          sizeof(SLightSourceShader)));
                    }

                    cgLights++;
                }
            }

            if (pLightCb) {
                pLightCb->UpdateRaw(0, QDemonDataRef<quint8>((quint8 *)&cgLights, sizeof(qint32)));
                theShader.m_LightsBuffer.Set();
            }
            if (pAreaLightCb) {
                pAreaLightCb->UpdateRaw(0, QDemonDataRef<quint8>((quint8 *)&areaLights,
                                                                 sizeof(qint32)));
                theShader.m_AreaLightsBuffer.Set();
            }

            theShader.m_LightCount.Set(cgLights);
            theShader.m_AreaLightCount.Set(areaLights);
        } else {
            QVector<SShaderLightProperties *> lprop;
            QVector<SShaderLightProperties *> alprop;
            for (quint32 lightIdx = 0; lightIdx < inLights.size(); ++lightIdx) {

                SShadowMapEntry *theShadow = nullptr;
                if (inShadowMaps && inLights[lightIdx]->m_CastShadow)
                    theShadow = inShadowMaps->GetShadowMapEntry(lightIdx);

                qint32 shdwIdx = (inLights[lightIdx]->m_LightType
                                  != RenderLightTypes::Directional)
                        ? numShadowCubes
                        : numShadowMaps;
                SetShadowMaps(inProgram, theShadow, numShadowMaps, numShadowCubes,
                              inLights[lightIdx]->m_LightType == RenderLightTypes::Directional,
                              theShader.m_shadowMaps, theShader.m_shadowCubes);

                SShaderLightProperties *p = SetLight(inProgram, lightIdx, areaLights,
                                                     inLights[lightIdx], theShadow,
                                                     shdwIdx, inCamera.m_ClipFar);
                if (inLights[lightIdx]->m_LightType == RenderLightTypes::Area)
                    alprop.push_back(p);
                else
                    lprop.push_back(p);
            }
            SLightConstantProperties<SShaderGeneratorGeneratedShader> *lightProperties
                    = theShader.GetLightProperties(lprop.size());
            SLightConstantProperties<SShaderGeneratorGeneratedShader> *areaLightProperties
                    = theShader.GetAreaLightProperties(alprop.size());

            lightProperties->updateLights(lprop);
            areaLightProperties->updateLights(alprop);

            theShader.m_LightCount.Set(lprop.size());
            theShader.m_AreaLightCount.Set(alprop.size());
        }
        for (int i = numShadowMaps; i < QDEMON_MAX_NUM_SHADOWS; ++i)
            theShader.m_shadowMaps.m_array[i] = nullptr;
        for (int i = numShadowCubes; i < QDEMON_MAX_NUM_SHADOWS; ++i)
            theShader.m_shadowCubes.m_array[i] = nullptr;
        theShader.m_shadowMaps.Set(numShadowMaps);
        theShader.m_shadowCubes.Set(numShadowCubes);
        theShader.m_ShadowMapCount.Set(numShadowMaps);
        theShader.m_ShadowCubeCount.Set(numShadowCubes);
    }

    void SetMaterialProperties(QDemonRenderShaderProgram &inProgram, const SCustomMaterial &inMaterial,
                               const QVector2D &, const QMatrix4x4 &inModelViewProjection,
                               const QMatrix3x3 &inNormalMatrix, const QMatrix4x4 &inGlobalTransform,
                               SRenderableImage *inFirstImage, float inOpacity,
                               QDemonRenderTexture2D *inDepthTexture, QDemonRenderTexture2D *inSSaoTexture,
                               SImage *inLightProbe, SImage *inLightProbe2, float inProbeHorizon,
                               float inProbeBright, float inProbe2Window, float inProbe2Pos,
                               float inProbe2Fade, float inProbeFOV)
    {
        ICustomMaterialSystem &theMaterialSystem(m_RenderContext.GetCustomMaterialSystem());
        SShaderGeneratorGeneratedShader &theShader(GetShaderForProgram(inProgram));

        theShader.m_ViewProjMatrix.Set(inModelViewProjection);
        theShader.m_NormalMatrix.Set(inNormalMatrix);
        theShader.m_ModelMatrix.Set(inGlobalTransform);

        theShader.m_DepthTexture.Set(inDepthTexture);
        theShader.m_AOTexture.Set(inSSaoTexture);

        theShader.m_Opacity.Set(inOpacity);

        SImage *theLightProbe = inLightProbe;
        SImage *theLightProbe2 = inLightProbe2;

        if (inMaterial.m_IblProbe && inMaterial.m_IblProbe->m_TextureData.m_Texture) {
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
                // light_probe_offsets.w is now no longer being used to enable/disable fast IBL,
                // (it's now the only option)
                // So now, it's storing the number of mip levels in the IBL image.
                QVector4D offsets(dataPtr[12], dataPtr[13],
                        theLightProbe->m_TextureData.m_TextureFlags.IsPreMultiplied() ? 1.0f
                                                                                      : 0.0f,
                        (float)theLightProbe->m_TextureData.m_Texture->GetNumMipmaps());
                // Fast IBL is always on;
                // inRenderContext.m_Layer.m_FastIbl ? 1.0f : 0.0f );
                // Grab just the upper 2x2 rotation matrix from the larger matrix.
                QVector4D rotations(dataPtr[0], dataPtr[4], dataPtr[1], dataPtr[5]);

                theShader.m_LightProbeRot.Set(rotations);
                theShader.m_LightProbeOfs.Set(offsets);

                if ((!inMaterial.m_IblProbe) && (inProbeFOV < 180.f)) {
                    theShader.m_LightProbeOpts.Set(
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
                    theShader.m_LightProbe2.Set(theLightProbe2->m_TextureData.m_Texture);
                    theShader.m_LightProbe2Props.Set(
                                QVector4D(inProbe2Window, inProbe2Pos, inProbe2Fade, 1.0f));

                    const QMatrix4x4 &xform2 = theLightProbe2->m_TextureTransform;
                    const float *dataPtr(xform2.front());

                    theShader.m_LightProbeProps.Set(
                                QVector4D(dataPtr[12], dataPtr[13], inProbeHorizon, inProbeBright * 0.01f));
                } else {
                    theShader.m_LightProbe2Props.Set(QVector4D(0.0f, 0.0f, 0.0f, 0.0f));
                    theShader.m_LightProbeProps.Set(
                                QVector4D(0.0f, 0.0f, inProbeHorizon, inProbeBright * 0.01f));
                }
            } else {
                theShader.m_LightProbeProps.Set(QVector4D(0.0f, 0.0f, -1.0f, 0.0f));
                theShader.m_LightProbe2Props.Set(QVector4D(0.0f, 0.0f, 0.0f, 0.0f));
            }

            theShader.m_LightProbe.Set(theLightProbe->m_TextureData.m_Texture);

        } else {
            theShader.m_LightProbeProps.Set(QVector4D(0.0f, 0.0f, -1.0f, 0.0f));
            theShader.m_LightProbe2Props.Set(QVector4D(0.0f, 0.0f, 0.0f, 0.0f));
        }

        // finally apply custom material shader properties
        theMaterialSystem.ApplyShaderPropertyValues(inMaterial, inProgram);

        // additional textures
        for (SRenderableImage *theImage = inFirstImage; theImage; theImage = theImage->m_NextImage)
            SetImageShaderVariables(theShader, *theImage);
    }

    void SetMaterialProperties(QDemonRenderShaderProgram &inProgram,
                               const SGraphObject &inMaterial, const QVector2D &inCameraVec,
                               const QMatrix4x4 &inModelViewProjection,
                               const QMatrix3x3 &inNormalMatrix,
                               const QMatrix4x4 &inGlobalTransform,
                               SRenderableImage *inFirstImage, float inOpacity,
                               SLayerGlobalRenderProperties inRenderProperties) override
    {
        const SCustomMaterial &theCustomMaterial(
                    reinterpret_cast<const SCustomMaterial &>(inMaterial));
        Q_ASSERT(inMaterial.m_Type == GraphObjectTypes::CustomMaterial);

        SetGlobalProperties(inProgram, inRenderProperties.m_Layer, inRenderProperties.m_Camera,
                            inRenderProperties.m_CameraDirection, inRenderProperties.m_Lights,
                            inRenderProperties.m_LightDirections,
                            inRenderProperties.m_ShadowMapManager);

        SetMaterialProperties(inProgram, theCustomMaterial, inCameraVec, inModelViewProjection,
                              inNormalMatrix, inGlobalTransform, inFirstImage, inOpacity,
                              inRenderProperties.m_DepthTexture, inRenderProperties.m_SSaoTexture,
                              inRenderProperties.m_LightProbe, inRenderProperties.m_LightProbe2,
                              inRenderProperties.m_ProbeHorizon, inRenderProperties.m_ProbeBright,
                              inRenderProperties.m_Probe2Window, inRenderProperties.m_Probe2Pos,
                              inRenderProperties.m_Probe2Fade, inRenderProperties.m_ProbeFOV);
    }

    void GenerateLightmapIndirectFunc(IShaderStageGenerator &inFragmentShader,
                                      SImage *pEmissiveLightmap)
    {
        inFragmentShader << "\n";
        inFragmentShader << "vec3 computeMaterialLightmapIndirect()\n{\n";
        inFragmentShader << "  vec4 indirect = vec4( 0.0, 0.0, 0.0, 0.0 );\n";
        if (pEmissiveLightmap) {
            SImageVariableNames names =
                    GetImageVariableNames(ConvertTextureTypeValue(ImageMapTypes::LightmapIndirect));
            inFragmentShader.AddUniform(names.m_ImageSampler, "sampler2D");
            inFragmentShader.AddUniform(m_ImageOffset, "vec3");
            inFragmentShader.AddUniform(m_ImageRotScale, "vec4");

            inFragmentShader << "\n  indirect = evalIndirectLightmap( " << m_ImageSampler
                             << ", varTexCoord1, ";
            inFragmentShader << m_ImageRotScale << ", ";
            inFragmentShader << m_ImageOffset << " );\n\n";
        }

        inFragmentShader << "  return indirect.rgb;\n";
        inFragmentShader << "}\n\n";
    }

    void GenerateLightmapRadiosityFunc(IShaderStageGenerator &inFragmentShader,
                                       SImage *pRadiosityLightmap)
    {
        inFragmentShader << "\n";
        inFragmentShader << "vec3 computeMaterialLightmapRadiosity()\n{\n";
        inFragmentShader << "  vec4 radiosity = vec4( 1.0, 1.0, 1.0, 1.0 );\n";
        if (pRadiosityLightmap) {
            SImageVariableNames names =
                    GetImageVariableNames(ConvertTextureTypeValue(ImageMapTypes::LightmapRadiosity));
            inFragmentShader.AddUniform(names.m_ImageSampler, "sampler2D");
            inFragmentShader.AddUniform(m_ImageOffset, "vec3");
            inFragmentShader.AddUniform(m_ImageRotScale, "vec4");

            inFragmentShader << "\n  radiosity = evalRadiosityLightmap( " << m_ImageSampler
                             << ", varTexCoord1, ";
            inFragmentShader << m_ImageRotScale << ", ";
            inFragmentShader << m_ImageOffset << " );\n\n";
        }

        inFragmentShader << "  return radiosity.rgb;\n";
        inFragmentShader << "}\n\n";
    }

    void GenerateLightmapShadowFunc(IShaderStageGenerator &inFragmentShader,
                                    SImage *pBakedShadowMap)
    {
        inFragmentShader << "\n";
        inFragmentShader << "vec4 computeMaterialLightmapShadow()\n{\n";
        inFragmentShader << "  vec4 shadowMask = vec4( 1.0, 1.0, 1.0, 1.0 );\n";
        if (pBakedShadowMap) {
            SImageVariableNames names =
                    GetImageVariableNames((quint32)QDemonRenderTextureTypeValue::LightmapShadow);
            // Add uniforms
            inFragmentShader.AddUniform(names.m_ImageSampler, "sampler2D");
            inFragmentShader.AddUniform(m_ImageOffset, "vec3");
            inFragmentShader.AddUniform(m_ImageRotScale, "vec4");

            inFragmentShader << "\n  shadowMask = evalShadowLightmap( " << m_ImageSampler
                             << ", texCoord0, ";
            inFragmentShader << m_ImageRotScale << ", ";
            inFragmentShader << m_ImageOffset << " );\n\n";
        }

        inFragmentShader << "  return shadowMask;\n";
        inFragmentShader << "}\n\n";
    }

    void GenerateLightmapIndirectSetupCode(IShaderStageGenerator &inFragmentShader,
                                           SRenderableImage *pIndirectLightmap,
                                           SRenderableImage *pRadiosityLightmap)
    {
        if (!pIndirectLightmap && !pRadiosityLightmap)
            return;

        QString finalValue;

        inFragmentShader << "\n";
        inFragmentShader << "void initializeLayerVariablesWithLightmap(void)\n{\n";
        if (pIndirectLightmap) {
            inFragmentShader
                    << "  vec3 lightmapIndirectValue = computeMaterialLightmapIndirect( );\n";
            finalValue.append("vec4(lightmapIndirectValue, 1.0)");
        }
        if (pRadiosityLightmap) {
            inFragmentShader
                    << "  vec3 lightmapRadisoityValue = computeMaterialLightmapRadiosity( );\n";
            if (finalValue.empty())
                finalValue.append("vec4(lightmapRadisoityValue, 1.0)");
            else
                finalValue.append(" + vec4(lightmapRadisoityValue, 1.0)");
        }

        finalValue.append(";\n");

        char buf[16];
        for (quint32 idx = 0; idx < Material().m_LayerCount; idx++) {
            _snprintf(buf, 16, "[%d]", idx);
            inFragmentShader << "  layers" << buf << ".base += " << finalValue.c_str();
            inFragmentShader << "  layers" << buf << ".layer += " << finalValue.c_str();
        }

        inFragmentShader << "}\n\n";
    }

    void GenerateLightmapShadowCode(IShaderStageGenerator &inFragmentShader,
                                    SRenderableImage *pBakedShadowMap)
    {
        if (pBakedShadowMap) {
            inFragmentShader << " tmpShadowTerm *= computeMaterialLightmapShadow( );\n\n";
        }
    }

    void ApplyEmissiveMask(IShaderStageGenerator &inFragmentShader, SImage *pEmissiveMaskMap)
    {
        inFragmentShader << "\n";
        inFragmentShader << "vec3 computeMaterialEmissiveMask()\n{\n";
        inFragmentShader << "  vec3 emissiveMask = vec3( 1.0, 1.0, 1.0 );\n";
        if (pEmissiveMaskMap) {
            inFragmentShader << "  texture_coordinate_info tci;\n";
            inFragmentShader << "  texture_coordinate_info transformed_tci;\n";
            inFragmentShader << "  tci = textureCoordinateInfo( texCoord0, tangent, binormal );\n";
            inFragmentShader << "  transformed_tci = transformCoordinate( "
                                "rotationTranslationScale( vec3( 0.000000, 0.000000, 0.000000 ), ";
            inFragmentShader << "vec3( 0.000000, 0.000000, 0.000000 ), vec3( 1.000000, 1.000000, "
                                "1.000000 ) ), tci );\n";
            inFragmentShader << "  emissiveMask = fileTexture( "
                             << pEmissiveMaskMap->m_ImageShaderName.c_str()
                             << ", vec3( 0, 0, 0 ), vec3( 1, 1, 1 ), mono_alpha, transformed_tci, ";
            inFragmentShader << "vec2( 0.000000, 1.000000 ), vec2( 0.000000, 1.000000 ), "
                                "wrap_repeat, wrap_repeat, gamma_default ).tint;\n";
        }

        inFragmentShader << "  return emissiveMask;\n";
        inFragmentShader << "}\n\n";
    }

    void GenerateFragmentShader(SShaderDefaultMaterialKey &, const char *inShaderPathName)
    {
        IDynamicObjectSystem &theDynamicSystem(
                    m_RenderContext.GetDynamicObjectSystem());
        CRenderString theShaderBuffer;
        const char *fragSource = theDynamicSystem.GetShaderSource(
                    m_RenderContext.GetStringTable().RegisterStr(inShaderPathName), theShaderBuffer);

        Q_ASSERT(fragSource);

        // light maps
        bool hasLightmaps = false;
        SRenderableImage *lightmapShadowImage = nullptr;
        SRenderableImage *lightmapIndirectImage = nullptr;
        SRenderableImage *lightmapRadisoityImage = nullptr;

        for (SRenderableImage *img = m_FirstImage; img != nullptr; img = img->m_NextImage) {
            if (img->m_MapType == ImageMapTypes::LightmapIndirect) {
                lightmapIndirectImage = img;
                hasLightmaps = true;
            } else if (img->m_MapType == ImageMapTypes::LightmapRadiosity) {
                lightmapRadisoityImage = img;
                hasLightmaps = true;
            } else if (img->m_MapType == ImageMapTypes::LightmapShadow) {
                lightmapShadowImage = img;
            }
        }

        VertexGenerator().GenerateUVCoords(0);
        // for lightmaps we expect a second set of uv coordinates
        if (hasLightmaps)
            VertexGenerator().GenerateUVCoords(1);

        IDefaultMaterialVertexPipeline &vertexShader(VertexGenerator());
        IShaderStageGenerator &fragmentShader(FragmentGenerator());

        QString srcString(fragSource);

        if (m_RenderContext.GetRenderContext().GetRenderContextType()
                == QDemonRenderContextValues::GLES2) {
            QString::size_type pos = 0;
            while ((pos = srcString.find("out vec4 fragColor", pos)) != QString::npos) {
                srcString.insert(pos, "//");
                pos += int(strlen("//out vec4 fragColor"));
            }
        }

        fragmentShader << "#define FRAGMENT_SHADER\n\n";

        if (srcString.find("void main()") == QString::npos)
            fragmentShader.AddInclude("evalLightmaps.glsllib");

        // check dielectric materials
        if (!Material().IsDielectric())
            fragmentShader << "#define MATERIAL_IS_NON_DIELECTRIC 1\n\n";
        else
            fragmentShader << "#define MATERIAL_IS_NON_DIELECTRIC 0\n\n";

        fragmentShader << "#define QDEMON_ENABLE_RNM 0\n\n";

        fragmentShader << srcString.data() << Endl;

        if (srcString.find("void main()") != QString::npos) // If a "main()" is already
            // written, we'll assume that the
            // shader
        { // pass is already written out and we don't need to add anything.
            // Nothing beyond the basics, anyway
            vertexShader.GenerateWorldNormal();
            vertexShader.GenerateVarTangentAndBinormal();
            vertexShader.GenerateWorldPosition();

            vertexShader.GenerateViewVector();
            return;
        }

        if (Material().HasLighting() && lightmapIndirectImage) {
            GenerateLightmapIndirectFunc(fragmentShader, &lightmapIndirectImage->m_Image);
        }
        if (Material().HasLighting() && lightmapRadisoityImage) {
            GenerateLightmapRadiosityFunc(fragmentShader, &lightmapRadisoityImage->m_Image);
        }
        if (Material().HasLighting() && lightmapShadowImage) {
            GenerateLightmapShadowFunc(fragmentShader, &lightmapShadowImage->m_Image);
        }

        if (Material().HasLighting() && (lightmapIndirectImage || lightmapRadisoityImage))
            GenerateLightmapIndirectSetupCode(fragmentShader, lightmapIndirectImage,
                                              lightmapRadisoityImage);

        if (Material().HasLighting()) {
            ApplyEmissiveMask(fragmentShader, Material().m_EmissiveMap2);
        }

        // setup main
        VertexGenerator().BeginFragmentGeneration();

        // since we do pixel lighting we always need this if lighting is enabled
        // We write this here because the functions below may also write to
        // the fragment shader
        if (Material().HasLighting()) {
            vertexShader.GenerateWorldNormal();
            vertexShader.GenerateVarTangentAndBinormal();
            vertexShader.GenerateWorldPosition();

            if (Material().IsSpecularEnabled())
                vertexShader.GenerateViewVector();
        }

        fragmentShader << "  initializeBaseFragmentVariables();" << Endl;
        fragmentShader << "  computeTemporaries();" << Endl;
        fragmentShader << "  normal = normalize( computeNormal() );" << Endl;
        fragmentShader << "  initializeLayerVariables();" << Endl;
        fragmentShader << "  float alpha = clamp( evalCutout(), 0.0, 1.0 );" << Endl;

        if (Material().IsCutOutEnabled()) {
            fragmentShader << "  if ( alpha <= 0.0f )" << Endl;
            fragmentShader << "    discard;" << Endl;
        }

        // indirect / direct lightmap init
        if (Material().HasLighting() && (lightmapIndirectImage || lightmapRadisoityImage))
            fragmentShader << "  initializeLayerVariablesWithLightmap();" << Endl;

        // shadow map
        GenerateLightmapShadowCode(fragmentShader, lightmapShadowImage);

        // main Body
        fragmentShader << "#include \"customMaterialFragBodyAO.glsllib\"" << Endl;

        // for us right now transparency means we render a glass style material
        if (m_HasTransparency && !Material().IsTransmissive())
            fragmentShader << " rgba = computeGlass( normal, materialIOR, alpha, rgba );" << Endl;
        if (Material().IsTransmissive())
            fragmentShader << " rgba = computeOpacity( rgba );" << Endl;

        if (VertexGenerator().HasActiveWireframe()) {
            fragmentShader.Append("vec3 edgeDistance = varEdgeDistance * gl_FragCoord.w;");
            fragmentShader.Append(
                        "\tfloat d = min(min(edgeDistance.x, edgeDistance.y), edgeDistance.z);");
            fragmentShader.Append("\tfloat mixVal = smoothstep(0.0, 1.0, d);"); // line width 1.0

            fragmentShader.Append("\trgba = mix( vec4(0.0, 1.0, 0.0, 1.0), rgba, mixVal);");
        }
        fragmentShader << "  rgba.a *= object_opacity;" << Endl;
        if (m_RenderContext.GetRenderContext().GetRenderContextType()
                == QDemonRenderContextValues::GLES2)
            fragmentShader << "  gl_FragColor = rgba;" << Endl;
        else
            fragmentShader << "  fragColor = rgba;" << Endl;
    }

    QDemonRenderShaderProgram *GenerateCustomMaterialShader(const char *inShaderPrefix,
                                                            const char *inCustomMaterialName)
    {
        // build a string that allows us to print out the shader we are generating to the log.
        // This is time consuming but I feel like it doesn't happen all that often and is very
        // useful to users
        // looking at the log file.
        m_GeneratedShaderString.clear();
        m_GeneratedShaderString.assign(nonNull(inShaderPrefix));
        m_GeneratedShaderString.append(inCustomMaterialName);
        SShaderDefaultMaterialKey theKey(Key());
        theKey.ToString(m_GeneratedShaderString, m_DefaultMaterialShaderKeyProperties);

        GenerateVertexShader();
        GenerateFragmentShader(theKey, inCustomMaterialName);

        VertexGenerator().EndVertexGeneration();
        VertexGenerator().EndFragmentGeneration();

        return ProgramGenerator().CompileGeneratedShader(m_GeneratedShaderString.c_str(),
                                                         SShaderCacheProgramFlags(), FeatureSet());
    }

    virtual QDemonRenderShaderProgram *
    GenerateShader(const SGraphObject &inMaterial, SShaderDefaultMaterialKey inShaderDescription,
                   IShaderStageGenerator &inVertexPipeline, TShaderFeatureSet inFeatureSet,
                   QDemonDataRef<SLight *> inLights, SRenderableImage *inFirstImage,
                   bool inHasTransparency, const char *inShaderPrefix,
                   const char *inCustomMaterialName) override
    {
        Q_ASSERT(inMaterial.m_Type == GraphObjectTypes::CustomMaterial);
        m_CurrentMaterial = reinterpret_cast<const SCustomMaterial *>(&inMaterial);
        m_CurrentKey = &inShaderDescription;
        m_CurrentPipeline = static_cast<IDefaultMaterialVertexPipeline *>(&inVertexPipeline);
        m_CurrentFeatureSet = inFeatureSet;
        m_Lights = inLights;
        m_FirstImage = inFirstImage;
        m_HasTransparency = inHasTransparency;

        return GenerateCustomMaterialShader(inShaderPrefix, inCustomMaterialName);
    }
};
}

ICustomMaterialShaderGenerator &
ICustomMaterialShaderGenerator::CreateCustomMaterialShaderGenerator(IQDemonRenderContext &inRc)
{
    return *QDEMON_NEW(inRc.GetAllocator(), SShaderGenerator)(inRc);
}

QT_END_NAMESPACE
