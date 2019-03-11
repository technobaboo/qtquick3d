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
#include "qdemonrendercustommaterialsystem.h"

#include <QtDemon/qdemontime.h>
#include <QtDemon/qdemonutils.h>

#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>

#include <QtDemonRuntimeRender/qdemonrendercustommaterialrendercontext.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRuntimeRender/qdemonrendercustommaterial.h>
#include <QtDemonRuntimeRender/qdemonrenderdynamicobjectsystemcommands.h>
#include <QtDemonRuntimeRender/qdemonrenderbuffermanager.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcemanager.h>
#include <QtDemonRuntimeRender/qdemonrendermesh.h>
#include <QtDemonRuntimeRender/qdemonrendercamera.h>
#include <QtDemonRuntimeRender/qdemonrenderlight.h>
#include <QtDemonRuntimeRender/qdemonrenderlayer.h>
#include <QtDemonRuntimeRender/qdemonrenderdynamicobjectsystemutil.h>
#include <QtDemonRuntimeRender/qdemonrenderableimage.h>
#include <QtDemonRuntimeRender/qdemonvertexpipelineimpl.h>
#include <QtDemonRuntimeRender/qdemonrendererimpllayerrenderdata.h>
#include <QtDemonRuntimeRender/qdemonrendermodel.h>

QT_BEGIN_NAMESPACE

QDemonCustomMaterialVertexPipeline::QDemonCustomMaterialVertexPipeline(QDemonRenderContextInterface *inContext,
                                                                       TessModeValues inTessMode)
    : QDemonVertexPipelineImpl(inContext->getCustomMaterialShaderGenerator(), inContext->getShaderProgramGenerator(), false)
    , m_context(inContext)
    , m_tessMode(TessModeValues::NoTess)
{
    if (m_context->getRenderContext()->supportsTessellation()) {
        m_tessMode = inTessMode;
    }

    if (m_context->getRenderContext()->supportsGeometryStage() && m_tessMode != TessModeValues::NoTess) {
        m_wireframe = inContext->getWireframeMode();
    }
}

void QDemonCustomMaterialVertexPipeline::initializeTessControlShader()
{
    if (m_tessMode == TessModeValues::NoTess || programGenerator()->getStage(QDemonShaderGeneratorStage::TessControl) == nullptr) {
        return;
    }

    QDemonShaderStageGeneratorInterface &tessCtrlShader(*programGenerator()->getStage(QDemonShaderGeneratorStage::TessControl));

    tessCtrlShader.addUniform("tessLevelInner", "float");
    tessCtrlShader.addUniform("tessLevelOuter", "float");

    setupTessIncludes(QDemonShaderGeneratorStage::TessControl, m_tessMode);

    tessCtrlShader.append("void main() {\n");

    tessCtrlShader.append("\tctWorldPos[0] = varWorldPos[0];");
    tessCtrlShader.append("\tctWorldPos[1] = varWorldPos[1];");
    tessCtrlShader.append("\tctWorldPos[2] = varWorldPos[2];");

    if (m_tessMode == TessModeValues::TessPhong || m_tessMode == TessModeValues::TessNPatch) {
        tessCtrlShader.append("\tctNorm[0] = varObjectNormal[0];");
        tessCtrlShader.append("\tctNorm[1] = varObjectNormal[1];");
        tessCtrlShader.append("\tctNorm[2] = varObjectNormal[2];");
    }
    if (m_tessMode == TessModeValues::TessNPatch) {
        tessCtrlShader.append("\tctTangent[0] = varObjTangent[0];");
        tessCtrlShader.append("\tctTangent[1] = varObjTangent[1];");
        tessCtrlShader.append("\tctTangent[2] = varObjTangent[2];");
    }

    tessCtrlShader.append("\tgl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;");
    tessCtrlShader.append("\ttessShader( tessLevelOuter, tessLevelInner);\n");
}

void QDemonCustomMaterialVertexPipeline::initializeTessEvaluationShader()
{
    if (m_tessMode == TessModeValues::NoTess || programGenerator()->getStage(QDemonShaderGeneratorStage::TessEval) == nullptr) {
        return;
    }

    QDemonShaderStageGeneratorInterface &tessEvalShader(*programGenerator()->getStage(QDemonShaderGeneratorStage::TessEval));

    tessEvalShader.addUniform("model_view_projection", "mat4");
    tessEvalShader.addUniform("normal_matrix", "mat3");

    setupTessIncludes(QDemonShaderGeneratorStage::TessEval, m_tessMode);

    if (m_tessMode == TessModeValues::TessLinear && m_displacementImage) {
        tessEvalShader.addInclude("defaultMaterialFileDisplacementTexture.glsllib");
        tessEvalShader.addUniform("model_matrix", "mat4");
        tessEvalShader.addUniform("displace_tiling", "vec3");
        tessEvalShader.addUniform("displaceAmount", "float");
        tessEvalShader.addUniform(m_displacementImage->m_image.m_imageShaderName.toUtf8(), "sampler2D");
    }

    tessEvalShader.append("void main() {");

    if (m_tessMode == TessModeValues::TessNPatch) {
        tessEvalShader.append("\tctNorm[0] = varObjectNormalTC[0];");
        tessEvalShader.append("\tctNorm[1] = varObjectNormalTC[1];");
        tessEvalShader.append("\tctNorm[2] = varObjectNormalTC[2];");

        tessEvalShader.append("\tctTangent[0] = varTangentTC[0];");
        tessEvalShader.append("\tctTangent[1] = varTangentTC[1];");
        tessEvalShader.append("\tctTangent[2] = varTangentTC[2];");
    }

    tessEvalShader.append("\tvec4 pos = tessShader( );\n");
}

void QDemonCustomMaterialVertexPipeline::finalizeTessControlShader()
{
    QDemonShaderStageGeneratorInterface &tessCtrlShader(*programGenerator()->getStage(QDemonShaderGeneratorStage::TessControl));
    // add varyings we must pass through
    typedef TStrTableStrMap::const_iterator TParamIter;
    for (TParamIter iter = m_interpolationParameters.begin(), end = m_interpolationParameters.end(); iter != end; ++iter) {
        tessCtrlShader << "\t" << iter.key() << "TC[gl_InvocationID] = " << iter.key() << "[gl_InvocationID];\n";
    }
}

void QDemonCustomMaterialVertexPipeline::finalizeTessEvaluationShader()
{
    QDemonShaderStageGeneratorInterface &tessEvalShader(*programGenerator()->getStage(QDemonShaderGeneratorStage::TessEval));

    QByteArray outExt;
    if (programGenerator()->getEnabledStages() & QDemonShaderGeneratorStage::Geometry)
        outExt = "TE";

    // add varyings we must pass through
    typedef TStrTableStrMap::const_iterator TParamIter;
    if (m_tessMode == TessModeValues::TessNPatch) {
        for (TParamIter iter = m_interpolationParameters.begin(), end = m_interpolationParameters.end(); iter != end; ++iter) {
            tessEvalShader << "\t" << iter.key() << outExt << " = gl_TessCoord.z * " << iter.key() << "TC[0] + ";
            tessEvalShader << "gl_TessCoord.x * " << iter.key() << "TC[1] + ";
            tessEvalShader << "gl_TessCoord.y * " << iter.key() << "TC[2];\n";
        }

        // transform the normal
        if (m_generationFlags & GenerationFlag::WorldNormal)
            tessEvalShader << "\n\tvarNormal" << outExt << " = normalize(normal_matrix * teNorm);\n";
        // transform the tangent
        if (m_generationFlags & GenerationFlag::TangentBinormal) {
            tessEvalShader << "\n\tvarTangent" << outExt << " = normalize(normal_matrix * teTangent);\n";
            // transform the binormal
            tessEvalShader << "\n\tvarBinormal" << outExt << " = normalize(normal_matrix * teBinormal);\n";
        }
    } else {
        for (TParamIter iter = m_interpolationParameters.begin(), end = m_interpolationParameters.end(); iter != end; ++iter) {
            tessEvalShader << "\t" << iter.key() << outExt << " = gl_TessCoord.x * " << iter.key() << "TC[0] + ";
            tessEvalShader << "gl_TessCoord.y * " << iter.key() << "TC[1] + ";
            tessEvalShader << "gl_TessCoord.z * " << iter.key() << "TC[2];\n";
        }

        // displacement mapping makes only sense with linear tessellation
        if (m_tessMode == TessModeValues::TessLinear && m_displacementImage) {
            tessEvalShader << "\ttexture_coordinate_info tmp = textureCoordinateInfo( varTexCoord0" << outExt
                           << ", varTangent" << outExt << ", varBinormal" << outExt << " );"
                           << "\n";
            tessEvalShader << "\ttmp = transformCoordinate( rotationTranslationScale( vec3( "
                              "0.000000, 0.000000, 0.000000 ), vec3( 0.000000, 0.000000, "
                              "0.000000 ), displace_tiling ), tmp);"
                           << "\n";

            tessEvalShader << "\tpos.xyz = defaultMaterialFileDisplacementTexture( "
                           << m_displacementImage->m_image.m_imageShaderName.toUtf8() << ", displaceAmount, "
                           << "tmp.position.xy";
            tessEvalShader << ", varObjectNormal" << outExt << ", pos.xyz );"
                           << "\n";
            tessEvalShader << "\tvarWorldPos" << outExt << "= (model_matrix * pos).xyz;"
                           << "\n";
        }

        // transform the normal
        tessEvalShader << "\n\tvarNormal" << outExt << " = normalize(normal_matrix * varObjectNormal" << outExt << ");\n";
    }

    tessEvalShader.append("\tgl_Position = model_view_projection * pos;\n");
}

// Responsible for beginning all vertex and fragment generation (void main() { etc).
void QDemonCustomMaterialVertexPipeline::beginVertexGeneration(quint32 displacementImageIdx, QDemonRenderableImage *displacementImage)
{
    m_displacementIdx = displacementImageIdx;
    m_displacementImage = displacementImage;

    QDemonShaderGeneratorStageFlags theStages(QDemonShaderProgramGeneratorInterface::defaultFlags());

    if (m_tessMode != TessModeValues::NoTess) {
        theStages |= QDemonShaderGeneratorStage::TessControl;
        theStages |= QDemonShaderGeneratorStage::TessEval;
    }
    if (m_wireframe) {
        theStages |= QDemonShaderGeneratorStage::Geometry;
    }

    programGenerator()->beginProgram(theStages);

    if (m_tessMode != TessModeValues::NoTess) {
        initializeTessControlShader();
        initializeTessEvaluationShader();
    }
    if (m_wireframe) {
        initializeWireframeGeometryShader();
    }

    QDemonShaderStageGeneratorInterface &vertexShader(vertex());

    // thinks we need
    vertexShader.addInclude("viewProperties.glsllib");
    vertexShader.addInclude("customMaterial.glsllib");

    vertexShader.addIncoming("attr_pos", "vec3");
    vertexShader << "void main()"
                 << "\n"
                 << "{"
                 << "\n";

    if (displacementImage) {
        generateUVCoords(0);
        if (!hasTessellation()) {
            vertexShader.addUniform("displaceAmount", "float");
            vertexShader.addUniform("displace_tiling", "vec3");
            // we create the world position setup here
            // because it will be replaced with the displaced position
            setCode(GenerationFlag::WorldPosition);
            vertexShader.addUniform("model_matrix", "mat4");

            vertexShader.addInclude("defaultMaterialFileDisplacementTexture.glsllib");
            vertexShader.addUniform(displacementImage->m_image.m_imageShaderName.toUtf8(), "sampler2D");

            vertexShader << "\ttexture_coordinate_info tmp = textureCoordinateInfo( texCoord0, "
                            "varTangent, varBinormal );"
                         << "\n";
            vertexShader << "\ttmp = transformCoordinate( rotationTranslationScale( vec3( "
                            "0.000000, 0.000000, 0.000000 ), vec3( 0.000000, 0.000000, "
                            "0.000000 ), displace_tiling ), tmp);"
                         << "\n";

            vertexShader << "\tvec3 displacedPos = defaultMaterialFileDisplacementTexture( "
                         << displacementImage->m_image.m_imageShaderName.toUtf8() << ", displaceAmount, "
                         << "tmp.position.xy"
                         << ", attr_norm, attr_pos );"
                         << "\n";

            addInterpolationParameter("varWorldPos", "vec3");
            vertexShader.append("\tvec3 local_model_world_position = (model_matrix * "
                                "vec4(displacedPos, 1.0)).xyz;");
            assignOutput("varWorldPos", "local_model_world_position");
        }
    }

    if (hasTessellation()) {
        vertexShader.append("\tgl_Position = vec4(attr_pos, 1.0);");
    } else {
        vertexShader.addUniform("model_view_projection", "mat4");
        if (displacementImage)
            vertexShader.append("\tgl_Position = model_view_projection * vec4(displacedPos, 1.0);");
        else
            vertexShader.append("\tgl_Position = model_view_projection * vec4(attr_pos, 1.0);");
    }

    if (hasTessellation()) {
        generateWorldPosition();
        generateWorldNormal();
        generateObjectNormal();
        generateVarTangentAndBinormal();
    }
}

void QDemonCustomMaterialVertexPipeline::beginFragmentGeneration()
{
    fragment().addUniform("object_opacity", "float");
    fragment() << "void main()"
               << "\n"
               << "{"
               << "\n";
}

void QDemonCustomMaterialVertexPipeline::assignOutput(const QByteArray &inVarName, const QByteArray &inVarValue)
{
    vertex() << "\t" << inVarName << " = " << inVarValue << ";\n";
}

void QDemonCustomMaterialVertexPipeline::generateUVCoords(quint32 inUVSet)
{
    if (inUVSet == 0 && setCode(GenerationFlag::UVCoords))
        return;
    if (inUVSet == 1 && setCode(GenerationFlag::UVCoords1))
        return;

    Q_ASSERT(inUVSet == 0 || inUVSet == 1);

    if (inUVSet == 0)
        addInterpolationParameter("varTexCoord0", "vec3");
    else if (inUVSet == 1)
        addInterpolationParameter("varTexCoord1", "vec3");

    doGenerateUVCoords(inUVSet);
}

void QDemonCustomMaterialVertexPipeline::generateWorldNormal()
{
    if (setCode(GenerationFlag::WorldNormal))
        return;
    addInterpolationParameter("varNormal", "vec3");
    doGenerateWorldNormal();
}

void QDemonCustomMaterialVertexPipeline::generateObjectNormal()
{
    if (setCode(GenerationFlag::ObjectNormal))
        return;
    doGenerateObjectNormal();
}

void QDemonCustomMaterialVertexPipeline::generateVarTangentAndBinormal()
{
    if (setCode(GenerationFlag::TangentBinormal))
        return;
    addInterpolationParameter("varTangent", "vec3");
    addInterpolationParameter("varBinormal", "vec3");
    addInterpolationParameter("varObjTangent", "vec3");
    addInterpolationParameter("varObjBinormal", "vec3");
    doGenerateVarTangentAndBinormal();
}

void QDemonCustomMaterialVertexPipeline::generateWorldPosition()
{
    if (setCode(GenerationFlag::WorldPosition))
        return;

    activeStage().addUniform("model_matrix", "mat4");
    addInterpolationParameter("varWorldPos", "vec3");
    addInterpolationParameter("varObjPos", "vec3");
    doGenerateWorldPosition();
}

// responsible for closing all vertex and fragment generation
void QDemonCustomMaterialVertexPipeline::endVertexGeneration()
{
    if (hasTessellation()) {
        // finalize tess control shader
        finalizeTessControlShader();
        // finalize tess evaluation shader
        finalizeTessEvaluationShader();

        tessControl().append("}");
        tessEval().append("}");

        if (m_wireframe) {
            // finalize geometry shader
            finalizeWireframeGeometryShader();
            geometry().append("}");
        }
    }

    vertex().append("}");
}

void QDemonCustomMaterialVertexPipeline::endFragmentGeneration()
{
    fragment().append("}");
}

QDemonShaderStageGeneratorInterface &QDemonCustomMaterialVertexPipeline::activeStage()
{
    return vertex();
}

void QDemonCustomMaterialVertexPipeline::addInterpolationParameter(const QByteArray &inName, const QByteArray &inType)
{
    m_interpolationParameters.insert(inName, inType);
    vertex().addOutgoing(inName, inType);
    fragment().addIncoming(inName, inType);

    if (hasTessellation()) {
        QByteArray nameBuilder(inName);
        nameBuilder.append("TC");
        tessControl().addOutgoing(nameBuilder, inType);

        nameBuilder = inName;
        if (programGenerator()->getEnabledStages() & QDemonShaderGeneratorStage::Geometry) {
            nameBuilder.append("TE");
            geometry().addOutgoing(inName, inType);
        }
        tessEval().addOutgoing(nameBuilder, inType);
    }
}

void QDemonCustomMaterialVertexPipeline::doGenerateUVCoords(quint32 inUVSet)
{
    Q_ASSERT(inUVSet == 0 || inUVSet == 1);

    if (inUVSet == 0) {
        vertex().addIncoming("attr_uv0", "vec2");
        vertex() << "\tvec3 texCoord0 = vec3( attr_uv0, 0.0 );"
                 << "\n";
        assignOutput("varTexCoord0", "texCoord0");
    } else if (inUVSet == 1) {
        vertex().addIncoming("attr_uv1", "vec2");
        vertex() << "\tvec3 texCoord1 = vec3( attr_uv1, 1.0 );"
                 << "\n";
        assignOutput("varTexCoord1", "texCoord1");
    }
}

void QDemonCustomMaterialVertexPipeline::doGenerateWorldNormal()
{
    QDemonShaderStageGeneratorInterface &vertexGenerator(vertex());
    vertexGenerator.addIncoming("attr_norm", "vec3");
    vertexGenerator.addUniform("normal_matrix", "mat3");

    if (hasTessellation() == false) {
        vertex().append("\tvarNormal = normalize( normal_matrix * attr_norm );");
    }
}

void QDemonCustomMaterialVertexPipeline::doGenerateObjectNormal()
{
    addInterpolationParameter("varObjectNormal", "vec3");
    vertex().append("\tvarObjectNormal = attr_norm;");
}

void QDemonCustomMaterialVertexPipeline::doGenerateWorldPosition()
{
    vertex().append("\tvarObjPos = attr_pos;");
    vertex().append("\tvec4 worldPos = (model_matrix * vec4(attr_pos, 1.0));");
    assignOutput("varWorldPos", "worldPos.xyz");
}

void QDemonCustomMaterialVertexPipeline::doGenerateVarTangentAndBinormal()
{
    vertex().addIncoming("attr_textan", "vec3");
    vertex().addIncoming("attr_binormal", "vec3");

    vertex() << "\tvarTangent = normal_matrix * attr_textan;"
             << "\n"
             << "\tvarBinormal = normal_matrix * attr_binormal;"
             << "\n";

    vertex() << "\tvarObjTangent = attr_textan;"
             << "\n"
             << "\tvarObjBinormal = attr_binormal;"
             << "\n";
}

void QDemonCustomMaterialVertexPipeline::doGenerateVertexColor()
{
    vertex().addIncoming("attr_color", "vec3");
    vertex().append("\tvarColor = attr_color;");
}

struct QDemonMaterialClass
{
    QAtomicInt ref;
    QDemonDynamicObjectClassInterface *m_class;
    bool m_hasTransparency = false;
    bool m_hasRefraction = false;
    bool m_hasDisplacement = false;
    bool m_alwaysDirty = false;
    quint32 m_shaderKey = 0;
    quint32 m_layerCount = 0;
    QDemonMaterialClass(QDemonDynamicObjectClassInterface &inCls) : m_class(&inCls) {}

    void afterWrite() { m_class = nullptr; }

    void afterRead(QDemonDynamicObjectClassInterface &inCls) { m_class = &inCls; }
};

struct QDemonShaderMapKey
{
    TStrStrPair m_name;
    QVector<QDemonShaderPreprocessorFeature> m_features;
    TessModeValues m_tessMode;
    bool m_wireframeMode;
    QDemonShaderDefaultMaterialKey m_materialKey;
    uint m_hashCode;
    QDemonShaderMapKey(const TStrStrPair &inName,
                       const TShaderFeatureSet &inFeatures,
                       TessModeValues inTessMode,
                       bool inWireframeMode,
                       QDemonShaderDefaultMaterialKey inMaterialKey)
        : m_name(inName), m_features(inFeatures), m_tessMode(inTessMode), m_wireframeMode(inWireframeMode), m_materialKey(inMaterialKey)
    {
        m_hashCode = qHash(m_name) ^ hashShaderFeatureSet(m_features) ^ qHash(m_tessMode) ^ qHash(m_wireframeMode)
                ^ qHash(inMaterialKey.hash());
    }
    bool operator==(const QDemonShaderMapKey &inKey) const
    {
        return m_name == inKey.m_name && m_features == inKey.m_features && m_tessMode == inKey.m_tessMode
                && m_wireframeMode == inKey.m_wireframeMode && m_materialKey == inKey.m_materialKey;
    }
};

uint qHash(const QDemonShaderMapKey &key)
{
    return key.m_hashCode;
}

struct QDemonCustomMaterialTextureData
{
    QAtomicInt ref;
    QDemonRef<QDemonRenderShaderProgram> shader;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> sampler;
    QDemonRef<QDemonRenderTexture2D> texture;
    bool needsMips;

    QDemonCustomMaterialTextureData(const QDemonRef<QDemonRenderShaderProgram> &inShader,
                                    const QDemonRef<QDemonRenderTexture2D> &inTexture,
                                    const QByteArray &inTexName,
                                    bool inNeedMips)
        : shader(inShader), sampler(inTexName, inShader), texture(inTexture), needsMips(inNeedMips)
    {
    }

    void set(const dynamic::QDemonPropertyDefinition *inDefinition)
    {
        if (texture && inDefinition) {
            texture->setMagFilter(inDefinition->magFilterOp);
            texture->setMinFilter(inDefinition->minFilterOp);
            texture->setTextureWrapS(inDefinition->coordOp);
            texture->setTextureWrapT(inDefinition->coordOp);
        } else if (texture) {
            // set some defaults
            texture->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
            texture->setTextureWrapS(QDemonRenderTextureCoordOp::ClampToEdge);
            texture->setTextureWrapT(QDemonRenderTextureCoordOp::ClampToEdge);
        }

        if ((texture->numMipmaps() == 0) && needsMips)
            texture->generateMipmaps();

        sampler.set(texture.data());
    }

    static QDemonCustomMaterialTextureData createTextureEntry(const QDemonRef<QDemonRenderShaderProgram> &inShader,
                                                              const QDemonRef<QDemonRenderTexture2D> &inTexture,
                                                              const QByteArray &inTexName,
                                                              bool needMips)
    {
        return QDemonCustomMaterialTextureData(inShader, inTexture, inTexName, needMips);
    }
};

/**
 *	Cached tessellation property lookups this is on a per mesh base
 */
struct QDemonCustomMaterialsTessellationProperties
{
    QDemonRenderCachedShaderProperty<float> m_edgeTessLevel; ///< tesselation value for the edges
    QDemonRenderCachedShaderProperty<float> m_insideTessLevel; ///< tesselation value for the inside
    QDemonRenderCachedShaderProperty<float> m_phongBlend; ///< blending between linear and phong component
    QDemonRenderCachedShaderProperty<QVector2D> m_distanceRange; ///< distance range for min and max tess level
    QDemonRenderCachedShaderProperty<float> m_disableCulling; ///< if set to 1.0 this disables backface
    /// culling optimization in the tess shader

    QDemonCustomMaterialsTessellationProperties() = default;
    QDemonCustomMaterialsTessellationProperties(const QDemonRef<QDemonRenderShaderProgram> &inShader)
        : m_edgeTessLevel("tessLevelOuter", inShader)
        , m_insideTessLevel("tessLevelInner", inShader)
        , m_phongBlend("phongBlend", inShader)
        , m_distanceRange("distanceRange", inShader)
        , m_disableCulling("disableCulling", inShader)
    {
    }
};

/* We setup some shared state on the custom material shaders */
struct QDemonCustomMaterialShader
{
    QAtomicInt ref;
    QDemonRef<QDemonRenderShaderProgram> shader;
    QDemonRenderCachedShaderProperty<QMatrix4x4> modelMatrix;
    QDemonRenderCachedShaderProperty<QMatrix4x4> viewProjMatrix;
    QDemonRenderCachedShaderProperty<QMatrix4x4> viewMatrix;
    QDemonRenderCachedShaderProperty<QMatrix3x3> normalMatrix;
    QDemonRenderCachedShaderProperty<QVector3D> cameraPos;
    QDemonRenderCachedShaderProperty<QMatrix4x4> projMatrix;
    QDemonRenderCachedShaderProperty<QMatrix4x4> viewportMatrix;
    QDemonRenderCachedShaderProperty<QVector2D> camProperties;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> depthTexture;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> aoTexture;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> lightProbe;
    QDemonRenderCachedShaderProperty<QVector4D> lightProbeProps;
    QDemonRenderCachedShaderProperty<QVector4D> lightProbeOpts;
    QDemonRenderCachedShaderProperty<QVector4D> lightProbeRot;
    QDemonRenderCachedShaderProperty<QVector4D> lightProbeOfs;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> lightProbe2;
    QDemonRenderCachedShaderProperty<QVector4D> lightProbe2Props;
    QDemonRenderCachedShaderProperty<qint32> lightCount;
    QDemonRenderCachedShaderProperty<qint32> areaLightCount;
    QDemonRenderCachedShaderBuffer<QDemonRenderShaderConstantBuffer> aoShadowParams;
    QDemonCustomMaterialsTessellationProperties tessellation;
    dynamic::QDemonDynamicShaderProgramFlags programFlags;

    QDemonCustomMaterialShader(const QDemonRef<QDemonRenderShaderProgram> &inShader, dynamic::QDemonDynamicShaderProgramFlags inFlags)
        : shader(inShader)
        , modelMatrix("model_matrix", inShader)
        , viewProjMatrix("model_view_projection", inShader)
        , viewMatrix("view_matrix", inShader)
        , normalMatrix("normal_matrix", inShader)
        , cameraPos("camera_position", inShader)
        , projMatrix("view_projection_matrix", inShader)
        , viewportMatrix("viewport_matrix", inShader)
        , camProperties("camera_properties", inShader)
        , depthTexture("depth_sampler", inShader)
        , aoTexture("ao_sampler", inShader)
        , lightProbe("light_probe", inShader)
        , lightProbeProps("light_probe_props", inShader)
        , lightProbeOpts("light_probe_opts", inShader)
        , lightProbeRot("light_probe_rotation", inShader)
        , lightProbeOfs("light_probe_offset", inShader)
        , lightProbe2("light_probe2", inShader)
        , lightProbe2Props("light_probe2_props", inShader)
        , lightCount("uNumLights", inShader)
        , areaLightCount("uNumAreaLights", inShader)
        , aoShadowParams("cbAoShadow", inShader)
        , tessellation(inShader)
        , programFlags(inFlags)
    {
    }
};

struct QDemonMaterialOrComputeShader
{
    // TODO: struct/class?
    QDemonRef<QDemonCustomMaterialShader> m_materialShader;
    QDemonRef<QDemonRenderShaderProgram> m_computeShader;
    QDemonMaterialOrComputeShader() = default;
    QDemonMaterialOrComputeShader(const QDemonRef<QDemonCustomMaterialShader> &inMaterialShader)
        : m_materialShader(inMaterialShader)
    {
    }
    QDemonMaterialOrComputeShader(const QDemonRef<QDemonRenderShaderProgram> &inComputeShader)
        : m_computeShader(inComputeShader)
    {
        Q_ASSERT(inComputeShader->programType() == QDemonRenderShaderProgram::ProgramType::Compute);
    }
    bool isValid() const { return m_materialShader || m_computeShader; }
    bool isComputeShader() const { return m_computeShader != nullptr; }
    bool isMaterialShader() const { return m_materialShader != nullptr; }
    QDemonRef<QDemonCustomMaterialShader> materialShader()
    {
        Q_ASSERT(isMaterialShader());
        return m_materialShader;
    }
    QDemonRef<QDemonRenderShaderProgram> computeShader()
    {
        Q_ASSERT(isComputeShader());
        return m_computeShader;
    }
};

struct QDemonCustomMaterialBuffer
{
    QString name;
    QDemonRef<QDemonRenderFrameBuffer> frameBuffer;
    QDemonRef<QDemonRenderTexture2D> texture;
    dynamic::QDemonAllocateBufferFlags flags;

    QDemonCustomMaterialBuffer(const QString &inName,
                               const QDemonRef<QDemonRenderFrameBuffer> &inFb,
                               const QDemonRef<QDemonRenderTexture2D> &inTexture,
                               dynamic::QDemonAllocateBufferFlags inFlags)
        : name(inName), frameBuffer(inFb), texture(inTexture), flags(inFlags)
    {
    }
    QDemonCustomMaterialBuffer() = default;
};

struct QDemonStringMemoryBarrierFlagMap
{
    const char *name;
    QDemonRenderBufferBarrierValues value;
    constexpr QDemonStringMemoryBarrierFlagMap(const char *nm, QDemonRenderBufferBarrierValues val) : name(nm), value(val)
    {
    }
};

const QDemonStringMemoryBarrierFlagMap g_StringMemoryFlagMap[] = {
    QDemonStringMemoryBarrierFlagMap("vertex_attribute", QDemonRenderBufferBarrierValues::VertexAttribArray),
    QDemonStringMemoryBarrierFlagMap("element_array", QDemonRenderBufferBarrierValues::ElementArray),
    QDemonStringMemoryBarrierFlagMap("uniform_buffer", QDemonRenderBufferBarrierValues::UniformBuffer),
    QDemonStringMemoryBarrierFlagMap("texture_fetch", QDemonRenderBufferBarrierValues::TextureFetch),
    QDemonStringMemoryBarrierFlagMap("shader_image_access", QDemonRenderBufferBarrierValues::ShaderImageAccess),
    QDemonStringMemoryBarrierFlagMap("command_buffer", QDemonRenderBufferBarrierValues::CommandBuffer),
    QDemonStringMemoryBarrierFlagMap("pixel_buffer", QDemonRenderBufferBarrierValues::PixelBuffer),
    QDemonStringMemoryBarrierFlagMap("texture_update", QDemonRenderBufferBarrierValues::TextureUpdate),
    QDemonStringMemoryBarrierFlagMap("buffer_update", QDemonRenderBufferBarrierValues::BufferUpdate),
    QDemonStringMemoryBarrierFlagMap("frame_buffer", QDemonRenderBufferBarrierValues::Framebuffer),
    QDemonStringMemoryBarrierFlagMap("transform_feedback", QDemonRenderBufferBarrierValues::TransformFeedback),
    QDemonStringMemoryBarrierFlagMap("atomic_counter", QDemonRenderBufferBarrierValues::AtomicCounter),
    QDemonStringMemoryBarrierFlagMap("shader_storage", QDemonRenderBufferBarrierValues::ShaderStorage),
};

struct QDemonStringBlendFuncMap
{
    const char *name;
    QDemonRenderSrcBlendFunc value;
    constexpr QDemonStringBlendFuncMap(const char *nm, QDemonRenderSrcBlendFunc val) : name(nm), value(val) {}
};

const QDemonStringBlendFuncMap g_BlendFuncMap[] = {
    QDemonStringBlendFuncMap("Unknown", QDemonRenderSrcBlendFunc::Unknown),
    QDemonStringBlendFuncMap("Zero", QDemonRenderSrcBlendFunc::Zero),
    QDemonStringBlendFuncMap("One", QDemonRenderSrcBlendFunc::One),
    QDemonStringBlendFuncMap("SrcColor", QDemonRenderSrcBlendFunc::SrcColor),
    QDemonStringBlendFuncMap("OneMinusSrcColor", QDemonRenderSrcBlendFunc::OneMinusSrcColor),
    QDemonStringBlendFuncMap("DstColor", QDemonRenderSrcBlendFunc::DstColor),
    QDemonStringBlendFuncMap("OneMinusDstColor", QDemonRenderSrcBlendFunc::OneMinusDstColor),
    QDemonStringBlendFuncMap("SrcAlpha", QDemonRenderSrcBlendFunc::SrcAlpha),
    QDemonStringBlendFuncMap("OneMinusSrcAlpha", QDemonRenderSrcBlendFunc::OneMinusSrcAlpha),
    QDemonStringBlendFuncMap("DstAlpha", QDemonRenderSrcBlendFunc::DstAlpha),
    QDemonStringBlendFuncMap("OneMinusDstAlpha", QDemonRenderSrcBlendFunc::OneMinusDstAlpha),
    QDemonStringBlendFuncMap("ConstantColor", QDemonRenderSrcBlendFunc::ConstantColor),
    QDemonStringBlendFuncMap("OneMinusConstantColor", QDemonRenderSrcBlendFunc::OneMinusConstantColor),
    QDemonStringBlendFuncMap("ConstantAlpha", QDemonRenderSrcBlendFunc::ConstantAlpha),
    QDemonStringBlendFuncMap("OneMinusConstantAlpha", QDemonRenderSrcBlendFunc::OneMinusConstantAlpha),
    QDemonStringBlendFuncMap("SrcAlphaSaturate", QDemonRenderSrcBlendFunc::SrcAlphaSaturate)
};


QDemonMaterialSystem::QDemonMaterialSystem(QDemonRenderContextCoreInterface *ct)
{
    coreContext = ct;
}

QDemonMaterialSystem::~QDemonMaterialSystem()
{
    while (allocatedBuffers.size()) { // replace_with_last
        allocatedBuffers[0] = allocatedBuffers.back();
        allocatedBuffers.pop_back();
    }

    for (qint32 idx = 0; idx < allocatedImages.size(); ++idx) {
        QDemonRenderImage *pImage = allocatedImages[idx].second;
        ::free(pImage);
    }
    allocatedImages.clear();
}

void QDemonMaterialSystem::releaseBuffer(qint32 inIdx)
{
    // Don't call this on MaterialSystem destroy.
    // This causes issues for scene liftime buffers
    // because the resource manager is destroyed before
    QDemonRef<QDemonResourceManagerInterface> theManager(context->getResourceManager());
    QDemonCustomMaterialBuffer &theEntry(allocatedBuffers[inIdx]);
    theEntry.frameBuffer->attach(QDemonRenderFrameBufferAttachment::Color0, QDemonRenderTextureOrRenderBuffer());

    theManager->release(theEntry.frameBuffer);
    theManager->release(theEntry.texture);
    { // replace_with_last
        allocatedBuffers[inIdx] = allocatedBuffers.back();
        allocatedBuffers.pop_back();
    }
}

bool QDemonMaterialSystem::isMaterialRegistered(const QString &inStr)
{
    return stringMaterialMap.find(inStr) != stringMaterialMap.end();
}

bool QDemonMaterialSystem::registerMaterialClass(const QString &inName, const QDemonConstDataRef<dynamic::QDemonPropertyDeclaration> &inProperties)
{
    if (isMaterialRegistered(inName))
        return false;
    coreContext->getDynamicObjectSystemCore()->doRegister(inName,
                                                            inProperties,
                                                            sizeof(QDemonRenderCustomMaterial),
                                                            QDemonGraphObject::Type::CustomMaterial);
    QDemonDynamicObjectClassInterface *theClass = coreContext->getDynamicObjectSystemCore()->getDynamicObjectClass(inName);
    if (theClass == nullptr) {
        Q_ASSERT(false);
        return false;
    }
    QDemonRef<QDemonMaterialClass> theNewClass(new QDemonMaterialClass(*theClass));
    stringMaterialMap.insert(inName, theNewClass);
    return true;
}

QDemonMaterialClass *QDemonMaterialSystem::getMaterialClass(const QString &inStr)
{
    auto theIter = stringMaterialMap.constFind(inStr);
    if (theIter != stringMaterialMap.constEnd())
        return theIter.value().data();
    return nullptr;
}

const QDemonMaterialClass *QDemonMaterialSystem::getMaterialClass(const QString &inStr) const
{
    return const_cast<QDemonMaterialSystem *>(this)->getMaterialClass(inStr);
}

QDemonConstDataRef<dynamic::QDemonPropertyDefinition> QDemonMaterialSystem::getCustomMaterialProperties(const QString &inCustomMaterialName) const
{
    QDemonDynamicObjectClassInterface *theMaterialClass = coreContext->getDynamicObjectSystemCore()->getDynamicObjectClass(
                inCustomMaterialName);

    if (theMaterialClass)
        return theMaterialClass->getProperties();

    return QDemonConstDataRef<dynamic::QDemonPropertyDefinition>();
}

qint32 QDemonMaterialSystem::findBuffer(const QString &inName)
{
    for (qint32 idx = 0, end = allocatedBuffers.size(); idx < end; ++idx)
        if (allocatedBuffers[idx].name == inName)
            return idx;
    return allocatedBuffers.size();
}

qint32 QDemonMaterialSystem::findAllocatedImage(const QString &inName)
{
    for (qint32 idx = 0, end = allocatedImages.size(); idx < end; ++idx)
        if (allocatedImages[idx].first == inName)
            return idx;
    return -1;
}

bool QDemonMaterialSystem::textureNeedsMips(const dynamic::QDemonPropertyDefinition *inPropDec, QDemonRenderTexture2D *inTexture)
{
    if (inPropDec && inTexture) {
        return bool((inPropDec->minFilterOp == QDemonRenderTextureMinifyingOp::LinearMipmapLinear)
                    && (inTexture->numMipmaps() == 0));
    }

    return false;
}

void QDemonMaterialSystem::setTexture(const QDemonRef<QDemonRenderShaderProgram> &inShader, const QString &inPropName,
                                      const QDemonRef<QDemonRenderTexture2D> &inTexture, const dynamic::QDemonPropertyDefinition *inPropDec,
                                      bool needMips)
{
    QDemonRef<QDemonCustomMaterialTextureData> theTextureEntry;
    for (quint32 idx = 0, end = textureEntries.size(); idx < end && theTextureEntry == nullptr; ++idx) {
        if (textureEntries[idx].first == inPropName && textureEntries[idx].second->shader == inShader
                && textureEntries[idx].second->texture == inTexture) {
            theTextureEntry = textureEntries[idx].second;
            break;
        }
    }
    if (theTextureEntry == nullptr) {
        QDemonRef<QDemonCustomMaterialTextureData> theNewEntry(new QDemonCustomMaterialTextureData(
                                                                   QDemonCustomMaterialTextureData::createTextureEntry(inShader, inTexture, inPropName.toUtf8(), needMips)));
        textureEntries.push_back(QPair<QString, QDemonRef<QDemonCustomMaterialTextureData>>(inPropName, theNewEntry));
        theTextureEntry = theNewEntry;
    }
    theTextureEntry->set(inPropDec);
}

QDemonMaterialSystem::QDemonMaterialSystem() = default;

void QDemonMaterialSystem::setPropertyEnumNames(const QString &inName, const QString &inPropName, const QDemonConstDataRef<QString> &inNames)
{
    coreContext->getDynamicObjectSystemCore()->setPropertyEnumNames(inName, inPropName, inNames);
}

void QDemonMaterialSystem::setPropertyTextureSettings(const QString &inName, const QString &inPropName, const QString &inPropPath, QDemonRenderTextureTypeValue inTexType, QDemonRenderTextureCoordOp inCoordOp, QDemonRenderTextureMagnifyingOp inMagFilterOp, QDemonRenderTextureMinifyingOp inMinFilterOp)
{
    QDemonMaterialClass *theClass = getMaterialClass(inName);
    if (theClass && inTexType == QDemonRenderTextureTypeValue::Displace) {
        theClass->m_hasDisplacement = true;
    }
    coreContext->getDynamicObjectSystemCore()
            ->setPropertyTextureSettings(inName, inPropName, inPropPath, inTexType, inCoordOp, inMagFilterOp, inMinFilterOp);
}

void QDemonMaterialSystem::setMaterialClassShader(QString inName, const char *inShaderType, const char *inShaderVersion, const char *inShaderData, bool inHasGeomShader, bool inIsComputeShader)
{
    coreContext->getDynamicObjectSystemCore()->setShaderData(inName, inShaderData, inShaderType, inShaderVersion, inHasGeomShader, inIsComputeShader);
}

QDemonRenderCustomMaterial *QDemonMaterialSystem::createCustomMaterial(const QString &inName)
{
    QDemonRenderCustomMaterial *theMaterial = static_cast<QDemonRenderCustomMaterial *>(
                coreContext->getDynamicObjectSystemCore()->createInstance(inName));
    QDemonMaterialClass *theClass = getMaterialClass(inName);

    if (theMaterial) {
        quint32 key = 0, count = 0;
        if (theClass) {
            key = theClass->m_shaderKey;
            count = theClass->m_layerCount;
        }
        theMaterial->initialize(key, count);
    }

    return theMaterial;
}

void QDemonMaterialSystem::setCustomMaterialTransparency(const QString &inName, bool inHasTransparency)
{
    QDemonMaterialClass *theClass = getMaterialClass(inName);

    if (theClass == nullptr) {
        Q_ASSERT(false);
        return;
    }

    theClass->m_hasTransparency = inHasTransparency;
}

void QDemonMaterialSystem::setCustomMaterialRefraction(const QString &inName, bool inHasRefraction)
{
    QDemonMaterialClass *theClass = getMaterialClass(inName);

    if (theClass == nullptr) {
        Q_ASSERT(false);
        return;
    }

    theClass->m_hasRefraction = inHasRefraction;
}

void QDemonMaterialSystem::setCustomMaterialAlwaysDirty(const QString &inName, bool inIsAlwaysDirty)
{
    QDemonMaterialClass *theClass = getMaterialClass(inName);

    if (theClass == nullptr) {
        Q_ASSERT(false);
        return;
    }

    theClass->m_alwaysDirty = inIsAlwaysDirty;
}

void QDemonMaterialSystem::setCustomMaterialShaderKey(const QString &inName, quint32 inShaderKey)
{
    QDemonMaterialClass *theClass = getMaterialClass(inName);

    if (theClass == nullptr) {
        Q_ASSERT(false);
        return;
    }

    theClass->m_shaderKey = inShaderKey;
}

void QDemonMaterialSystem::setCustomMaterialLayerCount(const QString &inName, quint32 inLayerCount)
{
    QDemonMaterialClass *theClass = getMaterialClass(inName);

    if (theClass == nullptr) {
        Q_ASSERT(false);
        return;
    }

    theClass->m_layerCount = inLayerCount;
}

void QDemonMaterialSystem::setCustomMaterialCommands(QString inName, QDemonConstDataRef<dynamic::QDemonCommand *> inCommands)
{
    coreContext->getDynamicObjectSystemCore()->setRenderCommands(inName, inCommands);
}

QDemonRef<QDemonRenderShaderProgram> QDemonMaterialSystem::getShader(QDemonCustomMaterialRenderContext &inRenderContext, const QDemonRenderCustomMaterial &inMaterial, const dynamic::QDemonBindShader &inCommand, const TShaderFeatureSet &inFeatureSet, const dynamic::QDemonDynamicShaderProgramFlags &inFlags)
{
    QDemonRef<QDemonMaterialShaderGeneratorInterface> theMaterialGenerator(context->getCustomMaterialShaderGenerator());

    // generate key
    //        QString theKey = getShaderCacheKey(theShaderKeyBuffer, inCommand.m_shaderPath,
    //                                           inCommand.m_shaderDefine, inFlags);
    // ### TODO: Enable caching?

    QDemonCustomMaterialVertexPipeline thePipeline(context, inRenderContext.model.tessellationMode);

    QDemonRef<QDemonRenderShaderProgram>
            theProgram = theMaterialGenerator->generateShader(inMaterial,
                                                              inRenderContext.materialKey,
                                                              thePipeline,
                                                              inFeatureSet,
                                                              inRenderContext.lights,
                                                              inRenderContext.firstImage,
                                                              (inMaterial.m_hasTransparency || inMaterial.m_hasRefraction),
                                                              "custom material pipeline-- ",
                                                              inCommand.m_shaderPath.toUtf8());

    return theProgram;
}

QDemonMaterialOrComputeShader QDemonMaterialSystem::bindShader(QDemonCustomMaterialRenderContext &inRenderContext, const QDemonRenderCustomMaterial &inMaterial, const dynamic::QDemonBindShader &inCommand, const TShaderFeatureSet &inFeatureSet)
{
    QDemonRef<QDemonRenderShaderProgram> theProgram;

    dynamic::QDemonDynamicShaderProgramFlags theFlags(inRenderContext.model.tessellationMode, inRenderContext.subset.wireframeMode);
    if (inRenderContext.model.tessellationMode != TessModeValues::NoTess)
        theFlags |= ShaderCacheProgramFlagValues::TessellationEnabled;
    if (inRenderContext.subset.wireframeMode)
        theFlags |= ShaderCacheProgramFlagValues::GeometryShaderEnabled;

    QDemonShaderMapKey skey = QDemonShaderMapKey(TStrStrPair(inCommand.m_shaderPath, inCommand.m_shaderDefine),
                                                 inFeatureSet,
                                                 theFlags.tessMode,
                                                 theFlags.wireframeMode,
                                                 inRenderContext.materialKey);
    auto theInsertResult = shaderMap.find(skey);
    // QPair<TShaderMap::iterator, bool> theInsertResult(m_ShaderMap.insert(skey, QDemonRef<SCustomMaterialShader>(nullptr)));

    if (theInsertResult == shaderMap.end()) {
        theProgram = getShader(inRenderContext, inMaterial, inCommand, inFeatureSet, theFlags);

        if (theProgram) {
            theInsertResult = shaderMap.insert(skey,
                                                 QDemonRef<QDemonCustomMaterialShader>(
                                                     new QDemonCustomMaterialShader(theProgram, theFlags)));
        }
    } else if (theInsertResult.value())
        theProgram = theInsertResult.value()->shader;

    if (theProgram) {
        if (theProgram->programType() == QDemonRenderShaderProgram::ProgramType::Graphics) {
            if (theInsertResult.value()) {
                QDemonRef<QDemonRenderContext> theContext(context->getRenderContext());
                theContext->setActiveShader(theInsertResult.value()->shader);
            }

            return theInsertResult.value();
        } else {
            QDemonRef<QDemonRenderContext> theContext(context->getRenderContext());
            theContext->setActiveShader(theProgram);
            return theProgram;
        }
    }
    return QDemonMaterialOrComputeShader();
}

void QDemonMaterialSystem::doApplyInstanceValue(QDemonRenderCustomMaterial &, quint8 *inDataPtr, const QString &inPropertyName, QDemonRenderShaderDataType inPropertyType, const QDemonRef<QDemonRenderShaderProgram> &inShader, const dynamic::QDemonPropertyDefinition &inDefinition)
{
    QDemonRef<QDemonRenderShaderConstantBase> theConstant = inShader->shaderConstant(inPropertyName.toLocal8Bit());
    if (theConstant) {
        if (theConstant->getShaderConstantType() == inPropertyType) {
            if (inPropertyType == QDemonRenderShaderDataType::Texture2D) {
                //                    StaticAssert<sizeof(QString) == sizeof(QDemonRenderTexture2DPtr)>::valid_expression();
                QString *theStrPtr = reinterpret_cast<QString *>(inDataPtr);
                QDemonBufferManager theBufferManager(context->getBufferManager());
                QDemonRef<QDemonRenderTexture2D> theTexture;

                if (!theStrPtr->isNull()) {
                    QDemonRenderImageTextureData theTextureData = theBufferManager.loadRenderImage(*theStrPtr);
                    if (theTextureData.m_texture) {
                        theTexture = theTextureData.m_texture;
                        setTexture(inShader,
                                   inPropertyName,
                                   theTexture,
                                   &inDefinition,
                                   textureNeedsMips(&inDefinition, theTexture.data()));
                    }
                }
            } else {
                switch (inPropertyType) {
                case QDemonRenderShaderDataType::Integer:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<qint32 *>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::IntegerVec2:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<qint32_2 *>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::IntegerVec3:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<qint32_3 *>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::IntegerVec4:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<qint32_4 *>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::Boolean:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<bool *>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::BooleanVec2:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<bool_2 *>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::BooleanVec3:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<bool_3 *>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::BooleanVec4:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<bool_4 *>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::Float:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<float *>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::Vec2:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QVector2D *>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::Vec3:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QVector3D *>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::Vec4:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QVector4D *>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::UnsignedInteger:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<quint32 *>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::UnsignedIntegerVec2:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<quint32_2 *>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::UnsignedIntegerVec3:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<quint32_3 *>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::UnsignedIntegerVec4:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<quint32_4 *>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::Matrix3x3:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QMatrix3x3 *>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::Matrix4x4:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QMatrix4x4 *>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::Texture2D:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderTexture2D **>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::Texture2DHandle:
                    inShader->setPropertyValue(theConstant.data(),
                                               *(reinterpret_cast<QDemonRenderTexture2D ***>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::Texture2DArray:
                    inShader->setPropertyValue(theConstant.data(),
                                               *(reinterpret_cast<QDemonRenderTexture2DArray **>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::TextureCube:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderTextureCube **>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::TextureCubeHandle:
                    inShader->setPropertyValue(theConstant.data(),
                                               *(reinterpret_cast<QDemonRenderTextureCube ***>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::Image2D:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderImage2D **>(inDataPtr)));
                    break;
                case QDemonRenderShaderDataType::DataBuffer:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderDataBuffer **>(inDataPtr)));
                    break;
                default:
                    Q_UNREACHABLE();
                }
            }
        } else {
            qCCritical(INVALID_OPERATION,
                       "CustomMaterial ApplyInstanceValue command datatype and "
                       "shader datatypes differ for property %s",
                       qPrintable(inPropertyName));
            Q_ASSERT(false);
        }
    }
}

void QDemonMaterialSystem::applyInstanceValue(QDemonRenderCustomMaterial &inMaterial, QDemonMaterialClass &inClass, const QDemonRef<QDemonRenderShaderProgram> &inShader, const dynamic::QDemonApplyInstanceValue &inCommand)
{
    // sanity check
    if (!inCommand.m_propertyName.isNull()) {
        bool canGetData = inCommand.m_valueOffset + dynamic::getSizeofShaderDataType(inCommand.m_valueType)
                <= inMaterial.dataSectionByteSize;
        if (canGetData == false) {
            Q_ASSERT(false);
            return;
        }
        quint8 *dataPtr = inMaterial.getDataSectionBegin() + inCommand.m_valueOffset;
        const dynamic::QDemonPropertyDefinition *theDefinition = inClass.m_class->findPropertyByName(inCommand.m_propertyName);
        if (theDefinition)
            doApplyInstanceValue(inMaterial, dataPtr, inCommand.m_propertyName, inCommand.m_valueType, inShader, *theDefinition);
    } else {
        QDemonConstDataRef<dynamic::QDemonPropertyDefinition> theDefs = inClass.m_class->getProperties();
        for (quint32 idx = 0, end = theDefs.size(); idx < end; ++idx) {
            const dynamic::QDemonPropertyDefinition &theDefinition(theDefs[idx]);
            QDemonRef<QDemonRenderShaderConstantBase> theConstant = inShader->shaderConstant(
                        theDefinition.name.toLocal8Bit().constData());

            // This is fine, the property wasn't found and we continue, no problem.
            if (!theConstant)
                continue;
            quint8 *dataPtr = inMaterial.getDataSectionBegin() + theDefinition.offset;
            doApplyInstanceValue(inMaterial, dataPtr, theDefinition.name, theDefinition.dataType, inShader, theDefinition);
        }
    }
}

void QDemonMaterialSystem::applyBlending(const dynamic::QDemonApplyBlending &inCommand)
{
    QDemonRef<QDemonRenderContext> theContext(context->getRenderContext());

    theContext->setBlendingEnabled(true);

    QDemonRenderBlendFunctionArgument blendFunc = QDemonRenderBlendFunctionArgument(inCommand.m_srcBlendFunc,
                                                                                    inCommand.m_dstBlendFunc,
                                                                                    inCommand.m_srcBlendFunc,
                                                                                    inCommand.m_dstBlendFunc);

    QDemonRenderBlendEquationArgument blendEqu(QDemonRenderBlendEquation::Add, QDemonRenderBlendEquation::Add);

    theContext->setBlendFunction(blendFunc);
    theContext->setBlendEquation(blendEqu);
}

const QDemonRef<QDemonRenderTexture2D> QDemonMaterialSystem::applyBufferValue(const QDemonRenderCustomMaterial &inMaterial, const QDemonRef<QDemonRenderShaderProgram> &inShader, const dynamic::QDemonApplyBufferValue &inCommand, const QDemonRef<QDemonRenderTexture2D> inSourceTexture)
{
    QDemonRef<QDemonRenderTexture2D> theTexture = nullptr;

    if (!inCommand.m_bufferName.isNull()) {
        qint32 bufferIdx = findBuffer(inCommand.m_bufferName);
        if (bufferIdx < allocatedBuffers.size()) {
            QDemonCustomMaterialBuffer &theEntry(allocatedBuffers[bufferIdx]);
            theTexture = theEntry.texture;
        } else {
            // we must have allocated the read target before
            qCCritical(INTERNAL_ERROR, "CustomMaterial: ApplyBufferValue: Failed to setup read target");
            Q_ASSERT(false);
        }
    } else
        theTexture = inSourceTexture;

    if (!inCommand.m_paramName.isNull()) {
        QDemonRef<QDemonRenderShaderConstantBase> theConstant = inShader->shaderConstant(
                    inCommand.m_paramName.toLocal8Bit().constData());

        if (theConstant) {
            if (theConstant->getShaderConstantType() != QDemonRenderShaderDataType::Texture2D) {
                qCCritical(INVALID_OPERATION,
                           "CustomMaterial %s: Binding buffer to parameter %s that is not a texture",
                           qPrintable(inMaterial.className),
                           qPrintable(inCommand.m_paramName));
                Q_ASSERT(false);
            } else {
                setTexture(inShader, inCommand.m_paramName, theTexture);
            }
        }
    }

    return theTexture;
}

void QDemonMaterialSystem::allocateBuffer(const dynamic::QDemonAllocateBuffer &inCommand, const QDemonRef<QDemonRenderFrameBuffer> &inTarget)
{
    QDemonTextureDetails theSourceTextureDetails;
    QDemonRef<QDemonRenderTexture2D> theTexture;
    // get color attachment we always assume at location 0
    if (inTarget) {
        QDemonRenderTextureOrRenderBuffer theSourceTexture = inTarget->attachment(QDemonRenderFrameBufferAttachment::Color0);
        // we need a texture
        if (theSourceTexture.hasTexture2D()) {
            theSourceTextureDetails = theSourceTexture.texture2D()->textureDetails();
        } else {
            qCCritical(INVALID_OPERATION, "CustomMaterial %s: Invalid source texture", qPrintable(inCommand.m_name));
            Q_ASSERT(false);
            return;
        }
    } else {
        QDemonRef<QDemonRenderContext> theContext = context->getRenderContext();
        // if we allocate a buffer based on the default target use viewport to get the dimension
        QRect theViewport(theContext->viewport());
        theSourceTextureDetails.height = theViewport.height();
        theSourceTextureDetails.width = theViewport.width();
    }

    const qint32 theWidth = qint32(theSourceTextureDetails.width * inCommand.m_sizeMultiplier);
    const qint32 theHeight = qint32(theSourceTextureDetails.height * inCommand.m_sizeMultiplier);
    QDemonRenderTextureFormat theFormat = inCommand.m_format;
    if (theFormat == QDemonRenderTextureFormat::Unknown)
        theFormat = theSourceTextureDetails.format;
    QDemonRef<QDemonResourceManagerInterface> theResourceManager(context->getResourceManager());
    // size intentionally requiried every loop;
    qint32 bufferIdx = findBuffer(inCommand.m_name);
    if (bufferIdx < allocatedBuffers.size()) {
        QDemonCustomMaterialBuffer &theEntry(allocatedBuffers[bufferIdx]);
        QDemonTextureDetails theDetails = theEntry.texture->textureDetails();
        if (theDetails.width == theWidth && theDetails.height == theHeight && theDetails.format == theFormat) {
            theTexture = theEntry.texture;
        } else {
            releaseBuffer(bufferIdx);
        }
    }

    if (theTexture == nullptr) {
        QDemonRef<QDemonRenderFrameBuffer> theFB(theResourceManager->allocateFrameBuffer());
        QDemonRef<QDemonRenderTexture2D> theTexture(theResourceManager->allocateTexture2D(theWidth, theHeight, theFormat));
        theTexture->setMagFilter(inCommand.m_filterOp);
        theTexture->setMinFilter(static_cast<QDemonRenderTextureMinifyingOp>(inCommand.m_filterOp));
        theTexture->setTextureWrapS(inCommand.m_texCoordOp);
        theTexture->setTextureWrapT(inCommand.m_texCoordOp);
        theFB->attach(QDemonRenderFrameBufferAttachment::Color0, theTexture);
        allocatedBuffers.push_back(QDemonCustomMaterialBuffer(inCommand.m_name, theFB, theTexture, inCommand.m_bufferFlags));
    }
}

QDemonRef<QDemonRenderFrameBuffer> QDemonMaterialSystem::bindBuffer(const QDemonRenderCustomMaterial &inMaterial, const dynamic::QDemonBindBuffer &inCommand, bool &outClearTarget, QVector2D &outDestSize)
{
    QDemonRef<QDemonRenderFrameBuffer> theBuffer;
    QDemonRef<QDemonRenderTexture2D> theTexture;

    // search for the buffer
    qint32 bufferIdx = findBuffer(inCommand.m_bufferName);
    if (bufferIdx < allocatedBuffers.size()) {
        theBuffer = allocatedBuffers[bufferIdx].frameBuffer;
        theTexture = allocatedBuffers[bufferIdx].texture;
    }

    if (theBuffer == nullptr) {
        qCCritical(INVALID_OPERATION,
                   "Effect %s: Failed to find buffer %s for bind",
                   qPrintable(inMaterial.className),
                   qPrintable(inCommand.m_bufferName));
        Q_ASSERT(false);
        return nullptr;
    }

    if (theTexture) {
        QDemonTextureDetails theDetails(theTexture->textureDetails());
        context->getRenderContext()->setViewport(QRect(0, 0, theDetails.width, theDetails.height));
        outDestSize = QVector2D(float(theDetails.width), float(theDetails.height));
        outClearTarget = inCommand.m_needsClear;
    }

    return theBuffer;
}

void QDemonMaterialSystem::computeScreenCoverage(QDemonCustomMaterialRenderContext &inRenderContext, qint32 *xMin, qint32 *yMin, qint32 *xMax, qint32 *yMax)
{
    QDemonRef<QDemonRenderContext> theContext(context->getRenderContext());
    QDemonBounds2BoxPoints outPoints;
    const float MaxFloat = std::numeric_limits<float>::max();
    QVector4D projMin(MaxFloat, MaxFloat, MaxFloat, MaxFloat);
    QVector4D projMax(-MaxFloat, -MaxFloat, -MaxFloat, -MaxFloat);

    // get points
    inRenderContext.subset.bounds.expand(outPoints);
    for (quint32 idx = 0; idx < 8; ++idx) {
        QVector4D homPoint(outPoints[idx], 1.0);
        QVector4D projPoint = mat44::transform(inRenderContext.modelViewProjection, homPoint);
        projPoint /= projPoint.w();

        if (projMin.x() > projPoint.x())
            projMin.setX(projPoint.x());
        if (projMin.y() > projPoint.y())
            projMin.setY(projPoint.y());
        if (projMin.z() > projPoint.z())
            projMin.setZ(projPoint.z());

        if (projMax.x() < projPoint.x())
            projMax.setX(projPoint.x());
        if (projMax.y() < projPoint.y())
            projMax.setY(projPoint.y());
        if (projMax.z() < projPoint.z())
            projMax.setZ(projPoint.z());
    }

    QRect theViewport(theContext->viewport());
    qint32 x1 = qint32(projMax.x() * (theViewport.width() / 2) + (theViewport.x() + (theViewport.width() / 2)));
    qint32 y1 = qint32(projMax.y() * (theViewport.height() / 2) + (theViewport.y() + (theViewport.height() / 2)));

    qint32 x2 = qint32(projMin.x() * (theViewport.width() / 2) + (theViewport.x() + (theViewport.width() / 2)));
    qint32 y2 = qint32(projMin.y() * (theViewport.height() / 2) + (theViewport.y() + (theViewport.height() / 2)));

    if (x1 > x2) {
        *xMin = x2;
        *xMax = x1;
    } else {
        *xMin = x1;
        *xMax = x2;
    }
    if (y1 > y2) {
        *yMin = y2;
        *yMax = y1;
    } else {
        *yMin = y1;
        *yMax = y2;
    }
}

void QDemonMaterialSystem::blitFramebuffer(QDemonCustomMaterialRenderContext &inRenderContext, const dynamic::QDemonApplyBlitFramebuffer &inCommand, const QDemonRef<QDemonRenderFrameBuffer> &inTarget)
{
    QDemonRef<QDemonRenderContext> theContext(context->getRenderContext());
    // we change the read/render targets here
    QDemonRenderContextScopedProperty<QDemonRef<QDemonRenderFrameBuffer>> __framebuffer(*theContext,
                                                                                        &QDemonRenderContext::renderTarget,
                                                                                        &QDemonRenderContext::setRenderTarget);
    // we may alter scissor
    QDemonRenderContextScopedProperty<bool> theScissorEnabled(*theContext,
                                                              &QDemonRenderContext::isScissorTestEnabled,
                                                              &QDemonRenderContext::setScissorTestEnabled);

    if (!inCommand.m_destBufferName.isNull()) {
        qint32 bufferIdx = findBuffer(inCommand.m_destBufferName);
        if (bufferIdx < allocatedBuffers.size()) {
            QDemonCustomMaterialBuffer &theEntry(allocatedBuffers[bufferIdx]);
            theContext->setRenderTarget(theEntry.frameBuffer);
        } else {
            // we must have allocated the read target before
            qCCritical(INTERNAL_ERROR, "CustomMaterial: BlitFramebuffer: Failed to setup render target");
            Q_ASSERT(false);
        }
    } else {
        // our dest is the default render target
        theContext->setRenderTarget(inTarget);
    }

    if (!inCommand.m_sourceBufferName.isNull()) {
        qint32 bufferIdx = findBuffer(inCommand.m_sourceBufferName);
        if (bufferIdx < allocatedBuffers.size()) {
            QDemonCustomMaterialBuffer &theEntry(allocatedBuffers[bufferIdx]);
            theContext->setReadTarget(theEntry.frameBuffer);
            theContext->setReadBuffer(QDemonReadFace::Color0);
        } else {
            // we must have allocated the read target before
            qCCritical(INTERNAL_ERROR, "CustomMaterial: BlitFramebuffer: Failed to setup read target");
            Q_ASSERT(false);
        }
    } else {
        // our source is the default read target
        // depending on what we render we assume color0 or back
        theContext->setReadTarget(inTarget);
        QDemonReadFace value = (inTarget) ? QDemonReadFace::Color0 : QDemonReadFace::Back;
        theContext->setReadBuffer(value);
    }

    QRect theViewport(theContext->viewport());
    theContext->setScissorTestEnabled(false);

    if (!useFastBlits) {
        // only copy sreen amount of pixels
        qint32 xMin, yMin, xMax, yMax;
        computeScreenCoverage(inRenderContext, &xMin, &yMin, &xMax, &yMax);

        // same dimension
        theContext->blitFramebuffer(xMin, yMin, xMax, yMax, xMin, yMin, xMax, yMax, QDemonRenderClearValues::Color, QDemonRenderTextureMagnifyingOp::Nearest);
    } else {
        // same dimension
        theContext->blitFramebuffer(theViewport.x(),
                                    theViewport.y(),
                                    theViewport.x() + theViewport.width(),
                                    theViewport.y() + theViewport.height(),
                                    theViewport.x(),
                                    theViewport.y(),
                                    theViewport.x() + theViewport.width(),
                                    theViewport.y() + theViewport.height(),
                                    QDemonRenderClearValues::Color,
                                    QDemonRenderTextureMagnifyingOp::Nearest);
    }
}

QDemonLayerGlobalRenderProperties QDemonMaterialSystem::getLayerGlobalRenderProperties(QDemonCustomMaterialRenderContext &inRenderContext)
{
    const QDemonRenderLayer &theLayer = inRenderContext.layer;
    const QDemonLayerRenderData &theData = inRenderContext.layerData;

    QVector<QVector3D> tempDirection;

    return QDemonLayerGlobalRenderProperties{ theLayer,
                const_cast<QDemonRenderCamera &>(inRenderContext.camera),
                theData.cameraDirection,
                const_cast<QVector<QDemonRenderLight *> &>(inRenderContext.lights),
                tempDirection,
                theData.shadowMapManager,
                inRenderContext.depthTexture,
                inRenderContext.aoTexture,
                theLayer.lightProbe,
                theLayer.lightProbe2,
                theLayer.probeHorizon,
                theLayer.probeBright,
                theLayer.probe2Window,
                theLayer.probe2Pos,
                theLayer.probe2Fade,
                theLayer.probeFov };
}

void QDemonMaterialSystem::renderPass(QDemonCustomMaterialRenderContext &inRenderContext, const QDemonRef<QDemonCustomMaterialShader> &inShader, const QDemonRef<QDemonRenderTexture2D> &, const QDemonRef<QDemonRenderFrameBuffer> &inFrameBuffer, bool inRenderTargetNeedsClear, const QDemonRef<QDemonRenderInputAssembler> &inAssembler, quint32 inCount, quint32 inOffset)
{
    QDemonRef<QDemonRenderContext> theContext(context->getRenderContext());
    theContext->setRenderTarget(inFrameBuffer);

    QVector4D clearColor(0.0, 0.0, 0.0, 0.0);
    QDemonRenderContextScopedProperty<QVector4D> __clearColor(*theContext,
                                                              &QDemonRenderContext::clearColor,
                                                              &QDemonRenderContext::setClearColor,
                                                              clearColor);
    if (inRenderTargetNeedsClear) {
        theContext->clear(QDemonRenderClearValues::Color);
    }

    QDemonRef<QDemonMaterialShaderGeneratorInterface> theMaterialGenerator(context->getCustomMaterialShaderGenerator());

    theMaterialGenerator->setMaterialProperties(inShader->shader,
                                                inRenderContext.material,
                                                QVector2D(1.0, 1.0),
                                                inRenderContext.modelViewProjection,
                                                inRenderContext.normalMatrix,
                                                inRenderContext.modelMatrix,
                                                inRenderContext.firstImage,
                                                inRenderContext.opacity,
                                                getLayerGlobalRenderProperties(inRenderContext));

    // I think the prim type should always be fetched from the
    // current mesh subset setup because there you get the actual draw mode
    // for this frame
    QDemonRenderDrawMode theDrawMode = inAssembler->drawMode();

    // tesselation
    if (inRenderContext.subset.primitiveType == QDemonRenderDrawMode::Patches) {
        QVector2D camProps(inRenderContext.camera.clipNear, inRenderContext.camera.clipFar);
        theDrawMode = inRenderContext.subset.primitiveType;
        inShader->tessellation.m_edgeTessLevel.set(inRenderContext.subset.edgeTessFactor);
        inShader->tessellation.m_insideTessLevel.set(inRenderContext.subset.innerTessFactor);
        // the blend value is hardcoded
        inShader->tessellation.m_phongBlend.set(0.75);
        // this should finally be based on some user input
        inShader->tessellation.m_distanceRange.set(camProps);
        // enable culling
        inShader->tessellation.m_disableCulling.set(0.0);
    }

    if (inRenderContext.subset.wireframeMode) {
        QRect theViewport(theContext->viewport());
        QMatrix4x4 vpMatrix = { (float)theViewport.width() / 2.0f,
                                0.0,
                                0.0,
                                0.0,
                                0.0,
                                (float)theViewport.height() / 2.0f,
                                0.0,
                                0.0,
                                0.0,
                                0.0,
                                1.0,
                                0.0,
                                (float)theViewport.width() / 2.0f + (float)theViewport.x(),
                                (float)theViewport.height() / 2.0f + (float)theViewport.y(),
                                0.0,
                                1.0 };

        inShader->viewportMatrix.set(vpMatrix);
    }

    theContext->setInputAssembler(inAssembler);

    theContext->setCullingEnabled(true);
    quint32 count = inCount;
    quint32 offset = inOffset;

    theContext->draw(theDrawMode, count, offset);
}

void QDemonMaterialSystem::doRenderCustomMaterial(QDemonCustomMaterialRenderContext &inRenderContext, const QDemonRenderCustomMaterial &inMaterial, QDemonMaterialClass &inClass, const QDemonRef<QDemonRenderFrameBuffer> &inTarget, const TShaderFeatureSet &inFeatureSet)
{
    QDemonRef<QDemonRenderContext> theContext = context->getRenderContext();
    QDemonRef<QDemonCustomMaterialShader> theCurrentShader(nullptr);

    QDemonRef<QDemonRenderFrameBuffer> theCurrentRenderTarget(inTarget);
    QRect theOriginalViewport(theContext->viewport());
    QDemonRef<QDemonRenderTexture2D> theCurrentSourceTexture;

    // for refrative materials we come from the transparent render path
    // but we do not want to do blending
    bool wasBlendingEnabled = theContext->isBlendingEnabled();
    if (inMaterial.m_hasRefraction)
        theContext->setBlendingEnabled(false);

    QDemonRenderContextScopedProperty<QDemonRef<QDemonRenderFrameBuffer>> __framebuffer(*theContext,
                                                                                        &QDemonRenderContext::renderTarget,
                                                                                        &QDemonRenderContext::setRenderTarget);
    QDemonRenderContextScopedProperty<QRect> __viewport(*theContext, &QDemonRenderContext::viewport, &QDemonRenderContext::setViewport);

    QVector2D theDestSize;
    bool theRenderTargetNeedsClear = false;

    QDemonConstDataRef<dynamic::QDemonCommand *> theCommands(inClass.m_class->getRenderCommands());
    for (qint32 commandIdx = 0, commandEnd = theCommands.size(); commandIdx < commandEnd; ++commandIdx) {
        const dynamic::QDemonCommand &theCommand(*theCommands[commandIdx]);

        switch (theCommand.m_type) {
        case dynamic::CommandType::AllocateBuffer:
            allocateBuffer(static_cast<const dynamic::QDemonAllocateBuffer &>(theCommand), inTarget);
            break;
        case dynamic::CommandType::BindBuffer:
            theCurrentRenderTarget = bindBuffer(inMaterial,
                                                static_cast<const dynamic::QDemonBindBuffer &>(theCommand),
                                                theRenderTargetNeedsClear,
                                                theDestSize);
            break;
        case dynamic::CommandType::BindTarget:
            // Restore the previous render target and info.
            theCurrentRenderTarget = inTarget;
            theContext->setViewport(theOriginalViewport);
            break;
        case dynamic::CommandType::BindShader: {
            theCurrentShader = nullptr;
            QDemonMaterialOrComputeShader theBindResult = bindShader(inRenderContext,
                                                                     inMaterial,
                                                                     static_cast<const dynamic::QDemonBindShader &>(theCommand),
                                                                     inFeatureSet);
            if (theBindResult.isMaterialShader())
                theCurrentShader = theBindResult.materialShader();
        } break;
        case dynamic::CommandType::ApplyInstanceValue:
            // we apply the property update explicitly at the render pass
            break;
        case dynamic::CommandType::Render:
            if (theCurrentShader) {
                renderPass(inRenderContext,
                           theCurrentShader,
                           theCurrentSourceTexture,
                           theCurrentRenderTarget,
                           theRenderTargetNeedsClear,
                           inRenderContext.subset.inputAssembler,
                           inRenderContext.subset.count,
                           inRenderContext.subset.offset);
            }
            // reset
            theRenderTargetNeedsClear = false;
            break;
        case dynamic::CommandType::ApplyBlending:
            applyBlending(static_cast<const dynamic::QDemonApplyBlending &>(theCommand));
            break;
        case dynamic::CommandType::ApplyBufferValue:
            if (theCurrentShader)
                applyBufferValue(inMaterial,
                                 theCurrentShader->shader,
                                 static_cast<const dynamic::QDemonApplyBufferValue &>(theCommand),
                                 theCurrentSourceTexture);
            break;
        case dynamic::CommandType::ApplyBlitFramebuffer:
            blitFramebuffer(inRenderContext, static_cast<const dynamic::QDemonApplyBlitFramebuffer &>(theCommand), inTarget);
            break;
        default:
            Q_ASSERT(false);
            break;
        }
    }

    if (inMaterial.m_hasRefraction)
        theContext->setBlendingEnabled(wasBlendingEnabled);

    // Release any per-frame buffers
    for (qint32 idx = 0; idx < allocatedBuffers.size(); ++idx) {
        if (allocatedBuffers[idx].flags.isSceneLifetime() == false) {
            releaseBuffer(idx);
            --idx;
        }
    }
}

QString QDemonMaterialSystem::getShaderName(const QDemonRenderCustomMaterial &inMaterial)
{
    QDemonMaterialClass *theClass = getMaterialClass(inMaterial.className);
    if (!theClass)
        return QString();

    QDemonConstDataRef<dynamic::QDemonCommand *> theCommands = theClass->m_class->getRenderCommands();
    TShaderAndFlags thePrepassShader;
    for (qint32 idx = 0, end = theCommands.size(); idx < end && thePrepassShader.first == nullptr; ++idx) {
        const dynamic::QDemonCommand &theCommand = *theCommands[idx];
        if (theCommand.m_type == dynamic::CommandType::BindShader) {
            const dynamic::QDemonBindShader &theBindCommand = static_cast<const dynamic::QDemonBindShader &>(theCommand);
            return theBindCommand.m_shaderPath;
        }
    }

    Q_UNREACHABLE();
    return QString();
}

void QDemonMaterialSystem::applyShaderPropertyValues(const QDemonRenderCustomMaterial &inMaterial, const QDemonRef<QDemonRenderShaderProgram> &inProgram)
{
    QDemonMaterialClass *theClass = getMaterialClass(inMaterial.className);
    if (!theClass)
        return;

    dynamic::QDemonApplyInstanceValue applier;
    applyInstanceValue(const_cast<QDemonRenderCustomMaterial &>(inMaterial), *theClass, inProgram, applier);
}

void QDemonMaterialSystem::prepareTextureForRender(QDemonMaterialClass &inClass, QDemonRenderCustomMaterial &inMaterial)
{
    QDemonConstDataRef<dynamic::QDemonPropertyDefinition> thePropDefs = inClass.m_class->getProperties();
    for (qint32 idx = 0, end = thePropDefs.size(); idx < end; ++idx) {
        if (thePropDefs[idx].dataType == QDemonRenderShaderDataType::Texture2D) {
            if (thePropDefs[idx].texUsageType == QDemonRenderTextureTypeValue::Displace) {
                QDemonRenderImage *pImage = nullptr;

                // we only do this to not miss if "None" is selected
                QString theStrPtr = *reinterpret_cast<QString *>(inMaterial.getDataSectionBegin() + thePropDefs[idx].offset);

                if (!theStrPtr.isNull()) {

                    const qint32 index = findAllocatedImage(thePropDefs[idx].imagePath);
                    if (index == -1) {
                        pImage = new QDemonRenderImage();
                        allocatedImages.push_back(QPair<QString, QDemonRenderImage *>(thePropDefs[idx].imagePath, pImage));
                    } else
                        pImage = allocatedImages[index].second;

                    if (inMaterial.m_displacementMap != pImage) {
                        inMaterial.m_displacementMap = pImage;
                        inMaterial.m_displacementMap->m_imagePath = thePropDefs[idx].imagePath;
                        inMaterial.m_displacementMap->m_imageShaderName = thePropDefs[idx].name; // this is our name in the shader
                        inMaterial.m_displacementMap->m_verticalTilingMode = thePropDefs[idx].coordOp;
                        inMaterial.m_displacementMap->m_horizontalTilingMode = thePropDefs[idx].coordOp;
                    }
                } else {
                    inMaterial.m_displacementMap = nullptr;
                }
            } else if (thePropDefs[idx].texUsageType == QDemonRenderTextureTypeValue::Emissive2) {
                QDemonRenderImage *pImage = nullptr;

                // we only do this to not miss if "None" is selected
                QString theStrPtr = *reinterpret_cast<QString *>(inMaterial.getDataSectionBegin() + thePropDefs[idx].offset);

                if (!theStrPtr.isNull()) {
                    const qint32 index = findAllocatedImage(thePropDefs[idx].imagePath);
                    if (index == -1) {
                        pImage = new QDemonRenderImage();
                        allocatedImages.push_back(QPair<QString, QDemonRenderImage *>(thePropDefs[idx].imagePath, pImage));
                    } else
                        pImage = allocatedImages[index].second;

                    if (inMaterial.m_emissiveMap2 != pImage) {
                        inMaterial.m_emissiveMap2 = pImage;
                        inMaterial.m_emissiveMap2->m_imagePath = thePropDefs[idx].imagePath;
                        inMaterial.m_emissiveMap2->m_imageShaderName = thePropDefs[idx].name; // this is our name in the shader
                        inMaterial.m_emissiveMap2->m_verticalTilingMode = thePropDefs[idx].coordOp;
                        inMaterial.m_emissiveMap2->m_horizontalTilingMode = thePropDefs[idx].coordOp;
                    }
                } else {
                    inMaterial.m_emissiveMap2 = nullptr;
                }
            }
        }
    }
}

void QDemonMaterialSystem::prepareDisplacementForRender(QDemonMaterialClass &inClass, QDemonRenderCustomMaterial &inMaterial)
{
    if (inMaterial.m_displacementMap == nullptr)
        return;

    // our displacement mappin in MDL has fixed naming
    QDemonConstDataRef<dynamic::QDemonPropertyDefinition> thePropDefs = inClass.m_class->getProperties();
    for (qint32 idx = 0, end = thePropDefs.size(); idx < end; ++idx) {
        if (thePropDefs[idx].dataType == QDemonRenderShaderDataType::Float && (thePropDefs[idx].name == "displaceAmount")) {
            float theValue = *reinterpret_cast<const float *>(inMaterial.getDataSectionBegin() + thePropDefs[idx].offset);
            inMaterial.m_displaceAmount = theValue;
        } else if (thePropDefs[idx].dataType == QDemonRenderShaderDataType::Vec3 && (thePropDefs[idx].name == "displace_tiling")) {
            QVector3D theValue = *reinterpret_cast<const QVector3D *>(inMaterial.getDataSectionBegin()
                                                                      + thePropDefs[idx].offset);
            if (theValue.x() != inMaterial.m_displacementMap->m_scale.x()
                    || theValue.y() != inMaterial.m_displacementMap->m_scale.y()) {
                inMaterial.m_displacementMap->m_scale = QVector2D(theValue.x(), theValue.y());
                inMaterial.m_displacementMap->m_flags.setTransformDirty(true);
            }
        }
    }
}

void QDemonMaterialSystem::prepareMaterialForRender(QDemonMaterialClass &inClass, QDemonRenderCustomMaterial &inMaterial)
{
    prepareTextureForRender(inClass, inMaterial);

    if (inClass.m_hasDisplacement)
        prepareDisplacementForRender(inClass, inMaterial);
}

// Returns true if the material is dirty and thus will produce a different render result
// than previously.  This effects things like progressive AA.
// TODO - return more information, specifically about transparency (object is transparent,
// object is completely transparent
bool QDemonMaterialSystem::prepareForRender(const QDemonRenderModel &, const QDemonRenderSubset &, QDemonRenderCustomMaterial &inMaterial, bool clearMaterialDirtyFlags)
{
    QDemonMaterialClass *theMaterialClass = getMaterialClass(inMaterial.className);
    if (theMaterialClass == nullptr) {
        Q_ASSERT(false);
        return false;
    }

    prepareMaterialForRender(*theMaterialClass, inMaterial);

    inMaterial.m_hasTransparency = theMaterialClass->m_hasTransparency;
    inMaterial.m_hasRefraction = theMaterialClass->m_hasRefraction;
    inMaterial.m_hasVolumetricDF = false;

    bool wasDirty = inMaterial.isDirty() || theMaterialClass->m_alwaysDirty;
    if (clearMaterialDirtyFlags)
        inMaterial.updateDirtyForFrame();

    return wasDirty;
}

// TODO - handle UIC specific features such as vertex offsets for prog-aa and opacity.
void QDemonMaterialSystem::renderSubset(QDemonCustomMaterialRenderContext &inRenderContext, const TShaderFeatureSet &inFeatureSet)
{
    QDemonMaterialClass *theClass = getMaterialClass(inRenderContext.material.className);

    // Ensure that our overall render context comes back no matter what the client does.
    QDemonRenderContextScopedProperty<QDemonRenderBlendFunctionArgument> __blendFunction(*context->getRenderContext(),
                                                                                         &QDemonRenderContext::blendFunction,
                                                                                         &QDemonRenderContext::setBlendFunction,
                                                                                         QDemonRenderBlendFunctionArgument());
    QDemonRenderContextScopedProperty<QDemonRenderBlendEquationArgument> __blendEquation(*context->getRenderContext(),
                                                                                         &QDemonRenderContext::blendEquation,
                                                                                         &QDemonRenderContext::setBlendEquation,
                                                                                         QDemonRenderBlendEquationArgument());

    QDemonRenderContextScopedProperty<bool> theBlendEnabled(*context->getRenderContext(),
                                                            &QDemonRenderContext::isBlendingEnabled,
                                                            &QDemonRenderContext::setBlendingEnabled);

    doRenderCustomMaterial(inRenderContext, inRenderContext.material, *theClass, context->getRenderContext()->renderTarget(), inFeatureSet);
}

bool QDemonMaterialSystem::renderDepthPrepass(const QMatrix4x4 &inMVP, const QDemonRenderCustomMaterial &inMaterial, const QDemonRenderSubset &inSubset)
{
    QDemonMaterialClass *theClass = getMaterialClass(inMaterial.className);
    QDemonConstDataRef<dynamic::QDemonCommand *> theCommands = theClass->m_class->getRenderCommands();
    TShaderAndFlags thePrepassShader;
    for (qint32 idx = 0, end = theCommands.size(); idx < end && thePrepassShader.first == nullptr; ++idx) {
        const dynamic::QDemonCommand &theCommand = *theCommands[idx];
        if (theCommand.m_type == dynamic::CommandType::BindShader) {
            const dynamic::QDemonBindShader &theBindCommand = static_cast<const dynamic::QDemonBindShader &>(theCommand);
            thePrepassShader = context->getDynamicObjectSystem()->getDepthPrepassShader(theBindCommand.m_shaderPath,
                                                                                          QString(),
                                                                                          TShaderFeatureSet());
        }
    }

    if (thePrepassShader.first == nullptr)
        return false;

    QDemonRef<QDemonRenderContext> theContext = context->getRenderContext();
    QDemonRef<QDemonRenderShaderProgram> theProgram = thePrepassShader.first;
    theContext->setActiveShader(theProgram);
    theProgram->setPropertyValue("model_view_projection", inMVP);
    theContext->setInputAssembler(inSubset.inputAssemblerPoints);
    theContext->draw(QDemonRenderDrawMode::Lines, inSubset.posVertexBuffer->numVertexes(), 0);
    return true;
}

void QDemonMaterialSystem::onMaterialActivationChange(const QDemonRenderCustomMaterial &inMaterial, bool inActive)
{
    Q_UNUSED(inMaterial)
    Q_UNUSED(inActive)
}

void QDemonMaterialSystem::endFrame()
{
    quint64 currentFrameTime = QDemonTime::getCurrentTimeInTensOfNanoSeconds();
    if (lastFrameTime) {
        quint64 timePassed = currentFrameTime - lastFrameTime;
        msSinceLastFrame = static_cast<float>(timePassed / 100000.0);
    }
    lastFrameTime = currentFrameTime;
}

void QDemonMaterialSystem::setRenderContextInterface(QDemonRenderContextInterface *inContext)
{
    context = inContext;

    // check for fast blits
    QDemonRef<QDemonRenderContext> theContext = context->getRenderContext();
    useFastBlits = theContext->renderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::FastBlits);
}

QT_END_NAMESPACE
