/****************************************************************************
**
** Copyright (C) 2008-2012 NVIDIA Corporation.
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Quick 3D.
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

#include "qdemonrenderdynamicobjectsystem.h"

#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRuntimeRender/qdemonrendershadercache.h>
#include <QtDemonRuntimeRender/qdemonrenderinputstreamfactory.h>
#include <QtDemonRuntimeRender/qdemonrenderer.h>
#include <QtDemonRuntimeRender/qdemonrenderdynamicobjectsystemcommands.h>
#include <QtDemonRuntimeRender/qdemonrenderdynamicobjectsystemutil.h>
#include <QtDemonRuntimeRender/qdemonrendershadercodegenerator.h>

#include <QtDemonRender/qdemonrendershaderconstant.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>

#include <QtDemon/qdemonutils.h>

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

QT_BEGIN_NAMESPACE

uint qHash(const TStrStrPair &item)
{
    return qHash(item.first) ^ qHash(item.second);
}

namespace dynamic {

uint qHash(const QDemonDynamicShaderMapKey &inKey)
{
    return inKey.m_hashCode;
}

//quint32 QDemonCommand::getSizeofCommand(const QDemonCommand &inCommand)
//{
//    switch (inCommand.m_type) {
//    case CommandType::AllocateBuffer:
//        return sizeof(QDemonAllocateBuffer);
//    case CommandType::BindBuffer:
//        return sizeof(QDemonBindBuffer);
//    case CommandType::BindTarget:
//        return sizeof(QDemonBindTarget);
//    case CommandType::BindShader:
//        return sizeof(QDemonBindShader);
//    case CommandType::Render:
//        return sizeof(QDemonRender);
//    case CommandType::ApplyBufferValue:
//        return sizeof(QDemonApplyBufferValue);
//    case CommandType::ApplyDepthValue:
//        return sizeof(QDemonApplyDepthValue);
//    case CommandType::ApplyInstanceValue:
//        return sizeof(QDemonApplyInstanceValue);
//    case CommandType::ApplyBlending:
//        return sizeof(QDemonApplyBlending);
//    case CommandType::ApplyRenderState:
//        return sizeof(QDemonApplyRenderState);
//    case CommandType::ApplyBlitFramebuffer:
//        return sizeof(QDemonApplyBlitFramebuffer);
//    case CommandType::ApplyValue:
//        return sizeof(QDemonApplyValue) + static_cast<const QDemonApplyValue &>(inCommand).m_value.mSize;
//    case CommandType::DepthStencil:
//        return sizeof(QDemonDepthStencil);
//    case CommandType::AllocateImage:
//        return sizeof(QDemonAllocateImage);
//    case CommandType::ApplyImageValue:
//        return sizeof(QDemonApplyImageValue);
//    case CommandType::AllocateDataBuffer:
//        return sizeof(QDemonAllocateDataBuffer);
//    case CommandType::ApplyDataBufferValue:
//        return sizeof(QDemonApplyDataBufferValue);
//    default:
//        break;
//    }
//    Q_ASSERT(false);
//    return 0;
//}

//template<typename TCommandType>
//inline void CopyConstructCommandT(quint8 *inDataBuffer, const QDemonCommand &inCommand)
//{
//    TCommandType *theCommand = (TCommandType *)inDataBuffer;
//    theCommand = new (theCommand) TCommandType(static_cast<const TCommandType &>(inCommand));
//}

//void QDemonCommand::copyConstructCommand(quint8 *inDataBuffer, const QDemonCommand &inCommand)
//{
//    switch (inCommand.m_type) {
//    case CommandType::AllocateBuffer:
//        CopyConstructCommandT<QDemonAllocateBuffer>(inDataBuffer, inCommand);
//        break;
//    case CommandType::BindBuffer:
//        CopyConstructCommandT<QDemonBindBuffer>(inDataBuffer, inCommand);
//        break;
//    case CommandType::BindTarget:
//        CopyConstructCommandT<QDemonBindTarget>(inDataBuffer, inCommand);
//        break;
//    case CommandType::BindShader:
//        CopyConstructCommandT<QDemonBindShader>(inDataBuffer, inCommand);
//        break;
//    case CommandType::Render:
//        CopyConstructCommandT<QDemonRender>(inDataBuffer, inCommand);
//        break;
//    case CommandType::ApplyBufferValue:
//        CopyConstructCommandT<QDemonApplyBufferValue>(inDataBuffer, inCommand);
//        break;
//    case CommandType::ApplyDepthValue:
//        CopyConstructCommandT<QDemonApplyDepthValue>(inDataBuffer, inCommand);
//        break;
//    case CommandType::ApplyInstanceValue:
//        CopyConstructCommandT<QDemonApplyInstanceValue>(inDataBuffer, inCommand);
//        break;
//    case CommandType::ApplyBlending:
//        CopyConstructCommandT<QDemonApplyBlending>(inDataBuffer, inCommand);
//        break;
//    case CommandType::ApplyRenderState:
//        CopyConstructCommandT<QDemonApplyRenderState>(inDataBuffer, inCommand);
//        break;
//    case CommandType::ApplyBlitFramebuffer:
//        CopyConstructCommandT<QDemonApplyBlitFramebuffer>(inDataBuffer, inCommand);
//        break;
//    case CommandType::ApplyValue: {
//        CopyConstructCommandT<QDemonApplyValue>(inDataBuffer, inCommand);
//        QDemonApplyValue &dest = *reinterpret_cast<QDemonApplyValue *>(inDataBuffer);
//        quint8 *destMem = inDataBuffer + sizeof(QDemonApplyValue);
//        const QDemonApplyValue &src = static_cast<const QDemonApplyValue &>(inCommand);
//        memcpy(destMem, src.m_value.mData, src.m_value.mSize);
//        dest.m_value.mData = destMem;
//        break;
//    }
//    case CommandType::DepthStencil:
//        CopyConstructCommandT<QDemonDepthStencil>(inDataBuffer, inCommand);
//        break;
//    case CommandType::AllocateImage:
//        CopyConstructCommandT<QDemonAllocateImage>(inDataBuffer, inCommand);
//        break;
//    case CommandType::ApplyImageValue:
//        CopyConstructCommandT<QDemonApplyImageValue>(inDataBuffer, inCommand);
//        break;
//    case CommandType::AllocateDataBuffer:
//        CopyConstructCommandT<QDemonAllocateDataBuffer>(inDataBuffer, inCommand);
//        break;
//    case CommandType::ApplyDataBufferValue:
//        CopyConstructCommandT<QDemonApplyDataBufferValue>(inDataBuffer, inCommand);
//        break;
//    default:
//        Q_ASSERT(false);
//        break;
//    }
//}
}

QString QDemonDynamicObjectSystem::getShaderCodeLibraryDirectory()
{
    return QStringLiteral("res/effectlib");
}
static QByteArray includeSearch() { return QByteArrayLiteral("#include \""); };
static QByteArray copyrightHeaderStart() { return QByteArrayLiteral("/****************************************************************************"); }
static QByteArray copyrightHeaderEnd() { return QByteArrayLiteral("****************************************************************************/"); }


QDemonDynamicObjectSystem::QDemonDynamicObjectSystem(QDemonRenderContextInterface *ctx)
    : m_context(ctx), m_propertyLoadMutex()
{
}

QDemonDynamicObjectSystem::~QDemonDynamicObjectSystem() {}

void QDemonDynamicObjectSystem::setShaderData(const QByteArray &inPath,
                                              const QByteArray &inData,
                                              const QByteArray &inShaderType,
                                              const QByteArray &inShaderVersion,
                                              bool inHasGeomShader,
                                              bool inIsComputeShader)
{
    //        inData = inData ? inData : "";
    auto foundIt = m_expandedFiles.find(inPath);
    if (foundIt != m_expandedFiles.end())
        foundIt.value() = inData;
    else
        m_expandedFiles.insert(inPath, inData);

    // set shader type and version if available
    if (!inShaderType.isNull() || !inShaderVersion.isNull() || inHasGeomShader || inIsComputeShader) {
        // UdoL TODO: Add this to the load / save setction
        // In addition we should merge the source code into SDynamicObjectShaderInfo as well
        QDemonDynamicObjectShaderInfo &theShaderInfo = m_shaderInfoMap.insert(inPath, QDemonDynamicObjectShaderInfo()).value();
        theShaderInfo.m_type = inShaderType;
        theShaderInfo.m_version = inShaderVersion;
        theShaderInfo.m_hasGeomShader = inHasGeomShader;
        theShaderInfo.m_isComputeShader = inIsComputeShader;
    }
}

QByteArray QDemonDynamicObjectSystem::getShaderCacheKey(const QByteArray &inId,
                                                        const QByteArray &inProgramMacro,
                                                        const dynamic::QDemonDynamicShaderProgramFlags &inFlags)
{
    QByteArray shaderKey = inId;
    if (!inProgramMacro.isEmpty()) {
        shaderKey.append("#");
        shaderKey.append(inProgramMacro);
    }
    if (inFlags & ShaderCacheProgramFlagValues::TessellationEnabled) {
        shaderKey.append("#");
        shaderKey.append(toString(inFlags.tessMode));
    }
    if (inFlags & ShaderCacheProgramFlagValues::GeometryShaderEnabled && inFlags.wireframeMode) {
        shaderKey.append("#");
        shaderKey.append(inFlags.wireframeToString(inFlags.wireframeMode));
    }
    return shaderKey;
}

void QDemonDynamicObjectSystem::insertShaderHeaderInformation(QByteArray &theReadBuffer, const QByteArray &inPathToEffect)
{
    doInsertShaderHeaderInformation(theReadBuffer, inPathToEffect);
}

void QDemonDynamicObjectSystem::doInsertShaderHeaderInformation(QByteArray &theReadBuffer, const QByteArray &inPathToEffect)
{
    // Now do search and replace for the headers
    for (int thePos = theReadBuffer.indexOf(includeSearch()); thePos != -1;
         thePos = theReadBuffer.indexOf(includeSearch(), thePos + 1)) {
        int theEndQuote = theReadBuffer.indexOf('\"', thePos + includeSearch().length() + 1);
        // Indicates an unterminated include file.
        if (theEndQuote == -1) {
            qCCritical(INVALID_OPERATION, "Unterminated include in file: %s", inPathToEffect.constData());
            theReadBuffer.clear();
            break;
        }
        const int theActualBegin = thePos + includeSearch().length();
        const auto &theInclude = theReadBuffer.mid(theActualBegin, theEndQuote - theActualBegin);
        // If we haven't included the file yet this round
        auto theHeader = doLoadShader(theInclude);
        // Strip copywrite headers from include if present
        if (theHeader.startsWith(copyrightHeaderStart())) {
            int clipPos = theHeader.indexOf(copyrightHeaderEnd()) ;
            if (clipPos >= 0)
                theHeader.remove(0, clipPos + copyrightHeaderEnd().count());
        }
        // Write insert comment for begin source
        theHeader.prepend(QByteArrayLiteral("\n// begin \"") + theInclude + QByteArrayLiteral("\"\n"));
        // Write insert comment for end source
        theHeader.append(QByteArrayLiteral("\n// end \"" ) + theInclude + QByteArrayLiteral("\"\n"));
        theReadBuffer = theReadBuffer.replace(thePos, (theEndQuote + 1) - thePos, theHeader);
    }
}

QByteArray QDemonDynamicObjectSystem::doLoadShader(const QByteArray &inPathToEffect)
{
    auto theInsert = m_expandedFiles.find(inPathToEffect);
    const bool found = (theInsert != m_expandedFiles.end());

    QByteArray theReadBuffer;
    if (!found) {
        const QString defaultDir = getShaderCodeLibraryDirectory();
        const QString platformDir = shaderCodeLibraryPlatformDirectory();
        const auto ver = shaderCodeLibraryVersion();

        QString fullPath;
        QSharedPointer<QIODevice> theStream;
        if (!platformDir.isEmpty()) {
            QTextStream stream(&fullPath);
            stream << platformDir << QLatin1Char('/') << QString::fromLocal8Bit(inPathToEffect);
            theStream = m_context->inputStreamFactory()->getStreamForFile(fullPath, true);
        }

        if (theStream.isNull()) {
            fullPath.clear();
            QTextStream stream(&fullPath);
            stream << defaultDir << QLatin1Char('/') << ver << QLatin1Char('/') << QString::fromLocal8Bit(inPathToEffect);
            theStream = m_context->inputStreamFactory()->getStreamForFile(fullPath, true);
            if (theStream.isNull()) {
                fullPath.clear();
                QTextStream stream(&fullPath);
                stream << defaultDir << QLatin1Char('/') << QString::fromLocal8Bit(inPathToEffect);
                theStream = m_context->inputStreamFactory()->getStreamForFile(fullPath, false);
            }
        }
        if (!theStream.isNull()) {
            char readBuf[1024];
            qint64 amountRead = 0;
            do {
                amountRead = theStream->read(readBuf, 1024);
                if (amountRead)
                    theReadBuffer.append(readBuf, int(amountRead));
            } while (amountRead);
        } else {
            qCCritical(INVALID_OPERATION, "Failed to find include file %s", qPrintable(QString::fromLocal8Bit(inPathToEffect)));
            Q_ASSERT(false);
        }
        theInsert = m_expandedFiles.insert(inPathToEffect, theReadBuffer);
    } else {
        theReadBuffer = theInsert.value();
    }
    doInsertShaderHeaderInformation(theReadBuffer, inPathToEffect);
    return theReadBuffer;
}

QStringList QDemonDynamicObjectSystem::getParameters(const QString &str, int begin, int end)
{
    const QString s = str.mid(begin, end - begin + 1);
    return s.split(',');
}

void QDemonDynamicObjectSystem::insertSnapperDirectives(QString &str)
{
    int beginIndex = 0;
    // Snapper macros:
    //  #define SNAPPER_SAMPLER2D(propName, propNiceName, texFilter, texWrap, showUI )
    //      uniform sampler2D propName;
    //      uniform int flag##propName;
    //      uniform vec4 propName##Info;
    //      vec4 texture2D_##propName(vec2 uv)
    //      {
    //          return GetTextureValue( propName, uv, propName##Info.z );
    //      }
    //
    //  #define SNAPPER_SAMPLER2DWITHDEFAULT(propName, propNiceName, texFilter, texWrap, defaultPath, showUI )
    //      SNAPPER_SAMPLER2D( propName, propNiceName, texFilter, texWrap, showUI )
    //
    //  #define SNAPPER_SAMPLERCUBE(propName, propNiceName, texFilter, texWrap )
    //      uniform samplerCube propName;
    //      uniform vec2 propName##UVRange;
    //      uniform int flag##propName;
    //      uniform vec2 propName##Size;
    QString snapperSampler = QStringLiteral("SNAPPER_SAMPLER2D(");
    QString snapperSamplerDefault = QStringLiteral("SNAPPER_SAMPLER2DWITHDEFAULT(");
    QString snapperSamplerCube = QStringLiteral("SNAPPER_SAMPLERCUBE(");
    QString endingBracket = QStringLiteral(")");

    while ((beginIndex = str.indexOf(snapperSampler, beginIndex)) >= 0) {
        int endIndex = str.indexOf(endingBracket, beginIndex);
        const QStringList list = getParameters(str, beginIndex + snapperSampler.length(), endIndex);
        str.remove(beginIndex, endIndex - beginIndex + 1);
        if (list.size() == 5) {
            QString insertStr;
            QTextStream stream(&insertStr);
            stream << "uniform sampler2D " << list[0] << ";\n";
            stream << "uniform int flag" << list[0] << ";\n";
            stream << "vec4 " << list[0] << "Info;\n";
            stream << "vec4 texture2D_" << list[0] << "(vec2 uv) "
                   << "{ return GetTextureValue( " << list[0] << ", uv, " << list[0] << "Info.z ); }\n";
            str.insert(beginIndex, insertStr);
        }
    }
    beginIndex = 0;
    while ((beginIndex = str.indexOf(snapperSamplerDefault, beginIndex)) >= 0) {
        int endIndex = str.indexOf(endingBracket, beginIndex);
        const QStringList list = getParameters(str, beginIndex + snapperSamplerDefault.length(), endIndex);
        str.remove(beginIndex, endIndex - beginIndex + 1);
        if (list.size() == 5) {
            QString insertStr;
            QTextStream stream(&insertStr);
            stream << "uniform sampler2D " << list[0] << ";\n";
            stream << "uniform int flag" << list[0] << ";\n";
            stream << "vec4 " << list[0] << "Info;\n";
            stream << "vec4 texture2D_" << list[0] << "(vec2 uv) "
                   << "{ return GetTextureValue( " << list[0] << ", uv, " << list[0] << "Info.z ); }\n";
            str.insert(beginIndex, insertStr);
        }
    }
    beginIndex = 0;
    while ((beginIndex = str.indexOf(snapperSamplerCube, beginIndex)) >= 0) {
        int endIndex = str.indexOf(endingBracket, beginIndex);
        const QStringList list = getParameters(str, beginIndex + snapperSamplerCube.length(), endIndex);
        str.remove(beginIndex, endIndex - beginIndex + 1);
        if (list.size() == 4) {
            QString insertStr;
            QTextStream stream(&insertStr);
            stream << "uniform samplerCube " << list[0] << ";\n";
            stream << "uniform vec2 " << list[0] << "UVRange;\n";
            stream << "uniform int flag" << list[0] << ";\n";
            stream << "uniform vec2 " << list[0] << "Size;\n";
            str.insert(beginIndex, insertStr);
        }
    }
}

QDemonRef<QDemonRenderShaderProgram> QDemonDynamicObjectSystem::compileShader(const QByteArray &inId,
                                                                              const QByteArray &inProgramSource,
                                                                              const QByteArray &inGeomSource,
                                                                              const QByteArray &inProgramMacroName,
                                                                              const TShaderFeatureSet &inFeatureSet,
                                                                              const dynamic::QDemonDynamicShaderProgramFlags &inFlags,
                                                                              bool inForceCompilation)
{
    m_vertShader.clear();
    m_fragShader.clear();
    m_geometryShader.clear();
    QDemonShaderCacheProgramFlags theFlags;

    m_vertShader.append("#define VERTEX_SHADER\n");
    m_fragShader.append("#define FRAGMENT_SHADER\n");

    if (!inProgramMacroName.isEmpty()) {
        m_vertShader.append("#define ");
        m_vertShader.append(inProgramMacroName);
        m_vertShader.append("\n");

        m_fragShader.append("#define ");
        m_fragShader.append(inProgramMacroName);
        m_fragShader.append("\n");
    }

    if (!inGeomSource.isEmpty() && inFlags & ShaderCacheProgramFlagValues::GeometryShaderEnabled) {
        theFlags |= ShaderCacheProgramFlagValues::GeometryShaderEnabled;

        m_geometryShader.append("#define GEOMETRY_SHADER 1\n");
        m_geometryShader.append(inGeomSource);

        m_vertShader.append("#define GEOMETRY_SHADER 1\n");
    } else if (inFlags & ShaderCacheProgramFlagValues::GeometryShaderEnabled) {
        theFlags |= ShaderCacheProgramFlagValues::GeometryShaderEnabled;
        m_geometryShader.append("#define USER_GEOMETRY_SHADER 1\n");
        m_geometryShader.append(inProgramSource);
        m_vertShader.append("#define GEOMETRY_SHADER 0\n");
        m_fragShader.append("#define GEOMETRY_WIREFRAME_SHADER 0\n");
    } else {
        m_vertShader.append("#define GEOMETRY_SHADER 0\n");
        m_fragShader.append("#define GEOMETRY_WIREFRAME_SHADER 0\n");
    }

    if (strstr(inProgramSource, "SNAPPER_SAMPLER")) {
        QString programSource = QString::fromLatin1(inProgramSource);
        insertSnapperDirectives(programSource);
        QByteArray data = programSource.toLatin1();
        const char *source = data.constData();

        m_vertShader.append(source);
        m_fragShader.append(source);
    } else {
        m_vertShader.append(inProgramSource);
        m_fragShader.append(inProgramSource);
    }

    QDemonRef<QDemonShaderCache> theShaderCache = m_context->shaderCache();

    QByteArray theKey = getShaderCacheKey(inId, inProgramMacroName, inFlags);
    if (inForceCompilation) {
        return theShaderCache->forceCompileProgram(theKey, m_vertShader, m_fragShader, nullptr, nullptr, m_geometryShader, theFlags, inFeatureSet, false);
    }

    return theShaderCache->compileProgram(theKey, m_vertShader, m_fragShader, nullptr, nullptr, m_geometryShader, theFlags, inFeatureSet);
}

QByteArray QDemonDynamicObjectSystem::getShaderSource(const QByteArray &inPath)
{
//    QByteArray source(QByteArrayLiteral("#define FRAGMENT_SHADER\n"));
//    source.append(doLoadShader(inPath));
    return doLoadShader(inPath);
}

TShaderAndFlags QDemonDynamicObjectSystem::getShaderProgram(const QByteArray &inPath,
                                                            const QByteArray &inProgramMacro,
                                                            const TShaderFeatureSet &inFeatureSet,
                                                            const dynamic::QDemonDynamicShaderProgramFlags &inFlags,
                                                            bool inForceCompilation)
{
    dynamic::QDemonDynamicShaderMapKey shaderMapKey(TStrStrPair(inPath, inProgramMacro), inFeatureSet, inFlags.tessMode, inFlags.wireframeMode);
    auto theInserter = m_shaderMap.find(shaderMapKey);
    const bool found = (theInserter != m_shaderMap.end());

    if (!found)
        theInserter = m_shaderMap.insert(shaderMapKey, TShaderAndFlags());

    // TODO: This looks funky (if found)...
    if (found || inForceCompilation) {
        QDemonRef<QDemonRenderShaderProgram> theProgram = m_context->shaderCache()
                                                                  ->getProgram(getShaderCacheKey(inPath,
                                                                                                 inProgramMacro,
                                                                                                 inFlags),
                                                                               inFeatureSet);
        dynamic::QDemonDynamicShaderProgramFlags theFlags(inFlags);
        if (!theProgram || inForceCompilation) {
            QDemonDynamicObjectShaderInfo
                    &theShaderInfo = m_shaderInfoMap.insert(inPath, QDemonDynamicObjectShaderInfo()).value();
            if (theShaderInfo.m_isComputeShader == false) {
                QByteArray programSource = doLoadShader(inPath);
                if (theShaderInfo.m_hasGeomShader)
                    theFlags |= ShaderCacheProgramFlagValues::GeometryShaderEnabled;
                theProgram = compileShader(inPath, programSource.constData(), nullptr, inProgramMacro, inFeatureSet, theFlags, inForceCompilation);
            } else {
                QByteArray theShaderBuffer;
                const char *shaderVersionStr = "#version 430\n";
                if (m_context->renderContext()->renderContextType() == QDemonRenderContextType::GLES3PLUS)
                    shaderVersionStr = "#version 310 es\n";
                theShaderBuffer = doLoadShader(inPath);
                theShaderBuffer.insert(0, shaderVersionStr);
                theProgram = m_context->renderContext()->compileComputeSource(inPath, toByteView(theShaderBuffer)).m_shader;
            }
        }
        theInserter.value() = TShaderAndFlags(theProgram, theFlags);
    }
    return theInserter.value();
}

TShaderAndFlags QDemonDynamicObjectSystem::getDepthPrepassShader(const QByteArray &inPath, const QByteArray &inPMacro, const TShaderFeatureSet &inFeatureSet)
{
    QDemonDynamicObjectShaderInfo &theShaderInfo = m_shaderInfoMap.insert(inPath, QDemonDynamicObjectShaderInfo()).value();
    if (theShaderInfo.m_hasGeomShader == false)
        return TShaderAndFlags();
    // else, here we go...
    dynamic::QDemonDynamicShaderProgramFlags theFlags;
    const QByteArray shaderKey = inPMacro + QByteArrayLiteral("depthprepass");

    const QByteArray &theProgramMacro = shaderKey;

    const dynamic::QDemonDynamicShaderMapKey shaderMapKey(TStrStrPair(inPath, theProgramMacro),
                                                          inFeatureSet,
                                                          theFlags.tessMode,
                                                          theFlags.wireframeMode);
    const TShaderAndFlags shaderFlags;
    auto theInserter = m_shaderMap.find(shaderMapKey);
    const bool found = theInserter != m_shaderMap.end();
    if (found) {
        QDemonRef<QDemonRenderShaderProgram> theProgram = m_context->shaderCache()
                                                                  ->getProgram(getShaderCacheKey(inPath,
                                                                                                 theProgramMacro,
                                                                                                 theFlags),
                                                                               inFeatureSet);
        dynamic::QDemonDynamicShaderProgramFlags flags(theFlags);
        if (!theProgram) {
            QByteArray geomSource = doLoadShader(inPath);
            QDemonShaderVertexCodeGenerator vertexShader(m_context->renderContext()->renderContextType());
            QDemonShaderFragmentCodeGenerator fragmentShader(vertexShader, m_context->renderContext()->renderContextType());

            vertexShader.addAttribute("attr_pos", "vec3");
            vertexShader.addUniform("model_view_projection", "mat4");
            vertexShader.append("void main() {");
            vertexShader.append("\tgl_Position = model_view_projection * vec4(attr_pos, 1.0);");
            vertexShader.append("}");
            fragmentShader.append("void main() {");
            fragmentShader.append("\tfragOutput = vec4(0.0, 0.0, 0.0, 0.0);");
            fragmentShader.append("}");
            QByteArray vertexSource = vertexShader.buildShaderSource();
            QByteArray fragmentSource = fragmentShader.buildShaderSource();

            QByteArray programBuffer(QByteArrayLiteral("#ifdef VERTEX_SHADER\n"));
            programBuffer.append(vertexSource);
            programBuffer.append("\n#endif\n");
            programBuffer.append("\n#ifdef FRAGMENT_SHADER\n");
            programBuffer.append(fragmentSource);
            programBuffer.append("\n#endif");
            flags |= ShaderCacheProgramFlagValues::GeometryShaderEnabled;
            theProgram = compileShader(inPath, programBuffer, geomSource, theProgramMacro, inFeatureSet, flags);
        }
        theInserter.value() = TShaderAndFlags(theProgram, flags);
    }
    return theInserter.value();
}

void QDemonDynamicObjectSystem::setShaderCodeLibraryVersion(const QByteArray &version)
{
    m_shaderLibraryVersion = version;
}

QByteArray QDemonDynamicObjectSystem::shaderCodeLibraryVersion()
{
    return m_shaderLibraryVersion;
}

void QDemonDynamicObjectSystem::setShaderCodeLibraryPlatformDirectory(const QString &directory)
{
    m_shaderLibraryPlatformDirectory = directory;
}

QString QDemonDynamicObjectSystem::shaderCodeLibraryPlatformDirectory()
{
    return m_shaderLibraryPlatformDirectory;
}

QT_END_NAMESPACE
