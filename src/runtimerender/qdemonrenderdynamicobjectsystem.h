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

typedef QPair<QString, QString> TStrStrPair;

namespace dynamic {

struct QDemonDynamicShaderMapKey
{
    TStrStrPair m_name;
    QVector<QDemonShaderPreprocessorFeature> m_features;
    TessModeValues m_tessMode;
    bool m_wireframeMode;
    size_t m_hashCode;
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

struct QDemonPropertyDeclaration
{
    // NOTE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // The data here is memcpyied, so only trivial type with know life expectency!!!
    // Alternatively we need to do a copyConstruct as done for the commands
    const char *name;
    // The datatypes map directly to the obvious types *except*
    // for QDemonRenderTexture2DPtr.  This type will be interpreted as a
    // QString (they are the same binary size)
    // and will be used to lookup the texture from the buffer manager.
    QDemonRenderShaderDataType dataType = QDemonRenderShaderDataType::Unknown;

    QDemonPropertyDeclaration(const char *inName, QDemonRenderShaderDataType inDtype)
        : name(inName), dataType(inDtype)
    {
    }
    QDemonPropertyDeclaration() = default;
};

struct QDemonPropertyDefinition
{
    // NOTE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // The data here is memcpyied, so only trivial type with know life expectency!!!
    // Alternatively we need to do a copyConstruct as done for the commands
    const char *name = nullptr;

    //*not* relative to the presentation directory
    const char *imagePath = nullptr;
    // The datatypes map directly to the obvious types *except*
    // for QDemonRenderTexture2DPtr.  This type will be interpreted as a
    // QString and will be used to lookup the texture
    // from the buffer manager.
    QDemonRenderShaderDataType dataType = QDemonRenderShaderDataType::Unknown;
    // All offsets are relative to the beginning of the SEffect
    // and are aligned to 4 byte boundaries.
    quint32 offset = 0;
    // Sizeof this datatype.
    quint32 byteSize = 0;
    QDemonDataView<QString> enumValueNames;

    ///< texture usage type like diffuse, specular, ...
    QDemonRenderTextureTypeValue texUsageType = QDemonRenderTextureTypeValue::Unknown;
    // Applies to both s,t
    QDemonRenderTextureCoordOp coordOp = QDemonRenderTextureCoordOp::ClampToEdge;
    // Set mag Filter
    QDemonRenderTextureMagnifyingOp magFilterOp = QDemonRenderTextureMagnifyingOp::Linear;
    // Set min Filter
    QDemonRenderTextureMinifyingOp minFilterOp = QDemonRenderTextureMinifyingOp::Linear;
    bool isEnumProperty = false;

    QDemonPropertyDefinition() = default;
    QDemonPropertyDefinition(const char *inName, QDemonRenderShaderDataType inType, quint32 inOffset, quint32 inByteSize)
        : name(inName), dataType(inType), offset(inOffset), byteSize(inByteSize)
    {
    }
};

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
    QString m_type; ///< shader type (GLSL or HLSL)
    QString m_version; ///< shader version (e.g. 330 vor GLSL)
    bool m_hasGeomShader;
    bool m_isComputeShader;

    QDemonDynamicObjectShaderInfo() : m_hasGeomShader(false), m_isComputeShader(false) {}
    QDemonDynamicObjectShaderInfo(QString inType, QString inVersion, bool inHasGeomShader, bool inIsComputeShader)
        : m_type(inType), m_version(inVersion), m_hasGeomShader(inHasGeomShader), m_isComputeShader(inIsComputeShader)
    {
    }
};

struct QDemonDynamicObjectClass
{
    QAtomicInt ref;
    QString m_id;
    QDemonDataView<dynamic::QDemonPropertyDefinition> m_propertyDefinitions;
    quint32 m_propertySectionByteSize;
    quint32 m_baseObjectSize;
    QDemonRenderGraphObject::Type m_graphObjectType;
    quint8 *m_propertyDefaultData;
    QDemonDataView<dynamic::QDemonCommand *> m_renderCommands;
    bool m_requiresDepthTexture;
    bool m_requiresCompilation;
    QDemonRenderTextureFormat m_outputFormat;

    QDemonDynamicObjectClass(QString id,
                             QDemonDataView<dynamic::QDemonPropertyDefinition> definitions,
                             quint32 propertySectionByteSize,
                             quint32 baseObjectSize,
                             QDemonRenderGraphObject::Type objectType,
                             quint8 *propDefaultData,
                             bool inRequiresDepthTexture = false,
                             QDemonRenderTextureFormat inOutputFormat = QDemonRenderTextureFormat::RGBA8);

    ~QDemonDynamicObjectClass();

    void releaseCommands();

    QString getId() const;
    QDemonDataView<dynamic::QDemonPropertyDefinition> getProperties() const;
    quint32 getPropertySectionByteSize() const;
    const quint8 *getDefaultValueBuffer() const;
    quint32 getBaseObjectSize() const;
    QDemonRenderGraphObject::Type graphObjectType() const;
    const dynamic::QDemonPropertyDefinition *findDefinition(QString &str) const;
    const dynamic::QDemonPropertyDefinition *findPropertyByName(QString inName) const;
    QDemonDataView<dynamic::QDemonCommand *> getRenderCommands() const;
    bool requiresDepthTexture() const;
    void setRequiresDepthTexture(bool inVal);
    bool requiresCompilation() const;
    void setRequiresCompilation(bool inVal);
    QDemonRenderTextureFormat getOutputTextureFormat() const;
};

typedef QPair<QDemonRef<QDemonRenderShaderProgram>, dynamic::QDemonDynamicShaderProgramFlags> TShaderAndFlags;

struct QDemonDynamicObjectSystem
{
    typedef QHash<QString, QDemonRef<QDemonDynamicObjectClass>> TStringClassMap;
    typedef QHash<QString, QByteArray> TPathDataMap;
    typedef QHash<QString, QDemonDynamicObjectShaderInfo> TShaderInfoMap;
    typedef QSet<QString> TPathSet;
    typedef QHash<dynamic::QDemonDynamicShaderMapKey, TShaderAndFlags> TShaderMap;

    QDemonRenderContextInterface *m_context;
    TStringClassMap m_classes;
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

    bool isRegistered(QString inStr);

    bool doRegister(QString inName,
                    QDemonDataView<dynamic::QDemonPropertyDeclaration> inProperties,
                    quint32 inBaseObjectSize,
                    QDemonRenderGraphObject::Type inGraphObjectType);

    bool unregister(QString inName);

    QDemonRef<QDemonDynamicObjectClass> findClass(QString inName);

    QPair<const dynamic::QDemonPropertyDefinition *, QDemonRef<QDemonDynamicObjectClass>> findProperty(QString inName, QString inPropName);

    void setPropertyDefaultValue(const QString &inName, const QString &inPropName, const QDemonByteView &inDefaultData);

    void setPropertyEnumNames(const QString &inName, const QString &inPropName, const QDemonDataView<QString> &inNames);

    QDemonDataView<QString> getPropertyEnumNames(const QString &inName, const QString &inPropName) const;

    // Called during loading which is pretty heavily multithreaded.
    QDemonDataView<dynamic::QDemonPropertyDefinition> getProperties(const QString &inName) const;

    void setPropertyTextureSettings(const QString &inName,
                                    const QString &inPropName,
                                    const QString &inPropPath,
                                    QDemonRenderTextureTypeValue inTexType,
                                    QDemonRenderTextureCoordOp inCoordOp,
                                    QDemonRenderTextureMagnifyingOp inMagFilterOp,
                                    QDemonRenderTextureMinifyingOp inMinFilterOp);

    QDemonDynamicObjectClass *dynamicObjectClass(const QString &inName);

    void setRenderCommands(const QString &inClassName, const QDemonDataView<dynamic::QDemonCommand *> &inCommands);

    QDemonDataView<dynamic::QDemonCommand *> getRenderCommands(const QString &inClassName) const;

    QDemonRenderDynamicGraphObject *createInstance(const QString &inClassName);

    void setShaderData(const QString &inPath,
                       const QByteArray &inData,
                       const QByteArray &inShaderType,
                       const QByteArray &inShaderVersion,
                       bool inHasGeomShader,
                       bool inIsComputeShader);

    QByteArray getShaderCacheKey(const char *inId, const char *inProgramMacro, const dynamic::QDemonDynamicShaderProgramFlags &inFlags);

    void insertShaderHeaderInformation(QByteArray &theReadBuffer, const char *inPathToEffect);

    void doInsertShaderHeaderInformation(QByteArray &theReadBuffer, const QString &inPathToEffect);

    QByteArray doLoadShader(const QString &inPathToEffect);

    QStringList getParameters(const QString &str, int begin, int end);

    void insertSnapperDirectives(QString &str);

    QDemonRef<QDemonRenderShaderProgram> compileShader(QString inId,
                                                       const char *inProgramSource,
                                                       const char *inGeomSource,
                                                       QString inProgramMacroName,
                                                       TShaderFeatureSet inFeatureSet,
                                                       const dynamic::QDemonDynamicShaderProgramFlags &inFlags,
                                                       bool inForceCompilation = false);

    // This just returns the custom material shader source without compiling
    QString getShaderSource(QString inPath);

    TShaderAndFlags getShaderProgram(QString inPath,
                                     QString inProgramMacro,
                                     TShaderFeatureSet inFeatureSet,
                                     const dynamic::QDemonDynamicShaderProgramFlags &inFlags,
                                     bool inForceCompilation);

    TShaderAndFlags getDepthPrepassShader(QString inPath, QString inPMacro, TShaderFeatureSet inFeatureSet);

    void setShaderCodeLibraryVersion(const QByteArray &version);

    QString shaderCodeLibraryVersion();

    void setShaderCodeLibraryPlatformDirectory(const QString &directory);

    QString shaderCodeLibraryPlatformDirectory();
};

QT_END_NAMESPACE

#endif
