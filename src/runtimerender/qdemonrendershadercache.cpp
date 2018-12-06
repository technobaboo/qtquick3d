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
#include <QtDemonRuntimeRender/qdemonrendershadercache.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRuntimeRender/qdemonrenderstring.h>
#include <qdemonrenderinputstreamfactory.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>
#include <QtDemonRuntimeRender/qdemonrenderer.h>
#include <memory>

#include <QRegularExpression>
#include <QString>

QT_BEGIN_NAMESPACE

namespace {
using QDemonRenderContextScopedProperty;
const char *TessellationEnabledStr = "TessellationStageEnabled";
const char *GeometryEnabledStr = "GeometryStageEnabled";
inline void AppendFlagValue(CRenderString &inStr, const char *flag)
{
    if (inStr.length())
        inStr.append(1, ',');
    inStr.append(flag);
}
inline void CacheFlagsToStr(const SShaderCacheProgramFlags &inFlags, CRenderString &inString)
{
    inString.clear();
    if (inFlags.IsTessellationEnabled())
        AppendFlagValue(inString, TessellationEnabledStr);
    if (inFlags.IsGeometryShaderEnabled())
        AppendFlagValue(inString, GeometryEnabledStr);
}

struct ShaderType
{
    enum Enum { Vertex, TessControl, TessEval, Fragment, Geometry, Compute };
};

inline ShaderType::Enum StringToShaderType(CRenderString &inShaderType)
{
    ShaderType::Enum retval = ShaderType::Vertex;

    if (inShaderType.size() == 0)
        return retval;

    if (!inShaderType.compare("VertexCode"))
        retval = ShaderType::Vertex;
    else if (!inShaderType.compare("FragmentCode"))
        retval = ShaderType::Fragment;
    else if (!inShaderType.compare("TessControlCode"))
        retval = ShaderType::TessControl;
    else if (!inShaderType.compare("TessEvalCode"))
        retval = ShaderType::TessEval;
    else if (!inShaderType.compare("GeometryCode"))
        retval = ShaderType::Geometry;
    else
        Q_ASSERT(false);

    return retval;
}

inline SShaderCacheProgramFlags CacheFlagsToStr(const CRenderString &inString)
{
    SShaderCacheProgramFlags retval;
    if (inString.find(TessellationEnabledStr) != CRenderString::npos)
        retval.SetTessellationEnabled(true);
    if (inString.find(GeometryEnabledStr) != CRenderString::npos)
        retval.SetGeometryShaderEnabled(true);
    return retval;
}

typedef eastl::pair<const char *, QDemonRenderContextValues::Enum> TStringToContextValuePair;

/*GLES2	= 1 << 0,
GL2		= 1 << 1,
GLES3	= 1 << 2,
GL3		= 1 << 3,
GL4		= 1 << 4,
NullContext = 1 << 5,*/
TStringToContextValuePair g_StringToContextTypeValue[] = {
    TStringToContextValuePair("GLES2", QDemonRenderContextValues::GLES2),
    TStringToContextValuePair("GL2", QDemonRenderContextValues::GL2),
    TStringToContextValuePair("GLES3", QDemonRenderContextValues::GLES3),
    TStringToContextValuePair("GLES3PLUS", QDemonRenderContextValues::GLES3PLUS),
    TStringToContextValuePair("GL3", QDemonRenderContextValues::GL3),
    TStringToContextValuePair("GL4", QDemonRenderContextValues::GL4),
    TStringToContextValuePair("NullContext", QDemonRenderContextValues::NullContext),
};

size_t g_NumStringToContextValueEntries =
        sizeof(g_StringToContextTypeValue) / sizeof(*g_StringToContextTypeValue);

inline void ContextTypeToString(QDemonRenderContextType inType,
                                CRenderString &outContextType)
{
    outContextType.clear();
    for (size_t idx = 0, end = g_NumStringToContextValueEntries; idx < end; ++idx) {
        if (inType & g_StringToContextTypeValue[idx].second) {
            if (outContextType.size())
                outContextType.append(1, '|');
            outContextType.append(g_StringToContextTypeValue[idx].first);
        }
    }
}

inline QDemonRenderContextType StringToContextType(const CRenderString &inContextType)
{
    QDemonRenderContextType retval;
    char tempBuffer[128];
    memZero(tempBuffer, 128);
    const QString::size_type lastTempBufIdx = 127;
    QString::size_type pos = 0, lastpos = 0;
    if (inContextType.size() == 0)
        return retval;

    do {
        pos = int(inContextType.find('|', lastpos));
        if (pos == QString::npos)
            pos = int(inContextType.size());
        {

            QString::size_type sectionLen = NVMin(pos - lastpos, lastTempBufIdx);
            intrinsics::::memcpy(tempBuffer, inContextType.c_str() + lastpos, sectionLen);
            tempBuffer[lastTempBufIdx] = 0;
            for (size_t idx = 0, end = g_NumStringToContextValueEntries; idx < end; ++idx) {
                if (strcmp(g_StringToContextTypeValue[idx].first, tempBuffer) == 0)
                    retval = retval | g_StringToContextTypeValue[idx].second;
            }
        }
        // iterate past the bar
        ++pos;
        lastpos = pos;
    } while (pos < inContextType.size() && pos != QString::npos);

    return retval;
}

struct SShaderCacheKey
{
    CRegisteredString m_Key;
    eastl::vector<SShaderPreprocessorFeature> m_Features;
    size_t m_HashCode;

    SShaderCacheKey(CRegisteredString key = CRegisteredString())
        : m_Key(key)
        , m_HashCode(0)
    {
    }

    SShaderCacheKey(const SShaderCacheKey &other)
        : m_Key(other.m_Key)
        , m_Features(other.m_Features)
        , m_HashCode(other.m_HashCode)
    {
    }

    SShaderCacheKey &operator=(const SShaderCacheKey &other)
    {
        m_Key = other.m_Key;
        m_Features = other.m_Features;
        m_HashCode = other.m_HashCode;
        return *this;
    }

    void GenerateHashCode()
    {
        m_HashCode = m_Key.hash();
        m_HashCode = m_HashCode
                ^ HashShaderFeatureSet(toDataRef(m_Features.data(), (quint32)m_Features.size()));
    }
    bool operator==(const SShaderCacheKey &inOther) const
    {
        return m_Key == inOther.m_Key && m_Features == inOther.m_Features;
    }
};
}

namespace eastl {
template <>
struct hash<SShaderCacheKey>
{
    size_t operator()(const SShaderCacheKey &inKey) const { return inKey.m_HashCode; }
};
}

namespace {

struct ShaderCache : public IShaderCache
{
    typedef QHash<SShaderCacheKey, QDemonScopedRefCounted<QDemonRenderShaderProgram>> TShaderMap;
    QDemonRenderContext &m_RenderContext;
    IPerfTimer &m_PerfTimer;
    TShaderMap m_Shaders;
    CRenderString m_CacheFilePath;
    CRenderString m_VertexCode;
    CRenderString m_TessCtrlCode;
    CRenderString m_TessEvalCode;
    CRenderString m_GeometryCode;
    CRenderString m_FragmentCode;
    CRenderString m_InsertStr;
    CRenderString m_FlagString;
    CRenderString m_ContextTypeString;
    SShaderCacheKey m_TempKey;

    QDemonScopedRefCounted<IDOMWriter> m_ShaderCache;
    IInputStreamFactory &m_InputStreamFactory;
    bool m_ShaderCompilationEnabled;
    volatile qint32 mRefCount;

    ShaderCache(QDemonRenderContext &ctx, IInputStreamFactory &inInputStreamFactory,
                IPerfTimer &inPerfTimer)
        : m_RenderContext(ctx)
        , m_PerfTimer(inPerfTimer)
        , m_Shaders(ctx.GetAllocator(), "ShaderCache::m_Shaders")
        , m_InputStreamFactory(inInputStreamFactory)
        , m_ShaderCompilationEnabled(true)
        , mRefCount(0)
    {
    }
    QDEMON_IMPLEMENT_REF_COUNT_ADDREF_RELEASE_OVERRIDE(m_RenderContext.GetAllocator())

    QDemonRenderShaderProgram *GetProgram(CRegisteredString inKey,
                                          QDemonConstDataRef<SShaderPreprocessorFeature> inFeatures) override
    {
        m_TempKey.m_Key = inKey;
        m_TempKey.m_Features.assign(inFeatures.begin(), inFeatures.end());
        m_TempKey.GenerateHashCode();
        TShaderMap::iterator theIter = m_Shaders.find(m_TempKey);
        if (theIter != m_Shaders.end())
            return theIter->second;
        return nullptr;
    }

    void AddBackwardCompatibilityDefines(ShaderType::Enum shaderType)
    {
        if (shaderType == ShaderType::Vertex || shaderType == ShaderType::TessControl
                || shaderType == ShaderType::TessEval || shaderType == ShaderType::Geometry) {
            m_InsertStr += "#define attribute in\n";
            m_InsertStr += "#define varying out\n";
        } else if (shaderType == ShaderType::Fragment) {
            m_InsertStr += "#define varying in\n";
            m_InsertStr += "#define texture2D texture\n";
            m_InsertStr += "#define gl_FragColor fragOutput\n";

            if (m_RenderContext.IsAdvancedBlendHwSupportedKHR())
                m_InsertStr += "layout(blend_support_all_equations) out;\n ";
            m_InsertStr += "out vec4 fragOutput;\n";
        }
    }

    void AddShaderExtensionStrings(ShaderType::Enum shaderType, bool isGLES)
    {
        if (isGLES) {
            if (m_RenderContext.IsStandardDerivativesSupported())
                m_InsertStr += "#extension GL_OES_standard_derivatives : enable\n";
            else
                m_InsertStr += "#extension GL_OES_standard_derivatives : disable\n";
        }

        if (IQt3DSRenderer::IsGlEs3Context(m_RenderContext.GetRenderContextType())) {
            if (shaderType == ShaderType::TessControl || shaderType == ShaderType::TessEval) {
                m_InsertStr += "#extension GL_EXT_tessellation_shader : enable\n";
            } else if (shaderType == ShaderType::Geometry) {
                m_InsertStr += "#extension GL_EXT_geometry_shader : enable\n";
            } else if (shaderType == ShaderType::Vertex || shaderType == ShaderType::Fragment) {
                if (m_RenderContext.GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::gpuShader5))
                    m_InsertStr += "#extension GL_EXT_gpu_shader5 : enable\n";
                if (m_RenderContext.IsAdvancedBlendHwSupportedKHR())
                    m_InsertStr += "#extension GL_KHR_blend_equation_advanced : enable\n";
            }
        } else {
            if (shaderType == ShaderType::Vertex || shaderType == ShaderType::Fragment
                    || shaderType == ShaderType::Geometry) {
                if (m_RenderContext.GetRenderContextType() != QDemonRenderContextValues::GLES2) {
                    m_InsertStr += "#extension GL_ARB_gpu_shader5 : enable\n";
                    m_InsertStr += "#extension GL_ARB_shading_language_420pack : enable\n";
                }
                if (isGLES && m_RenderContext.IsTextureLodSupported())
                    m_InsertStr += "#extension GL_EXT_shader_texture_lod : enable\n";
                if (m_RenderContext.IsShaderImageLoadStoreSupported())
                    m_InsertStr += "#extension GL_ARB_shader_image_load_store : enable\n";
                if (m_RenderContext.IsAtomicCounterBufferSupported())
                    m_InsertStr += "#extension GL_ARB_shader_atomic_counters : enable\n";
                if (m_RenderContext.IsStorageBufferSupported())
                    m_InsertStr += "#extension GL_ARB_shader_storage_buffer_object : enable\n";
                if (m_RenderContext.IsAdvancedBlendHwSupportedKHR())
                    m_InsertStr += "#extension GL_KHR_blend_equation_advanced : enable\n";
            }
        }
    }

    void AddShaderPreprocessor(CRenderString &str, CRegisteredString inKey,
                               ShaderType::Enum shaderType,
                               QDemonConstDataRef<SShaderPreprocessorFeature> inFeatures)
    {
        // Don't use shading language version returned by the driver as it might
        // differ from the context version. Instead use the context type to specify
        // the version string.
        bool isGlES = IQt3DSRenderer::IsGlEsContext(m_RenderContext.GetRenderContextType());
        m_InsertStr.clear();
        int minor = m_RenderContext.format().minorVersion();
        QString versionStr;
        QTextStream stream(&versionStr);
        stream << "#version ";
        const quint32 type = (quint32)m_RenderContext.GetRenderContextType();
        switch (type) {
        case QDemonRenderContextValues::GLES2:
            stream << "1" << minor << "0\n";
            break;
        case QDemonRenderContextValues::GL2:
            stream << "1" << minor << "0\n";
            break;
        case QDemonRenderContextValues::GLES3PLUS:
        case QDemonRenderContextValues::GLES3:
            stream << "3" << minor << "0 es\n";
            break;
        case QDemonRenderContextValues::GL3:
            if (minor == 3)
                stream << "3" << minor << "0\n";
            else
                stream << "1" << 3 + minor << "0\n";
            break;
        case QDemonRenderContextValues::GL4:
            stream << "4" << minor << "0\n";
            break;
        default:
            Q_ASSERT(false);
            break;
        }

        m_InsertStr.append(versionStr.toLatin1().data());

        if (isGlES) {
            if (!IQt3DSRenderer::IsGlEs3Context(m_RenderContext.GetRenderContextType())) {
                if (shaderType == ShaderType::Fragment) {
                    m_InsertStr += "#define fragOutput gl_FragData[0]\n";
                }
            } else {
                m_InsertStr += "#define texture2D texture\n";
            }

            // add extenions strings before any other non-processor token
            AddShaderExtensionStrings(shaderType, isGlES);

            // add precision qualifier depending on backend
            if (IQt3DSRenderer::IsGlEs3Context(m_RenderContext.GetRenderContextType())) {
                m_InsertStr.append("precision highp float;\n"
                                   "precision highp int;\n");
                if( m_RenderContext.GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::gpuShader5) ) {
                    m_InsertStr.append("precision mediump sampler2D;\n"
                                       "precision mediump sampler2DArray;\n"
                                       "precision mediump sampler2DShadow;\n");
                    if (m_RenderContext.IsShaderImageLoadStoreSupported()) {
                        m_InsertStr.append("precision mediump image2D;\n");
                    }
                }

                AddBackwardCompatibilityDefines(shaderType);
            } else {
                // GLES2
                m_InsertStr.append("precision mediump float;\n"
                                   "precision mediump int;\n"
                                   "#define texture texture2D\n");
                if (m_RenderContext.IsTextureLodSupported())
                    m_InsertStr.append("#define textureLod texture2DLodEXT\n");
                else
                    m_InsertStr.append("#define textureLod(s, co, lod) texture2D(s, co)\n");
            }
        } else {
            if (!IQt3DSRenderer::IsGl2Context(m_RenderContext.GetRenderContextType())) {
                m_InsertStr += "#define texture2D texture\n";

                AddShaderExtensionStrings(shaderType, isGlES);

                m_InsertStr += "#if __VERSION__ >= 330\n";

                AddBackwardCompatibilityDefines(shaderType);

                m_InsertStr += "#else\n";
                if (shaderType == ShaderType::Fragment) {
                    m_InsertStr += "#define fragOutput gl_FragData[0]\n";
                }
                m_InsertStr += "#endif\n";
            }
        }

        if (inKey.IsValid()) {
            m_InsertStr += "//Shader name -";
            m_InsertStr += inKey.c_str();
            m_InsertStr += "\n";
        }

        if (shaderType == ShaderType::TessControl) {
            m_InsertStr += "#define TESSELLATION_CONTROL_SHADER 1\n";
            m_InsertStr += "#define TESSELLATION_EVALUATION_SHADER 0\n";
        } else if (shaderType == ShaderType::TessEval) {
            m_InsertStr += "#define TESSELLATION_CONTROL_SHADER 0\n";
            m_InsertStr += "#define TESSELLATION_EVALUATION_SHADER 1\n";
        }

        str.insert(0, m_InsertStr);
        if (inFeatures.size()) {
            QString::size_type insertPos = int(m_InsertStr.size());
            m_InsertStr.clear();
            for (quint32 idx = 0, end = inFeatures.size(); idx < end; ++idx) {
                SShaderPreprocessorFeature feature(inFeatures[idx]);
                m_InsertStr.append("#define ");
                m_InsertStr.append(inFeatures[idx].m_Name.c_str());
                m_InsertStr.append(" ");
                m_InsertStr.append(feature.m_Enabled ? "1" : "0");
                m_InsertStr.append("\n");
            }
            str.insert(insertPos, m_InsertStr);
        }
    }
    // Compile this program overwriting any existing ones.
    QDemonRenderShaderProgram *
    ForceCompileProgram(CRegisteredString inKey, const char8_t *inVert, const char8_t *inFrag,
                        const char8_t *inTessCtrl, const char8_t *inTessEval, const char8_t *inGeom,
                        const SShaderCacheProgramFlags &inFlags,
                        QDemonConstDataRef<SShaderPreprocessorFeature> inFeatures,
                        bool separableProgram, bool fromDisk = false) override
    {
        if (m_ShaderCompilationEnabled == false)
            return nullptr;
        SShaderCacheKey tempKey(inKey);
        tempKey.m_Features.assign(inFeatures.begin(), inFeatures.end());
        tempKey.GenerateHashCode();

        eastl::pair<TShaderMap::iterator, bool> theInserter = m_Shaders.insert(tempKey);
        if (fromDisk) {
            qCInfo(TRACE_INFO) << "Loading from persistent shader cache: '<"
                               << tempKey.m_Key << ">'";
        } else {
            qCInfo(TRACE_INFO) << "Compiling into shader cache: '"
                               << tempKey.m_Key << ">'";
        }

        if (!inVert)
            inVert = "";
        if (!inTessCtrl)
            inTessCtrl = "";
        if (!inTessEval)
            inTessEval = "";
        if (!inGeom)
            inGeom = "";
        if (!inFrag)
            inFrag = "";

        SStackPerfTimer __perfTimer(m_PerfTimer, "Shader Compilation");
        m_VertexCode.assign(inVert);
        m_TessCtrlCode.assign(inTessCtrl);
        m_TessEvalCode.assign(inTessEval);
        m_GeometryCode.assign(inGeom);
        m_FragmentCode.assign(inFrag);
        // Add defines and such so we can write unified shaders that work across platforms.
        // vertex and fragment shaders are optional for separable shaders
        if (!separableProgram || !m_VertexCode.empty())
            AddShaderPreprocessor(m_VertexCode, inKey, ShaderType::Vertex, inFeatures);
        if (!separableProgram || !m_FragmentCode.empty())
            AddShaderPreprocessor(m_FragmentCode, inKey, ShaderType::Fragment, inFeatures);
        // optional shaders
        if (inFlags.IsTessellationEnabled()) {
            Q_ASSERT(m_TessCtrlCode.size() && m_TessEvalCode.size());
            AddShaderPreprocessor(m_TessCtrlCode, inKey, ShaderType::TessControl, inFeatures);
            AddShaderPreprocessor(m_TessEvalCode, inKey, ShaderType::TessEval, inFeatures);
        }
        if (inFlags.IsGeometryShaderEnabled())
            AddShaderPreprocessor(m_GeometryCode, inKey, ShaderType::Geometry, inFeatures);

        theInserter.first->second =
                m_RenderContext
                .CompileSource(inKey, m_VertexCode.c_str(), quint32(m_VertexCode.size()),
                               m_FragmentCode.c_str(), quint32(m_FragmentCode.size()),
                               m_TessCtrlCode.c_str(), quint32(m_TessCtrlCode.size()),
                               m_TessEvalCode.c_str(), quint32(m_TessEvalCode.size()),
                               m_GeometryCode.c_str(), quint32(m_GeometryCode.size()),
                               separableProgram).mShader;
        if (theInserter.first->second) {
            if (m_ShaderCache) {
                IDOMWriter::Scope __writeScope(*m_ShaderCache, "Program");
                m_ShaderCache->Att("key", inKey.c_str());
                CacheFlagsToStr(inFlags, m_FlagString);
                if (m_FlagString.size())
                    m_ShaderCache->Att("glflags", m_FlagString.c_str());
                // write out the GL version.
                {
                    QDemonRenderContextType theContextType =
                            m_RenderContext.GetRenderContextType();
                    ContextTypeToString(theContextType, m_ContextTypeString);
                    m_ShaderCache->Att("gl-context-type", m_ContextTypeString.c_str());
                }
                if (inFeatures.size()) {
                    IDOMWriter::Scope __writeScope(*m_ShaderCache, "Features");
                    for (quint32 idx = 0, end = inFeatures.size(); idx < end; ++idx) {
                        m_ShaderCache->Att(inFeatures[idx].m_Name, inFeatures[idx].m_Enabled);
                    }
                }

                {
                    IDOMWriter::Scope __writeScope(*m_ShaderCache, "VertexCode");
                    m_ShaderCache->Value(inVert);
                }
                {
                    IDOMWriter::Scope __writeScope(*m_ShaderCache, "FragmentCode");
                    m_ShaderCache->Value(inFrag);
                }
                if (m_TessCtrlCode.size()) {
                    IDOMWriter::Scope __writeScope(*m_ShaderCache, "TessControlCode");
                    m_ShaderCache->Value(inTessCtrl);
                }
                if (m_TessEvalCode.size()) {
                    IDOMWriter::Scope __writeScope(*m_ShaderCache, "TessEvalCode");
                    m_ShaderCache->Value(inTessEval);
                }
                if (m_GeometryCode.size()) {
                    IDOMWriter::Scope __writeScope(*m_ShaderCache, "GeometryCode");
                    m_ShaderCache->Value(inGeom);
                }
            }
        }
        return theInserter.first->second;
    }

    virtual QDemonRenderShaderProgram *
    CompileProgram(CRegisteredString inKey, const char8_t *inVert, const char8_t *inFrag,
                   const char8_t *inTessCtrl, const char8_t *inTessEval, const char8_t *inGeom,
                   const SShaderCacheProgramFlags &inFlags,
                   QDemonConstDataRef<SShaderPreprocessorFeature> inFeatures, bool separableProgram) override
    {
        QDemonRenderShaderProgram *theProgram = GetProgram(inKey, inFeatures);
        if (theProgram)
            return theProgram;

        QDemonRenderShaderProgram *retval =
                ForceCompileProgram(inKey, inVert, inFrag, inTessCtrl, inTessEval, inGeom, inFlags,
                                    inFeatures, separableProgram);
        if (m_CacheFilePath.c_str() && m_ShaderCache && m_ShaderCompilationEnabled) {
            CFileSeekableIOStream theStream(m_CacheFilePath.c_str(), FileWriteFlags());
            if (theStream.IsOpen()) {
                QDemonScopedRefCounted<IStringTable> theStringTable(
                            IStringTable::CreateStringTable(m_RenderContext.GetAllocator()));
                CDOMSerializer::WriteXMLHeader(theStream);
                CDOMSerializer::Write(m_RenderContext.GetAllocator(),
                                      *m_ShaderCache->GetTopElement(), theStream, *theStringTable);
            }
        }
        return retval;
    }

    void BootupDOMWriter()
    {
        QDemonScopedRefCounted<IStringTable> theStringTable(
                    IStringTable::CreateStringTable(m_RenderContext.GetAllocator()));
        m_ShaderCache = IDOMWriter::CreateDOMWriter(m_RenderContext.GetAllocator(),
                                                    "Qt3DSShaderCache", theStringTable)
                .first;
        m_ShaderCache->Att("cache_version", IShaderCache::GetShaderVersion());
    }

    void SetShaderCachePersistenceEnabled(const char8_t *inDirectory) override
    {
        if (inDirectory == nullptr) {
            m_ShaderCache = nullptr;
            return;
        }
        BootupDOMWriter();
        m_CacheFilePath = QDir(inDirectory).filePath(GetShaderCacheFileName()).toStdString();

        QDemonScopedRefCounted<IRefCountedInputStream> theInStream =
                m_InputStreamFactory.GetStreamForFile(m_CacheFilePath.c_str());
        if (theInStream) {
            SStackPerfTimer __perfTimer(m_PerfTimer, "ShaderCache - Load");
            QDemonScopedRefCounted<IStringTable> theStringTable(
                        IStringTable::CreateStringTable(m_RenderContext.GetAllocator()));
            QDemonScopedRefCounted<IDOMFactory> theFactory(
                        IDOMFactory::CreateDOMFactory(m_RenderContext.GetAllocator(), theStringTable));
            eastl::vector<SShaderPreprocessorFeature> theFeatures;

            SDOMElement *theElem = CDOMSerializer::Read(*theFactory, *theInStream).second;
            if (theElem) {
                QDemonScopedRefCounted<IDOMReader> theReader = IDOMReader::CreateDOMReader(
                            m_RenderContext.GetAllocator(), *theElem, theStringTable, theFactory);
                quint32 theAttValue = 0;
                theReader->Att("cache_version", theAttValue);
                if (theAttValue == IShaderCache::GetShaderVersion()) {
                    CRenderString loadVertexData;
                    CRenderString loadFragmentData;
                    CRenderString loadTessControlData;
                    CRenderString loadTessEvalData;
                    CRenderString loadGeometryData;
                    CRenderString shaderTypeString;
                    IStringTable &theStringTable(m_RenderContext.GetStringTable());
                    for (bool success = theReader->MoveToFirstChild(); success;
                         success = theReader->MoveToNextSibling()) {
                        const char8_t *theKeyStr = nullptr;
                        theReader->UnregisteredAtt("key", theKeyStr);

                        CRegisteredString theKey = theStringTable.RegisterStr(theKeyStr);
                        if (theKey.IsValid()) {
                            m_FlagString.clear();
                            const char8_t *theFlagStr = "";
                            SShaderCacheProgramFlags theFlags;
                            if (theReader->UnregisteredAtt("glflags", theFlagStr)) {
                                m_FlagString.assign(theFlagStr);
                                theFlags = CacheFlagsToStr(m_FlagString);
                            }

                            m_ContextTypeString.clear();
                            if (theReader->UnregisteredAtt("gl-context-type", theFlagStr))
                                m_ContextTypeString.assign(theFlagStr);

                            theFeatures.clear();
                            {
                                IDOMReader::Scope __featureScope(*theReader);
                                if (theReader->MoveToFirstChild("Features")) {
                                    for (SDOMAttribute *theAttribute =
                                         theReader->GetFirstAttribute();
                                         theAttribute;
                                         theAttribute = theAttribute->m_NextAttribute) {
                                        bool featureValue = false;
                                        StringConversion<bool>().StrTo(theAttribute->m_Value,
                                                                       featureValue);
                                        theFeatures.push_back(SShaderPreprocessorFeature(
                                                                  theStringTable.RegisterStr(
                                                                      theAttribute->m_Name.c_str()),
                                                                  featureValue));
                                    }
                                }
                            }

                            QDemonRenderContextType theContextType =
                                    StringToContextType(m_ContextTypeString);
                            if (((quint32)theContextType != 0)
                                    && (theContextType & m_RenderContext.GetRenderContextType())
                                    == theContextType) {
                                IDOMReader::Scope __readerScope(*theReader);
                                loadVertexData.clear();
                                loadFragmentData.clear();
                                loadTessControlData.clear();
                                loadTessEvalData.clear();
                                loadGeometryData.clear();

                                // Vertex *MUST* be the first
                                // Todo deal with pure compute shader programs
                                if (theReader->MoveToFirstChild("VertexCode")) {
                                    const char8_t *theValue = nullptr;
                                    theReader->Value(theValue);
                                    loadVertexData.assign(theValue);
                                    while (theReader->MoveToNextSibling()) {
                                        theReader->Value(theValue);

                                        shaderTypeString.assign(
                                                    theReader->GetElementName().c_str());
                                        ShaderType::Enum shaderType =
                                                StringToShaderType(shaderTypeString);

                                        if (shaderType == ShaderType::Fragment)
                                            loadFragmentData.assign(theValue);
                                        else if (shaderType == ShaderType::TessControl)
                                            loadTessControlData.assign(theValue);
                                        else if (shaderType == ShaderType::TessEval)
                                            loadTessEvalData.assign(theValue);
                                        else if (shaderType == ShaderType::Geometry)
                                            loadGeometryData.assign(theValue);
                                    }
                                }

                                if (loadVertexData.size()
                                        && (loadFragmentData.size() || loadGeometryData.size())) {

                                    QDemonRenderShaderProgram *theShader = ForceCompileProgram(
                                                theKey, loadVertexData.c_str(), loadFragmentData.c_str(),
                                                loadTessControlData.c_str(), loadTessEvalData.c_str(),
                                                loadGeometryData.c_str(), theFlags,
                                                toDataRef(theFeatures.data(),
                                                          (quint32)theFeatures.size()),
                                                false, true /*fromDisk*/);
                                    // If something doesn't save or load correctly, get the runtime
                                    // to re-generate.
                                    if (!theShader)
                                        m_Shaders.erase(theKey);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    bool IsShaderCachePersistenceEnabled() const override { return m_ShaderCache != nullptr; }

    void SetShaderCompilationEnabled(bool inEnableShaderCompilation) override
    {
        m_ShaderCompilationEnabled = inEnableShaderCompilation;
    }
};
}

size_t HashShaderFeatureSet(QDemonConstDataRef<SShaderPreprocessorFeature> inFeatureSet)
{
    size_t retval(0);
    for (quint32 idx = 0, end = inFeatureSet.size(); idx < end; ++idx) {
        // From previous implementation, it seems we need to ignore the order of the features.
        // But we need to bind the feature flag together with its name, so that the flags will
        // influence
        // the final hash not only by the true-value count.
        retval = retval
                ^ (inFeatureSet[idx].m_Name.hash() * eastl::hash<bool>()(inFeatureSet[idx].m_Enabled));
    }
    return retval;
}

bool SShaderPreprocessorFeature::operator<(const SShaderPreprocessorFeature &other) const
{
    return strcmp(m_Name.c_str(), other.m_Name.c_str()) < 0;
}

bool SShaderPreprocessorFeature::operator==(const SShaderPreprocessorFeature &other) const
{
    return m_Name == other.m_Name && m_Enabled == other.m_Enabled;
}

IShaderCache &IShaderCache::CreateShaderCache(QDemonRenderContext &inContext,
                                              IInputStreamFactory &inInputStreamFactory,
                                              IPerfTimer &inPerfTimer)
{
    return *QDEMON_NEW(inContext.GetAllocator(), ShaderCache)(inContext, inInputStreamFactory,
                                                              inPerfTimer);
}

QT_END_NAMESPACE
