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

#ifndef QDEMON_VERTEX_PIPELINE_IMPL_H
#define QDEMON_VERTEX_PIPELINE_IMPL_H
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterialshadergenerator.h>
#include <QtDemonRuntimeRender/qdemonrendertessmodevalues.h>

#include <QtCore/QSharedPointer>

QT_BEGIN_NAMESPACE
// Baseclass for the vertex pipelines to be sure we have consistent implementations.
struct SVertexPipelineImpl : public IDefaultMaterialVertexPipeline
{
    struct GenerationFlagValues
    {
        enum Enum {
            UVCoords = 1,
            EnvMapReflection = 1 << 1,
            ViewVector = 1 << 2,
            WorldNormal = 1 << 3,
            ObjectNormal = 1 << 4,
            WorldPosition = 1 << 5,
            TangentBinormal = 1 << 6,
            UVCoords1 = 1 << 7,
            VertexColor = 1 << 8,
        };
    };

    typedef TStrTableStrMap::const_iterator TParamIter;
    typedef QDemonFlags<GenerationFlagValues::Enum> TGenerationFlags;

    QSharedPointer<IMaterialShaderGenerator> m_MaterialGenerator;
    QSharedPointer<IShaderProgramGenerator> m_ProgramGenerator;
    QString m_TempString;

    TGenerationFlags m_GenerationFlags;
    bool m_Wireframe;
    TStrTableStrMap m_InterpolationParameters;
    quint32 m_DisplacementIdx;
    SRenderableImage *m_DisplacementImage;
    QStringList m_addedFunctions;

    SVertexPipelineImpl(QSharedPointer<IMaterialShaderGenerator> inMaterial,
                        QSharedPointer<IShaderProgramGenerator> inProgram,
                        bool inWireframe // only works if tessellation is true
                        )

        : m_MaterialGenerator(inMaterial)
        , m_ProgramGenerator(inProgram)
        , m_Wireframe(inWireframe)
        , m_DisplacementIdx(0)
        , m_DisplacementImage(nullptr)
    {
    }

    // Trues true if the code was *not* set.
    bool SetCode(GenerationFlagValues::Enum inCode)
    {
        if ((quint32(m_GenerationFlags) & inCode) != 0)
            return true;
        m_GenerationFlags |= inCode;
        return false;
    }
    bool HasCode(GenerationFlagValues::Enum inCode)
    {
        return ((quint32(m_GenerationFlags) & inCode)) != 0;
    }
    QSharedPointer<IShaderProgramGenerator> ProgramGenerator() { return m_ProgramGenerator; }

    IShaderStageGenerator &Vertex()
    {
        return *ProgramGenerator()->GetStage(ShaderGeneratorStages::Vertex);
    }
    IShaderStageGenerator &TessControl()
    {
        return *ProgramGenerator()->GetStage(ShaderGeneratorStages::TessControl);
    }
    IShaderStageGenerator &TessEval()
    {
        return *ProgramGenerator()->GetStage(ShaderGeneratorStages::TessEval);
    }
    IShaderStageGenerator &Geometry()
    {
        return *ProgramGenerator()->GetStage(ShaderGeneratorStages::Geometry);
    }
    IShaderStageGenerator &Fragment()
    {
        return *ProgramGenerator()->GetStage(ShaderGeneratorStages::Fragment);
    }
    QSharedPointer<IMaterialShaderGenerator> MaterialGenerator() { return m_MaterialGenerator; }

    void SetupDisplacement(quint32 displacementImageIdx, SRenderableImage *displacementImage)
    {
        m_DisplacementIdx = displacementImageIdx;
        m_DisplacementImage = displacementImage;
    }

    QString Str(const char *inItem) { return QString::fromLocal8Bit(inItem); }

    bool HasTessellation() const
    {
        return m_ProgramGenerator->GetEnabledStages() & ShaderGeneratorStages::TessEval;
    }
    bool HasGeometryStage() const
    {
        return m_ProgramGenerator->GetEnabledStages() & ShaderGeneratorStages::Geometry;
    }
    bool HasDisplacment() const { return m_DisplacementImage != nullptr; }

    void InitializeWireframeGeometryShader()
    {
        if (m_Wireframe && ProgramGenerator()->GetStage(ShaderGeneratorStages::Geometry)
                && ProgramGenerator()->GetStage(ShaderGeneratorStages::TessEval)) {
            IShaderStageGenerator &geometryShader(*ProgramGenerator()->GetStage(ShaderGeneratorStages::Geometry));
            // currently geometry shader is only used for drawing wireframe
            if (m_Wireframe) {
                geometryShader.AddUniform(QStringLiteral("viewport_matrix"), QStringLiteral("mat4"));
                geometryShader.AddOutgoing(QStringLiteral("varEdgeDistance"), QStringLiteral("vec3"));
                geometryShader.Append(QStringLiteral("layout (triangles) in;"));
                geometryShader.Append(QStringLiteral("layout (triangle_strip, max_vertices = 3) out;"));
                geometryShader.Append(QStringLiteral("void main() {"));

                // how this all work see
                // http://developer.download.nvidia.com/SDK/10.5/direct3d/Source/SolidWireframe/Doc/SolidWireframe.pdf

                geometryShader.Append(QStringLiteral(
                            "// project points to screen space\n"
                            "\tvec3 p0 = vec3(viewport_matrix * (gl_in[0].gl_Position / "
                            "gl_in[0].gl_Position.w));\n"
                            "\tvec3 p1 = vec3(viewport_matrix * (gl_in[1].gl_Position / "
                            "gl_in[1].gl_Position.w));\n"
                            "\tvec3 p2 = vec3(viewport_matrix * (gl_in[2].gl_Position / "
                            "gl_in[2].gl_Position.w));\n"
                            "// compute triangle heights\n"
                            "\tfloat e1 = length(p1 - p2);\n"
                            "\tfloat e2 = length(p2 - p0);\n"
                            "\tfloat e3 = length(p1 - p0);\n"
                            "\tfloat alpha = acos( (e2*e2 + e3*e3 - e1*e1) / (2.0*e2*e3) );\n"
                            "\tfloat beta = acos( (e1*e1 + e3*e3 - e2*e2) / (2.0*e1*e3) );\n"
                            "\tfloat ha = abs( e3 * sin( beta ) );\n"
                            "\tfloat hb = abs( e3 * sin( alpha ) );\n"
                            "\tfloat hc = abs( e2 * sin( alpha ) );\n"));
            }
        }
    }

    void FinalizeWireframeGeometryShader()
    {
        IShaderStageGenerator &geometryShader(*ProgramGenerator()->GetStage(ShaderGeneratorStages::Geometry));

        if (m_Wireframe == true && ProgramGenerator()->GetStage(ShaderGeneratorStages::Geometry)
                && ProgramGenerator()->GetStage(ShaderGeneratorStages::TessEval)) {
            const char *theExtension("TE[");
            // we always assume triangles
            for (int i = 0; i < 3; i++) {
                char buf[10];
                sprintf(buf, "%d", i);
                for (TStrTableStrMap::iterator iter = m_InterpolationParameters.begin(), end = m_InterpolationParameters.end(); iter != end; ++iter) {
                    geometryShader << QStringLiteral("\t")
                                   << iter.key()
                                   << QStringLiteral(" = ")
                                   << iter.key()
                                   << QString::fromLocal8Bit(theExtension)
                                   << QString::fromLocal8Bit(buf)
                                   << QStringLiteral("];\n");
                }

                geometryShader << QStringLiteral("\tgl_Position = gl_in[") << QString::fromLocal8Bit(buf) << QStringLiteral("].gl_Position;\n");
                // the triangle distance is interpolated through the shader stage
                if (i == 0)
                    geometryShader << QStringLiteral("\n\tvarEdgeDistance = vec3(ha*")
                                   << QStringLiteral("gl_in[") << QString::fromLocal8Bit(buf) << QStringLiteral("].gl_Position.w, 0.0, 0.0);\n");
                else if (i == 1)
                    geometryShader << QStringLiteral("\n\tvarEdgeDistance = vec3(0.0, hb*")
                                   << QStringLiteral("gl_in[") << QString::fromLocal8Bit(buf) << QStringLiteral("].gl_Position.w, 0.0);\n");
                else if (i == 2)
                    geometryShader << QStringLiteral("\n\tvarEdgeDistance = vec3(0.0, 0.0, hc*")
                                   << QStringLiteral("gl_in[") << QString::fromLocal8Bit(buf) << QStringLiteral("].gl_Position.w);\n");

                // submit vertex
                geometryShader << QStringLiteral("\tEmitVertex();\n");
            }
            // end primitive
            geometryShader << QStringLiteral("\tEndPrimitive();\n");
        }
    }

    virtual void SetupTessIncludes(ShaderGeneratorStages::Enum inStage, TessModeValues::Enum inTessMode)
    {
        IShaderStageGenerator &tessShader(*ProgramGenerator()->GetStage(inStage));

        // depending on the selected tessellation mode chose program
        switch (inTessMode) {
        case TessModeValues::TessPhong:
            tessShader.AddInclude(QStringLiteral("tessellationPhong.glsllib"));
            break;
        case TessModeValues::TessNPatch:
            tessShader.AddInclude(QStringLiteral("tessellationNPatch.glsllib"));
            break;
        default:
            Q_ASSERT(false); // fallthrough intentional
        case TessModeValues::TessLinear:
            tessShader.AddInclude(QStringLiteral("tessellationLinear.glsllib"));
            break;
        }
    }

    void GenerateUVCoords(quint32 inUVSet = 0) override
    {
        if (inUVSet == 0 && SetCode(GenerationFlagValues::UVCoords))
            return;
        if (inUVSet == 1 && SetCode(GenerationFlagValues::UVCoords1))
            return;

        Q_ASSERT(inUVSet == 0 || inUVSet == 1);

        if (inUVSet == 0)
            AddInterpolationParameter(QStringLiteral("varTexCoord0"), QStringLiteral("vec2"));
        else if (inUVSet == 1)
            AddInterpolationParameter(QStringLiteral("varTexCoord1"), QStringLiteral("vec2"));

        DoGenerateUVCoords(inUVSet);
    }
    void GenerateEnvMapReflection() override
    {
        if (SetCode(GenerationFlagValues::EnvMapReflection))
            return;

        GenerateWorldPosition();
        GenerateWorldNormal();
        IShaderStageGenerator &activeGenerator(ActiveStage());
        activeGenerator.AddInclude(QStringLiteral("viewProperties.glsllib"));
        AddInterpolationParameter(QStringLiteral("var_object_to_camera"), QStringLiteral("vec3"));
        activeGenerator.Append(QStringLiteral("\tvar_object_to_camera = normalize( local_model_world_position "
                               "- camera_position );"));
        // World normal cannot be relied upon in the vertex shader because of bump maps.
        Fragment().Append(QStringLiteral("\tvec3 environment_map_reflection = reflect( "
                          "normalize(var_object_to_camera), world_normal.xyz );"));
        Fragment().Append(QStringLiteral("\tenvironment_map_reflection *= vec3( 0.5, 0.5, 0 );"));
        Fragment().Append(QStringLiteral("\tenvironment_map_reflection += vec3( 0.5, 0.5, 1.0 );"));
    }
    void GenerateViewVector() override
    {
        if (SetCode(GenerationFlagValues::ViewVector))
            return;
        GenerateWorldPosition();
        IShaderStageGenerator &activeGenerator(ActiveStage());
        activeGenerator.AddInclude(QStringLiteral("viewProperties.glsllib"));
        AddInterpolationParameter(QStringLiteral("varViewVector"), QStringLiteral("vec3"));
        activeGenerator.Append(QStringLiteral("\tvec3 local_view_vector = normalize(camera_position - "
                               "local_model_world_position);"));
        AssignOutput(QStringLiteral("varViewVector"), QStringLiteral("local_view_vector"));
        Fragment() << QStringLiteral("\tvec3 view_vector = normalize(varViewVector);\n");
    }

    // fragment shader expects varying vertex normal
    // lighting in vertex pipeline expects world_normal
    void GenerateWorldNormal() override
    {
        if (SetCode(GenerationFlagValues::WorldNormal))
            return;
        AddInterpolationParameter(QStringLiteral("varNormal"), QStringLiteral("vec3"));
        DoGenerateWorldNormal();
        Fragment().Append(QStringLiteral("\tvec3 world_normal = normalize( varNormal );"));
    }
    void GenerateObjectNormal() override
    {
        if (SetCode(GenerationFlagValues::ObjectNormal))
            return;
        DoGenerateObjectNormal();
        Fragment().Append(QStringLiteral("\tvec3 object_normal = normalize(varObjectNormal);"));
    }
    void GenerateWorldPosition() override
    {
        if (SetCode(GenerationFlagValues::WorldPosition))
            return;

        ActiveStage().AddUniform(QStringLiteral("model_matrix"), QStringLiteral("mat4"));
        AddInterpolationParameter(QStringLiteral("varWorldPos"), QStringLiteral("vec3"));
        DoGenerateWorldPosition();

        AssignOutput(QStringLiteral("varWorldPos"), QStringLiteral("local_model_world_position"));
    }
    void GenerateVarTangentAndBinormal() override
    {
        if (SetCode(GenerationFlagValues::TangentBinormal))
            return;
        AddInterpolationParameter(QStringLiteral("varTangent"), QStringLiteral("vec3"));
        AddInterpolationParameter(QStringLiteral("varBinormal"), QStringLiteral("vec3"));
        DoGenerateVarTangentAndBinormal();
        Fragment() << QStringLiteral("\tvec3 tangent = normalize(varTangent);\n")
                   << QStringLiteral("\tvec3 binormal = normalize(varBinormal);\n");
    }
    void GenerateVertexColor() override
    {
        if (SetCode(GenerationFlagValues::VertexColor))
            return;
        AddInterpolationParameter(QStringLiteral("varColor"), QStringLiteral("vec3"));
        DoGenerateVertexColor();
        Fragment().Append(QStringLiteral("\tvec3 vertColor = varColor;"));
    }

    bool HasActiveWireframe() override { return m_Wireframe; }

    void AddIncoming(const QString &name, const QString &type) override
    {
        ActiveStage().AddIncoming(name, type);
    }

    void AddOutgoing(const QString &name, const QString &type) override
    {
        AddInterpolationParameter(name, type);
    }

    void AddUniform(const QString &name, const QString &type) override
    {
        ActiveStage().AddUniform(name, type);
    }

    void AddInclude(const QString &name) override { ActiveStage().AddInclude(name); }

    void AddFunction(const QString &functionName) override
    {
        if (!m_addedFunctions.contains(functionName)) {
            m_addedFunctions.push_back(functionName);
            QString includeName;
            QTextStream stream(&includeName);
            stream << "func" << functionName << ".glsllib";
            AddInclude(includeName);
        }
    }

    void AddConstantBuffer(const QString &name, const QString &layout) override
    {
        ActiveStage().AddConstantBuffer(name, layout);
    }
    void AddConstantBufferParam(const QString &cbName, const QString &paramName,
                                const QString &type) override
    {
        ActiveStage().AddConstantBufferParam(cbName, paramName, type);
    }

    IShaderStageGenerator &operator<<(const QString &data) override
    {
        ActiveStage() << data;
        return *this;
    }

    void Append(const QString &data) override { ActiveStage().Append(data); }
    void AppendPartial(const QString &data) override { ActiveStage().Append(data); }

    ShaderGeneratorStages::Enum Stage() const override
    {
        return const_cast<SVertexPipelineImpl *>(this)->ActiveStage().Stage();
    }

    void BeginVertexGeneration(quint32 displacementImageIdx, SRenderableImage *displacementImage) override = 0;
    void AssignOutput(const QString &inVarName, const QString &inVarValueExpr) override = 0;
    void EndVertexGeneration() override = 0;

    void BeginFragmentGeneration() override = 0;
    void EndFragmentGeneration() override = 0;

    virtual IShaderStageGenerator &ActiveStage() = 0;
    virtual void AddInterpolationParameter(const QString &inParamName, const QString &inParamType) = 0;

    virtual void DoGenerateUVCoords(quint32 inUVSet) = 0;
    virtual void DoGenerateWorldNormal() = 0;
    virtual void DoGenerateObjectNormal() = 0;
    virtual void DoGenerateWorldPosition() = 0;
    virtual void DoGenerateVarTangentAndBinormal() = 0;
    virtual void DoGenerateVertexColor() = 0;
};
QT_END_NAMESPACE

#endif
