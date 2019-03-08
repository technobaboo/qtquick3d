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

QT_BEGIN_NAMESPACE
struct QDemonDynamicObject;
// struct SWriteBuffer;
// struct SStrRemapMap;
class QDemonRenderContextInterface;
class QDemonDynamicObjectSystemInterface;
class QDemonRenderContextCoreInterface;

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
    const char *name;
    // The datatypes map directly to the obvious types *except*
    // for QDemonRenderTexture2DPtr.  This type will be interpreted as a
    // QString (they are the same binary size)
    // and will be used to lookup the texture from the buffer manager.
    QDemonRenderShaderDataType dataType;

    QDemonPropertyDeclaration(const char *inName, QDemonRenderShaderDataType inDtype)
        : name(inName), dataType(inDtype)
    {
    }
    QDemonPropertyDeclaration() : name(""), dataType(QDemonRenderShaderDataType::Unknown) {}
};

struct QDemonPropertyDefinition
{
    QString name;

    //*not* relative to the presentation directory
    QString imagePath;
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
    QDemonConstDataRef<QString> enumValueNames;

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
    QDemonPropertyDefinition(QString inName, QDemonRenderShaderDataType inType, quint32 inOffset, quint32 inByteSize)
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

class Q_DEMONRUNTIMERENDER_EXPORT QDemonDynamicObjectClassInterface
{
public:
    QAtomicInt ref;
    virtual ~QDemonDynamicObjectClassInterface() {}
    virtual QString getId() const = 0;
    virtual QDemonConstDataRef<dynamic::QDemonPropertyDefinition> getProperties() const = 0;
    virtual quint32 getPropertySectionByteSize() const = 0;
    virtual const quint8 *getDefaultValueBuffer() const = 0;
    virtual quint32 getBaseObjectSize() const = 0;
    virtual QDemonGraphObject::Type graphObjectType() const = 0;
    virtual const dynamic::QDemonPropertyDefinition *findPropertyByName(QString inName) const = 0;
    virtual QDemonConstDataRef<dynamic::QDemonCommand *> getRenderCommands() const = 0;
    virtual bool requiresDepthTexture() const = 0;
    virtual void setRequiresDepthTexture(bool inRequires) = 0;
    virtual bool requiresCompilation() const = 0;
    virtual void setRequiresCompilation(bool inRequires) = 0;
    virtual QDemonRenderTextureFormat getOutputTextureFormat() const = 0;
};

typedef QPair<QDemonRef<QDemonRenderShaderProgram>, dynamic::QDemonDynamicShaderProgramFlags> TShaderAndFlags;

class Q_DEMONRUNTIMERENDER_EXPORT QDemonDynamicObjectSystemInterface
{
public:
    QAtomicInt ref;
    virtual ~QDemonDynamicObjectSystemInterface();
    virtual bool isRegistered(QString inStr) = 0;

    virtual bool doRegister(QString inName,
                            QDemonConstDataRef<dynamic::QDemonPropertyDeclaration> inProperties,
                            quint32 inBaseObjectSize,
                            QDemonGraphObject::Type inGraphObjectType) = 0;

    virtual bool unregister(QString inName) = 0;

    // Set the default value.  THis is unnecessary if the default is zero as that is what it is
    // assumed to be.
    virtual void setPropertyDefaultValue(const QString &inName,
                                         const QString &inPropName,
                                         const QDemonConstDataRef<quint8> &inDefaultData) = 0;

    virtual void setPropertyEnumNames(const QString &inName, const QString &inPropName, const QDemonConstDataRef<QString> &inNames) = 0;

    virtual QDemonConstDataRef<QString> getPropertyEnumNames(const QString &inName, const QString &inPropName) const = 0;

    virtual QDemonConstDataRef<dynamic::QDemonPropertyDefinition> getProperties(const QString &inName) const = 0;

    virtual void setPropertyTextureSettings(const QString &inName,
                                            const QString &inPropName,
                                            const QString &inPropPath,
                                            QDemonRenderTextureTypeValue inTexType,
                                            QDemonRenderTextureCoordOp inCoordOp,
                                            QDemonRenderTextureMagnifyingOp inMagFilterOp,
                                            QDemonRenderTextureMinifyingOp inMinFilterOp) = 0;

    virtual QDemonDynamicObjectClassInterface *getDynamicObjectClass(const QString &inName) = 0;

    // The effect commands are the actual commands that run for a given effect.  The tell the
    // system exactly
    // explicitly things like bind this shader, bind this render target, apply this property,
    // run this shader
    // See qdemonrenderdynamicobjectssystemcommands.h for the list of commands.
    // These commands are copied into the effect.
    virtual void setRenderCommands(const QString &inClassName, const QDemonConstDataRef<dynamic::QDemonCommand *> &inCommands) = 0;
    virtual QDemonConstDataRef<dynamic::QDemonCommand *> getRenderCommands(const QString &inClassName) const = 0;

    virtual QDemonDynamicObject *createInstance(const QString &inClassName) = 0;

    // scan shader for #includes and insert any found"
    virtual void insertShaderHeaderInformation(QByteArray &inShader, const char *inLogPath) = 0;

    // Set the shader data for a given path.  Used when a path doesn't correspond to a file but
    // the data has been
    // auto-generated.  The system will look for data under this path key during the BindShader
    // effect command.
    virtual void setShaderData(const QString &inPath,
                               const char *inData,
                               const char *inShaderType = nullptr,
                               const char *inShaderVersion = nullptr,
                               bool inHasGeomShader = false,
                               bool inIsComputeShader = false) = 0;

    //    // Overall save functions for saving the class information out to the binary file.
    //    virtual void Save(SWriteBuffer &ioBuffer,
    //                      const SStrRemapMap &inRemapMap,
    //                      const char *inProjectDir) const = 0;
    //    virtual void Load(QDemonDataRef<quint8> inData, CStrTableOrDataRef inStrDataBlock,
    //                      const char *inProjectDir) = 0;

    virtual void setContextInterface(QDemonRenderContextInterface *rc) = 0;

    static QDemonRef<QDemonDynamicObjectSystemInterface> createDynamicSystem(QDemonRenderContextCoreInterface *rc);

    virtual TShaderAndFlags getShaderProgram(QString inPath,
                                             QString inProgramMacro,
                                             TShaderFeatureSet inFeatureSet,
                                             const dynamic::QDemonDynamicShaderProgramFlags &inFlags,
                                             bool inForceCompilation = false) = 0;

    virtual QString getShaderSource(QString inPath) = 0;

    // Will return null in the case where a custom prepass shader isn't needed for this object
    // If no geom shader, then no depth prepass shader.
    virtual TShaderAndFlags getDepthPrepassShader(QString inPath, QString inProgramMacro, TShaderFeatureSet inFeatureSet) = 0;

    virtual void setShaderCodeLibraryVersion(const QByteArray &version) = 0;
    virtual QString shaderCodeLibraryVersion() = 0;

    virtual void setShaderCodeLibraryPlatformDirectory(const QString &directory) = 0;
    virtual QString shaderCodeLibraryPlatformDirectory() = 0;

    static QString getShaderCodeLibraryDirectory() { return QStringLiteral("res/effectlib"); }
};

QT_END_NAMESPACE

#endif
