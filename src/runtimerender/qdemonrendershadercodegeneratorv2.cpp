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
#include "qdemonrendershadercodegeneratorv2.h"

#include <QtDemon/qdemonutils.h>

#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRuntimeRender/qdemonrenderdynamicobjectsystem.h>

#include <QtGui/qopengl.h>

QT_BEGIN_NAMESPACE

namespace {
struct QDemonStageGeneratorBase : public QDemonShaderStageGeneratorInterface
{
    TStrTableStrMap m_incoming;
    TStrTableStrMap *m_outgoing;
    QSet<QByteArray> m_includes;
    TStrTableStrMap m_uniforms;
    TStrTableStrMap m_constantBuffers;
    TConstantBufferParamArray m_constantBufferParams;
    QByteArray m_codeBuilder;
    QByteArray m_finalBuilder;
    QDemonShaderGeneratorStage m_stage;
    QDemonShaderGeneratorStageFlags m_enabledStages;
    QList<QByteArray> m_addedFunctions;

    QDemonStageGeneratorBase(QDemonShaderGeneratorStage inStage)

        : m_outgoing(nullptr), m_stage(inStage)
    {
    }

    virtual void begin(QDemonShaderGeneratorStageFlags inEnabledStages)
    {
        m_incoming.clear();
        m_outgoing = nullptr;
        m_includes.clear();
        m_uniforms.clear();
        m_constantBuffers.clear();
        m_constantBufferParams.clear();
        m_codeBuilder.clear();
        m_finalBuilder.clear();
        m_enabledStages = inEnabledStages;
        m_addedFunctions.clear();
        // the shared buffers will be cleared elsewhere.
    }

    void addIncoming(const QByteArray &name, const QByteArray &type) override { m_incoming.insert(name, type); }
    virtual const QByteArray GetIncomingVariableName() { return "in"; }

    void addOutgoing(const QByteArray &name, const QByteArray &type) override
    {
        if (m_outgoing == nullptr) {
            Q_ASSERT(false);
            return;
        }
        m_outgoing->insert(name, type);
    }

    void addUniform(const QByteArray &name, const QByteArray &type) override { m_uniforms.insert(name, type); }

    void addConstantBuffer(const QByteArray &name, const QByteArray &layout) override
    {
        m_constantBuffers.insert(name, layout);
    }
    void addConstantBufferParam(const QByteArray &cbName, const QByteArray &paramName, const QByteArray &type) override
    {
        TParamPair theParamPair(paramName, type);
        TConstantBufferParamPair theBufferParamPair(cbName, theParamPair);
        m_constantBufferParams.push_back(theBufferParamPair);
    }

    QDemonShaderStageGeneratorInterface &operator<<(const QByteArray &data) override
    {
        m_codeBuilder.append(data);
        return *this;
    }
    void append(const QByteArray &data) override
    {
        m_codeBuilder.append(data);
        m_codeBuilder.append("\n");
    }
    QDemonShaderGeneratorStage stage() const override { return m_stage; }

    virtual void addShaderItemMap(const QByteArray &itemType, const TStrTableStrMap &itemMap, const QByteArray &inItemSuffix = QByteArray())
    {
        m_finalBuilder.append("\n");

        for (TStrTableStrMap::const_iterator iter = itemMap.begin(), end = itemMap.end(); iter != end; ++iter) {
            m_finalBuilder.append(itemType);
            m_finalBuilder.append(" ");
            m_finalBuilder.append(iter.value());
            m_finalBuilder.append(" ");
            m_finalBuilder.append(iter.key());
            m_finalBuilder.append(inItemSuffix);
            m_finalBuilder.append(";\n");
        }
    }

    virtual void addShaderIncomingMap() { addShaderItemMap(GetIncomingVariableName(), m_incoming); }

    virtual void addShaderUniformMap() { addShaderItemMap("uniform", m_uniforms); }

    virtual void addShaderOutgoingMap()
    {
        if (m_outgoing)
            addShaderItemMap("varying", *m_outgoing);
    }

    virtual void addShaderConstantBufferItemMap(const QByteArray &itemType, const TStrTableStrMap &cbMap, TConstantBufferParamArray cbParamsArray)
    {
        m_finalBuilder.append("\n");

        // iterate over all constant buffers
        for (TStrTableStrMap::const_iterator iter = cbMap.begin(), end = cbMap.end(); iter != end; ++iter) {
            m_finalBuilder.append(iter.value());
            m_finalBuilder.append(" ");
            m_finalBuilder.append(itemType);
            m_finalBuilder.append(" ");
            m_finalBuilder.append(iter.key());
            m_finalBuilder.append(" {\n");
            // iterate over all param entries and add match
            for (TConstantBufferParamArray::const_iterator iter1 = cbParamsArray.begin(), end = cbParamsArray.end(); iter1 != end;
                 ++iter1) {
                if (iter1->first == iter.key()) {
                    m_finalBuilder.append(iter1->second.second);
                    m_finalBuilder.append(" ");
                    m_finalBuilder.append(iter1->second.first);
                    m_finalBuilder.append(";\n");
                }
            }

            m_finalBuilder.append("};\n");
        }
    }

    virtual void appendShaderCode() { m_finalBuilder.append(m_codeBuilder); }

    virtual void updateShaderCacheFlags(QDemonShaderCacheProgramFlags &) {}

    void addInclude(const QByteArray &name) override { m_includes.insert(name); }

    virtual QByteArray buildShaderSource()
    {
        auto iter = m_includes.constBegin();
        const auto end = m_includes.constEnd();
        while (iter != end) {
            m_finalBuilder.append("#include \"");
            m_finalBuilder.append(*iter);
            m_finalBuilder.append("\"\n");
            ++iter;
        }
        addShaderIncomingMap();
        addShaderUniformMap();
        addShaderConstantBufferItemMap("uniform", m_constantBuffers, m_constantBufferParams);
        addShaderOutgoingMap();
        m_finalBuilder.append("\n");
        appendShaderCode();

        return m_finalBuilder;
    }

    void addFunction(const QByteArray &functionName) override
    {
        if (!m_addedFunctions.contains(functionName)) {
            m_addedFunctions.push_back(functionName);
            QByteArray includeName;
            includeName = "func" + functionName + ".glsllib";
            addInclude(includeName);
        }
    }
};

struct QDemonVertexShaderGenerator : public QDemonStageGeneratorBase
{
    QDemonVertexShaderGenerator() : QDemonStageGeneratorBase(QDemonShaderGeneratorStage::Vertex) {}

    const QByteArray GetIncomingVariableName() override { return "attribute"; }
    virtual void AddIncomingInterpolatedMap() {}

    virtual const QByteArray GetInterpolatedIncomingSuffix() const { return "_attr"; }
    virtual const QByteArray GetInterpolatedOutgoingSuffix() const { return ""; }
};

struct QDemonTessControlShaderGenerator : public QDemonStageGeneratorBase
{
    QDemonTessControlShaderGenerator() : QDemonStageGeneratorBase(QDemonShaderGeneratorStage::TessControl) {}

    void addShaderIncomingMap() override { addShaderItemMap("attribute", m_incoming, "[]"); }

    void addShaderOutgoingMap() override
    {
        if (m_outgoing)
            addShaderItemMap("varying", *m_outgoing, "[]");
    }

    void updateShaderCacheFlags(QDemonShaderCacheProgramFlags &inFlags) override
    {
        inFlags |= ShaderCacheProgramFlagValues::TessellationEnabled;
    }
};

struct QDemonTessEvalShaderGenerator : public QDemonStageGeneratorBase
{
    QDemonTessEvalShaderGenerator() : QDemonStageGeneratorBase(QDemonShaderGeneratorStage::TessEval) {}

    void addShaderIncomingMap() override { addShaderItemMap("attribute", m_incoming, "[]"); }

    void updateShaderCacheFlags(QDemonShaderCacheProgramFlags &inFlags) override
    {
        inFlags |= ShaderCacheProgramFlagValues::TessellationEnabled;
    }
};

struct QDemonGeometryShaderGenerator : public QDemonStageGeneratorBase
{
    QDemonGeometryShaderGenerator() : QDemonStageGeneratorBase(QDemonShaderGeneratorStage::Geometry) {}

    void addShaderIncomingMap() override { addShaderItemMap("attribute", m_incoming, "[]"); }

    void addShaderOutgoingMap() override
    {
        if (m_outgoing)
            addShaderItemMap("varying", *m_outgoing);
    }
    void updateShaderCacheFlags(QDemonShaderCacheProgramFlags &inFlags) override
    {
        inFlags |= ShaderCacheProgramFlagValues::GeometryShaderEnabled;
    }
};

struct QDemonFragmentShaderGenerator : public QDemonStageGeneratorBase
{
    QDemonFragmentShaderGenerator() : QDemonStageGeneratorBase(QDemonShaderGeneratorStage::Fragment) {}
    void addShaderIncomingMap() override { addShaderItemMap("varying", m_incoming); }
    void addShaderOutgoingMap() override {}
};

struct QDemonShaderGeneratedProgramOutput
{
    // never null; so safe to call strlen on.
    const char *m_vertexShader{ "" };
    const char *m_tessControlShader{ "" };
    const char *m_tessEvalShader{ "" };
    const char *m_geometryShader{ "" };
    const char *m_fragmentShader{ "" };

    QDemonShaderGeneratedProgramOutput() = default;
    QDemonShaderGeneratedProgramOutput(const char *vs, const char *tc, const char *te, const char *gs, const char *fs)
        : m_vertexShader(vs), m_tessControlShader(tc), m_tessEvalShader(te), m_geometryShader(gs), m_fragmentShader(fs)
    {
    }
};

struct QDemonProgramGenerator : public QDemonShaderProgramGeneratorInterface
{
    QDemonRenderContextInterface *m_context;
    QDemonVertexShaderGenerator m_vs;
    QDemonTessControlShaderGenerator m_tc;
    QDemonTessEvalShaderGenerator m_te;
    QDemonGeometryShaderGenerator m_gs;
    QDemonFragmentShaderGenerator m_fs;

    QDemonShaderGeneratorStageFlags m_enabledStages;

    QDemonProgramGenerator(QDemonRenderContextInterface *inContext) : m_context(inContext) {}

    void linkStages()
    {
        // Link stages incoming to outgoing variables.
        QDemonStageGeneratorBase *previous = nullptr;
        quint32 theStageId = 1;
        for (quint32 idx = 0, end = quint32(QDemonShaderGeneratorStage::StageCount); idx < end; ++idx, theStageId = theStageId << 1) {
            QDemonStageGeneratorBase *thisStage = nullptr;
            QDemonShaderGeneratorStage theStageEnum = static_cast<QDemonShaderGeneratorStage>(theStageId);
            if ((m_enabledStages & theStageEnum)) {
                thisStage = &internalGetStage(theStageEnum);
                if (previous)
                    previous->m_outgoing = &thisStage->m_incoming;
                previous = thisStage;
            }
        }
    }

    void beginProgram(QDemonShaderGeneratorStageFlags inEnabledStages) override
    {
        m_vs.begin(inEnabledStages);
        m_tc.begin(inEnabledStages);
        m_te.begin(inEnabledStages);
        m_gs.begin(inEnabledStages);
        m_fs.begin(inEnabledStages);
        m_enabledStages = inEnabledStages;
        linkStages();
    }

    QDemonShaderGeneratorStageFlags getEnabledStages() const override { return m_enabledStages; }

    QDemonStageGeneratorBase &internalGetStage(QDemonShaderGeneratorStage inStage)
    {
        switch (inStage) {
        case QDemonShaderGeneratorStage::Vertex:
            return m_vs;
        case QDemonShaderGeneratorStage::TessControl:
            return m_tc;
        case QDemonShaderGeneratorStage::TessEval:
            return m_te;
        case QDemonShaderGeneratorStage::Geometry:
            return m_gs;
        case QDemonShaderGeneratorStage::Fragment:
            return m_fs;
        default:
            Q_ASSERT(false);
            break;
        }
        return m_vs;
    }
    // get the stage or nullptr if it has not been created.
    QDemonShaderStageGeneratorInterface *getStage(QDemonShaderGeneratorStage inStage) override
    {
        if ((m_enabledStages & inStage))
            return &internalGetStage(inStage);
        return nullptr;
    }

    QDemonRef<QDemonRenderShaderProgram> compileGeneratedShader(const QByteArray &inShaderName,
                                                                const QDemonShaderCacheProgramFlags &inFlags,
                                                                const TShaderFeatureSet &inFeatureSet,
                                                                bool separableProgram) override
    {
        // No stages enabled
        if (((quint32)m_enabledStages) == 0) {
            Q_ASSERT(false);
            return nullptr;
        }

        QDemonRef<QDemonDynamicObjectSystem> theDynamicSystem(m_context->dynamicObjectSystem());
        QDemonShaderCacheProgramFlags theCacheFlags(inFlags);
        for (quint32 stageIdx = 0; stageIdx < static_cast<quint32>(QDemonShaderGeneratorStage::StageCount); ++stageIdx) {
            QDemonShaderGeneratorStage stageName = static_cast<QDemonShaderGeneratorStage>(1 << stageIdx);
            if (m_enabledStages & stageName) {
                QDemonStageGeneratorBase &theStage(internalGetStage(stageName));
                theStage.buildShaderSource();
                theStage.updateShaderCacheFlags(theCacheFlags);
                theDynamicSystem->insertShaderHeaderInformation(theStage.m_finalBuilder, inShaderName);
            }
        }

        const QDemonRef<QDemonShaderCache> &theCache = m_context->shaderCache();
        return theCache->compileProgram(inShaderName,
                                        m_vs.m_finalBuilder,
                                        m_fs.m_finalBuilder,
                                        m_tc.m_finalBuilder,
                                        m_te.m_finalBuilder,
                                        m_gs.m_finalBuilder,
                                        theCacheFlags,
                                        inFeatureSet,
                                        separableProgram);
    }
};
};

QDemonRef<QDemonRenderShaderProgram> QDemonShaderProgramGeneratorInterface::compileGeneratedShader(const QByteArray &inShaderName,
                                                                                                   bool separableProgram)
{
    return compileGeneratedShader(inShaderName, QDemonShaderCacheProgramFlags(), TShaderFeatureSet(), separableProgram);
}

QDemonRef<QDemonShaderProgramGeneratorInterface> QDemonShaderProgramGeneratorInterface::createProgramGenerator(QDemonRenderContextInterface *inContext)
{
    return QDemonRef<QDemonShaderProgramGeneratorInterface>(new QDemonProgramGenerator(inContext));
}

void QDemonShaderProgramGeneratorInterface::outputParaboloidDepthVertex(QDemonShaderStageGeneratorInterface &vertexShader)
{
    vertexShader.addIncoming("attr_pos", "vec3");
    vertexShader.addInclude("shadowMapping.glsllib");
    vertexShader.addUniform("model_view_projection", "mat4");
    // vertexShader.AddUniform("model_view", "mat4");
    vertexShader.addUniform("camera_properties", "vec2");
    // vertexShader.AddOutgoing("view_pos", "vec4");
    vertexShader.addOutgoing("world_pos", "vec4");

    // Project the location onto screen space.
    // This will be horrible if you have a single large polygon.  Tessellation is your friend here!
    vertexShader.append("void main() {\n"
                        "   ParaboloidMapResult data = VertexParaboloidDepth( attr_pos, model_view_projection );\n"
                        "   gl_Position = data.m_Position;\n"
                        "   world_pos = data.m_WorldPos;\n"
                        "}\n");
}

void QDemonShaderProgramGeneratorInterface::outputParaboloidDepthTessEval(QDemonShaderStageGeneratorInterface &tessEvalShader)
{
    tessEvalShader.addInclude("shadowMapping.glsllib");
    tessEvalShader.addUniform("model_view_projection", "mat4");
    tessEvalShader.addOutgoing("world_pos", "vec4");
    tessEvalShader.append(
                "   ParaboloidMapResult data = VertexParaboloidDepth( vec3(pos.xyz), model_view_projection );\n"
                "   gl_Position = data.m_Position;\n"
                "   world_pos = data.m_WorldPos;\n");
}

void QDemonShaderProgramGeneratorInterface::outputParaboloidDepthFragment(QDemonShaderStageGeneratorInterface &fragmentShader)
{
    fragmentShader.addInclude("shadowMappingFragment.glsllib");
    fragmentShader.addUniform("model_view_projection", "mat4");
    fragmentShader.addUniform("camera_properties", "vec2");
    fragmentShader.append(
                "void main() {\n"
                "   gl_FragDepth = FragmentParaboloidDepth( world_pos, model_view_projection, camera_properties );\n"
                "}"
                );
}

void QDemonShaderProgramGeneratorInterface::outputCubeFaceDepthVertex(QDemonShaderStageGeneratorInterface &vertexShader)
{
    vertexShader.addIncoming("attr_pos", "vec3");
    vertexShader.addUniform("model_matrix", "mat4");
    vertexShader.addUniform("model_view_projection", "mat4");

    vertexShader.addOutgoing("raw_pos", "vec4");
    vertexShader.addOutgoing("world_pos", "vec4");

    vertexShader.append("void main() {\n"
                        "   world_pos = model_matrix * vec4( attr_pos, 1.0 );\n"
                        "   world_pos /= world_pos.w;\n"
                        "	gl_Position = model_view_projection * vec4( attr_pos, 1.0 );\n"
                        "   raw_pos = vec4( attr_pos, 1.0 );\n"
                        //	vertexShader->Append("   gl_Position = vec4( attr_pos, 1.0 );\n"
                        "}");
}

void QDemonShaderProgramGeneratorInterface::outputCubeFaceDepthGeometry(QDemonShaderStageGeneratorInterface &geometryShader)
{
    geometryShader.append("layout(triangles) in;\n"
                          "layout(triangle_strip, max_vertices = 18) out;");
    // geometryShader->AddUniform("shadow_mvp[6]", "mat4");

    geometryShader.addUniform("shadow_mv0", "mat4");
    geometryShader.addUniform("shadow_mv1", "mat4");
    geometryShader.addUniform("shadow_mv2", "mat4");
    geometryShader.addUniform("shadow_mv3", "mat4");
    geometryShader.addUniform("shadow_mv4", "mat4");
    geometryShader.addUniform("shadow_mv5", "mat4");
    geometryShader.addUniform("projection", "mat4");

    geometryShader.addUniform("model_matrix", "mat4");
    geometryShader.addOutgoing("world_pos", "vec4");

    geometryShader.append("void main() {\n"
                          "   mat4 layerMVP[6];\n"
                          "   layerMVP[0] = projection * shadow_mv0;\n"
                          "   layerMVP[1] = projection * shadow_mv1;\n"
                          "   layerMVP[2] = projection * shadow_mv2;\n"
                          "   layerMVP[3] = projection * shadow_mv3;\n"
                          "   layerMVP[4] = projection * shadow_mv4;\n"
                          "   layerMVP[5] = projection * shadow_mv5;\n"
                          "   for (int i = 0; i < 6; ++i)\n"
                          "   {\n"
                          "      gl_Layer = i;\n"
                          "      for(int j = 0; j < 3; ++j)\n"
                          "      {\n"
                          "         world_pos = model_matrix * raw_pos[j];\n"
                          "         world_pos /= world_pos.w;\n"
                          "         gl_Position = layerMVP[j] * raw_pos[j];\n"
                          "         world_pos.w = gl_Position.w;\n"
                          "         EmitVertex();\n"
                          "      }\n"
                          "      EndPrimitive();\n"
                          "   }\n"
                          "}");
}

void QDemonShaderProgramGeneratorInterface::outputCubeFaceDepthFragment(QDemonShaderStageGeneratorInterface &fragmentShader)
{
    fragmentShader.addUniform("camera_position", "vec3");
    fragmentShader.addUniform("camera_properties", "vec2");

    fragmentShader.append("void main() {\n"
                          "    vec3 camPos = vec3( camera_position.x, camera_position.y, -camera_position.z );\n"
                          "    float dist = length( world_pos.xyz - camPos );\n"
                          "    dist = (dist - camera_properties.x) / (camera_properties.y - camera_properties.x);\n"
                          // "    gl_FragDepth = dist;\n"
                          "    fragOutput = vec4(dist, dist, dist, 1.0);\n"
                          "}");
}

QDemonShaderStageGeneratorInterface::~QDemonShaderStageGeneratorInterface() = default;

QT_END_NAMESPACE
