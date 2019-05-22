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
#ifndef QDEMON_RENDER_DYNAMIC_OBJECT_SYSTEM_H
#define QDEMON_RENDER_DYNAMIC_OBJECT_SYSTEM_H

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>

#include <QtDemonRuntimeRender/qdemonrendershadercache.h>
#include <QtDemonRuntimeRender/qdemonrendertessmodevalues.h>
#include <QtDemonRuntimeRender/qdemonrendergraphobject.h>

#include <QtGui/QVector2D>

#include <QtCore/QString>
#include <QtCore/qmutex.h>

QT_BEGIN_NAMESPACE
struct QDemonRenderDynamicGraphObject;
// struct SWriteBuffer;
// struct SStrRemapMap;
class QDemonRenderContextInterface;
struct QDemonDynamicObjectClass;

typedef QPair<QByteArray, QByteArray> TStrStrPair;

namespace dynamic {

struct QDemonDynamicShaderMapKey
{
    TStrStrPair m_name;
    QVector<QDemonShaderPreprocessorFeature> m_features;
    TessModeValues m_tessMode;
    bool m_wireframeMode;
    uint m_hashCode;
    QDemonDynamicShaderMapKey(TStrStrPair inName, TShaderFeatureSet inFeatures, TessModeValues inTessMode, bool inWireframeMode)
        : m_name(inName), m_tessMode(inTessMode), m_wireframeMode(inWireframeMode)
    {
        for (int i = 0; i < inFeatures.size(); ++i) {
            m_features.append(inFeatures[i]);
        }

        m_hashCode = qHash(m_name) ^ hashShaderFeatureSet(m_features) ^ qHash(m_tessMode) ^ qHash(m_wireframeMode);
    }
    bool operator==(const QDemonDynamicShaderMapKey &inKey) const
    {
        return m_name == inKey.m_name && m_features == inKey.m_features && m_tessMode == inKey.m_tessMode
                && m_wireframeMode == inKey.m_wireframeMode;
    }
};

struct QDemonCommand;

struct QDemonDynamicShaderProgramFlags : public QDemonShaderCacheProgramFlags
{
    TessModeValues tessMode = TessModeValues::NoTess;
    bool wireframeMode = false;

    QDemonDynamicShaderProgramFlags() = default;
    QDemonDynamicShaderProgramFlags(TessModeValues inTessMode, bool inWireframeMode)
        : tessMode(inTessMode), wireframeMode(inWireframeMode)
    {
    }

    static const char *wireframeToString(bool inEnable)
    {
        if (inEnable)
            return "wireframeMode:true";
        else
            return "wireframeMode:false";
    }
};
}

struct QDemonDynamicObjectShaderInfo
{
    QByteArray m_type; ///< shader type (GLSL or HLSL)
    QByteArray m_version; ///< shader version (e.g. 330 vor GLSL)
    bool m_hasGeomShader;
    bool m_isComputeShader;

    QDemonDynamicObjectShaderInfo() : m_hasGeomShader(false), m_isComputeShader(false) {}
    QDemonDynamicObjectShaderInfo(const QByteArray &inType, const QByteArray &inVersion, bool inHasGeomShader, bool inIsComputeShader)
        : m_type(inType), m_version(inVersion), m_hasGeomShader(inHasGeomShader), m_isComputeShader(inIsComputeShader)
    {
    }
};

typedef QPair<QDemonRef<QDemonRenderShaderProgram>, dynamic::QDemonDynamicShaderProgramFlags> TShaderAndFlags;

struct QDemonDynamicObjectSystem
{
    typedef QHash<QByteArray, QByteArray> TPathDataMap;
    typedef QHash<QByteArray, QDemonDynamicObjectShaderInfo> TShaderInfoMap;
    typedef QSet<QString> TPathSet;
    typedef QHash<dynamic::QDemonDynamicShaderMapKey, TShaderAndFlags> TShaderMap;

    QDemonRenderContextInterface *m_context;
    TPathDataMap m_expandedFiles;
    TShaderMap m_shaderMap;
    TShaderInfoMap m_shaderInfoMap;
    QByteArray m_vertShader;
    QByteArray m_fragShader;
    QByteArray m_geometryShader;
    QByteArray m_shaderLibraryVersion;
    QString m_shaderLibraryPlatformDirectory;
    mutable QMutex m_propertyLoadMutex;
    QAtomicInt ref;

    static QString getShaderCodeLibraryDirectory();

    QDemonDynamicObjectSystem(QDemonRenderContextInterface *ctx);

    ~QDemonDynamicObjectSystem();

    void setShaderData(const QByteArray &inPath,
                       const QByteArray &inData,
                       const QByteArray &inShaderType,
                       const QByteArray &inShaderVersion,
                       bool inHasGeomShader,
                       bool inIsComputeShader);

    QByteArray getShaderCacheKey(const QByteArray &inId, const QByteArray &inProgramMacro, const dynamic::QDemonDynamicShaderProgramFlags &inFlags);

    void insertShaderHeaderInformation(QByteArray &theReadBuffer, const QByteArray &inPathToEffect);

    void doInsertShaderHeaderInformation(QByteArray &theReadBuffer, const QByteArray &inPathToEffect);

    QByteArray doLoadShader(const QByteArray &inPathToEffect);

    QStringList getParameters(const QString &str, int begin, int end);

    void insertSnapperDirectives(QString &str);

    QDemonRef<QDemonRenderShaderProgram> compileShader(const QByteArray &inId,
                                                       const QByteArray &inProgramSource,
                                                       const QByteArray &inGeomSource,
                                                       QByteArray inProgramMacroName,
                                                       TShaderFeatureSet inFeatureSet,
                                                       const dynamic::QDemonDynamicShaderProgramFlags &inFlags,
                                                       bool inForceCompilation = false);

    // This just returns the custom material shader source without compiling
    QByteArray getShaderSource(const QByteArray &inPath);

    TShaderAndFlags getShaderProgram(const QByteArray &inPath,
                                     const QByteArray &inProgramMacro,
                                     TShaderFeatureSet inFeatureSet,
                                     const dynamic::QDemonDynamicShaderProgramFlags &inFlags,
                                     bool inForceCompilation);

    TShaderAndFlags getDepthPrepassShader(const QByteArray &inPath, const QByteArray &inPMacro, const TShaderFeatureSet &inFeatureSet);

    void setShaderCodeLibraryVersion(const QByteArray &version);

    QByteArray shaderCodeLibraryVersion();

    void setShaderCodeLibraryPlatformDirectory(const QString &directory);

    QString shaderCodeLibraryPlatformDirectory();
};

QT_END_NAMESPACE

#endif
