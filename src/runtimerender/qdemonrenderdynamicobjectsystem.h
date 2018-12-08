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
#pragma once
#ifndef QDEMON_RENDER_DYNAMIC_OBJECT_SYSTEM_H
#define QDEMON_RENDER_DYNAMIC_OBJECT_SYSTEM_H

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemon/qdemonrefcounted.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtGui/QVector2D>
#include <QtDemonRuntimeRender/qdemonrendershadercache.h>
#include <QtDemonRuntimeRender/qdemonrendertessmodevalues.h>
#include <QtDemonRuntimeRender/qdemonrendergraphobjecttypes.h>

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE
struct SDynamicObject;

namespace dynamic {

struct SCommand;

struct SPropertyDeclaration
{
    const char *m_Name;
    // The datatypes map directly to the obvious types *except*
    // for QDemonRenderTexture2DPtr.  This type will be interpreted as a
    // QString (they are the same binary size)
    // and will be used to lookup the texture from the buffer manager.
    QDemonRenderShaderDataTypes::Enum m_DataType;

    SPropertyDeclaration(const char *inName, QDemonRenderShaderDataTypes::Enum inDtype)
        : m_Name(inName)
        , m_DataType(inDtype)
    {
    }
    SPropertyDeclaration()
        : m_Name("")
        , m_DataType(QDemonRenderShaderDataTypes::Unknown)
    {
    }
};

struct SPropertyDefinition
{
    QString m_Name;

    //*not* relative to the presentation directory
    QString m_ImagePath;
    // The datatypes map directly to the obvious types *except*
    // for QDemonRenderTexture2DPtr.  This type will be interpreted as a
    // QString and will be used to lookup the texture
    // from the buffer manager.
    QDemonRenderShaderDataTypes::Enum m_DataType;
    // All offsets are relative to the beginning of the SEffect
    // and are aligned to 4 byte boundaries.
    quint32 m_Offset;
    // Sizeof this datatype.
    quint32 m_ByteSize;
    QDemonConstDataRef<QString> m_EnumValueNames;

    QDemonRenderTextureTypeValue::Enum
    m_TexUsageType; ///< texture usage type like diffuse, specular, ...
    // Applies to both s,t
    QDemonRenderTextureCoordOp::Enum m_CoordOp;
    // Set mag Filter
    QDemonRenderTextureMagnifyingOp::Enum m_MagFilterOp;
    // Set min Filter
    QDemonRenderTextureMinifyingOp::Enum m_MinFilterOp;
    bool m_IsEnumProperty;
    SPropertyDefinition()
        : m_DataType(QDemonRenderShaderDataTypes::Unknown)
        , m_Offset(0)
        , m_ByteSize(0)
        , m_TexUsageType(QDemonRenderTextureTypeValue::Unknown)
        , m_CoordOp(QDemonRenderTextureCoordOp::ClampToEdge)
        , m_MagFilterOp(QDemonRenderTextureMagnifyingOp::Linear)
        , m_MinFilterOp(QDemonRenderTextureMinifyingOp::Linear)
        , m_IsEnumProperty(false)
    {
    }
    SPropertyDefinition(QString inName, QDemonRenderShaderDataTypes::Enum inType,
                        quint32 inOffset, quint32 inByteSize)
        : m_Name(inName)
        , m_DataType(inType)
        , m_Offset(inOffset)
        , m_ByteSize(inByteSize)
        , m_TexUsageType(QDemonRenderTextureTypeValue::Unknown)
        , m_CoordOp(QDemonRenderTextureCoordOp::ClampToEdge)
        , m_MagFilterOp(QDemonRenderTextureMagnifyingOp::Linear)
        , m_MinFilterOp(QDemonRenderTextureMinifyingOp::Linear)
        , m_IsEnumProperty(false)
    {
    }
};

struct SDynamicShaderProgramFlags : public SShaderCacheProgramFlags
{
    TessModeValues::Enum m_TessMode;
    bool m_WireframeMode;

    SDynamicShaderProgramFlags()
        : m_TessMode(TessModeValues::NoTess)
        , m_WireframeMode(false)
    {
    }

    SDynamicShaderProgramFlags(TessModeValues::Enum inTessMode, bool inWireframeMode)
        : m_TessMode(inTessMode)
        , m_WireframeMode(inWireframeMode)
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

class IDynamicObjectClass
{
protected:
    virtual ~IDynamicObjectClass() {}
public:
    virtual QString GetId() const = 0;
    virtual QDemonConstDataRef<dynamic::SPropertyDefinition> GetProperties() const = 0;
    virtual quint32 GetPropertySectionByteSize() const = 0;
    virtual const quint8 *GetDefaultValueBuffer() const = 0;
    virtual quint32 GetBaseObjectSize() const = 0;
    virtual GraphObjectTypes::Enum GraphObjectType() const = 0;
    virtual const dynamic::SPropertyDefinition *
    FindPropertyByName(QString inName) const = 0;
    virtual QDemonConstDataRef<dynamic::SCommand *> GetRenderCommands() const = 0;
    virtual bool RequiresDepthTexture() const = 0;
    virtual void SetRequiresDepthTexture(bool inRequires) = 0;
    virtual bool RequiresCompilation() const = 0;
    virtual void SetRequiresCompilation(bool inRequires) = 0;
    virtual QDemonRenderTextureFormats::Enum GetOutputTextureFormat() const = 0;
};

class IDynamicObjectSystemCore : public QDemonRefCounted
{
protected:
    virtual ~IDynamicObjectSystemCore() {}
public:
    virtual bool IsRegistered(QString inStr) = 0;

    virtual bool Register(QString inName,
                          QDemonConstDataRef<dynamic::SPropertyDeclaration> inProperties,
                          quint32 inBaseObjectSize, GraphObjectTypes::Enum inGraphObjectType) = 0;

    virtual bool Unregister(QString inName) = 0;

    // Set the default value.  THis is unnecessary if the default is zero as that is what it is
    // assumed to be.
    virtual void SetPropertyDefaultValue(QString inName, QString inPropName,
                                         QDemonConstDataRef<quint8> inDefaultData) = 0;

    virtual void SetPropertyEnumNames(QString inName, QString inPropName,
                                      QDemonConstDataRef<QString> inNames) = 0;

    virtual QDemonConstDataRef<QString>
    GetPropertyEnumNames(QString inName, QString inPropName) const = 0;

    virtual QDemonConstDataRef<dynamic::SPropertyDefinition>
    GetProperties(QString inName) const = 0;

    virtual void SetPropertyTextureSettings(QString inName,
                                            QString inPropName,
                                            QString inPropPath,
                                            QDemonRenderTextureTypeValue::Enum inTexType,
                                            QDemonRenderTextureCoordOp::Enum inCoordOp,
                                            QDemonRenderTextureMagnifyingOp::Enum inMagFilterOp,
                                            QDemonRenderTextureMinifyingOp::Enum inMinFilterOp) = 0;

    virtual IDynamicObjectClass *GetDynamicObjectClass(QString inName) = 0;

    // The effect commands are the actual commands that run for a given effect.  The tell the
    // system exactly
    // explicitly things like bind this shader, bind this render target, apply this property,
    // run this shader
    // See UICRenderEffectCommands.h for the list of commands.
    // These commands are copied into the effect.
    virtual void SetRenderCommands(QString inClassName,
                                   QDemonConstDataRef<dynamic::SCommand *> inCommands) = 0;
    virtual QDemonConstDataRef<dynamic::SCommand *>
    GetRenderCommands(QString inClassName) const = 0;

    virtual SDynamicObject *CreateInstance(QString inClassName) = 0;

    // scan shader for #includes and insert any found"
    virtual void InsertShaderHeaderInformation(QString &inShader,
                                               const char *inLogPath) = 0;

    // Set the shader data for a given path.  Used when a path doesn't correspond to a file but
    // the data has been
    // auto-generated.  The system will look for data under this path key during the BindShader
    // effect command.
    virtual void SetShaderData(QString inPath, const char *inData,
                               const char *inShaderType = nullptr,
                               const char *inShaderVersion = nullptr,
                               bool inHasGeomShader = false,
                               bool inIsComputeShader = false) = 0;

    // Overall save functions for saving the class information out to the binary file.
    virtual void Save(SWriteBuffer &ioBuffer,
                      const SStrRemapMap &inRemapMap,
                      const char *inProjectDir) const = 0;
    virtual void Load(QDemonDataRef<quint8> inData, CStrTableOrDataRef inStrDataBlock,
                      const char *inProjectDir) = 0;

    virtual IDynamicObjectSystem &CreateDynamicSystem(IQDemonRenderContext &rc) = 0;

    static IDynamicObjectSystemCore &CreateDynamicSystemCore(IQDemonRenderContextCore &rc);
};

typedef QPair<QDemonScopedRefCounted<QDemonRenderShaderProgram>,
dynamic::SDynamicShaderProgramFlags>
TShaderAndFlags;

class IDynamicObjectSystem : public IDynamicObjectSystemCore
{
protected:
    virtual ~IDynamicObjectSystem() {}

public:
    virtual TShaderAndFlags
    GetShaderProgram(QString inPath, QString inProgramMacro,
                     TShaderFeatureSet inFeatureSet,
                     const dynamic::SDynamicShaderProgramFlags &inFlags,
                     bool inForceCompilation = false) = 0;

    virtual const char *GetShaderSource(QString inPath, CRenderString &source) = 0;

    // Will return null in the case where a custom prepass shader isn't needed for this object
    // If no geom shader, then no depth prepass shader.
    virtual TShaderAndFlags GetDepthPrepassShader(QString inPath,
                                                  QString inProgramMacro,
                                                  TShaderFeatureSet inFeatureSet) = 0;

    virtual void setShaderCodeLibraryVersion(const QString &version) = 0;
    virtual QString shaderCodeLibraryVersion() = 0;

    virtual void setShaderCodeLibraryPlatformDirectory(const QString &directory) = 0;
    virtual QString shaderCodeLibraryPlatformDirectory() = 0;

    static QString GetShaderCodeLibraryDirectory() { return QStringLiteral("res/effectlib"); }
};
QT_END_NAMESPACE

#endif
