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
//#include <QtDemonRuntimeRender/qdemonrendererimpllayerrenderdata.h>
#include <QtDemonRuntimeRender/qdemonrendercustommaterialshadergenerator.h>
#include <QtDemonRuntimeRender/qdemonrendermodel.h>

#include <QtCore/QEnableSharedFromThis>


QT_BEGIN_NAMESPACE

SCustomMaterialVertexPipeline::SCustomMaterialVertexPipeline(QSharedPointer<IQDemonRenderContext> inContext, TessModeValues::Enum inTessMode)
    : SVertexPipelineImpl(
          inContext->GetCustomMaterialShaderGenerator(),
          inContext->GetShaderProgramGenerator(),
          false)
    , m_Context(inContext)
    , m_TessMode(TessModeValues::NoTess)
{
    if (m_Context->GetRenderContext()->IsTessellationSupported()) {
        m_TessMode = inTessMode;
    }

    if (m_Context->GetRenderContext()->IsGeometryStageSupported()
            && m_TessMode != TessModeValues::NoTess) {
        m_Wireframe = inContext->GetWireframeMode();
    }
}

void SCustomMaterialVertexPipeline::InitializeTessControlShader()
{
    if (m_TessMode == TessModeValues::NoTess
            || ProgramGenerator()->GetStage(ShaderGeneratorStages::TessControl) == nullptr) {
        return;
    }

    IShaderStageGenerator &tessCtrlShader(*ProgramGenerator()->GetStage(ShaderGeneratorStages::TessControl));

    tessCtrlShader.AddUniform(QStringLiteral("tessLevelInner"), QStringLiteral("float"));
    tessCtrlShader.AddUniform(QStringLiteral("tessLevelOuter"), QStringLiteral("float"));

    SetupTessIncludes(ShaderGeneratorStages::TessControl, m_TessMode);

    tessCtrlShader.Append(QStringLiteral("void main() {\n"));

    tessCtrlShader.Append(QStringLiteral("\tctWorldPos[0] = varWorldPos[0];"));
    tessCtrlShader.Append(QStringLiteral("\tctWorldPos[1] = varWorldPos[1];"));
    tessCtrlShader.Append(QStringLiteral("\tctWorldPos[2] = varWorldPos[2];"));

    if (m_TessMode == TessModeValues::TessPhong || m_TessMode == TessModeValues::TessNPatch) {
        tessCtrlShader.Append(QStringLiteral("\tctNorm[0] = varObjectNormal[0];"));
        tessCtrlShader.Append(QStringLiteral("\tctNorm[1] = varObjectNormal[1];"));
        tessCtrlShader.Append(QStringLiteral("\tctNorm[2] = varObjectNormal[2];"));
    }
    if (m_TessMode == TessModeValues::TessNPatch) {
        tessCtrlShader.Append(QStringLiteral("\tctTangent[0] = varObjTangent[0];"));
        tessCtrlShader.Append(QStringLiteral("\tctTangent[1] = varObjTangent[1];"));
        tessCtrlShader.Append(QStringLiteral("\tctTangent[2] = varObjTangent[2];"));
    }

    tessCtrlShader.Append(
                QStringLiteral("\tgl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;"));
    tessCtrlShader.Append(QStringLiteral("\ttessShader( tessLevelOuter, tessLevelInner);\n"));
}

void SCustomMaterialVertexPipeline::InitializeTessEvaluationShader()
{
    if (m_TessMode == TessModeValues::NoTess
            || ProgramGenerator()->GetStage(ShaderGeneratorStages::TessEval) == nullptr) {
        return;
    }

    IShaderStageGenerator &tessEvalShader(
                *ProgramGenerator()->GetStage(ShaderGeneratorStages::TessEval));

    tessEvalShader.AddUniform(QStringLiteral("model_view_projection"), QStringLiteral("mat4"));
    tessEvalShader.AddUniform(QStringLiteral("normal_matrix"), QStringLiteral("mat3"));

    SetupTessIncludes(ShaderGeneratorStages::TessEval, m_TessMode);

    if (m_TessMode == TessModeValues::TessLinear && m_DisplacementImage) {
        tessEvalShader.AddInclude(QStringLiteral("defaultMaterialFileDisplacementTexture.glsllib"));
        tessEvalShader.AddUniform(QStringLiteral("model_matrix"), QStringLiteral("mat4"));
        tessEvalShader.AddUniform(QStringLiteral("displace_tiling"), QStringLiteral("vec3"));
        tessEvalShader.AddUniform(QStringLiteral("displaceAmount"), QStringLiteral("float"));
        tessEvalShader.AddUniform(m_DisplacementImage->m_Image.m_ImageShaderName, QStringLiteral("sampler2D"));
    }

    tessEvalShader.Append(QStringLiteral("void main() {"));

    if (m_TessMode == TessModeValues::TessNPatch) {
        tessEvalShader.Append(QStringLiteral("\tctNorm[0] = varObjectNormalTC[0];"));
        tessEvalShader.Append(QStringLiteral("\tctNorm[1] = varObjectNormalTC[1];"));
        tessEvalShader.Append(QStringLiteral("\tctNorm[2] = varObjectNormalTC[2];"));

        tessEvalShader.Append(QStringLiteral("\tctTangent[0] = varTangentTC[0];"));
        tessEvalShader.Append(QStringLiteral("\tctTangent[1] = varTangentTC[1];"));
        tessEvalShader.Append(QStringLiteral("\tctTangent[2] = varTangentTC[2];"));
    }

    tessEvalShader.Append(QStringLiteral("\tvec4 pos = tessShader( );\n"));
}

void SCustomMaterialVertexPipeline::FinalizeTessControlShader()
{
    IShaderStageGenerator &tessCtrlShader(
                *ProgramGenerator()->GetStage(ShaderGeneratorStages::TessControl));
    // add varyings we must pass through
    typedef TStrTableStrMap::const_iterator TParamIter;
    for (TParamIter iter = m_InterpolationParameters.begin(),
         end = m_InterpolationParameters.end();
         iter != end; ++iter) {
        tessCtrlShader << QStringLiteral("\t")
                       << iter.key()
                       << QStringLiteral("TC[gl_InvocationID] = ")
                       << iter.key()
                       << QStringLiteral("[gl_InvocationID];\n");
    }
}

void SCustomMaterialVertexPipeline::FinalizeTessEvaluationShader()
{
    IShaderStageGenerator &tessEvalShader(*ProgramGenerator()->GetStage(ShaderGeneratorStages::TessEval));

    QString outExt;
    if (ProgramGenerator()->GetEnabledStages() & ShaderGeneratorStages::Geometry)
        outExt = QStringLiteral("TE");

    // add varyings we must pass through
    typedef TStrTableStrMap::const_iterator TParamIter;
    if (m_TessMode == TessModeValues::TessNPatch) {
        for (TParamIter iter = m_InterpolationParameters.begin(),
             end = m_InterpolationParameters.end();
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
        if (m_GenerationFlags & GenerationFlagValues::WorldNormal)
            tessEvalShader << QStringLiteral("\n\tvarNormal") << outExt
                           << QStringLiteral(" = normalize(normal_matrix * teNorm);\n");
        // transform the tangent
        if (m_GenerationFlags & GenerationFlagValues::TangentBinormal) {
            tessEvalShader << QStringLiteral("\n\tvarTangent") << outExt
                           << QStringLiteral(" = normalize(normal_matrix * teTangent);\n");
            // transform the binormal
            tessEvalShader << QStringLiteral("\n\tvarBinormal") << outExt
                           << QStringLiteral(" = normalize(normal_matrix * teBinormal);\n");
        }
    } else {
        for (TParamIter iter = m_InterpolationParameters.begin(),
             end = m_InterpolationParameters.end();
             iter != end; ++iter) {
            tessEvalShader << QStringLiteral("\t") << iter.key() << outExt
                           << QStringLiteral(" = gl_TessCoord.x * ") << iter.key() << QStringLiteral("TC[0] + ");
            tessEvalShader << QStringLiteral("gl_TessCoord.y * ") << iter.key() << QStringLiteral("TC[1] + ");
            tessEvalShader << QStringLiteral("gl_TessCoord.z * ") << iter.key() << QStringLiteral("TC[2];\n");
        }

        // displacement mapping makes only sense with linear tessellation
        if (m_TessMode == TessModeValues::TessLinear && m_DisplacementImage) {
            tessEvalShader
                    << "\ttexture_coordinate_info tmp = textureCoordinateInfo( varTexCoord0"
                    << outExt << ", varTangent" << outExt << ", varBinormal"
                    << outExt << " );" << QStringLiteral("\n");
            tessEvalShader << "\ttmp = transformCoordinate( rotationTranslationScale( vec3( "
                              "0.000000, 0.000000, 0.000000 ), vec3( 0.000000, 0.000000, "
                              "0.000000 ), displace_tiling ), tmp);"
                           << QStringLiteral("\n");

            tessEvalShader << "\tpos.xyz = defaultMaterialFileDisplacementTexture( "
                           << m_DisplacementImage->m_Image.m_ImageShaderName
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

    tessEvalShader.Append("\tgl_Position = model_view_projection * pos;\n");
}

// Responsible for beginning all vertex and fragment generation (void main() { etc).
void SCustomMaterialVertexPipeline::BeginVertexGeneration(quint32 displacementImageIdx,
                                                          SRenderableImage *displacementImage)
{
    m_DisplacementIdx = displacementImageIdx;
    m_DisplacementImage = displacementImage;

    TShaderGeneratorStageFlags theStages(IShaderProgramGenerator::DefaultFlags());

    if (m_TessMode != TessModeValues::NoTess) {
        theStages |= ShaderGeneratorStages::TessControl;
        theStages |= ShaderGeneratorStages::TessEval;
    }
    if (m_Wireframe) {
        theStages |= ShaderGeneratorStages::Geometry;
    }

    ProgramGenerator()->BeginProgram(theStages);

    if (m_TessMode != TessModeValues::NoTess) {
        InitializeTessControlShader();
        InitializeTessEvaluationShader();
    }
    if (m_Wireframe) {
        InitializeWireframeGeometryShader();
    }

    IShaderStageGenerator &vertexShader(Vertex());

    // thinks we need
    vertexShader.AddInclude("viewProperties.glsllib");
    vertexShader.AddInclude("customMaterial.glsllib");

    vertexShader.AddIncoming("attr_pos", "vec3");
    vertexShader << "void main()" << QStringLiteral("\n") << "{" << QStringLiteral("\n");

    if (displacementImage) {
        GenerateUVCoords(0);
        if (!HasTessellation()) {
            vertexShader.AddUniform("displaceAmount", "float");
            vertexShader.AddUniform("displace_tiling", "vec3");
            // we create the world position setup here
            // because it will be replaced with the displaced position
            SetCode(GenerationFlagValues::WorldPosition);
            vertexShader.AddUniform("model_matrix", "mat4");

            vertexShader.AddInclude("defaultMaterialFileDisplacementTexture.glsllib");
            vertexShader.AddUniform(displacementImage->m_Image.m_ImageShaderName,
                                    "sampler2D");

            vertexShader << "\ttexture_coordinate_info tmp = textureCoordinateInfo( texCoord0, "
                            "varTangent, varBinormal );"
                         << QStringLiteral("\n");
            vertexShader << "\ttmp = transformCoordinate( rotationTranslationScale( vec3( "
                            "0.000000, 0.000000, 0.000000 ), vec3( 0.000000, 0.000000, "
                            "0.000000 ), displace_tiling ), tmp);"
                         << QStringLiteral("\n");

            vertexShader << "\tvec3 displacedPos = defaultMaterialFileDisplacementTexture( "
                         << displacementImage->m_Image.m_ImageShaderName
                         << ", displaceAmount, "
                         << "tmp.position.xy"
                         << ", attr_norm, attr_pos );" << QStringLiteral("\n");

            AddInterpolationParameter("varWorldPos", "vec3");
            vertexShader.Append("\tvec3 local_model_world_position = (model_matrix * "
                                "vec4(displacedPos, 1.0)).xyz;");
            AssignOutput("varWorldPos", "local_model_world_position");
        }
    }

    if (HasTessellation()) {
        vertexShader.Append("\tgl_Position = vec4(attr_pos, 1.0);");
    } else {
        vertexShader.AddUniform("model_view_projection", "mat4");
        if (displacementImage)
            vertexShader.Append(
                        "\tgl_Position = model_view_projection * vec4(displacedPos, 1.0);");
        else
            vertexShader.Append("\tgl_Position = model_view_projection * vec4(attr_pos, 1.0);");
    }

    if (HasTessellation()) {
        GenerateWorldPosition();
        GenerateWorldNormal();
        GenerateObjectNormal();
        GenerateVarTangentAndBinormal();
    }
}

void SCustomMaterialVertexPipeline::BeginFragmentGeneration()
{
    Fragment().AddUniform("object_opacity", "float");
    Fragment() << "void main()" << QStringLiteral("\n") << "{" << QStringLiteral("\n");
}

void SCustomMaterialVertexPipeline::AssignOutput(const QString &inVarName,
                                                 const QString &inVarValue)
{
    Vertex() << "\t" << inVarName << " = " << inVarValue << ";\n";
}

void SCustomMaterialVertexPipeline::GenerateUVCoords(quint32 inUVSet)
{
    if (inUVSet == 0 && SetCode(GenerationFlagValues::UVCoords))
        return;
    if (inUVSet == 1 && SetCode(GenerationFlagValues::UVCoords1))
        return;

    Q_ASSERT(inUVSet == 0 || inUVSet == 1);

    if (inUVSet == 0)
        AddInterpolationParameter("varTexCoord0", "vec3");
    else if (inUVSet == 1)
        AddInterpolationParameter("varTexCoord1", "vec3");

    DoGenerateUVCoords(inUVSet);
}

void SCustomMaterialVertexPipeline::GenerateWorldNormal()
{
    if (SetCode(GenerationFlagValues::WorldNormal))
        return;
    AddInterpolationParameter("varNormal", "vec3");
    DoGenerateWorldNormal();
}

void SCustomMaterialVertexPipeline::GenerateObjectNormal()
{
    if (SetCode(GenerationFlagValues::ObjectNormal))
        return;
    DoGenerateObjectNormal();
}

void SCustomMaterialVertexPipeline::GenerateVarTangentAndBinormal()
{
    if (SetCode(GenerationFlagValues::TangentBinormal))
        return;
    AddInterpolationParameter("varTangent", "vec3");
    AddInterpolationParameter("varBinormal", "vec3");
    AddInterpolationParameter("varObjTangent", "vec3");
    AddInterpolationParameter("varObjBinormal", "vec3");
    DoGenerateVarTangentAndBinormal();
}

void SCustomMaterialVertexPipeline::GenerateWorldPosition()
{
    if (SetCode(GenerationFlagValues::WorldPosition))
        return;

    ActiveStage().AddUniform("model_matrix", "mat4");
    AddInterpolationParameter("varWorldPos", "vec3");
    AddInterpolationParameter("varObjPos", "vec3");
    DoGenerateWorldPosition();
}

// responsible for closing all vertex and fragment generation
void SCustomMaterialVertexPipeline::EndVertexGeneration()
{
    if (HasTessellation()) {
        // finalize tess control shader
        FinalizeTessControlShader();
        // finalize tess evaluation shader
        FinalizeTessEvaluationShader();

        TessControl().Append("}");
        TessEval().Append("}");

        if (m_Wireframe) {
            // finalize geometry shader
            FinalizeWireframeGeometryShader();
            Geometry().Append("}");
        }
    }

    Vertex().Append("}");
}

void SCustomMaterialVertexPipeline::EndFragmentGeneration()
{
    Fragment().Append("}");
}

IShaderStageGenerator &SCustomMaterialVertexPipeline::ActiveStage()
{
    return Vertex();
}

void SCustomMaterialVertexPipeline::AddInterpolationParameter(const QString &inName, const QString &inType)
{
    m_InterpolationParameters.insert(inName, inType);
    Vertex().AddOutgoing(inName, inType);
    Fragment().AddIncoming(inName, inType);

    if (HasTessellation()) {
        QString nameBuilder(inName);
        nameBuilder.append("TC");
        TessControl().AddOutgoing(nameBuilder, inType);

        nameBuilder = inName;
        if (ProgramGenerator()->GetEnabledStages() & ShaderGeneratorStages::Geometry) {
            nameBuilder.append("TE");
            Geometry().AddOutgoing(inName, inType);
        }
        TessEval().AddOutgoing(nameBuilder, inType);
    }
}

void SCustomMaterialVertexPipeline::DoGenerateUVCoords(quint32 inUVSet)
{
    Q_ASSERT(inUVSet == 0 || inUVSet == 1);

    if (inUVSet == 0) {
        Vertex().AddIncoming("attr_uv0", "vec2");
        Vertex() << "\tvec3 texCoord0 = vec3( attr_uv0, 0.0 );" << QStringLiteral("\n");
        AssignOutput("varTexCoord0", "texCoord0");
    } else if (inUVSet == 1) {
        Vertex().AddIncoming("attr_uv1", "vec2");
        Vertex() << "\tvec3 texCoord1 = vec3( attr_uv1, 1.0 );" << QStringLiteral("\n");
        AssignOutput("varTexCoord1", "texCoord1");
    }
}

void SCustomMaterialVertexPipeline::DoGenerateWorldNormal()
{
    IShaderStageGenerator &vertexGenerator(Vertex());
    vertexGenerator.AddIncoming("attr_norm", "vec3");
    vertexGenerator.AddUniform("normal_matrix", "mat3");

    if (HasTessellation() == false) {
        Vertex().Append("\tvarNormal = normalize( normal_matrix * attr_norm );");
    }
}

void SCustomMaterialVertexPipeline::DoGenerateObjectNormal()
{
    AddInterpolationParameter("varObjectNormal", "vec3");
    Vertex().Append("\tvarObjectNormal = attr_norm;");
}

void SCustomMaterialVertexPipeline::DoGenerateWorldPosition()
{
    Vertex().Append("\tvarObjPos = attr_pos;");
    Vertex().Append("\tvec4 worldPos = (model_matrix * vec4(attr_pos, 1.0));");
    AssignOutput("varWorldPos", "worldPos.xyz");
}

void SCustomMaterialVertexPipeline::DoGenerateVarTangentAndBinormal()
{
    Vertex().AddIncoming("attr_textan", "vec3");
    Vertex().AddIncoming("attr_binormal", "vec3");

    Vertex() << "\tvarTangent = normal_matrix * attr_textan;" << QStringLiteral("\n")
             << "\tvarBinormal = normal_matrix * attr_binormal;" << QStringLiteral("\n");

    Vertex() << "\tvarObjTangent = attr_textan;" << QStringLiteral("\n") << "\tvarObjBinormal = attr_binormal;"
             << QStringLiteral("\n");
}

void SCustomMaterialVertexPipeline::DoGenerateVertexColor()
{
    Vertex().AddIncoming("attr_color", "vec3");
    Vertex().Append("\tvarColor = attr_color;");
}


struct SMaterialClass
{
    IDynamicObjectClass *m_Class;
    bool m_HasTransparency;
    bool m_HasRefraction;
    bool m_HasDisplacement;
    bool m_AlwaysDirty;
    quint32 m_ShaderKey;
    quint32 m_LayerCount;
    SMaterialClass(IDynamicObjectClass &inCls)
        : m_Class(&inCls)
        , m_HasTransparency(false)
        , m_HasRefraction(false)
        , m_HasDisplacement(false)
        , m_AlwaysDirty(false)
        , m_ShaderKey(0)
        , m_LayerCount(0)
    {
    }

    ~SMaterialClass() {}

    void AfterWrite()
    {
        m_Class = nullptr;
    }

    void AfterRead(IDynamicObjectClass &inCls)
    {
        m_Class = &inCls;
    }
};

typedef QHash<QString, QSharedPointer<SMaterialClass>> TStringMaterialMap;
typedef QPair<QString, QString> TStrStrPair;

struct SShaderMapKey
{
    TStrStrPair m_Name;
    QVector<SShaderPreprocessorFeature> m_Features;
    TessModeValues::Enum m_TessMode;
    bool m_WireframeMode;
    SShaderDefaultMaterialKey m_MaterialKey;
    uint m_HashCode;
    SShaderMapKey(TStrStrPair inName, TShaderFeatureSet inFeatures, TessModeValues::Enum inTessMode,
                  bool inWireframeMode, SShaderDefaultMaterialKey inMaterialKey)
        : m_Name(inName)
        , m_Features(inFeatures)
        , m_TessMode(inTessMode)
        , m_WireframeMode(inWireframeMode)
        , m_MaterialKey(inMaterialKey)
    {
        m_HashCode = qHash(m_Name)
                ^ HashShaderFeatureSet(m_Features)
                ^ qHash(m_TessMode) ^ qHash(m_WireframeMode)
                ^ qHash(inMaterialKey.hash());
    }
    bool operator==(const SShaderMapKey &inKey) const
    {
        return m_Name == inKey.m_Name && m_Features == inKey.m_Features
                && m_TessMode == inKey.m_TessMode && m_WireframeMode == inKey.m_WireframeMode
                && m_MaterialKey == inKey.m_MaterialKey;
    }
};

uint qHash(const SShaderMapKey &key)
{
    return key.m_HashCode;
}

namespace {

struct SCustomMaterialTextureData
{
    QSharedPointer<QDemonRenderShaderProgram> m_Shader;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> m_Sampler;
    QSharedPointer<QDemonRenderTexture2D> m_Texture;
    bool m_needsMips;

    SCustomMaterialTextureData(QSharedPointer<QDemonRenderShaderProgram> inShader,
                               QSharedPointer<QDemonRenderTexture2D> inTexture,
                               const QString &inTexName,
                               bool needMips)
        : m_Shader(inShader)
        , m_Sampler(inTexName, inShader)
        , m_Texture(inTexture)
        , m_needsMips(needMips)
    {
    }

    void Set(const dynamic::SPropertyDefinition *inDefinition)
    {
        if (m_Texture && inDefinition) {
            m_Texture->SetMagFilter(inDefinition->m_MagFilterOp);
            m_Texture->SetMinFilter(inDefinition->m_MinFilterOp);
            m_Texture->SetTextureWrapS(inDefinition->m_CoordOp);
            m_Texture->SetTextureWrapT(inDefinition->m_CoordOp);
        } else if (m_Texture) {
            // set some defaults
            m_Texture->SetMinFilter(QDemonRenderTextureMinifyingOp::Linear);
            m_Texture->SetTextureWrapS(QDemonRenderTextureCoordOp::ClampToEdge);
            m_Texture->SetTextureWrapT(QDemonRenderTextureCoordOp::ClampToEdge);
        }

        if ((m_Texture->GetNumMipmaps() == 0) && m_needsMips)
            m_Texture->GenerateMipmaps();

        m_Sampler.Set(m_Texture.data());
    }

    static SCustomMaterialTextureData CreateTextureEntry(QSharedPointer<QDemonRenderShaderProgram> inShader,
                                                         QSharedPointer<QDemonRenderTexture2D> inTexture,
                                                         const QString &inTexName,
                                                         bool needMips)
    {
        return SCustomMaterialTextureData(inShader, inTexture, inTexName, needMips);
    }
};

typedef QPair<QString, QSharedPointer<SCustomMaterialTextureData>> TCustomMaterialTextureEntry;

/**
 *	Cached tessellation property lookups this is on a per mesh base
 */
struct SCustomMaterialsTessellationProperties
{
    QDemonRenderCachedShaderProperty<float> m_EdgeTessLevel; ///< tesselation value for the edges
    QDemonRenderCachedShaderProperty<float> m_InsideTessLevel; ///< tesselation value for the inside
    QDemonRenderCachedShaderProperty<float> m_PhongBlend; ///< blending between linear and phong component
    QDemonRenderCachedShaderProperty<QVector2D> m_DistanceRange; ///< distance range for min and max tess level
    QDemonRenderCachedShaderProperty<float> m_DisableCulling; ///< if set to 1.0 this disables backface
    ///culling optimization in the tess shader

    SCustomMaterialsTessellationProperties() {}
    SCustomMaterialsTessellationProperties(QSharedPointer<QDemonRenderShaderProgram> inShader)
        : m_EdgeTessLevel("tessLevelOuter", inShader)
        , m_InsideTessLevel("tessLevelInner", inShader)
        , m_PhongBlend("phongBlend", inShader)
        , m_DistanceRange("distanceRange", inShader)
        , m_DisableCulling("disableCulling", inShader)
    {
    }
};

/* We setup some shared state on the custom material shaders */
struct SCustomMaterialShader
{
    QSharedPointer<QDemonRenderShaderProgram> m_Shader;
    QDemonRenderCachedShaderProperty<QMatrix4x4> m_ModelMatrix;
    QDemonRenderCachedShaderProperty<QMatrix4x4> m_ViewProjMatrix;
    QDemonRenderCachedShaderProperty<QMatrix4x4> m_ViewMatrix;
    QDemonRenderCachedShaderProperty<QMatrix3x3> m_NormalMatrix;
    QDemonRenderCachedShaderProperty<QVector3D> m_CameraPos;
    QDemonRenderCachedShaderProperty<QMatrix4x4> m_ProjMatrix;
    QDemonRenderCachedShaderProperty<QMatrix4x4> m_ViewportMatrix;
    QDemonRenderCachedShaderProperty<QVector2D> m_CamProperties;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> m_DepthTexture;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> m_AOTexture;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> m_LightProbe;
    QDemonRenderCachedShaderProperty<QVector4D> m_LightProbeProps;
    QDemonRenderCachedShaderProperty<QVector4D> m_LightProbeOpts;
    QDemonRenderCachedShaderProperty<QVector4D> m_LightProbeRot;
    QDemonRenderCachedShaderProperty<QVector4D> m_LightProbeOfs;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> m_LightProbe2;
    QDemonRenderCachedShaderProperty<QVector4D> m_LightProbe2Props;
    QDemonRenderCachedShaderProperty<qint32> m_LightCount;
    QDemonRenderCachedShaderProperty<qint32> m_AreaLightCount;
    QDemonRenderCachedShaderBuffer<QDemonRenderShaderConstantBuffer> m_AoShadowParams;
    SCustomMaterialsTessellationProperties m_Tessellation;
    dynamic::SDynamicShaderProgramFlags m_ProgramFlags;

    SCustomMaterialShader(QSharedPointer<QDemonRenderShaderProgram> inShader, dynamic::SDynamicShaderProgramFlags inFlags)
        : m_Shader(inShader)
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
        , m_AoShadowParams("cbAoShadow", inShader)
        , m_Tessellation(inShader)
        , m_ProgramFlags(inFlags)
    {
    }
};

struct SMaterialOrComputeShader
{
    QSharedPointer<SCustomMaterialShader> m_MaterialShader;
    QSharedPointer<QDemonRenderShaderProgram> m_ComputeShader;
    SMaterialOrComputeShader()
        : m_MaterialShader(nullptr)
        , m_ComputeShader(nullptr)
    {
    }
    SMaterialOrComputeShader(QSharedPointer<SCustomMaterialShader> inMaterialShader)
        : m_MaterialShader(inMaterialShader)
        , m_ComputeShader(nullptr)
    {
    }
    SMaterialOrComputeShader(QSharedPointer<QDemonRenderShaderProgram> inComputeShader)
        : m_MaterialShader(nullptr)
        , m_ComputeShader(inComputeShader)
    {
        Q_ASSERT(inComputeShader->GetProgramType() == QDemonRenderShaderProgram::ProgramType::Compute);
    }
    bool IsValid() const { return m_MaterialShader || m_ComputeShader; }
    bool IsComputeShader() const { return m_ComputeShader != nullptr; }
    bool IsMaterialShader() const { return m_MaterialShader != nullptr; }
    QSharedPointer<SCustomMaterialShader> MaterialShader()
    {
        Q_ASSERT(IsMaterialShader());
        return m_MaterialShader;
    }
    QSharedPointer<QDemonRenderShaderProgram> ComputeShader()
    {
        Q_ASSERT(IsComputeShader());
        return m_ComputeShader;
    }
};

struct SCustomMaterialBuffer
{
    QString m_Name;
    QSharedPointer<QDemonRenderFrameBuffer> m_FrameBuffer;
    QSharedPointer<QDemonRenderTexture2D> m_Texture;
    dynamic::SAllocateBufferFlags m_Flags;

    SCustomMaterialBuffer(QString inName,
                          QSharedPointer<QDemonRenderFrameBuffer> inFb,
                          QSharedPointer<QDemonRenderTexture2D> inTexture,
                          dynamic::SAllocateBufferFlags inFlags)
        : m_Name(inName)
        , m_FrameBuffer(inFb)
        , m_Texture(inTexture)
        , m_Flags(inFlags)
    {
    }
    SCustomMaterialBuffer() {}
};

struct SMaterialSystem;
typedef QHash<QString, QSharedPointer<QDemonRenderVertexBuffer>>
TStringVertexBufferMap;
typedef QHash<QString, QSharedPointer<QDemonRenderInputAssembler>>
TStringAssemblerMap;

struct SStringMemoryBarrierFlagMap
{
    const char *m_Name;
    QDemonRenderBufferBarrierValues::Enum m_Value;
    SStringMemoryBarrierFlagMap(const char *nm,
                                QDemonRenderBufferBarrierValues::Enum val)
        : m_Name(nm)
        , m_Value(val)
    {
    }
};

SStringMemoryBarrierFlagMap g_StringMemoryFlagMap[] = {
    SStringMemoryBarrierFlagMap("vertex_attribute",
    QDemonRenderBufferBarrierValues::VertexAttribArray),
    SStringMemoryBarrierFlagMap("element_array",
    QDemonRenderBufferBarrierValues::ElementArray),
    SStringMemoryBarrierFlagMap("uniform_buffer",
    QDemonRenderBufferBarrierValues::UniformBuffer),
    SStringMemoryBarrierFlagMap("texture_fetch",
    QDemonRenderBufferBarrierValues::TextureFetch),
    SStringMemoryBarrierFlagMap("shader_image_access",
    QDemonRenderBufferBarrierValues::ShaderImageAccess),
    SStringMemoryBarrierFlagMap("command_buffer",
    QDemonRenderBufferBarrierValues::CommandBuffer),
    SStringMemoryBarrierFlagMap("pixel_buffer",
    QDemonRenderBufferBarrierValues::PixelBuffer),
    SStringMemoryBarrierFlagMap("texture_update",
    QDemonRenderBufferBarrierValues::TextureUpdate),
    SStringMemoryBarrierFlagMap("buffer_update",
    QDemonRenderBufferBarrierValues::BufferUpdate),
    SStringMemoryBarrierFlagMap("frame_buffer",
    QDemonRenderBufferBarrierValues::Framebuffer),
    SStringMemoryBarrierFlagMap("transform_feedback",
    QDemonRenderBufferBarrierValues::TransformFeedback),
    SStringMemoryBarrierFlagMap("atomic_counter",
    QDemonRenderBufferBarrierValues::AtomicCounter),
    SStringMemoryBarrierFlagMap("shader_storage",
    QDemonRenderBufferBarrierValues::ShaderStorage),
};

struct SStringBlendFuncMap
{
    const char *m_Name;
    QDemonRenderSrcBlendFunc::Enum m_Value;
    SStringBlendFuncMap(const char *nm, QDemonRenderSrcBlendFunc::Enum val)
        : m_Name(nm)
        , m_Value(val)
    {
    }
};

SStringBlendFuncMap g_BlendFuncMap[] = {
    SStringBlendFuncMap("Unknown", QDemonRenderSrcBlendFunc::Unknown),
    SStringBlendFuncMap("Zero", QDemonRenderSrcBlendFunc::Zero),
    SStringBlendFuncMap("One", QDemonRenderSrcBlendFunc::One),
    SStringBlendFuncMap("SrcColor", QDemonRenderSrcBlendFunc::SrcColor),
    SStringBlendFuncMap("OneMinusSrcColor", QDemonRenderSrcBlendFunc::OneMinusSrcColor),
    SStringBlendFuncMap("DstColor", QDemonRenderSrcBlendFunc::DstColor),
    SStringBlendFuncMap("OneMinusDstColor", QDemonRenderSrcBlendFunc::OneMinusDstColor),
    SStringBlendFuncMap("SrcAlpha", QDemonRenderSrcBlendFunc::SrcAlpha),
    SStringBlendFuncMap("OneMinusSrcAlpha", QDemonRenderSrcBlendFunc::OneMinusSrcAlpha),
    SStringBlendFuncMap("DstAlpha", QDemonRenderSrcBlendFunc::DstAlpha),
    SStringBlendFuncMap("OneMinusDstAlpha", QDemonRenderSrcBlendFunc::OneMinusDstAlpha),
    SStringBlendFuncMap("ConstantColor", QDemonRenderSrcBlendFunc::ConstantColor),
    SStringBlendFuncMap("OneMinusConstantColor", QDemonRenderSrcBlendFunc::OneMinusConstantColor),
    SStringBlendFuncMap("ConstantAlpha", QDemonRenderSrcBlendFunc::ConstantAlpha),
    SStringBlendFuncMap("OneMinusConstantAlpha", QDemonRenderSrcBlendFunc::OneMinusConstantAlpha),
    SStringBlendFuncMap("SrcAlphaSaturate", QDemonRenderSrcBlendFunc::SrcAlphaSaturate)
};

struct SMaterialSystem : public ICustomMaterialSystem, public QEnableSharedFromThis<SMaterialSystem>
{
    typedef QHash<SShaderMapKey, QSharedPointer<SCustomMaterialShader>> TShaderMap;
    typedef QPair<QString, SImage *> TAllocatedImageEntry;

    QSharedPointer<IQDemonRenderContextCore> m_CoreContext;
    QSharedPointer<IQDemonRenderContext> m_Context;
    TStringMaterialMap m_StringMaterialMap;
    TShaderMap m_ShaderMap;
    QVector<TCustomMaterialTextureEntry> m_TextureEntries;
    QVector<SCustomMaterialBuffer> m_AllocatedBuffers;
    QVector<TAllocatedImageEntry> m_AllocatedImages;
    bool m_UseFastBlits;
    QString m_ShaderNameBuilder;
    quint64 m_LastFrameTime;
    float m_MillisecondsSinceLastFrame;

    SMaterialSystem(QSharedPointer<IQDemonRenderContextCore> ct)
        : m_CoreContext(ct)
        , m_Context(nullptr)
        , m_UseFastBlits(true)
        , m_LastFrameTime(0)
        , m_MillisecondsSinceLastFrame(0)
    {
    }

    ~SMaterialSystem()
    {
        while (m_AllocatedBuffers.size())
        { // replace_with_last
            m_AllocatedBuffers[0] = m_AllocatedBuffers.back();
            m_AllocatedBuffers.pop_back();
        }


        for (quint32 idx = 0; idx < m_AllocatedImages.size(); ++idx) {
            SImage *pImage = m_AllocatedImages[idx].second;
            ::free(pImage);
        }
        m_AllocatedImages.clear();
    }

    void ReleaseBuffer(quint32 inIdx)
    {
        // Don't call this on MaterialSystem destroy.
        // This causes issues for scene liftime buffers
        // because the resource manager is destroyed before
        QSharedPointer<IResourceManager> theManager(m_Context->GetResourceManager());
        SCustomMaterialBuffer &theEntry(m_AllocatedBuffers[inIdx]);
        theEntry.m_FrameBuffer->Attach(QDemonRenderFrameBufferAttachments::Color0, QDemonRenderTextureOrRenderBuffer());

        theManager->Release(theEntry.m_FrameBuffer);
        theManager->Release(theEntry.m_Texture);
        { // replace_with_last
            m_AllocatedBuffers[inIdx] = m_AllocatedBuffers.back();
            m_AllocatedBuffers.pop_back();
        }
    }

    bool IsMaterialRegistered(QString inStr) override
    {
        return m_StringMaterialMap.find(inStr) != m_StringMaterialMap.end();
    }

    bool RegisterMaterialClass(QString inName,
                               QDemonConstDataRef<dynamic::SPropertyDeclaration> inProperties) override
    {
        if (IsMaterialRegistered(inName))
            return false;
        m_CoreContext->GetDynamicObjectSystemCore()->Register(inName, inProperties, sizeof(SCustomMaterial), GraphObjectTypes::CustomMaterial);
        IDynamicObjectClass *theClass =
                m_CoreContext->GetDynamicObjectSystemCore()->GetDynamicObjectClass(inName);
        if (theClass == nullptr) {
            Q_ASSERT(false);
            return false;
        }
        QSharedPointer<SMaterialClass> theNewClass(new SMaterialClass(*theClass));
        m_StringMaterialMap.insert(inName, theNewClass);
        return true;
    }

    SMaterialClass *GetMaterialClass(QString inStr)
    {
        TStringMaterialMap::iterator theIter = m_StringMaterialMap.find(inStr);
        if (theIter != m_StringMaterialMap.end())
            return theIter.value().data();
        return nullptr;
    }

    const SMaterialClass *GetMaterialClass(QString inStr) const
    {
        return const_cast<SMaterialSystem *>(this)->GetMaterialClass(inStr);
    }

    virtual QDemonConstDataRef<dynamic::SPropertyDefinition>
    GetCustomMaterialProperties(QString inCustomMaterialName) const override
    {
        IDynamicObjectClass *theMaterialClass =
                m_CoreContext->GetDynamicObjectSystemCore()->GetDynamicObjectClass(inCustomMaterialName);

        if (theMaterialClass)
            return theMaterialClass->GetProperties();

        return QDemonConstDataRef<dynamic::SPropertyDefinition>();
    }

    virtual quint32 FindBuffer(QString inName)
    {
        for (quint32 idx = 0, end = m_AllocatedBuffers.size(); idx < end; ++idx)
            if (m_AllocatedBuffers[idx].m_Name == inName)
                return idx;
        return m_AllocatedBuffers.size();
    }

    virtual quint32 FindAllocatedImage(QString inName)
    {
        for (quint32 idx = 0, end = m_AllocatedImages.size(); idx < end; ++idx)
            if (m_AllocatedImages[idx].first == inName)
                return idx;
        return quint32(-1);
    }

    virtual bool TextureNeedsMips(const dynamic::SPropertyDefinition *inPropDec,
                                  QDemonRenderTexture2D *inTexture)
    {
        if (inPropDec && inTexture) {
            return bool((inPropDec->m_MinFilterOp == QDemonRenderTextureMinifyingOp::LinearMipmapLinear)
                        && (inTexture->GetNumMipmaps() == 0));
        }

        return false;
    }

    virtual void SetTexture(QSharedPointer<QDemonRenderShaderProgram> inShader,
                            QString inPropName,
                            QSharedPointer<QDemonRenderTexture2D> inTexture,
                            const dynamic::SPropertyDefinition *inPropDec = nullptr,
                            bool needMips = false)
    {
        QSharedPointer<SCustomMaterialTextureData> theTextureEntry;
        for (quint32 idx = 0, end = m_TextureEntries.size(); idx < end && theTextureEntry == nullptr;
             ++idx) {
            if (m_TextureEntries[idx].first == inPropName
                    && m_TextureEntries[idx].second->m_Shader == inShader
                    && m_TextureEntries[idx].second->m_Texture == inTexture) {
                theTextureEntry = m_TextureEntries[idx].second;
                break;
            }
        }
        if (theTextureEntry == nullptr) {
            QSharedPointer<SCustomMaterialTextureData> theNewEntry(new SCustomMaterialTextureData(SCustomMaterialTextureData::CreateTextureEntry(inShader, inTexture, inPropName, needMips)));
            m_TextureEntries.push_back(QPair<QString, QSharedPointer<SCustomMaterialTextureData>>(inPropName, theNewEntry));
            theTextureEntry = theNewEntry;
        }
        theTextureEntry->Set(inPropDec);
    }

    void SetPropertyEnumNames(QString inName, QString inPropName,
                              QDemonConstDataRef<QString> inNames) override
    {
        m_CoreContext->GetDynamicObjectSystemCore()->SetPropertyEnumNames(inName, inPropName,inNames);
    }

    void SetPropertyTextureSettings(QString inName,
                                    QString inPropName,
                                    QString inPropPath,
                                    QDemonRenderTextureTypeValue::Enum inTexType,
                                    QDemonRenderTextureCoordOp::Enum inCoordOp,
                                    QDemonRenderTextureMagnifyingOp::Enum inMagFilterOp,
                                    QDemonRenderTextureMinifyingOp::Enum inMinFilterOp) override
    {
        SMaterialClass *theClass = GetMaterialClass(inName);
        if (theClass && inTexType == QDemonRenderTextureTypeValue::Displace) {
            theClass->m_HasDisplacement = true;
        }
        m_CoreContext->GetDynamicObjectSystemCore()->SetPropertyTextureSettings(
                    inName, inPropName, inPropPath, inTexType, inCoordOp, inMagFilterOp, inMinFilterOp);
    }

    void SetMaterialClassShader(QString inName, const char *inShaderType,
                                const char *inShaderVersion, const char *inShaderData,
                                bool inHasGeomShader, bool inIsComputeShader) override
    {
        m_CoreContext->GetDynamicObjectSystemCore()->SetShaderData(inName, inShaderData, inShaderType,
                                                                   inShaderVersion, inHasGeomShader,
                                                                   inIsComputeShader);
    }

    SCustomMaterial *CreateCustomMaterial(QString inName) override
    {
        SCustomMaterial *theMaterial = static_cast<SCustomMaterial *>(m_CoreContext->GetDynamicObjectSystemCore()->CreateInstance(inName));
        SMaterialClass *theClass = GetMaterialClass(inName);

        if (theMaterial) {
            quint32 key = 0, count = 0;
            if (theClass) {
                key = theClass->m_ShaderKey;
                count = theClass->m_LayerCount;
            }
            theMaterial->Initialize(key, count);
        }

        return theMaterial;
    }

    void SetCustomMaterialTransparency(QString inName, bool inHasTransparency) override
    {
        SMaterialClass *theClass = GetMaterialClass(inName);

        if (theClass == nullptr) {
            Q_ASSERT(false);
            return;
        }

        theClass->m_HasTransparency = inHasTransparency;
    }

    void SetCustomMaterialRefraction(QString inName, bool inHasRefraction) override
    {
        SMaterialClass *theClass = GetMaterialClass(inName);

        if (theClass == nullptr) {
            Q_ASSERT(false);
            return;
        }

        theClass->m_HasRefraction = inHasRefraction;
    }

    void SetCustomMaterialAlwaysDirty(QString inName, bool inIsAlwaysDirty) override
    {
        SMaterialClass *theClass = GetMaterialClass(inName);

        if (theClass == nullptr) {
            Q_ASSERT(false);
            return;
        }

        theClass->m_AlwaysDirty = inIsAlwaysDirty;
    }

    void SetCustomMaterialShaderKey(QString inName, quint32 inShaderKey) override
    {
        SMaterialClass *theClass = GetMaterialClass(inName);

        if (theClass == nullptr) {
            Q_ASSERT(false);
            return;
        }

        theClass->m_ShaderKey = inShaderKey;
    }

    void SetCustomMaterialLayerCount(QString inName, quint32 inLayerCount) override
    {
        SMaterialClass *theClass = GetMaterialClass(inName);

        if (theClass == nullptr) {
            Q_ASSERT(false);
            return;
        }

        theClass->m_LayerCount = inLayerCount;
    }

    void SetCustomMaterialCommands(QString inName,
                                   QDemonConstDataRef<dynamic::SCommand *> inCommands) override
    {
        m_CoreContext->GetDynamicObjectSystemCore()->SetRenderCommands(inName, inCommands);
    }

    QString GetShaderCacheKey(QString &inShaderKeyBuffer,
                              const QString &inId,
                              const QString &inProgramMacro,
                              const dynamic::SDynamicShaderProgramFlags &inFlags)
    {
        inShaderKeyBuffer = inId;
        if (!inProgramMacro.isNull() && !inProgramMacro.isNull()) {
            inShaderKeyBuffer.append("#");
            inShaderKeyBuffer.append(inProgramMacro);
        }
        if (inFlags.IsTessellationEnabled()) {
            inShaderKeyBuffer.append("#");
            inShaderKeyBuffer.append(TessModeValues::toString(inFlags.m_TessMode));
        }
        if (inFlags.IsGeometryShaderEnabled() && inFlags.m_WireframeMode) {
            inShaderKeyBuffer.append("#");
            inShaderKeyBuffer.append(inFlags.wireframeToString(inFlags.m_WireframeMode));
        }

        return inShaderKeyBuffer;
    }

    QSharedPointer<QDemonRenderShaderProgram> GetShader(QSharedPointer<SCustomMaterialRenderContext> inRenderContext,
                                                        const SCustomMaterial &inMaterial,
                                                        const dynamic::SBindShader &inCommand, TShaderFeatureSet inFeatureSet,
                                                        const dynamic::SDynamicShaderProgramFlags &inFlags)
    {
        QSharedPointer<ICustomMaterialShaderGenerator> theMaterialGenerator(m_Context->GetCustomMaterialShaderGenerator());

        // generate key
        QString theShaderKeyBuffer;
        QString theKey = GetShaderCacheKey(theShaderKeyBuffer, inCommand.m_ShaderPath,
                                           inCommand.m_ShaderDefine, inFlags);

        SCustomMaterialVertexPipeline thePipeline(m_Context, inRenderContext->m_Model.m_TessellationMode);

        QSharedPointer<QDemonRenderShaderProgram> theProgram = theMaterialGenerator->GenerateShader(
                    inMaterial, inRenderContext->m_MaterialKey, thePipeline, inFeatureSet,
                    inRenderContext->m_Lights, inRenderContext->m_FirstImage,
                    (inMaterial.m_hasTransparency || inMaterial.m_hasRefraction),
                    "custom material pipeline-- ", inCommand.m_ShaderPath);

        return theProgram;
    }

    SMaterialOrComputeShader BindShader(QSharedPointer<SCustomMaterialRenderContext> inRenderContext,
                                        const SCustomMaterial &inMaterial,
                                        const dynamic::SBindShader &inCommand,
                                        TShaderFeatureSet inFeatureSet)
    {
        QSharedPointer<QDemonRenderShaderProgram> theProgram;

        dynamic::SDynamicShaderProgramFlags theFlags(inRenderContext->m_Model.m_TessellationMode,
                                                     inRenderContext->m_Subset.m_WireframeMode);
        theFlags.SetTessellationEnabled(inRenderContext->m_Model.m_TessellationMode != TessModeValues::NoTess);
        theFlags.SetGeometryShaderEnabled(inRenderContext->m_Subset.m_WireframeMode);

        SShaderMapKey skey = SShaderMapKey(
                    TStrStrPair(inCommand.m_ShaderPath, inCommand.m_ShaderDefine), inFeatureSet,
                    theFlags.m_TessMode, theFlags.m_WireframeMode, inRenderContext->m_MaterialKey);
        auto theInsertResult = m_ShaderMap.find(skey);
        //QPair<TShaderMap::iterator, bool> theInsertResult(m_ShaderMap.insert(skey, QSharedPointer<SCustomMaterialShader>(nullptr)));

        if (theInsertResult == m_ShaderMap.end()) {
            theProgram = GetShader(inRenderContext, inMaterial, inCommand, inFeatureSet, theFlags);

            if (theProgram) {
                theInsertResult = m_ShaderMap.insert(skey, QSharedPointer<SCustomMaterialShader>(new SCustomMaterialShader(theProgram, theFlags)));
            }
        } else if (theInsertResult.value())
            theProgram = theInsertResult.value()->m_Shader;

        if (theProgram) {
            if (theProgram->GetProgramType() == QDemonRenderShaderProgram::ProgramType::Graphics) {
                if (theInsertResult.value()) {
                    QSharedPointer<QDemonRenderContext> theContext(m_Context->GetRenderContext());
                    theContext->SetActiveShader(theInsertResult.value()->m_Shader);
                }

                return theInsertResult.value();
            } else {
                QSharedPointer<QDemonRenderContext> theContext(m_Context->GetRenderContext());
                theContext->SetActiveShader(theProgram);
                return theProgram;
            }
        }
        return SMaterialOrComputeShader();
    }

    void DoApplyInstanceValue(SCustomMaterial & /* inMaterial */, quint8 *inDataPtr,
                              QString inPropertyName,
                              QDemonRenderShaderDataTypes::Enum inPropertyType,
                              QSharedPointer<QDemonRenderShaderProgram> inShader,
                              const dynamic::SPropertyDefinition &inDefinition)
    {
        QSharedPointer<QDemonRenderShaderConstantBase> theConstant = inShader->GetShaderConstant(inPropertyName.toLocal8Bit());
        if (theConstant) {
            if (theConstant->GetShaderConstantType() == inPropertyType) {
                if (inPropertyType == QDemonRenderShaderDataTypes::Texture2D) {
                    //                    StaticAssert<sizeof(QString) == sizeof(QDemonRenderTexture2DPtr)>::valid_expression();
                    QString *theStrPtr = reinterpret_cast<QString *>(inDataPtr);
                    QSharedPointer<IBufferManager> theBufferManager(m_Context->GetBufferManager());
                    QSharedPointer<QDemonRenderTexture2D> theTexture;

                    if (!theStrPtr->isNull()) {
                        SImageTextureData theTextureData = theBufferManager->LoadRenderImage(*theStrPtr);
                        if (theTextureData.m_Texture) {
                            theTexture = theTextureData.m_Texture;
                            SetTexture(inShader, inPropertyName, theTexture, &inDefinition,
                                       TextureNeedsMips(&inDefinition, theTexture.data()));
                        }
                    }
                } else {
                    switch (inPropertyType) {
                    case QDemonRenderShaderDataTypes::Integer:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<qint32 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::IntegerVec2:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<qint32_2 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::IntegerVec3:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<qint32_3 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::IntegerVec4:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<qint32_4 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Boolean:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<bool *>(inDataPtr)));
                        break;                    case QDemonRenderShaderDataTypes::BooleanVec2:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<bool_2 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::BooleanVec3:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<bool_3 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::BooleanVec4:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<bool_4 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Float:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<float *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Vec2:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QVector2D *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Vec3:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QVector3D *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Vec4:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QVector4D *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::UnsignedInteger:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<quint32 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::UnsignedIntegerVec2:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<quint32_2 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::UnsignedIntegerVec3:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<quint32_3 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::UnsignedIntegerVec4:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<quint32_4 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Matrix3x3:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QMatrix3x3 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Matrix4x4:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QMatrix4x4 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Texture2D:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderTexture2DPtr *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Texture2DHandle:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderTexture2DHandle *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Texture2DArray:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderTexture2DArrayPtr *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::TextureCube:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderTextureCubePtr *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::TextureCubeHandle:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderTextureCubeHandle *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Image2D:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderImage2DPtr *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::DataBuffer:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderDataBufferPtr *>(inDataPtr)));
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

    void ApplyInstanceValue(SCustomMaterial &inMaterial,
                            SMaterialClass &inClass,
                            QSharedPointer<QDemonRenderShaderProgram> inShader,
                            const dynamic::SApplyInstanceValue &inCommand)
    {
        // sanity check
        if (!inCommand.m_PropertyName.isNull()) {
            bool canGetData =
                    inCommand.m_ValueOffset + dynamic::getSizeofShaderDataType(inCommand.m_ValueType)
                    <= inMaterial.m_DataSectionByteSize;
            if (canGetData == false) {
                Q_ASSERT(false);
                return;
            }
            quint8 *dataPtr = inMaterial.GetDataSectionBegin() + inCommand.m_ValueOffset;
            const dynamic::SPropertyDefinition *theDefinition =
                    inClass.m_Class->FindPropertyByName(inCommand.m_PropertyName);
            if (theDefinition)
                DoApplyInstanceValue(inMaterial, dataPtr, inCommand.m_PropertyName,
                                     inCommand.m_ValueType, inShader, *theDefinition);
        } else {
            QDemonConstDataRef<dynamic::SPropertyDefinition> theDefs = inClass.m_Class->GetProperties();
            for (quint32 idx = 0, end = theDefs.size(); idx < end; ++idx) {
                const dynamic::SPropertyDefinition &theDefinition(theDefs[idx]);
                QSharedPointer<QDemonRenderShaderConstantBase> theConstant = inShader->GetShaderConstant(theDefinition.m_Name.toLocal8Bit().constData());

                // This is fine, the property wasn't found and we continue, no problem.
                if (!theConstant)
                    continue;
                quint8 *dataPtr = inMaterial.GetDataSectionBegin() + theDefinition.m_Offset;
                DoApplyInstanceValue(inMaterial, dataPtr, theDefinition.m_Name, theDefinition.m_DataType, inShader, theDefinition);
            }
        }
    }

    void ApplyBlending(const dynamic::SApplyBlending &inCommand)
    {
        QSharedPointer<QDemonRenderContext> theContext(m_Context->GetRenderContext());

        theContext->SetBlendingEnabled(true);

        QDemonRenderBlendFunctionArgument blendFunc =
                QDemonRenderBlendFunctionArgument(
                    inCommand.m_SrcBlendFunc, inCommand.m_DstBlendFunc, inCommand.m_SrcBlendFunc,
                    inCommand.m_DstBlendFunc);

        QDemonRenderBlendEquationArgument blendEqu(QDemonRenderBlendEquation::Add,
                                                   QDemonRenderBlendEquation::Add);

        theContext->SetBlendFunction(blendFunc);
        theContext->SetBlendEquation(blendEqu);
    }

    // we currently only bind a source texture
    const QSharedPointer<QDemonRenderTexture2D> ApplyBufferValue(const SCustomMaterial &inMaterial,
                                                                 QSharedPointer<QDemonRenderShaderProgram> inShader,
                                                                 const dynamic::SApplyBufferValue &inCommand,
                                                                 const QSharedPointer<QDemonRenderTexture2D> inSourceTexture)
    {
        QSharedPointer<QDemonRenderTexture2D> theTexture = nullptr;

        if (!inCommand.m_BufferName.isNull()) {
            quint32 bufferIdx = FindBuffer(inCommand.m_BufferName);
            if (bufferIdx < m_AllocatedBuffers.size()) {
                SCustomMaterialBuffer &theEntry(m_AllocatedBuffers[bufferIdx]);
                theTexture = theEntry.m_Texture;
            } else {
                // we must have allocated the read target before
                qCCritical(INTERNAL_ERROR,
                           "CustomMaterial: ApplyBufferValue: Failed to setup read target");
                Q_ASSERT(false);
            }
        } else
            theTexture = inSourceTexture;

        if (!inCommand.m_ParamName.isNull()) {
            QSharedPointer<QDemonRenderShaderConstantBase> theConstant =
                    inShader->GetShaderConstant(inCommand.m_ParamName.toLocal8Bit().constData());

            if (theConstant) {
                if (theConstant->GetShaderConstantType()
                        != QDemonRenderShaderDataTypes::Texture2D) {
                    qCCritical(INVALID_OPERATION,
                               "CustomMaterial %s: Binding buffer to parameter %s that is not a texture",
                               qPrintable(inMaterial.m_ClassName), qPrintable(inCommand.m_ParamName));
                    Q_ASSERT(false);
                } else {
                    SetTexture(inShader, inCommand.m_ParamName, theTexture);
                }
            }
        }

        return theTexture;
    }

    void AllocateBuffer(const dynamic::SAllocateBuffer &inCommand, QSharedPointer<QDemonRenderFrameBuffer> inTarget)
    {
        STextureDetails theSourceTextureDetails;
        QSharedPointer<QDemonRenderTexture2D> theTexture;
        // get color attachment we always assume at location 0
        if (inTarget) {
            QDemonRenderTextureOrRenderBuffer theSourceTexture =
                    inTarget->GetAttachment(QDemonRenderFrameBufferAttachments::Color0);
            // we need a texture
            if (theSourceTexture.HasTexture2D()) {
                theSourceTextureDetails = theSourceTexture.GetTexture2D()->GetTextureDetails();
            } else {
                qCCritical(INVALID_OPERATION, "CustomMaterial %s: Invalid source texture",
                           qPrintable(inCommand.m_Name));
                Q_ASSERT(false);
                return;
            }
        } else {
            QSharedPointer<QDemonRenderContext> theContext = m_Context->GetRenderContext();
            // if we allocate a buffer based on the default target use viewport to get the dimension
            QDemonRenderRect theViewport(theContext->GetViewport());
            theSourceTextureDetails.m_Height = theViewport.m_Height;
            theSourceTextureDetails.m_Width = theViewport.m_Width;
        }

        quint32 theWidth = (quint32)(theSourceTextureDetails.m_Width * inCommand.m_SizeMultiplier);
        quint32 theHeight = (quint32)(theSourceTextureDetails.m_Height * inCommand.m_SizeMultiplier);
        QDemonRenderTextureFormats::Enum theFormat = inCommand.m_Format;
        if (theFormat == QDemonRenderTextureFormats::Unknown)
            theFormat = theSourceTextureDetails.m_Format;
        QSharedPointer<IResourceManager> theResourceManager(m_Context->GetResourceManager());
        // size intentionally requiried every loop;
        quint32 bufferIdx = FindBuffer(inCommand.m_Name);
        if (bufferIdx < m_AllocatedBuffers.size()) {
            SCustomMaterialBuffer &theEntry(m_AllocatedBuffers[bufferIdx]);
            STextureDetails theDetails = theEntry.m_Texture->GetTextureDetails();
            if (theDetails.m_Width == theWidth && theDetails.m_Height == theHeight
                    && theDetails.m_Format == theFormat) {
                theTexture = theEntry.m_Texture;
            } else {
                ReleaseBuffer(bufferIdx);
            }
        }

        if (theTexture == nullptr) {
            QSharedPointer<QDemonRenderFrameBuffer> theFB(theResourceManager->AllocateFrameBuffer());
            QSharedPointer<QDemonRenderTexture2D> theTexture(theResourceManager->AllocateTexture2D(theWidth, theHeight, theFormat));
            theTexture->SetMagFilter(inCommand.m_FilterOp);
            theTexture->SetMinFilter(
                        static_cast<QDemonRenderTextureMinifyingOp::Enum>(inCommand.m_FilterOp));
            theTexture->SetTextureWrapS(inCommand.m_TexCoordOp);
            theTexture->SetTextureWrapT(inCommand.m_TexCoordOp);
            theFB->Attach(QDemonRenderFrameBufferAttachments::Color0, theTexture);
            m_AllocatedBuffers.push_back(SCustomMaterialBuffer(inCommand.m_Name, theFB, theTexture, inCommand.m_BufferFlags));
        }
    }

    QSharedPointer<QDemonRenderFrameBuffer> BindBuffer(const SCustomMaterial &inMaterial, const dynamic::SBindBuffer &inCommand,
                                                       bool &outClearTarget, QVector2D &outDestSize)
    {
        QSharedPointer<QDemonRenderFrameBuffer> theBuffer;
        QSharedPointer<QDemonRenderTexture2D> theTexture;

        // search for the buffer
        quint32 bufferIdx = FindBuffer(inCommand.m_BufferName);
        if (bufferIdx < m_AllocatedBuffers.size()) {
            theBuffer = m_AllocatedBuffers[bufferIdx].m_FrameBuffer;
            theTexture = m_AllocatedBuffers[bufferIdx].m_Texture;
        }

        if (theBuffer == nullptr) {
            qCCritical(INVALID_OPERATION, "Effect %s: Failed to find buffer %s for bind",
                       qPrintable(inMaterial.m_ClassName), qPrintable(inCommand.m_BufferName));
            Q_ASSERT(false);
            return nullptr;
        }

        if (theTexture) {
            STextureDetails theDetails(theTexture->GetTextureDetails());
            m_Context->GetRenderContext()->SetViewport(QDemonRenderRect(0, 0, (quint32)theDetails.m_Width, (quint32)theDetails.m_Height));
            outDestSize = QVector2D((float)theDetails.m_Width, (float)theDetails.m_Height);
            outClearTarget = inCommand.m_NeedsClear;
        }

        return theBuffer;
    }

    void computeScreenCoverage(QSharedPointer<SCustomMaterialRenderContext> inRenderContext, qint32 *xMin,
                               qint32 *yMin, qint32 *xMax, qint32 *yMax)
    {
        QSharedPointer<QDemonRenderContext> theContext(m_Context->GetRenderContext());
        QDemonBounds2BoxPoints outPoints;
        const float MaxFloat = std::numeric_limits<float>::max();
        QVector4D projMin(MaxFloat, MaxFloat, MaxFloat, MaxFloat);
        QVector4D projMax(-MaxFloat, -MaxFloat, -MaxFloat, -MaxFloat);

        // get points
        inRenderContext->m_Subset.m_Bounds.expand(outPoints);
        for (quint32 idx = 0; idx < 8; ++idx) {
            QVector4D homPoint(outPoints[idx], 1.0);
            QVector4D projPoint = mat44::transform(inRenderContext->m_ModelViewProjection, homPoint);
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

        QDemonRenderRect theViewport(theContext->GetViewport());
        qint32 x1 = qint32(projMax.x() * (theViewport.m_Width / 2)
                           + (theViewport.m_X + (theViewport.m_Width / 2)));
        qint32 y1 = qint32(projMax.y() * (theViewport.m_Height / 2)
                           + (theViewport.m_Y + (theViewport.m_Height / 2)));

        qint32 x2 = qint32(projMin.x() * (theViewport.m_Width / 2)
                           + (theViewport.m_X + (theViewport.m_Width / 2)));
        qint32 y2 = qint32(projMin.y() * (theViewport.m_Height / 2)
                           + (theViewport.m_Y + (theViewport.m_Height / 2)));

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

    void BlitFramebuffer(QSharedPointer<SCustomMaterialRenderContext> inRenderContext,
                         const dynamic::SApplyBlitFramebuffer &inCommand,
                         QSharedPointer<QDemonRenderFrameBuffer> inTarget)
    {
        QSharedPointer<QDemonRenderContext> theContext(m_Context->GetRenderContext());
        // we change the read/render targets here
        QDemonRenderContextScopedProperty<QSharedPointer<QDemonRenderFrameBuffer>> __framebuffer(
                    *theContext, &QDemonRenderContext::GetRenderTarget, &QDemonRenderContext::SetRenderTarget);
        // we may alter scissor
        QDemonRenderContextScopedProperty<bool> theScissorEnabled(
                    *theContext, &QDemonRenderContext::IsScissorTestEnabled,
                    &QDemonRenderContext::SetScissorTestEnabled);

        if (!inCommand.m_DestBufferName.isNull()) {
            quint32 bufferIdx = FindBuffer(inCommand.m_DestBufferName);
            if (bufferIdx < m_AllocatedBuffers.size()) {
                SCustomMaterialBuffer &theEntry(m_AllocatedBuffers[bufferIdx]);
                theContext->SetRenderTarget(theEntry.m_FrameBuffer);
            } else {
                // we must have allocated the read target before
                qCCritical(INTERNAL_ERROR,
                           "CustomMaterial: BlitFramebuffer: Failed to setup render target");
                Q_ASSERT(false);
            }
        } else {
            // our dest is the default render target
            theContext->SetRenderTarget(inTarget);
        }

        if (!inCommand.m_SourceBufferName.isNull()) {
            quint32 bufferIdx = FindBuffer(inCommand.m_SourceBufferName);
            if (bufferIdx < m_AllocatedBuffers.size()) {
                SCustomMaterialBuffer &theEntry(m_AllocatedBuffers[bufferIdx]);
                theContext->SetReadTarget(theEntry.m_FrameBuffer);
                theContext->SetReadBuffer(QDemonReadFaces::Color0);
            } else {
                // we must have allocated the read target before
                qCCritical(INTERNAL_ERROR,
                           "CustomMaterial: BlitFramebuffer: Failed to setup read target");
                Q_ASSERT(false);
            }
        } else {
            // our source is the default read target
            // depending on what we render we assume color0 or back
            theContext->SetReadTarget(inTarget);
            QDemonReadFaces::Enum value = (inTarget) ? QDemonReadFaces::Color0 : QDemonReadFaces::Back;
            theContext->SetReadBuffer(value);
        }

        QDemonRenderRect theViewport(theContext->GetViewport());
        theContext->SetScissorTestEnabled(false);

        if (!m_UseFastBlits) {
            // only copy sreen amount of pixels
            qint32 xMin, yMin, xMax, yMax;
            computeScreenCoverage(inRenderContext, &xMin, &yMin, &xMax, &yMax);

            // same dimension
            theContext->BlitFramebuffer(xMin, yMin, xMax, yMax, xMin, yMin, xMax, yMax,
                                       QDemonRenderClearValues::Color,
                                       QDemonRenderTextureMagnifyingOp::Nearest);
        } else {
            // same dimension
            theContext->BlitFramebuffer(
                        theViewport.m_X, theViewport.m_Y, theViewport.m_X + theViewport.m_Width,
                        theViewport.m_Y + theViewport.m_Height, theViewport.m_X, theViewport.m_Y,
                        theViewport.m_X + theViewport.m_Width, theViewport.m_Y + theViewport.m_Height,
                        QDemonRenderClearValues::Color, QDemonRenderTextureMagnifyingOp::Nearest);
        }
    }

    SLayerGlobalRenderProperties
    GetLayerGlobalRenderProperties(QSharedPointer<SCustomMaterialRenderContext> inRenderContext)
    {
        const SLayer &theLayer = inRenderContext->m_Layer;
        const SLayerRenderData &theData = inRenderContext->m_LayerData;

        // ### Use SLayerRenderData when it is ready
//        return SLayerGlobalRenderProperties(
//                    theLayer, const_cast<SCamera &>(inRenderContext->m_Camera), theData.m_CameraDirection,
//                    inRenderContext->m_Lights, QDemonDataRef<QVector3D>(), theData.m_ShadowMapManager,
//                    const_cast<QDemonRenderTexture2D *>(inRenderContext->m_DepthTexture),
//                    const_cast<QDemonRenderTexture2D *>(inRenderContext->m_AOTexture), theLayer.m_LightProbe,
//                    theLayer.m_LightProbe2, theLayer.m_ProbeHorizon, theLayer.m_ProbeBright,
//                    theLayer.m_Probe2Window, theLayer.m_Probe2Pos, theLayer.m_Probe2Fade,
//                    theLayer.m_ProbeFov);
        return SLayerGlobalRenderProperties(
                    theLayer, const_cast<SCamera &>(inRenderContext->m_Camera), QVector3D(),
                    inRenderContext->m_Lights, QDemonDataRef<QVector3D>(), QSharedPointer<QDemonRenderShadowMap>(),
                    inRenderContext->m_DepthTexture,
                    inRenderContext->m_AOTexture, theLayer.m_LightProbe,
                    theLayer.m_LightProbe2, theLayer.m_ProbeHorizon, theLayer.m_ProbeBright,
                    theLayer.m_Probe2Window, theLayer.m_Probe2Pos, theLayer.m_Probe2Fade,
                    theLayer.m_ProbeFov);
    }

    void RenderPass(QSharedPointer<SCustomMaterialRenderContext> inRenderContext,
                    QSharedPointer<SCustomMaterialShader> inShader,
                    QSharedPointer<QDemonRenderTexture2D> /* inSourceTexture */,
                    QSharedPointer<QDemonRenderFrameBuffer> inFrameBuffer,
                    bool inRenderTargetNeedsClear,
                    QSharedPointer<QDemonRenderInputAssembler> inAssembler,
                    quint32 inCount,
                    quint32 inOffset)
    {
        QSharedPointer<QDemonRenderContext> theContext(m_Context->GetRenderContext());
        theContext->SetRenderTarget(inFrameBuffer);

        QVector4D clearColor(0.0, 0.0, 0.0, 0.0);
        QDemonRenderContextScopedProperty<QVector4D> __clearColor(
                    *theContext, &QDemonRenderContext::GetClearColor, &QDemonRenderContext::SetClearColor,
                    clearColor);
        if (inRenderTargetNeedsClear) {
            theContext->Clear(QDemonRenderClearValues::Color);
        }

        QSharedPointer<ICustomMaterialShaderGenerator> theMaterialGenerator(m_Context->GetCustomMaterialShaderGenerator());

        theMaterialGenerator->SetMaterialProperties(
                    inShader->m_Shader, inRenderContext->m_Material, QVector2D(1.0, 1.0),
                    inRenderContext->m_ModelViewProjection, inRenderContext->m_NormalMatrix,
                    inRenderContext->m_ModelMatrix, inRenderContext->m_FirstImage, inRenderContext->m_Opacity,
                    GetLayerGlobalRenderProperties(inRenderContext));

        // I think the prim type should always be fetched from the
        // current mesh subset setup because there you get the actual draw mode
        // for this frame
        QDemonRenderDrawMode::Enum theDrawMode = inAssembler->GetPrimitiveType();

        // tesselation
        if (inRenderContext->m_Subset.m_PrimitiveType == QDemonRenderDrawMode::Patches) {
            QVector2D camProps(inRenderContext->m_Camera.m_ClipNear,
                               inRenderContext->m_Camera.m_ClipFar);
            theDrawMode = inRenderContext->m_Subset.m_PrimitiveType;
            inShader->m_Tessellation.m_EdgeTessLevel.Set(inRenderContext->m_Subset.m_EdgeTessFactor);
            inShader->m_Tessellation.m_InsideTessLevel.Set(
                        inRenderContext->m_Subset.m_InnerTessFactor);
            // the blend value is hardcoded
            inShader->m_Tessellation.m_PhongBlend.Set(0.75);
            // this should finally be based on some user input
            inShader->m_Tessellation.m_DistanceRange.Set(camProps);
            // enable culling
            inShader->m_Tessellation.m_DisableCulling.Set(0.0);
        }

        if (inRenderContext->m_Subset.m_WireframeMode) {
            QDemonRenderRect theViewport(theContext->GetViewport());
            QMatrix4x4 vpMatrix = {
                (float)theViewport.m_Width / 2.0f, 0.0, 0.0, 0.0,
                0.0, (float)theViewport.m_Height / 2.0f, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                (float)theViewport.m_Width / 2.0f + (float)theViewport.m_X,
                (float)theViewport.m_Height / 2.0f + (float)theViewport.m_Y, 0.0, 1.0
            };

            inShader->m_ViewportMatrix.Set(vpMatrix);
        }

        theContext->SetInputAssembler(inAssembler);

        theContext->SetCullingEnabled(true);
        quint32 count = inCount;
        quint32 offset = inOffset;

        theContext->Draw(theDrawMode, count, offset);
    }

    void DoRenderCustomMaterial(QSharedPointer<SCustomMaterialRenderContext> inRenderContext,
                                const SCustomMaterial &inMaterial,
                                SMaterialClass &inClass,
                                QSharedPointer<QDemonRenderFrameBuffer> inTarget,
                                TShaderFeatureSet inFeatureSet)
    {
        QSharedPointer<QDemonRenderContext> theContext = m_Context->GetRenderContext();
        QSharedPointer<SCustomMaterialShader> theCurrentShader(nullptr);

        QSharedPointer<QDemonRenderFrameBuffer> theCurrentRenderTarget(inTarget);
        QDemonRenderRect theOriginalViewport(theContext->GetViewport());
        QSharedPointer<QDemonRenderTexture2D> theCurrentSourceTexture;

        // for refrative materials we come from the transparent render path
        // but we do not want to do blending
        bool wasBlendingEnabled = theContext->IsBlendingEnabled();
        if (inMaterial.m_hasRefraction)
            theContext->SetBlendingEnabled(false);

        QDemonRenderContextScopedProperty<QSharedPointer<QDemonRenderFrameBuffer>> __framebuffer(
                    *theContext, &QDemonRenderContext::GetRenderTarget, &QDemonRenderContext::SetRenderTarget);
        QDemonRenderContextScopedProperty<QDemonRenderRect> __viewport(
                    *theContext, &QDemonRenderContext::GetViewport, &QDemonRenderContext::SetViewport);

        QVector2D theDestSize;
        bool theRenderTargetNeedsClear = false;

        QDemonConstDataRef<dynamic::SCommand *> theCommands(inClass.m_Class->GetRenderCommands());
        for (quint32 commandIdx = 0, commandEnd = theCommands.size(); commandIdx < commandEnd;
             ++commandIdx) {
            const dynamic::SCommand &theCommand(*theCommands[commandIdx]);

            switch (theCommand.m_Type) {
            case dynamic::CommandTypes::AllocateBuffer:
                AllocateBuffer(static_cast<const dynamic::SAllocateBuffer &>(theCommand), inTarget);
                break;
            case dynamic::CommandTypes::BindBuffer:
                theCurrentRenderTarget =
                        BindBuffer(inMaterial, static_cast<const dynamic::SBindBuffer &>(theCommand),
                                   theRenderTargetNeedsClear, theDestSize);
                break;
            case dynamic::CommandTypes::BindTarget:
                // Restore the previous render target and info.
                theCurrentRenderTarget = inTarget;
                theContext->SetViewport(theOriginalViewport);
                break;
            case dynamic::CommandTypes::BindShader: {
                theCurrentShader = nullptr;
                SMaterialOrComputeShader theBindResult =
                        BindShader(inRenderContext, inMaterial,
                                   static_cast<const dynamic::SBindShader &>(theCommand), inFeatureSet);
                if (theBindResult.IsMaterialShader())
                    theCurrentShader = theBindResult.MaterialShader();
            } break;
            case dynamic::CommandTypes::ApplyInstanceValue:
                // we apply the property update explicitly at the render pass
                break;
            case dynamic::CommandTypes::Render:
                if (theCurrentShader) {
                    RenderPass(inRenderContext, theCurrentShader, theCurrentSourceTexture,
                               theCurrentRenderTarget, theRenderTargetNeedsClear,
                               inRenderContext->m_Subset.m_InputAssembler,
                               inRenderContext->m_Subset.m_Count, inRenderContext->m_Subset.m_Offset);
                }
                // reset
                theRenderTargetNeedsClear = false;
                break;
            case dynamic::CommandTypes::ApplyBlending:
                ApplyBlending(static_cast<const dynamic::SApplyBlending &>(theCommand));
                break;
            case dynamic::CommandTypes::ApplyBufferValue:
                if (theCurrentShader)
                    ApplyBufferValue(inMaterial, theCurrentShader->m_Shader,
                                     static_cast<const dynamic::SApplyBufferValue &>(theCommand),
                                     theCurrentSourceTexture);
                break;
            case dynamic::CommandTypes::ApplyBlitFramebuffer:
                BlitFramebuffer(inRenderContext,
                                static_cast<const dynamic::SApplyBlitFramebuffer &>(theCommand), inTarget);
                break;
            default:
                Q_ASSERT(false);
                break;
            }
        }

        if (inMaterial.m_hasRefraction)
            theContext->SetBlendingEnabled(wasBlendingEnabled);

        // Release any per-frame buffers
        for (quint32 idx = 0; idx < m_AllocatedBuffers.size(); ++idx) {
            if (m_AllocatedBuffers[idx].m_Flags.IsSceneLifetime() == false) {
                ReleaseBuffer(idx);
                --idx;
            }
        }
    }

    QString GetShaderName(const SCustomMaterial &inMaterial) override
    {
        SMaterialClass *theClass = GetMaterialClass(inMaterial.m_ClassName);
        if (!theClass)
            return QString();

        QDemonConstDataRef<dynamic::SCommand *> theCommands = theClass->m_Class->GetRenderCommands();
        TShaderAndFlags thePrepassShader;
        for (quint32 idx = 0, end = theCommands.size();
             idx < end && thePrepassShader.first == nullptr; ++idx) {
            const dynamic::SCommand &theCommand = *theCommands[idx];
            if (theCommand.m_Type == dynamic::CommandTypes::BindShader) {
                const dynamic::SBindShader &theBindCommand = static_cast<const dynamic::SBindShader &>(theCommand);
                return theBindCommand.m_ShaderPath;
            }
        }

        Q_UNREACHABLE();
        return QString();
    }

    void ApplyShaderPropertyValues(const SCustomMaterial &inMaterial, QSharedPointer<QDemonRenderShaderProgram> inProgram) override
    {
        SMaterialClass *theClass = GetMaterialClass(inMaterial.m_ClassName);
        if (!theClass)
            return;

        dynamic::SApplyInstanceValue applier;
        ApplyInstanceValue(const_cast<SCustomMaterial &>(inMaterial), *theClass, inProgram,
                           applier);
    }

    virtual void PrepareTextureForRender(SMaterialClass &inClass, SCustomMaterial &inMaterial)
    {
        QDemonConstDataRef<dynamic::SPropertyDefinition> thePropDefs = inClass.m_Class->GetProperties();
        for (quint32 idx = 0, end = thePropDefs.size(); idx < end; ++idx) {
            if (thePropDefs[idx].m_DataType == QDemonRenderShaderDataTypes::Texture2D) {
                if (thePropDefs[idx].m_TexUsageType == QDemonRenderTextureTypeValue::Displace) {
                    SImage *pImage = nullptr;

                    // we only do this to not miss if "None" is selected
                    QString theStrPtr = *reinterpret_cast<QString *>(
                                inMaterial.GetDataSectionBegin() + thePropDefs[idx].m_Offset);

                    if (!theStrPtr.isNull()) {

                        quint32 index = FindAllocatedImage(thePropDefs[idx].m_ImagePath);
                        if (index == quint32(-1)) {
                            pImage = new SImage();
                            m_AllocatedImages.push_back(
                                        QPair<QString, SImage *>(thePropDefs[idx].m_ImagePath, pImage));
                        } else
                            pImage = m_AllocatedImages[index].second;

                        if (inMaterial.m_DisplacementMap != pImage) {
                            inMaterial.m_DisplacementMap = pImage;
                            inMaterial.m_DisplacementMap->m_ImagePath =
                                    thePropDefs[idx].m_ImagePath;
                            inMaterial.m_DisplacementMap->m_ImageShaderName =
                                    thePropDefs[idx].m_Name; // this is our name in the shader
                            inMaterial.m_DisplacementMap->m_VerticalTilingMode =
                                    thePropDefs[idx].m_CoordOp;
                            inMaterial.m_DisplacementMap->m_HorizontalTilingMode =
                                    thePropDefs[idx].m_CoordOp;
                        }
                    } else {
                        inMaterial.m_DisplacementMap = nullptr;
                    }
                } else if (thePropDefs[idx].m_TexUsageType == QDemonRenderTextureTypeValue::Emissive2) {
                    SImage *pImage = nullptr;

                    // we only do this to not miss if "None" is selected
                    QString theStrPtr = *reinterpret_cast<QString *>(
                                inMaterial.GetDataSectionBegin() + thePropDefs[idx].m_Offset);

                    if (!theStrPtr.isNull()) {
                        quint32 index = FindAllocatedImage(thePropDefs[idx].m_ImagePath);
                        if (index == quint32(-1)) {
                            pImage = new SImage();
                            m_AllocatedImages.push_back(
                                        QPair<QString, SImage *>(thePropDefs[idx].m_ImagePath, pImage));
                        } else
                            pImage = m_AllocatedImages[index].second;

                        if (inMaterial.m_EmissiveMap2 != pImage) {
                            inMaterial.m_EmissiveMap2 = pImage;
                            inMaterial.m_EmissiveMap2->m_ImagePath = thePropDefs[idx].m_ImagePath;
                            inMaterial.m_EmissiveMap2->m_ImageShaderName =
                                    thePropDefs[idx].m_Name; // this is our name in the shader
                            inMaterial.m_EmissiveMap2->m_VerticalTilingMode =
                                    thePropDefs[idx].m_CoordOp;
                            inMaterial.m_EmissiveMap2->m_HorizontalTilingMode =
                                    thePropDefs[idx].m_CoordOp;
                        }
                    } else {
                        inMaterial.m_EmissiveMap2 = nullptr;
                    }
                }
            }
        }
    }

    virtual void PrepareDisplacementForRender(SMaterialClass &inClass, SCustomMaterial &inMaterial)
    {
        if (inMaterial.m_DisplacementMap == nullptr)
            return;

        // our displacement mappin in MDL has fixed naming
        QDemonConstDataRef<dynamic::SPropertyDefinition> thePropDefs = inClass.m_Class->GetProperties();
        for (quint32 idx = 0, end = thePropDefs.size(); idx < end; ++idx) {
            if (thePropDefs[idx].m_DataType == QDemonRenderShaderDataTypes::Float
                    && (thePropDefs[idx].m_Name == QStringLiteral("displaceAmount"))) {
                float theValue = *reinterpret_cast<const float *>(inMaterial.GetDataSectionBegin()
                                                                  + thePropDefs[idx].m_Offset);
                inMaterial.m_DisplaceAmount = theValue;
            } else if (thePropDefs[idx].m_DataType == QDemonRenderShaderDataTypes::Vec3
                       && (thePropDefs[idx].m_Name == QStringLiteral("displace_tiling"))) {
                QVector3D theValue = *reinterpret_cast<const QVector3D *>(inMaterial.GetDataSectionBegin()
                                                                          + thePropDefs[idx].m_Offset);
                if (theValue.x() != inMaterial.m_DisplacementMap->m_Scale.x()
                        || theValue.y() != inMaterial.m_DisplacementMap->m_Scale.y()) {
                    inMaterial.m_DisplacementMap->m_Scale = QVector2D(theValue.x(), theValue.y());
                    inMaterial.m_DisplacementMap->m_Flags.SetTransformDirty(true);
                }
            }
        }
    }

    void PrepareMaterialForRender(SMaterialClass &inClass, SCustomMaterial &inMaterial)
    {
        PrepareTextureForRender(inClass, inMaterial);

        if (inClass.m_HasDisplacement)
            PrepareDisplacementForRender(inClass, inMaterial);
    }

    // Returns true if the material is dirty and thus will produce a different render result
    // than previously.  This effects things like progressive AA.
    // TODO - return more information, specifically about transparency (object is transparent,
    // object is completely transparent
    bool PrepareForRender(const SModel & /*inModel*/, const SRenderSubset & /*inSubset*/,
                          SCustomMaterial &inMaterial, bool clearMaterialDirtyFlags) override
    {
        SMaterialClass *theMaterialClass = GetMaterialClass(inMaterial.m_ClassName);
        if (theMaterialClass == nullptr) {
            Q_ASSERT(false);
            return false;
        }

        PrepareMaterialForRender(*theMaterialClass, inMaterial);

        inMaterial.m_hasTransparency = theMaterialClass->m_HasTransparency;
        inMaterial.m_hasRefraction = theMaterialClass->m_HasRefraction;
        inMaterial.m_hasVolumetricDF = false;

        bool wasDirty = inMaterial.IsDirty() || theMaterialClass->m_AlwaysDirty;
        if (clearMaterialDirtyFlags)
            inMaterial.UpdateDirtyForFrame();

        return wasDirty;
    }

    // TODO - handle UIC specific features such as vertex offsets for prog-aa and opacity.
    void RenderSubset(QSharedPointer<SCustomMaterialRenderContext> inRenderContext,
                      TShaderFeatureSet inFeatureSet) override
    {
        SMaterialClass *theClass = GetMaterialClass(inRenderContext->m_Material.m_ClassName);

        // Ensure that our overall render context comes back no matter what the client does.
        QDemonRenderContextScopedProperty<QDemonRenderBlendFunctionArgument>
                __blendFunction(*m_Context->GetRenderContext(), &QDemonRenderContext::GetBlendFunction,
                                &QDemonRenderContext::SetBlendFunction,
                                QDemonRenderBlendFunctionArgument());
        QDemonRenderContextScopedProperty<QDemonRenderBlendEquationArgument>
                __blendEquation(*m_Context->GetRenderContext(), &QDemonRenderContext::GetBlendEquation,
                                &QDemonRenderContext::SetBlendEquation,
                                QDemonRenderBlendEquationArgument());

        QDemonRenderContextScopedProperty<bool> theBlendEnabled(*m_Context->GetRenderContext(),
                                                                &QDemonRenderContext::IsBlendingEnabled,
                                                                &QDemonRenderContext::SetBlendingEnabled);

        DoRenderCustomMaterial(inRenderContext, inRenderContext->m_Material, *theClass,
                               m_Context->GetRenderContext()->GetRenderTarget(), inFeatureSet);
    }

    bool RenderDepthPrepass(const QMatrix4x4 &inMVP, const SCustomMaterial &inMaterial, const SRenderSubset &inSubset) override
    {
        SMaterialClass *theClass = GetMaterialClass(inMaterial.m_ClassName);
        QDemonConstDataRef<dynamic::SCommand *> theCommands = theClass->m_Class->GetRenderCommands();
        TShaderAndFlags thePrepassShader;
        for (quint32 idx = 0, end = theCommands.size();
             idx < end && thePrepassShader.first == nullptr; ++idx) {
            const dynamic::SCommand &theCommand = *theCommands[idx];
            if (theCommand.m_Type == dynamic::CommandTypes::BindShader) {
                const dynamic::SBindShader &theBindCommand = static_cast<const dynamic::SBindShader &>(theCommand);
                thePrepassShader = m_Context->GetDynamicObjectSystem()->GetDepthPrepassShader(
                            theBindCommand.m_ShaderPath, QString(), TShaderFeatureSet());
            }
        }

        if (thePrepassShader.first == nullptr)
            return false;

        QSharedPointer<QDemonRenderContext> theContext = m_Context->GetRenderContext();
        QSharedPointer<QDemonRenderShaderProgram> theProgram = thePrepassShader.first;
        theContext->SetActiveShader(theProgram);
        theProgram->SetPropertyValue("model_view_projection", inMVP);
        theContext->SetInputAssembler(inSubset.m_InputAssemblerPoints);
        theContext->Draw(QDemonRenderDrawMode::Lines, inSubset.m_PosVertexBuffer->GetNumVertexes(), 0);
        return true;
    }

    void OnMaterialActivationChange(const SCustomMaterial &inMaterial, bool inActive) override
    {
        Q_UNUSED(inMaterial)
        Q_UNUSED(inActive)
    }

    void EndFrame() override
    {
        quint64 currentFrameTime = QDemonTime::getCurrentTimeInTensOfNanoSeconds();
        if (m_LastFrameTime) {
            quint64 timePassed = currentFrameTime - m_LastFrameTime;
            m_MillisecondsSinceLastFrame = static_cast<float>(timePassed / 100000.0);
        }
        m_LastFrameTime = currentFrameTime;
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

    QSharedPointer<ICustomMaterialSystem> GetCustomMaterialSystem(QSharedPointer<IQDemonRenderContext> inContext) override
    {
        m_Context = inContext;

        // check for fast blits
        QSharedPointer<QDemonRenderContext> theContext = m_Context->GetRenderContext();
        m_UseFastBlits = theContext->GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::FastBlits);

        return this->sharedFromThis();
    }
};
}

QSharedPointer<ICustomMaterialSystemCore> ICustomMaterialSystemCore::CreateCustomMaterialSystemCore(QSharedPointer<IQDemonRenderContextCore> ctx)
{
    return QSharedPointer<SMaterialSystem>(new SMaterialSystem(ctx));
}

QT_END_NAMESPACE

