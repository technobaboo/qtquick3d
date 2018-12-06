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
#include <QtDemonRuntimeRender/qdemonrendershadercodegeneratorv2.h>
#include <Qt3DSFoundation.h>
#include <Qt3DSBroadcastingAllocator.h>
#include <StringTable.h>
#include <Qt3DSIntrinsics.h>
#include <Qt3DSAtomic.h>
#include <Utils.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRuntimeRender/qdemonrenderdynamicobjectsystem.h>

#include <QtGui/qopengl.h>

QT_BEGIN_NAMESPACE

namespace {
struct SStageGeneratorBase : public IShaderStageGenerator
{
    NVFoundationBase &m_Foundation;
    IStringTable &m_StringTable;
    TStrTableStrMap m_Incoming;
    TStrTableStrMap *m_Outgoing;
    nvhash_set<CRegisteredString> m_Includes;
    TStrTableStrMap m_Uniforms;
    TStrTableStrMap m_ConstantBuffers;
    TConstantBufferParamArray m_ConstantBufferParams;
    CRenderString m_CodeBuilder;
    CRenderString m_FinalBuilder;
    ShaderGeneratorStages::Enum m_Stage;
    TShaderGeneratorStageFlags m_EnabledStages;
    QStringList m_addedFunctions;

    SStageGeneratorBase(NVFoundationBase &inFnd, IStringTable &strTable,
                        ShaderGeneratorStages::Enum inStage)

        : m_Foundation(inFnd)
        , m_StringTable(strTable)
        , m_Incoming(inFnd.getAllocator(), "m_Incoming")
        , m_Outgoing(nullptr)
        , m_Includes(inFnd.getAllocator(), "m_Includes")
        , m_Uniforms(inFnd.getAllocator(), "m_Uniforms")
        , m_ConstantBuffers(inFnd.getAllocator(), "m_ConstantBuffers")
        , m_ConstantBufferParams(inFnd.getAllocator(), "m_ConstantBufferParams")
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

    CRegisteredString Str(const char8_t *var) { return m_StringTable.RegisterStr(var); }

    void AddIncoming(const char8_t *name, const char8_t *type) override
    {
        m_Incoming.insert(eastl::make_pair(Str(name), Str(type)));
    }
    virtual const char8_t *GetIncomingVariableName()
    {
        return "in";
    }

    void AddIncoming(const TStrType &name, const char8_t *type) override
    {
        AddIncoming(name.c_str(), type);
    }
    void AddOutgoing(const char8_t *name, const char8_t *type) override
    {
        if (m_Outgoing == nullptr) {
            Q_ASSERT(false);
            return;
        }
        m_Outgoing->insert(eastl::make_pair(Str(name), Str(type)));
    }
    void AddOutgoing(const TStrType &name, const char8_t *type) override
    {
        AddOutgoing(name.c_str(), type);
    }

    void AddUniform(const char8_t *name, const char8_t *type) override
    {
        m_Uniforms.insert(eastl::make_pair(Str(name), Str(type)));
    }
    void AddUniform(const TStrType &name, const char8_t *type) override
    {
        AddUniform(name.c_str(), type);
    }

    void AddConstantBuffer(const char *name, const char *layout) override
    {
        m_ConstantBuffers.insert(eastl::make_pair(Str(name), Str(layout)));
    }
    void AddConstantBufferParam(const char *cbName, const char *paramName, const char *type) override
    {
        TParamPair theParamPair(m_StringTable.RegisterStr(paramName),
                                m_StringTable.RegisterStr(type));
        TConstantBufferParamPair theBufferParamPair(m_StringTable.RegisterStr(cbName),
                                                    theParamPair);
        m_ConstantBufferParams.push_back(theBufferParamPair);
    }

    IShaderStageGenerator &operator<<(const char *data) override
    {
        m_CodeBuilder.append(nonNull(data));
        return *this;
    }
    IShaderStageGenerator &operator<<(const TStrType &data) override
    {
        m_CodeBuilder.append(data);
        return *this;
    }
    IShaderStageGenerator &operator<<(const SEndlType & /*data*/) override
    {
        m_CodeBuilder.append("\n");
        return *this;
    }
    void Append(const char *data) override
    {
        m_CodeBuilder.append(nonNull(data));
        m_CodeBuilder.append("\n");
    }
    void AppendPartial(const char *data) override { m_CodeBuilder.append(nonNull(data)); }
    ShaderGeneratorStages::Enum Stage() const override { return m_Stage; }

    virtual void AddShaderItemMap(const char *itemType, const TStrTableStrMap &itemMap,
                                  const char8_t *inItemSuffix = "")
    {
        m_FinalBuilder.append("\n");

        for (TStrTableStrMap::const_iterator iter = itemMap.begin(), end = itemMap.end();
             iter != end; ++iter) {
            m_FinalBuilder.append(itemType);
            m_FinalBuilder.append(" ");
            m_FinalBuilder.append(iter->second);
            m_FinalBuilder.append(" ");
            m_FinalBuilder.append(iter->first);
            m_FinalBuilder.append(inItemSuffix);
            m_FinalBuilder.append(";\n");
        }
    }

    virtual void AddShaderIncomingMap() { AddShaderItemMap(GetIncomingVariableName(), m_Incoming); }

    virtual void AddShaderUniformMap() { AddShaderItemMap("uniform", m_Uniforms); }

    virtual void AddShaderOutgoingMap()
    {
        if (m_Outgoing)
            AddShaderItemMap("varying", *m_Outgoing);
    }

    virtual void AddShaderConstantBufferItemMap(const char *itemType, const TStrTableStrMap &cbMap,
                                                TConstantBufferParamArray cbParamsArray)
    {
        m_FinalBuilder.append("\n");

        // iterate over all constant buffers
        for (TStrTableStrMap::const_iterator iter = cbMap.begin(), end = cbMap.end(); iter != end;
             ++iter) {
            m_FinalBuilder.append(iter->second);
            m_FinalBuilder.append(" ");
            m_FinalBuilder.append(itemType);
            m_FinalBuilder.append(" ");
            m_FinalBuilder.append(iter->first);
            m_FinalBuilder.append(" {\n");
            // iterate over all param entries and add match
            for (TConstantBufferParamArray::const_iterator iter1 = cbParamsArray.begin(),
                 end = cbParamsArray.end();
                 iter1 != end; ++iter1) {
                if (iter1->first == iter->first) {
                    m_FinalBuilder.append(iter1->second.second);
                    m_FinalBuilder.append(" ");
                    m_FinalBuilder.append(iter1->second.first);
                    m_FinalBuilder.append(";\n");
                }
            }

            m_FinalBuilder.append("};\n");
        }
    }

    virtual void AppendShaderCode() { m_FinalBuilder.append(m_CodeBuilder); }

    virtual void UpdateShaderCacheFlags(SShaderCacheProgramFlags &) {}

    void AddInclude(const char8_t *name) override { m_Includes.insert(Str(name)); }

    void AddInclude(const TStrType &name) override { AddInclude(name.c_str()); }

    void AddInclude(const QString &name) override
    {
        QByteArray arr = name.toLatin1();
        AddInclude(arr.data());
    }

    virtual const char8_t *BuildShaderSource()
    {
        for (nvhash_set<CRegisteredString>::const_iterator iter = m_Includes.begin(),
             end = m_Includes.end();
             iter != end; ++iter) {
            m_FinalBuilder.append("#include \"");
            m_FinalBuilder.append(iter->c_str());
            m_FinalBuilder.append("\"\n");
        }
        AddShaderIncomingMap();
        AddShaderUniformMap();
        AddShaderConstantBufferItemMap("uniform", m_ConstantBuffers, m_ConstantBufferParams);
        AddShaderOutgoingMap();
        m_FinalBuilder.append("\n");
        AppendShaderCode();
        return m_FinalBuilder.c_str();
    }

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
};

struct SVertexShaderGenerator : public SStageGeneratorBase
{
    SVertexShaderGenerator(NVFoundationBase &inFnd, IStringTable &strTable)
        : SStageGeneratorBase(inFnd, strTable, ShaderGeneratorStages::Vertex)
    {
    }

    const char8_t *GetIncomingVariableName() override { return "attribute"; }
    virtual void AddIncomingInterpolatedMap() {}

    virtual const char8_t *GetInterpolatedIncomingSuffix() const { return "_attr"; }
    virtual const char8_t *GetInterpolatedOutgoingSuffix() const { return ""; }
};

struct STessControlShaderGenerator : public SStageGeneratorBase
{
    STessControlShaderGenerator(NVFoundationBase &inFnd, IStringTable &strTable)
        : SStageGeneratorBase(inFnd, strTable, ShaderGeneratorStages::TessControl)
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
    STessEvalShaderGenerator(NVFoundationBase &inFnd, IStringTable &strTable)
        : SStageGeneratorBase(inFnd, strTable, ShaderGeneratorStages::TessEval)
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
    SGeometryShaderGenerator(NVFoundationBase &inFnd, IStringTable &strTable)
        : SStageGeneratorBase(inFnd, strTable, ShaderGeneratorStages::Geometry)
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
    SFragmentShaderGenerator(NVFoundationBase &inFnd, IStringTable &strTable)
        : SStageGeneratorBase(inFnd, strTable, ShaderGeneratorStages::Fragment)
    {
    }
    void AddShaderIncomingMap() override { AddShaderItemMap("varying", m_Incoming); }
    void AddShaderOutgoingMap() override {}
};

struct SShaderGeneratedProgramOutput
{
    // never null; so safe to call strlen on.
    const char8_t *m_VertexShader;
    const char8_t *m_TessControlShader;
    const char8_t *m_TessEvalShader;
    const char8_t *m_GeometryShader;
    const char8_t *m_FragmentShader;

    SShaderGeneratedProgramOutput()
        : m_VertexShader("")
        , m_TessControlShader("")
        , m_TessEvalShader("")
        , m_GeometryShader("")
        , m_FragmentShader("")
    {
    }

    SShaderGeneratedProgramOutput(const char8_t *vs, const char8_t *tc, const char8_t *te,
                                  const char8_t *gs, const char8_t *fs)
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
    IQt3DSRenderContext &m_Context;
    SVertexShaderGenerator m_VS;
    STessControlShaderGenerator m_TC;
    STessEvalShaderGenerator m_TE;
    SGeometryShaderGenerator m_GS;
    SFragmentShaderGenerator m_FS;

    TShaderGeneratorStageFlags m_EnabledStages;

    qint32 m_RefCount;

    SProgramGenerator(IQt3DSRenderContext &inContext)
        : m_Context(inContext)
        , m_VS(inContext.GetFoundation(), inContext.GetStringTable())
        , m_TC(inContext.GetFoundation(), inContext.GetStringTable())
        , m_TE(inContext.GetFoundation(), inContext.GetStringTable())
        , m_GS(inContext.GetFoundation(), inContext.GetStringTable())
        , m_FS(inContext.GetFoundation(), inContext.GetStringTable())
        , m_RefCount(0)
    {
    }

    void addRef() override { atomicIncrement(&m_RefCount); }
    void release() override
    {
        atomicDecrement(&m_RefCount);
        if (m_RefCount <= 0) {
            NVFoundationBase &theFoundation(m_Context.GetFoundation());
            NVDelete(theFoundation.getAllocator(), this);
        }
    }

    void LinkStages()
    {
        // Link stages incoming to outgoing variables.
        SStageGeneratorBase *previous = nullptr;
        quint32 theStageId = 1;
        for (quint32 idx = 0, end = (quint32)ShaderGeneratorStages::StageCount; idx < end;
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

    QDemonRenderShaderProgram *
    CompileGeneratedShader(const char *inShaderName, const SShaderCacheProgramFlags &inFlags,
                           TShaderFeatureSet inFeatureSet, bool separableProgram) override
    {
        // No stages enabled
        if (((quint32)m_EnabledStages) == 0) {
            Q_ASSERT(false);
            return nullptr;
        }

        IDynamicObjectSystem &theDynamicSystem(m_Context.GetDynamicObjectSystem());
        SShaderCacheProgramFlags theCacheFlags(inFlags);
        for (quint32 stageIdx = 0, stageEnd = ShaderGeneratorStages::StageCount; stageIdx < stageEnd;
             ++stageIdx) {
            ShaderGeneratorStages::Enum stageName =
                    static_cast<ShaderGeneratorStages::Enum>(1 << stageIdx);
            if (m_EnabledStages & stageName) {
                SStageGeneratorBase &theStage(InternalGetStage(stageName));
                theStage.BuildShaderSource();
                theStage.UpdateShaderCacheFlags(theCacheFlags);
                theDynamicSystem.InsertShaderHeaderInformation(theStage.m_FinalBuilder,
                                                               inShaderName);
            }
        }

        const char *vertexShaderSource = m_VS.m_FinalBuilder.c_str();
        const char *tcShaderSource = m_TC.m_FinalBuilder.c_str();
        const char *teShaderSource = m_TE.m_FinalBuilder.c_str();
        const char *geShaderSource = m_GS.m_FinalBuilder.c_str();
        const char *fragmentShaderSource = m_FS.m_FinalBuilder.c_str();

        IShaderCache &theCache = m_Context.GetShaderCache();
        CRegisteredString theCacheKey = m_Context.GetStringTable().RegisterStr(inShaderName);
        return theCache.CompileProgram(theCacheKey, vertexShaderSource, fragmentShaderSource,
                                       tcShaderSource, teShaderSource, geShaderSource,
                                       theCacheFlags, inFeatureSet, separableProgram);
    }
};
};

IShaderProgramGenerator &
IShaderProgramGenerator::CreateProgramGenerator(IQt3DSRenderContext &inContext)
{
    return *QDEMON_NEW(inContext.GetAllocator(), SProgramGenerator)(inContext);
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
    //	vertexShader.Append("   gl_Position = vec4( attr_pos, 1.0 );");
    vertexShader.Append("}");
}

void IShaderProgramGenerator::OutputCubeFaceDepthGeometry(IShaderStageGenerator &geometryShader)
{
    geometryShader.Append("layout(triangles) in;");
    geometryShader.Append("layout(triangle_strip, max_vertices = 18) out;");
    // geometryShader.AddUniform("shadow_mvp[6]", "mat4");

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
