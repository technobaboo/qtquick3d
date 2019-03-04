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
#include <QtDemonRender/qdemonrendercomputeshader.h>

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
#include <QtDemonRuntimeRender/qdemonrendercustommaterialshadergenerator.h>
#include <QtDemonRuntimeRender/qdemonrendermodel.h>

QT_BEGIN_NAMESPACE

QDemonCustomMaterialVertexPipeline::QDemonCustomMaterialVertexPipeline(QDemonRenderContextInterface *inContext, TessModeValues::Enum inTessMode)
    : QDemonVertexPipelineImpl(
          inContext->getCustomMaterialShaderGenerator(),
          inContext->getShaderProgramGenerator(),
          false)
    , m_context(inContext)
    , m_tessMode(TessModeValues::NoTess)
{
    if (m_context->getRenderContext()->isTessellationSupported()) {
        m_tessMode = inTessMode;
    }

    if (m_context->getRenderContext()->isGeometryStageSupported()
            && m_tessMode != TessModeValues::NoTess) {
        m_wireframe = inContext->getWireframeMode();
    }
}

void QDemonCustomMaterialVertexPipeline::initializeTessControlShader()
{
    if (m_tessMode == TessModeValues::NoTess
            || programGenerator()->getStage(ShaderGeneratorStages::TessControl) == nullptr) {
        return;
    }

    QDemonShaderStageGeneratorInterface &tessCtrlShader(*programGenerator()->getStage(ShaderGeneratorStages::TessControl));

    tessCtrlShader.addUniform(QStringLiteral("tessLevelInner"), QStringLiteral("float"));
    tessCtrlShader.addUniform(QStringLiteral("tessLevelOuter"), QStringLiteral("float"));

    setupTessIncludes(ShaderGeneratorStages::TessControl, m_tessMode);

    tessCtrlShader.append(QStringLiteral("void main() {\n"));

    tessCtrlShader.append(QStringLiteral("\tctWorldPos[0] = varWorldPos[0];"));
    tessCtrlShader.append(QStringLiteral("\tctWorldPos[1] = varWorldPos[1];"));
    tessCtrlShader.append(QStringLiteral("\tctWorldPos[2] = varWorldPos[2];"));

    if (m_tessMode == TessModeValues::TessPhong || m_tessMode == TessModeValues::TessNPatch) {
        tessCtrlShader.append(QStringLiteral("\tctNorm[0] = varObjectNormal[0];"));
        tessCtrlShader.append(QStringLiteral("\tctNorm[1] = varObjectNormal[1];"));
        tessCtrlShader.append(QStringLiteral("\tctNorm[2] = varObjectNormal[2];"));
    }
    if (m_tessMode == TessModeValues::TessNPatch) {
        tessCtrlShader.append(QStringLiteral("\tctTangent[0] = varObjTangent[0];"));
        tessCtrlShader.append(QStringLiteral("\tctTangent[1] = varObjTangent[1];"));
        tessCtrlShader.append(QStringLiteral("\tctTangent[2] = varObjTangent[2];"));
    }

    tessCtrlShader.append(
                QStringLiteral("\tgl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;"));
    tessCtrlShader.append(QStringLiteral("\ttessShader( tessLevelOuter, tessLevelInner);\n"));
}

void QDemonCustomMaterialVertexPipeline::initializeTessEvaluationShader()
{
    if (m_tessMode == TessModeValues::NoTess
            || programGenerator()->getStage(ShaderGeneratorStages::TessEval) == nullptr) {
        return;
    }

    QDemonShaderStageGeneratorInterface &tessEvalShader(
                *programGenerator()->getStage(ShaderGeneratorStages::TessEval));

    tessEvalShader.addUniform(QStringLiteral("model_view_projection"), QStringLiteral("mat4"));
    tessEvalShader.addUniform(QStringLiteral("normal_matrix"), QStringLiteral("mat3"));

    setupTessIncludes(ShaderGeneratorStages::TessEval, m_tessMode);

    if (m_tessMode == TessModeValues::TessLinear && m_displacementImage) {
        tessEvalShader.addInclude(QStringLiteral("defaultMaterialFileDisplacementTexture.glsllib"));
        tessEvalShader.addUniform(QStringLiteral("model_matrix"), QStringLiteral("mat4"));
        tessEvalShader.addUniform(QStringLiteral("displace_tiling"), QStringLiteral("vec3"));
        tessEvalShader.addUniform(QStringLiteral("displaceAmount"), QStringLiteral("float"));
        tessEvalShader.addUniform(m_displacementImage->m_image.m_imageShaderName, QStringLiteral("sampler2D"));
    }

    tessEvalShader.append(QStringLiteral("void main() {"));

    if (m_tessMode == TessModeValues::TessNPatch) {
        tessEvalShader.append(QStringLiteral("\tctNorm[0] = varObjectNormalTC[0];"));
        tessEvalShader.append(QStringLiteral("\tctNorm[1] = varObjectNormalTC[1];"));
        tessEvalShader.append(QStringLiteral("\tctNorm[2] = varObjectNormalTC[2];"));

        tessEvalShader.append(QStringLiteral("\tctTangent[0] = varTangentTC[0];"));
        tessEvalShader.append(QStringLiteral("\tctTangent[1] = varTangentTC[1];"));
        tessEvalShader.append(QStringLiteral("\tctTangent[2] = varTangentTC[2];"));
    }

    tessEvalShader.append(QStringLiteral("\tvec4 pos = tessShader( );\n"));
}

void QDemonCustomMaterialVertexPipeline::finalizeTessControlShader()
{
    QDemonShaderStageGeneratorInterface &tessCtrlShader(
                *programGenerator()->getStage(ShaderGeneratorStages::TessControl));
    // add varyings we must pass through
    typedef TStrTableStrMap::const_iterator TParamIter;
    for (TParamIter iter = m_interpolationParameters.begin(),
         end = m_interpolationParameters.end();
         iter != end; ++iter) {
        tessCtrlShader << QStringLiteral("\t")
                       << iter.key()
                       << QStringLiteral("TC[gl_InvocationID] = ")
                       << iter.key()
                       << QStringLiteral("[gl_InvocationID];\n");
    }
}

void QDemonCustomMaterialVertexPipeline::finalizeTessEvaluationShader()
{
    QDemonShaderStageGeneratorInterface &tessEvalShader(*programGenerator()->getStage(ShaderGeneratorStages::TessEval));

    QString outExt;
    if (programGenerator()->getEnabledStages() & ShaderGeneratorStages::Geometry)
        outExt = QStringLiteral("TE");

    // add varyings we must pass through
    typedef TStrTableStrMap::const_iterator TParamIter;
    if (m_tessMode == TessModeValues::TessNPatch) {
        for (TParamIter iter = m_interpolationParameters.begin(),
             end = m_interpolationParameters.end();
             iter != end; ++iter) {
            tessEvalShader << QStringLiteral("\t")
                           << iter.key()
                           << outExt
                           << QStringLiteral(" = gl_TessCoord.z * ")
                           << iter.key()
                           << QStringLiteral("TC[0] + ");
            tessEvalShader << QStringLiteral("gl_TessCoord.x * ")
                           << iter.key()
                           << QStringLiteral("TC[1] + ");
            tessEvalShader << QStringLiteral("gl_TessCoord.y * ")
                           << iter.key()
                           << QStringLiteral("TC[2];\n");
        }

        // transform the normal
        if (m_generationFlags & GenerationFlagValues::WorldNormal)
            tessEvalShader << QStringLiteral("\n\tvarNormal") << outExt
                           << QStringLiteral(" = normalize(normal_matrix * teNorm);\n");
        // transform the tangent
        if (m_generationFlags & GenerationFlagValues::TangentBinormal) {
            tessEvalShader << QStringLiteral("\n\tvarTangent") << outExt
                           << QStringLiteral(" = normalize(normal_matrix * teTangent);\n");
            // transform the binormal
            tessEvalShader << QStringLiteral("\n\tvarBinormal") << outExt
                           << QStringLiteral(" = normalize(normal_matrix * teBinormal);\n");
        }
    } else {
        for (TParamIter iter = m_interpolationParameters.begin(),
             end = m_interpolationParameters.end();
             iter != end; ++iter) {
            tessEvalShader << QStringLiteral("\t") << iter.key() << outExt
                           << QStringLiteral(" = gl_TessCoord.x * ") << iter.key() << QStringLiteral("TC[0] + ");
            tessEvalShader << QStringLiteral("gl_TessCoord.y * ") << iter.key() << QStringLiteral("TC[1] + ");
            tessEvalShader << QStringLiteral("gl_TessCoord.z * ") << iter.key() << QStringLiteral("TC[2];\n");
        }

        // displacement mapping makes only sense with linear tessellation
        if (m_tessMode == TessModeValues::TessLinear && m_displacementImage) {
            tessEvalShader
                    << "\ttexture_coordinate_info tmp = textureCoordinateInfo( varTexCoord0"
                    << outExt << ", varTangent" << outExt << ", varBinormal"
                    << outExt << " );" << QStringLiteral("\n");
            tessEvalShader << "\ttmp = transformCoordinate( rotationTranslationScale( vec3( "
                              "0.000000, 0.000000, 0.000000 ), vec3( 0.000000, 0.000000, "
                              "0.000000 ), displace_tiling ), tmp);"
                           << QStringLiteral("\n");

            tessEvalShader << "\tpos.xyz = defaultMaterialFileDisplacementTexture( "
                           << m_displacementImage->m_image.m_imageShaderName
                           << ", displaceAmount, "
                           << "tmp.position.xy";
            tessEvalShader << ", varObjectNormal" << outExt << ", pos.xyz );" << QStringLiteral("\n");
            tessEvalShader << "\tvarWorldPos" << outExt << "= (model_matrix * pos).xyz;"
                           << QStringLiteral("\n");
        }

        // transform the normal
        tessEvalShader << "\n\tvarNormal" << outExt
                       << " = normalize(normal_matrix * varObjectNormal" << outExt
                       << ");\n";
    }

    tessEvalShader.append("\tgl_Position = model_view_projection * pos;\n");
}

// Responsible for beginning all vertex and fragment generation (void main() { etc).
void QDemonCustomMaterialVertexPipeline::beginVertexGeneration(quint32 displacementImageIdx,
                                                          QDemonRenderableImage *displacementImage)
{
    m_displacementIdx = displacementImageIdx;
    m_displacementImage = displacementImage;

    TShaderGeneratorStageFlags theStages(QDemonShaderProgramGeneratorInterface::defaultFlags());

    if (m_tessMode != TessModeValues::NoTess) {
        theStages |= ShaderGeneratorStages::TessControl;
        theStages |= ShaderGeneratorStages::TessEval;
    }
    if (m_wireframe) {
        theStages |= ShaderGeneratorStages::Geometry;
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
    vertexShader << "void main()" << QStringLiteral("\n") << "{" << QStringLiteral("\n");

    if (displacementImage) {
        generateUVCoords(0);
        if (!hasTessellation()) {
            vertexShader.addUniform("displaceAmount", "float");
            vertexShader.addUniform("displace_tiling", "vec3");
            // we create the world position setup here
            // because it will be replaced with the displaced position
            setCode(GenerationFlagValues::WorldPosition);
            vertexShader.addUniform("model_matrix", "mat4");

            vertexShader.addInclude("defaultMaterialFileDisplacementTexture.glsllib");
            vertexShader.addUniform(displacementImage->m_image.m_imageShaderName,
                                    "sampler2D");

            vertexShader << "\ttexture_coordinate_info tmp = textureCoordinateInfo( texCoord0, "
                            "varTangent, varBinormal );"
                         << QStringLiteral("\n");
            vertexShader << "\ttmp = transformCoordinate( rotationTranslationScale( vec3( "
                            "0.000000, 0.000000, 0.000000 ), vec3( 0.000000, 0.000000, "
                            "0.000000 ), displace_tiling ), tmp);"
                         << QStringLiteral("\n");

            vertexShader << "\tvec3 displacedPos = defaultMaterialFileDisplacementTexture( "
                         << displacementImage->m_image.m_imageShaderName
                         << ", displaceAmount, "
                         << "tmp.position.xy"
                         << ", attr_norm, attr_pos );" << QStringLiteral("\n");

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
            vertexShader.append(
                        "\tgl_Position = model_view_projection * vec4(displacedPos, 1.0);");
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
    fragment() << "void main()" << QStringLiteral("\n") << "{" << QStringLiteral("\n");
}

void QDemonCustomMaterialVertexPipeline::assignOutput(const QString &inVarName,
                                                 const QString &inVarValue)
{
    vertex() << "\t" << inVarName << " = " << inVarValue << ";\n";
}

void QDemonCustomMaterialVertexPipeline::generateUVCoords(quint32 inUVSet)
{
    if (inUVSet == 0 && setCode(GenerationFlagValues::UVCoords))
        return;
    if (inUVSet == 1 && setCode(GenerationFlagValues::UVCoords1))
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
    if (setCode(GenerationFlagValues::WorldNormal))
        return;
    addInterpolationParameter("varNormal", "vec3");
    doGenerateWorldNormal();
}

void QDemonCustomMaterialVertexPipeline::generateObjectNormal()
{
    if (setCode(GenerationFlagValues::ObjectNormal))
        return;
    doGenerateObjectNormal();
}

void QDemonCustomMaterialVertexPipeline::generateVarTangentAndBinormal()
{
    if (setCode(GenerationFlagValues::TangentBinormal))
        return;
    addInterpolationParameter("varTangent", "vec3");
    addInterpolationParameter("varBinormal", "vec3");
    addInterpolationParameter("varObjTangent", "vec3");
    addInterpolationParameter("varObjBinormal", "vec3");
    doGenerateVarTangentAndBinormal();
}

void QDemonCustomMaterialVertexPipeline::generateWorldPosition()
{
    if (setCode(GenerationFlagValues::WorldPosition))
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

void QDemonCustomMaterialVertexPipeline::addInterpolationParameter(const QString &inName, const QString &inType)
{
    m_interpolationParameters.insert(inName, inType);
    vertex().addOutgoing(inName, inType);
    fragment().addIncoming(inName, inType);

    if (hasTessellation()) {
        QString nameBuilder(inName);
        nameBuilder.append("TC");
        tessControl().addOutgoing(nameBuilder, inType);

        nameBuilder = inName;
        if (programGenerator()->getEnabledStages() & ShaderGeneratorStages::Geometry) {
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
        vertex() << "\tvec3 texCoord0 = vec3( attr_uv0, 0.0 );" << QStringLiteral("\n");
        assignOutput("varTexCoord0", "texCoord0");
    } else if (inUVSet == 1) {
        vertex().addIncoming("attr_uv1", "vec2");
        vertex() << "\tvec3 texCoord1 = vec3( attr_uv1, 1.0 );" << QStringLiteral("\n");
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

    vertex() << "\tvarTangent = normal_matrix * attr_textan;" << QStringLiteral("\n")
             << "\tvarBinormal = normal_matrix * attr_binormal;" << QStringLiteral("\n");

    vertex() << "\tvarObjTangent = attr_textan;" << QStringLiteral("\n") << "\tvarObjBinormal = attr_binormal;"
             << QStringLiteral("\n");
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
    QDemonMaterialClass(QDemonDynamicObjectClassInterface &inCls)
        : m_class(&inCls)
    {
    }

    void afterWrite()
    {
        m_class = nullptr;
    }

    void afterRead(QDemonDynamicObjectClassInterface &inCls)
    {
        m_class = &inCls;
    }
};

typedef QHash<QString, QDemonRef<QDemonMaterialClass>> TStringMaterialMap;
typedef QPair<QString, QString> TStrStrPair;

struct QDemonShaderMapKey
{
    TStrStrPair m_name;
    QVector<QDemonShaderPreprocessorFeature> m_features;
    TessModeValues::Enum m_tessMode;
    bool m_wireframeMode;
    QDemonShaderDefaultMaterialKey m_materialKey;
    uint m_hashCode;
    QDemonShaderMapKey(const TStrStrPair &inName,
                       const TShaderFeatureSet &inFeatures,
                       TessModeValues::Enum inTessMode,
                       bool inWireframeMode,
                       QDemonShaderDefaultMaterialKey inMaterialKey)
        : m_name(inName)
        , m_features(inFeatures)
        , m_tessMode(inTessMode)
        , m_wireframeMode(inWireframeMode)
        , m_materialKey(inMaterialKey)
    {
        m_hashCode = qHash(m_name)
                ^ hashShaderFeatureSet(m_features)
                ^ qHash(m_tessMode) ^ qHash(m_wireframeMode)
                ^ qHash(inMaterialKey.hash());
    }
    bool operator==(const QDemonShaderMapKey &inKey) const
    {
        return m_name == inKey.m_name && m_features == inKey.m_features
                && m_tessMode == inKey.m_tessMode && m_wireframeMode == inKey.m_wireframeMode
                && m_materialKey == inKey.m_materialKey;
    }
};

uint qHash(const QDemonShaderMapKey &key)
{
    return key.m_hashCode;
}

namespace {

struct QDemonCustomMaterialTextureData
{
    QAtomicInt ref;
    QDemonRef<QDemonRenderShaderProgram> shader;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> sampler;
    QDemonRef<QDemonRenderTexture2D> texture;
    bool needsMips;

    QDemonCustomMaterialTextureData(const QDemonRef<QDemonRenderShaderProgram> &inShader,
                                    const QDemonRef<QDemonRenderTexture2D> &inTexture,
                                    const QString &inTexName,
                                    bool inNeedMips)
        : shader(inShader)
        , sampler(inTexName, inShader)
        , texture(inTexture)
        , needsMips(inNeedMips)
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

        if ((texture->getNumMipmaps() == 0) && needsMips)
            texture->generateMipmaps();

        sampler.set(texture.data());
    }

    static QDemonCustomMaterialTextureData createTextureEntry(const QDemonRef<QDemonRenderShaderProgram> &inShader,
                                                              const QDemonRef<QDemonRenderTexture2D> &inTexture,
                                                              const QString &inTexName,
                                                              bool needMips)
    {
        return QDemonCustomMaterialTextureData(inShader, inTexture, inTexName, needMips);
    }
};

typedef QPair<QString, QDemonRef<QDemonCustomMaterialTextureData>> TCustomMaterialTextureEntry;

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
    ///culling optimization in the tess shader

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

    QDemonCustomMaterialShader(const QDemonRef<QDemonRenderShaderProgram> &inShader,
                               dynamic::QDemonDynamicShaderProgramFlags inFlags)
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
        Q_ASSERT(inComputeShader->getProgramType() == QDemonRenderShaderProgram::ProgramType::Compute);
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
        : name(inName)
        , frameBuffer(inFb)
        , texture(inTexture)
        , flags(inFlags)
    {
    }
    QDemonCustomMaterialBuffer() = default;
};

struct QDemonMaterialSystem;
typedef QHash<QString, QDemonRef<QDemonRenderVertexBuffer>>
TStringVertexBufferMap;
typedef QHash<QString, QDemonRef<QDemonRenderInputAssembler>>
TStringAssemblerMap;

struct QDemonStringMemoryBarrierFlagMap
{
    const char *name;
    QDemonRenderBufferBarrierValues::Enum value;
    QDemonStringMemoryBarrierFlagMap(const char *nm,
                                     QDemonRenderBufferBarrierValues::Enum val)
        : name(nm)
        , value(val)
    {
    }
};

QDemonStringMemoryBarrierFlagMap g_StringMemoryFlagMap[] = {
    QDemonStringMemoryBarrierFlagMap("vertex_attribute",
    QDemonRenderBufferBarrierValues::VertexAttribArray),
    QDemonStringMemoryBarrierFlagMap("element_array",
    QDemonRenderBufferBarrierValues::ElementArray),
    QDemonStringMemoryBarrierFlagMap("uniform_buffer",
    QDemonRenderBufferBarrierValues::UniformBuffer),
    QDemonStringMemoryBarrierFlagMap("texture_fetch",
    QDemonRenderBufferBarrierValues::TextureFetch),
    QDemonStringMemoryBarrierFlagMap("shader_image_access",
    QDemonRenderBufferBarrierValues::ShaderImageAccess),
    QDemonStringMemoryBarrierFlagMap("command_buffer",
    QDemonRenderBufferBarrierValues::CommandBuffer),
    QDemonStringMemoryBarrierFlagMap("pixel_buffer",
    QDemonRenderBufferBarrierValues::PixelBuffer),
    QDemonStringMemoryBarrierFlagMap("texture_update",
    QDemonRenderBufferBarrierValues::TextureUpdate),
    QDemonStringMemoryBarrierFlagMap("buffer_update",
    QDemonRenderBufferBarrierValues::BufferUpdate),
    QDemonStringMemoryBarrierFlagMap("frame_buffer",
    QDemonRenderBufferBarrierValues::Framebuffer),
    QDemonStringMemoryBarrierFlagMap("transform_feedback",
    QDemonRenderBufferBarrierValues::TransformFeedback),
    QDemonStringMemoryBarrierFlagMap("atomic_counter",
    QDemonRenderBufferBarrierValues::AtomicCounter),
    QDemonStringMemoryBarrierFlagMap("shader_storage",
    QDemonRenderBufferBarrierValues::ShaderStorage),
};

struct QDemonStringBlendFuncMap
{
    const char *name;
    QDemonRenderSrcBlendFunc::Enum value;
    QDemonStringBlendFuncMap(const char *nm, QDemonRenderSrcBlendFunc::Enum val)
        : name(nm)
        , value(val)
    {
    }
};

QDemonStringBlendFuncMap g_BlendFuncMap[] = {
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

struct QDemonMaterialSystem : public QDemonCustomMaterialSystemInterface
{
    typedef QHash<QDemonShaderMapKey, QDemonRef<QDemonCustomMaterialShader>> TShaderMap;
    typedef QPair<QString, QDemonRenderImage *> TAllocatedImageEntry;

    QDemonRenderContextCoreInterface * m_coreContext;
    QDemonRenderContextInterface *m_context;
    TStringMaterialMap m_stringMaterialMap;
    TShaderMap m_shaderMap;
    QVector<TCustomMaterialTextureEntry> m_textureEntries;
    QVector<QDemonCustomMaterialBuffer> m_allocatedBuffers;
    QVector<TAllocatedImageEntry> m_allocatedImages;
    bool m_useFastBlits;
    QString m_shaderNameBuilder;
    quint64 m_lastFrameTime;
    float m_msSinceLastFrame;

    QDemonMaterialSystem(QDemonRenderContextCoreInterface * ct)
        : m_coreContext(ct)
        , m_context(nullptr)
        , m_useFastBlits(true)
        , m_lastFrameTime(0)
        , m_msSinceLastFrame(0)
    {
    }

    ~QDemonMaterialSystem()
    {
        while (m_allocatedBuffers.size())
        { // replace_with_last
            m_allocatedBuffers[0] = m_allocatedBuffers.back();
            m_allocatedBuffers.pop_back();
        }


        for (quint32 idx = 0; idx < m_allocatedImages.size(); ++idx) {
            QDemonRenderImage *pImage = m_allocatedImages[idx].second;
            ::free(pImage);
        }
        m_allocatedImages.clear();
    }

    void releaseBuffer(quint32 inIdx)
    {
        // Don't call this on MaterialSystem destroy.
        // This causes issues for scene liftime buffers
        // because the resource manager is destroyed before
        QDemonRef<QDemonResourceManagerInterface> theManager(m_context->getResourceManager());
        QDemonCustomMaterialBuffer &theEntry(m_allocatedBuffers[inIdx]);
        theEntry.frameBuffer->attach(QDemonRenderFrameBufferAttachments::Color0, QDemonRenderTextureOrRenderBuffer());

        theManager->release(theEntry.frameBuffer);
        theManager->release(theEntry.texture);
        { // replace_with_last
            m_allocatedBuffers[inIdx] = m_allocatedBuffers.back();
            m_allocatedBuffers.pop_back();
        }
    }

    bool isMaterialRegistered(const QString &inStr) override
    {
        return m_stringMaterialMap.find(inStr) != m_stringMaterialMap.end();
    }

    bool registerMaterialClass(const QString &inName,
                               const QDemonConstDataRef<dynamic::QDemonPropertyDeclaration> &inProperties) override
    {
        if (isMaterialRegistered(inName))
            return false;
        m_coreContext->getDynamicObjectSystemCore()->doRegister(inName, inProperties, sizeof(QDemonRenderCustomMaterial), QDemonGraphObjectTypes::CustomMaterial);
        QDemonDynamicObjectClassInterface *theClass =
                m_coreContext->getDynamicObjectSystemCore()->getDynamicObjectClass(inName);
        if (theClass == nullptr) {
            Q_ASSERT(false);
            return false;
        }
        QDemonRef<QDemonMaterialClass> theNewClass(new QDemonMaterialClass(*theClass));
        m_stringMaterialMap.insert(inName, theNewClass);
        return true;
    }

    QDemonMaterialClass *getMaterialClass(const QString &inStr)
    {
        auto theIter = m_stringMaterialMap.constFind(inStr);
        if (theIter != m_stringMaterialMap.constEnd())
            return theIter.value().data();
        return nullptr;
    }

    const QDemonMaterialClass *getMaterialClass(const QString &inStr) const
    {
        return const_cast<QDemonMaterialSystem *>(this)->getMaterialClass(inStr);
    }

    virtual QDemonConstDataRef<dynamic::QDemonPropertyDefinition> getCustomMaterialProperties(const QString &inCustomMaterialName) const override
    {
        QDemonDynamicObjectClassInterface *theMaterialClass =
                m_coreContext->getDynamicObjectSystemCore()->getDynamicObjectClass(inCustomMaterialName);

        if (theMaterialClass)
            return theMaterialClass->getProperties();

        return QDemonConstDataRef<dynamic::QDemonPropertyDefinition>();
    }

    virtual quint32 findBuffer(const QString &inName)
    {
        for (quint32 idx = 0, end = m_allocatedBuffers.size(); idx < end; ++idx)
            if (m_allocatedBuffers[idx].name == inName)
                return idx;
        return m_allocatedBuffers.size();
    }

    virtual quint32 findAllocatedImage(const QString &inName)
    {
        for (quint32 idx = 0, end = m_allocatedImages.size(); idx < end; ++idx)
            if (m_allocatedImages[idx].first == inName)
                return idx;
        return quint32(-1);
    }

    virtual bool textureNeedsMips(const dynamic::QDemonPropertyDefinition *inPropDec,
                                  QDemonRenderTexture2D *inTexture)
    {
        if (inPropDec && inTexture) {
            return bool((inPropDec->minFilterOp == QDemonRenderTextureMinifyingOp::LinearMipmapLinear)
                        && (inTexture->getNumMipmaps() == 0));
        }

        return false;
    }

    virtual void setTexture(const QDemonRef<QDemonRenderShaderProgram> &inShader,
                            const QString &inPropName,
                            const QDemonRef<QDemonRenderTexture2D> &inTexture,
                            const dynamic::QDemonPropertyDefinition *inPropDec = nullptr,
                            bool needMips = false)
    {
        QDemonRef<QDemonCustomMaterialTextureData> theTextureEntry;
        for (quint32 idx = 0, end = m_textureEntries.size(); idx < end && theTextureEntry == nullptr; ++idx) {
            if (m_textureEntries[idx].first == inPropName
                    && m_textureEntries[idx].second->shader == inShader
                    && m_textureEntries[idx].second->texture == inTexture) {
                theTextureEntry = m_textureEntries[idx].second;
                break;
            }
        }
        if (theTextureEntry == nullptr) {
            QDemonRef<QDemonCustomMaterialTextureData> theNewEntry(new QDemonCustomMaterialTextureData(QDemonCustomMaterialTextureData::createTextureEntry(inShader, inTexture, inPropName, needMips)));
            m_textureEntries.push_back(QPair<QString, QDemonRef<QDemonCustomMaterialTextureData>>(inPropName, theNewEntry));
            theTextureEntry = theNewEntry;
        }
        theTextureEntry->set(inPropDec);
    }

    void setPropertyEnumNames(const QString &inName,
                              const QString &inPropName,
                              const QDemonConstDataRef<QString> &inNames) override
    {
        m_coreContext->getDynamicObjectSystemCore()->setPropertyEnumNames(inName, inPropName,inNames);
    }

    void setPropertyTextureSettings(const QString &inName,
                                    const QString &inPropName,
                                    const QString &inPropPath,
                                    QDemonRenderTextureTypeValue::Enum inTexType,
                                    QDemonRenderTextureCoordOp::Enum inCoordOp,
                                    QDemonRenderTextureMagnifyingOp::Enum inMagFilterOp,
                                    QDemonRenderTextureMinifyingOp::Enum inMinFilterOp) override
    {
        QDemonMaterialClass *theClass = getMaterialClass(inName);
        if (theClass && inTexType == QDemonRenderTextureTypeValue::Displace) {
            theClass->m_hasDisplacement = true;
        }
        m_coreContext->getDynamicObjectSystemCore()->setPropertyTextureSettings(
                    inName, inPropName, inPropPath, inTexType, inCoordOp, inMagFilterOp, inMinFilterOp);
    }

    void setMaterialClassShader(QString inName, const char *inShaderType,
                                const char *inShaderVersion, const char *inShaderData,
                                bool inHasGeomShader, bool inIsComputeShader) override
    {
        m_coreContext->getDynamicObjectSystemCore()->setShaderData(inName, inShaderData, inShaderType,
                                                                   inShaderVersion, inHasGeomShader,
                                                                   inIsComputeShader);
    }

    QDemonRenderCustomMaterial *createCustomMaterial(const QString &inName) override
    {
        QDemonRenderCustomMaterial *theMaterial = static_cast<QDemonRenderCustomMaterial *>(m_coreContext->getDynamicObjectSystemCore()->createInstance(inName));
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

    void setCustomMaterialTransparency(const QString &inName, bool inHasTransparency) override
    {
        QDemonMaterialClass *theClass = getMaterialClass(inName);

        if (theClass == nullptr) {
            Q_ASSERT(false);
            return;
        }

        theClass->m_hasTransparency = inHasTransparency;
    }

    void setCustomMaterialRefraction(const QString &inName, bool inHasRefraction) override
    {
        QDemonMaterialClass *theClass = getMaterialClass(inName);

        if (theClass == nullptr) {
            Q_ASSERT(false);
            return;
        }

        theClass->m_hasRefraction = inHasRefraction;
    }

    void setCustomMaterialAlwaysDirty(const QString &inName, bool inIsAlwaysDirty) override
    {
        QDemonMaterialClass *theClass = getMaterialClass(inName);

        if (theClass == nullptr) {
            Q_ASSERT(false);
            return;
        }

        theClass->m_alwaysDirty = inIsAlwaysDirty;
    }

    void setCustomMaterialShaderKey(const QString &inName, quint32 inShaderKey) override
    {
        QDemonMaterialClass *theClass = getMaterialClass(inName);

        if (theClass == nullptr) {
            Q_ASSERT(false);
            return;
        }

        theClass->m_shaderKey = inShaderKey;
    }

    void setCustomMaterialLayerCount(const QString &inName, quint32 inLayerCount) override
    {
        QDemonMaterialClass *theClass = getMaterialClass(inName);

        if (theClass == nullptr) {
            Q_ASSERT(false);
            return;
        }

        theClass->m_layerCount = inLayerCount;
    }

    void setCustomMaterialCommands(QString inName,
                                   QDemonConstDataRef<dynamic::QDemonCommand *> inCommands) override
    {
        m_coreContext->getDynamicObjectSystemCore()->setRenderCommands(inName, inCommands);
    }

    QString getShaderCacheKey(QString &inShaderKeyBuffer,
                              const QString &inId,
                              const QString &inProgramMacro,
                              const dynamic::QDemonDynamicShaderProgramFlags &inFlags)
    {
        inShaderKeyBuffer = inId;
        if (!inProgramMacro.isNull() && !inProgramMacro.isNull()) {
            inShaderKeyBuffer.append("#");
            inShaderKeyBuffer.append(inProgramMacro);
        }
        if (inFlags.isTessellationEnabled()) {
            inShaderKeyBuffer.append("#");
            inShaderKeyBuffer.append(TessModeValues::toString(inFlags.tessMode));
        }
        if (inFlags.isGeometryShaderEnabled() && inFlags.wireframeMode) {
            inShaderKeyBuffer.append("#");
            inShaderKeyBuffer.append(dynamic::QDemonDynamicShaderProgramFlags::wireframeToString(inFlags.wireframeMode));
        }

        return inShaderKeyBuffer;
    }

    QDemonRef<QDemonRenderShaderProgram> getShader(QDemonCustomMaterialRenderContext &inRenderContext,
                                                   const QDemonRenderCustomMaterial &inMaterial,
                                                   const dynamic::QDemonBindShader &inCommand,
                                                   const TShaderFeatureSet &inFeatureSet,
                                                   const dynamic::QDemonDynamicShaderProgramFlags &inFlags)
    {
        QDemonRef<ICustomMaterialShaderGenerator> theMaterialGenerator(m_context->getCustomMaterialShaderGenerator());

        // generate key
        QString theShaderKeyBuffer;
        QString theKey = getShaderCacheKey(theShaderKeyBuffer, inCommand.m_shaderPath,
                                           inCommand.m_shaderDefine, inFlags);

        QDemonCustomMaterialVertexPipeline thePipeline(m_context, inRenderContext.model.tessellationMode);

        QDemonRef<QDemonRenderShaderProgram> theProgram = theMaterialGenerator->generateShader(
                    inMaterial, inRenderContext.materialKey, thePipeline, inFeatureSet,
                    inRenderContext.lights, inRenderContext.firstImage,
                    (inMaterial.m_hasTransparency || inMaterial.m_hasRefraction),
                    "custom material pipeline-- ", inCommand.m_shaderPath);

        return theProgram;
    }

    QDemonMaterialOrComputeShader bindShader(QDemonCustomMaterialRenderContext &inRenderContext,
                                             const QDemonRenderCustomMaterial &inMaterial,
                                             const dynamic::QDemonBindShader &inCommand,
                                             const TShaderFeatureSet &inFeatureSet)
    {
        QDemonRef<QDemonRenderShaderProgram> theProgram;

        dynamic::QDemonDynamicShaderProgramFlags theFlags(inRenderContext.model.tessellationMode,
                                                     inRenderContext.subset.wireframeMode);
        theFlags.setTessellationEnabled(inRenderContext.model.tessellationMode != TessModeValues::NoTess);
        theFlags.setGeometryShaderEnabled(inRenderContext.subset.wireframeMode);

        QDemonShaderMapKey skey = QDemonShaderMapKey(
                    TStrStrPair(inCommand.m_shaderPath, inCommand.m_shaderDefine), inFeatureSet,
                    theFlags.tessMode, theFlags.wireframeMode, inRenderContext.materialKey);
        auto theInsertResult = m_shaderMap.find(skey);
        //QPair<TShaderMap::iterator, bool> theInsertResult(m_ShaderMap.insert(skey, QDemonRef<SCustomMaterialShader>(nullptr)));

        if (theInsertResult == m_shaderMap.end()) {
            theProgram = getShader(inRenderContext, inMaterial, inCommand, inFeatureSet, theFlags);

            if (theProgram) {
                theInsertResult = m_shaderMap.insert(skey, QDemonRef<QDemonCustomMaterialShader>(new QDemonCustomMaterialShader(theProgram, theFlags)));
            }
        } else if (theInsertResult.value())
            theProgram = theInsertResult.value()->shader;

        if (theProgram) {
            if (theProgram->getProgramType() == QDemonRenderShaderProgram::ProgramType::Graphics) {
                if (theInsertResult.value()) {
                    QDemonRef<QDemonRenderContext> theContext(m_context->getRenderContext());
                    theContext->setActiveShader(theInsertResult.value()->shader);
                }

                return theInsertResult.value();
            } else {
                QDemonRef<QDemonRenderContext> theContext(m_context->getRenderContext());
                theContext->setActiveShader(theProgram);
                return theProgram;
            }
        }
        return QDemonMaterialOrComputeShader();
    }

    void doApplyInstanceValue(QDemonRenderCustomMaterial & /* inMaterial */,
                              quint8 *inDataPtr,
                              const QString &inPropertyName,
                              QDemonRenderShaderDataTypes::Enum inPropertyType,
                              const QDemonRef<QDemonRenderShaderProgram> &inShader,
                              const dynamic::QDemonPropertyDefinition &inDefinition)
    {
        QDemonRef<QDemonRenderShaderConstantBase> theConstant = inShader->getShaderConstant(inPropertyName.toLocal8Bit());
        if (theConstant) {
            if (theConstant->getShaderConstantType() == inPropertyType) {
                if (inPropertyType == QDemonRenderShaderDataTypes::Texture2D) {
                    //                    StaticAssert<sizeof(QString) == sizeof(QDemonRenderTexture2DPtr)>::valid_expression();
                    QString *theStrPtr = reinterpret_cast<QString *>(inDataPtr);
                    QDemonRef<QDemonBufferManagerInterface> theBufferManager(m_context->getBufferManager());
                    QDemonRef<QDemonRenderTexture2D> theTexture;

                    if (!theStrPtr->isNull()) {
                        QDemonRenderImageTextureData theTextureData = theBufferManager->loadRenderImage(*theStrPtr);
                        if (theTextureData.m_texture) {
                            theTexture = theTextureData.m_texture;
                            setTexture(inShader, inPropertyName, theTexture, &inDefinition,
                                       textureNeedsMips(&inDefinition, theTexture.data()));
                        }
                    }
                } else {
                    switch (inPropertyType) {
                    case QDemonRenderShaderDataTypes::Integer:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<qint32 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::IntegerVec2:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<qint32_2 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::IntegerVec3:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<qint32_3 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::IntegerVec4:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<qint32_4 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Boolean:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<bool *>(inDataPtr)));
                        break;                    case QDemonRenderShaderDataTypes::BooleanVec2:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<bool_2 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::BooleanVec3:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<bool_3 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::BooleanVec4:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<bool_4 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Float:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<float *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Vec2:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QVector2D *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Vec3:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QVector3D *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Vec4:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QVector4D *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::UnsignedInteger:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<quint32 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::UnsignedIntegerVec2:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<quint32_2 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::UnsignedIntegerVec3:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<quint32_3 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::UnsignedIntegerVec4:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<quint32_4 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Matrix3x3:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QMatrix3x3 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Matrix4x4:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QMatrix4x4 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Texture2D:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderTexture2DPtr *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Texture2DHandle:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderTexture2DHandle *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Texture2DArray:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderTexture2DArrayPtr *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::TextureCube:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderTextureCubePtr *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::TextureCubeHandle:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderTextureCubeHandle *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Image2D:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderImage2DPtr *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::DataBuffer:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderDataBufferPtr *>(inDataPtr)));
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

    void applyInstanceValue(QDemonRenderCustomMaterial &inMaterial,
                            QDemonMaterialClass &inClass,
                            const QDemonRef<QDemonRenderShaderProgram> &inShader,
                            const dynamic::QDemonApplyInstanceValue &inCommand)
    {
        // sanity check
        if (!inCommand.m_propertyName.isNull()) {
            bool canGetData =
                    inCommand.m_valueOffset + dynamic::getSizeofShaderDataType(inCommand.m_valueType)
                    <= inMaterial.dataSectionByteSize;
            if (canGetData == false) {
                Q_ASSERT(false);
                return;
            }
            quint8 *dataPtr = inMaterial.getDataSectionBegin() + inCommand.m_valueOffset;
            const dynamic::QDemonPropertyDefinition *theDefinition =
                    inClass.m_class->findPropertyByName(inCommand.m_propertyName);
            if (theDefinition)
                doApplyInstanceValue(inMaterial, dataPtr, inCommand.m_propertyName,
                                     inCommand.m_valueType, inShader, *theDefinition);
        } else {
            QDemonConstDataRef<dynamic::QDemonPropertyDefinition> theDefs = inClass.m_class->getProperties();
            for (quint32 idx = 0, end = theDefs.size(); idx < end; ++idx) {
                const dynamic::QDemonPropertyDefinition &theDefinition(theDefs[idx]);
                QDemonRef<QDemonRenderShaderConstantBase> theConstant = inShader->getShaderConstant(theDefinition.name.toLocal8Bit().constData());

                // This is fine, the property wasn't found and we continue, no problem.
                if (!theConstant)
                    continue;
                quint8 *dataPtr = inMaterial.getDataSectionBegin() + theDefinition.offset;
                doApplyInstanceValue(inMaterial, dataPtr, theDefinition.name, theDefinition.dataType, inShader, theDefinition);
            }
        }
    }

    void applyBlending(const dynamic::QDemonApplyBlending &inCommand)
    {
        QDemonRef<QDemonRenderContext> theContext(m_context->getRenderContext());

        theContext->setBlendingEnabled(true);

        QDemonRenderBlendFunctionArgument blendFunc =
                QDemonRenderBlendFunctionArgument(
                    inCommand.m_srcBlendFunc, inCommand.m_dstBlendFunc, inCommand.m_srcBlendFunc,
                    inCommand.m_dstBlendFunc);

        QDemonRenderBlendEquationArgument blendEqu(QDemonRenderBlendEquation::Add,
                                                   QDemonRenderBlendEquation::Add);

        theContext->setBlendFunction(blendFunc);
        theContext->setBlendEquation(blendEqu);
    }

    // we currently only bind a source texture
    const QDemonRef<QDemonRenderTexture2D> applyBufferValue(const QDemonRenderCustomMaterial &inMaterial,
                                                            const QDemonRef<QDemonRenderShaderProgram> &inShader,
                                                            const dynamic::QDemonApplyBufferValue &inCommand,
                                                            const QDemonRef<QDemonRenderTexture2D> inSourceTexture)
    {
        QDemonRef<QDemonRenderTexture2D> theTexture = nullptr;

        if (!inCommand.m_bufferName.isNull()) {
            quint32 bufferIdx = findBuffer(inCommand.m_bufferName);
            if (bufferIdx < m_allocatedBuffers.size()) {
                QDemonCustomMaterialBuffer &theEntry(m_allocatedBuffers[bufferIdx]);
                theTexture = theEntry.texture;
            } else {
                // we must have allocated the read target before
                qCCritical(INTERNAL_ERROR,
                           "CustomMaterial: ApplyBufferValue: Failed to setup read target");
                Q_ASSERT(false);
            }
        } else
            theTexture = inSourceTexture;

        if (!inCommand.m_paramName.isNull()) {
            QDemonRef<QDemonRenderShaderConstantBase> theConstant =
                    inShader->getShaderConstant(inCommand.m_paramName.toLocal8Bit().constData());

            if (theConstant) {
                if (theConstant->getShaderConstantType()
                        != QDemonRenderShaderDataTypes::Texture2D) {
                    qCCritical(INVALID_OPERATION,
                               "CustomMaterial %s: Binding buffer to parameter %s that is not a texture",
                               qPrintable(inMaterial.className), qPrintable(inCommand.m_paramName));
                    Q_ASSERT(false);
                } else {
                    setTexture(inShader, inCommand.m_paramName, theTexture);
                }
            }
        }

        return theTexture;
    }

    void allocateBuffer(const dynamic::QDemonAllocateBuffer &inCommand,
                        const QDemonRef<QDemonRenderFrameBuffer> &inTarget)
    {
        QDemonTextureDetails theSourceTextureDetails;
        QDemonRef<QDemonRenderTexture2D> theTexture;
        // get color attachment we always assume at location 0
        if (inTarget) {
            QDemonRenderTextureOrRenderBuffer theSourceTexture =
                    inTarget->getAttachment(QDemonRenderFrameBufferAttachments::Color0);
            // we need a texture
            if (theSourceTexture.hasTexture2D()) {
                theSourceTextureDetails = theSourceTexture.getTexture2D()->getTextureDetails();
            } else {
                qCCritical(INVALID_OPERATION, "CustomMaterial %s: Invalid source texture",
                           qPrintable(inCommand.m_name));
                Q_ASSERT(false);
                return;
            }
        } else {
            QDemonRef<QDemonRenderContext> theContext = m_context->getRenderContext();
            // if we allocate a buffer based on the default target use viewport to get the dimension
            QRect theViewport(theContext->getViewport());
            theSourceTextureDetails.height = theViewport.height();
            theSourceTextureDetails.width = theViewport.width();
        }

        quint32 theWidth = (quint32)(theSourceTextureDetails.width * inCommand.m_sizeMultiplier);
        quint32 theHeight = (quint32)(theSourceTextureDetails.height * inCommand.m_sizeMultiplier);
        QDemonRenderTextureFormats::Enum theFormat = inCommand.m_format;
        if (theFormat == QDemonRenderTextureFormats::Unknown)
            theFormat = theSourceTextureDetails.format;
        QDemonRef<QDemonResourceManagerInterface> theResourceManager(m_context->getResourceManager());
        // size intentionally requiried every loop;
        quint32 bufferIdx = findBuffer(inCommand.m_name);
        if (bufferIdx < m_allocatedBuffers.size()) {
            QDemonCustomMaterialBuffer &theEntry(m_allocatedBuffers[bufferIdx]);
            QDemonTextureDetails theDetails = theEntry.texture->getTextureDetails();
            if (theDetails.width == theWidth && theDetails.height == theHeight
                    && theDetails.format == theFormat) {
                theTexture = theEntry.texture;
            } else {
                releaseBuffer(bufferIdx);
            }
        }

        if (theTexture == nullptr) {
            QDemonRef<QDemonRenderFrameBuffer> theFB(theResourceManager->allocateFrameBuffer());
            QDemonRef<QDemonRenderTexture2D> theTexture(theResourceManager->allocateTexture2D(theWidth, theHeight, theFormat));
            theTexture->setMagFilter(inCommand.m_filterOp);
            theTexture->setMinFilter(
                        static_cast<QDemonRenderTextureMinifyingOp::Enum>(inCommand.m_filterOp));
            theTexture->setTextureWrapS(inCommand.m_texCoordOp);
            theTexture->setTextureWrapT(inCommand.m_texCoordOp);
            theFB->attach(QDemonRenderFrameBufferAttachments::Color0, theTexture);
            m_allocatedBuffers.push_back(QDemonCustomMaterialBuffer(inCommand.m_name, theFB, theTexture, inCommand.m_bufferFlags));
        }
    }

    QDemonRef<QDemonRenderFrameBuffer> bindBuffer(const QDemonRenderCustomMaterial &inMaterial, const dynamic::QDemonBindBuffer &inCommand,
                                                       bool &outClearTarget, QVector2D &outDestSize)
    {
        QDemonRef<QDemonRenderFrameBuffer> theBuffer;
        QDemonRef<QDemonRenderTexture2D> theTexture;

        // search for the buffer
        quint32 bufferIdx = findBuffer(inCommand.m_bufferName);
        if (bufferIdx < m_allocatedBuffers.size()) {
            theBuffer = m_allocatedBuffers[bufferIdx].frameBuffer;
            theTexture = m_allocatedBuffers[bufferIdx].texture;
        }

        if (theBuffer == nullptr) {
            qCCritical(INVALID_OPERATION, "Effect %s: Failed to find buffer %s for bind",
                       qPrintable(inMaterial.className), qPrintable(inCommand.m_bufferName));
            Q_ASSERT(false);
            return nullptr;
        }

        if (theTexture) {
            QDemonTextureDetails theDetails(theTexture->getTextureDetails());
            m_context->getRenderContext()->setViewport(QRect(0, 0, (quint32)theDetails.width, (quint32)theDetails.height));
            outDestSize = QVector2D((float)theDetails.width, (float)theDetails.height);
            outClearTarget = inCommand.m_needsClear;
        }

        return theBuffer;
    }

    void computeScreenCoverage(QDemonCustomMaterialRenderContext &inRenderContext, qint32 *xMin,
                               qint32 *yMin, qint32 *xMax, qint32 *yMax)
    {
        QDemonRef<QDemonRenderContext> theContext(m_context->getRenderContext());
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

        QRect theViewport(theContext->getViewport());
        qint32 x1 = qint32(projMax.x() * (theViewport.width() / 2)
                           + (theViewport.x() + (theViewport.width() / 2)));
        qint32 y1 = qint32(projMax.y() * (theViewport.height() / 2)
                           + (theViewport.y() + (theViewport.height() / 2)));

        qint32 x2 = qint32(projMin.x() * (theViewport.width() / 2)
                           + (theViewport.x() + (theViewport.width() / 2)));
        qint32 y2 = qint32(projMin.y() * (theViewport.height() / 2)
                           + (theViewport.y() + (theViewport.height() / 2)));

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

    void blitFramebuffer(QDemonCustomMaterialRenderContext &inRenderContext,
                         const dynamic::QDemonApplyBlitFramebuffer &inCommand,
                         const QDemonRef<QDemonRenderFrameBuffer> &inTarget)
    {
        QDemonRef<QDemonRenderContext> theContext(m_context->getRenderContext());
        // we change the read/render targets here
        QDemonRenderContextScopedProperty<QDemonRef<QDemonRenderFrameBuffer>> __framebuffer(
                    *theContext, &QDemonRenderContext::getRenderTarget, &QDemonRenderContext::setRenderTarget);
        // we may alter scissor
        QDemonRenderContextScopedProperty<bool> theScissorEnabled(
                    *theContext, &QDemonRenderContext::isScissorTestEnabled,
                    &QDemonRenderContext::setScissorTestEnabled);

        if (!inCommand.m_destBufferName.isNull()) {
            quint32 bufferIdx = findBuffer(inCommand.m_destBufferName);
            if (bufferIdx < m_allocatedBuffers.size()) {
                QDemonCustomMaterialBuffer &theEntry(m_allocatedBuffers[bufferIdx]);
                theContext->setRenderTarget(theEntry.frameBuffer);
            } else {
                // we must have allocated the read target before
                qCCritical(INTERNAL_ERROR,
                           "CustomMaterial: BlitFramebuffer: Failed to setup render target");
                Q_ASSERT(false);
            }
        } else {
            // our dest is the default render target
            theContext->setRenderTarget(inTarget);
        }

        if (!inCommand.m_sourceBufferName.isNull()) {
            quint32 bufferIdx = findBuffer(inCommand.m_sourceBufferName);
            if (bufferIdx < m_allocatedBuffers.size()) {
                QDemonCustomMaterialBuffer &theEntry(m_allocatedBuffers[bufferIdx]);
                theContext->setReadTarget(theEntry.frameBuffer);
                theContext->setReadBuffer(QDemonReadFaces::Color0);
            } else {
                // we must have allocated the read target before
                qCCritical(INTERNAL_ERROR,
                           "CustomMaterial: BlitFramebuffer: Failed to setup read target");
                Q_ASSERT(false);
            }
        } else {
            // our source is the default read target
            // depending on what we render we assume color0 or back
            theContext->setReadTarget(inTarget);
            QDemonReadFaces::Enum value = (inTarget) ? QDemonReadFaces::Color0 : QDemonReadFaces::Back;
            theContext->setReadBuffer(value);
        }

        QRect theViewport(theContext->getViewport());
        theContext->setScissorTestEnabled(false);

        if (!m_useFastBlits) {
            // only copy sreen amount of pixels
            qint32 xMin, yMin, xMax, yMax;
            computeScreenCoverage(inRenderContext, &xMin, &yMin, &xMax, &yMax);

            // same dimension
            theContext->blitFramebuffer(xMin, yMin, xMax, yMax, xMin, yMin, xMax, yMax,
                                        QDemonRenderClearValues::Color,
                                        QDemonRenderTextureMagnifyingOp::Nearest);
        } else {
            // same dimension
            theContext->blitFramebuffer(theViewport.x(), theViewport.y(), theViewport.x() + theViewport.width(),
                                        theViewport.y() + theViewport.height(), theViewport.x(), theViewport.y(),
                                        theViewport.x() + theViewport.width(), theViewport.y() + theViewport.height(),
                                        QDemonRenderClearValues::Color, QDemonRenderTextureMagnifyingOp::Nearest);
        }
    }

    QDemonLayerGlobalRenderProperties getLayerGlobalRenderProperties(QDemonCustomMaterialRenderContext &inRenderContext)
    {
        const QDemonRenderLayer &theLayer = inRenderContext.layer;
        const QDemonLayerRenderData &theData = inRenderContext.layerData;

        QVector<QVector3D> tempDirection;

        return QDemonLayerGlobalRenderProperties { theLayer,
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

    void renderPass(QDemonCustomMaterialRenderContext &inRenderContext,
                    const QDemonRef<QDemonCustomMaterialShader> &inShader,
                    const QDemonRef<QDemonRenderTexture2D> &/* inSourceTexture */,
                    const QDemonRef<QDemonRenderFrameBuffer> &inFrameBuffer,
                    bool inRenderTargetNeedsClear,
                    const QDemonRef<QDemonRenderInputAssembler> &inAssembler,
                    quint32 inCount,
                    quint32 inOffset)
    {
        QDemonRef<QDemonRenderContext> theContext(m_context->getRenderContext());
        theContext->setRenderTarget(inFrameBuffer);

        QVector4D clearColor(0.0, 0.0, 0.0, 0.0);
        QDemonRenderContextScopedProperty<QVector4D> __clearColor(
                    *theContext, &QDemonRenderContext::getClearColor, &QDemonRenderContext::setClearColor,
                    clearColor);
        if (inRenderTargetNeedsClear) {
            theContext->clear(QDemonRenderClearValues::Color);
        }

        QDemonRef<ICustomMaterialShaderGenerator> theMaterialGenerator(m_context->getCustomMaterialShaderGenerator());

        theMaterialGenerator->setMaterialProperties(
                    inShader->shader, inRenderContext.material, QVector2D(1.0, 1.0),
                    inRenderContext.modelViewProjection, inRenderContext.normalMatrix,
                    inRenderContext.modelMatrix, inRenderContext.firstImage, inRenderContext.opacity,
                    getLayerGlobalRenderProperties(inRenderContext));

        // I think the prim type should always be fetched from the
        // current mesh subset setup because there you get the actual draw mode
        // for this frame
        QDemonRenderDrawMode::Enum theDrawMode = inAssembler->getPrimitiveType();

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
            QRect theViewport(theContext->getViewport());
            QMatrix4x4 vpMatrix = {
                (float)theViewport.width() / 2.0f, 0.0, 0.0, 0.0,
                0.0, (float)theViewport.height() / 2.0f, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                (float)theViewport.width() / 2.0f + (float)theViewport.x(),
                (float)theViewport.height() / 2.0f + (float)theViewport.y(), 0.0, 1.0
            };

            inShader->viewportMatrix.set(vpMatrix);
        }

        theContext->setInputAssembler(inAssembler);

        theContext->setCullingEnabled(true);
        quint32 count = inCount;
        quint32 offset = inOffset;

        theContext->draw(theDrawMode, count, offset);
    }

    void doRenderCustomMaterial(QDemonCustomMaterialRenderContext &inRenderContext,
                                const QDemonRenderCustomMaterial &inMaterial,
                                QDemonMaterialClass &inClass,
                                const QDemonRef<QDemonRenderFrameBuffer> &inTarget,
                                const TShaderFeatureSet &inFeatureSet)
    {
        QDemonRef<QDemonRenderContext> theContext = m_context->getRenderContext();
        QDemonRef<QDemonCustomMaterialShader> theCurrentShader(nullptr);

        QDemonRef<QDemonRenderFrameBuffer> theCurrentRenderTarget(inTarget);
        QRect theOriginalViewport(theContext->getViewport());
        QDemonRef<QDemonRenderTexture2D> theCurrentSourceTexture;

        // for refrative materials we come from the transparent render path
        // but we do not want to do blending
        bool wasBlendingEnabled = theContext->isBlendingEnabled();
        if (inMaterial.m_hasRefraction)
            theContext->setBlendingEnabled(false);

        QDemonRenderContextScopedProperty<QDemonRef<QDemonRenderFrameBuffer>> __framebuffer(
                    *theContext, &QDemonRenderContext::getRenderTarget, &QDemonRenderContext::setRenderTarget);
        QDemonRenderContextScopedProperty<QRect> __viewport(
                    *theContext, &QDemonRenderContext::getViewport, &QDemonRenderContext::setViewport);

        QVector2D theDestSize;
        bool theRenderTargetNeedsClear = false;

        QDemonConstDataRef<dynamic::QDemonCommand *> theCommands(inClass.m_class->getRenderCommands());
        for (quint32 commandIdx = 0, commandEnd = theCommands.size(); commandIdx < commandEnd;
             ++commandIdx) {
            const dynamic::QDemonCommand &theCommand(*theCommands[commandIdx]);

            switch (theCommand.m_type) {
            case dynamic::CommandTypes::AllocateBuffer:
                allocateBuffer(static_cast<const dynamic::QDemonAllocateBuffer &>(theCommand), inTarget);
                break;
            case dynamic::CommandTypes::BindBuffer:
                theCurrentRenderTarget =
                        bindBuffer(inMaterial, static_cast<const dynamic::QDemonBindBuffer &>(theCommand),
                                   theRenderTargetNeedsClear, theDestSize);
                break;
            case dynamic::CommandTypes::BindTarget:
                // Restore the previous render target and info.
                theCurrentRenderTarget = inTarget;
                theContext->setViewport(theOriginalViewport);
                break;
            case dynamic::CommandTypes::BindShader: {
                theCurrentShader = nullptr;
                QDemonMaterialOrComputeShader theBindResult =
                        bindShader(inRenderContext, inMaterial,
                                   static_cast<const dynamic::QDemonBindShader &>(theCommand), inFeatureSet);
                if (theBindResult.isMaterialShader())
                    theCurrentShader = theBindResult.materialShader();
            } break;
            case dynamic::CommandTypes::ApplyInstanceValue:
                // we apply the property update explicitly at the render pass
                break;
            case dynamic::CommandTypes::Render:
                if (theCurrentShader) {
                    renderPass(inRenderContext, theCurrentShader, theCurrentSourceTexture,
                               theCurrentRenderTarget, theRenderTargetNeedsClear,
                               inRenderContext.subset.inputAssembler,
                               inRenderContext.subset.count, inRenderContext.subset.offset);
                }
                // reset
                theRenderTargetNeedsClear = false;
                break;
            case dynamic::CommandTypes::ApplyBlending:
                applyBlending(static_cast<const dynamic::QDemonApplyBlending &>(theCommand));
                break;
            case dynamic::CommandTypes::ApplyBufferValue:
                if (theCurrentShader)
                    applyBufferValue(inMaterial, theCurrentShader->shader,
                                     static_cast<const dynamic::QDemonApplyBufferValue &>(theCommand),
                                     theCurrentSourceTexture);
                break;
            case dynamic::CommandTypes::ApplyBlitFramebuffer:
                blitFramebuffer(inRenderContext,
                                static_cast<const dynamic::QDemonApplyBlitFramebuffer &>(theCommand), inTarget);
                break;
            default:
                Q_ASSERT(false);
                break;
            }
        }

        if (inMaterial.m_hasRefraction)
            theContext->setBlendingEnabled(wasBlendingEnabled);

        // Release any per-frame buffers
        for (quint32 idx = 0; idx < m_allocatedBuffers.size(); ++idx) {
            if (m_allocatedBuffers[idx].flags.isSceneLifetime() == false) {
                releaseBuffer(idx);
                --idx;
            }
        }
    }

    QString getShaderName(const QDemonRenderCustomMaterial &inMaterial) override
    {
        QDemonMaterialClass *theClass = getMaterialClass(inMaterial.className);
        if (!theClass)
            return QString();

        QDemonConstDataRef<dynamic::QDemonCommand *> theCommands = theClass->m_class->getRenderCommands();
        TShaderAndFlags thePrepassShader;
        for (quint32 idx = 0, end = theCommands.size();
             idx < end && thePrepassShader.first == nullptr; ++idx) {
            const dynamic::QDemonCommand &theCommand = *theCommands[idx];
            if (theCommand.m_type == dynamic::CommandTypes::BindShader) {
                const dynamic::QDemonBindShader &theBindCommand = static_cast<const dynamic::QDemonBindShader &>(theCommand);
                return theBindCommand.m_shaderPath;
            }
        }

        Q_UNREACHABLE();
        return QString();
    }

    void applyShaderPropertyValues(const QDemonRenderCustomMaterial &inMaterial,
                                   const QDemonRef<QDemonRenderShaderProgram> &inProgram) override
    {
        QDemonMaterialClass *theClass = getMaterialClass(inMaterial.className);
        if (!theClass)
            return;

        dynamic::QDemonApplyInstanceValue applier;
        applyInstanceValue(const_cast<QDemonRenderCustomMaterial &>(inMaterial), *theClass, inProgram,
                           applier);
    }

    virtual void prepareTextureForRender(QDemonMaterialClass &inClass, QDemonRenderCustomMaterial &inMaterial)
    {
        QDemonConstDataRef<dynamic::QDemonPropertyDefinition> thePropDefs = inClass.m_class->getProperties();
        for (quint32 idx = 0, end = thePropDefs.size(); idx < end; ++idx) {
            if (thePropDefs[idx].dataType == QDemonRenderShaderDataTypes::Texture2D) {
                if (thePropDefs[idx].texUsageType == QDemonRenderTextureTypeValue::Displace) {
                    QDemonRenderImage *pImage = nullptr;

                    // we only do this to not miss if "None" is selected
                    QString theStrPtr = *reinterpret_cast<QString *>(
                                inMaterial.getDataSectionBegin() + thePropDefs[idx].offset);

                    if (!theStrPtr.isNull()) {

                        quint32 index = findAllocatedImage(thePropDefs[idx].imagePath);
                        if (index == quint32(-1)) {
                            pImage = new QDemonRenderImage();
                            m_allocatedImages.push_back(
                                        QPair<QString, QDemonRenderImage *>(thePropDefs[idx].imagePath, pImage));
                        } else
                            pImage = m_allocatedImages[index].second;

                        if (inMaterial.m_displacementMap != pImage) {
                            inMaterial.m_displacementMap = pImage;
                            inMaterial.m_displacementMap->m_imagePath =
                                    thePropDefs[idx].imagePath;
                            inMaterial.m_displacementMap->m_imageShaderName =
                                    thePropDefs[idx].name; // this is our name in the shader
                            inMaterial.m_displacementMap->m_verticalTilingMode =
                                    thePropDefs[idx].coordOp;
                            inMaterial.m_displacementMap->m_horizontalTilingMode =
                                    thePropDefs[idx].coordOp;
                        }
                    } else {
                        inMaterial.m_displacementMap = nullptr;
                    }
                } else if (thePropDefs[idx].texUsageType == QDemonRenderTextureTypeValue::Emissive2) {
                    QDemonRenderImage *pImage = nullptr;

                    // we only do this to not miss if "None" is selected
                    QString theStrPtr = *reinterpret_cast<QString *>(
                                inMaterial.getDataSectionBegin() + thePropDefs[idx].offset);

                    if (!theStrPtr.isNull()) {
                        quint32 index = findAllocatedImage(thePropDefs[idx].imagePath);
                        if (index == quint32(-1)) {
                            pImage = new QDemonRenderImage();
                            m_allocatedImages.push_back(
                                        QPair<QString, QDemonRenderImage *>(thePropDefs[idx].imagePath, pImage));
                        } else
                            pImage = m_allocatedImages[index].second;

                        if (inMaterial.m_emissiveMap2 != pImage) {
                            inMaterial.m_emissiveMap2 = pImage;
                            inMaterial.m_emissiveMap2->m_imagePath = thePropDefs[idx].imagePath;
                            inMaterial.m_emissiveMap2->m_imageShaderName =
                                    thePropDefs[idx].name; // this is our name in the shader
                            inMaterial.m_emissiveMap2->m_verticalTilingMode =
                                    thePropDefs[idx].coordOp;
                            inMaterial.m_emissiveMap2->m_horizontalTilingMode =
                                    thePropDefs[idx].coordOp;
                        }
                    } else {
                        inMaterial.m_emissiveMap2 = nullptr;
                    }
                }
            }
        }
    }

    virtual void prepareDisplacementForRender(QDemonMaterialClass &inClass, QDemonRenderCustomMaterial &inMaterial)
    {
        if (inMaterial.m_displacementMap == nullptr)
            return;

        // our displacement mappin in MDL has fixed naming
        QDemonConstDataRef<dynamic::QDemonPropertyDefinition> thePropDefs = inClass.m_class->getProperties();
        for (quint32 idx = 0, end = thePropDefs.size(); idx < end; ++idx) {
            if (thePropDefs[idx].dataType == QDemonRenderShaderDataTypes::Float
                    && (thePropDefs[idx].name == QStringLiteral("displaceAmount"))) {
                float theValue = *reinterpret_cast<const float *>(inMaterial.getDataSectionBegin()
                                                                  + thePropDefs[idx].offset);
                inMaterial.m_displaceAmount = theValue;
            } else if (thePropDefs[idx].dataType == QDemonRenderShaderDataTypes::Vec3
                       && (thePropDefs[idx].name == QStringLiteral("displace_tiling"))) {
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

    void prepareMaterialForRender(QDemonMaterialClass &inClass, QDemonRenderCustomMaterial &inMaterial)
    {
        prepareTextureForRender(inClass, inMaterial);

        if (inClass.m_hasDisplacement)
            prepareDisplacementForRender(inClass, inMaterial);
    }

    // Returns true if the material is dirty and thus will produce a different render result
    // than previously.  This effects things like progressive AA.
    // TODO - return more information, specifically about transparency (object is transparent,
    // object is completely transparent
    bool prepareForRender(const QDemonRenderModel & /*inModel*/, const QDemonRenderSubset & /*inSubset*/,
                          QDemonRenderCustomMaterial &inMaterial, bool clearMaterialDirtyFlags) override
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
    void renderSubset(QDemonCustomMaterialRenderContext &inRenderContext,
                      const TShaderFeatureSet &inFeatureSet) override
    {
        QDemonMaterialClass *theClass = getMaterialClass(inRenderContext.material.className);

        // Ensure that our overall render context comes back no matter what the client does.
        QDemonRenderContextScopedProperty<QDemonRenderBlendFunctionArgument>
                __blendFunction(*m_context->getRenderContext(), &QDemonRenderContext::getBlendFunction,
                                &QDemonRenderContext::setBlendFunction,
                                QDemonRenderBlendFunctionArgument());
        QDemonRenderContextScopedProperty<QDemonRenderBlendEquationArgument>
                __blendEquation(*m_context->getRenderContext(), &QDemonRenderContext::getBlendEquation,
                                &QDemonRenderContext::setBlendEquation,
                                QDemonRenderBlendEquationArgument());

        QDemonRenderContextScopedProperty<bool> theBlendEnabled(*m_context->getRenderContext(),
                                                                &QDemonRenderContext::isBlendingEnabled,
                                                                &QDemonRenderContext::setBlendingEnabled);

        doRenderCustomMaterial(inRenderContext, inRenderContext.material, *theClass,
                               m_context->getRenderContext()->getRenderTarget(), inFeatureSet);
    }

    bool renderDepthPrepass(const QMatrix4x4 &inMVP, const QDemonRenderCustomMaterial &inMaterial, const QDemonRenderSubset &inSubset) override
    {
        QDemonMaterialClass *theClass = getMaterialClass(inMaterial.className);
        QDemonConstDataRef<dynamic::QDemonCommand *> theCommands = theClass->m_class->getRenderCommands();
        TShaderAndFlags thePrepassShader;
        for (quint32 idx = 0, end = theCommands.size();
             idx < end && thePrepassShader.first == nullptr; ++idx) {
            const dynamic::QDemonCommand &theCommand = *theCommands[idx];
            if (theCommand.m_type == dynamic::CommandTypes::BindShader) {
                const dynamic::QDemonBindShader &theBindCommand = static_cast<const dynamic::QDemonBindShader &>(theCommand);
                thePrepassShader = m_context->getDynamicObjectSystem()->getDepthPrepassShader(
                            theBindCommand.m_shaderPath, QString(), TShaderFeatureSet());
            }
        }

        if (thePrepassShader.first == nullptr)
            return false;

        QDemonRef<QDemonRenderContext> theContext = m_context->getRenderContext();
        QDemonRef<QDemonRenderShaderProgram> theProgram = thePrepassShader.first;
        theContext->setActiveShader(theProgram);
        theProgram->setPropertyValue("model_view_projection", inMVP);
        theContext->setInputAssembler(inSubset.inputAssemblerPoints);
        theContext->draw(QDemonRenderDrawMode::Lines, inSubset.posVertexBuffer->getNumVertexes(), 0);
        return true;
    }

    void onMaterialActivationChange(const QDemonRenderCustomMaterial &inMaterial, bool inActive) override
    {
        Q_UNUSED(inMaterial)
        Q_UNUSED(inActive)
    }

    void endFrame() override
    {
        quint64 currentFrameTime = QDemonTime::getCurrentTimeInTensOfNanoSeconds();
        if (m_lastFrameTime) {
            quint64 timePassed = currentFrameTime - m_lastFrameTime;
            m_msSinceLastFrame = static_cast<float>(timePassed / 100000.0);
        }
        m_lastFrameTime = currentFrameTime;
    }

    //    void Save(SWriteBuffer &ioBuffer,
    //                      const SStrRemapMap &inRemapMap,
    //                      const char * /*inProjectDir*/) const override
    //    {
    //        quint32 offset = ioBuffer.size();
    //        ioBuffer.write((quint32)m_StringMaterialMap.size());
    //        for (TStringMaterialMap::const_iterator iter = m_StringMaterialMap.begin(),
    //                                                end = m_StringMaterialMap.end();
    //             iter != end; ++iter) {
    //            size_t nameOffset = ioBuffer.size() - offset;
    //            (void)nameOffset;
    //            QString materialName(iter->first);
    //            materialName.Remap(inRemapMap);
    //            ioBuffer.write(materialName);
    //            const SMaterialClass *materialClass = iter->second.mPtr;
    //            quint32 offset = ioBuffer.size();
    //            ioBuffer.write(*materialClass);
    //            quint8 *materialOffset = ioBuffer.begin() + offset;
    //            SMaterialClass *writtenClass = (SMaterialClass *)materialOffset;
    //            writtenClass->AfterWrite();
    //            ioBuffer.align(4);
    //        }
    //    }

    //    void Load(QDemonDataRef<quint8> inData, CStrTableOrDataRef inStrDataBlock,
    //                      const char * /*inProjectDir*/) override
    //    {
    //        m_Allocator.m_PreAllocatedBlock = inData;
    //        m_Allocator.m_OwnsMemory = false;
    //        SDataReader theReader(inData.begin(), inData.end());
    //        quint32 numMaterialClasses = theReader.LoadRef<quint32>();
    //        for (quint32 idx = 0; idx < numMaterialClasses; ++idx) {
    //            QString clsName = theReader.LoadRef<QString>();
    //            clsName.Remap(inStrDataBlock);
    //            IDynamicObjectClass *theDynamicCls =
    //                m_CoreContext.GetDynamicObjectSystemCore().GetDynamicObjectClass(clsName);
    //            SMaterialClass *theReadClass = theReader.Load<SMaterialClass>();
    //            theReader.Align(4);
    //            if (theDynamicCls) {
    //                theReadClass->AfterRead(m_Allocator, *theDynamicCls);
    //                m_StringMaterialMap.insert(clsName, theReadClass);
    //            }
    //        }
    //    }

    QDemonRef<QDemonCustomMaterialSystemInterface> getCustomMaterialSystem(QDemonRenderContextInterface *inContext) override
    {
        m_context = inContext;

        // check for fast blits
        QDemonRef<QDemonRenderContext> theContext = m_context->getRenderContext();
        m_useFastBlits = theContext->getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::FastBlits);

        return this;
    }
};
}

QDemonRef<QDemonCustomMaterialSystemInterface> QDemonCustomMaterialSystemInterface::createCustomMaterialSystem(QDemonRenderContextCoreInterface * ctx)
{
    return QDemonRef<QDemonMaterialSystem>(new QDemonMaterialSystem(ctx));
}

QT_END_NAMESPACE

