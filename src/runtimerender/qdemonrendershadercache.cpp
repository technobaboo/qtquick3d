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
inline void CacheFlagsToStr(const QDemonShaderCacheProgramFlags &inFlags, QString &inString)
{
    inString.clear();
    if (inFlags.isTessellationEnabled())
        AppendFlagValue(inString, TessellationEnabledStr);
    if (inFlags.isGeometryShaderEnabled())
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

inline QDemonShaderCacheProgramFlags CacheFlagsToStr(const QString &inString)
{
    QDemonShaderCacheProgramFlags retval;
    if (inString.contains(QString::fromLocal8Bit(TessellationEnabledStr)))
        retval.setTessellationEnabled(true);
    if (inString.contains(QString::fromLocal8Bit(GeometryEnabledStr)))
        retval.setGeometryShaderEnabled(true);
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

uint qHash(const QDemonShaderCacheKey &key)
{
    return key.m_hashCode;
}

namespace {

struct ShaderCache : public QDemonShaderCacheInterface
{
    typedef QHash<QDemonShaderCacheKey, QSharedPointer<QDemonRenderShaderProgram>> TShaderMap;
    QSharedPointer<QDemonRenderContext> m_renderContext;
    QSharedPointer<QDemonPerfTimerInterface> m_perfTimer;
    TShaderMap m_shaders;
    QString m_cacheFilePath;
    QString m_vertexCode;
    QString m_tessCtrlCode;
    QString m_tessEvalCode;
    QString m_geometryCode;
    QString m_fragmentCode;
    QString m_insertStr;
    QString m_flagString;
    QString m_contextTypeString;
    QDemonShaderCacheKey m_tempKey;

    // ### Shader Chache Writing Code is disabled
    //QSharedPointer<IDOMWriter> m_ShaderCache;
    QSharedPointer<QDemonInputStreamFactoryInterface> m_inputStreamFactory;
    bool m_shaderCompilationEnabled;

    ShaderCache(QSharedPointer<QDemonRenderContext> ctx,
                QSharedPointer<QDemonInputStreamFactoryInterface> inInputStreamFactory,
                QSharedPointer<QDemonPerfTimerInterface> inPerfTimer)
        : m_renderContext(ctx)
        , m_perfTimer(inPerfTimer)
        , m_inputStreamFactory(inInputStreamFactory)
        , m_shaderCompilationEnabled(true)
    {
    }

    QSharedPointer<QDemonRenderShaderProgram> getProgram(QString inKey, const QVector<QDemonShaderPreprocessorFeature> &inFeatures) override
    {
        m_tempKey.m_key = inKey;
        m_tempKey.m_features = inFeatures;
        m_tempKey.generateHashCode();
        TShaderMap::iterator theIter = m_shaders.find(m_tempKey);
        if (theIter != m_shaders.end())
            return theIter.value();
        return nullptr;
    }

    void addBackwardCompatibilityDefines(ShaderType::Enum shaderType)
    {
        if (shaderType == ShaderType::Vertex || shaderType == ShaderType::TessControl
                || shaderType == ShaderType::TessEval || shaderType == ShaderType::Geometry) {
            m_insertStr += QStringLiteral("#define attribute in\n");
            m_insertStr += QStringLiteral("#define varying out\n");
        } else if (shaderType == ShaderType::Fragment) {
            m_insertStr += QStringLiteral("#define varying in\n");
            m_insertStr += QStringLiteral("#define texture2D texture\n");
            m_insertStr += QStringLiteral("#define gl_FragColor fragOutput\n");

            if (m_renderContext->isAdvancedBlendHwSupportedKHR())
                m_insertStr += QStringLiteral("layout(blend_support_all_equations) out;\n ");
            m_insertStr += QStringLiteral("out vec4 fragOutput;\n");
        }
    }

    void addShaderExtensionStrings(ShaderType::Enum shaderType, bool isGLES)
    {
        if (isGLES) {
            if (m_renderContext->isStandardDerivativesSupported())
                m_insertStr += QStringLiteral("#extension GL_OES_standard_derivatives : enable\n");
            else
                m_insertStr += QStringLiteral("#extension GL_OES_standard_derivatives : disable\n");
        }

        if (QDemonRendererInterface::isGlEs3Context(m_renderContext->getRenderContextType())) {
            if (shaderType == ShaderType::TessControl || shaderType == ShaderType::TessEval) {
                m_insertStr += QStringLiteral("#extension GL_EXT_tessellation_shader : enable\n");
            } else if (shaderType == ShaderType::Geometry) {
                m_insertStr += QStringLiteral("#extension GL_EXT_geometry_shader : enable\n");
            } else if (shaderType == ShaderType::Vertex || shaderType == ShaderType::Fragment) {
                if (m_renderContext->getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::gpuShader5))
                    m_insertStr += QStringLiteral("#extension GL_EXT_gpu_shader5 : enable\n");
                if (m_renderContext->isAdvancedBlendHwSupportedKHR())
                    m_insertStr += QStringLiteral("#extension GL_KHR_blend_equation_advanced : enable\n");
            }
        } else {
            if (shaderType == ShaderType::Vertex || shaderType == ShaderType::Fragment
                    || shaderType == ShaderType::Geometry) {
                if (m_renderContext->getRenderContextType() != QDemonRenderContextValues::GLES2) {
                    m_insertStr += QStringLiteral("#extension GL_ARB_gpu_shader5 : enable\n");
                    m_insertStr += QStringLiteral("#extension GL_ARB_shading_language_420pack : enable\n");
                }
                if (isGLES && m_renderContext->isTextureLodSupported())
                    m_insertStr += QStringLiteral("#extension GL_EXT_shader_texture_lod : enable\n");
                if (m_renderContext->isShaderImageLoadStoreSupported())
                    m_insertStr += QStringLiteral("#extension GL_ARB_shader_image_load_store : enable\n");
                if (m_renderContext->isAtomicCounterBufferSupported())
                    m_insertStr += QStringLiteral("#extension GL_ARB_shader_atomic_counters : enable\n");
                if (m_renderContext->isStorageBufferSupported())
                    m_insertStr += QStringLiteral("#extension GL_ARB_shader_storage_buffer_object : enable\n");
                if (m_renderContext->isAdvancedBlendHwSupportedKHR())
                    m_insertStr += QStringLiteral("#extension GL_KHR_blend_equation_advanced : enable\n");
            }
        }
    }

    void addShaderPreprocessor(QString &str, QString inKey,
                               ShaderType::Enum shaderType,
                               const QVector<QDemonShaderPreprocessorFeature> &inFeatures)
    {
        // Don't use shading language version returned by the driver as it might
        // differ from the context version. Instead use the context type to specify
        // the version string.
        bool isGlES = QDemonRendererInterface::isGlEsContext(m_renderContext->getRenderContextType());
        m_insertStr.clear();
        int minor = m_renderContext->format().minorVersion();
        QString versionStr;
        QTextStream stream(&versionStr);
        stream << "#version ";
        const quint32 type = quint32(m_renderContext->getRenderContextType());
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

        m_insertStr.append(versionStr);

        if (isGlES) {
            if (!QDemonRendererInterface::isGlEs3Context(m_renderContext->getRenderContextType())) {
                if (shaderType == ShaderType::Fragment) {
                    m_insertStr += QStringLiteral("#define fragOutput gl_FragData[0]\n");
                }
            } else {
                m_insertStr += QStringLiteral("#define texture2D texture\n");
            }

            // add extenions strings before any other non-processor token
            addShaderExtensionStrings(shaderType, isGlES);

            // add precision qualifier depending on backend
            if (QDemonRendererInterface::isGlEs3Context(m_renderContext->getRenderContextType())) {
                m_insertStr.append(QStringLiteral("precision highp float;\n"
                                                  "precision highp int;\n"));
                if( m_renderContext->getRenderBackendCap(QDemonRenderBackend::QDemonRenderBackendCaps::gpuShader5) ) {
                    m_insertStr.append(QStringLiteral("precision mediump sampler2D;\n"
                                                      "precision mediump sampler2DArray;\n"
                                                      "precision mediump sampler2DShadow;\n"));
                    if (m_renderContext->isShaderImageLoadStoreSupported()) {
                        m_insertStr.append(QStringLiteral("precision mediump image2D;\n"));
                    }
                }

                addBackwardCompatibilityDefines(shaderType);
            } else {
                // GLES2
                m_insertStr.append(QStringLiteral("precision mediump float;\n"
                                                  "precision mediump int;\n"
                                                  "#define texture texture2D\n"));
                if (m_renderContext->isTextureLodSupported())
                    m_insertStr.append(QStringLiteral("#define textureLod texture2DLodEXT\n"));
                else
                    m_insertStr.append(QStringLiteral("#define textureLod(s, co, lod) texture2D(s, co)\n"));
            }
        } else {
            if (!QDemonRendererInterface::isGl2Context(m_renderContext->getRenderContextType())) {
                m_insertStr += QStringLiteral("#define texture2D texture\n");

                addShaderExtensionStrings(shaderType, isGlES);

                m_insertStr += QStringLiteral("#if __VERSION__ >= 330\n");

                addBackwardCompatibilityDefines(shaderType);

                m_insertStr += QStringLiteral("#else\n");
                if (shaderType == ShaderType::Fragment) {
                    m_insertStr += QStringLiteral("#define fragOutput gl_FragData[0]\n");
                }
                m_insertStr += QStringLiteral("#endif\n");
            }
        }

        if (!inKey.isNull()) {
            m_insertStr += QStringLiteral("//Shader name -");
            m_insertStr += inKey;
            m_insertStr += QStringLiteral("\n");
        }

        if (shaderType == ShaderType::TessControl) {
            m_insertStr += QStringLiteral("#define TESSELLATION_CONTROL_SHADER 1\n");
            m_insertStr += QStringLiteral("#define TESSELLATION_EVALUATION_SHADER 0\n");
        } else if (shaderType == ShaderType::TessEval) {
            m_insertStr += QStringLiteral("#define TESSELLATION_CONTROL_SHADER 0\n");
            m_insertStr += QStringLiteral("#define TESSELLATION_EVALUATION_SHADER 1\n");
        }

        str.insert(0, m_insertStr);
        if (inFeatures.size()) {
            QString::size_type insertPos = int(m_insertStr.size());
            m_insertStr.clear();
            for (int idx = 0, end = inFeatures.size(); idx < end; ++idx) {
                QDemonShaderPreprocessorFeature feature(inFeatures[idx]);
                m_insertStr.append(QStringLiteral("#define "));
                m_insertStr.append(inFeatures[idx].name);
                m_insertStr.append(QStringLiteral(" "));
                m_insertStr.append(feature.enabled ? QStringLiteral("1") : QStringLiteral("0"));
                m_insertStr.append(QStringLiteral("\n"));
            }
            str.insert(insertPos, m_insertStr);
        }
    }
    // Compile this program overwriting any existing ones.
    QSharedPointer<QDemonRenderShaderProgram> forceCompileProgram(QString inKey, const QString &inVert, const QString &inFrag,
                                                                  const QString &inTessCtrl, const QString &inTessEval, const QString &inGeom,
                                                                  const QDemonShaderCacheProgramFlags &inFlags,
                                                                  const QVector<QDemonShaderPreprocessorFeature> &inFeatures,
                                                                  bool separableProgram, bool fromDisk = false) override
    {
        if (m_shaderCompilationEnabled == false)
            return nullptr;
        QDemonShaderCacheKey tempKey(inKey);
        tempKey.m_features = inFeatures;
        tempKey.generateHashCode();

        if (fromDisk) {
            qCInfo(TRACE_INFO) << "Loading from persistent shader cache: '<"
                               << tempKey.m_key << ">'";
        } else {
            qCInfo(TRACE_INFO) << "Compiling into shader cache: '"
                               << tempKey.m_key << ">'";
        }

        //SStackPerfTimer __perfTimer(m_PerfTimer, "Shader Compilation");
        m_vertexCode = inVert;
        m_tessCtrlCode = inTessCtrl;
        m_tessEvalCode = inTessEval;
        m_geometryCode = inGeom;
        m_fragmentCode = inFrag;
        // Add defines and such so we can write unified shaders that work across platforms.
        // vertex and fragment shaders are optional for separable shaders
        if (!separableProgram || !m_vertexCode.isEmpty())
            addShaderPreprocessor(m_vertexCode, inKey, ShaderType::Vertex, inFeatures);
        if (!separableProgram || !m_fragmentCode.isEmpty())
            addShaderPreprocessor(m_fragmentCode, inKey, ShaderType::Fragment, inFeatures);
        // optional shaders
        if (inFlags.isTessellationEnabled()) {
            Q_ASSERT(m_tessCtrlCode.size() && m_tessEvalCode.size());
            addShaderPreprocessor(m_tessCtrlCode, inKey, ShaderType::TessControl, inFeatures);
            addShaderPreprocessor(m_tessEvalCode, inKey, ShaderType::TessEval, inFeatures);
        }
        if (inFlags.isGeometryShaderEnabled())
            addShaderPreprocessor(m_geometryCode, inKey, ShaderType::Geometry, inFeatures);
        // ### this call is awful because of QString conversions!
        auto shaderProgram = m_renderContext->compileSource(inKey.toLocal8Bit().constData(),
                                                            m_vertexCode.toLocal8Bit().constData(),
                                                            quint32(m_vertexCode.toLocal8Bit().size()),
                                                            m_fragmentCode.toLocal8Bit().constData(),
                                                            quint32(m_fragmentCode.toLocal8Bit().size()),
                                                            m_tessCtrlCode.toLocal8Bit().constData(),
                                                            quint32(m_tessCtrlCode.toLocal8Bit().size()),
                                                            m_tessEvalCode.toLocal8Bit().constData(),
                                                            quint32(m_tessEvalCode.toLocal8Bit().size()),
                                                            m_geometryCode.toLocal8Bit().constData(),
                                                            quint32(m_geometryCode.toLocal8Bit().size()),
                                                            separableProgram).m_shader;
        m_shaders.insert(tempKey, shaderProgram);
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

    virtual QSharedPointer<QDemonRenderShaderProgram> compileProgram(QString inKey, const QString &inVert, const QString &inFrag,
                                                                     const QString &inTessCtrl, const QString &inTessEval, const QString &inGeom,
                                                                     const QDemonShaderCacheProgramFlags &inFlags,
                                                                     const QVector<QDemonShaderPreprocessorFeature> &inFeatures, bool separableProgram) override
    {
        QSharedPointer<QDemonRenderShaderProgram> theProgram = getProgram(inKey, inFeatures);
        if (theProgram)
            return theProgram;

        QSharedPointer<QDemonRenderShaderProgram> retval =
                forceCompileProgram(inKey, inVert, inFrag, inTessCtrl, inTessEval, inGeom, inFlags,
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

    void setShaderCachePersistenceEnabled(const QString &inDirectory) override
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

    bool isShaderCachePersistenceEnabled() const override
    {
        // ### Shader Chache Writing Code is disabled
        //return m_ShaderCache != nullptr;
        return false;
    }

    void setShaderCompilationEnabled(bool inEnableShaderCompilation) override
    {
        m_shaderCompilationEnabled = inEnableShaderCompilation;
    }
};
}

uint hashShaderFeatureSet(QVector<QDemonShaderPreprocessorFeature> inFeatureSet)
{
    uint retval(0);
    for (int idx = 0, end = inFeatureSet.size(); idx < end; ++idx) {
        // From previous implementation, it seems we need to ignore the order of the features.
        // But we need to bind the feature flag together with its name, so that the flags will
        // influence
        // the final hash not only by the true-value count.
        retval = retval
                ^ (qHash(inFeatureSet[idx].name) * qHash(inFeatureSet[idx].enabled));
    }
    return retval;
}

bool QDemonShaderPreprocessorFeature::operator<(const QDemonShaderPreprocessorFeature &other) const
{
    return QString::compare(name, other.name) < 0;
}

bool QDemonShaderPreprocessorFeature::operator==(const QDemonShaderPreprocessorFeature &other) const
{
    return name == other.name && enabled == other.enabled;
}

QDemonShaderCacheInterface::~QDemonShaderCacheInterface()
{

}

QSharedPointer<QDemonShaderCacheInterface> QDemonShaderCacheInterface::createShaderCache(QSharedPointer<QDemonRenderContext> inContext,
                                                                                         QSharedPointer<QDemonInputStreamFactoryInterface> inInputStreamFactory,
                                                                                         QSharedPointer<QDemonPerfTimerInterface> inPerfTimer)
{
    return QSharedPointer<ShaderCache>(new ShaderCache(inContext, inInputStreamFactory, inPerfTimer));
}

QT_END_NAMESPACE
