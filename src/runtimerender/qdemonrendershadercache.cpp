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
#include "qdemonrendershadercache.h"

#include <QtDemon/qdemonutils.h>

#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>

#include <QtDemonRuntimeRender/qdemonrenderinputstreamfactory.h>
#include <QtDemonRuntimeRender/qdemonrenderer.h>
#include <QtDemonRender/qdemonrenderlogging.h>

#include <QtCore/QRegularExpression>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

namespace {
//using QDemonRenderContextScopedProperty;
const char *TessellationEnabledStr = "TessellationStageEnabled";
const char *GeometryEnabledStr = "GeometryStageEnabled";
inline void AppendFlagValue(QString &inStr, const char *flag)
{
    if (inStr.length())
        inStr.append(QStringLiteral(","));
    inStr.append(QString::fromLocal8Bit(flag));
}
inline void CacheFlagsToStr(const SShaderCacheProgramFlags &inFlags, QString &inString)
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

inline ShaderType::Enum StringToShaderType(QString &inShaderType)
{
    ShaderType::Enum retval = ShaderType::Vertex;

    if (inShaderType.size() == 0)
        return retval;

    if (!inShaderType.compare(QStringLiteral("VertexCode")))
        retval = ShaderType::Vertex;
    else if (!inShaderType.compare(QStringLiteral("FragmentCode")))
        retval = ShaderType::Fragment;
    else if (!inShaderType.compare(QStringLiteral("TessControlCode")))
        retval = ShaderType::TessControl;
    else if (!inShaderType.compare(QStringLiteral("TessEvalCode")))
        retval = ShaderType::TessEval;
    else if (!inShaderType.compare(QStringLiteral("GeometryCode")))
        retval = ShaderType::Geometry;
    else
        Q_ASSERT(false);

    return retval;
}

inline SShaderCacheProgramFlags CacheFlagsToStr(const QString &inString)
{
    SShaderCacheProgramFlags retval;
    if (inString.contains(QString::fromLocal8Bit(TessellationEnabledStr)))
        retval.SetTessellationEnabled(true);
    if (inString.contains(QString::fromLocal8Bit(GeometryEnabledStr)))
        retval.SetGeometryShaderEnabled(true);
    return retval;
}

typedef QPair<const char *, QDemonRenderContextValues::Enum> TStringToContextValuePair;

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
                                QString &outContextType)
{
    outContextType.clear();
    for (size_t idx = 0, end = g_NumStringToContextValueEntries; idx < end; ++idx) {
        if (inType & g_StringToContextTypeValue[idx].second) {
            if (outContextType.size())
                outContextType.append(QStringLiteral("|"));
            outContextType.append(QString::fromLocal8Bit(g_StringToContextTypeValue[idx].first));
        }
    }
}

inline QDemonRenderContextType StringToContextType(const QString &inContextType)
{
    QDemonRenderContextType retval;
    char tempBuffer[128];
    memZero(tempBuffer, 128);
    const QString::size_type lastTempBufIdx = 127;
    QString::size_type pos = 0, lastpos = 0;
    if (inContextType.size() == 0)
        return retval;

    do {
        pos = int(inContextType.indexOf('|', lastpos));
        if (pos == -1)
            pos = int(inContextType.size());
        {
            size_t sectionLen = size_t(qMin(pos - lastpos, lastTempBufIdx));
            ::memcpy(tempBuffer, inContextType.data() + lastpos, sectionLen);
            tempBuffer[lastTempBufIdx] = 0;
            for (size_t idx = 0, end = g_NumStringToContextValueEntries; idx < end; ++idx) {
                if (strcmp(g_StringToContextTypeValue[idx].first, tempBuffer) == 0)
                    retval = retval | g_StringToContextTypeValue[idx].second;
            }
        }
        // iterate past the bar
        ++pos;
        lastpos = pos;
    } while (pos < inContextType.size() && pos != -1);

    return retval;
}
}

uint qHash(const SShaderCacheKey &key)
{
    return key.m_HashCode;
}

namespace {

struct ShaderCache : public IShaderCache
{
    typedef QHash<SShaderCacheKey, QSharedPointer<QDemonRenderShaderProgram>> TShaderMap;
    QDemonRenderContext &m_RenderContext;
    IPerfTimer &m_PerfTimer;
    TShaderMap m_Shaders;
    QString m_CacheFilePath;
    QString m_VertexCode;
    QString m_TessCtrlCode;
    QString m_TessEvalCode;
    QString m_GeometryCode;
    QString m_FragmentCode;
    QString m_InsertStr;
    QString m_FlagString;
    QString m_ContextTypeString;
    SShaderCacheKey m_TempKey;

    // ### Shader Chache Writing Code is disabled
    //QSharedPointer<IDOMWriter> m_ShaderCache;
    IInputStreamFactory &m_InputStreamFactory;
    bool m_ShaderCompilationEnabled;

    ShaderCache(QDemonRenderContext &ctx, IInputStreamFactory &inInputStreamFactory, IPerfTimer &inPerfTimer)
        : m_RenderContext(ctx)
        , m_PerfTimer(inPerfTimer)
        , m_InputStreamFactory(inInputStreamFactory)
        , m_ShaderCompilationEnabled(true)
    {
    }

    QSharedPointer<QDemonRenderShaderProgram> GetProgram(QString inKey, const QVector<SShaderPreprocessorFeature> &inFeatures) override
    {
        m_TempKey.m_Key = inKey;
        m_TempKey.m_Features = inFeatures;
        m_TempKey.GenerateHashCode();
        TShaderMap::iterator theIter = m_Shaders.find(m_TempKey);
        if (theIter != m_Shaders.end())
            return theIter.value();
        return nullptr;
    }

    void AddBackwardCompatibilityDefines(ShaderType::Enum shaderType)
    {
        if (shaderType == ShaderType::Vertex || shaderType == ShaderType::TessControl
                || shaderType == ShaderType::TessEval || shaderType == ShaderType::Geometry) {
            m_InsertStr += QStringLiteral("#define attribute in\n");
            m_InsertStr += QStringLiteral("#define varying out\n");
        } else if (shaderType == ShaderType::Fragment) {
            m_InsertStr += QStringLiteral("#define varying in\n");
            m_InsertStr += QStringLiteral("#define texture2D texture\n");
            m_InsertStr += QStringLiteral("#define gl_FragColor fragOutput\n");

            if (m_RenderContext.IsAdvancedBlendHwSupportedKHR())
                m_InsertStr += QStringLiteral("layout(blend_support_all_equations) out;\n ");
            m_InsertStr += QStringLiteral("out vec4 fragOutput;\n");
        }
    }

    void AddShaderExtensionStrings(ShaderType::Enum shaderType, bool isGLES)
    {
        if (isGLES) {
            if (m_RenderContext.IsStandardDerivativesSupported())
                m_InsertStr += QStringLiteral("#extension GL_OES_standard_derivatives : enable\n");
            else
                m_InsertStr += QStringLiteral("#extension GL_OES_standard_derivatives : disable\n");
        }

        if (IQDemonRenderer::IsGlEs3Context(m_RenderContext.GetRenderContextType())) {
            if (shaderType == ShaderType::TessControl || shaderType == ShaderType::TessEval) {
                m_InsertStr += QStringLiteral("#extension GL_EXT_tessellation_shader : enable\n");
            } else if (shaderType == ShaderType::Geometry) {
                m_InsertStr += QStringLiteral("#extension GL_EXT_geometry_shader : enable\n");
            } else if (shaderType == ShaderType::Vertex || shaderType == ShaderType::Fragment) {
                if (m_RenderContext.GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::gpuShader5))
                    m_InsertStr += QStringLiteral("#extension GL_EXT_gpu_shader5 : enable\n");
                if (m_RenderContext.IsAdvancedBlendHwSupportedKHR())
                    m_InsertStr += QStringLiteral("#extension GL_KHR_blend_equation_advanced : enable\n");
            }
        } else {
            if (shaderType == ShaderType::Vertex || shaderType == ShaderType::Fragment
                    || shaderType == ShaderType::Geometry) {
                if (m_RenderContext.GetRenderContextType() != QDemonRenderContextValues::GLES2) {
                    m_InsertStr += QStringLiteral("#extension GL_ARB_gpu_shader5 : enable\n");
                    m_InsertStr += QStringLiteral("#extension GL_ARB_shading_language_420pack : enable\n");
                }
                if (isGLES && m_RenderContext.IsTextureLodSupported())
                    m_InsertStr += QStringLiteral("#extension GL_EXT_shader_texture_lod : enable\n");
                if (m_RenderContext.IsShaderImageLoadStoreSupported())
                    m_InsertStr += QStringLiteral("#extension GL_ARB_shader_image_load_store : enable\n");
                if (m_RenderContext.IsAtomicCounterBufferSupported())
                    m_InsertStr += QStringLiteral("#extension GL_ARB_shader_atomic_counters : enable\n");
                if (m_RenderContext.IsStorageBufferSupported())
                    m_InsertStr += QStringLiteral("#extension GL_ARB_shader_storage_buffer_object : enable\n");
                if (m_RenderContext.IsAdvancedBlendHwSupportedKHR())
                    m_InsertStr += QStringLiteral("#extension GL_KHR_blend_equation_advanced : enable\n");
            }
        }
    }

    void AddShaderPreprocessor(QString &str, QString inKey,
                               ShaderType::Enum shaderType,
                               const QVector<SShaderPreprocessorFeature> &inFeatures)
    {
        // Don't use shading language version returned by the driver as it might
        // differ from the context version. Instead use the context type to specify
        // the version string.
        bool isGlES = IQDemonRenderer::IsGlEsContext(m_RenderContext.GetRenderContextType());
        m_InsertStr.clear();
        int minor = m_RenderContext.format().minorVersion();
        QString versionStr;
        QTextStream stream(&versionStr);
        stream << "#version ";
        const quint32 type = quint32(m_RenderContext.GetRenderContextType());
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

        m_InsertStr.append(versionStr);

        if (isGlES) {
            if (!IQDemonRenderer::IsGlEs3Context(m_RenderContext.GetRenderContextType())) {
                if (shaderType == ShaderType::Fragment) {
                    m_InsertStr += QStringLiteral("#define fragOutput gl_FragData[0]\n");
                }
            } else {
                m_InsertStr += QStringLiteral("#define texture2D texture\n");
            }

            // add extenions strings before any other non-processor token
            AddShaderExtensionStrings(shaderType, isGlES);

            // add precision qualifier depending on backend
            if (IQDemonRenderer::IsGlEs3Context(m_RenderContext.GetRenderContextType())) {
                m_InsertStr.append(QStringLiteral("precision highp float;\n"
                                                  "precision highp int;\n"));
                if( m_RenderContext.GetRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::gpuShader5) ) {
                    m_InsertStr.append(QStringLiteral("precision mediump sampler2D;\n"
                                                      "precision mediump sampler2DArray;\n"
                                                      "precision mediump sampler2DShadow;\n"));
                    if (m_RenderContext.IsShaderImageLoadStoreSupported()) {
                        m_InsertStr.append(QStringLiteral("precision mediump image2D;\n"));
                    }
                }

                AddBackwardCompatibilityDefines(shaderType);
            } else {
                // GLES2
                m_InsertStr.append(QStringLiteral("precision mediump float;\n"
                                                  "precision mediump int;\n"
                                                  "#define texture texture2D\n"));
                if (m_RenderContext.IsTextureLodSupported())
                    m_InsertStr.append(QStringLiteral("#define textureLod texture2DLodEXT\n"));
                else
                    m_InsertStr.append(QStringLiteral("#define textureLod(s, co, lod) texture2D(s, co)\n"));
            }
        } else {
            if (!IQDemonRenderer::IsGl2Context(m_RenderContext.GetRenderContextType())) {
                m_InsertStr += QStringLiteral("#define texture2D texture\n");

                AddShaderExtensionStrings(shaderType, isGlES);

                m_InsertStr += QStringLiteral("#if __VERSION__ >= 330\n");

                AddBackwardCompatibilityDefines(shaderType);

                m_InsertStr += QStringLiteral("#else\n");
                if (shaderType == ShaderType::Fragment) {
                    m_InsertStr += QStringLiteral("#define fragOutput gl_FragData[0]\n");
                }
                m_InsertStr += QStringLiteral("#endif\n");
            }
        }

        if (!inKey.isNull()) {
            m_InsertStr += QStringLiteral("//Shader name -");
            m_InsertStr += inKey;
            m_InsertStr += QStringLiteral("\n");
        }

        if (shaderType == ShaderType::TessControl) {
            m_InsertStr += QStringLiteral("#define TESSELLATION_CONTROL_SHADER 1\n");
            m_InsertStr += QStringLiteral("#define TESSELLATION_EVALUATION_SHADER 0\n");
        } else if (shaderType == ShaderType::TessEval) {
            m_InsertStr += QStringLiteral("#define TESSELLATION_CONTROL_SHADER 0\n");
            m_InsertStr += QStringLiteral("#define TESSELLATION_EVALUATION_SHADER 1\n");
        }

        str.insert(0, m_InsertStr);
        if (inFeatures.size()) {
            QString::size_type insertPos = int(m_InsertStr.size());
            m_InsertStr.clear();
            for (int idx = 0, end = inFeatures.size(); idx < end; ++idx) {
                SShaderPreprocessorFeature feature(inFeatures[idx]);
                m_InsertStr.append(QStringLiteral("#define "));
                m_InsertStr.append(inFeatures[idx].m_Name);
                m_InsertStr.append(QStringLiteral(" "));
                m_InsertStr.append(feature.m_Enabled ? QStringLiteral("1") : QStringLiteral("0"));
                m_InsertStr.append(QStringLiteral("\n"));
            }
            str.insert(insertPos, m_InsertStr);
        }
    }
    // Compile this program overwriting any existing ones.
    QSharedPointer<QDemonRenderShaderProgram>
    ForceCompileProgram(QString inKey, const char *inVert, const char *inFrag,
                        const char *inTessCtrl, const char *inTessEval, const char *inGeom,
                        const SShaderCacheProgramFlags &inFlags,
                        const QVector<SShaderPreprocessorFeature> &inFeatures,
                        bool separableProgram, bool fromDisk = false) override
    {
        if (m_ShaderCompilationEnabled == false)
            return nullptr;
        SShaderCacheKey tempKey(inKey);
        tempKey.m_Features = inFeatures;
        tempKey.GenerateHashCode();

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

        //SStackPerfTimer __perfTimer(m_PerfTimer, "Shader Compilation");
        m_VertexCode = QString::fromLocal8Bit(inVert);
        m_TessCtrlCode = QString::fromLocal8Bit(inTessCtrl);
        m_TessEvalCode = QString::fromLocal8Bit(inTessEval);
        m_GeometryCode = QString::fromLocal8Bit(inGeom);
        m_FragmentCode = QString::fromLocal8Bit(inFrag);
        // Add defines and such so we can write unified shaders that work across platforms.
        // vertex and fragment shaders are optional for separable shaders
        if (!separableProgram || !m_VertexCode.isEmpty())
            AddShaderPreprocessor(m_VertexCode, inKey, ShaderType::Vertex, inFeatures);
        if (!separableProgram || !m_FragmentCode.isEmpty())
            AddShaderPreprocessor(m_FragmentCode, inKey, ShaderType::Fragment, inFeatures);
        // optional shaders
        if (inFlags.IsTessellationEnabled()) {
            Q_ASSERT(m_TessCtrlCode.size() && m_TessEvalCode.size());
            AddShaderPreprocessor(m_TessCtrlCode, inKey, ShaderType::TessControl, inFeatures);
            AddShaderPreprocessor(m_TessEvalCode, inKey, ShaderType::TessEval, inFeatures);
        }
        if (inFlags.IsGeometryShaderEnabled())
            AddShaderPreprocessor(m_GeometryCode, inKey, ShaderType::Geometry, inFeatures);
        // ### this call is awful because of QString conversions!
        auto shaderProgram = m_RenderContext.CompileSource(inKey.toLocal8Bit().constData(),
                                                           m_VertexCode.toLocal8Bit().constData(),
                                                           quint32(m_VertexCode.toLocal8Bit().size()),
                                                           m_FragmentCode.toLocal8Bit().constData(),
                                                           quint32(m_FragmentCode.toLocal8Bit().size()),
                                                           m_TessCtrlCode.toLocal8Bit().constData(),
                                                           quint32(m_TessCtrlCode.toLocal8Bit().size()),
                                                           m_TessEvalCode.toLocal8Bit().constData(),
                                                           quint32(m_TessEvalCode.toLocal8Bit().size()),
                                                           m_GeometryCode.toLocal8Bit().constData(),
                                                           quint32(m_GeometryCode.toLocal8Bit().size()),
                                                           separableProgram).mShader;
        m_Shaders.insert(tempKey, shaderProgram);
        if (shaderProgram) {
            // ### Shader Chache Writing Code is disabled
//            if (m_ShaderCache) {
//                IDOMWriter::Scope __writeScope(*m_ShaderCache, "Program");
//                m_ShaderCache->Att("key", inKey.toLocal8Bit().constData());
//                CacheFlagsToStr(inFlags, m_FlagString);
//                if (m_FlagString.size())
//                    m_ShaderCache->Att("glflags", m_FlagString.toLocal8Bit().constData());
//                // write out the GL version.
//                {
//                    QDemonRenderContextType theContextType =
//                            m_RenderContext.GetRenderContextType();
//                    ContextTypeToString(theContextType, m_ContextTypeString);
//                    m_ShaderCache->Att("gl-context-type", m_ContextTypeString.toLocal8Bit().constData());
//                }
//                if (inFeatures.size()) {
//                    IDOMWriter::Scope __writeScope(*m_ShaderCache, "Features");
//                    for (int idx = 0, end = inFeatures.size(); idx < end; ++idx) {
//                        m_ShaderCache->Att(inFeatures[idx].m_Name, inFeatures[idx].m_Enabled);
//                    }
//                }

//                {
//                    IDOMWriter::Scope __writeScope(*m_ShaderCache, "VertexCode");
//                    m_ShaderCache->Value(inVert);
//                }
//                {
//                    IDOMWriter::Scope __writeScope(*m_ShaderCache, "FragmentCode");
//                    m_ShaderCache->Value(inFrag);
//                }
//                if (m_TessCtrlCode.size()) {
//                    IDOMWriter::Scope __writeScope(*m_ShaderCache, "TessControlCode");
//                    m_ShaderCache->Value(inTessCtrl);
//                }
//                if (m_TessEvalCode.size()) {
//                    IDOMWriter::Scope __writeScope(*m_ShaderCache, "TessEvalCode");
//                    m_ShaderCache->Value(inTessEval);
//                }
//                if (m_GeometryCode.size()) {
//                    IDOMWriter::Scope __writeScope(*m_ShaderCache, "GeometryCode");
//                    m_ShaderCache->Value(inGeom);
//                }
//            }
        }
        return shaderProgram;
    }

    virtual QSharedPointer<QDemonRenderShaderProgram>
    CompileProgram(QString inKey, const char *inVert, const char *inFrag,
                   const char *inTessCtrl, const char *inTessEval, const char *inGeom,
                   const SShaderCacheProgramFlags &inFlags,
                   const QVector<SShaderPreprocessorFeature> &inFeatures, bool separableProgram) override
    {
        QSharedPointer<QDemonRenderShaderProgram> theProgram = GetProgram(inKey, inFeatures);
        if (theProgram)
            return theProgram;

        QSharedPointer<QDemonRenderShaderProgram> retval =
                ForceCompileProgram(inKey, inVert, inFrag, inTessCtrl, inTessEval, inGeom, inFlags,
                                    inFeatures, separableProgram);
        // ### Shader Chache Writing Code is disabled
//        if (m_CacheFilePath.toLocal8Bit().constData() && m_ShaderCache && m_ShaderCompilationEnabled) {
//            CFileSeekableIOStream theStream(m_CacheFilePath.toLocal8Bit().constData(), FileWriteFlags());
//            if (theStream.IsOpen()) {
//                CDOMSerializer::WriteXMLHeader(theStream);
//                CDOMSerializer::Write(*m_ShaderCache->GetTopElement(), theStream);
//            }
//        }
        return retval;
    }

//    void BootupDOMWriter()
//    {
//        m_ShaderCache = IDOMWriter::CreateDOMWriter("QDemonShaderCache").first;
//        m_ShaderCache->Att("cache_version", IShaderCache::GetShaderVersion());
//    }

    void SetShaderCachePersistenceEnabled(const char *inDirectory) override
    {
        // ### Shader Chache Writing Code is disabled
        Q_UNUSED(inDirectory)

//        if (inDirectory == nullptr) {
//            m_ShaderCache = nullptr;
//            return;
//        }
//        BootupDOMWriter();
//        m_CacheFilePath = QDir(inDirectory).filePath(GetShaderCacheFileName()).toStdString();

//        QSharedPointer<IRefCountedInputStream> theInStream =
//                m_InputStreamFactory.GetStreamForFile(m_CacheFilePath.c_str());
//        if (theInStream) {
//            SStackPerfTimer __perfTimer(m_PerfTimer, "ShaderCache - Load");
//            QSharedPointer<IDOMFactory> theFactory(
//                        IDOMFactory::CreateDOMFactory(m_RenderContext.GetAllocator(), theStringTable));
//            QVector<SShaderPreprocessorFeature> theFeatures;

//            SDOMElement *theElem = CDOMSerializer::Read(*theFactory, *theInStream).second;
//            if (theElem) {
//                QSharedPointer<IDOMReader> theReader = IDOMReader::CreateDOMReader(
//                            m_RenderContext.GetAllocator(), *theElem, theStringTable, theFactory);
//                quint32 theAttValue = 0;
//                theReader->Att("cache_version", theAttValue);
//                if (theAttValue == IShaderCache::GetShaderVersion()) {
//                    QString loadVertexData;
//                    QString loadFragmentData;
//                    QString loadTessControlData;
//                    QString loadTessEvalData;
//                    QString loadGeometryData;
//                    QString shaderTypeString;
//                    for (bool success = theReader->MoveToFirstChild(); success;
//                         success = theReader->MoveToNextSibling()) {
//                        const char *theKeyStr = nullptr;
//                        theReader->UnregisteredAtt("key", theKeyStr);

//                        QString theKey = QString::fromLocal8Bit(theKeyStr);
//                        if (theKey.IsValid()) {
//                            m_FlagString.clear();
//                            const char *theFlagStr = "";
//                            SShaderCacheProgramFlags theFlags;
//                            if (theReader->UnregisteredAtt("glflags", theFlagStr)) {
//                                m_FlagString.assign(theFlagStr);
//                                theFlags = CacheFlagsToStr(m_FlagString);
//                            }

//                            m_ContextTypeString.clear();
//                            if (theReader->UnregisteredAtt("gl-context-type", theFlagStr))
//                                m_ContextTypeString.assign(theFlagStr);

//                            theFeatures.clear();
//                            {
//                                IDOMReader::Scope __featureScope(*theReader);
//                                if (theReader->MoveToFirstChild("Features")) {
//                                    for (SDOMAttribute *theAttribute =
//                                         theReader->GetFirstAttribute();
//                                         theAttribute;
//                                         theAttribute = theAttribute->m_NextAttribute) {
//                                        bool featureValue = false;
//                                        StringConversion<bool>().StrTo(theAttribute->m_Value,
//                                                                       featureValue);
//                                        theFeatures.push_back(SShaderPreprocessorFeature(
//                                                                  QString::fromLocal8Bit(
//                                                                      theAttribute->m_Name.c_str()),
//                                                                  featureValue));
//                                    }
//                                }
//                            }

//                            QDemonRenderContextType theContextType =
//                                    StringToContextType(m_ContextTypeString);
//                            if (((quint32)theContextType != 0)
//                                    && (theContextType & m_RenderContext.GetRenderContextType())
//                                    == theContextType) {
//                                IDOMReader::Scope __readerScope(*theReader);
//                                loadVertexData.clear();
//                                loadFragmentData.clear();
//                                loadTessControlData.clear();
//                                loadTessEvalData.clear();
//                                loadGeometryData.clear();

//                                // Vertex *MUST* be the first
//                                // Todo deal with pure compute shader programs
//                                if (theReader->MoveToFirstChild("VertexCode")) {
//                                    const char *theValue = nullptr;
//                                    theReader->Value(theValue);
//                                    loadVertexData.assign(theValue);
//                                    while (theReader->MoveToNextSibling()) {
//                                        theReader->Value(theValue);

//                                        shaderTypeString.assign(
//                                                    theReader->GetElementName().c_str());
//                                        ShaderType::Enum shaderType =
//                                                StringToShaderType(shaderTypeString);

//                                        if (shaderType == ShaderType::Fragment)
//                                            loadFragmentData.assign(theValue);
//                                        else if (shaderType == ShaderType::TessControl)
//                                            loadTessControlData.assign(theValue);
//                                        else if (shaderType == ShaderType::TessEval)
//                                            loadTessEvalData.assign(theValue);
//                                        else if (shaderType == ShaderType::Geometry)
//                                            loadGeometryData.assign(theValue);
//                                    }
//                                }

//                                if (loadVertexData.size()
//                                        && (loadFragmentData.size() || loadGeometryData.size())) {

//                                    QSharedPointer<QDemonRenderShaderProgram> theShader = ForceCompileProgram(
//                                                theKey, loadVertexData.toLocal8Bit().constData(), loadFragmentData.toLocal8Bit().constData(),
//                                                loadTessControlData.toLocal8Bit().constData(), loadTessEvalData.toLocal8Bit().constData(),
//                                                loadGeometryData.toLocal8Bit().constData(), theFlags,
//                                                theFeatures, false, true /*fromDisk*/);
//                                    // If something doesn't save or load correctly, get the runtime
//                                    // to re-generate.
//                                    if (!theShader)
//                                        m_Shaders.remove(theKey);
//                                }
//                            }
//                        }
//                    }
//                }
//            }
//        }
    }

    bool IsShaderCachePersistenceEnabled() const override
    {
        // ### Shader Chache Writing Code is disabled
        //return m_ShaderCache != nullptr;
        return false;
    }

    void SetShaderCompilationEnabled(bool inEnableShaderCompilation) override
    {
        m_ShaderCompilationEnabled = inEnableShaderCompilation;
    }
};
}

uint HashShaderFeatureSet(QVector<SShaderPreprocessorFeature> inFeatureSet)
{
    uint retval(0);
    for (int idx = 0, end = inFeatureSet.size(); idx < end; ++idx) {
        // From previous implementation, it seems we need to ignore the order of the features.
        // But we need to bind the feature flag together with its name, so that the flags will
        // influence
        // the final hash not only by the true-value count.
        retval = retval
                ^ (qHash(inFeatureSet[idx].m_Name) * qHash(inFeatureSet[idx].m_Enabled));
    }
    return retval;
}

bool SShaderPreprocessorFeature::operator<(const SShaderPreprocessorFeature &other) const
{
    return QString::compare(m_Name, other.m_Name) < 0;
}

bool SShaderPreprocessorFeature::operator==(const SShaderPreprocessorFeature &other) const
{
    return m_Name == other.m_Name && m_Enabled == other.m_Enabled;
}

IShaderCache &IShaderCache::CreateShaderCache(QDemonRenderContext &inContext,
                                              IInputStreamFactory &inInputStreamFactory,
                                              IPerfTimer &inPerfTimer)
{
    return *new ShaderCache(inContext, inInputStreamFactory, inPerfTimer);
}

QT_END_NAMESPACE
