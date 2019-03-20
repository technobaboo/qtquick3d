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
#include "qdemonrenderpathmanager.h"

#include <QtDemonRender/qdemonrendervertexbuffer.h>
#include <QtDemonRender/qdemonrenderinputassembler.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrendervertexbuffer.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>
#include <QtDemonRender/qdemonrenderpathrender.h>
#include <QtDemonRender/qdemonrenderpathspecification.h>

#include <QtDemonRuntimeRender/qdemonrenderpath.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRuntimeRender/qdemonrendershadercodegenerator.h>
#include <QtDemonRuntimeRender/qdemonrenderdynamicobjectsystem.h>
#include <QtDemonRuntimeRender/qdemonrendercamera.h>
#include <QtDemonRuntimeRender/qdemonrenderpathrendercontext.h>
#include <QtDemonRuntimeRender/qdemonrendershadercodegeneratorv2.h>
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterialshadergenerator.h>
#include <QtDemonRuntimeRender/qdemonrendercustommaterial.h>
#include <QtDemonRuntimeRender/qdemonrendercustommaterialsystem.h>
#include <QtDemonRuntimeRender/qdemonvertexpipelineimpl.h>
#include <QtDemonRuntimeRender/qdemonrendersubpath.h>
#include <QtDemonRuntimeRender/qdemonrenderpathmath.h>
#include <QtDemonRuntimeRender/qdemonrenderinputstreamfactory.h>

#include <QtDemonAssetImport/qdemonpathutilities.h>

QT_BEGIN_NAMESPACE

typedef QDemonPathUtilities::QDemonPathBuffer TImportPathBuffer;
using namespace path;

typedef QPair<QString, QString> TStrStrPair;

struct QDemonPathShaderMapKey
{
    QString m_name;
    QDemonShaderDefaultMaterialKey m_materialKey;
    uint m_hashCode;
    QDemonPathShaderMapKey(const QString &inName, QDemonShaderDefaultMaterialKey inKey)
        : m_name(inName), m_materialKey(inKey)
    {
        m_hashCode = qHash(m_name) ^ m_materialKey.hash();
    }
    bool operator==(const QDemonPathShaderMapKey &inKey) const
    {
        return m_hashCode == inKey.m_hashCode && m_name == inKey.m_name && m_materialKey == inKey.m_materialKey;
    }
};

uint qHash(const QDemonPathShaderMapKey &inKey)
{
    return inKey.m_hashCode;
}

namespace {

struct QDemonPathSubPathBuffer
{
    QAtomicInt ref;
    QVector<QDemonPathAnchorPoint> m_sourceData;
    QDemonPathDirtyFlags m_flags;
    QDemonRenderSubPath &m_subPath;
    bool m_closed;

    QDemonPathSubPathBuffer(QDemonRenderSubPath &inSubPath) : m_subPath(inSubPath), m_closed(false) {}
};

struct QDemonImportPathWrapper
{
    QAtomicInt ref;
    QDemonPathUtilities::QDemonPathBuffer *m_path;

    QDemonImportPathWrapper(QDemonPathUtilities::QDemonPathBuffer &inPath) : m_path(&inPath) {}

    ~QDemonImportPathWrapper() { delete m_path; }
};

typedef QDemonRef<QDemonImportPathWrapper> TPathBufferPtr;

struct QDemonPathBuffer
{
    QAtomicInt ref;
    QVector<QDemonRef<QDemonPathSubPathBuffer>> m_subPaths;
    TPathBufferPtr m_pathBuffer;

    QDemonRef<QDemonRenderVertexBuffer> m_patchData;
    QDemonRef<QDemonRenderInputAssembler> m_inputAssembler;
    QDemonRef<QDemonRenderPathRender> m_pathRender;

    QVector2D m_beginTaperData;
    QVector2D m_endTaperData;
    quint32 m_numVertexes{ 0 };
    QDemonRenderPath::PathType m_pathType{ QDemonRenderPath::PathType::Geometry };
    float m_width{ 0.0f };
    float m_cpuError{ 0.0f };
    QDemonBounds3 m_bounds = QDemonBounds3::empty();
    QDemonOption<QDemonTaperInformation> m_beginTaper;
    QDemonOption<QDemonTaperInformation> m_endTaper;
    QString m_sourcePath;

    // Cached data for geometry paths

    QDemonPathDirtyFlags m_flags;

    void clearGeometryPathData()
    {
        m_patchData = nullptr;
        m_inputAssembler = nullptr;
    }

    void clearPaintedPathData() { m_pathRender = nullptr; }

    QDemonPathUtilities::QDemonPathBuffer getPathData(QDemonPathUtilities::QDemonPathBufferBuilder &inSpec)
    {
        if (m_subPaths.size()) {
            inSpec.clear();
            for (quint32 idx = 0, end = m_subPaths.size(); idx < end; ++idx) {
                const QDemonPathSubPathBuffer &theSubPathBuffer(*m_subPaths[idx]);
                for (quint32 equationIdx = 0, equationEnd = theSubPathBuffer.m_sourceData.size(); equationIdx < equationEnd;
                     ++equationIdx) {
                    const QDemonPathAnchorPoint &thePoint = theSubPathBuffer.m_sourceData[equationIdx];
                    if (equationIdx == 0) {
                        inSpec.moveTo(thePoint.position);
                    } else {
                        const QDemonPathAnchorPoint &thePrevPoint = theSubPathBuffer.m_sourceData[equationIdx - 1];
                        QVector2D c1 = QDemonPathManagerInterface::getControlPointFromAngleDistance(thePrevPoint.position,
                                                                                                    thePrevPoint.outgoingAngle,
                                                                                                    thePrevPoint.outgoingDistance);
                        QVector2D c2 = QDemonPathManagerInterface::getControlPointFromAngleDistance(thePoint.position,
                                                                                                    thePoint.incomingAngle,
                                                                                                    thePoint.incomingDistance);
                        QVector2D p2 = thePoint.position;
                        inSpec.cubicCurveTo(c1, c2, p2);
                    }
                }
                if (theSubPathBuffer.m_closed)
                    inSpec.close();
            }
            return inSpec.getPathBuffer();
        } else if (m_pathBuffer)
            return *m_pathBuffer->m_path;
        return QDemonPathUtilities::QDemonPathBuffer();
    }

    void setPathType(QDemonRenderPath::PathType inPathType)
    {
        if (inPathType != m_pathType) {
            switch (m_pathType) {
            case QDemonRenderPath::PathType::Geometry:
                clearGeometryPathData();
                break;
            case QDemonRenderPath::PathType::Painted:
                clearPaintedPathData();
                break;
            default:
                Q_UNREACHABLE();
                // No further processing for unexpected path type
                return;
            }
            m_flags |= QDemonPathDirtyFlagValue::PathType;
        }
        m_pathType = inPathType;
    }

    static QDemonOption<QDemonTaperInformation> toTaperInfo(QDemonRenderPath::Capping capping, float capOffset, float capOpacity, float capWidth)
    {
        if (capping == QDemonRenderPath::Capping::None)
            return QDemonEmpty();

        return QDemonTaperInformation(capOffset, capOpacity, capWidth);
    }

    void setBeginTaperInfo(QDemonRenderPath::Capping capping, float capOffset, float capOpacity, float capWidth)
    {
        QDemonOption<QDemonTaperInformation> newBeginInfo = toTaperInfo(capping, capOffset, capOpacity, capWidth);
        if (newBeginInfo != m_beginTaper) {
            m_beginTaper = newBeginInfo;
            m_flags |= QDemonPathDirtyFlagValue::BeginTaper;
        }
    }

    void setEndTaperInfo(QDemonRenderPath::Capping capping, float capOffset, float capOpacity, float capWidth)
    {
        QDemonOption<QDemonTaperInformation> newEndInfo = toTaperInfo(capping, capOffset, capOpacity, capWidth);
        if (newEndInfo != m_endTaper) {
            m_endTaper = newEndInfo;
            m_flags |= QDemonPathDirtyFlagValue::EndTaper;
        }
    }

    void setWidth(float inWidth)
    {
        if (inWidth != m_width) {
            m_width = inWidth;
            m_flags |= QDemonPathDirtyFlagValue::Width;
        }
    }

    void setCPUError(float inError)
    {
        if (inError != m_cpuError) {
            m_cpuError = inError;
            m_flags |= QDemonPathDirtyFlagValue::CPUError;
        }
    }
};

struct QDemonPathGeneratedShader
{
    QDemonRef<QDemonRenderShaderProgram> m_shader;
    QDemonRenderCachedShaderProperty<float> m_width;
    QDemonRenderCachedShaderProperty<float> m_innerTessAmount;
    QDemonRenderCachedShaderProperty<float> m_edgeTessAmount;
    QDemonRenderCachedShaderProperty<QVector2D> m_beginTaperData;
    QDemonRenderCachedShaderProperty<QVector2D> m_endTaperData;
    QDemonRenderCachedShaderProperty<QMatrix4x4> m_wireframeViewMatrix;

    QDemonPathGeneratedShader(const QDemonRef<QDemonRenderShaderProgram> &sh)
        : m_shader(sh)
        , m_width("pathWidth", sh)
        , m_innerTessAmount("tessInnerLevel", sh)
        , m_edgeTessAmount("tessEdgeLevel", sh)
        , m_beginTaperData("beginTaperInfo", sh)
        , m_endTaperData("endTaperInfo", sh)
        , m_wireframeViewMatrix("viewport_matrix", sh)
    {
    }
};

struct QDemonPathVertexPipeline : public QDemonVertexPipelineImpl
{

    QDemonPathVertexPipeline(const QDemonRef<QDemonShaderProgramGeneratorInterface> &inProgGenerator,
                             const QDemonRef<QDemonMaterialShaderGeneratorInterface> &inMaterialGenerator,
                             bool inWireframe)
        : QDemonVertexPipelineImpl(inMaterialGenerator, inProgGenerator, inWireframe)
    {
    }

    void assignTessEvalVarying(const QByteArray &inVarName, const QByteArray &inVarValueExpr)
    {
        QByteArray ext;
        if (programGenerator()->getEnabledStages() & QDemonShaderGeneratorStage::Geometry)
            ext = "TE";
        tessEval() << "\t" << inVarName << ext << " = " << inVarValueExpr << ";"
                   << "\n";
    }

    void assignOutput(const QByteArray &inVarName, const QByteArray &inVarValueExpr) override
    {
        assignTessEvalVarying(inVarName, inVarValueExpr);
    }

    void initializeTessShaders()
    {
        QDemonShaderStageGeneratorInterface &theTessControl(tessControl());
        QDemonShaderStageGeneratorInterface &theTessEval(tessEval());

        // first setup tessellation control shader
        theTessControl.addUniform("tessEdgeLevel", "float");
        theTessControl.addUniform("tessInnerLevel", "float");

        theTessControl.addInclude("tessellationPath.glsllib");

        theTessControl.append("void main() {\n");
        theTessControl.append("\tgl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;");
        theTessControl.append("\ttessShader( tessEdgeLevel, tessInnerLevel );\n");

        bool hasGeometryShader = programGenerator()->getStage(QDemonShaderGeneratorStage::Geometry) != nullptr;

        // second setup tessellation control shader
        QByteArray outExt("");
        if (hasGeometryShader)
            outExt = "TE";

        theTessEval.addInclude("tessellationPath.glsllib");
        theTessEval.addUniform("normal_matrix", "mat3");
        theTessEval.addUniform("model_view_projection", "mat4");
        theTessEval.addUniform("pathWidth", "float");
        theTessEval.addUniform("material_diffuse", "vec4");
        addInterpolationParameter("varTexCoord0", "vec2");
        addInterpolationParameter("varTessOpacity", "float");

        theTessEval.append("void main() {\n");
        theTessEval.append("\tSTessShaderResult shaderResult = tessShader( pathWidth );\n");
        theTessEval.append("\tvec3 pos = shaderResult.m_Position;\n");
        assignTessEvalVarying("varTessOpacity", "shaderResult.m_Opacity");
        assignTessEvalVarying("varTexCoord0", "shaderResult.m_TexCoord.xy");
        if (hasGeometryShader)
            theTessEval << "\tvec2 varTexCoord0 = shaderResult.m_TexCoord.xy;\n";

        theTessEval << "\tvec3 object_normal = vec3(0.0, 0.0, 1.0);\n";
        theTessEval << "\tvec3 world_normal = normal_matrix * object_normal;\n";
        theTessEval << "\tvec3 tangent = vec3( shaderResult.m_Tangent, 0.0 );\n";
        theTessEval << "\tvec3 binormal = vec3( shaderResult.m_Binormal, 0.0 );\n";

        // These are necessary for texture generation.
        theTessEval << "\tvec3 uTransform;"
                    << "\n";
        theTessEval << "\tvec3 vTransform;"
                    << "\n";

        if (m_displacementImage) {
            materialGenerator()->generateImageUVCoordinates(*this, m_displacementIdx, 0, *m_displacementImage);
            theTessEval.addUniform("displaceAmount", "float");
            theTessEval.addUniform("model_matrix", "mat4");
            theTessEval.addInclude("defaultMaterialFileDisplacementTexture.glsllib");
            QDemonDefaultMaterialShaderGeneratorInterface::ImageVariableNames theVarNames = materialGenerator()->getImageVariableNames(
                    m_displacementIdx);

            theTessEval.addUniform(theVarNames.m_imageSampler, "sampler2D");
            QDemonDefaultMaterialShaderGeneratorInterface::ImageVariableNames theNames = materialGenerator()->getImageVariableNames(
                    m_displacementIdx);
            theTessEval << "\tpos = defaultMaterialFileDisplacementTexture( " << theNames.m_imageSampler
                        << ", displaceAmount, " << theNames.m_imageFragCoords << outExt << ", vec3( 0.0, 0.0, 1.0 )"
                        << ", pos.xyz );"
                        << "\n";
        }
    }
    void finalizeTessControlShader() {}

    void finalizeTessEvaluationShader()
    {
        // ### Investigate whether the outExp should be used
        //        QString outExt("");
        //        if (programGenerator()->getEnabledStages() & ShaderGeneratorStages::Geometry)
        //            outExt = "TE";

        QDemonShaderStageGeneratorInterface &tessEvalShader(*programGenerator()->getStage(QDemonShaderGeneratorStage::TessEval));
        tessEvalShader.append("\tgl_Position = model_view_projection * vec4( pos, 1.0 );\n");
    }

    void beginVertexGeneration(quint32 displacementImageIdx, QDemonRenderableImage *displacementImage) override
    {
        setupDisplacement(displacementImageIdx, displacementImage);

        QDemonShaderGeneratorStageFlags theStages(QDemonShaderProgramGeneratorInterface::defaultFlags());
        theStages |= QDemonShaderGeneratorStage::TessControl;
        theStages |= QDemonShaderGeneratorStage::TessEval;
        if (m_wireframe) {
            theStages |= QDemonShaderGeneratorStage::Geometry;
        }
        programGenerator()->beginProgram(theStages);
        initializeTessShaders();
        if (m_wireframe) {
            initializeWireframeGeometryShader();
        }
        // Open up each stage.
        QDemonShaderStageGeneratorInterface &vertexShader(vertex());

        vertexShader.addIncoming("attr_pos", "vec4");

        // useless vert shader because real work is done in TES.
        vertexShader << "void main()\n"
                        "{\n";
        vertexShader << "\tgl_Position = attr_pos;\n"; // if tessellation is enabled pass down
        // object coordinates;
        vertexShader << "}\n";
    }

    void beginFragmentGeneration() override
    {
        fragment().addUniform("material_diffuse", "vec4");
        fragment() << "void main()"
                   << "\n"
                   << "{"
                   << "\n";
        // We do not pass object opacity through the pipeline.
        fragment() << "\tfloat object_opacity = varTessOpacity * material_diffuse.a;"
                   << "\n";
    }
    void doGenerateUVCoords(quint32) override
    {
        // these are always generated regardless
    }

    // fragment shader expects varying vertex normal
    // lighting in vertex pipeline expects world_normal
    void doGenerateWorldNormal() override { assignTessEvalVarying("varNormal", "world_normal"); }
    void doGenerateObjectNormal() override { assignTessEvalVarying("varObjectNormal", "object_normal"); }
    void doGenerateWorldPosition() override
    {
        tessEval().addUniform("model_matrix", "mat4");
        tessEval() << "\tvec3 local_model_world_position = vec3((model_matrix * vec4(pos, 1.0)).xyz);\n";
    }
    void doGenerateVarTangentAndBinormal() override
    {
        tessEval().addUniform("normal_matrix", "mat3");
        assignOutput("varTangent", "normal_matrix * tangent");
        assignOutput("varBinormal", "normal_matrix * binormal");
    }

    void doGenerateVertexColor() override
    {
        vertex().addIncoming("attr_color", "vec3");
        vertex() << "\tvarColor = attr_color;"
                 << "\n";
    }

    void endVertexGeneration() override
    {

        if (hasTessellation()) {
            // finalize tess control shader
            finalizeTessControlShader();
            // finalize tess evaluation shader
            finalizeTessEvaluationShader();

            tessControl().append("}");
            tessEval().append("}");
        }
        if (m_wireframe) {
            // finalize geometry shader
            finalizeWireframeGeometryShader();
            geometry().append("}");
        }
    }

    void endFragmentGeneration() override { fragment().append("}"); }

    void addInterpolationParameter(const QByteArray &inName, const QByteArray &inType) override
    {
        m_interpolationParameters.insert(inName, inType);
        fragment().addIncoming(inName, inType);
        if (hasTessellation()) {
            QByteArray nameBuilder;
            nameBuilder = inName;
            if (programGenerator()->getEnabledStages() & QDemonShaderGeneratorStage::Geometry)
                nameBuilder.append("TE");

            tessEval().addOutgoing(nameBuilder, inType);
        }
    }

    QDemonShaderStageGeneratorInterface &activeStage() override { return tessEval(); }
};

struct QDemonPathXYGeneratedShader
{
    QDemonRef<QDemonRenderShaderProgram> m_shader;
    QDemonRenderCachedShaderProperty<QVector4D> m_rectDimensions;
    QDemonRenderCachedShaderProperty<QMatrix4x4> m_modelMatrix;
    QDemonRenderCachedShaderProperty<QVector3D> m_cameraPosition;
    QDemonRenderCachedShaderProperty<QVector2D> m_cameraProperties;

    QDemonPathXYGeneratedShader(const QDemonRef<QDemonRenderShaderProgram> &sh)
        : m_shader(sh)
        , m_rectDimensions("uni_rect_dimensions", sh)
        , m_modelMatrix("model_matrix", sh)
        , m_cameraPosition("camera_position", sh)
        , m_cameraProperties("camera_properties", sh)
    {
    }
    virtual ~QDemonPathXYGeneratedShader() = default;
};

// Helper implements the vertex pipeline for mesh subsets when bound to the default material.
// Should be completely possible to use for custom materials with a bit of refactoring.
struct QDemonXYRectVertexPipeline : public QDemonVertexPipelineImpl
{

    QDemonXYRectVertexPipeline(QDemonRef<QDemonShaderProgramGeneratorInterface> inProgGenerator,
                               QDemonRef<QDemonMaterialShaderGeneratorInterface> inMaterialGenerator)
        : QDemonVertexPipelineImpl(inMaterialGenerator, inProgGenerator, false)
    {
    }

    void beginVertexGeneration(quint32 displacementImageIdx, QDemonRenderableImage *displacementImage) override
    {
        m_displacementIdx = displacementImageIdx;
        m_displacementImage = displacementImage;

        QDemonShaderGeneratorStageFlags theStages(QDemonShaderProgramGeneratorInterface::defaultFlags());
        programGenerator()->beginProgram(theStages);
        // Open up each stage.
        QDemonShaderStageGeneratorInterface &vertexShader(vertex());
        vertexShader.addIncoming("attr_pos", "vec2");
        vertexShader.addUniform("uni_rect_dimensions", "vec4");

        vertexShader << "void main()"
                     << "\n"
                     << "{"
                     << "\n";
        vertexShader << "\tvec3 uTransform;"
                     << "\n";
        vertexShader << "\tvec3 vTransform;"
                     << "\n";

        vertexShader.addUniform("model_view_projection", "mat4");
        vertexShader << "\tfloat posX = mix( uni_rect_dimensions.x, uni_rect_dimensions.z, attr_pos.x );"
                     << "\n";
        vertexShader << "\tfloat posY = mix( uni_rect_dimensions.y, uni_rect_dimensions.w, attr_pos.y );"
                     << "\n";
        vertexShader << "\tvec3  pos = vec3(posX, posY, 0.0 );"
                     << "\n";
        vertexShader.append("\tgl_Position = model_view_projection * vec4(pos, 1.0);");
    }

    void outputParaboloidDepthShaders()
    {
        QDemonShaderGeneratorStageFlags theStages(QDemonShaderProgramGeneratorInterface::defaultFlags());
        programGenerator()->beginProgram(theStages);
        QDemonShaderStageGeneratorInterface &vertexShader(vertex());
        vertexShader.addIncoming("attr_pos", "vec2");
        vertexShader.addUniform("uni_rect_dimensions", "vec4");
        vertexShader.addUniform("model_view_projection", "mat4");
        vertexShader << "void main()"
                     << "\n"
                     << "{"
                     << "\n";
        vertexShader << "\tfloat posX = mix( uni_rect_dimensions.x, uni_rect_dimensions.z, attr_pos.x );"
                     << "\n";
        vertexShader << "\tfloat posY = mix( uni_rect_dimensions.y, uni_rect_dimensions.w, attr_pos.y );"
                     << "\n";
        vertexShader << "\tvec3 pos = vec3(posX, posY, 0.0 );"
                     << "\n";
        QDemonShaderProgramGeneratorInterface::outputParaboloidDepthTessEval(vertexShader);
        vertexShader << "}"
                     << "\n";

        QDemonShaderProgramGeneratorInterface::outputParaboloidDepthFragment(fragment());
    }

    void outputCubeFaceDepthShaders()
    {
        QDemonShaderGeneratorStageFlags theStages(QDemonShaderProgramGeneratorInterface::defaultFlags());
        programGenerator()->beginProgram(theStages);
        QDemonShaderStageGeneratorInterface &vertexShader(vertex());
        QDemonShaderStageGeneratorInterface &fragmentShader(fragment());
        vertexShader.addIncoming("attr_pos", "vec2");
        vertexShader.addUniform("uni_rect_dimensions", "vec4");
        vertexShader.addUniform("model_matrix", "mat4");
        vertexShader.addUniform("model_view_projection", "mat4");

        vertexShader.addOutgoing("world_pos", "vec4");
        vertexShader.append("void main() {");
        vertexShader.append("   float posX = mix( uni_rect_dimensions.x, uni_rect_dimensions.z, attr_pos.x );");
        vertexShader.append("   float posY = mix( uni_rect_dimensions.y, uni_rect_dimensions.w, attr_pos.y );");
        vertexShader.append("   world_pos = model_matrix * vec4( posX, posY, 0.0, 1.0 );");
        vertexShader.append("   world_pos /= world_pos.w;");
        vertexShader.append("	gl_Position = model_view_projection * vec4( posX, posY, 0.0, 1.0 );");
        vertexShader.append("}");

        fragmentShader.addUniform("camera_position", "vec3");
        fragmentShader.addUniform("camera_properties", "vec2");

        beginFragmentGeneration();
        fragmentShader.append("\tfloat dist = 0.5 * length( world_pos.xyz - camera_position );"); // Why?
        fragmentShader.append("\tdist = (dist - camera_properties.x) / (camera_properties.y - camera_properties.x);");
        fragmentShader.append("\tfragOutput = vec4(dist);");
        fragmentShader.append("}");
    }

    void beginFragmentGeneration() override
    {
        fragment().addUniform("material_diffuse", "vec4");
        fragment() << "void main()"
                   << "\n"
                   << "{"
                   << "\n";
        // We do not pass object opacity through the pipeline.
        fragment() << "\tfloat object_opacity = material_diffuse.a;"
                   << "\n";
    }

    void assignOutput(const QByteArray &inVarName, const QByteArray &inVarValue) override
    {
        vertex() << "\t" << inVarName << " = " << inVarValue << ";\n";
    }
    void doGenerateUVCoords(quint32) override
    {
        vertex() << "\tvarTexCoord0 = attr_pos;"
                 << "\n";
    }

    // fragment shader expects varying vertex normal
    // lighting in vertex pipeline expects world_normal
    void doGenerateWorldNormal() override
    {
        QDemonShaderStageGeneratorInterface &vertexGenerator(vertex());
        vertexGenerator.addUniform("normal_matrix", "mat3");
        vertexGenerator.append("\tvec3 world_normal = normalize(normal_matrix * vec3( 0.0, 0.0, 1.0) ).xyz;");
        vertexGenerator.append("\tvarNormal = world_normal;");
    }

    void doGenerateObjectNormal() override
    {
        addInterpolationParameter("varObjectNormal", "vec3");
        vertex().append("\tvarObjectNormal = vec3(0.0, 0.0, 1.0 );");
    }

    void doGenerateWorldPosition() override
    {
        vertex().append("\tvec3 local_model_world_position = (model_matrix * vec4(pos, 1.0)).xyz;");
        assignOutput("varWorldPos", "local_model_world_position");
    }

    void doGenerateVarTangentAndBinormal() override
    {
        vertex().addIncoming("attr_textan", "vec3");
        vertex().addIncoming("attr_binormal", "vec3");
        vertex() << "\tvarTangent = normal_matrix * vec3(1.0, 0.0, 0.0);"
                 << "\n"
                 << "\tvarBinormal = normal_matrix * vec3(0.0, 1.0, 0.0);"
                 << "\n";
    }

    void doGenerateVertexColor() override
    {
        vertex().addIncoming("attr_color", "vec3");
        vertex() << "\tvarColor = attr_color;"
                 << "\n";
    }

    void endVertexGeneration() override { vertex().append("}"); }

    void endFragmentGeneration() override { fragment().append("}"); }

    void addInterpolationParameter(const QByteArray &inName, const QByteArray &inType) override
    {
        m_interpolationParameters.insert(inName, inType);
        vertex().addOutgoing(inName, inType);
        fragment().addIncoming(inName, inType);
    }

    QDemonShaderStageGeneratorInterface &activeStage() override { return vertex(); }
};

struct QDemonPathManager : public QDemonPathManagerInterface
{
    typedef QHash<QDemonRenderPath *, QDemonRef<QDemonPathBuffer>> TPathBufferHash;
    typedef QHash<QDemonRenderSubPath *, QDemonRef<QDemonPathSubPathBuffer>> TPathSubPathBufferHash;
    typedef QHash<QDemonPathShaderMapKey, QDemonPathGeneratedShader *> TShaderMap;
    typedef QHash<QDemonPathShaderMapKey, QDemonPathXYGeneratedShader *> TPaintedShaderMap;
    typedef QHash<QString, TPathBufferPtr> TStringPathBufferMap;

    QDemonRenderContextCore *m_coreContext;
    QDemonRenderContextInterface *m_renderContext;
    QString m_idBuilder;
    TPathSubPathBufferHash m_subPathBuffers;
    TPathBufferHash m_buffers;
    QVector<QDemonResultCubic> m_subdivResult;
    QVector<float> m_keyPointVec;
    QVector<QVector4D> m_patchBuffer;
    TShaderMap m_pathGeometryShaders;
    TPaintedShaderMap m_pathPaintedShaders;
    TStringPathBufferMap m_sourcePathBufferMap;
    QMutex m_pathBufferMutex;

    QScopedPointer<QDemonPathGeneratedShader> m_depthShader;
    QScopedPointer<QDemonPathGeneratedShader> m_depthDisplacementShader;

    QScopedPointer<QDemonPathXYGeneratedShader> m_paintedDepthShader;
    QScopedPointer<QDemonPathXYGeneratedShader> m_paintedShadowShader;
    QScopedPointer<QDemonPathXYGeneratedShader> m_paintedCubeShadowShader;
    QDemonRef<QDemonRenderInputAssembler> m_paintedRectInputAssembler;
    QDemonRef<QDemonRenderVertexBuffer> m_paintedRectVertexBuffer;
    QDemonRef<QDemonRenderIndexBuffer> m_paintedRectIndexBuffer;

    QVector<QDemonRef<QDemonRenderDepthStencilState>> m_depthStencilStates;

    QDemonRef<QDemonRenderPathSpecification> m_pathSpecification;
    QScopedPointer<QDemonPathUtilities::QDemonPathBufferBuilder> m_pathBuilder;

    QDemonPathManager(QDemonRenderContextCore *inRC) : m_coreContext(inRC), m_renderContext(nullptr) {}

    virtual ~QDemonPathManager() {
        m_paintedRectInputAssembler = nullptr;
        qDeleteAll(m_pathGeometryShaders);
        qDeleteAll(m_pathPaintedShaders);
    }

    // Called during binary load which is heavily threaded.
    void setPathSubPathData(const QDemonRenderSubPath &inPath, QDemonDataView<QDemonPathAnchorPoint> inPathCubicCurves) override
    {
        QMutexLocker locker(&m_pathBufferMutex);
        TPathSubPathBufferHash::iterator inserter = m_subPathBuffers.find((QDemonRenderSubPath *)&inPath);
        if (inserter == m_subPathBuffers.end()) {
            inserter = m_subPathBuffers.insert((QDemonRenderSubPath *)&inPath,
                                               QDemonRef<QDemonPathSubPathBuffer>(new QDemonPathSubPathBuffer(
                                                       const_cast<QDemonRenderSubPath &>(inPath))));
        }

        QDemonRef<QDemonPathSubPathBuffer> theBuffer = inserter.value();
        theBuffer->m_sourceData.clear();
        for (int i = 0; i < inPathCubicCurves.size(); ++i)
            theBuffer->m_sourceData.append(inPathCubicCurves[i]);
        theBuffer->m_flags |= QDemonPathDirtyFlagValue::SourceData;
    }

    QDemonRef<QDemonPathBuffer> getPathBufferObject(const QDemonRenderPath &inPath)
    {
        TPathBufferHash::iterator inserter = m_buffers.find((QDemonRenderPath *)&inPath);
        if (inserter == m_buffers.end())
            inserter = m_buffers.insert((QDemonRenderPath *)&inPath, QDemonRef<QDemonPathBuffer>(new QDemonPathBuffer()));

        return inserter.value();
    }

    QDemonRef<QDemonPathSubPathBuffer> getPathBufferObject(const QDemonRenderSubPath &inSubPath)
    {
        TPathSubPathBufferHash::iterator iter = m_subPathBuffers.find((QDemonRenderSubPath *)&inSubPath);
        if (iter != m_subPathBuffers.end())
            return iter.value();
        return nullptr;
    }

    QDemonDataRef<QDemonPathAnchorPoint> getPathSubPathBuffer(const QDemonRenderSubPath &inPath) override
    {
        QDemonRef<QDemonPathSubPathBuffer> theBuffer = getPathBufferObject(inPath);
        if (theBuffer)
            return toDataRef(theBuffer->m_sourceData.data(), (quint32)theBuffer->m_sourceData.size());
        return QDemonDataRef<QDemonPathAnchorPoint>();
    }

    QDemonDataRef<QDemonPathAnchorPoint> resizePathSubPathBuffer(const QDemonRenderSubPath &inPath, quint32 inNumAnchors) override
    {
        QDemonRef<QDemonPathSubPathBuffer> theBuffer = getPathBufferObject(inPath);
        if (theBuffer == nullptr)
            setPathSubPathData(inPath, QDemonDataView<QDemonPathAnchorPoint>());
        theBuffer = getPathBufferObject(inPath);
        theBuffer->m_sourceData.resize(inNumAnchors);
        theBuffer->m_flags |= QDemonPathDirtyFlagValue::SourceData;
        return toDataRef(theBuffer->m_sourceData.data(), (quint32)theBuffer->m_sourceData.size());
    }

    // This needs to be done using roots of the first derivative.
    QDemonBounds3 getBounds(const QDemonRenderPath &inPath) override
    {
        QDemonBounds3 retval(QDemonBounds3::empty());

        QDemonRef<QDemonPathBuffer> thePathBuffer = getPathBufferObject(inPath);
        if (thePathBuffer) {
            QDemonPathDirtyFlags geomDirtyFlags(QDemonPathDirtyFlagValue::SourceData | QDemonPathDirtyFlagValue::BeginTaper
                                                | QDemonPathDirtyFlagValue::EndTaper | QDemonPathDirtyFlagValue::Width
                                                | QDemonPathDirtyFlagValue::CPUError);

            if ((((quint32)thePathBuffer->m_flags) & (quint32)geomDirtyFlags) == 0) {
                return thePathBuffer->m_bounds;
            }
        }

        for (QDemonRenderSubPath *theSubPath = inPath.m_firstSubPath; theSubPath; theSubPath = theSubPath->m_nextSubPath) {
            QDemonRef<QDemonPathSubPathBuffer> theBuffer = getPathBufferObject(*theSubPath);
            if (!theBuffer)
                continue;

            quint32 numAnchors = theBuffer->m_sourceData.size();
            for (quint32 idx = 0, end = numAnchors; idx < end; ++idx) {
                const QDemonPathAnchorPoint &thePoint(theBuffer->m_sourceData[idx]);
                QVector2D position(thePoint.position);
                retval.include(QVector3D(position.x(), position.y(), 0.0f));
                if (idx) {
                    QVector2D incoming(QDemonPathManagerInterface::getControlPointFromAngleDistance(thePoint.position,
                                                                                                    thePoint.incomingAngle,
                                                                                                    thePoint.incomingDistance));
                    retval.include(QVector3D(incoming.x(), incoming.y(), 0.0f));
                }

                if (idx < (numAnchors - 1)) {
                    QVector2D outgoing(QDemonPathManagerInterface::getControlPointFromAngleDistance(thePoint.position,
                                                                                                    thePoint.outgoingAngle,
                                                                                                    thePoint.outgoingDistance));
                    retval.include(QVector3D(outgoing.x(), outgoing.y(), 0.0f));
                }
            }
        }

        return retval;
    }

    QDemonRef<QDemonPathManagerInterface> onRenderSystemInitialize(QDemonRenderContextInterface *context) override
    {
        m_renderContext = context;
        return this;
    }

    // find a point that will join these two curves *if* they are not first derivative continuous
    static QDemonOption<QVector2D> getAdjoiningPoint(QVector2D prevC2, QVector2D point, QVector2D C1, float pathWidth)
    {
        QVector2D incomingDxDy = (point - prevC2);
        QVector2D outgoingDxDy = (C1 - point);
        incomingDxDy.normalize();
        outgoingDxDy.normalize();
        float determinant = (incomingDxDy.x() * outgoingDxDy.y()) - (incomingDxDy.y() * outgoingDxDy.x());
        if (fabs(determinant) > .001f) {
            float mult = determinant > 0.0f ? 1.0f : -1.0f;
            QVector2D incomingNormal(incomingDxDy.y(), -incomingDxDy.x());
            QVector2D outgoingNormal(outgoingDxDy.y(), -outgoingDxDy.x());

            QVector2D leftEdge = point + mult * incomingNormal * pathWidth;
            QVector2D rightEdge = point + mult * outgoingNormal * pathWidth;

            return (leftEdge + rightEdge) / 2.0f;
        }
        return QDemonEmpty();
    }

    QDemonOption<QPair<quint32, float>> findBreakEquation(float inTaperStart)
    {
        float lengthTotal = 0;
        for (quint32 idx = 0, end = m_subdivResult.size(); idx < end; ++idx) {
            if (lengthTotal + m_subdivResult[idx].m_length > inTaperStart) {
                float breakTValue = (inTaperStart - lengthTotal) / m_subdivResult[idx].m_length;
                QVector<QDemonResultCubic>::iterator breakIter = m_subdivResult.begin() + idx;
                QDemonCubicBezierCurve theCurve(breakIter->m_p1, breakIter->m_c1, breakIter->m_c2, breakIter->m_p2);
                QPair<QDemonCubicBezierCurve, QDemonCubicBezierCurve> subdivCurve = theCurve.splitCubicBezierCurve(breakTValue);
                float originalBreakT = breakIter->m_tStart + (breakIter->m_tStop - breakIter->m_tStart) * breakTValue;
                // Update the existing item to point to the second equation
                breakIter->m_p1 = subdivCurve.second.m_points[0];
                breakIter->m_c1 = subdivCurve.second.m_points[1];
                breakIter->m_c2 = subdivCurve.second.m_points[2];
                breakIter->m_p2 = subdivCurve.second.m_points[3];
                float originalLength = breakIter->m_length;
                float originalStart = breakIter->m_tStart;
                breakIter->m_length *= (1.0f - breakTValue);
                breakIter->m_tStart = originalBreakT;
                QDemonResultCubic newCubic(subdivCurve.first.m_points[0],
                                           subdivCurve.first.m_points[1],
                                           subdivCurve.first.m_points[2],
                                           subdivCurve.first.m_points[3],
                                           breakIter->m_equationIndex,
                                           originalStart,
                                           originalBreakT,
                                           originalLength * breakTValue);

                m_subdivResult.insert(breakIter, newCubic);
                return QPair<quint32, float>(idx, breakTValue);
            }
            lengthTotal += m_subdivResult[idx].m_length;
        }
        return QDemonEmpty();
    }

    bool prepareGeometryPathForRender(const QDemonRenderPath &inPath, QDemonPathBuffer &inPathBuffer)
    {

        m_subdivResult.clear();
        m_keyPointVec.clear();
        const QDemonRenderPath &thePath(inPath);

        inPathBuffer.setBeginTaperInfo(thePath.m_beginCapping, thePath.m_beginCapOffset, thePath.m_beginCapOpacity, thePath.m_beginCapWidth);
        inPathBuffer.setEndTaperInfo(thePath.m_endCapping, thePath.m_endCapOffset, thePath.m_endCapOpacity, thePath.m_endCapWidth);
        inPathBuffer.setWidth(inPath.m_width);
        inPathBuffer.setCPUError(inPath.m_linearError);

        QDemonPathDirtyFlags geomDirtyFlags(QDemonPathDirtyFlagValue::SourceData | QDemonPathDirtyFlagValue::BeginTaper | QDemonPathDirtyFlagValue::EndTaper
                                            | QDemonPathDirtyFlagValue::Width | QDemonPathDirtyFlagValue::CPUError);

        bool retval = false;
        if (!inPathBuffer.m_patchData || (((quint32)inPathBuffer.m_flags) & (quint32)geomDirtyFlags) != 0) {
            QDemonPathUtilities::QDemonPathBuffer thePathData = inPathBuffer.getPathData(*m_pathBuilder);

            quint32 dataIdx = 0;
            QVector2D prevPoint(0, 0);
            quint32 equationIdx = 0;
            for (quint32 commandIdx = 0, commandEnd = thePathData.commands.size(); commandIdx < commandEnd; ++commandIdx) {
                switch (thePathData.commands[commandIdx]) {
                case QDemonPathUtilities::PathCommand::MoveTo:
                    prevPoint = QVector2D(thePathData.data[dataIdx], thePathData.data[dataIdx + 1]);
                    dataIdx += 2;
                    break;
                case QDemonPathUtilities::PathCommand::CubicCurveTo: {
                    QVector2D c1(thePathData.data[dataIdx], thePathData.data[dataIdx + 1]);
                    dataIdx += 2;
                    QVector2D c2(thePathData.data[dataIdx], thePathData.data[dataIdx + 1]);
                    dataIdx += 2;
                    QVector2D p2(thePathData.data[dataIdx], thePathData.data[dataIdx + 1]);
                    dataIdx += 2;
                    outerAdaptiveSubdivideBezierCurve(m_subdivResult,
                                                      m_keyPointVec,
                                                      QDemonCubicBezierCurve(prevPoint, c1, c2, p2),
                                                      qMax(inPath.m_linearError, 1.0f),
                                                      equationIdx);
                    ++equationIdx;
                    prevPoint = p2;
                } break;
                case QDemonPathUtilities::PathCommand::Close:
                    break;

                default:
                    Q_ASSERT(false);
                    break;
                }
            }

            float theLocalWidth = inPath.m_width / 2.0f;

            QVector2D theBeginTaperData(theLocalWidth, thePath.globalOpacity);
            QVector2D theEndTaperData(theLocalWidth, thePath.globalOpacity);

            float pathLength = 0.0f;
            for (quint32 idx = 0, end = m_subdivResult.size(); idx < end; ++idx)
                pathLength += m_subdivResult[idx].m_length;

            if (thePath.m_beginCapping == QDemonRenderPath::Capping::Taper || thePath.m_endCapping == QDemonRenderPath::Capping::Taper) {
                float maxTaperStart = pathLength / 2.0f;
                if (thePath.m_beginCapping == QDemonRenderPath::Capping::Taper) {
                    // Can't start more than halfway across the path.
                    float taperStart = qMin(thePath.m_beginCapOffset, maxTaperStart);
                    float endTaperWidth = thePath.m_beginCapWidth;
                    float endTaperOpacity = thePath.globalOpacity * thePath.m_beginCapOpacity;
                    theBeginTaperData = QVector2D(endTaperWidth, endTaperOpacity);
                    // Find where we need to break the current equations.
                    QDemonOption<QPair<quint32, float>> breakEquationAndT(findBreakEquation(taperStart));
                    if (breakEquationAndT.hasValue()) {
                        quint32 breakEquation = breakEquationAndT->first;

                        float lengthTotal = 0;
                        for (quint32 idx = 0, end = breakEquation; idx <= end; ++idx) {
                            QDemonResultCubic &theCubic = m_subdivResult[idx];
                            theCubic.m_mode = QDemonResultCubic::BeginTaper;

                            theCubic.m_taperMultiplier[0] = lengthTotal / taperStart;
                            lengthTotal += theCubic.m_length;
                            theCubic.m_taperMultiplier[1] = lengthTotal / taperStart;
                        }
                    }
                }
                if (thePath.m_endCapping == QDemonRenderPath::Capping::Taper) {
                    float taperStart = qMin(thePath.m_endCapOffset, maxTaperStart);
                    float endTaperWidth = thePath.m_endCapWidth;
                    float endTaperOpacity = thePath.globalOpacity * thePath.m_endCapOpacity;
                    theEndTaperData = QVector2D(endTaperWidth, endTaperOpacity);
                    // Invert taper start so that the forward search works.
                    QDemonOption<QPair<quint32, float>> breakEquationAndT(findBreakEquation(pathLength - taperStart));

                    if (breakEquationAndT.hasValue()) {
                        quint32 breakEquation = breakEquationAndT->first;
                        ++breakEquation;

                        float lengthTotal = 0;
                        for (quint32 idx = breakEquation, end = m_subdivResult.size(); idx < end; ++idx) {
                            QDemonResultCubic &theCubic = m_subdivResult[idx];
                            theCubic.m_mode = QDemonResultCubic::EndTaper;

                            theCubic.m_taperMultiplier[0] = 1.0f - (lengthTotal / taperStart);
                            lengthTotal += theCubic.m_length;
                            theCubic.m_taperMultiplier[1] = 1.0f - (lengthTotal / taperStart);
                        }
                    }
                }
            }

            QDemonRef<QDemonRenderContext> theRenderContext(m_renderContext->getRenderContext());
            // Create quads out of each point.
            if (m_subdivResult.empty())
                return false;

            // Generate patches.
            m_patchBuffer.clear();
            float pathWidth = thePath.m_width / 2.0f;
            // texture coords
            float texCoordU = 0.0;

            for (quint32 idx = 0, end = m_subdivResult.size(); idx < end; ++idx) {
                // create patches
                QDemonResultCubic thePoint(m_subdivResult[idx]);

                m_patchBuffer.push_back(createVec4(thePoint.m_p1, thePoint.m_c1));
                m_patchBuffer.push_back(createVec4(thePoint.m_c2, thePoint.m_p2));

                // Now we need to take care of cases where the control points of the adjoining
                // SubPaths
                // do not line up; i.e. there is a discontinuity of the 1st derivative
                // The simplest way to do this is to move the edge vertex to a halfway point
                // between a line bisecting the two control lines
                QVector2D incomingAdjoining(thePoint.m_p1);
                QVector2D outgoingAdjoining(thePoint.m_p2);
                if (idx) {
                    QDemonResultCubic previousCurve = m_subdivResult[idx - 1];
                    if (previousCurve.m_equationIndex != thePoint.m_equationIndex) {
                        float anchorWidth = thePoint.getP1Width(pathWidth, theBeginTaperData.x(), theEndTaperData.x());
                        QDemonOption<QVector2D> adjoining = getAdjoiningPoint(previousCurve.m_c2, thePoint.m_p1, thePoint.m_c1, anchorWidth);
                        if (adjoining.hasValue())
                            incomingAdjoining = *adjoining;
                    }
                }
                if (idx < (end - 1)) {
                    QDemonResultCubic nextCurve = m_subdivResult[idx + 1];
                    if (nextCurve.m_equationIndex != thePoint.m_equationIndex) {
                        float anchorWidth = thePoint.getP2Width(pathWidth, theBeginTaperData.x(), theEndTaperData.x());
                        QDemonOption<QVector2D> adjoining = getAdjoiningPoint(thePoint.m_c2, thePoint.m_p2, nextCurve.m_c1, anchorWidth);
                        if (adjoining.hasValue())
                            outgoingAdjoining = *adjoining;
                    }
                }
                m_patchBuffer.push_back(createVec4(incomingAdjoining, outgoingAdjoining));

                QVector4D taperData(0.0f, 0.0f, 0.0f, 0.0f);
                taperData.setX(thePoint.m_taperMultiplier.x());
                taperData.setY(thePoint.m_taperMultiplier.y());
                // Note we could put a *lot* more data into this thing.
                taperData.setZ((float)thePoint.m_mode);
                m_patchBuffer.push_back(taperData);

                // texture coord generation
                // note we only generate u here. v is generated in the tess shader
                // u coord for P1 and C1
                QVector2D udata(texCoordU, texCoordU + (thePoint.m_length / pathLength));
                texCoordU = udata.y();
                m_patchBuffer.push_back(QVector4D(udata.x(), udata.y(), 0.0, 0.0));
            }

            // buffer size is 3.0*4.0*bufSize
            quint32 bufSize = (quint32)m_patchBuffer.size() * sizeof(QVector4D);
            quint32 stride = sizeof(QVector4D);

            if ((!inPathBuffer.m_patchData) || inPathBuffer.m_patchData->size() < bufSize) {
                inPathBuffer.m_patchData = new QDemonRenderVertexBuffer(theRenderContext, QDemonRenderBufferUsageType::Dynamic,
                                                                        stride,
                                                                        toByteView(m_patchBuffer));
                inPathBuffer.m_numVertexes = (quint32)m_patchBuffer.size();
                inPathBuffer.m_inputAssembler = nullptr;
            } else {
                Q_ASSERT(inPathBuffer.m_patchData->size() >= bufSize);
                inPathBuffer.m_patchData->updateBuffer(toByteView(m_patchBuffer));
            }

            if (!inPathBuffer.m_inputAssembler) {
                QDemonRenderVertexBufferEntry theEntries[] = {
                    QDemonRenderVertexBufferEntry("attr_pos", QDemonRenderComponentType::Float32, 4),
                };

                QDemonRenderDrawMode primType = QDemonRenderDrawMode::Patches;

                QDemonRef<QDemonRenderAttribLayout> theLayout = theRenderContext->createAttributeLayout(toDataView(theEntries, 1));
                // How many vertices the TCS shader has access to in order to produce its output
                // array of vertices.
                const quint32 inputPatchVertexCount = 5;
                inPathBuffer.m_inputAssembler = theRenderContext->createInputAssembler(theLayout,
                                                                                       toDataView(inPathBuffer.m_patchData),
                                                                                       nullptr,
                                                                                       toDataView(stride),
                                                                                       toDataView((quint32)0),
                                                                                       primType,
                                                                                       inputPatchVertexCount);
            }
            inPathBuffer.m_beginTaperData = theBeginTaperData;
            inPathBuffer.m_endTaperData = theEndTaperData;

            // cache bounds
            QDemonBounds3 bounds = getBounds(inPath);
            inPathBuffer.m_bounds.minimum = bounds.minimum;
            inPathBuffer.m_bounds.maximum = bounds.maximum;
        }

        return retval;
    }

    QDemonRef<QDemonMaterialShaderGeneratorInterface> getMaterialShaderGenertator(QDemonPathRenderContext &inRenderContext)
    {
        bool isDefaultMaterial = (inRenderContext.material.type == QDemonRenderGraphObject::Type::DefaultMaterial);

        QDemonRef<QDemonMaterialShaderGeneratorInterface> theMaterialGenerator = nullptr;
        if (isDefaultMaterial)
            theMaterialGenerator = m_renderContext->getDefaultMaterialShaderGenerator();
        else
            theMaterialGenerator = m_renderContext->getCustomMaterialShaderGenerator();

        return theMaterialGenerator;
    }

    QString getMaterialNameForKey(QDemonPathRenderContext &inRenderContext)
    {
        bool isDefaultMaterial = (inRenderContext.material.type == QDemonRenderGraphObject::Type::DefaultMaterial);

        if (!isDefaultMaterial) {
            QDemonRef<QDemonMaterialSystem> theMaterialSystem(m_renderContext->getCustomMaterialSystem());
            const QDemonRenderCustomMaterial &theCustomMaterial(
                    reinterpret_cast<const QDemonRenderCustomMaterial &>(inRenderContext.material));

            return theMaterialSystem->getShaderName(theCustomMaterial);
        }

        return QString();
    }

    bool preparePaintedPathForRender(const QDemonRenderPath &inPath, QDemonPathBuffer &inPathBuffer)
    {
        QDemonRef<QDemonRenderContext> theContext(this->m_renderContext->getRenderContext());
        if (!inPathBuffer.m_pathRender || (inPathBuffer.m_flags & QDemonPathDirtyFlagValue::SourceData)) {
            if (!inPathBuffer.m_pathRender) {
                inPathBuffer.m_pathRender = theContext->createPathRender();
            }

            if (inPathBuffer.m_pathRender == nullptr || m_pathSpecification == nullptr) {
                //	Q_ASSERT( false );
                return false;
            }

            m_pathSpecification->reset();
            QDemonPathUtilities::QDemonPathBuffer thePathData = inPathBuffer.getPathData(*m_pathBuilder);

            quint32 dataIdx = 0;
            for (quint32 commandIdx = 0, commandEnd = thePathData.commands.size(); commandIdx < commandEnd; ++commandIdx) {

                switch (thePathData.commands[commandIdx]) {
                case QDemonPathUtilities::PathCommand::MoveTo:
                    m_pathSpecification->moveTo(QVector2D(thePathData.data[dataIdx], thePathData.data[dataIdx + 1]));
                    dataIdx += 2;
                    break;
                case QDemonPathUtilities::PathCommand::CubicCurveTo: {
                    QVector2D c1(thePathData.data[dataIdx], thePathData.data[dataIdx + 1]);
                    dataIdx += 2;
                    QVector2D c2(thePathData.data[dataIdx], thePathData.data[dataIdx + 1]);
                    dataIdx += 2;
                    QVector2D p2(thePathData.data[dataIdx], thePathData.data[dataIdx + 1]);
                    dataIdx += 2;
                    m_pathSpecification->cubicCurveTo(c1, c2, p2);
                } break;
                case QDemonPathUtilities::PathCommand::Close:
                    m_pathSpecification->closePath();
                    break;
                default:
                    Q_ASSERT(false);
                    break;
                }
            }

            inPathBuffer.m_pathRender->setPathSpecification(m_pathSpecification);

            // cache bounds
            QDemonBounds3 bounds = getBounds(inPath);
            inPathBuffer.m_bounds.minimum = bounds.minimum;
            inPathBuffer.m_bounds.maximum = bounds.maximum;

            return true;
        }

        return false;
    }

    bool prepareForRender(const QDemonRenderPath &inPath) override
    {
        QDemonRef<QDemonPathBuffer> thePathBuffer = getPathBufferObject(inPath);
        if (!thePathBuffer) {
            return false;
        }
        QDemonRef<QDemonRenderContext> theContext(this->m_renderContext->getRenderContext());
        if (!m_pathSpecification)
            m_pathSpecification = theContext->createPathSpecification();
        if (!m_pathSpecification)
            return false;
        if (!m_pathBuilder)
            m_pathBuilder.reset(new QDemonPathUtilities::QDemonPathBufferBuilder());

        thePathBuffer->setPathType(inPath.m_pathType);
        bool retval = false;
        if (inPath.m_pathBuffer.isEmpty()) {
            thePathBuffer->m_pathBuffer = nullptr;
            // Ensure the SubPath list is identical and clear, percolating any dirty flags up to the
            // path buffer.
            int subPathIdx = 0;
            for (const QDemonRenderSubPath *theSubPath = inPath.m_firstSubPath; theSubPath;
                 theSubPath = theSubPath->m_nextSubPath, ++subPathIdx) {
                QDemonRef<QDemonPathSubPathBuffer> theSubPathBuffer = getPathBufferObject(*theSubPath);
                if (theSubPathBuffer == nullptr)
                    continue;
                thePathBuffer->m_flags |= theSubPathBuffer->m_flags;

                if (theSubPathBuffer->m_closed != theSubPath->m_closed) {
                    thePathBuffer->m_flags |= QDemonPathDirtyFlagValue::SourceData;
                    theSubPathBuffer->m_closed = theSubPath->m_closed;
                }

                if (thePathBuffer->m_subPaths.size() <= subPathIdx || thePathBuffer->m_subPaths[subPathIdx] != theSubPathBuffer) {
                    thePathBuffer->m_flags |= QDemonPathDirtyFlagValue::SourceData;
                    if (thePathBuffer->m_subPaths.size() <= subPathIdx)
                        thePathBuffer->m_subPaths.push_back(theSubPathBuffer);
                    else
                        thePathBuffer->m_subPaths[subPathIdx] = theSubPathBuffer;
                }

                theSubPathBuffer->m_flags = QDemonPathDirtyFlags();
            }

            if (subPathIdx != thePathBuffer->m_subPaths.size()) {
                thePathBuffer->m_subPaths.resize(subPathIdx);
                thePathBuffer->m_flags |= QDemonPathDirtyFlagValue::SourceData;
            }
        } else {
            thePathBuffer->m_subPaths.clear();
            TStringPathBufferMap::iterator inserter = m_sourcePathBufferMap.find(inPath.m_pathBuffer);
            //            QPair<TStringPathBufferMap::iterator, bool> inserter =
            //                    m_SourcePathBufferMap.insert(inPath.m_PathBuffer, TPathBufferPtr());
            if (inserter == m_sourcePathBufferMap.end()) {
                QSharedPointer<QIODevice> theStream = m_coreContext->inputStreamFactory()->getStreamForFile(inPath.m_pathBuffer);
                if (theStream) {
                    QDemonPathUtilities::QDemonPathBuffer *theNewBuffer = QDemonPathUtilities::QDemonPathBuffer::load(*theStream);
                    if (theNewBuffer)
                        inserter = m_sourcePathBufferMap.insert(inPath.m_pathBuffer,
                                                                QDemonRef<QDemonImportPathWrapper>(
                                                                        new QDemonImportPathWrapper(*theNewBuffer)));
                }
            }
            if (thePathBuffer->m_pathBuffer != inserter.value()) {
                thePathBuffer->m_pathBuffer = inserter.value();
                thePathBuffer->m_flags |= QDemonPathDirtyFlagValue::SourceData;
            }
        }

        if (inPath.m_pathType == QDemonRenderPath::PathType::Geometry)
            retval = prepareGeometryPathForRender(inPath, *thePathBuffer);
        else
            retval = preparePaintedPathForRender(inPath, *thePathBuffer);
        thePathBuffer->m_flags = QDemonPathDirtyFlags();
        return retval;
    }

    void setMaterialProperties(const QDemonRef<QDemonRenderShaderProgram> &inShader,
                               QDemonPathRenderContext &inRenderContext,
                               QDemonLayerGlobalRenderProperties &inRenderProperties)
    {
        QDemonRef<QDemonMaterialShaderGeneratorInterface> theMaterialGenerator = getMaterialShaderGenertator(inRenderContext);
        QDemonRef<QDemonRenderContext> theRenderContext(m_renderContext->getRenderContext());
        theRenderContext->setActiveShader(inShader);

        theMaterialGenerator->setMaterialProperties(inShader,
                                                    inRenderContext.material,
                                                    inRenderContext.cameraVec,
                                                    inRenderContext.mvp,
                                                    inRenderContext.normalMatrix,
                                                    inRenderContext.path.globalTransform,
                                                    inRenderContext.firstImage,
                                                    inRenderContext.opacity,
                                                    inRenderProperties);
    }

    void doRenderGeometryPath(QDemonPathGeneratedShader *inShader,
                              QDemonPathRenderContext &inRenderContext,
                              QDemonLayerGlobalRenderProperties &inRenderProperties,
                              const QDemonRef<QDemonPathBuffer> &inPathBuffer)
    {
        if (inPathBuffer->m_inputAssembler == nullptr)
            return;

        setMaterialProperties(inShader->m_shader, inRenderContext, inRenderProperties);
        QDemonRef<QDemonRenderContext> theRenderContext(m_renderContext->getRenderContext());

        inShader->m_beginTaperData.set(inPathBuffer->m_beginTaperData);
        inShader->m_endTaperData.set(inPathBuffer->m_endTaperData);
        if (inRenderContext.enableWireframe) {
            // we need the viewport matrix
            QRect theViewport(theRenderContext->viewport());
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
            inShader->m_wireframeViewMatrix.set(vpMatrix);
        }

        float tessEdgeValue = qMin(64.0f, qMax(1.0f, inRenderContext.path.m_edgeTessAmount));
        float tessInnerValue = qMin(64.0f, qMax(1.0f, inRenderContext.path.m_innerTessAmount));
        inShader->m_edgeTessAmount.set(tessEdgeValue);
        inShader->m_innerTessAmount.set(tessInnerValue);
        inShader->m_width.set(inRenderContext.path.m_width / 2.0f);
        theRenderContext->setInputAssembler(inPathBuffer->m_inputAssembler);
        theRenderContext->setCullingEnabled(false);
        QDemonRenderDrawMode primType = QDemonRenderDrawMode::Patches;
        theRenderContext->draw(primType, (quint32)inPathBuffer->m_numVertexes, 0);
    }

    QDemonRef<QDemonRenderDepthStencilState> getDepthStencilState()
    {
        QDemonRef<QDemonRenderContext> theRenderContext(m_renderContext->getRenderContext());
        QDemonRenderBoolOp theDepthFunction = theRenderContext->depthFunction();
        bool isDepthEnabled = theRenderContext->isDepthTestEnabled();
        bool isStencilEnabled = theRenderContext->isStencilTestEnabled();
        bool isDepthWriteEnabled = theRenderContext->isDepthWriteEnabled();
        for (quint32 idx = 0, end = m_depthStencilStates.size(); idx < end; ++idx) {
            QDemonRef<QDemonRenderDepthStencilState> theState = m_depthStencilStates[idx];
            if (theState->depthFunction() == theDepthFunction && theState->depthEnabled() == isDepthEnabled
                && theState->depthMask() == isDepthWriteEnabled)
                return theState;
        }
        QDemonRenderStencilFunction theArg(QDemonRenderBoolOp::NotEqual, 0, 0xFF);
        QDemonRenderStencilOperation theOpArg(QDemonRenderStencilOp::Keep, QDemonRenderStencilOp::Keep, QDemonRenderStencilOp::Zero);
        m_depthStencilStates.push_back(
                new QDemonRenderDepthStencilState(theRenderContext, isDepthEnabled, isDepthWriteEnabled,
                                                      theDepthFunction, isStencilEnabled, theArg, theArg, theOpArg, theOpArg));
        return m_depthStencilStates.back();
    }

    static void doSetCorrectiveScale(const QMatrix4x4 &mvp, QMatrix4x4 &outScale, QDemonBounds3 pathBounds)
    {
        // Compute the projected locations for the paraboloid and regular projection
        // and thereby set the appropriate scaling factor.
        QVector3D points[4];
        QVector3D projReg[4], projParab[4];
        points[0] = pathBounds.minimum;
        points[1] = QVector3D(pathBounds.maximum.x(), pathBounds.minimum.y(), pathBounds.minimum.z());
        points[2] = pathBounds.maximum;
        points[3] = QVector3D(pathBounds.minimum.x(), pathBounds.maximum.y(), pathBounds.maximum.z());

        // Do the two different projections.
        for (int i = 0; i < 4; ++i) {
            QVector4D tmp;
            tmp = mat44::transform(mvp, QVector4D(points[i], 1.0f));
            tmp /= tmp.w();
            QVector3D tmp3d(tmp.x(), tmp.y(), tmp.z());
            projReg[i] = tmp3d;
            projParab[i] = tmp3d.normalized();
            projParab[i] /= projParab[i].z() + 1.0f;
        }

        QDemonBounds3 boundsA, boundsB;
        for (int i = 0; i < 4; ++i) {
            boundsA.include(projReg[i]);
            boundsB.include(projParab[i]);
        }
        float xscale = (boundsB.maximum.x() - boundsB.minimum.x()) / (boundsA.maximum.x() - boundsA.minimum.x());
        float yscale = (boundsB.maximum.y() - boundsB.minimum.y()) / (boundsA.maximum.y() - boundsA.minimum.y());
        float zscale = vec3::magnitudeSquared(boundsB.maximum - boundsB.minimum)
                / vec3::magnitudeSquared(boundsA.maximum - boundsA.minimum);
        // The default minimum here is just a stupid figure that looks good on our content because
        // we'd
        // been using it for a little while before.  Just for demo.
        xscale = qMin<float>(0.5333333f, qMin<float>(xscale, yscale));
        yscale = qMin<float>(0.5333333f, qMin<float>(xscale, yscale));
        outScale.scale(xscale, yscale, zscale);
    }

    void doRenderPaintedPath(QDemonPathXYGeneratedShader *inShader,
                             QDemonPathRenderContext &inRenderContext,
                             QDemonLayerGlobalRenderProperties &inRenderProperties,
                             const QDemonRef<QDemonPathBuffer> &inPathBuffer,
                             bool isParaboloidPass = false)
    {
        if (!inPathBuffer->m_pathRender)
            return;
        QDemonRef<QDemonRenderContext> theRenderContext(m_renderContext->getRenderContext());
        if (!m_paintedRectInputAssembler) {
            const QVector2D vertexes[] = {
                QVector2D(0.0, 0.0),
                QVector2D(1.0, 0.0),
                QVector2D(1.0, 1.0),
                QVector2D(0.0, 1.0),
            };

            const quint8 indexes[] = {
                0, 1, 2, 2, 3, 0,
            };

            quint32 stride = sizeof(QVector2D);

            QDemonRenderVertexBufferEntry theBufferEntries[] = {
                QDemonRenderVertexBufferEntry("attr_pos", QDemonRenderComponentType::Float32, 2, 0)
            };

            m_paintedRectVertexBuffer = new QDemonRenderVertexBuffer(theRenderContext, QDemonRenderBufferUsageType::Static,
                                                                     sizeof(QVector2D),
                                                                     toByteView(vertexes, 4));
            m_paintedRectIndexBuffer = new QDemonRenderIndexBuffer(theRenderContext,
                                                                   QDemonRenderBufferUsageType::Static,
                                                                   QDemonRenderComponentType::UnsignedInteger8,
                                                                   toByteView(indexes, 6));
            QDemonRef<QDemonRenderAttribLayout> theAttribLayout = theRenderContext->createAttributeLayout(
                    toDataView(theBufferEntries, 1));
            m_paintedRectInputAssembler = theRenderContext->createInputAssembler(theAttribLayout,
                                                                                 toDataView(m_paintedRectVertexBuffer),
                                                                                 m_paintedRectIndexBuffer,
                                                                                 toDataView(stride),
                                                                                 toDataView((quint32)0),
                                                                                 QDemonRenderDrawMode::Triangles);
        }

        // our current render target needs stencil
        Q_ASSERT(theRenderContext->stencilBits() > 0);

        theRenderContext->setDepthStencilState(getDepthStencilState());

        // http://developer.download.nvidia.com/assets/gamedev/files/Mixing_Path_Rendering_and_3D.pdf
        theRenderContext->setPathStencilDepthOffset(-.05f, -1.0f);

        // Stencil out the geometry.
        QMatrix4x4 pathMdlView;
        // Why is this happening?  Well, it's because the painted-on path rendering is always
        // a flat splatted 2D object.  This is bad because a paraboloid projection demands a very
        // different
        // non-linear space into which we must draw.  Path Rendering does not allow this sort of
        // spatial
        // warping internally, and all we end up passing in as a simple perspective projection.
        // So for the fix, I'm scaling the actual "object" size so that it fits into the correctly
        // projected
        // polygon inside the paraboloid depth pass.  Obviously, this scaling factor is wrong, and
        // not generic
        // enough to cover cases like polygons covering a large spread of the FOV and so on.  It's
        // really
        // just a filthy awful, morally deplorable HACK.  But it's basically the quickest fix at
        // hand.
        // This is also about the only possible approach that *could* work short of rendering the
        // paths in
        // a render-to-texture pass and splatting that texture on a sufficiently tessellated quad.
        // Unless
        // there's a way to program NVPR's internal projection scheme, that is.
        // Geometry-based paths will work out better, I think, because they're actually creating
        // geometry.
        // This is essentially a 2D painting process inside a quad where the actual rendered region
        // isn't
        // exactly where NVPR thinks it should be because they're not projecting points the same
        // way.
        if (isParaboloidPass) {
            doSetCorrectiveScale(inRenderContext.mvp, pathMdlView, inPathBuffer->m_pathRender->getPathObjectStrokeBox());
        }

        bool isStencilEnabled = theRenderContext->isStencilTestEnabled();
        theRenderContext->setStencilTestEnabled(true);
        theRenderContext->setPathProjectionMatrix(inRenderContext.mvp);
        theRenderContext->setPathModelViewMatrix(pathMdlView);

        if (inRenderContext.isStroke) {
            inPathBuffer->m_pathRender->setStrokeWidth(inRenderContext.path.m_width);
            inPathBuffer->m_pathRender->stencilStroke();
        } else
            inPathBuffer->m_pathRender->stencilFill();

        // The stencil buffer will dictate whether this object renders or not.  So we need to ignore
        // the depth test result.
        QDemonRenderBoolOp theDepthFunc = theRenderContext->depthFunction();
        theRenderContext->setDepthFunction(QDemonRenderBoolOp::AlwaysTrue);
        // Now render the path; this resets the stencil buffer.
        setMaterialProperties(inShader->m_shader, inRenderContext, inRenderProperties);
        QDemonBounds3 rectBounds = inPathBuffer->m_pathRender->getPathObjectStrokeBox();
        if (isParaboloidPass) {
            rectBounds.scale(1.570796326795f);
        } // PKC : More of the same ugly hack.
        inShader->m_rectDimensions.set(
                QVector4D(rectBounds.minimum.x(), rectBounds.minimum.y(), rectBounds.maximum.x(), rectBounds.maximum.y()));
        theRenderContext->setInputAssembler(m_paintedRectInputAssembler);
        theRenderContext->setCullingEnabled(false);
        // Render exactly two triangles
        theRenderContext->draw(QDemonRenderDrawMode::Triangles, 6, 0);
        theRenderContext->setStencilTestEnabled(isStencilEnabled);
        theRenderContext->setDepthFunction(theDepthFunc);
    }

    void renderDepthPrepass(QDemonPathRenderContext &inRenderContext,
                            QDemonLayerGlobalRenderProperties inRenderProperties,
                            TShaderFeatureSet inFeatureSet) override
    {
        QDemonRef<QDemonPathBuffer> thePathBuffer = getPathBufferObject(inRenderContext.path);
        if (!thePathBuffer)
            return;

        if (thePathBuffer->m_pathType == QDemonRenderPath::PathType::Geometry) {
            quint32 displacementIdx = 0;
            quint32 imageIdx = 0;
            QDemonRenderableImage *displacementImage = nullptr;

            for (QDemonRenderableImage *theImage = inRenderContext.firstImage; theImage != nullptr && displacementImage == nullptr;
                 theImage = theImage->m_nextImage, ++imageIdx) {
                if (theImage->m_mapType == QDemonImageMapTypes::Displacement) {
                    displacementIdx = imageIdx;
                    displacementImage = theImage;
                }
            }

            QScopedPointer<QDemonPathGeneratedShader> &theDesiredDepthShader = displacementImage == nullptr ? m_depthShader : m_depthDisplacementShader;

            if (!theDesiredDepthShader) {
                QDemonRef<QDemonDefaultMaterialShaderGeneratorInterface> theMaterialGenerator(
                        m_renderContext->getDefaultMaterialShaderGenerator());
                QDemonPathVertexPipeline thePipeline(m_renderContext->getShaderProgramGenerator(), theMaterialGenerator, false);
                thePipeline.beginVertexGeneration(displacementIdx, displacementImage);
                thePipeline.beginFragmentGeneration();
                thePipeline.fragment().append("\tfragOutput = vec4(1.0, 1.0, 1.0, 1.0);");
                thePipeline.endVertexGeneration();
                thePipeline.endFragmentGeneration();
                const char *shaderName = "path depth";
                if (displacementImage)
                    shaderName = "path depth displacement";

                QDemonShaderCacheProgramFlags theFlags;
                QDemonRef<QDemonRenderShaderProgram> theProgram = thePipeline.programGenerator()->compileGeneratedShader(shaderName, theFlags, inFeatureSet);
                if (theProgram)
                    theDesiredDepthShader.reset(new QDemonPathGeneratedShader(theProgram));
            }
            if (theDesiredDepthShader)
                doRenderGeometryPath(theDesiredDepthShader.get(), inRenderContext, inRenderProperties, thePathBuffer);
        } else {
            // painted path, go stroke route for now.
            if (!m_paintedDepthShader) {
                QDemonRef<QDemonDefaultMaterialShaderGeneratorInterface> theMaterialGenerator(
                        m_renderContext->getDefaultMaterialShaderGenerator());
                QDemonXYRectVertexPipeline thePipeline(m_renderContext->getShaderProgramGenerator(), theMaterialGenerator);
                thePipeline.beginVertexGeneration(0, nullptr);
                thePipeline.beginFragmentGeneration();
                thePipeline.fragment().append("\tfragOutput = vec4(1.0, 1.0, 1.0, 1.0);");
                thePipeline.endVertexGeneration();
                thePipeline.endFragmentGeneration();
                QDemonShaderCacheProgramFlags theFlags;
                QDemonRef<QDemonRenderShaderProgram> theProgram = thePipeline.programGenerator()->compileGeneratedShader("path painted depth",
                                                                                                                         theFlags,
                                                                                                                         inFeatureSet);
                if (theProgram)
                    m_paintedDepthShader.reset(new QDemonPathXYGeneratedShader(theProgram));
            }
            if (m_paintedDepthShader)
                doRenderPaintedPath(m_paintedDepthShader.get(), inRenderContext, inRenderProperties, thePathBuffer);
        }
    }

    void renderShadowMapPass(QDemonPathRenderContext &inRenderContext,
                             QDemonLayerGlobalRenderProperties inRenderProperties,
                             TShaderFeatureSet inFeatureSet) override
    {
        QDemonRef<QDemonPathBuffer> thePathBuffer = getPathBufferObject(inRenderContext.path);
        if (!thePathBuffer)
            return;

        if (inRenderContext.material.type != QDemonRenderGraphObject::Type::DefaultMaterial)
            return;

        if (thePathBuffer->m_pathType == QDemonRenderPath::PathType::Painted) {
            // painted path, go stroke route for now.
            if (!m_paintedShadowShader) {
                QDemonRef<QDemonDefaultMaterialShaderGeneratorInterface> theMaterialGenerator(
                        m_renderContext->getDefaultMaterialShaderGenerator());
                QDemonXYRectVertexPipeline thePipeline(m_renderContext->getShaderProgramGenerator(), theMaterialGenerator);
                thePipeline.outputParaboloidDepthShaders();
                QDemonShaderCacheProgramFlags theFlags;
                QDemonRef<QDemonRenderShaderProgram> theProgram = thePipeline.programGenerator()->compileGeneratedShader("path painted paraboloid depth",
                                                                                                                         theFlags,
                                                                                                                         inFeatureSet);
                if (theProgram) {
                    m_paintedShadowShader.reset(new QDemonPathXYGeneratedShader(theProgram));
                }
            }
            if (m_paintedShadowShader) {
                // Setup the shader paraboloid information.
                QDemonRef<QDemonRenderContext> theRenderContext(m_renderContext->getRenderContext());
                theRenderContext->setActiveShader(m_paintedShadowShader->m_shader);

                doRenderPaintedPath(m_paintedShadowShader.get(), inRenderContext, inRenderProperties, thePathBuffer, true);
            }
        } else {
            // Until we've also got a proper path render path for this, we'll call the old-fashioned
            // stuff.
            renderDepthPrepass(inRenderContext, inRenderProperties, inFeatureSet);
            // Q_ASSERT( false );
        }
    }

    void renderCubeFaceShadowPass(QDemonPathRenderContext &inRenderContext,
                                  QDemonLayerGlobalRenderProperties inRenderProperties,
                                  TShaderFeatureSet inFeatureSet) override
    {
        QDemonRef<QDemonPathBuffer> thePathBuffer = getPathBufferObject(inRenderContext.path);
        if (!thePathBuffer)
            return;

        if (inRenderContext.material.type != QDemonRenderGraphObject::Type::DefaultMaterial)
            return;

        if (thePathBuffer->m_pathType == QDemonRenderPath::PathType::Painted) {
            if (!m_paintedCubeShadowShader) {
                QDemonRef<QDemonDefaultMaterialShaderGeneratorInterface> theMaterialGenerator(
                        m_renderContext->getDefaultMaterialShaderGenerator());
                QDemonXYRectVertexPipeline thePipeline(m_renderContext->getShaderProgramGenerator(), theMaterialGenerator);
                thePipeline.outputCubeFaceDepthShaders();
                QDemonShaderCacheProgramFlags theFlags;
                QDemonRef<QDemonRenderShaderProgram> theProgram = thePipeline.programGenerator()->compileGeneratedShader("path painted cube face depth",
                                                                                                                         theFlags,
                                                                                                                         inFeatureSet);
                if (theProgram) {
                    m_paintedCubeShadowShader.reset(new QDemonPathXYGeneratedShader(theProgram));
                }
            }
            if (m_paintedCubeShadowShader) {
                // Setup the shader information.
                QDemonRef<QDemonRenderContext> theRenderContext(m_renderContext->getRenderContext());
                theRenderContext->setActiveShader(m_paintedCubeShadowShader->m_shader);

                m_paintedCubeShadowShader->m_cameraPosition.set(inRenderContext.camera.getGlobalPos());
                m_paintedCubeShadowShader->m_cameraProperties.set(QVector2D(1.0f, inRenderContext.camera.clipFar));
                m_paintedCubeShadowShader->m_modelMatrix.set(inRenderContext.modelMatrix);

                doRenderPaintedPath(m_paintedCubeShadowShader.get(), inRenderContext, inRenderProperties, thePathBuffer, false);
            }
        } else {
            // Until we've also got a proper path render path for this, we'll call the old-fashioned
            // stuff.
            renderDepthPrepass(inRenderContext, inRenderProperties, inFeatureSet);
        }
    }

    void renderPath(QDemonPathRenderContext &inRenderContext,
                    QDemonLayerGlobalRenderProperties inRenderProperties,
                    TShaderFeatureSet inFeatureSet) override
    {
        QDemonRef<QDemonPathBuffer> thePathBuffer = getPathBufferObject(inRenderContext.path);
        if (!thePathBuffer) {
            return;
        }

        bool isDefaultMaterial = (inRenderContext.material.type == QDemonRenderGraphObject::Type::DefaultMaterial);

        if (thePathBuffer->m_pathType == QDemonRenderPath::PathType::Geometry) {
            QDemonRef<QDemonMaterialShaderGeneratorInterface> theMaterialGenerator = getMaterialShaderGenertator(inRenderContext);

            // we need a more evolved key her for custom materials
            // the same key can still need a different shader
            QDemonPathShaderMapKey sPathkey = QDemonPathShaderMapKey(getMaterialNameForKey(inRenderContext),
                                                                     inRenderContext.materialKey);
            TShaderMap::iterator inserter = m_pathGeometryShaders.find(sPathkey);
            // QPair<TShaderMap::iterator, bool> inserter = m_PathGeometryShaders.insert(sPathkey, QDemonRef<SPathGeneratedShader>(nullptr));
            if (inserter == m_pathGeometryShaders.end()) {
                QDemonPathVertexPipeline thePipeline(m_renderContext->getShaderProgramGenerator(),
                                                     theMaterialGenerator,
                                                     m_renderContext->getWireframeMode());

                QDemonRef<QDemonRenderShaderProgram> theProgram = nullptr;

                if (isDefaultMaterial) {
                    theProgram = theMaterialGenerator->generateShader(inRenderContext.material,
                                                                      inRenderContext.materialKey,
                                                                      thePipeline,
                                                                      inFeatureSet,
                                                                      inRenderProperties.lights,
                                                                      inRenderContext.firstImage,
                                                                      inRenderContext.opacity < 1.0,
                                                                      "path geometry pipeline-- ");
                } else {
                    QDemonRef<QDemonMaterialSystem> theMaterialSystem(m_renderContext->getCustomMaterialSystem());
                    const QDemonRenderCustomMaterial &theCustomMaterial(
                            reinterpret_cast<const QDemonRenderCustomMaterial &>(inRenderContext.material));

                    theProgram = theMaterialGenerator
                                         ->generateShader(inRenderContext.material,
                                                          inRenderContext.materialKey,
                                                          thePipeline,
                                                          inFeatureSet,
                                                          inRenderProperties.lights,
                                                          inRenderContext.firstImage,
                                                          inRenderContext.opacity < 1.0,
                                                          "path geometry pipeline-- ",
                                                          theMaterialSystem->getShaderName(theCustomMaterial).toUtf8());
                }

                if (theProgram)
                    inserter = m_pathGeometryShaders.insert(sPathkey, new QDemonPathGeneratedShader(theProgram));
            }
            if (inserter == m_pathGeometryShaders.end())
                return;

            doRenderGeometryPath(inserter.value(), inRenderContext, inRenderProperties, thePathBuffer);
        } else {
            QDemonRef<QDemonMaterialShaderGeneratorInterface> theMaterialGenerator = getMaterialShaderGenertator(inRenderContext);

            // we need a more evolved key her for custom materials
            // the same key can still need a different shader
            QDemonPathShaderMapKey sPathkey = QDemonPathShaderMapKey(getMaterialNameForKey(inRenderContext),
                                                                     inRenderContext.materialKey);
            TPaintedShaderMap::iterator inserter = m_pathPaintedShaders.find(sPathkey);

            if (inserter == m_pathPaintedShaders.end()) {
                QDemonXYRectVertexPipeline thePipeline(m_renderContext->getShaderProgramGenerator(), theMaterialGenerator);

                QDemonRef<QDemonRenderShaderProgram> theProgram = nullptr;

                if (isDefaultMaterial) {
                    theProgram = theMaterialGenerator->generateShader(inRenderContext.material,
                                                                      inRenderContext.materialKey,
                                                                      thePipeline,
                                                                      inFeatureSet,
                                                                      inRenderProperties.lights,
                                                                      inRenderContext.firstImage,
                                                                      inRenderContext.opacity < 1.0,
                                                                      "path painted pipeline-- ");
                } else {
                    QDemonRef<QDemonMaterialSystem> theMaterialSystem(m_renderContext->getCustomMaterialSystem());
                    const QDemonRenderCustomMaterial &theCustomMaterial(
                            reinterpret_cast<const QDemonRenderCustomMaterial &>(inRenderContext.material));

                    theProgram = theMaterialGenerator
                                         ->generateShader(inRenderContext.material,
                                                          inRenderContext.materialKey,
                                                          thePipeline,
                                                          inFeatureSet,
                                                          inRenderProperties.lights,
                                                          inRenderContext.firstImage,
                                                          inRenderContext.opacity < 1.0,
                                                          "path painted pipeline-- ",
                                                          theMaterialSystem->getShaderName(theCustomMaterial).toUtf8());
                }

                if (theProgram)
                    inserter = m_pathPaintedShaders.insert(sPathkey, new QDemonPathXYGeneratedShader(theProgram));
            }
            if (inserter == m_pathPaintedShaders.end())
                return;

            doRenderPaintedPath(inserter.value(), inRenderContext, inRenderProperties, thePathBuffer);
        }
    }
};
}

QDemonPathManagerInterface::~QDemonPathManagerInterface() = default;

QVector2D QDemonPathManagerInterface::getControlPointFromAngleDistance(QVector2D inPosition, float inIncomingAngle, float inIncomingDistance)
{
    if (inIncomingDistance == 0.0f)
        return inPosition;
    const float angleRad = degToRad(inIncomingAngle);
    const float angleSin = std::sin(angleRad);
    const float angleCos = std::cos(angleRad);
    const QVector2D relativeAngles = QVector2D(angleCos * inIncomingDistance, angleSin * inIncomingDistance);
    return inPosition + relativeAngles;
}

QVector2D QDemonPathManagerInterface::getAngleDistanceFromControlPoint(QVector2D inPosition, QVector2D inControlPoint)
{
    const QVector2D relative = inControlPoint - inPosition;
    const float angleRad = std::atan2(relative.y(), relative.x());
    const float distance = vec2::magnitude(relative);
    return QVector2D(radToDeg(angleRad), distance);
}

QDemonRef<QDemonPathManagerInterface> QDemonPathManagerInterface::createPathManager(QDemonRenderContextCore *ctx)
{
    return QDemonRef<QDemonPathManager>(new QDemonPathManager(ctx));
}

QT_END_NAMESPACE
