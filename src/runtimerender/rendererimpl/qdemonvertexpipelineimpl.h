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
struct QDemonVertexPipelineImpl : public QDemonDefaultMaterialVertexPipelineInterface
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

    QDemonRef<QDemonMaterialShaderGeneratorInterface> m_materialGenerator;
    QDemonRef<QDemonShaderProgramGeneratorInterface> m_programGenerator;
    QString m_tempString;

    TGenerationFlags m_generationFlags;
    bool m_wireframe;
    TStrTableStrMap m_interpolationParameters;
    quint32 m_displacementIdx;
    QDemonRenderableImage *m_displacementImage;
    QStringList m_addedFunctions;

    QDemonVertexPipelineImpl(QDemonRef<QDemonMaterialShaderGeneratorInterface> inMaterial,
                             QDemonRef<QDemonShaderProgramGeneratorInterface> inProgram,
                             bool inWireframe /* only works if tessellation is true */)

        : m_materialGenerator(inMaterial)
        , m_programGenerator(inProgram)
        , m_wireframe(inWireframe)
        , m_displacementIdx(0)
        , m_displacementImage(nullptr)
    {
    }

    // Trues true if the code was *not* set.
    bool setCode(GenerationFlagValues::Enum inCode)
    {
        if ((quint32(m_generationFlags) & inCode) != 0)
            return true;
        m_generationFlags |= inCode;
        return false;
    }
    bool hasCode(GenerationFlagValues::Enum inCode)
    {
        return ((quint32(m_generationFlags) & inCode)) != 0;
    }
    QDemonRef<QDemonShaderProgramGeneratorInterface> programGenerator() { return m_programGenerator; }

    QDemonShaderStageGeneratorInterface &vertex()
    {
        return *programGenerator()->getStage(ShaderGeneratorStages::Vertex);
    }
    QDemonShaderStageGeneratorInterface &tessControl()
    {
        return *programGenerator()->getStage(ShaderGeneratorStages::TessControl);
    }
    QDemonShaderStageGeneratorInterface &tessEval()
    {
        return *programGenerator()->getStage(ShaderGeneratorStages::TessEval);
    }
    QDemonShaderStageGeneratorInterface &geometry()
    {
        return *programGenerator()->getStage(ShaderGeneratorStages::Geometry);
    }
    QDemonShaderStageGeneratorInterface &fragment()
    {
        return *programGenerator()->getStage(ShaderGeneratorStages::Fragment);
    }
    QDemonRef<QDemonMaterialShaderGeneratorInterface> materialGenerator() { return m_materialGenerator; }

    void setupDisplacement(quint32 displacementImageIdx, QDemonRenderableImage *displacementImage)
    {
        m_displacementIdx = displacementImageIdx;
        m_displacementImage = displacementImage;
    }

    QString toQString(const char *inItem) { return QString::fromLocal8Bit(inItem); }

    bool hasTessellation() const
    {
        return m_programGenerator->getEnabledStages() & ShaderGeneratorStages::TessEval;
    }
    bool hasGeometryStage() const
    {
        return m_programGenerator->getEnabledStages() & ShaderGeneratorStages::Geometry;
    }
    bool hasDisplacment() const { return m_displacementImage != nullptr; }

    void initializeWireframeGeometryShader()
    {
        if (m_wireframe && programGenerator()->getStage(ShaderGeneratorStages::Geometry)
                && programGenerator()->getStage(ShaderGeneratorStages::TessEval)) {
            QDemonShaderStageGeneratorInterface &geometryShader(*programGenerator()->getStage(ShaderGeneratorStages::Geometry));
            // currently geometry shader is only used for drawing wireframe
            if (m_wireframe) {
                geometryShader.addUniform(QStringLiteral("viewport_matrix"), QStringLiteral("mat4"));
                geometryShader.addOutgoing(QStringLiteral("varEdgeDistance"), QStringLiteral("vec3"));
                geometryShader.append(QStringLiteral("layout (triangles) in;"));
                geometryShader.append(QStringLiteral("layout (triangle_strip, max_vertices = 3) out;"));
                geometryShader.append(QStringLiteral("void main() {"));

                // how this all work see
                // http://developer.download.nvidia.com/SDK/10.5/direct3d/Source/SolidWireframe/Doc/SolidWireframe.pdf

                geometryShader.append(QStringLiteral(
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

    void finalizeWireframeGeometryShader()
    {
        QDemonShaderStageGeneratorInterface &geometryShader(*programGenerator()->getStage(ShaderGeneratorStages::Geometry));

        if (m_wireframe == true && programGenerator()->getStage(ShaderGeneratorStages::Geometry)
                && programGenerator()->getStage(ShaderGeneratorStages::TessEval)) {
            const char *theExtension("TE[");
            // we always assume triangles
            for (int i = 0; i < 3; i++) {
                char buf[10];
                sprintf(buf, "%d", i);
                for (TStrTableStrMap::iterator iter = m_interpolationParameters.begin(), end = m_interpolationParameters.end(); iter != end; ++iter) {
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
                if (i == 0) {
                    geometryShader << QStringLiteral("\n\tvarEdgeDistance = vec3(ha*")
                                   << QStringLiteral("gl_in[") << QString::fromLocal8Bit(buf) << QStringLiteral("].gl_Position.w, 0.0, 0.0);\n");
                } else if (i == 1) {
                    geometryShader << QStringLiteral("\n\tvarEdgeDistance = vec3(0.0, hb*")
                                   << QStringLiteral("gl_in[") << QString::fromLocal8Bit(buf) << QStringLiteral("].gl_Position.w, 0.0);\n");
                } else if (i == 2) {
                    geometryShader << QStringLiteral("\n\tvarEdgeDistance = vec3(0.0, 0.0, hc*")
                                   << QStringLiteral("gl_in[") << QString::fromLocal8Bit(buf) << QStringLiteral("].gl_Position.w);\n");
                }

                // submit vertex
                geometryShader << QStringLiteral("\tEmitVertex();\n");
            }
            // end primitive
            geometryShader << QStringLiteral("\tEndPrimitive();\n");
        }
    }

    virtual void setupTessIncludes(ShaderGeneratorStages::Enum inStage, TessModeValues::Enum inTessMode)
    {
        QDemonShaderStageGeneratorInterface &tessShader(*programGenerator()->getStage(inStage));

        // depending on the selected tessellation mode chose program
        switch (inTessMode) {
        case TessModeValues::TessPhong:
            tessShader.addInclude(QStringLiteral("tessellationPhong.glsllib"));
            break;
        case TessModeValues::TessNPatch:
            tessShader.addInclude(QStringLiteral("tessellationNPatch.glsllib"));
            break;
        default:
            Q_ASSERT(false); // fallthrough intentional
        case TessModeValues::TessLinear:
            tessShader.addInclude(QStringLiteral("tessellationLinear.glsllib"));
            break;
        }
    }

    void generateUVCoords(quint32 inUVSet = 0) override
    {
        if (inUVSet == 0 && setCode(GenerationFlagValues::UVCoords))
            return;
        if (inUVSet == 1 && setCode(GenerationFlagValues::UVCoords1))
            return;

        Q_ASSERT(inUVSet == 0 || inUVSet == 1);

        if (inUVSet == 0)
            addInterpolationParameter(QStringLiteral("varTexCoord0"), QStringLiteral("vec2"));
        else if (inUVSet == 1)
            addInterpolationParameter(QStringLiteral("varTexCoord1"), QStringLiteral("vec2"));

        doGenerateUVCoords(inUVSet);
    }
    void generateEnvMapReflection() override
    {
        if (setCode(GenerationFlagValues::EnvMapReflection))
            return;

        generateWorldPosition();
        generateWorldNormal();
        QDemonShaderStageGeneratorInterface &activeGenerator(activeStage());
        activeGenerator.addInclude(QStringLiteral("viewProperties.glsllib"));
        addInterpolationParameter(QStringLiteral("var_object_to_camera"), QStringLiteral("vec3"));
        activeGenerator.append(QStringLiteral("\tvar_object_to_camera = normalize( local_model_world_position "
                               "- camera_position );"));
        // World normal cannot be relied upon in the vertex shader because of bump maps.
        fragment().append(QStringLiteral("\tvec3 environment_map_reflection = reflect( "
                          "normalize(var_object_to_camera), world_normal.xyz );"));
        fragment().append(QStringLiteral("\tenvironment_map_reflection *= vec3( 0.5, 0.5, 0 );"));
        fragment().append(QStringLiteral("\tenvironment_map_reflection += vec3( 0.5, 0.5, 1.0 );"));
    }
    void generateViewVector() override
    {
        if (setCode(GenerationFlagValues::ViewVector))
            return;
        generateWorldPosition();
        QDemonShaderStageGeneratorInterface &activeGenerator(activeStage());
        activeGenerator.addInclude(QStringLiteral("viewProperties.glsllib"));
        addInterpolationParameter(QStringLiteral("varViewVector"), QStringLiteral("vec3"));
        activeGenerator.append(QStringLiteral("\tvec3 local_view_vector = normalize(camera_position - "
                               "local_model_world_position);"));
        assignOutput(QStringLiteral("varViewVector"), QStringLiteral("local_view_vector"));
        fragment() << QStringLiteral("\tvec3 view_vector = normalize(varViewVector);\n");
    }

    // fragment shader expects varying vertex normal
    // lighting in vertex pipeline expects world_normal
    void generateWorldNormal() override
    {
        if (setCode(GenerationFlagValues::WorldNormal))
            return;
        addInterpolationParameter(QStringLiteral("varNormal"), QStringLiteral("vec3"));
        doGenerateWorldNormal();
        fragment().append(QStringLiteral("\tvec3 world_normal = normalize( varNormal );"));
    }
    void generateObjectNormal() override
    {
        if (setCode(GenerationFlagValues::ObjectNormal))
            return;
        doGenerateObjectNormal();
        fragment().append(QStringLiteral("\tvec3 object_normal = normalize(varObjectNormal);"));
    }
    void generateWorldPosition() override
    {
        if (setCode(GenerationFlagValues::WorldPosition))
            return;

        activeStage().addUniform(QStringLiteral("model_matrix"), QStringLiteral("mat4"));
        addInterpolationParameter(QStringLiteral("varWorldPos"), QStringLiteral("vec3"));
        doGenerateWorldPosition();

        assignOutput(QStringLiteral("varWorldPos"), QStringLiteral("local_model_world_position"));
    }
    void generateVarTangentAndBinormal() override
    {
        if (setCode(GenerationFlagValues::TangentBinormal))
            return;
        addInterpolationParameter(QStringLiteral("varTangent"), QStringLiteral("vec3"));
        addInterpolationParameter(QStringLiteral("varBinormal"), QStringLiteral("vec3"));
        doGenerateVarTangentAndBinormal();
        fragment() << QStringLiteral("\tvec3 tangent = normalize(varTangent);\n")
                   << QStringLiteral("\tvec3 binormal = normalize(varBinormal);\n");
    }
    void generateVertexColor() override
    {
        if (setCode(GenerationFlagValues::VertexColor))
            return;
        addInterpolationParameter(QStringLiteral("varColor"), QStringLiteral("vec3"));
        doGenerateVertexColor();
        fragment().append(QStringLiteral("\tvec3 vertColor = varColor;"));
    }

    bool hasActiveWireframe() override { return m_wireframe; }

    void addIncoming(const QString &name, const QString &type) override
    {
        activeStage().addIncoming(name, type);
    }

    void addOutgoing(const QString &name, const QString &type) override
    {
        addInterpolationParameter(name, type);
    }

    void addUniform(const QString &name, const QString &type) override
    {
        activeStage().addUniform(name, type);
    }

    void addInclude(const QString &name) override { activeStage().addInclude(name); }

    void addFunction(const QString &functionName) override
    {
        if (!m_addedFunctions.contains(functionName)) {
            m_addedFunctions.push_back(functionName);
            QString includeName = QStringLiteral("func") + functionName + QStringLiteral(".glsllib");
            addInclude(includeName);
        }
    }

    void addConstantBuffer(const QString &name, const QString &layout) override
    {
        activeStage().addConstantBuffer(name, layout);
    }
    void addConstantBufferParam(const QString &cbName, const QString &paramName,
                                const QString &type) override
    {
        activeStage().addConstantBufferParam(cbName, paramName, type);
    }

    QDemonShaderStageGeneratorInterface &operator<<(const QString &data) override
    {
        activeStage() << data;
        return *this;
    }

    void append(const QString &data) override { activeStage().append(data); }
    void appendPartial(const QString &data) override { activeStage().append(data); }

    ShaderGeneratorStages::Enum stage() const override
    {
        return const_cast<QDemonVertexPipelineImpl *>(this)->activeStage().stage();
    }

    void beginVertexGeneration(quint32 displacementImageIdx, QDemonRenderableImage *displacementImage) override = 0;
    void assignOutput(const QString &inVarName, const QString &inVarValueExpr) override = 0;
    void endVertexGeneration() override = 0;

    void beginFragmentGeneration() override = 0;
    void endFragmentGeneration() override = 0;

    virtual QDemonShaderStageGeneratorInterface &activeStage() = 0;
    virtual void addInterpolationParameter(const QString &inParamName, const QString &inParamType) = 0;

    virtual void doGenerateUVCoords(quint32 inUVSet) = 0;
    virtual void doGenerateWorldNormal() = 0;
    virtual void doGenerateObjectNormal() = 0;
    virtual void doGenerateWorldPosition() = 0;
    virtual void doGenerateVarTangentAndBinormal() = 0;
    virtual void doGenerateVertexColor() = 0;
};
QT_END_NAMESPACE

#endif
