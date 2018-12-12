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

#include <QtDemonRuntimeRender/qdemonrendererimpl.h>
#include <QtDemonRuntimeRender/qdemonrenderlight.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRuntimeRender/qdemonrendershadercache.h>
#include <QtDemonRuntimeRender/qdemonrenderdynamicobjectsystem.h>
#include <QtDemonRuntimeRender/qdemonrendershadercodegeneratorv2.h>
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterialshadergenerator.h>
#include <QtDemonRuntimeRender/qdemonvertexpipelineimpl.h>

// This adds support for the depth buffers in the shader so we can do depth
// texture-based effects.
#define QDEMON_RENDER_SUPPORT_DEPTH_TEXTURE 1

QT_BEGIN_NAMESPACE

void STextShader::Render(QDemonRenderTexture2D &inTexture,
                         const STextScaleAndOffset &inScaleAndOffset, const QVector4D &inTextColor,
                         const QMatrix4x4 &inMVP, const QVector2D &inCameraVec,
                         QDemonRenderContext &inRenderContext,
                         QDemonRenderInputAssembler &inInputAssemblerBuffer, quint32 count,
                         const STextTextureDetails &inTextTextureDetails,
                         const QVector3D &inBackgroundColor)
{
    inRenderContext.SetCullingEnabled(false);
    inRenderContext.SetActiveShader(&m_Shader);
    m_MVP.Set(inMVP);
    m_Sampler.Set(&inTexture);
    m_TextColor.Set(inTextColor);
    m_Dimensions.Set(QVector4D(inScaleAndOffset.m_TextScale.x, inScaleAndOffset.m_TextScale.y,
                               inScaleAndOffset.m_TextOffset.x, inScaleAndOffset.m_TextOffset.y));
    m_CameraProperties.Set(inCameraVec);
    STextureDetails theTextureDetails = inTexture.GetTextureDetails();
    float theWidthScale =
            (float)inTextTextureDetails.m_TextWidth / (float)theTextureDetails.m_Width;
    float theHeightScale =
            (float)inTextTextureDetails.m_TextHeight / (float)theTextureDetails.m_Height;
    m_BackgroundColor.Set(inBackgroundColor);

    m_TextDimensions.Set(
                QVector3D(theWidthScale, theHeightScale, inTextTextureDetails.m_FlipY ? 1.0f : 0.0f));
    inRenderContext.SetInputAssembler(&inInputAssemblerBuffer);
    inRenderContext.Draw(QDemonRenderDrawMode::Triangles, count, 0);
}

void STextShader::RenderPath(QDemonRenderPathFontItem &inPathFontItem,
                             QDemonRenderPathFontSpecification &inPathFontSpec,
                             const STextScaleAndOffset &inScaleAndOffset,
                             const QVector4D &inTextColor, const QMatrix4x4 &inViewProjection,
                             const QMatrix4x4 &inModel, const QVector2D &,
                             QDemonRenderContext &inRenderContext,
                             const STextTextureDetails &inTextTextureDetails,
                             const QVector3D &inBackgroundColor)
{
    QDemonRenderBoolOp::Enum theDepthFunction = inRenderContext.GetDepthFunction();
    bool isDepthEnabled = inRenderContext.IsDepthTestEnabled();
    bool isStencilEnabled = inRenderContext.IsStencilTestEnabled();
    bool isDepthWriteEnabled = inRenderContext.IsDepthWriteEnabled();
    QDemonRenderStencilFunctionArgument theArg(QDemonRenderBoolOp::NotEqual, 0,
                                               0xFF);
    QDemonRenderStencilOperationArgument theOpArg(QDemonRenderStencilOp::Keep,
                                                  QDemonRenderStencilOp::Keep,
                                                  QDemonRenderStencilOp::Zero);
    QSharedPointer<QDemonRenderDepthStencilState> depthStencilState =
            inRenderContext.CreateDepthStencilState(isDepthEnabled, isDepthWriteEnabled,
                                                    theDepthFunction, false, theArg, theArg,
                                                    theOpArg, theOpArg);

    inRenderContext.SetActiveShader(nullptr);
    inRenderContext.SetCullingEnabled(false);

    inRenderContext.SetDepthStencilState(depthStencilState);

    // setup transform
    QMatrix4x4 offsetMatrix = QMatrix4x4::createIdentity();
    offsetMatrix.setPosition(QVector3D(
                                 inScaleAndOffset.m_TextOffset.x - (float)inTextTextureDetails.m_TextWidth / 2.0f,
                                 inScaleAndOffset.m_TextOffset.y - (float)inTextTextureDetails.m_TextHeight / 2.0f,
                                 0.0));

    QMatrix4x4 pathMatrix = inPathFontItem.GetTransform();

    inRenderContext.SetPathProjectionMatrix(inViewProjection);
    inRenderContext.SetPathModelViewMatrix(inModel * offsetMatrix * pathMatrix);

    // first pass
    inPathFontSpec.StencilFillPathInstanced(inPathFontItem);

    // second pass
    inRenderContext.SetActiveProgramPipeline(m_ProgramPipeline);
    m_TextColor.Set(inTextColor);
    m_BackgroundColor.Set(inBackgroundColor);

    inRenderContext.SetStencilTestEnabled(true);
    inPathFontSpec.CoverFillPathInstanced(inPathFontItem);

    inRenderContext.SetStencilTestEnabled(isStencilEnabled);
    inRenderContext.SetDepthFunction(theDepthFunction);

    inRenderContext.SetActiveProgramPipeline(nullptr);
}

void STextShader::Render2D(QDemonRenderTexture2D &inTexture, const QVector4D &inTextColor,
                           const QMatrix4x4 &inMVP, QDemonRenderContext &inRenderContext,
                           QDemonRenderInputAssembler &inInputAssemblerBuffer, quint32 count,
                           QVector2D inVertexOffsets)
{
    // inRenderContext.SetCullingEnabled( false );
    inRenderContext.SetBlendingEnabled(true);
    inRenderContext.SetDepthWriteEnabled(false);
    inRenderContext.SetDepthTestEnabled(false);

    inRenderContext.SetActiveShader(&m_Shader);

    QDemonRenderBlendFunctionArgument blendFunc(
                QDemonRenderSrcBlendFunc::SrcAlpha, QDemonRenderDstBlendFunc::OneMinusSrcAlpha,
                QDemonRenderSrcBlendFunc::One, QDemonRenderDstBlendFunc::One);
    QDemonRenderBlendEquationArgument blendEqu(QDemonRenderBlendEquation::Add,
                                               QDemonRenderBlendEquation::Add);

    inRenderContext.SetBlendFunction(blendFunc);
    inRenderContext.SetBlendEquation(blendEqu);

    m_MVP.Set(inMVP);
    m_Sampler.Set(&inTexture);
    m_TextColor.Set(inTextColor);
    m_VertexOffsets.Set(inVertexOffsets);

    inRenderContext.SetInputAssembler(&inInputAssemblerBuffer);
    inRenderContext.Draw(QDemonRenderDrawMode::Triangles, count, 0);
}

static inline void AddVertexDepth(SShaderVertexCodeGenerator &vertexShader)
{
    // near plane, far plane
    vertexShader.AddInclude("viewProperties.glsllib");
    vertexShader.AddVarying("vertex_depth", "float");
    // the w coordinate is the unormalized distance to the object from the camera
    // We want the normalized distance, with 0 representing the far plane and 1 representing
    // the near plane, of the object in the vertex depth variable.

    vertexShader << "\tvertex_depth = calculateVertexDepth( camera_properties, gl_Position );"
                 << Endl;
}

// Helper implements the vertex pipeline for mesh subsets when bound to the default material.
// Should be completely possible to use for custom materials with a bit of refactoring.
struct SSubsetMaterialVertexPipeline : public SVertexPipelineImpl
{
    QDemonRendererImpl &m_Renderer;
    SSubsetRenderable &m_Renderable;
    TessModeValues::Enum m_TessMode;

    SSubsetMaterialVertexPipeline(QDemonRendererImpl &renderer, SSubsetRenderable &renderable,
                                  bool inWireframeRequested)
        : SVertexPipelineImpl(renderer.GetDemonContext().GetAllocator(),
                              renderer.GetDemonContext().GetDefaultMaterialShaderGenerator(),
                              renderer.GetDemonContext().GetShaderProgramGenerator(),
                              renderer.GetDemonContext().GetStringTable(), false)
        , m_Renderer(renderer)
        , m_Renderable(renderable)
        , m_TessMode(TessModeValues::NoTess)
    {
        if (m_Renderer.GetContext().IsTessellationSupported()) {
            m_TessMode = renderable.m_TessellationMode;
        }

        if (m_Renderer.GetContext().IsGeometryStageSupported()
                && m_TessMode != TessModeValues::NoTess)
            m_Wireframe = inWireframeRequested;
    }

    void InitializeTessControlShader()
    {
        if (m_TessMode == TessModeValues::NoTess
                || ProgramGenerator().GetStage(ShaderGeneratorStages::TessControl) == nullptr)
            return;

        IShaderStageGenerator &tessCtrlShader(
                    *ProgramGenerator().GetStage(ShaderGeneratorStages::TessControl));

        tessCtrlShader.AddUniform("tessLevelInner", "float");
        tessCtrlShader.AddUniform("tessLevelOuter", "float");

        SetupTessIncludes(ShaderGeneratorStages::TessControl, m_TessMode);

        tessCtrlShader.Append("void main() {\n");

        tessCtrlShader.Append("\tctWorldPos[0] = varWorldPos[0];");
        tessCtrlShader.Append("\tctWorldPos[1] = varWorldPos[1];");
        tessCtrlShader.Append("\tctWorldPos[2] = varWorldPos[2];");

        if (m_TessMode == TessModeValues::TessPhong
                || m_TessMode == TessModeValues::TessNPatch) {
            tessCtrlShader.Append("\tctNorm[0] = varObjectNormal[0];");
            tessCtrlShader.Append("\tctNorm[1] = varObjectNormal[1];");
            tessCtrlShader.Append("\tctNorm[2] = varObjectNormal[2];");
        }
        if (m_TessMode == TessModeValues::TessNPatch) {
            tessCtrlShader.Append("\tctTangent[0] = varTangent[0];");
            tessCtrlShader.Append("\tctTangent[1] = varTangent[1];");
            tessCtrlShader.Append("\tctTangent[2] = varTangent[2];");
        }

        tessCtrlShader.Append(
                    "\tgl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;");
        tessCtrlShader.Append("\ttessShader( tessLevelOuter, tessLevelInner);\n");
    }
    void InitializeTessEvaluationShader()
    {
        if (m_TessMode == TessModeValues::NoTess
                || ProgramGenerator().GetStage(ShaderGeneratorStages::TessEval) == nullptr)
            return;

        IShaderStageGenerator &tessEvalShader(
                    *ProgramGenerator().GetStage(ShaderGeneratorStages::TessEval));

        SetupTessIncludes(ShaderGeneratorStages::TessEval, m_TessMode);

        if (m_TessMode == TessModeValues::TessLinear)
            m_Renderer.GetDemonContext()
                    .GetDefaultMaterialShaderGenerator()
                    .AddDisplacementImageUniforms(tessEvalShader, m_DisplacementIdx,
                                                  m_DisplacementImage);

        tessEvalShader.AddUniform("model_view_projection", "mat4");
        tessEvalShader.AddUniform("normal_matrix", "mat3");

        tessEvalShader.Append("void main() {");

        if (m_TessMode == TessModeValues::TessNPatch) {
            tessEvalShader.Append("\tctNorm[0] = varObjectNormalTC[0];");
            tessEvalShader.Append("\tctNorm[1] = varObjectNormalTC[1];");
            tessEvalShader.Append("\tctNorm[2] = varObjectNormalTC[2];");

            tessEvalShader.Append("\tctTangent[0] = varTangentTC[0];");
            tessEvalShader.Append("\tctTangent[1] = varTangentTC[1];");
            tessEvalShader.Append("\tctTangent[2] = varTangentTC[2];");
        }

        tessEvalShader.Append("\tvec4 pos = tessShader( );\n");
    }

    void FinalizeTessControlShader()
    {
        IShaderStageGenerator &tessCtrlShader(
                    *ProgramGenerator().GetStage(ShaderGeneratorStages::TessControl));
        // add varyings we must pass through
        typedef TStrTableStrMap::const_iterator TParamIter;
        for (TParamIter iter = m_InterpolationParameters.begin(),
             end = m_InterpolationParameters.end();
             iter != end; ++iter) {
            tessCtrlShader << "\t" << iter->first.c_str()
                           << "TC[gl_InvocationID] = " << iter->first.c_str()
                           << "[gl_InvocationID];\n";
        }
    }

    void FinalizeTessEvaluationShader()
    {
        IShaderStageGenerator &tessEvalShader(
                    *ProgramGenerator().GetStage(ShaderGeneratorStages::TessEval));

        QString outExt("");
        if (ProgramGenerator().GetEnabledStages() & ShaderGeneratorStages::Geometry)
            outExt = "TE";

        // add varyings we must pass through
        typedef TStrTableStrMap::const_iterator TParamIter;
        if (m_TessMode == TessModeValues::TessNPatch) {
            for (TParamIter iter = m_InterpolationParameters.begin(),
                 end = m_InterpolationParameters.end();
                 iter != end; ++iter) {
                tessEvalShader << "\t" << iter->first.c_str() << outExt.c_str()
                               << " = gl_TessCoord.z * " << iter->first.c_str() << "TC[0] + ";
                tessEvalShader << "gl_TessCoord.x * " << iter->first.c_str() << "TC[1] + ";
                tessEvalShader << "gl_TessCoord.y * " << iter->first.c_str() << "TC[2];\n";
            }

            // transform the normal
            if (m_GenerationFlags & GenerationFlagValues::WorldNormal)
                tessEvalShader << "\n\tvarNormal" << outExt.c_str()
                               << " = normalize(normal_matrix * teNorm);\n";
            // transform the tangent
            if (m_GenerationFlags & GenerationFlagValues::TangentBinormal) {
                tessEvalShader << "\n\tvarTangent" << outExt.c_str()
                               << " = normalize(normal_matrix * teTangent);\n";
                // transform the binormal
                tessEvalShader << "\n\tvarBinormal" << outExt.c_str()
                               << " = normalize(normal_matrix * teBinormal);\n";
            }
        } else {
            for (TParamIter iter = m_InterpolationParameters.begin(),
                 end = m_InterpolationParameters.end();
                 iter != end; ++iter) {
                tessEvalShader << "\t" << iter->first.c_str() << outExt.c_str()
                               << " = gl_TessCoord.x * " << iter->first.c_str() << "TC[0] + ";
                tessEvalShader << "gl_TessCoord.y * " << iter->first.c_str() << "TC[1] + ";
                tessEvalShader << "gl_TessCoord.z * " << iter->first.c_str() << "TC[2];\n";
            }

            // displacement mapping makes only sense with linear tessellation
            if (m_TessMode == TessModeValues::TessLinear && m_DisplacementImage) {
                IDefaultMaterialShaderGenerator::SImageVariableNames theNames =
                        m_Renderer.GetDemonContext()
                        .GetDefaultMaterialShaderGenerator()
                        .GetImageVariableNames(m_DisplacementIdx);
                tessEvalShader << "\tpos.xyz = defaultMaterialFileDisplacementTexture( "
                               << theNames.m_ImageSampler << ", displaceAmount, "
                               << theNames.m_ImageFragCoords << outExt.c_str();
                tessEvalShader << ", varObjectNormal" << outExt.c_str() << ", pos.xyz );"
                               << Endl;
                tessEvalShader << "\tvarWorldPos" << outExt.c_str()
                               << "= (model_matrix * pos).xyz;" << Endl;
                tessEvalShader << "\tvarViewVector" << outExt.c_str()
                               << "= normalize(camera_position - "
                               << "varWorldPos" << outExt.c_str() << ");" << Endl;
            }

            // transform the normal
            tessEvalShader << "\n\tvarNormal" << outExt.c_str()
                           << " = normalize(normal_matrix * varObjectNormal" << outExt.c_str()
                           << ");\n";
        }

        tessEvalShader.Append("\tgl_Position = model_view_projection * pos;\n");
    }

    void BeginVertexGeneration(quint32 displacementImageIdx,
                               SRenderableImage *displacementImage) override
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
        ProgramGenerator().BeginProgram(theStages);
        if (m_TessMode != TessModeValues::NoTess) {
            InitializeTessControlShader();
            InitializeTessEvaluationShader();
        }
        if (m_Wireframe) {
            InitializeWireframeGeometryShader();
        }
        // Open up each stage.
        IShaderStageGenerator &vertexShader(Vertex());
        vertexShader.AddIncoming("attr_pos", "vec3");
        vertexShader << "void main()" << Endl << "{" << Endl;
        vertexShader << "\tvec3 uTransform;" << Endl;
        vertexShader << "\tvec3 vTransform;" << Endl;

        if (displacementImage) {
            GenerateUVCoords();
            MaterialGenerator().GenerateImageUVCoordinates(*this, displacementImageIdx, 0,
                                                           *displacementImage);
            if (!HasTessellation()) {
                vertexShader.AddUniform("displaceAmount", "float");
                // we create the world position setup here
                // because it will be replaced with the displaced position
                SetCode(GenerationFlagValues::WorldPosition);
                vertexShader.AddUniform("model_matrix", "mat4");

                vertexShader.AddInclude("defaultMaterialFileDisplacementTexture.glsllib");
                IDefaultMaterialShaderGenerator::SImageVariableNames theVarNames =
                        MaterialGenerator().GetImageVariableNames(displacementImageIdx);

                vertexShader.AddUniform(theVarNames.m_ImageSampler, "sampler2D");

                vertexShader
                        << "\tvec3 displacedPos = defaultMaterialFileDisplacementTexture( "
                        << theVarNames.m_ImageSampler << ", displaceAmount, "
                        << theVarNames.m_ImageFragCoords << ", attr_norm, attr_pos );" << Endl;
                AddInterpolationParameter("varWorldPos", "vec3");
                vertexShader.Append("\tvec3 local_model_world_position = (model_matrix * "
                                    "vec4(displacedPos, 1.0)).xyz;");
                AssignOutput("varWorldPos", "local_model_world_position");
            }
        }
        // for tessellation we pass on the position in object coordinates
        // Also note that gl_Position is written in the tess eval shader
        if (HasTessellation())
            vertexShader.Append("\tgl_Position = vec4(attr_pos, 1.0);");
        else {
            vertexShader.AddUniform("model_view_projection", "mat4");
            if (displacementImage)
                vertexShader.Append(
                            "\tgl_Position = model_view_projection * vec4(displacedPos, 1.0);");
            else
                vertexShader.Append(
                            "\tgl_Position = model_view_projection * vec4(attr_pos, 1.0);");
        }

        if (HasTessellation()) {
            GenerateWorldPosition();
            GenerateWorldNormal();
            GenerateObjectNormal();
            GenerateVarTangentAndBinormal();
        }
    }

    void BeginFragmentGeneration() override
    {
        Fragment().AddUniform("material_diffuse", "vec4");
        Fragment() << "void main()" << Endl << "{" << Endl;
        // We do not pass object opacity through the pipeline.
        Fragment() << "\tfloat object_opacity = material_diffuse.a;" << Endl;
    }

    void AssignOutput(const char *inVarName, const char *inVarValue) override
    {
        Vertex() << "\t" << inVarName << " = " << inVarValue << ";\n";
    }
    void DoGenerateUVCoords(quint32 inUVSet = 0) override
    {
        Q_ASSERT(inUVSet == 0 || inUVSet == 1);

        if (inUVSet == 0) {
            Vertex().AddIncoming("attr_uv0", "vec2");
            Vertex() << "\tvarTexCoord0 = attr_uv0;" << Endl;
        } else if (inUVSet == 1) {
            Vertex().AddIncoming("attr_uv1", "vec2");
            Vertex() << "\tvarTexCoord1 = attr_uv1;" << Endl;
        }
    }

    // fragment shader expects varying vertex normal
    // lighting in vertex pipeline expects world_normal
    void DoGenerateWorldNormal() override
    {
        IShaderStageGenerator &vertexGenerator(Vertex());
        vertexGenerator.AddIncoming("attr_norm", "vec3");
        vertexGenerator.AddUniform("normal_matrix", "mat3");
        if (HasTessellation() == false) {
            vertexGenerator.Append(
                        "\tvec3 world_normal = normalize(normal_matrix * attr_norm).xyz;");
            vertexGenerator.Append("\tvarNormal = world_normal;");
        }
    }
    void DoGenerateObjectNormal() override
    {
        AddInterpolationParameter("varObjectNormal", "vec3");
        Vertex().Append("\tvarObjectNormal = attr_norm;");
    }
    void DoGenerateWorldPosition() override
    {
        Vertex().Append(
                    "\tvec3 local_model_world_position = (model_matrix * vec4(attr_pos, 1.0)).xyz;");
        AssignOutput("varWorldPos", "local_model_world_position");
    }

    void DoGenerateVarTangentAndBinormal() override
    {
        Vertex().AddIncoming("attr_textan", "vec3");
        Vertex().AddIncoming("attr_binormal", "vec3");

        bool hasNPatchTessellation = m_TessMode == TessModeValues::TessNPatch;

        if (!hasNPatchTessellation) {
            Vertex() << "\tvarTangent = normal_matrix * attr_textan;" << Endl
                     << "\tvarBinormal = normal_matrix * attr_binormal;" << Endl;
        } else {
            Vertex() << "\tvarTangent = attr_textan;" << Endl
                     << "\tvarBinormal = attr_binormal;" << Endl;
        }
    }

    void DoGenerateVertexColor() override
    {
        Vertex().AddIncoming("attr_color", "vec3");
        Vertex().Append("\tvarColor = attr_color;");
    }

    void EndVertexGeneration() override
    {

        if (HasTessellation()) {
            // finalize tess control shader
            FinalizeTessControlShader();
            // finalize tess evaluation shader
            FinalizeTessEvaluationShader();

            TessControl().Append("}");
            TessEval().Append("}");
        }
        if (m_Wireframe) {
            // finalize geometry shader
            FinalizeWireframeGeometryShader();
            Geometry().Append("}");
        }
        Vertex().Append("}");
    }

    void EndFragmentGeneration() override { Fragment().Append("}"); }

    void AddInterpolationParameter(const char *inName, const char *inType) override
    {
        m_InterpolationParameters.insert(Str(inName), Str(inType));
        Vertex().AddOutgoing(inName, inType);
        Fragment().AddIncoming(inName, inType);
        if (HasTessellation()) {
            QString nameBuilder(inName);
            nameBuilder.append("TC");
            TessControl().AddOutgoing(nameBuilder.c_str(), inType);

            nameBuilder.assign(inName);
            if (ProgramGenerator().GetEnabledStages() & ShaderGeneratorStages::Geometry) {
                nameBuilder.append("TE");
                Geometry().AddOutgoing(inName, inType);
            }
            TessEval().AddOutgoing(nameBuilder.c_str(), inType);
        }
    }

    IShaderStageGenerator &ActiveStage() override { return Vertex(); }
};

QDemonRenderShaderProgram *QDemonRendererImpl::GenerateShader(SSubsetRenderable &inRenderable,
                                                             TShaderFeatureSet inFeatureSet)
{
    // build a string that allows us to print out the shader we are generating to the log.
    // This is time consuming but I feel like it doesn't happen all that often and is very
    // useful to users
    // looking at the log file.
    QLatin1String logPrefix("mesh subset pipeline-- ");

    m_GeneratedShaderString.clear();
    m_GeneratedShaderString.assign(logPrefix.data());

    SShaderDefaultMaterialKey theKey(inRenderable.m_ShaderDescription);
    theKey.ToString(m_GeneratedShaderString, m_DefaultMaterialShaderKeyProperties);
    IShaderCache &theCache = m_demonContext.GetShaderCache();
    QString theCacheKey =QString::fromLocal8Bit(m_GeneratedShaderString.c_str());
    QDemonRenderShaderProgram *cachedProgram = theCache.GetProgram(theCacheKey, inFeatureSet);
    if (cachedProgram)
        return cachedProgram;

    SSubsetMaterialVertexPipeline pipeline(
                *this, inRenderable,
                m_DefaultMaterialShaderKeyProperties.m_WireframeMode.GetValue(theKey));
    return m_demonContext.GetDefaultMaterialShaderGenerator().GenerateShader(
                inRenderable.m_Material, inRenderable.m_ShaderDescription, pipeline, inFeatureSet,
                m_CurrentLayer->m_Lights, inRenderable.m_FirstImage,
                inRenderable.m_RenderableFlags.HasTransparency(),
                logPrefix.data());
}

// --------------  Special cases for shadows  -------------------

SRenderableDepthPrepassShader *
QDemonRendererImpl::GetParaboloidDepthShader(TessModeValues::Enum inTessMode)
{
    if (!m_demonContext.GetRenderContext().IsTessellationSupported()
            || inTessMode == TessModeValues::NoTess) {
        return GetParaboloidDepthNoTessShader();
    } else if (inTessMode == TessModeValues::TessLinear) {
        return GetParaboloidDepthTessLinearShader();
    } else if (inTessMode == TessModeValues::TessPhong) {
        return GetParaboloidDepthTessPhongShader();
    } else if (inTessMode == TessModeValues::TessNPatch) {
        return GetParaboloidDepthTessNPatchShader();
    }

    return GetParaboloidDepthNoTessShader();
}

SRenderableDepthPrepassShader *QDemonRendererImpl::GetParaboloidDepthNoTessShader()
{
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> &theDepthShader =
            m_ParaboloidDepthShader;

    if (theDepthShader.hasValue() == false) {
        TStrType name;
        name.assign("paraboloid depth shader");

        IShaderCache &theCache = m_demonContext.GetShaderCache();
        QString theCacheKey = QString::fromLocal8Bit(name.c_str());
        QDemonRenderShaderProgram *depthShaderProgram =
                theCache.GetProgram(theCacheKey, TShaderFeatureSet());
        if (!depthShaderProgram) {
            GetProgramGenerator().BeginProgram();
            IShaderStageGenerator &vertexShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
            IShaderStageGenerator &fragmentShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));
            IShaderProgramGenerator::OutputParaboloidDepthVertex(vertexShader);
            IShaderProgramGenerator::OutputParaboloidDepthFragment(fragmentShader);
        }

        depthShaderProgram = GetProgramGenerator().CompileGeneratedShader(
                    name.c_str(), SShaderCacheProgramFlags(), TShaderFeatureSet());

        if (depthShaderProgram) {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>(
                        new SRenderableDepthPrepassShader(*depthShaderProgram, GetContext()));
        } else {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>();
        }
    }

    return theDepthShader.getValue();
}

SRenderableDepthPrepassShader *QDemonRendererImpl::GetParaboloidDepthTessLinearShader()
{
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> &theDepthShader =
            m_ParaboloidDepthTessLinearShader;

    if (theDepthShader.hasValue() == false) {
        TStrType name;
        name.assign("paraboloid depth tess linear shader");

        IShaderCache &theCache = m_demonContext.GetShaderCache();
        QString theCacheKey = QString::fromLocal8Bit(name.c_str());
        QDemonRenderShaderProgram *depthShaderProgram =
                theCache.GetProgram(theCacheKey, TShaderFeatureSet());
        if (!depthShaderProgram) {
            GetProgramGenerator().BeginProgram(TShaderGeneratorStageFlags(
                                                   ShaderGeneratorStages::Vertex | ShaderGeneratorStages::TessControl
                                                   | ShaderGeneratorStages::TessEval | ShaderGeneratorStages::Fragment));
            IShaderStageGenerator &vertexShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
            IShaderStageGenerator &tessCtrlShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessControl));
            IShaderStageGenerator &tessEvalShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessEval));
            IShaderStageGenerator &fragmentShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));

            vertexShader.AddIncoming("attr_pos", "vec3");
            // vertexShader.AddOutgoing("world_pos", "vec4");
            vertexShader.AddUniform("model_view_projection", "mat4");

            vertexShader.Append("void main() {");
            vertexShader.Append("\tgl_Position = vec4(attr_pos, 1.0);");
            // vertexShader.Append("\tworld_pos = attr_pos;");
            vertexShader.Append("}");

            tessCtrlShader.AddInclude("tessellationLinear.glsllib");
            tessCtrlShader.AddUniform("tessLevelInner", "float");
            tessCtrlShader.AddUniform("tessLevelOuter", "float");
            // tessCtrlShader.AddOutgoing( "outUVTC", "vec2" );
            // tessCtrlShader.AddOutgoing( "outNormalTC", "vec3" );
            tessCtrlShader.Append("void main() {\n");
            // tessCtrlShader.Append("\tctWorldPos[0] = outWorldPos[0];");
            // tessCtrlShader.Append("\tctWorldPos[1] = outWorldPos[1];");
            // tessCtrlShader.Append("\tctWorldPos[2] = outWorldPos[2];");
            tessCtrlShader.Append(
                        "\tgl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;");
            tessCtrlShader.Append("\ttessShader( tessLevelOuter, tessLevelInner);\n");
            tessCtrlShader.Append("}");

            tessEvalShader.AddInclude("tessellationLinear.glsllib");
            tessEvalShader.AddUniform("model_view_projection", "mat4");
            tessEvalShader.AddOutgoing("world_pos", "vec4");
            tessEvalShader.Append("void main() {");
            tessEvalShader.Append("\tvec4 pos = tessShader( );\n");
            IShaderProgramGenerator::OutputParaboloidDepthTessEval(tessEvalShader);
            tessEvalShader.Append("}");

            IShaderProgramGenerator::OutputParaboloidDepthFragment(fragmentShader);
        }
        depthShaderProgram = GetProgramGenerator().CompileGeneratedShader(
                    name.c_str(), SShaderCacheProgramFlags(), TShaderFeatureSet());

        if (depthShaderProgram) {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>(
                        new SRenderableDepthPrepassShader(*depthShaderProgram, GetContext()));
        } else {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>();
        }
    }

    return theDepthShader.getValue();
}

SRenderableDepthPrepassShader *QDemonRendererImpl::GetParaboloidDepthTessPhongShader()
{
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> &theDepthShader =
            m_ParaboloidDepthTessPhongShader;

    if (theDepthShader.hasValue() == false) {
        TStrType name;
        name.assign("paraboloid depth tess phong shader");

        IShaderCache &theCache = m_demonContext.GetShaderCache();
        QString theCacheKey = QString::fromLocal8Bit(name.c_str());
        QDemonRenderShaderProgram *depthShaderProgram =
                theCache.GetProgram(theCacheKey, TShaderFeatureSet());
        if (!depthShaderProgram) {
            GetProgramGenerator().BeginProgram(TShaderGeneratorStageFlags(
                                                   ShaderGeneratorStages::Vertex | ShaderGeneratorStages::TessControl
                                                   | ShaderGeneratorStages::TessEval | ShaderGeneratorStages::Fragment));
            IShaderStageGenerator &vertexShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
            IShaderStageGenerator &tessCtrlShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessControl));
            IShaderStageGenerator &tessEvalShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessEval));
            IShaderStageGenerator &fragmentShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));

            vertexShader.AddIncoming("attr_pos", "vec3");
            // vertexShader.AddOutgoing("world_pos", "vec4");
            vertexShader.AddUniform("model_view_projection", "mat4");

            vertexShader.Append("void main() {");
            vertexShader.Append("\tgl_Position = vec4(attr_pos, 1.0);");
            // vertexShader.Append("\tworld_pos = attr_pos;");
            vertexShader.Append("}");

            tessCtrlShader.AddInclude("tessellationPhong.glsllib");
            tessCtrlShader.AddUniform("tessLevelInner", "float");
            tessCtrlShader.AddUniform("tessLevelOuter", "float");
            // tessCtrlShader.AddOutgoing( "outUVTC", "vec2" );
            // tessCtrlShader.AddOutgoing( "outNormalTC", "vec3" );
            tessCtrlShader.Append("void main() {\n");
            // tessCtrlShader.Append("\tctWorldPos[0] = outWorldPos[0];");
            // tessCtrlShader.Append("\tctWorldPos[1] = outWorldPos[1];");
            // tessCtrlShader.Append("\tctWorldPos[2] = outWorldPos[2];");
            tessCtrlShader.Append(
                        "\tgl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;");
            tessCtrlShader.Append("\ttessShader( tessLevelOuter, tessLevelInner);\n");
            tessCtrlShader.Append("}");

            tessEvalShader.AddInclude("tessellationPhong.glsllib");
            tessEvalShader.AddUniform("model_view_projection", "mat4");
            tessEvalShader.AddOutgoing("world_pos", "vec4");
            tessEvalShader.Append("void main() {");
            tessEvalShader.Append("\tvec4 pos = tessShader( );\n");
            IShaderProgramGenerator::OutputParaboloidDepthTessEval(tessEvalShader);
            tessEvalShader.Append("}");

            IShaderProgramGenerator::OutputParaboloidDepthFragment(fragmentShader);
        }
        depthShaderProgram = GetProgramGenerator().CompileGeneratedShader(
                    name.c_str(), SShaderCacheProgramFlags(), TShaderFeatureSet());

        if (depthShaderProgram) {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>(
                        new SRenderableDepthPrepassShader(*depthShaderProgram, GetContext()));
        } else {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>();
        }
    }

    return theDepthShader.getValue();
}

SRenderableDepthPrepassShader *QDemonRendererImpl::GetParaboloidDepthTessNPatchShader()
{
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> &theDepthShader =
            m_ParaboloidDepthTessNPatchShader;

    if (theDepthShader.hasValue() == false) {
        TStrType name;
        name.assign("paraboloid depth tess NPatch shader");

        IShaderCache &theCache = m_demonContext.GetShaderCache();
        QString theCacheKey = QString::fromLocal8Bit(name.c_str());
        QDemonRenderShaderProgram *depthShaderProgram =
                theCache.GetProgram(theCacheKey, TShaderFeatureSet());
        if (!depthShaderProgram) {
            GetProgramGenerator().BeginProgram(TShaderGeneratorStageFlags(
                                                   ShaderGeneratorStages::Vertex | ShaderGeneratorStages::TessControl
                                                   | ShaderGeneratorStages::TessEval | ShaderGeneratorStages::Fragment));
            IShaderStageGenerator &vertexShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
            IShaderStageGenerator &tessCtrlShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessControl));
            IShaderStageGenerator &tessEvalShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessEval));
            IShaderStageGenerator &fragmentShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));

            vertexShader.AddIncoming("attr_pos", "vec3");
            // vertexShader.AddOutgoing("world_pos", "vec4");
            vertexShader.AddUniform("model_view_projection", "mat4");

            vertexShader.Append("void main() {");
            vertexShader.Append("\tgl_Position = vec4(attr_pos, 1.0);");
            // vertexShader.Append("\tworld_pos = attr_pos;");
            vertexShader.Append("}");

            tessCtrlShader.AddInclude("tessellationNPatch.glsllib");
            tessCtrlShader.AddUniform("tessLevelInner", "float");
            tessCtrlShader.AddUniform("tessLevelOuter", "float");
            // tessCtrlShader.AddOutgoing( "outUVTC", "vec2" );
            // tessCtrlShader.AddOutgoing( "outNormalTC", "vec3" );
            tessCtrlShader.Append("void main() {\n");
            // tessCtrlShader.Append("\tctWorldPos[0] = outWorldPos[0];");
            // tessCtrlShader.Append("\tctWorldPos[1] = outWorldPos[1];");
            // tessCtrlShader.Append("\tctWorldPos[2] = outWorldPos[2];");
            tessCtrlShader.Append(
                        "\tgl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;");
            tessCtrlShader.Append("\ttessShader( tessLevelOuter, tessLevelInner);\n");
            tessCtrlShader.Append("}");

            tessEvalShader.AddInclude("tessellationNPatch.glsllib");
            tessEvalShader.AddUniform("model_view_projection", "mat4");
            tessEvalShader.AddOutgoing("world_pos", "vec4");
            tessEvalShader.Append("void main() {");
            tessEvalShader.Append("\tvec4 pos = tessShader( );\n");
            IShaderProgramGenerator::OutputParaboloidDepthTessEval(tessEvalShader);
            tessEvalShader.Append("}");

            IShaderProgramGenerator::OutputParaboloidDepthFragment(fragmentShader);
        }
        depthShaderProgram = GetProgramGenerator().CompileGeneratedShader(
                    name.c_str(), SShaderCacheProgramFlags(), TShaderFeatureSet());

        if (depthShaderProgram) {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>(
                        new SRenderableDepthPrepassShader(*depthShaderProgram, GetContext()));
        } else {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>();
        }
    }

    return theDepthShader.getValue();
}

SRenderableDepthPrepassShader *
QDemonRendererImpl::GetCubeShadowDepthShader(TessModeValues::Enum inTessMode)
{
    if (!m_demonContext.GetRenderContext().IsTessellationSupported()
            || inTessMode == TessModeValues::NoTess) {
        return GetCubeDepthNoTessShader();
    } else if (inTessMode == TessModeValues::TessLinear) {
        return GetCubeDepthTessLinearShader();
    } else if (inTessMode == TessModeValues::TessPhong) {
        return GetCubeDepthTessPhongShader();
    } else if (inTessMode == TessModeValues::TessNPatch) {
        return GetCubeDepthTessNPatchShader();
    }

    return GetCubeDepthNoTessShader();
}

SRenderableDepthPrepassShader *QDemonRendererImpl::GetCubeDepthNoTessShader()
{
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> &theDepthShader =
            m_CubemapDepthShader;

    if (theDepthShader.hasValue() == false) {
        TStrType name;
        name.assign("cubemap face depth shader");

        IShaderCache &theCache = m_demonContext.GetShaderCache();
        QString theCacheKey = QString::fromLocal8Bit(name.c_str());
        QDemonRenderShaderProgram *depthShaderProgram =
                theCache.GetProgram(theCacheKey, TShaderFeatureSet());

        if (!depthShaderProgram) {
            // GetProgramGenerator().BeginProgram(
            // TShaderGeneratorStageFlags(ShaderGeneratorStages::Vertex |
            // ShaderGeneratorStages::Fragment | ShaderGeneratorStages::Geometry) );
            GetProgramGenerator().BeginProgram();
            IShaderStageGenerator &vertexShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
            IShaderStageGenerator &fragmentShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));
            // IShaderStageGenerator& geometryShader( *GetProgramGenerator().GetStage(
            // ShaderGeneratorStages::Geometry ) );

            IShaderProgramGenerator::OutputCubeFaceDepthVertex(vertexShader);
            // IShaderProgramGenerator::OutputCubeFaceDepthGeometry( geometryShader );
            IShaderProgramGenerator::OutputCubeFaceDepthFragment(fragmentShader);
        } else if (theCache.IsShaderCachePersistenceEnabled()) {
            // we load from shader cache set default shader stages
            GetProgramGenerator().BeginProgram();
        }

        depthShaderProgram = GetProgramGenerator().CompileGeneratedShader(
                    name.c_str(), SShaderCacheProgramFlags(), TShaderFeatureSet());

        if (depthShaderProgram) {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>(
                        new SRenderableDepthPrepassShader(*depthShaderProgram, GetContext()));
        } else {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>();
        }
    }

    return theDepthShader.getValue();
}

SRenderableDepthPrepassShader *QDemonRendererImpl::GetCubeDepthTessLinearShader()
{
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> &theDepthShader =
            m_CubemapDepthTessLinearShader;

    if (theDepthShader.hasValue() == false) {
        TStrType name;
        name.assign("cubemap face depth linear tess shader");

        IShaderCache &theCache = m_demonContext.GetShaderCache();
        QString theCacheKey = QString::fromLocal8Bit(name.c_str());
        QDemonRenderShaderProgram *depthShaderProgram =
                theCache.GetProgram(theCacheKey, TShaderFeatureSet());

        if (!depthShaderProgram) {
            // GetProgramGenerator().BeginProgram(
            // TShaderGeneratorStageFlags(ShaderGeneratorStages::Vertex |
            // ShaderGeneratorStages::Fragment | ShaderGeneratorStages::Geometry) );
            GetProgramGenerator().BeginProgram(TShaderGeneratorStageFlags(
                                                   ShaderGeneratorStages::Vertex | ShaderGeneratorStages::TessControl
                                                   | ShaderGeneratorStages::TessEval | ShaderGeneratorStages::Fragment));
            IShaderStageGenerator &vertexShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
            IShaderStageGenerator &fragmentShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));
            // IShaderStageGenerator& geometryShader( *GetProgramGenerator().GetStage(
            // ShaderGeneratorStages::Geometry ) );
            IShaderStageGenerator &tessCtrlShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessControl));
            IShaderStageGenerator &tessEvalShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessEval));

            vertexShader.AddIncoming("attr_pos", "vec3");
            vertexShader.Append("void main() {");
            vertexShader.Append("\tgl_Position = vec4(attr_pos, 1.0);");
            vertexShader.Append("}");

            // IShaderProgramGenerator::OutputCubeFaceDepthGeometry( geometryShader );
            IShaderProgramGenerator::OutputCubeFaceDepthFragment(fragmentShader);

            tessCtrlShader.AddInclude("tessellationLinear.glsllib");
            tessCtrlShader.AddUniform("tessLevelInner", "float");
            tessCtrlShader.AddUniform("tessLevelOuter", "float");
            tessCtrlShader.Append("void main() {\n");
            tessCtrlShader.Append(
                        "\tgl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;");
            tessCtrlShader.Append("\ttessShader( tessLevelOuter, tessLevelInner);\n");
            tessCtrlShader.Append("}");

            tessEvalShader.AddInclude("tessellationLinear.glsllib");
            tessEvalShader.AddUniform("model_view_projection", "mat4");
            tessEvalShader.AddUniform("model_matrix", "mat4");
            tessEvalShader.AddOutgoing("world_pos", "vec4");
            tessEvalShader.Append("void main() {");
            tessEvalShader.Append("\tvec4 pos = tessShader( );\n");
            tessEvalShader.Append("\tworld_pos = model_matrix * pos;");
            tessEvalShader.Append("\tworld_pos /= world_pos.w;");
            tessEvalShader.Append("\tgl_Position = model_view_projection * pos;");
            tessEvalShader.Append("}");
        }

        depthShaderProgram = GetProgramGenerator().CompileGeneratedShader(
                    name.c_str(), SShaderCacheProgramFlags(), TShaderFeatureSet());

        if (depthShaderProgram) {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>(
                        new SRenderableDepthPrepassShader(*depthShaderProgram, GetContext()));
        } else {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>();
        }
    }

    return theDepthShader.getValue();
}

SRenderableDepthPrepassShader *QDemonRendererImpl::GetCubeDepthTessPhongShader()
{
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> &theDepthShader =
            m_CubemapDepthTessPhongShader;

    if (theDepthShader.hasValue() == false) {
        TStrType name;
        name.assign("cubemap face depth phong tess shader");

        IShaderCache &theCache = m_demonContext.GetShaderCache();
        QString theCacheKey = QString::fromLocal8Bit(name.c_str());
        QDemonRenderShaderProgram *depthShaderProgram =
                theCache.GetProgram(theCacheKey, TShaderFeatureSet());

        if (!depthShaderProgram) {
            // GetProgramGenerator().BeginProgram(
            // TShaderGeneratorStageFlags(ShaderGeneratorStages::Vertex |
            // ShaderGeneratorStages::Fragment | ShaderGeneratorStages::Geometry) );
            GetProgramGenerator().BeginProgram(TShaderGeneratorStageFlags(
                                                   ShaderGeneratorStages::Vertex | ShaderGeneratorStages::TessControl
                                                   | ShaderGeneratorStages::TessEval | ShaderGeneratorStages::Fragment));
            IShaderStageGenerator &vertexShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
            IShaderStageGenerator &fragmentShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));
            // IShaderStageGenerator& geometryShader( *GetProgramGenerator().GetStage(
            // ShaderGeneratorStages::Geometry ) );
            IShaderStageGenerator &tessCtrlShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessControl));
            IShaderStageGenerator &tessEvalShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessEval));

            vertexShader.AddIncoming("attr_pos", "vec3");
            vertexShader.AddIncoming("attr_norm", "vec3");
            vertexShader.AddOutgoing("outNormal", "vec3");
            vertexShader.Append("void main() {");
            vertexShader.Append("\tgl_Position = vec4(attr_pos, 1.0);");
            vertexShader.Append("\toutNormal = attr_norm;");
            vertexShader.Append("}");

            // IShaderProgramGenerator::OutputCubeFaceDepthGeometry( geometryShader );
            IShaderProgramGenerator::OutputCubeFaceDepthFragment(fragmentShader);

            tessCtrlShader.AddInclude("tessellationPhong.glsllib");
            tessCtrlShader.AddUniform("tessLevelInner", "float");
            tessCtrlShader.AddUniform("tessLevelOuter", "float");
            tessCtrlShader.Append("void main() {\n");
            tessCtrlShader.Append("\tctNorm[0] = outNormal[0];");
            tessCtrlShader.Append("\tctNorm[1] = outNormal[1];");
            tessCtrlShader.Append("\tctNorm[2] = outNormal[2];");
            tessCtrlShader.Append(
                        "\tgl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;");
            tessCtrlShader.Append("\ttessShader( tessLevelOuter, tessLevelInner);\n");
            tessCtrlShader.Append("}");

            tessEvalShader.AddInclude("tessellationPhong.glsllib");
            tessEvalShader.AddUniform("model_view_projection", "mat4");
            tessEvalShader.AddUniform("model_matrix", "mat4");
            tessEvalShader.AddOutgoing("world_pos", "vec4");
            tessEvalShader.Append("void main() {");
            tessEvalShader.Append("\tvec4 pos = tessShader( );\n");
            tessEvalShader.Append("\tworld_pos = model_matrix * pos;");
            tessEvalShader.Append("\tworld_pos /= world_pos.w;");
            tessEvalShader.Append("\tgl_Position = model_view_projection * pos;");
            tessEvalShader.Append("}");
        }

        depthShaderProgram = GetProgramGenerator().CompileGeneratedShader(
                    name.c_str(), SShaderCacheProgramFlags(), TShaderFeatureSet());

        if (depthShaderProgram) {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>(
                        new SRenderableDepthPrepassShader(*depthShaderProgram, GetContext()));
        } else {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>();
        }
    }

    return theDepthShader.getValue();
}

SRenderableDepthPrepassShader *QDemonRendererImpl::GetCubeDepthTessNPatchShader()
{
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> &theDepthShader =
            m_CubemapDepthTessNPatchShader;

    if (theDepthShader.hasValue() == false) {
        TStrType name;
        name.assign("cubemap face depth npatch tess shader");

        IShaderCache &theCache = m_demonContext.GetShaderCache();
        QString theCacheKey = QString::fromLocal8Bit(name.c_str());
        QDemonRenderShaderProgram *depthShaderProgram =
                theCache.GetProgram(theCacheKey, TShaderFeatureSet());

        if (!depthShaderProgram) {
            // GetProgramGenerator().BeginProgram(
            // TShaderGeneratorStageFlags(ShaderGeneratorStages::Vertex |
            // ShaderGeneratorStages::Fragment | ShaderGeneratorStages::Geometry) );
            GetProgramGenerator().BeginProgram(TShaderGeneratorStageFlags(
                                                   ShaderGeneratorStages::Vertex | ShaderGeneratorStages::TessControl
                                                   | ShaderGeneratorStages::TessEval | ShaderGeneratorStages::Fragment));
            IShaderStageGenerator &vertexShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
            IShaderStageGenerator &fragmentShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));
            // IShaderStageGenerator& geometryShader( *GetProgramGenerator().GetStage(
            // ShaderGeneratorStages::Geometry ) );
            IShaderStageGenerator &tessCtrlShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessControl));
            IShaderStageGenerator &tessEvalShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessEval));

            vertexShader.AddIncoming("attr_pos", "vec3");
            vertexShader.AddIncoming("attr_norm", "vec3");
            vertexShader.AddOutgoing("outNormal", "vec3");
            vertexShader.Append("void main() {");
            vertexShader.Append("\tgl_Position = vec4(attr_pos, 1.0);");
            vertexShader.Append("\toutNormal = attr_norm;");
            vertexShader.Append("}");

            // IShaderProgramGenerator::OutputCubeFaceDepthGeometry( geometryShader );
            IShaderProgramGenerator::OutputCubeFaceDepthFragment(fragmentShader);

            tessCtrlShader.AddOutgoing("outNormalTC", "vec3");
            tessCtrlShader.AddInclude("tessellationNPatch.glsllib");
            tessCtrlShader.AddUniform("tessLevelInner", "float");
            tessCtrlShader.AddUniform("tessLevelOuter", "float");
            tessCtrlShader.Append("void main() {\n");
            tessCtrlShader.Append("\tctNorm[0] = outNormal[0];");
            tessCtrlShader.Append("\tctNorm[1] = outNormal[1];");
            tessCtrlShader.Append("\tctNorm[2] = outNormal[2];");
            tessCtrlShader.Append(
                        "\tgl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;");
            tessCtrlShader.Append("\ttessShader( tessLevelOuter, tessLevelInner);\n");
            tessCtrlShader.Append(
                        "\toutNormalTC[gl_InvocationID] = outNormal[gl_InvocationID];\n");
            tessCtrlShader.Append("}");

            tessEvalShader.AddInclude("tessellationNPatch.glsllib");
            tessEvalShader.AddUniform("model_view_projection", "mat4");
            tessEvalShader.AddUniform("model_matrix", "mat4");
            tessEvalShader.AddOutgoing("world_pos", "vec4");
            tessEvalShader.Append("void main() {");
            tessEvalShader.Append("\tctNorm[0] = outNormalTC[0];");
            tessEvalShader.Append("\tctNorm[1] = outNormalTC[1];");
            tessEvalShader.Append("\tctNorm[2] = outNormalTC[2];");
            tessEvalShader.Append("\tvec4 pos = tessShader( );\n");
            tessEvalShader.Append("\tworld_pos = model_matrix * pos;");
            tessEvalShader.Append("\tworld_pos /= world_pos.w;");
            tessEvalShader.Append("\tgl_Position = model_view_projection * pos;");
            tessEvalShader.Append("}");
        }

        depthShaderProgram = GetProgramGenerator().CompileGeneratedShader(
                    name.c_str(), SShaderCacheProgramFlags(), TShaderFeatureSet());

        if (depthShaderProgram) {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>(
                        new SRenderableDepthPrepassShader(*depthShaderProgram, GetContext()));
        } else {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>();
        }
    }

    return theDepthShader.getValue();
}

SRenderableDepthPrepassShader *
QDemonRendererImpl::GetOrthographicDepthShader(TessModeValues::Enum inTessMode)
{
    if (!m_demonContext.GetRenderContext().IsTessellationSupported()
            || inTessMode == TessModeValues::NoTess) {
        return GetOrthographicDepthNoTessShader();
    } else if (inTessMode == TessModeValues::TessLinear) {
        return GetOrthographicDepthTessLinearShader();
    } else if (inTessMode == TessModeValues::TessPhong) {
        return GetOrthographicDepthTessPhongShader();
    } else if (inTessMode == TessModeValues::TessNPatch) {
        return GetOrthographicDepthTessNPatchShader();
    }

    return GetOrthographicDepthNoTessShader();
}

SRenderableDepthPrepassShader *QDemonRendererImpl::GetOrthographicDepthNoTessShader()
{
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> &theDepthShader =
            m_OrthographicDepthShader;

    if (theDepthShader.hasValue() == false) {
        TStrType name;
        name.assign("orthographic depth shader");

        IShaderCache &theCache = m_demonContext.GetShaderCache();
        QString theCacheKey = QString::fromLocal8Bit(name.c_str());
        QDemonRenderShaderProgram *depthShaderProgram =
                theCache.GetProgram(theCacheKey, TShaderFeatureSet());
        if (!depthShaderProgram) {
            GetProgramGenerator().BeginProgram();
            IShaderStageGenerator &vertexShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
            IShaderStageGenerator &fragmentShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));
            vertexShader.AddIncoming("attr_pos", "vec3");
            vertexShader.AddUniform("model_view_projection", "mat4");
            vertexShader.AddOutgoing("outDepth", "vec3");
            vertexShader.Append("void main() {");
            vertexShader.Append(
                        "   gl_Position = model_view_projection * vec4( attr_pos, 1.0 );");
            vertexShader.Append("   outDepth.x = gl_Position.z / gl_Position.w;");
            vertexShader.Append("}");
            fragmentShader.Append("void main() {");
            fragmentShader.Append("\tfloat depth = (outDepth.x + 1.0) * 0.5;");
            fragmentShader.Append("\tfragOutput = vec4(depth);");
            fragmentShader.Append("}");
        }

        depthShaderProgram = GetProgramGenerator().CompileGeneratedShader(
                    name.c_str(), SShaderCacheProgramFlags(), TShaderFeatureSet());

        if (depthShaderProgram) {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>(
                        new SRenderableDepthPrepassShader(*depthShaderProgram, GetContext()));
        } else {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>();
        }
    }

    return theDepthShader.getValue();
}

SRenderableDepthPrepassShader *QDemonRendererImpl::GetOrthographicDepthTessLinearShader()
{
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> &theDepthShader =
            m_OrthographicDepthTessLinearShader;

    if (theDepthShader.hasValue() == false) {
        TStrType name;
        name.assign("orthographic depth tess linear shader");

        IShaderCache &theCache = m_demonContext.GetShaderCache();
        QString theCacheKey = QString::fromLocal8Bit(name.c_str());
        QDemonRenderShaderProgram *depthShaderProgram =
                theCache.GetProgram(theCacheKey, TShaderFeatureSet());
        if (!depthShaderProgram) {
            GetProgramGenerator().BeginProgram(TShaderGeneratorStageFlags(
                                                   ShaderGeneratorStages::Vertex | ShaderGeneratorStages::TessControl
                                                   | ShaderGeneratorStages::TessEval | ShaderGeneratorStages::Fragment));
            IShaderStageGenerator &vertexShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
            IShaderStageGenerator &tessCtrlShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessControl));
            IShaderStageGenerator &tessEvalShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessEval));
            IShaderStageGenerator &fragmentShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));

            vertexShader.AddIncoming("attr_pos", "vec3");
            vertexShader.AddUniform("model_view_projection", "mat4");

            vertexShader.Append("void main() {");
            vertexShader.Append("\tgl_Position = vec4(attr_pos, 1.0);");
            vertexShader.Append("}");
            fragmentShader.Append("void main() {");
            fragmentShader.Append("\tfloat depth = (outDepth.x + 1.0) * 0.5;");
            fragmentShader.Append("\tfragOutput = vec4(depth);");
            fragmentShader.Append("}");

            tessCtrlShader.AddInclude("tessellationLinear.glsllib");
            tessCtrlShader.AddUniform("tessLevelInner", "float");
            tessCtrlShader.AddUniform("tessLevelOuter", "float");
            tessCtrlShader.Append("void main() {\n");
            tessCtrlShader.Append(
                        "\tgl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;");
            tessCtrlShader.Append("\ttessShader( tessLevelOuter, tessLevelInner);\n");
            tessCtrlShader.Append("}");

            tessEvalShader.AddInclude("tessellationLinear.glsllib");
            tessEvalShader.AddUniform("model_view_projection", "mat4");
            tessEvalShader.AddOutgoing("outDepth", "vec3");
            tessEvalShader.Append("void main() {");
            tessEvalShader.Append("\tvec4 pos = tessShader( );\n");
            tessEvalShader.Append("\tgl_Position = model_view_projection * pos;");
            tessEvalShader.Append("\toutDepth.x = gl_Position.z / gl_Position.w;");
            tessEvalShader.Append("}");
        }

        depthShaderProgram = GetProgramGenerator().CompileGeneratedShader(
                    name.c_str(), SShaderCacheProgramFlags(), TShaderFeatureSet());

        if (depthShaderProgram) {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>(
                        new SRenderableDepthPrepassShader(*depthShaderProgram, GetContext()));
        } else {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>();
        }
    }

    return theDepthShader.getValue();
}

SRenderableDepthPrepassShader *QDemonRendererImpl::GetOrthographicDepthTessPhongShader()
{
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> &theDepthShader =
            m_OrthographicDepthTessPhongShader;

    if (theDepthShader.hasValue() == false) {
        TStrType name;
        name.assign("orthographic depth tess phong shader");

        IShaderCache &theCache = m_demonContext.GetShaderCache();
        QString theCacheKey = QString::fromLocal8Bit(name.c_str());
        QDemonRenderShaderProgram *depthShaderProgram =
                theCache.GetProgram(theCacheKey, TShaderFeatureSet());
        if (!depthShaderProgram) {
            GetProgramGenerator().BeginProgram(TShaderGeneratorStageFlags(
                                                   ShaderGeneratorStages::Vertex | ShaderGeneratorStages::TessControl
                                                   | ShaderGeneratorStages::TessEval | ShaderGeneratorStages::Fragment));
            IShaderStageGenerator &vertexShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
            IShaderStageGenerator &tessCtrlShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessControl));
            IShaderStageGenerator &tessEvalShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessEval));
            IShaderStageGenerator &fragmentShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));

            vertexShader.AddIncoming("attr_pos", "vec3");
            vertexShader.AddIncoming("attr_norm", "vec3");
            vertexShader.AddOutgoing("outNormal", "vec3");
            vertexShader.AddUniform("model_view_projection", "mat4");

            vertexShader.Append("void main() {");
            vertexShader.Append("\tgl_Position = vec4(attr_pos, 1.0);");
            vertexShader.Append("\toutNormal = attr_norm;");
            vertexShader.Append("}");
            fragmentShader.Append("void main() {");
            fragmentShader.Append("\tfloat depth = (outDepth.x + 1.0) * 0.5;");
            fragmentShader.Append("\tfragOutput = vec4(depth);");
            fragmentShader.Append("}");

            tessCtrlShader.AddInclude("tessellationPhong.glsllib");
            tessCtrlShader.AddUniform("tessLevelInner", "float");
            tessCtrlShader.AddUniform("tessLevelOuter", "float");
            tessCtrlShader.Append("void main() {\n");
            tessCtrlShader.Append("\tctNorm[0] = outNormal[0];");
            tessCtrlShader.Append("\tctNorm[1] = outNormal[1];");
            tessCtrlShader.Append("\tctNorm[2] = outNormal[2];");
            tessCtrlShader.Append(
                        "\tgl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;");
            tessCtrlShader.Append("\ttessShader( tessLevelOuter, tessLevelInner);\n");
            tessCtrlShader.Append("}");

            tessEvalShader.AddInclude("tessellationPhong.glsllib");
            tessEvalShader.AddUniform("model_view_projection", "mat4");
            tessEvalShader.AddOutgoing("outDepth", "vec3");
            tessEvalShader.Append("void main() {");
            tessEvalShader.Append("\tvec4 pos = tessShader( );\n");
            tessEvalShader.Append("\tgl_Position = model_view_projection * pos;");
            tessEvalShader.Append("\toutDepth.x = gl_Position.z / gl_Position.w;");
            tessEvalShader.Append("}");
        }

        depthShaderProgram = GetProgramGenerator().CompileGeneratedShader(
                    name.c_str(), SShaderCacheProgramFlags(), TShaderFeatureSet());

        if (depthShaderProgram) {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>(
                        SRenderableDepthPrepassShader(*depthShaderProgram, GetContext()));
        } else {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>();
        }
    }

    return theDepthShader.getValue();
}

SRenderableDepthPrepassShader *QDemonRendererImpl::GetOrthographicDepthTessNPatchShader()
{
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> &theDepthShader =
            m_OrthographicDepthTessNPatchShader;

    if (theDepthShader.hasValue() == false) {
        TStrType name;
        name.assign("orthographic depth tess npatch shader");

        IShaderCache &theCache = m_demonContext.GetShaderCache();
        QString theCacheKey = QString::fromLocal8Bit(name.c_str());
        QDemonRenderShaderProgram *depthShaderProgram =
                theCache.GetProgram(theCacheKey, TShaderFeatureSet());
        if (!depthShaderProgram) {
            GetProgramGenerator().BeginProgram(TShaderGeneratorStageFlags(
                                                   ShaderGeneratorStages::Vertex | ShaderGeneratorStages::TessControl
                                                   | ShaderGeneratorStages::TessEval | ShaderGeneratorStages::Fragment));
            IShaderStageGenerator &vertexShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
            IShaderStageGenerator &tessCtrlShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessControl));
            IShaderStageGenerator &tessEvalShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessEval));
            IShaderStageGenerator &fragmentShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));

            vertexShader.AddIncoming("attr_pos", "vec3");
            vertexShader.AddIncoming("attr_norm", "vec3");
            vertexShader.AddOutgoing("outNormal", "vec3");
            vertexShader.AddUniform("model_view_projection", "mat4");
            fragmentShader.AddUniform("model_view_projection", "mat4");
            fragmentShader.AddUniform("camera_properties", "vec2");
            fragmentShader.AddUniform("camera_position", "vec3");
            fragmentShader.AddUniform("camera_direction", "vec3");
            fragmentShader.AddInclude("depthpass.glsllib");

            vertexShader.Append("void main() {");
            vertexShader.Append("\tgl_Position = vec4(attr_pos, 1.0);");
            vertexShader.Append("\toutNormal = attr_norm;");
            vertexShader.Append("}");
            fragmentShader.Append("void main() {");
            // fragmentShader.Append("\tfragOutput = vec4(0.0, 0.0, 0.0, 0.0);");
            fragmentShader.Append("\tfloat depth = (outDepth.x - camera_properties.x) / "
                                  "(camera_properties.y - camera_properties.x);");
            fragmentShader.Append("\tfragOutput = vec4(depth);");
            fragmentShader.Append("}");

            tessCtrlShader.AddInclude("tessellationNPatch.glsllib");
            tessCtrlShader.AddUniform("tessLevelInner", "float");
            tessCtrlShader.AddUniform("tessLevelOuter", "float");
            tessCtrlShader.AddOutgoing("outNormalTC", "vec3");
            tessCtrlShader.Append("void main() {\n");
            tessCtrlShader.Append("\tctNorm[0] = outNormal[0];");
            tessCtrlShader.Append("\tctNorm[1] = outNormal[1];");
            tessCtrlShader.Append("\tctNorm[2] = outNormal[2];");
            tessCtrlShader.Append(
                        "\tgl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;");
            tessCtrlShader.Append("\ttessShader( tessLevelOuter, tessLevelInner);\n");
            tessCtrlShader.Append("}");

            tessEvalShader.AddInclude("tessellationNPatch.glsllib");
            tessEvalShader.AddUniform("model_view_projection", "mat4");
            tessEvalShader.AddUniform("model_matrix", "mat4");
            tessEvalShader.AddOutgoing("outDepth", "vec3");
            tessEvalShader.Append("void main() {");
            tessEvalShader.Append("\tvec4 pos = tessShader( );\n");
            tessEvalShader.Append("\tgl_Position = model_view_projection * pos;");
            tessEvalShader.Append("\toutDepth.x = gl_Position.z / gl_Position.w;");
            tessEvalShader.Append("}");
        }

        depthShaderProgram = GetProgramGenerator().CompileGeneratedShader(
                    name.c_str(), SShaderCacheProgramFlags(), TShaderFeatureSet());

        if (depthShaderProgram) {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>(
                        new SRenderableDepthPrepassShader(*depthShaderProgram, GetContext()));
        } else {
            theDepthShader = QSharedPointer<SRenderableDepthPrepassShader>();
        }
    }

    return theDepthShader.getValue();
}

// ---------------------------------

SRenderableDepthPrepassShader *QDemonRendererImpl::GetDepthPrepassShader(bool inDisplaced)
{
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> &theDepthPrePassShader =
            (!inDisplaced) ? m_DepthPrepassShader : m_DepthPrepassShaderDisplaced;

    if (theDepthPrePassShader.hasValue() == false) {
        // check if we do displacement mapping
        TStrType name;
        name.assign("depth prepass shader");
        if (inDisplaced)
            name.append(" displacement");

        IShaderCache &theCache = m_demonContext.GetShaderCache();
        QString theCacheKey = QString::fromLocal8Bit(name.c_str());
        QDemonRenderShaderProgram *depthShaderProgram =
                theCache.GetProgram(theCacheKey, TShaderFeatureSet());
        if (!depthShaderProgram) {
            GetProgramGenerator().BeginProgram();
            IShaderStageGenerator &vertexShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
            IShaderStageGenerator &fragmentShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));
            vertexShader.AddIncoming("attr_pos", "vec3");
            vertexShader.AddUniform("model_view_projection", "mat4");

            vertexShader.Append("void main() {");

            if (inDisplaced) {
                GetDemonContext()
                        .GetDefaultMaterialShaderGenerator()
                        .AddDisplacementMappingForDepthPass(vertexShader);
            } else {
                vertexShader.Append(
                            "\tgl_Position = model_view_projection * vec4(attr_pos, 1.0);");
            }
            vertexShader.Append("}");
            fragmentShader.Append("void main() {");
            fragmentShader.Append("\tfragOutput = vec4(0.0, 0.0, 0.0, 0.0);");
            fragmentShader.Append("}");
        } else if (theCache.IsShaderCachePersistenceEnabled()) {
            // we load from shader cache set default shader stages
            GetProgramGenerator().BeginProgram();
        }

        depthShaderProgram = GetProgramGenerator().CompileGeneratedShader(
                    name.c_str(), SShaderCacheProgramFlags(), TShaderFeatureSet());

        if (depthShaderProgram) {
            theDepthPrePassShader = QSharedPointer<SRenderableDepthPrepassShader>(
                        new SRenderableDepthPrepassShader(*depthShaderProgram, GetContext()));
        } else {
            theDepthPrePassShader = QSharedPointer<SRenderableDepthPrepassShader>();
        }
    }
    return theDepthPrePassShader.getValue();
}

SRenderableDepthPrepassShader *
QDemonRendererImpl::GetDepthTessPrepassShader(TessModeValues::Enum inTessMode, bool inDisplaced)
{
    if (!m_demonContext.GetRenderContext().IsTessellationSupported()
            || inTessMode == TessModeValues::NoTess) {
        return GetDepthPrepassShader(inDisplaced);
    } else if (inTessMode == TessModeValues::TessLinear) {
        return GetDepthTessLinearPrepassShader(inDisplaced);
    } else if (inTessMode == TessModeValues::TessPhong) {
        return GetDepthTessPhongPrepassShader();
    } else if (inTessMode == TessModeValues::TessNPatch) {
        return GetDepthTessNPatchPrepassShader();
    }

    return GetDepthPrepassShader(inDisplaced);
}

SRenderableDepthPrepassShader *
QDemonRendererImpl::GetDepthTessLinearPrepassShader(bool inDisplaced)
{
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> &theDepthPrePassShader =
            (!inDisplaced) ? m_DepthTessLinearPrepassShader
                           : m_DepthTessLinearPrepassShaderDisplaced;

    if (theDepthPrePassShader.hasValue() == false) {
        // check if we do displacement mapping
        TStrType name;
        name.assign("depth tess linear prepass shader");
        if (inDisplaced)
            name.append(" displacement");

        IShaderCache &theCache = m_demonContext.GetShaderCache();
        QString theCacheKey = QString::fromLocal8Bit(name.c_str());
        QDemonRenderShaderProgram *depthShaderProgram =
                theCache.GetProgram(theCacheKey, TShaderFeatureSet());
        if (!depthShaderProgram) {
            GetProgramGenerator().BeginProgram(TShaderGeneratorStageFlags(
                                                   ShaderGeneratorStages::Vertex | ShaderGeneratorStages::TessControl
                                                   | ShaderGeneratorStages::TessEval | ShaderGeneratorStages::Fragment));
            IShaderStageGenerator &vertexShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
            IShaderStageGenerator &tessCtrlShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessControl));
            IShaderStageGenerator &tessEvalShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessEval));
            IShaderStageGenerator &fragmentShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));
            vertexShader.AddIncoming("attr_pos", "vec3");
            if (inDisplaced) {
                vertexShader.AddIncoming("attr_uv0", "vec2");
                vertexShader.AddIncoming("attr_norm", "vec3");

                vertexShader.AddUniform("displacementMap_rot", "vec4");
                vertexShader.AddUniform("displacementMap_offset", "vec3");

                vertexShader.AddOutgoing("outNormal", "vec3");
                vertexShader.AddOutgoing("outUV", "vec2");
            }
            vertexShader.AddOutgoing("outWorldPos", "vec3");
            vertexShader.AddUniform("model_view_projection", "mat4");
            vertexShader.AddUniform("model_matrix", "mat4");
            vertexShader.Append("void main() {");
            vertexShader.Append("\tgl_Position = vec4(attr_pos, 1.0);");
            if (inDisplaced) {
                vertexShader.Append("\toutNormal = attr_norm;");
                vertexShader.Append("\tvec3 uTransform = vec3( displacementMap_rot.x, "
                                    "displacementMap_rot.y, displacementMap_offset.x );");
                vertexShader.Append("\tvec3 vTransform = vec3( displacementMap_rot.z, "
                                    "displacementMap_rot.w, displacementMap_offset.y );");
                vertexShader.AddInclude(
                            "defaultMaterialLighting.glsllib"); // getTransformedUVCoords is in the
                // lighting code addition.
                vertexShader << "\tvec2 uv_coords = attr_uv0;" << Endl;
                vertexShader << "\toutUV = getTransformedUVCoords( vec3( uv_coords, 1.0), "
                                "uTransform, vTransform );\n";
            }
            vertexShader.Append("\toutWorldPos = (model_matrix * vec4(attr_pos, 1.0)).xyz;");
            vertexShader.Append("}");
            fragmentShader.Append("void main() {");
            fragmentShader.Append("\tfragOutput = vec4(0.0, 0.0, 0.0, 0.0);");
            fragmentShader.Append("}");

            tessCtrlShader.AddInclude("tessellationLinear.glsllib");
            tessCtrlShader.AddUniform("tessLevelInner", "float");
            tessCtrlShader.AddUniform("tessLevelOuter", "float");
            tessCtrlShader.AddOutgoing("outUVTC", "vec2");
            tessCtrlShader.AddOutgoing("outNormalTC", "vec3");
            tessCtrlShader.Append("void main() {\n");
            tessCtrlShader.Append("\tctWorldPos[0] = outWorldPos[0];");
            tessCtrlShader.Append("\tctWorldPos[1] = outWorldPos[1];");
            tessCtrlShader.Append("\tctWorldPos[2] = outWorldPos[2];");
            tessCtrlShader.Append(
                        "\tgl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;");
            tessCtrlShader.Append("\ttessShader( tessLevelOuter, tessLevelInner);\n");

            if (inDisplaced) {
                tessCtrlShader.Append("\toutUVTC[gl_InvocationID] = outUV[gl_InvocationID];");
                tessCtrlShader.Append(
                            "\toutNormalTC[gl_InvocationID] = outNormal[gl_InvocationID];");
            }

            tessCtrlShader.Append("}");

            tessEvalShader.AddInclude("tessellationLinear.glsllib");
            tessEvalShader.AddUniform("model_view_projection", "mat4");
            if (inDisplaced) {
                tessEvalShader.AddUniform("displacementSampler", "sampler2D");
                tessEvalShader.AddUniform("displaceAmount", "float");
                tessEvalShader.AddInclude("defaultMaterialFileDisplacementTexture.glsllib");
            }
            tessEvalShader.AddOutgoing("outUV", "vec2");
            tessEvalShader.AddOutgoing("outNormal", "vec3");
            tessEvalShader.Append("void main() {");
            tessEvalShader.Append("\tvec4 pos = tessShader( );\n");

            if (inDisplaced) {
                tessEvalShader << "\toutUV = gl_TessCoord.x * outUVTC[0] + gl_TessCoord.y * "
                                  "outUVTC[1] + gl_TessCoord.z * outUVTC[2];"
                               << Endl;
                tessEvalShader
                        << "\toutNormal = gl_TessCoord.x * outNormalTC[0] + gl_TessCoord.y * "
                           "outNormalTC[1] + gl_TessCoord.z * outNormalTC[2];"
                        << Endl;
                tessEvalShader
                        << "\tvec3 displacedPos = defaultMaterialFileDisplacementTexture( "
                           "displacementSampler , displaceAmount, outUV , outNormal, pos.xyz );"
                        << Endl;
                tessEvalShader.Append(
                            "\tgl_Position = model_view_projection * vec4(displacedPos, 1.0);");
            } else
                tessEvalShader.Append("\tgl_Position = model_view_projection * pos;");

            tessEvalShader.Append("}");
        } else if (theCache.IsShaderCachePersistenceEnabled()) {
            // we load from shader cache set default shader stages
            GetProgramGenerator().BeginProgram(TShaderGeneratorStageFlags(
                                                   ShaderGeneratorStages::Vertex | ShaderGeneratorStages::TessControl
                                                   | ShaderGeneratorStages::TessEval | ShaderGeneratorStages::Fragment));
        }

        SShaderCacheProgramFlags theFlags;
        theFlags.SetTessellationEnabled(true);

        depthShaderProgram = GetProgramGenerator().CompileGeneratedShader(
                    name.c_str(), theFlags, TShaderFeatureSet());

        if (depthShaderProgram) {
            theDepthPrePassShader = QSharedPointer<SRenderableDepthPrepassShader>(
                        new SRenderableDepthPrepassShader(*depthShaderProgram, GetContext()));
        } else {
            theDepthPrePassShader = QSharedPointer<SRenderableDepthPrepassShader>();
        }
    }
    return theDepthPrePassShader->mPtr;
}

SRenderableDepthPrepassShader *QDemonRendererImpl::GetDepthTessPhongPrepassShader()
{
    if (m_DepthTessPhongPrepassShader.hasValue() == false) {
        IShaderCache &theCache = m_demonContext.GetShaderCache();
        QString theCacheKey = QString::fromLocal8Bit("depth tess phong prepass shader");
        QDemonRenderShaderProgram *depthShaderProgram =
                theCache.GetProgram(theCacheKey, TShaderFeatureSet());
        if (!depthShaderProgram) {
            GetProgramGenerator().BeginProgram(TShaderGeneratorStageFlags(
                                                   ShaderGeneratorStages::Vertex | ShaderGeneratorStages::TessControl
                                                   | ShaderGeneratorStages::TessEval | ShaderGeneratorStages::Fragment));
            IShaderStageGenerator &vertexShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
            IShaderStageGenerator &tessCtrlShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessControl));
            IShaderStageGenerator &tessEvalShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessEval));
            IShaderStageGenerator &fragmentShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));
            vertexShader.AddIncoming("attr_pos", "vec3");
            vertexShader.AddIncoming("attr_norm", "vec3");
            vertexShader.AddOutgoing("outNormal", "vec3");
            vertexShader.AddOutgoing("outWorldPos", "vec3");
            vertexShader.AddUniform("model_view_projection", "mat4");
            vertexShader.AddUniform("model_matrix", "mat4");
            vertexShader.Append("void main() {");
            vertexShader.Append("\tgl_Position = vec4(attr_pos, 1.0);");
            vertexShader.Append("\toutWorldPos = (model_matrix * vec4(attr_pos, 1.0)).xyz;");
            vertexShader.Append("\toutNormal = attr_norm;");
            vertexShader.Append("}");
            fragmentShader.Append("void main() {");
            fragmentShader.Append("\tfragOutput = vec4(0.0, 0.0, 0.0, 0.0);");
            fragmentShader.Append("}");

            tessCtrlShader.AddInclude("tessellationPhong.glsllib");
            tessCtrlShader.AddUniform("tessLevelInner", "float");
            tessCtrlShader.AddUniform("tessLevelOuter", "float");
            tessCtrlShader.Append("void main() {\n");
            tessCtrlShader.Append("\tctWorldPos[0] = outWorldPos[0];");
            tessCtrlShader.Append("\tctWorldPos[1] = outWorldPos[1];");
            tessCtrlShader.Append("\tctWorldPos[2] = outWorldPos[2];");
            tessCtrlShader.Append("\tctNorm[0] = outNormal[0];");
            tessCtrlShader.Append("\tctNorm[1] = outNormal[1];");
            tessCtrlShader.Append("\tctNorm[2] = outNormal[2];");
            tessCtrlShader.Append(
                        "\tgl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;");
            tessCtrlShader.Append("\ttessShader( tessLevelOuter, tessLevelInner);\n");
            tessCtrlShader.Append("}");

            tessEvalShader.AddInclude("tessellationPhong.glsllib");
            tessEvalShader.AddUniform("model_view_projection", "mat4");
            tessEvalShader.Append("void main() {");
            tessEvalShader.Append("\tvec4 pos = tessShader( );\n");
            tessEvalShader.Append("\tgl_Position = model_view_projection * pos;\n");
            tessEvalShader.Append("}");
        } else if (theCache.IsShaderCachePersistenceEnabled()) {
            // we load from shader cache set default shader stages
            GetProgramGenerator().BeginProgram(TShaderGeneratorStageFlags(
                                                   ShaderGeneratorStages::Vertex | ShaderGeneratorStages::TessControl
                                                   | ShaderGeneratorStages::TessEval | ShaderGeneratorStages::Fragment));
        }

        SShaderCacheProgramFlags theFlags;
        theFlags.SetTessellationEnabled(true);

        depthShaderProgram = GetProgramGenerator().CompileGeneratedShader(
                    "depth tess phong prepass shader", theFlags, TShaderFeatureSet());

        if (depthShaderProgram) {
            m_DepthTessPhongPrepassShader = QSharedPointer<SRenderableDepthPrepassShader>(
                        new SRenderableDepthPrepassShader(*depthShaderProgram, GetContext()));
        } else {
            m_DepthTessPhongPrepassShader = QSharedPointer<SRenderableDepthPrepassShader>();
        }
    }
    return m_DepthTessPhongPrepassShader->mPtr;
}

SRenderableDepthPrepassShader *QDemonRendererImpl::GetDepthTessNPatchPrepassShader()
{
    if (m_DepthTessNPatchPrepassShader.hasValue() == false) {
        IShaderCache &theCache = m_demonContext.GetShaderCache();
        QString theCacheKey = QString::fromLocal8Bit("depth tess npatch prepass shader");
        QDemonRenderShaderProgram *depthShaderProgram =
                theCache.GetProgram(theCacheKey, TShaderFeatureSet());
        if (!depthShaderProgram) {
            GetProgramGenerator().BeginProgram(TShaderGeneratorStageFlags(
                                                   ShaderGeneratorStages::Vertex | ShaderGeneratorStages::TessControl
                                                   | ShaderGeneratorStages::TessEval | ShaderGeneratorStages::Fragment));
            IShaderStageGenerator &vertexShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
            IShaderStageGenerator &tessCtrlShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessControl));
            IShaderStageGenerator &tessEvalShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::TessEval));
            IShaderStageGenerator &fragmentShader(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));
            vertexShader.AddIncoming("attr_pos", "vec3");
            vertexShader.AddIncoming("attr_norm", "vec3");
            vertexShader.AddOutgoing("outNormal", "vec3");
            vertexShader.AddOutgoing("outWorldPos", "vec3");
            vertexShader.AddUniform("model_view_projection", "mat4");
            vertexShader.AddUniform("model_matrix", "mat4");
            vertexShader.Append("void main() {");
            vertexShader.Append("\tgl_Position = vec4(attr_pos, 1.0);");
            vertexShader.Append("\toutWorldPos = (model_matrix * vec4(attr_pos, 1.0)).xyz;");
            vertexShader.Append("\toutNormal = attr_norm;");
            vertexShader.Append("}");
            fragmentShader.Append("void main() {");
            fragmentShader.Append("\tfragOutput = vec4(0.0, 0.0, 0.0, 0.0);");
            fragmentShader.Append("}");

            tessCtrlShader.AddOutgoing("outNormalTC", "vec3");
            tessCtrlShader.AddInclude("tessellationNPatch.glsllib");
            tessCtrlShader.AddUniform("tessLevelInner", "float");
            tessCtrlShader.AddUniform("tessLevelOuter", "float");
            tessCtrlShader.Append("void main() {\n");
            tessCtrlShader.Append("\tctWorldPos[0] = outWorldPos[0];");
            tessCtrlShader.Append("\tctWorldPos[1] = outWorldPos[1];");
            tessCtrlShader.Append("\tctWorldPos[2] = outWorldPos[2];");
            tessCtrlShader.Append("\tctNorm[0] = outNormal[0];");
            tessCtrlShader.Append("\tctNorm[1] = outNormal[1];");
            tessCtrlShader.Append("\tctNorm[2] = outNormal[2];");
            tessCtrlShader.Append(
                        "\tctTangent[0] = outNormal[0];"); // we don't care for the tangent
            tessCtrlShader.Append("\tctTangent[1] = outNormal[1];");
            tessCtrlShader.Append("\tctTangent[2] = outNormal[2];");
            tessCtrlShader.Append(
                        "\tgl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;");
            tessCtrlShader.Append("\ttessShader( tessLevelOuter, tessLevelInner);\n");
            tessCtrlShader.Append(
                        "\toutNormalTC[gl_InvocationID] = outNormal[gl_InvocationID];\n");
            tessCtrlShader.Append("}");

            tessEvalShader.AddInclude("tessellationNPatch.glsllib");
            tessEvalShader.AddUniform("model_view_projection", "mat4");
            tessEvalShader.Append("void main() {");
            tessEvalShader.Append("\tctNorm[0] = outNormalTC[0];");
            tessEvalShader.Append("\tctNorm[1] = outNormalTC[1];");
            tessEvalShader.Append("\tctNorm[2] = outNormalTC[2];");
            tessEvalShader.Append(
                        "\tctTangent[0] = outNormalTC[0];"); // we don't care for the tangent
            tessEvalShader.Append("\tctTangent[1] = outNormalTC[1];");
            tessEvalShader.Append("\tctTangent[2] = outNormalTC[2];");
            tessEvalShader.Append("\tvec4 pos = tessShader( );\n");
            tessEvalShader.Append("\tgl_Position = model_view_projection * pos;\n");
            tessEvalShader.Append("}");
        } else if (theCache.IsShaderCachePersistenceEnabled()) {
            // we load from shader cache set default shader stages
            GetProgramGenerator().BeginProgram(TShaderGeneratorStageFlags(
                                                   ShaderGeneratorStages::Vertex | ShaderGeneratorStages::TessControl
                                                   | ShaderGeneratorStages::TessEval | ShaderGeneratorStages::Fragment));
        }

        SShaderCacheProgramFlags theFlags;
        theFlags.SetTessellationEnabled(true);

        depthShaderProgram = GetProgramGenerator().CompileGeneratedShader(
                    "depth tess npatch prepass shader", theFlags, TShaderFeatureSet());

        if (depthShaderProgram) {
            m_DepthTessNPatchPrepassShader = QSharedPointer<SRenderableDepthPrepassShader>(
                        new SRenderableDepthPrepassShader(*depthShaderProgram, GetContext()));
        } else {
            m_DepthTessNPatchPrepassShader =
                    QSharedPointer<SRenderableDepthPrepassShader>();
        }
    }
    return m_DepthTessNPatchPrepassShader->mPtr;
}

SDefaultAoPassShader *QDemonRendererImpl::GetDefaultAoPassShader(TShaderFeatureSet inFeatureSet)
{
    if (m_DefaultAoPassShader.hasValue() == false) {
        IShaderCache &theCache = m_demonContext.GetShaderCache();
        QString theCacheKey = QString::fromLocal8Bit("fullscreen AO pass shader");
        QDemonRenderShaderProgram *aoPassShaderProgram =
                theCache.GetProgram(theCacheKey, TShaderFeatureSet());
        if (!aoPassShaderProgram) {
            GetProgramGenerator().BeginProgram();
            IShaderStageGenerator &theVertexGenerator(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
            IShaderStageGenerator &theFragmentGenerator(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));
            theVertexGenerator.AddIncoming("attr_pos", "vec3");
            theVertexGenerator.AddIncoming("attr_uv", "vec2");
            theVertexGenerator.AddOutgoing("uv_coords", "vec2");
            theVertexGenerator.Append("void main() {");
            theVertexGenerator.Append("\tgl_Position = vec4(attr_pos.xy, 0.5, 1.0 );");
            theVertexGenerator.Append("\tuv_coords = attr_uv;");
            theVertexGenerator.Append("}");

            // fragmentGenerator.AddInclude( "SSAOCustomMaterial.glsllib" );
            theFragmentGenerator.AddInclude("viewProperties.glsllib");
            theFragmentGenerator.AddInclude("screenSpaceAO.glsllib");
            if (m_Context->GetRenderContextType() == QDemonRenderContextValues::GLES2) {
                theFragmentGenerator
                        << "\tuniform vec4 ao_properties;" << Endl
                        << "\tuniform vec4 ao_properties2;" << Endl
                        << "\tuniform vec4 shadow_properties;" << Endl
                        << "\tuniform vec4 aoScreenConst;" << Endl
                        << "\tuniform vec4 UvToEyeConst;" << Endl;
            } else {
                theFragmentGenerator
                        << "layout (std140) uniform cbAoShadow { " << Endl << "\tvec4 ao_properties;"
                        << Endl << "\tvec4 ao_properties2;" << Endl << "\tvec4 shadow_properties;"
                        << Endl << "\tvec4 aoScreenConst;" << Endl << "\tvec4 UvToEyeConst;" << Endl
                        << "};" << Endl;
            }
            theFragmentGenerator.AddUniform("camera_direction", "vec3");
            theFragmentGenerator.AddUniform("depth_sampler", "sampler2D");
            theFragmentGenerator.Append("void main() {");
            theFragmentGenerator << "\tfloat aoFactor;" << Endl;
            theFragmentGenerator << "\tvec3 screenNorm;" << Endl;

            // We're taking multiple depth samples and getting the derivatives at each of them
            // to get a more
            // accurate view space normal vector.  When we do only one, we tend to get bizarre
            // values at the edges
            // surrounding objects, and this also ends up giving us weird AO values.
            // If we had a proper screen-space normal map, that would also do the trick.
            if (m_Context->GetRenderContextType() == QDemonRenderContextValues::GLES2) {
                theFragmentGenerator.AddUniform("depth_sampler_size", "vec2");
                theFragmentGenerator.Append("\tivec2 iCoords = ivec2( gl_FragCoord.xy );");
                theFragmentGenerator.Append("\tfloat depth = getDepthValue( "
                                            "texture2D(depth_sampler, vec2(iCoords)"
                                            " / depth_sampler_size), camera_properties );");
                theFragmentGenerator.Append(
                            "\tdepth = depthValueToLinearDistance( depth, camera_properties );");
                theFragmentGenerator.Append("\tdepth = (depth - camera_properties.x) / "
                                            "(camera_properties.y - camera_properties.x);");
                theFragmentGenerator.Append("\tfloat depth2 = getDepthValue( "
                                            "texture2D(depth_sampler, vec2(iCoords+ivec2(1))"
                                            " / depth_sampler_size), camera_properties );");
                theFragmentGenerator.Append(
                            "\tdepth2 = depthValueToLinearDistance( depth, camera_properties );");
                theFragmentGenerator.Append("\tfloat depth3 = getDepthValue( "
                                            "texture2D(depth_sampler, vec2(iCoords-ivec2(1))"
                                            " / depth_sampler_size), camera_properties );");
            } else {
                theFragmentGenerator.Append("\tivec2 iCoords = ivec2( gl_FragCoord.xy );");
                theFragmentGenerator.Append("\tfloat depth = getDepthValue( "
                                            "texelFetch(depth_sampler, iCoords, 0), "
                                            "camera_properties );");
                theFragmentGenerator.Append(
                            "\tdepth = depthValueToLinearDistance( depth, camera_properties );");
                theFragmentGenerator.Append("\tdepth = (depth - camera_properties.x) / "
                                            "(camera_properties.y - camera_properties.x);");
                theFragmentGenerator.Append("\tfloat depth2 = getDepthValue( "
                                            "texelFetch(depth_sampler, iCoords+ivec2(1), 0), "
                                            "camera_properties );");
                theFragmentGenerator.Append(
                            "\tdepth2 = depthValueToLinearDistance( depth, camera_properties );");
                theFragmentGenerator.Append("\tfloat depth3 = getDepthValue( "
                                            "texelFetch(depth_sampler, iCoords-ivec2(1), 0), "
                                            "camera_properties );");
            }
            theFragmentGenerator.Append(
                        "\tdepth3 = depthValueToLinearDistance( depth, camera_properties );");
            theFragmentGenerator.Append("\tvec3 tanU = vec3(10, 0, dFdx(depth));");
            theFragmentGenerator.Append("\tvec3 tanV = vec3(0, 10, dFdy(depth));");
            theFragmentGenerator.Append("\tscreenNorm = normalize(cross(tanU, tanV));");
            theFragmentGenerator.Append("\ttanU = vec3(10, 0, dFdx(depth2));");
            theFragmentGenerator.Append("\ttanV = vec3(0, 10, dFdy(depth2));");
            theFragmentGenerator.Append("\tscreenNorm += normalize(cross(tanU, tanV));");
            theFragmentGenerator.Append("\ttanU = vec3(10, 0, dFdx(depth3));");
            theFragmentGenerator.Append("\ttanV = vec3(0, 10, dFdy(depth3));");
            theFragmentGenerator.Append("\tscreenNorm += normalize(cross(tanU, tanV));");
            theFragmentGenerator.Append("\tscreenNorm = -normalize(screenNorm);");

            theFragmentGenerator.Append("\taoFactor = \
                                        SSambientOcclusion( depth_sampler, screenNorm, ao_properties, ao_properties2, \
                                                            camera_properties, aoScreenConst, UvToEyeConst );");

                    theFragmentGenerator.Append(
                        "\tgl_FragColor = vec4(aoFactor, aoFactor, aoFactor, 1.0);");

            theFragmentGenerator.Append("}");
        }

        aoPassShaderProgram = GetProgramGenerator().CompileGeneratedShader(
                    "fullscreen AO pass shader", SShaderCacheProgramFlags(), inFeatureSet);

        if (aoPassShaderProgram) {
            m_DefaultAoPassShader = QSharedPointer<SDefaultAoPassShader>(
                        new SDefaultAoPassShader(*aoPassShaderProgram,
                                                                                      GetContext()));
        } else {
            m_DefaultAoPassShader = QSharedPointer<SDefaultAoPassShader>();
        }
    }
    return m_DefaultAoPassShader->mPtr;
}

SDefaultAoPassShader *QDemonRendererImpl::GetFakeDepthShader(TShaderFeatureSet inFeatureSet)
{
    if (m_FakeDepthShader.hasValue() == false) {
        IShaderCache &theCache = m_demonContext.GetShaderCache();
        QString theCacheKey = QString::fromLocal8Bit("depth display shader");
        QDemonRenderShaderProgram *depthShaderProgram =
                theCache.GetProgram(theCacheKey, TShaderFeatureSet());
        if (!depthShaderProgram) {
            GetProgramGenerator().BeginProgram();
            IShaderStageGenerator &theVertexGenerator(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
            IShaderStageGenerator &theFragmentGenerator(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));
            theVertexGenerator.AddIncoming("attr_pos", "vec3");
            theVertexGenerator.AddIncoming("attr_uv", "vec2");
            theVertexGenerator.AddOutgoing("uv_coords", "vec2");
            theVertexGenerator.Append("void main() {");
            theVertexGenerator.Append("\tgl_Position = vec4(attr_pos.xy, 0.5, 1.0 );");
            theVertexGenerator.Append("\tuv_coords = attr_uv;");
            theVertexGenerator.Append("}");

            theFragmentGenerator.AddUniform("depth_sampler", "sampler2D");
            theFragmentGenerator.Append("void main() {");
            theFragmentGenerator.Append("\tivec2 iCoords = ivec2( gl_FragCoord.xy );");
            theFragmentGenerator.Append(
                        "\tfloat depSample = texelFetch(depth_sampler, iCoords, 0).x;");
            theFragmentGenerator.Append(
                        "\tgl_FragColor = vec4( depSample, depSample, depSample, 1.0 );");
            theFragmentGenerator.Append("\treturn;");
            theFragmentGenerator.Append("}");
        }

        depthShaderProgram = GetProgramGenerator().CompileGeneratedShader(
                    "depth display shader", SShaderCacheProgramFlags(), inFeatureSet);

        if (depthShaderProgram) {
            m_FakeDepthShader = QSharedPointer<SDefaultAoPassShader>(
                        new SDefaultAoPassShader(
                            *depthShaderProgram, GetContext()));
        } else {
            m_FakeDepthShader = QSharedPointer<SDefaultAoPassShader>();
        }
    }
    return m_FakeDepthShader->mPtr;
}

SDefaultAoPassShader *QDemonRendererImpl::GetFakeCubeDepthShader(TShaderFeatureSet inFeatureSet)
{
    if (!m_FakeCubemapDepthShader.hasValue()) {
        IShaderCache &theCache = m_demonContext.GetShaderCache();
        QString theCacheKey = QString::fromLocal8Bit("cube depth display shader");
        QDemonRenderShaderProgram *cubeShaderProgram =
                theCache.GetProgram(theCacheKey, TShaderFeatureSet());
        if (!cubeShaderProgram) {
            GetProgramGenerator().BeginProgram();
            IShaderStageGenerator &theVertexGenerator(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
            IShaderStageGenerator &theFragmentGenerator(
                        *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));
            theVertexGenerator.AddIncoming("attr_pos", "vec3");
            theVertexGenerator.AddIncoming("attr_uv", "vec2");
            theVertexGenerator.AddOutgoing("sample_dir", "vec3");
            theVertexGenerator.Append("void main() {");
            theVertexGenerator.Append("\tgl_Position = vec4(attr_pos.xy, 0.5, 1.0 );");
            theVertexGenerator.Append(
                        "\tsample_dir = vec3(4.0 * (attr_uv.x - 0.5), -1.0, 4.0 * (attr_uv.y - 0.5));");
            theVertexGenerator.Append("}");
            theFragmentGenerator.AddUniform("depth_cube", "samplerCube");
            theFragmentGenerator.Append("void main() {");
            theFragmentGenerator.Append(
                        "\tfloat smpDepth = texture( depth_cube, sample_dir ).x;");
            theFragmentGenerator.Append(
                        "\tgl_FragColor = vec4(smpDepth, smpDepth, smpDepth, 1.0);");
            theFragmentGenerator.Append("}");
        }

        cubeShaderProgram = GetProgramGenerator().CompileGeneratedShader(
                    "cube depth display shader", SShaderCacheProgramFlags(), inFeatureSet);

        if (cubeShaderProgram) {
            m_FakeCubemapDepthShader = QSharedPointer<SDefaultAoPassShader>(
                        new SDefaultAoPassShader(*cubeShaderProgram,
                                                                                      GetContext()));
        } else {
            m_FakeCubemapDepthShader = QSharedPointer<SDefaultAoPassShader>();
        }
    }
    return m_FakeCubemapDepthShader.getValue();
}

STextRenderHelper QDemonRendererImpl::GetTextShader(bool inUsePathRendering)
{
    STextShaderPtr &thePtr = (!inUsePathRendering) ? m_TextShader : m_TextPathShader;
    if (thePtr.HasGeneratedShader())
        return STextRenderHelper(thePtr, *m_QuadInputAssembler);

    QDemonRenderShaderProgram *theShader = nullptr;
    QDemonRenderProgramPipeline *thePipeline = nullptr;

    if (!inUsePathRendering) {
        GetProgramGenerator().BeginProgram();
        IShaderStageGenerator &vertexGenerator(
                    *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
        IShaderStageGenerator &fragmentGenerator(
                    *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));

        vertexGenerator.AddIncoming("attr_pos", "vec3");
        vertexGenerator.AddIncoming("attr_uv", "vec2");
        // xy of text dimensions are scaling factors, zw are offset factors.
        vertexGenerator.AddUniform("text_dimensions", "vec4");
        vertexGenerator.AddUniform("model_view_projection", "mat4");
        vertexGenerator.AddOutgoing("uv_coords", "vec2");
        vertexGenerator.Append("void main() {");
        vertexGenerator
                << "\tvec3 textPos = vec3(attr_pos.x * text_dimensions.x + text_dimensions.z"
                << ", attr_pos.y * text_dimensions.y + text_dimensions.w"
                << ", attr_pos.z);" << Endl;

        vertexGenerator.Append("\tgl_Position = model_view_projection * vec4(textPos, 1.0);");
        vertexGenerator.Append("\tuv_coords = attr_uv;");

        fragmentGenerator.AddUniform("text_textcolor", "vec4");
        fragmentGenerator.AddUniform("text_textdimensions", "vec3");
        fragmentGenerator.AddUniform("text_image", "sampler2D");
        fragmentGenerator.AddUniform("text_backgroundcolor", "vec3");
        fragmentGenerator.Append("void main() {");
        fragmentGenerator.Append("\tvec2 theCoords = uv_coords;");
        // Enable rendering from a sub-rect

        fragmentGenerator
                << "\ttheCoords.x = theCoords.x * text_textdimensions.x;" << Endl
                << "\ttheCoords.y = theCoords.y * text_textdimensions.y;" << Endl
                   // flip the y uv coord if the dimension's z variable is set
                << "\tif ( text_textdimensions.z > 0.0 ) theCoords.y = 1.0 - theCoords.y;" << Endl;
        fragmentGenerator.Append(
                    "\tvec4 c = texture2D(text_image, theCoords);");
        fragmentGenerator.Append(
                    "\tfragOutput = vec4(mix(text_backgroundcolor.rgb, "
                    "text_textcolor.rgb, c.rgb), c.a) * text_textcolor.a;");

        vertexGenerator.Append("}");
        fragmentGenerator.Append("}");
        const char *shaderName = "text shader";
        theShader = GetProgramGenerator().CompileGeneratedShader(
                    shaderName, SShaderCacheProgramFlags(), TShaderFeatureSet(), false);
    } else {
        GetProgramGenerator().BeginProgram(
                    TShaderGeneratorStageFlags(ShaderGeneratorStages::Fragment));
        IShaderStageGenerator &fragmentGenerator(
                    *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));

        fragmentGenerator.AddUniform("text_textcolor", "vec4");
        fragmentGenerator.AddUniform("text_backgroundcolor", "vec3");

        fragmentGenerator.Append("void main() {");
        fragmentGenerator.Append("\tfragOutput = vec4(mix(text_backgroundcolor.rgb, "
                                 "text_textcolor.rgb, text_textcolor.a), text_textcolor.a );");
        fragmentGenerator.Append("}");

        const char *shaderName = "text path shader";
        theShader = GetProgramGenerator().CompileGeneratedShader(
                    shaderName, SShaderCacheProgramFlags(), TShaderFeatureSet(), true);

        // setup program pipeline
        if (theShader) {
            thePipeline = GetContext().CreateProgramPipeline();
            if (thePipeline) {
                thePipeline->SetProgramStages(
                            theShader,
                            QDemonRenderShaderTypeFlags(QDemonRenderShaderTypeValue::Fragment));
            }
        }
    }

    if (theShader == nullptr) {
        thePtr.Set(nullptr);
    } else {
        GenerateXYQuad();
        thePtr.Set(new STextShader(*theShader, thePipeline));
    }
    return STextRenderHelper(thePtr, *m_QuadInputAssembler);
}

STextDepthShader *QDemonRendererImpl::GetTextDepthShader()
{
    if (m_TextDepthPrepassShader.hasValue() == false) {
        GetProgramGenerator().BeginProgram();

        IShaderStageGenerator &vertexGenerator(
                    *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
        IShaderStageGenerator &fragmentGenerator(
                    *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));
        vertexGenerator.AddIncoming("attr_pos", "vec3");
        vertexGenerator.AddIncoming("attr_uv", "vec2");
        // xy of text dimensions are scaling factors, zw are offset factors.
        vertexGenerator.AddUniform("text_dimensions", "vec4");
        vertexGenerator.AddUniform("model_view_projection", "mat4");
        vertexGenerator.AddOutgoing("uv_coords", "vec2");
        vertexGenerator.Append("void main() {");
        vertexGenerator
                << "\tvec3 textPos = vec3(attr_pos.x * text_dimensions.x + text_dimensions.z"
                << ", attr_pos.y * text_dimensions.y + text_dimensions.w"
                << ", attr_pos.z);" << Endl;

        vertexGenerator.Append("\tgl_Position = model_view_projection * vec4(textPos, 1.0);");
        vertexGenerator.Append("\tuv_coords = attr_uv;");

        fragmentGenerator.AddUniform("text_textdimensions", "vec3");
        fragmentGenerator.AddUniform("text_image", "sampler2D");
        fragmentGenerator.Append("void main() {");
        fragmentGenerator.Append("\tvec2 theCoords = uv_coords;");
        // Enable rendering from a sub-rect

        fragmentGenerator
                << "\ttheCoords.x = theCoords.x * text_textdimensions.x;" << Endl
                << "\ttheCoords.y = theCoords.y * text_textdimensions.y;" << Endl
                   // flip the y uv coord if the dimension's z variable is set
                << "\tif ( text_textdimensions.z > 0.0 ) theCoords.y = 1.0 - theCoords.y;" << Endl;
        fragmentGenerator.Append("\tfloat alpha_mask = texture2D( text_image, theCoords ).r;");
        fragmentGenerator.Append("\tif ( alpha_mask < .05 ) discard;");
        vertexGenerator.Append("}");
        fragmentGenerator.Append("}");
        const char *shaderName = "text depth shader";
        QDemonRenderShaderProgram *theShader = GetProgramGenerator().CompileGeneratedShader(
                    shaderName, SShaderCacheProgramFlags(), TShaderFeatureSet());
        if (theShader == nullptr) {
            m_TextDepthPrepassShader = QSharedPointer<STextDepthShader>();
        } else {
            GenerateXYQuad();
            m_TextDepthPrepassShader = QSharedPointer<STextDepthShader>(
                        new STextDepthShader(*theShader, *m_QuadInputAssembler));
        }
    }
    return m_TextDepthPrepassShader->mPtr;
}

STextRenderHelper QDemonRendererImpl::GetTextWidgetShader()
{
    if (m_TextWidgetShader.HasGeneratedShader())
        return STextRenderHelper(m_TextWidgetShader, *m_QuadInputAssembler);

    GetProgramGenerator().BeginProgram();

    IShaderStageGenerator &vertexGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
    IShaderStageGenerator &fragmentGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));

    vertexGenerator.AddIncoming("attr_pos", "vec3");
    vertexGenerator.AddIncoming("attr_uv", "vec2");
    // xy of text dimensions are scaling factors, zw are offset factors.
    vertexGenerator.AddUniform("text_dimensions", "vec4");
    vertexGenerator.AddUniform("model_view_projection", "mat4");
    vertexGenerator.AddOutgoing("uv_coords", "vec2");
    vertexGenerator.Append("void main() {");
    vertexGenerator
            << "\tvec3 textPos = vec3(attr_pos.x * text_dimensions.x + text_dimensions.z"
            << ", attr_pos.y * text_dimensions.y + text_dimensions.w"
            << ", attr_pos.z);" << Endl;

    vertexGenerator.Append("\tgl_Position = model_view_projection * vec4(textPos, 1.0);");
    vertexGenerator.Append("\tuv_coords = attr_uv;");
    vertexGenerator.Append("}");

    fragmentGenerator.AddUniform("text_textcolor", "vec4");
    fragmentGenerator.AddUniform("text_textdimensions", "vec3");
    fragmentGenerator.AddUniform("text_image", "sampler2D");
    fragmentGenerator.AddUniform("text_backgroundcolor", "vec3");
    fragmentGenerator.Append("void main() {");
    fragmentGenerator.Append("\tvec2 theCoords = uv_coords;");
    // Enable rendering from a sub-rect

    fragmentGenerator << "\ttheCoords.x = theCoords.x * text_textdimensions.x;" << Endl
                      << "\ttheCoords.y = theCoords.y * text_textdimensions.y;" << Endl
                         // flip the y uv coord if the dimension's z variable is set
                      << "\tif ( text_textdimensions.z > 0.0 ) theCoords.y = 1.0 - theCoords.y;"
                      << Endl;
    fragmentGenerator.Append(
                "\tfloat alpha_mask = texture2D( text_image, theCoords ).r * text_textcolor.a;");
    fragmentGenerator.Append("\tfragOutput = vec4(mix(text_backgroundcolor.rgb, "
                             "text_textcolor.rgb, alpha_mask), 1.0 );");
    fragmentGenerator.Append("}");
    QDemonRenderShaderProgram *theShader = GetProgramGenerator().CompileGeneratedShader(
                "text widget shader", SShaderCacheProgramFlags(), TShaderFeatureSet());

    if (theShader == nullptr)
        m_TextWidgetShader.Set(nullptr);
    else {
        GenerateXYQuad();
        m_TextWidgetShader.Set(new STextShader(*theShader));
    }
    return STextRenderHelper(m_TextWidgetShader, *m_QuadInputAssembler);
}

STextRenderHelper QDemonRendererImpl::GetOnscreenTextShader()
{
    if (m_TextOnscreenShader.HasGeneratedShader())
        return STextRenderHelper(m_TextOnscreenShader, *m_QuadStripInputAssembler);

    GetProgramGenerator().BeginProgram();

    IShaderStageGenerator &vertexGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
    IShaderStageGenerator &fragmentGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));

    vertexGenerator.AddIncoming("attr_pos", "vec3");
    vertexGenerator.AddIncoming("attr_uv", "vec2");
    vertexGenerator.AddUniform("model_view_projection", "mat4");
    vertexGenerator.AddUniform("vertex_offsets", "vec2");
    vertexGenerator.AddOutgoing("uv_coords", "vec2");
    vertexGenerator.Append("void main() {");

    vertexGenerator.Append("\tvec3 pos = attr_pos + vec3(vertex_offsets, 0.0);");
    vertexGenerator.Append("\tgl_Position = model_view_projection * vec4(pos, 1.0);");
    vertexGenerator.Append("\tuv_coords = attr_uv;");
    vertexGenerator.Append("}");

    fragmentGenerator.AddUniform("text_textcolor", "vec4");
    fragmentGenerator.AddUniform("text_image", "sampler2D");
    fragmentGenerator.Append("void main() {");
    fragmentGenerator.Append("\tfloat alpha = texture2D( text_image, uv_coords ).a;");
    fragmentGenerator.Append(
                "\tfragOutput = vec4(text_textcolor.r, text_textcolor.g, text_textcolor.b, alpha);");
    fragmentGenerator.Append("}");

    QDemonRenderShaderProgram *theShader = GetProgramGenerator().CompileGeneratedShader(
                "onscreen texture shader", SShaderCacheProgramFlags(), TShaderFeatureSet());

    if (theShader == nullptr)
        m_TextOnscreenShader.Set(nullptr);
    else {
        GenerateXYQuadStrip();
        m_TextOnscreenShader.Set(new STextShader(*theShader));
    }
    return STextRenderHelper(m_TextOnscreenShader, *m_QuadStripInputAssembler);
}

QDemonRenderShaderProgram *QDemonRendererImpl::GetTextAtlasEntryShader()
{
    GetProgramGenerator().BeginProgram();

    IShaderStageGenerator &vertexGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
    IShaderStageGenerator &fragmentGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));

    vertexGenerator.AddIncoming("attr_pos", "vec3");
    vertexGenerator.AddIncoming("attr_uv", "vec2");
    vertexGenerator.AddUniform("model_view_projection", "mat4");
    vertexGenerator.AddOutgoing("uv_coords", "vec2");
    vertexGenerator.Append("void main() {");

    vertexGenerator.Append("\tgl_Position = model_view_projection * vec4(attr_pos, 1.0);");
    vertexGenerator.Append("\tuv_coords = attr_uv;");
    vertexGenerator.Append("}");

    fragmentGenerator.AddUniform("text_image", "sampler2D");
    fragmentGenerator.Append("void main() {");
    fragmentGenerator.Append("\tfloat alpha = texture2D( text_image, uv_coords ).a;");
    fragmentGenerator.Append("\tfragOutput = vec4(alpha, alpha, alpha, alpha);");
    fragmentGenerator.Append("}");

    QDemonRenderShaderProgram *theShader = GetProgramGenerator().CompileGeneratedShader(
                "texture atlas entry shader", SShaderCacheProgramFlags(), TShaderFeatureSet());

    return theShader;
}

STextRenderHelper QDemonRendererImpl::GetShader(STextRenderable & /*inRenderable*/,
                                               bool inUsePathRendering)
{
    return GetTextShader(inUsePathRendering);
}

SLayerSceneShader *QDemonRendererImpl::GetSceneLayerShader()
{
    if (m_SceneLayerShader.hasValue())
        return m_SceneLayerShader.getValue();

    GetProgramGenerator().BeginProgram();

    IShaderStageGenerator &vertexGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
    IShaderStageGenerator &fragmentGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));

    vertexGenerator.AddIncoming("attr_pos", "vec3");
    vertexGenerator.AddIncoming("attr_uv", "vec2");
    // xy of text dimensions are scaling factors, zw are offset factors.
    vertexGenerator.AddUniform("layer_dimensions", "vec2");
    vertexGenerator.AddUniform("model_view_projection", "mat4");
    vertexGenerator.AddOutgoing("uv_coords", "vec2");
    vertexGenerator.Append("void main() {");
    vertexGenerator << "\tvec3 layerPos = vec3(attr_pos.x * layer_dimensions.x / 2.0"
                    << ", attr_pos.y * layer_dimensions.y / 2.0"
                    << ", attr_pos.z);" << Endl;

    vertexGenerator.Append("\tgl_Position = model_view_projection * vec4(layerPos, 1.0);");
    vertexGenerator.Append("\tuv_coords = attr_uv;");
    vertexGenerator.Append("}");

    fragmentGenerator.AddUniform("layer_image", "sampler2D");
    fragmentGenerator.Append("void main() {");
    fragmentGenerator.Append("\tvec2 theCoords = uv_coords;\n");
    fragmentGenerator.Append("\tvec4 theLayerTexture = texture2D( layer_image, theCoords );\n");
    fragmentGenerator.Append("\tif( theLayerTexture.a == 0.0 ) discard;\n");
    fragmentGenerator.Append("\tfragOutput = theLayerTexture;\n");
    fragmentGenerator.Append("}");
    QDemonRenderShaderProgram *theShader = GetProgramGenerator().CompileGeneratedShader(
                "layer shader", SShaderCacheProgramFlags(), TShaderFeatureSet());
    QSharedPointer<SLayerSceneShader> retval;
    if (theShader)
        retval = new SLayerSceneShader(*theShader);
    m_SceneLayerShader = retval;
    return m_SceneLayerShader.getValue();
}

SLayerProgAABlendShader *QDemonRendererImpl::GetLayerProgAABlendShader()
{
    if (m_LayerProgAAShader.hasValue())
        return m_LayerProgAAShader.getValue();

    GetProgramGenerator().BeginProgram();

    IShaderStageGenerator &vertexGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
    IShaderStageGenerator &fragmentGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));
    vertexGenerator.AddIncoming("attr_pos", "vec3");
    vertexGenerator.AddIncoming("attr_uv", "vec2");
    vertexGenerator.AddOutgoing("uv_coords", "vec2");
    vertexGenerator.Append("void main() {");
    vertexGenerator.Append("\tgl_Position = vec4(attr_pos, 1.0 );");
    vertexGenerator.Append("\tuv_coords = attr_uv;");
    vertexGenerator.Append("}");
    fragmentGenerator.AddUniform("accumulator", "sampler2D");
    fragmentGenerator.AddUniform("last_frame", "sampler2D");
    fragmentGenerator.AddUniform("blend_factors", "vec2");
    fragmentGenerator.Append("void main() {");
    fragmentGenerator.Append("\tvec4 accum = texture2D( accumulator, uv_coords );");
    fragmentGenerator.Append("\tvec4 lastFrame = texture2D( last_frame, uv_coords );");
    fragmentGenerator.Append(
                "\tgl_FragColor = accum*blend_factors.y + lastFrame*blend_factors.x;");
    fragmentGenerator.Append("}");
    QDemonRenderShaderProgram *theShader = GetProgramGenerator().CompileGeneratedShader(
                "layer progressiveAA blend shader", SShaderCacheProgramFlags(), TShaderFeatureSet());
    QSharedPointer<SLayerProgAABlendShader> retval;
    if (theShader)
        retval = new SLayerProgAABlendShader(*theShader);
    m_LayerProgAAShader = retval;
    return m_LayerProgAAShader.getValue();
}

SShadowmapPreblurShader *QDemonRendererImpl::GetCubeShadowBlurXShader()
{
    if (m_CubeShadowBlurXShader.hasValue())
        return m_CubeShadowBlurXShader.getValue();

    GetProgramGenerator().BeginProgram();

    IShaderStageGenerator &vertexGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
    IShaderStageGenerator &fragmentGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));
    vertexGenerator.AddIncoming("attr_pos", "vec3");
    // vertexGenerator.AddIncoming("attr_uv", "vec2");
    vertexGenerator.AddOutgoing("uv_coords", "vec2");
    vertexGenerator.Append("void main() {");
    vertexGenerator.Append("\tgl_Position = vec4(attr_pos, 1.0 );");
    vertexGenerator.Append("\tuv_coords.xy = attr_pos.xy;");
    vertexGenerator.Append("}");

    // This with the ShadowBlurYShader design for a 2-pass 5x5 (sigma=1.0)
    // Weights computed using -- http://dev.theomader.com/gaussian-kernel-calculator/
    fragmentGenerator.AddUniform("camera_properties", "vec2");
    fragmentGenerator.AddUniform("depthCube", "samplerCube");
    // fragmentGenerator.AddUniform("depthSrc", "sampler2D");
    fragmentGenerator.Append("layout(location = 0) out vec4 frag0;");
    fragmentGenerator.Append("layout(location = 1) out vec4 frag1;");
    fragmentGenerator.Append("layout(location = 2) out vec4 frag2;");
    fragmentGenerator.Append("layout(location = 3) out vec4 frag3;");
    fragmentGenerator.Append("layout(location = 4) out vec4 frag4;");
    fragmentGenerator.Append("layout(location = 5) out vec4 frag5;");
    fragmentGenerator.Append("void main() {");
    fragmentGenerator.Append("\tfloat ofsScale = camera_properties.x / 2500.0;");
    fragmentGenerator.Append("\tvec3 dir0 = vec3(1.0, -uv_coords.y, -uv_coords.x);");
    fragmentGenerator.Append("\tvec3 dir1 = vec3(-1.0, -uv_coords.y, uv_coords.x);");
    fragmentGenerator.Append("\tvec3 dir2 = vec3(uv_coords.x, 1.0, uv_coords.y);");
    fragmentGenerator.Append("\tvec3 dir3 = vec3(uv_coords.x, -1.0, -uv_coords.y);");
    fragmentGenerator.Append("\tvec3 dir4 = vec3(uv_coords.x, -uv_coords.y, 1.0);");
    fragmentGenerator.Append("\tvec3 dir5 = vec3(-uv_coords.x, -uv_coords.y, -1.0);");
    fragmentGenerator.Append("\tfloat depth0;");
    fragmentGenerator.Append("\tfloat depth1;");
    fragmentGenerator.Append("\tfloat depth2;");
    fragmentGenerator.Append("\tfloat outDepth;");
    fragmentGenerator.Append("\tdepth0 = texture(depthCube, dir0).x;");
    fragmentGenerator.Append(
                "\tdepth1 = texture(depthCube, dir0 + vec3(0.0, 0.0, -ofsScale)).x;");
    fragmentGenerator.Append(
                "\tdepth1 += texture(depthCube, dir0 + vec3(0.0, 0.0, ofsScale)).x;");
    fragmentGenerator.Append(
                "\tdepth2 = texture(depthCube, dir0 + vec3(0.0, 0.0, -2.0*ofsScale)).x;");
    fragmentGenerator.Append(
                "\tdepth2 += texture(depthCube, dir0 + vec3(0.0, 0.0, 2.0*ofsScale)).x;");
    fragmentGenerator.Append(
                "\toutDepth = 0.38774 * depth0 + 0.24477 * depth1 + 0.06136 * depth2;");
    fragmentGenerator.Append("\tfrag0 = vec4(outDepth);");

    fragmentGenerator.Append("\tdepth0 = texture(depthCube, dir1).x;");
    fragmentGenerator.Append(
                "\tdepth1 = texture(depthCube, dir1 + vec3(0.0, 0.0, -ofsScale)).x;");
    fragmentGenerator.Append(
                "\tdepth1 += texture(depthCube, dir1 + vec3(0.0, 0.0, ofsScale)).x;");
    fragmentGenerator.Append(
                "\tdepth2 = texture(depthCube, dir1 + vec3(0.0, 0.0, -2.0*ofsScale)).x;");
    fragmentGenerator.Append(
                "\tdepth2 += texture(depthCube, dir1 + vec3(0.0, 0.0, 2.0*ofsScale)).x;");
    fragmentGenerator.Append(
                "\toutDepth = 0.38774 * depth0 + 0.24477 * depth1 + 0.06136 * depth2;");
    fragmentGenerator.Append("\tfrag1 = vec4(outDepth);");

    fragmentGenerator.Append("\tdepth0 = texture(depthCube, dir2).x;");
    fragmentGenerator.Append(
                "\tdepth1 = texture(depthCube, dir2 + vec3(-ofsScale, 0.0, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth1 += texture(depthCube, dir2 + vec3(ofsScale, 0.0, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth2 = texture(depthCube, dir2 + vec3(-2.0*ofsScale, 0.0, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth2 += texture(depthCube, dir2 + vec3(2.0*ofsScale, 0.0, 0.0)).x;");
    fragmentGenerator.Append(
                "\toutDepth = 0.38774 * depth0 + 0.24477 * depth1 + 0.06136 * depth2;");
    fragmentGenerator.Append("\tfrag2 = vec4(outDepth);");

    fragmentGenerator.Append("\tdepth0 = texture(depthCube, dir3).x;");
    fragmentGenerator.Append(
                "\tdepth1 = texture(depthCube, dir3 + vec3(-ofsScale, 0.0, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth1 += texture(depthCube, dir3 + vec3(ofsScale, 0.0, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth2 = texture(depthCube, dir3 + vec3(-2.0*ofsScale, 0.0, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth2 += texture(depthCube, dir3 + vec3(2.0*ofsScale, 0.0, 0.0)).x;");
    fragmentGenerator.Append(
                "\toutDepth = 0.38774 * depth0 + 0.24477 * depth1 + 0.06136 * depth2;");
    fragmentGenerator.Append("\tfrag3 = vec4(outDepth);");

    fragmentGenerator.Append("\tdepth0 = texture(depthCube, dir4).x;");
    fragmentGenerator.Append(
                "\tdepth1 = texture(depthCube, dir4 + vec3(-ofsScale, 0.0, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth1 += texture(depthCube, dir4 + vec3(ofsScale, 0.0, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth2 = texture(depthCube, dir4 + vec3(-2.0*ofsScale, 0.0, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth2 += texture(depthCube, dir4 + vec3(2.0*ofsScale, 0.0, 0.0)).x;");
    fragmentGenerator.Append(
                "\toutDepth = 0.38774 * depth0 + 0.24477 * depth1 + 0.06136 * depth2;");
    fragmentGenerator.Append("\tfrag4 = vec4(outDepth);");

    fragmentGenerator.Append("\tdepth0 = texture(depthCube, dir5).x;");
    fragmentGenerator.Append(
                "\tdepth1 = texture(depthCube, dir5 + vec3(-ofsScale, 0.0, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth1 += texture(depthCube, dir5 + vec3(ofsScale, 0.0, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth2 = texture(depthCube, dir5 + vec3(-2.0*ofsScale, 0.0, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth2 += texture(depthCube, dir5 + vec3(2.0*ofsScale, 0.0, 0.0)).x;");
    fragmentGenerator.Append(
                "\toutDepth = 0.38774 * depth0 + 0.24477 * depth1 + 0.06136 * depth2;");
    fragmentGenerator.Append("\tfrag5 = vec4(outDepth);");

    fragmentGenerator.Append("}");

    QDemonRenderShaderProgram *theShader = GetProgramGenerator().CompileGeneratedShader(
                "cubemap shadow blur X shader", SShaderCacheProgramFlags(), TShaderFeatureSet());
    QSharedPointer<SShadowmapPreblurShader> retval;
    if (theShader)
        retval = new SShadowmapPreblurShader(*theShader);
    m_CubeShadowBlurXShader = retval;
    return m_CubeShadowBlurXShader.getValue();
}

SShadowmapPreblurShader *QDemonRendererImpl::GetCubeShadowBlurYShader()
{
    if (m_CubeShadowBlurYShader.hasValue())
        return m_CubeShadowBlurYShader.getValue();

    GetProgramGenerator().BeginProgram();

    IShaderStageGenerator &vertexGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
    IShaderStageGenerator &fragmentGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));
    vertexGenerator.AddIncoming("attr_pos", "vec3");
    // vertexGenerator.AddIncoming("attr_uv", "vec2");
    vertexGenerator.AddOutgoing("uv_coords", "vec2");
    vertexGenerator.Append("void main() {");
    vertexGenerator.Append("\tgl_Position = vec4(attr_pos, 1.0 );");
    vertexGenerator.Append("\tuv_coords.xy = attr_pos.xy;");
    vertexGenerator.Append("}");

    // This with the ShadowBlurXShader design for a 2-pass 5x5 (sigma=1.0)
    // Weights computed using -- http://dev.theomader.com/gaussian-kernel-calculator/
    fragmentGenerator.AddUniform("camera_properties", "vec2");
    fragmentGenerator.AddUniform("depthCube", "samplerCube");
    // fragmentGenerator.AddUniform("depthSrc", "sampler2D");
    fragmentGenerator.Append("layout(location = 0) out vec4 frag0;");
    fragmentGenerator.Append("layout(location = 1) out vec4 frag1;");
    fragmentGenerator.Append("layout(location = 2) out vec4 frag2;");
    fragmentGenerator.Append("layout(location = 3) out vec4 frag3;");
    fragmentGenerator.Append("layout(location = 4) out vec4 frag4;");
    fragmentGenerator.Append("layout(location = 5) out vec4 frag5;");
    fragmentGenerator.Append("void main() {");
    fragmentGenerator.Append("\tfloat ofsScale = camera_properties.x / 2500.0;");
    fragmentGenerator.Append("\tvec3 dir0 = vec3(1.0, -uv_coords.y, -uv_coords.x);");
    fragmentGenerator.Append("\tvec3 dir1 = vec3(-1.0, -uv_coords.y, uv_coords.x);");
    fragmentGenerator.Append("\tvec3 dir2 = vec3(uv_coords.x, 1.0, uv_coords.y);");
    fragmentGenerator.Append("\tvec3 dir3 = vec3(uv_coords.x, -1.0, -uv_coords.y);");
    fragmentGenerator.Append("\tvec3 dir4 = vec3(uv_coords.x, -uv_coords.y, 1.0);");
    fragmentGenerator.Append("\tvec3 dir5 = vec3(-uv_coords.x, -uv_coords.y, -1.0);");
    fragmentGenerator.Append("\tfloat depth0;");
    fragmentGenerator.Append("\tfloat depth1;");
    fragmentGenerator.Append("\tfloat depth2;");
    fragmentGenerator.Append("\tfloat outDepth;");
    fragmentGenerator.Append("\tdepth0 = texture(depthCube, dir0).x;");
    fragmentGenerator.Append(
                "\tdepth1 = texture(depthCube, dir0 + vec3(0.0, -ofsScale, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth1 += texture(depthCube, dir0 + vec3(0.0, ofsScale, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth2 = texture(depthCube, dir0 + vec3(0.0, -2.0*ofsScale, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth2 += texture(depthCube, dir0 + vec3(0.0, 2.0*ofsScale, 0.0)).x;");
    fragmentGenerator.Append(
                "\toutDepth = 0.38774 * depth0 + 0.24477 * depth1 + 0.06136 * depth2;");
    fragmentGenerator.Append("\tfrag0 = vec4(outDepth);");

    fragmentGenerator.Append("\tdepth0 = texture(depthCube, dir1).x;");
    fragmentGenerator.Append(
                "\tdepth1 = texture(depthCube, dir1 + vec3(0.0, -ofsScale, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth1 += texture(depthCube, dir1 + vec3(0.0, ofsScale, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth2 = texture(depthCube, dir1 + vec3(0.0, -2.0*ofsScale, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth2 += texture(depthCube, dir1 + vec3(0.0, 2.0*ofsScale, 0.0)).x;");
    fragmentGenerator.Append(
                "\toutDepth = 0.38774 * depth0 + 0.24477 * depth1 + 0.06136 * depth2;");
    fragmentGenerator.Append("\tfrag1 = vec4(outDepth);");

    fragmentGenerator.Append("\tdepth0 = texture(depthCube, dir2).x;");
    fragmentGenerator.Append(
                "\tdepth1 = texture(depthCube, dir2 + vec3(0.0, 0.0, -ofsScale)).x;");
    fragmentGenerator.Append(
                "\tdepth1 += texture(depthCube, dir2 + vec3(0.0, 0.0, ofsScale)).x;");
    fragmentGenerator.Append(
                "\tdepth2 = texture(depthCube, dir2 + vec3(0.0, 0.0, -2.0*ofsScale)).x;");
    fragmentGenerator.Append(
                "\tdepth2 += texture(depthCube, dir2 + vec3(0.0, 0.0, 2.0*ofsScale)).x;");
    fragmentGenerator.Append(
                "\toutDepth = 0.38774 * depth0 + 0.24477 * depth1 + 0.06136 * depth2;");
    fragmentGenerator.Append("\tfrag2 = vec4(outDepth);");

    fragmentGenerator.Append("\tdepth0 = texture(depthCube, dir3).x;");
    fragmentGenerator.Append(
                "\tdepth1 = texture(depthCube, dir3 + vec3(0.0, 0.0, -ofsScale)).x;");
    fragmentGenerator.Append(
                "\tdepth1 += texture(depthCube, dir3 + vec3(0.0, 0.0, ofsScale)).x;");
    fragmentGenerator.Append(
                "\tdepth2 = texture(depthCube, dir3 + vec3(0.0, 0.0, -2.0*ofsScale)).x;");
    fragmentGenerator.Append(
                "\tdepth2 += texture(depthCube, dir3 + vec3(0.0, 0.0, 2.0*ofsScale)).x;");
    fragmentGenerator.Append(
                "\toutDepth = 0.38774 * depth0 + 0.24477 * depth1 + 0.06136 * depth2;");
    fragmentGenerator.Append("\tfrag3 = vec4(outDepth);");

    fragmentGenerator.Append("\tdepth0 = texture(depthCube, dir4).x;");
    fragmentGenerator.Append(
                "\tdepth1 = texture(depthCube, dir4 + vec3(0.0, -ofsScale, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth1 += texture(depthCube, dir4 + vec3(0.0, ofsScale, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth2 = texture(depthCube, dir4 + vec3(0.0, -2.0*ofsScale, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth2 += texture(depthCube, dir4 + vec3(0.0, 2.0*ofsScale, 0.0)).x;");
    fragmentGenerator.Append(
                "\toutDepth = 0.38774 * depth0 + 0.24477 * depth1 + 0.06136 * depth2;");
    fragmentGenerator.Append("\tfrag4 = vec4(outDepth);");

    fragmentGenerator.Append("\tdepth0 = texture(depthCube, dir5).x;");
    fragmentGenerator.Append(
                "\tdepth1 = texture(depthCube, dir5 + vec3(0.0, -ofsScale, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth1 += texture(depthCube, dir5 + vec3(0.0, ofsScale, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth2 = texture(depthCube, dir5 + vec3(0.0, -2.0*ofsScale, 0.0)).x;");
    fragmentGenerator.Append(
                "\tdepth2 += texture(depthCube, dir5 + vec3(0.0, 2.0*ofsScale, 0.0)).x;");
    fragmentGenerator.Append(
                "\toutDepth = 0.38774 * depth0 + 0.24477 * depth1 + 0.06136 * depth2;");
    fragmentGenerator.Append("\tfrag5 = vec4(outDepth);");

    fragmentGenerator.Append("}");

    QDemonRenderShaderProgram *theShader = GetProgramGenerator().CompileGeneratedShader(
                "cubemap shadow blur Y shader", SShaderCacheProgramFlags(), TShaderFeatureSet());
    QSharedPointer<SShadowmapPreblurShader> retval;
    if (theShader)
        retval = new SShadowmapPreblurShader(*theShader);
    m_CubeShadowBlurYShader = retval;
    return m_CubeShadowBlurYShader.getValue();
}

SShadowmapPreblurShader *QDemonRendererImpl::GetOrthoShadowBlurXShader()
{
    if (m_OrthoShadowBlurXShader.hasValue())
        return m_OrthoShadowBlurXShader.getValue();

    GetProgramGenerator().BeginProgram();

    IShaderStageGenerator &vertexGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
    IShaderStageGenerator &fragmentGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));
    vertexGenerator.AddIncoming("attr_pos", "vec3");
    vertexGenerator.AddIncoming("attr_uv", "vec2");
    vertexGenerator.AddOutgoing("uv_coords", "vec2");
    vertexGenerator.Append("void main() {");
    vertexGenerator.Append("\tgl_Position = vec4(attr_pos, 1.0 );");
    vertexGenerator.Append("\tuv_coords.xy = attr_uv.xy;");
    vertexGenerator.Append("}");

    fragmentGenerator.AddUniform("camera_properties", "vec2");
    fragmentGenerator.AddUniform("depthSrc", "sampler2D");
    fragmentGenerator.Append("void main() {");
    fragmentGenerator.Append("\tvec2 ofsScale = vec2( camera_properties.x / 7680.0, 0.0 );");
    fragmentGenerator.Append("\tfloat depth0 = texture(depthSrc, uv_coords).x;");
    fragmentGenerator.Append("\tfloat depth1 = texture(depthSrc, uv_coords + ofsScale).x;");
    fragmentGenerator.Append("\tdepth1 += texture(depthSrc, uv_coords - ofsScale).x;");
    fragmentGenerator.Append(
                "\tfloat depth2 = texture(depthSrc, uv_coords + 2.0 * ofsScale).x;");
    fragmentGenerator.Append("\tdepth2 += texture(depthSrc, uv_coords - 2.0 * ofsScale).x;");
    fragmentGenerator.Append(
                "\tfloat outDepth = 0.38774 * depth0 + 0.24477 * depth1 + 0.06136 * depth2;");
    fragmentGenerator.Append("\tfragOutput = vec4(outDepth);");
    fragmentGenerator.Append("}");

    QDemonRenderShaderProgram *theShader = GetProgramGenerator().CompileGeneratedShader(
                "shadow map blur X shader", SShaderCacheProgramFlags(), TShaderFeatureSet());
    QSharedPointer<SShadowmapPreblurShader> retval;
    if (theShader)
        retval = new SShadowmapPreblurShader(*theShader);
    m_OrthoShadowBlurXShader = retval;
    return m_OrthoShadowBlurXShader.getValue();
}

SShadowmapPreblurShader *QDemonRendererImpl::GetOrthoShadowBlurYShader()
{
    if (m_OrthoShadowBlurYShader.hasValue())
        return m_OrthoShadowBlurYShader.getValue();

    GetProgramGenerator().BeginProgram();

    IShaderStageGenerator &vertexGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
    IShaderStageGenerator &fragmentGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));
    vertexGenerator.AddIncoming("attr_pos", "vec3");
    vertexGenerator.AddIncoming("attr_uv", "vec2");
    vertexGenerator.AddOutgoing("uv_coords", "vec2");
    vertexGenerator.Append("void main() {");
    vertexGenerator.Append("\tgl_Position = vec4(attr_pos, 1.0 );");
    vertexGenerator.Append("\tuv_coords.xy = attr_uv.xy;");
    vertexGenerator.Append("}");

    fragmentGenerator.AddUniform("camera_properties", "vec2");
    fragmentGenerator.AddUniform("depthSrc", "sampler2D");
    fragmentGenerator.Append("void main() {");
    fragmentGenerator.Append("\tvec2 ofsScale = vec2( 0.0, camera_properties.x / 7680.0 );");
    fragmentGenerator.Append("\tfloat depth0 = texture(depthSrc, uv_coords).x;");
    fragmentGenerator.Append("\tfloat depth1 = texture(depthSrc, uv_coords + ofsScale).x;");
    fragmentGenerator.Append("\tdepth1 += texture(depthSrc, uv_coords - ofsScale).x;");
    fragmentGenerator.Append(
                "\tfloat depth2 = texture(depthSrc, uv_coords + 2.0 * ofsScale).x;");
    fragmentGenerator.Append("\tdepth2 += texture(depthSrc, uv_coords - 2.0 * ofsScale).x;");
    fragmentGenerator.Append(
                "\tfloat outDepth = 0.38774 * depth0 + 0.24477 * depth1 + 0.06136 * depth2;");
    fragmentGenerator.Append("\tfragOutput = vec4(outDepth);");
    fragmentGenerator.Append("}");

    QDemonRenderShaderProgram *theShader = GetProgramGenerator().CompileGeneratedShader(
                "shadow map blur Y shader", SShaderCacheProgramFlags(), TShaderFeatureSet());
    QSharedPointer<SShadowmapPreblurShader> retval;
    if (theShader)
        retval = new SShadowmapPreblurShader(*theShader);
    m_OrthoShadowBlurYShader = retval;
    return m_OrthoShadowBlurYShader.getValue();
}

#ifdef ADVANCED_BLEND_SW_FALLBACK
SAdvancedModeBlendShader *
QDemonRendererImpl::GetAdvancedBlendModeShader(AdvancedBlendModes::Enum blendMode)
{
    // Select between blend equations.
    if (blendMode == AdvancedBlendModes::Overlay) {
        return GetOverlayBlendModeShader();
    } else if (blendMode == AdvancedBlendModes::ColorBurn) {
        return GetColorBurnBlendModeShader();
    } else if (blendMode == AdvancedBlendModes::ColorDodge) {
        return GetColorDodgeBlendModeShader();
    }
    return {};
}

SAdvancedModeBlendShader *QDemonRendererImpl::GetOverlayBlendModeShader()
{
    if (m_AdvancedModeOverlayBlendShader.hasValue())
        return m_AdvancedModeOverlayBlendShader.getValue();

    GetProgramGenerator().BeginProgram();

    IShaderStageGenerator &vertexGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
    IShaderStageGenerator &fragmentGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));
    vertexGenerator.AddIncoming("attr_pos", "vec3");
    vertexGenerator.AddIncoming("attr_uv", "vec2");
    vertexGenerator.AddOutgoing("uv_coords", "vec2");
    vertexGenerator.Append("void main() {");
    vertexGenerator.Append("\tgl_Position = vec4(attr_pos, 1.0 );");
    vertexGenerator.Append("\tuv_coords = attr_uv;");
    vertexGenerator.Append("}");

    fragmentGenerator.AddUniform("base_layer", "sampler2D");
    fragmentGenerator.AddUniform("blend_layer", "sampler2D");

    fragmentGenerator.Append("void main() {");
    fragmentGenerator.Append("\tvec4 base = texture2D(base_layer, uv_coords);");
    fragmentGenerator.Append("\tif (base.a != 0.0) base.rgb /= base.a;");
    fragmentGenerator.Append("\telse base = vec4(0.0);");
    fragmentGenerator.Append("\tvec4 blend = texture2D(blend_layer, uv_coords);");
    fragmentGenerator.Append("\tif (blend.a != 0.0) blend.rgb /= blend.a;");
    fragmentGenerator.Append("\telse blend = vec4(0.0);");

    fragmentGenerator.Append("\tvec4 res = vec4(0.0);");
    fragmentGenerator.Append("\tfloat p0 = base.a * blend.a;");
    fragmentGenerator.Append("\tfloat p1 = base.a * (1.0 - blend.a);");
    fragmentGenerator.Append("\tfloat p2 = blend.a * (1.0 - base.a);");
    fragmentGenerator.Append("\tres.a = p0 + p1 + p2;");

    QDemonRenderShaderProgram *theShader;
    fragmentGenerator.Append(
                "\tfloat f_rs_rd = (base.r < 0.5? (2.0 * base.r * blend.r) : "
                "(1.0 - 2.0 * (1.0 - base.r) * (1.0 - blend.r)));");
    fragmentGenerator.Append(
                "\tfloat f_gs_gd = (base.g < 0.5? (2.0 * base.g * blend.g) : "
                "(1.0 - 2.0 * (1.0 - base.g) * (1.0 - blend.g)));");
    fragmentGenerator.Append(
                "\tfloat f_bs_bd = (base.b < 0.5? (2.0 * base.b * blend.b) : "
                "(1.0 - 2.0 * (1.0 - base.b) * (1.0 - blend.b)));");
    fragmentGenerator.Append("\tres.r = f_rs_rd * p0 + base.r * p1 + blend.r * p2;");
    fragmentGenerator.Append("\tres.g = f_gs_gd * p0 + base.g * p1 + blend.g * p2;");
    fragmentGenerator.Append("\tres.b = f_bs_bd * p0 + base.b * p1 + blend.b * p2;");
    fragmentGenerator.Append("\tgl_FragColor = vec4(res.rgb * res.a, res.a);");
    fragmentGenerator.Append("}");
    theShader = GetProgramGenerator().CompileGeneratedShader(
                "advanced overlay shader", SShaderCacheProgramFlags(), TShaderFeatureSet());

    QSharedPointer<SAdvancedModeBlendShader> retval;
    if (theShader)
        retval = new SAdvancedModeBlendShader(*theShader);
    m_AdvancedModeOverlayBlendShader = retval;
    return m_AdvancedModeOverlayBlendShader.getValue();
}

SAdvancedModeBlendShader *QDemonRendererImpl::GetColorBurnBlendModeShader()
{
    if (m_AdvancedModeColorBurnBlendShader.hasValue())
        return m_AdvancedModeColorBurnBlendShader.getValue();

    GetProgramGenerator().BeginProgram();

    IShaderStageGenerator &vertexGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
    IShaderStageGenerator &fragmentGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));
    vertexGenerator.AddIncoming("attr_pos", "vec3");
    vertexGenerator.AddIncoming("attr_uv", "vec2");
    vertexGenerator.AddOutgoing("uv_coords", "vec2");
    vertexGenerator.Append("void main() {");
    vertexGenerator.Append("\tgl_Position = vec4(attr_pos, 1.0 );");
    vertexGenerator.Append("\tuv_coords = attr_uv;");
    vertexGenerator.Append("}");

    fragmentGenerator.AddUniform("base_layer", "sampler2D");
    fragmentGenerator.AddUniform("blend_layer", "sampler2D");

    fragmentGenerator.Append("void main() {");
    fragmentGenerator.Append("\tvec4 base = texture2D(base_layer, uv_coords);");
    fragmentGenerator.Append("\tif (base.a != 0.0) base.rgb /= base.a;");
    fragmentGenerator.Append("\telse base = vec4(0.0);");
    fragmentGenerator.Append("\tvec4 blend = texture2D(blend_layer, uv_coords);");
    fragmentGenerator.Append("\tif (blend.a != 0.0) blend.rgb /= blend.a;");
    fragmentGenerator.Append("\telse blend = vec4(0.0);");

    fragmentGenerator.Append("\tvec4 res = vec4(0.0);");
    fragmentGenerator.Append("\tfloat p0 = base.a * blend.a;");
    fragmentGenerator.Append("\tfloat p1 = base.a * (1.0 - blend.a);");
    fragmentGenerator.Append("\tfloat p2 = blend.a * (1.0 - base.a);");
    fragmentGenerator.Append("\tres.a = p0 + p1 + p2;");

    QDemonRenderShaderProgram *theShader;
    fragmentGenerator.Append(
                "\tfloat f_rs_rd = ((base.r == 1.0) ? 1.0 : "
                "(blend.r == 0.0) ? 0.0 : 1.0 - min(1.0, ((1.0 - base.r) / blend.r)));");
    fragmentGenerator.Append(
                "\tfloat f_gs_gd = ((base.g == 1.0) ? 1.0 : "
                "(blend.g == 0.0) ? 0.0 : 1.0 - min(1.0, ((1.0 - base.g) / blend.g)));");
    fragmentGenerator.Append(
                "\tfloat f_bs_bd = ((base.b == 1.0) ? 1.0 : "
                "(blend.b == 0.0) ? 0.0 : 1.0 - min(1.0, ((1.0 - base.b) / blend.b)));");
    fragmentGenerator.Append("\tres.r = f_rs_rd * p0 + base.r * p1 + blend.r * p2;");
    fragmentGenerator.Append("\tres.g = f_gs_gd * p0 + base.g * p1 + blend.g * p2;");
    fragmentGenerator.Append("\tres.b = f_bs_bd * p0 + base.b * p1 + blend.b * p2;");
    fragmentGenerator.Append("\tgl_FragColor =  vec4(res.rgb * res.a, res.a);");
    fragmentGenerator.Append("}");

    theShader = GetProgramGenerator().CompileGeneratedShader(
                "advanced colorBurn shader", SShaderCacheProgramFlags(), TShaderFeatureSet());
    QSharedPointer<SAdvancedModeBlendShader> retval;
    if (theShader)
        retval = new SAdvancedModeBlendShader(*theShader);
    m_AdvancedModeColorBurnBlendShader = retval;
    return m_AdvancedModeColorBurnBlendShader.getValue();

}

SAdvancedModeBlendShader *QDemonRendererImpl::GetColorDodgeBlendModeShader()
{
    if (m_AdvancedModeColorDodgeBlendShader.hasValue())
        return m_AdvancedModeColorDodgeBlendShader.getValue();

    GetProgramGenerator().BeginProgram();

    IShaderStageGenerator &vertexGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Vertex));
    IShaderStageGenerator &fragmentGenerator(
                *GetProgramGenerator().GetStage(ShaderGeneratorStages::Fragment));
    vertexGenerator.AddIncoming("attr_pos", "vec3");
    vertexGenerator.AddIncoming("attr_uv", "vec2");
    vertexGenerator.AddOutgoing("uv_coords", "vec2");
    vertexGenerator.Append("void main() {");
    vertexGenerator.Append("\tgl_Position = vec4(attr_pos, 1.0 );");
    vertexGenerator.Append("\tuv_coords = attr_uv;");
    vertexGenerator.Append("}");

    fragmentGenerator.AddUniform("base_layer", "sampler2D");
    fragmentGenerator.AddUniform("blend_layer", "sampler2D");

    fragmentGenerator.Append("void main() {");
    fragmentGenerator.Append("\tvec4 base = texture2D(base_layer, uv_coords);");
    fragmentGenerator.Append("\tif (base.a != 0.0) base.rgb /= base.a;");
    fragmentGenerator.Append("\telse base = vec4(0.0);");
    fragmentGenerator.Append("\tvec4 blend = texture2D(blend_layer, uv_coords);");
    fragmentGenerator.Append("\tif (blend.a != 0.0) blend.rgb /= blend.a;");
    fragmentGenerator.Append("\telse blend = vec4(0.0);");

    fragmentGenerator.Append("\tvec4 res = vec4(0.0);");
    fragmentGenerator.Append("\tfloat p0 = base.a * blend.a;");
    fragmentGenerator.Append("\tfloat p1 = base.a * (1.0 - blend.a);");
    fragmentGenerator.Append("\tfloat p2 = blend.a * (1.0 - base.a);");
    fragmentGenerator.Append("\tres.a = p0 + p1 + p2;");

    QDemonRenderShaderProgram *theShader;
    fragmentGenerator.Append(
                "\tfloat f_rs_rd = ((base.r == 0.0) ? 0.0 : "
                "(blend.r == 1.0) ? 1.0 : min(base.r / (1.0 - blend.r), 1.0));");
    fragmentGenerator.Append(
                "\tfloat f_gs_gd = ((base.g == 0.0) ? 0.0 : "
                "(blend.g == 1.0) ? 1.0 : min(base.g / (1.0 - blend.g), 1.0));");
    fragmentGenerator.Append(
                "\tfloat f_bs_bd = ((base.b == 0.0) ? 0.0 : "
                "(blend.b == 1.0) ? 1.0 : min(base.b / (1.0 - blend.b), 1.0));");
    fragmentGenerator.Append("\tres.r = f_rs_rd * p0 + base.r * p1 + blend.r * p2;");
    fragmentGenerator.Append("\tres.g = f_gs_gd * p0 + base.g * p1 + blend.g * p2;");
    fragmentGenerator.Append("\tres.b = f_bs_bd * p0 + base.b * p1 + blend.b * p2;");

    fragmentGenerator.Append("\tgl_FragColor =  vec4(res.rgb * res.a, res.a);");
    fragmentGenerator.Append("}");
    theShader = GetProgramGenerator().CompileGeneratedShader(
                "advanced colorDodge shader", SShaderCacheProgramFlags(), TShaderFeatureSet());
    QSharedPointer<SAdvancedModeBlendShader> retval;
    if (theShader)
        retval = new SAdvancedModeBlendShader(*theShader);
    m_AdvancedModeColorDodgeBlendShader = retval;
    return m_AdvancedModeColorDodgeBlendShader.getValue();

}
#endif
QT_END_NAMESPACE
