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
struct SStageGeneratorBase : public IShaderStageGenerator
{
    TStrTableStrMap m_Incoming;
    TStrTableStrMap *m_Outgoing;
    QSet<QString> m_Includes;
    TStrTableStrMap m_Uniforms;
    TStrTableStrMap m_ConstantBuffers;
    TConstantBufferParamArray m_ConstantBufferParams;
    QString m_CodeBuilder;
    QString m_FinalBuilder;
    ShaderGeneratorStages::Enum m_Stage;
    TShaderGeneratorStageFlags m_EnabledStages;
    QStringList m_addedFunctions;

    SStageGeneratorBase(ShaderGeneratorStages::Enum inStage)

        : m_Outgoing(nullptr)
        , m_Stage(inStage)
    {
    }

    virtual void Begin(TShaderGeneratorStageFlags inEnabledStages)
    {
        m_Incoming.clear();
        m_Outgoing = nullptr;
        m_Includes.clear();
        m_Uniforms.clear();
        m_ConstantBuffers.clear();
        m_ConstantBufferParams.clear();
        m_CodeBuilder.clear();
        m_FinalBuilder.clear();
        m_EnabledStages = inEnabledStages;
        m_addedFunctions.clear();
        // the shared buffers will be cleared elsewhere.
    }

    void AddIncoming(const QString &name, const QString &type) override
    {
        m_Incoming.insert(name, type);
    }
    virtual const QString GetIncomingVariableName()
    {
        return QStringLiteral("in");
    }

    void AddOutgoing(const QString &name, const QString &type) override
    {
        if (m_Outgoing == nullptr) {
            Q_ASSERT(false);
            return;
        }
        m_Outgoing->insert(name, type);
    }

    void AddUniform(const QString &name, const QString &type) override
    {
        m_Uniforms.insert(name, type);
    }

    void AddConstantBuffer(const QString &name, const QString &layout) override
    {
        m_ConstantBuffers.insert(name, layout);
    }
    void AddConstantBufferParam(const QString &cbName, const QString &paramName, const QString &type) override
    {
        TParamPair theParamPair(paramName, type);
        TConstantBufferParamPair theBufferParamPair(cbName, theParamPair);
        m_ConstantBufferParams.push_back(theBufferParamPair);
    }

    IShaderStageGenerator &operator<<(const QString &data) override
    {
        m_CodeBuilder.append(data);
        return *this;
    }
    void Append(const QString &data) override
    {
        m_CodeBuilder.append(data);
        m_CodeBuilder.append(QStringLiteral("\n"));
    }
    void AppendPartial(const QString &data) override { m_CodeBuilder.append(data); }
    ShaderGeneratorStages::Enum Stage() const override { return m_Stage; }

    virtual void AddShaderItemMap(const QString &itemType, const TStrTableStrMap &itemMap,
                                  const QString &inItemSuffix = QString())
    {
        m_FinalBuilder.append(QStringLiteral("\n"));

        for (TStrTableStrMap::const_iterator iter = itemMap.begin(), end = itemMap.end();
             iter != end; ++iter) {
            m_FinalBuilder.append(itemType);
            m_FinalBuilder.append(QStringLiteral(" "));
            m_FinalBuilder.append(iter.value());
            m_FinalBuilder.append(QStringLiteral(" "));
            m_FinalBuilder.append(iter.key());
            m_FinalBuilder.append(inItemSuffix);
            m_FinalBuilder.append(QStringLiteral(";\n"));
        }
    }

    virtual void AddShaderIncomingMap() { AddShaderItemMap(GetIncomingVariableName(), m_Incoming); }

    virtual void AddShaderUniformMap() { AddShaderItemMap("uniform", m_Uniforms); }

    virtual void AddShaderOutgoingMap()
    {
        if (m_Outgoing)
            AddShaderItemMap("varying", *m_Outgoing);
    }

    virtual void AddShaderConstantBufferItemMap(const QString &itemType, const TStrTableStrMap &cbMap,
                                                TConstantBufferParamArray cbParamsArray)
    {
        m_FinalBuilder.append(QStringLiteral("\n"));

        // iterate over all constant buffers
        for (TStrTableStrMap::const_iterator iter = cbMap.begin(), end = cbMap.end(); iter != end;
             ++iter) {
            m_FinalBuilder.append(iter.value());
            m_FinalBuilder.append(QStringLiteral(" "));
            m_FinalBuilder.append(itemType);
            m_FinalBuilder.append(QStringLiteral(" "));
            m_FinalBuilder.append(iter.key());
            m_FinalBuilder.append(QStringLiteral(" {\n"));
            // iterate over all param entries and add match
            for (TConstantBufferParamArray::const_iterator iter1 = cbParamsArray.begin(),
                 end = cbParamsArray.end();
                 iter1 != end; ++iter1) {
                if (iter1->first == iter.key()) {
                    m_FinalBuilder.append(iter1->second.second);
                    m_FinalBuilder.append(QStringLiteral(" "));
                    m_FinalBuilder.append(iter1->second.first);
                    m_FinalBuilder.append(QStringLiteral(";\n"));
                }
            }

            m_FinalBuilder.append(QStringLiteral("};\n"));
        }
    }

    virtual void AppendShaderCode() { m_FinalBuilder.append(m_CodeBuilder); }

    virtual void UpdateShaderCacheFlags(SShaderCacheProgramFlags &) {}

    void AddInclude(const QString &name) override { m_Includes.insert(name); }

    virtual const QString BuildShaderSource()
    {
        for (QSet<QString>::const_iterator iter = m_Includes.begin(),
             end = m_Includes.end();
             iter != end; ++iter) {
            m_FinalBuilder.append(QStringLiteral("#include \""));
            m_FinalBuilder.append(*iter);
            m_FinalBuilder.append(QStringLiteral("\"\n"));
        }
        AddShaderIncomingMap();
        AddShaderUniformMap();
        AddShaderConstantBufferItemMap("uniform", m_ConstantBuffers, m_ConstantBufferParams);
        AddShaderOutgoingMap();
        m_FinalBuilder.append(QStringLiteral("\n"));
        AppendShaderCode();

        qDebug() << "####: " << m_FinalBuilder;
        return m_FinalBuilder;
    }

    void AddFunction(const QString &functionName) override
    {
        if (!m_addedFunctions.contains(functionName)) {
            m_addedFunctions.push_back(functionName);
            QString includeName;
            includeName = QStringLiteral("func") + functionName + QStringLiteral(".glsllib");
            AddInclude(includeName);
        }
    }
};

struct SVertexShaderGenerator : public SStageGeneratorBase
{
    SVertexShaderGenerator()
        : SStageGeneratorBase(ShaderGeneratorStages::Vertex)
    {
    }

    const QString GetIncomingVariableName() override { return QStringLiteral("attribute"); }
    virtual void AddIncomingInterpolatedMap() {}

    virtual const QString GetInterpolatedIncomingSuffix() const { return QStringLiteral("_attr"); }
    virtual const QString GetInterpolatedOutgoingSuffix() const { return QString(); }
};

struct STessControlShaderGenerator : public SStageGeneratorBase
{
    STessControlShaderGenerator()
        : SStageGeneratorBase(ShaderGeneratorStages::TessControl)
    {
    }

    void AddShaderIncomingMap() override { AddShaderItemMap("attribute", m_Incoming, "[]"); }

    void AddShaderOutgoingMap() override
    {
        if (m_Outgoing)
            AddShaderItemMap("varying", *m_Outgoing, "[]");
    }

    void UpdateShaderCacheFlags(SShaderCacheProgramFlags &inFlags) override
    {
        inFlags.SetTessellationEnabled(true);
    }
};

struct STessEvalShaderGenerator : public SStageGeneratorBase
{
    STessEvalShaderGenerator()
        : SStageGeneratorBase(ShaderGeneratorStages::TessEval)
    {
    }

    void AddShaderIncomingMap() override { AddShaderItemMap("attribute", m_Incoming, "[]"); }

    void UpdateShaderCacheFlags(SShaderCacheProgramFlags &inFlags) override
    {
        inFlags.SetTessellationEnabled(true);
    }
};

struct SGeometryShaderGenerator : public SStageGeneratorBase
{
    SGeometryShaderGenerator()
        : SStageGeneratorBase(ShaderGeneratorStages::Geometry)
    {
    }

    void AddShaderIncomingMap() override { AddShaderItemMap("attribute", m_Incoming, "[]"); }

    void AddShaderOutgoingMap() override
    {
        if (m_Outgoing)
            AddShaderItemMap("varying", *m_Outgoing);
    }
    void UpdateShaderCacheFlags(SShaderCacheProgramFlags &inFlags) override
    {
        inFlags.SetGeometryShaderEnabled(true);
    }
};

struct SFragmentShaderGenerator : public SStageGeneratorBase
{
    SFragmentShaderGenerator()
        : SStageGeneratorBase(ShaderGeneratorStages::Fragment)
    {
    }
    void AddShaderIncomingMap() override { AddShaderItemMap("varying", m_Incoming); }
    void AddShaderOutgoingMap() override {}
};

struct SShaderGeneratedProgramOutput
{
    // never null; so safe to call strlen on.
    const char *m_VertexShader;
    const char *m_TessControlShader;
    const char *m_TessEvalShader;
    const char *m_GeometryShader;
    const char *m_FragmentShader;

    SShaderGeneratedProgramOutput()
        : m_VertexShader("")
        , m_TessControlShader("")
        , m_TessEvalShader("")
        , m_GeometryShader("")
        , m_FragmentShader("")
    {
    }

    SShaderGeneratedProgramOutput(const char *vs, const char *tc, const char *te,
                                  const char *gs, const char *fs)
        : m_VertexShader(vs)
        , m_TessControlShader(tc)
        , m_TessEvalShader(te)
        , m_GeometryShader(gs)
        , m_FragmentShader(fs)
    {
    }
};

struct SProgramGenerator : public IShaderProgramGenerator
{
    IQDemonRenderContext *m_Context;
    SVertexShaderGenerator m_VS;
    STessControlShaderGenerator m_TC;
    STessEvalShaderGenerator m_TE;
    SGeometryShaderGenerator m_GS;
    SFragmentShaderGenerator m_FS;

    TShaderGeneratorStageFlags m_EnabledStages;

    SProgramGenerator(IQDemonRenderContext *inContext)
        : m_Context(inContext)
    {
    }

    void LinkStages()
    {
        // Link stages incoming to outgoing variables.
        SStageGeneratorBase *previous = nullptr;
        quint32 theStageId = 1;
        for (quint32 idx = 0, end = quint32(ShaderGeneratorStages::StageCount); idx < end;
             ++idx, theStageId = theStageId << 1) {
            SStageGeneratorBase *thisStage = nullptr;
            ShaderGeneratorStages::Enum theStageEnum =
                    static_cast<ShaderGeneratorStages::Enum>(theStageId);
            if ((m_EnabledStages & theStageEnum)) {
                thisStage = &InternalGetStage(theStageEnum);
                if (previous)
                    previous->m_Outgoing = &thisStage->m_Incoming;
                previous = thisStage;
            }
        }
    }

    void BeginProgram(TShaderGeneratorStageFlags inEnabledStages) override
    {
        m_VS.Begin(inEnabledStages);
        m_TC.Begin(inEnabledStages);
        m_TE.Begin(inEnabledStages);
        m_GS.Begin(inEnabledStages);
        m_FS.Begin(inEnabledStages);
        m_EnabledStages = inEnabledStages;
        LinkStages();
    }

    TShaderGeneratorStageFlags GetEnabledStages() const override { return m_EnabledStages; }

    SStageGeneratorBase &InternalGetStage(ShaderGeneratorStages::Enum inStage)
    {
        switch (inStage) {
        case ShaderGeneratorStages::Vertex:
            return m_VS;
        case ShaderGeneratorStages::TessControl:
            return m_TC;
        case ShaderGeneratorStages::TessEval:
            return m_TE;
        case ShaderGeneratorStages::Geometry:
            return m_GS;
        case ShaderGeneratorStages::Fragment:
            return m_FS;
        default:
            Q_ASSERT(false);
            break;
        }
        return m_VS;
    }
    // get the stage or nullptr if it has not been created.
    IShaderStageGenerator *GetStage(ShaderGeneratorStages::Enum inStage) override
    {
        if (inStage > 0 || inStage < ShaderGeneratorStages::StageCount) {
            if ((m_EnabledStages & inStage))
                return &InternalGetStage(inStage);
        } else {
            Q_ASSERT(false);
        }
        return nullptr;
    }

    QSharedPointer<QDemonRenderShaderProgram>
    CompileGeneratedShader(const QString &inShaderName, const SShaderCacheProgramFlags &inFlags,
                           TShaderFeatureSet inFeatureSet, bool separableProgram) override
    {
        // No stages enabled
        if (((quint32)m_EnabledStages) == 0) {
            Q_ASSERT(false);
            return nullptr;
        }

        QSharedPointer<IDynamicObjectSystem> theDynamicSystem(m_Context->GetDynamicObjectSystem());
        SShaderCacheProgramFlags theCacheFlags(inFlags);
        for (quint32 stageIdx = 0, stageEnd = ShaderGeneratorStages::StageCount; stageIdx < stageEnd;
             ++stageIdx) {
            ShaderGeneratorStages::Enum stageName =
                    static_cast<ShaderGeneratorStages::Enum>(1 << stageIdx);
            if (m_EnabledStages & stageName) {
                SStageGeneratorBase &theStage(InternalGetStage(stageName));
                theStage.BuildShaderSource();
                theStage.UpdateShaderCacheFlags(theCacheFlags);
                theDynamicSystem->InsertShaderHeaderInformation(theStage.m_FinalBuilder, inShaderName.toLocal8Bit().constData());
            }
        }

        qDebug() << "VS: " << m_VS.m_FinalBuilder;
        qDebug() << "FS: " << m_FS.m_FinalBuilder;

        QSharedPointer<IShaderCache> theCache = m_Context->GetShaderCache();
        QString theCacheKey = inShaderName;
        return theCache->CompileProgram(theCacheKey, m_VS.m_FinalBuilder, m_FS.m_FinalBuilder,
                                        m_TC.m_FinalBuilder, m_TE.m_FinalBuilder, m_GS.m_FinalBuilder,
                                        theCacheFlags, inFeatureSet, separableProgram);
    }
};
};

QSharedPointer<IShaderProgramGenerator>
IShaderProgramGenerator::CreateProgramGenerator(IQDemonRenderContext *inContext)
{
    return QSharedPointer<IShaderProgramGenerator>(new SProgramGenerator(inContext));
}

void IShaderProgramGenerator::OutputParaboloidDepthVertex(IShaderStageGenerator &vertexShader)
{
    vertexShader.AddIncoming("attr_pos", "vec3");
    vertexShader.AddInclude("shadowMapping.glsllib");
    vertexShader.AddUniform("model_view_projection", "mat4");
    // vertexShader.AddUniform("model_view", "mat4");
    vertexShader.AddUniform("camera_properties", "vec2");
    // vertexShader.AddOutgoing("view_pos", "vec4");
    vertexShader.AddOutgoing("world_pos", "vec4");

    // Project the location onto screen space.
    // This will be horrible if you have a single large polygon.  Tessellation is your friend here!
    vertexShader.Append("void main() {");
    vertexShader.Append(
                "   ParaboloidMapResult data = VertexParaboloidDepth( attr_pos, model_view_projection );");
    vertexShader.Append("   gl_Position = data.m_Position;");
    vertexShader.Append("   world_pos = data.m_WorldPos;");
    vertexShader.Append("}");
}

void IShaderProgramGenerator::OutputParaboloidDepthTessEval(IShaderStageGenerator &tessEvalShader)
{
    tessEvalShader.AddInclude("shadowMapping.glsllib");
    tessEvalShader.AddUniform("model_view_projection", "mat4");
    tessEvalShader.AddOutgoing("world_pos", "vec4");
    tessEvalShader.Append("   ParaboloidMapResult data = VertexParaboloidDepth( vec3(pos.xyz), "
                           "model_view_projection );");
    tessEvalShader.Append("   gl_Position = data.m_Position;");
    tessEvalShader.Append("   world_pos = data.m_WorldPos;");
}

void IShaderProgramGenerator::OutputParaboloidDepthFragment(IShaderStageGenerator &fragmentShader)
{
    fragmentShader.AddInclude("shadowMappingFragment.glsllib");
    fragmentShader.AddUniform("model_view_projection", "mat4");
    fragmentShader.AddUniform("camera_properties", "vec2");
    fragmentShader.Append("void main() {");
    fragmentShader.Append("   gl_FragDepth = FragmentParaboloidDepth( world_pos, "
                           "model_view_projection, camera_properties );");
    fragmentShader.Append("}");
}

void IShaderProgramGenerator::OutputCubeFaceDepthVertex(IShaderStageGenerator &vertexShader)
{
    vertexShader.AddIncoming("attr_pos", "vec3");
    vertexShader.AddUniform("model_matrix", "mat4");
    vertexShader.AddUniform("model_view_projection", "mat4");

    vertexShader.AddOutgoing("raw_pos", "vec4");
    vertexShader.AddOutgoing("world_pos", "vec4");

    vertexShader.Append("void main() {");
    vertexShader.Append("   world_pos = model_matrix * vec4( attr_pos, 1.0 );");
    vertexShader.Append("   world_pos /= world_pos.w;");
    vertexShader.Append("	gl_Position = model_view_projection * vec4( attr_pos, 1.0 );");
    vertexShader.Append("   raw_pos = vec4( attr_pos, 1.0 );");
    //	vertexShader->Append("   gl_Position = vec4( attr_pos, 1.0 );");
    vertexShader.Append("}");
}

void IShaderProgramGenerator::OutputCubeFaceDepthGeometry(IShaderStageGenerator &geometryShader)
{
    geometryShader.Append("layout(triangles) in;");
    geometryShader.Append("layout(triangle_strip, max_vertices = 18) out;");
    // geometryShader->AddUniform("shadow_mvp[6]", "mat4");

    geometryShader.AddUniform("shadow_mv0", "mat4");
    geometryShader.AddUniform("shadow_mv1", "mat4");
    geometryShader.AddUniform("shadow_mv2", "mat4");
    geometryShader.AddUniform("shadow_mv3", "mat4");
    geometryShader.AddUniform("shadow_mv4", "mat4");
    geometryShader.AddUniform("shadow_mv5", "mat4");
    geometryShader.AddUniform("projection", "mat4");

    geometryShader.AddUniform("model_matrix", "mat4");
    geometryShader.AddOutgoing("world_pos", "vec4");

    geometryShader.Append("void main() {");
    geometryShader.Append("   mat4 layerMVP[6];");
    geometryShader.Append("   layerMVP[0] = projection * shadow_mv0;");
    geometryShader.Append("   layerMVP[1] = projection * shadow_mv1;");
    geometryShader.Append("   layerMVP[2] = projection * shadow_mv2;");
    geometryShader.Append("   layerMVP[3] = projection * shadow_mv3;");
    geometryShader.Append("   layerMVP[4] = projection * shadow_mv4;");
    geometryShader.Append("   layerMVP[5] = projection * shadow_mv5;");
    geometryShader.Append("   for (int i = 0; i < 6; ++i)");
    geometryShader.Append("   {");
    geometryShader.Append("      gl_Layer = i;");
    geometryShader.Append("      for(int j = 0; j < 3; ++j)");
    geometryShader.Append("      {");
    geometryShader.Append("         world_pos = model_matrix * raw_pos[j];");
    geometryShader.Append("         world_pos /= world_pos.w;");
    geometryShader.Append("         gl_Position = layerMVP[j] * raw_pos[j];");
    geometryShader.Append("         world_pos.w = gl_Position.w;");
    geometryShader.Append("         EmitVertex();");
    geometryShader.Append("      }");
    geometryShader.Append("      EndPrimitive();");
    geometryShader.Append("   }");
    geometryShader.Append("}");
}

void IShaderProgramGenerator::OutputCubeFaceDepthFragment(IShaderStageGenerator &fragmentShader)
{
    fragmentShader.AddUniform("camera_position", "vec3");
    fragmentShader.AddUniform("camera_properties", "vec2");

    fragmentShader.Append("void main() {");
    fragmentShader.Append(
                "\tvec3 camPos = vec3( camera_position.x, camera_position.y, -camera_position.z );");
    fragmentShader.Append("\tfloat dist = length( world_pos.xyz - camPos );");
    fragmentShader.Append(
                "\tdist = (dist - camera_properties.x) / (camera_properties.y - camera_properties.x);");
    // fragmentShader.Append("\tgl_FragDepth = dist;");
    fragmentShader.Append("\tfragOutput = vec4(dist, dist, dist, 1.0);");
    fragmentShader.Append("}");
}

QT_END_NAMESPACE
