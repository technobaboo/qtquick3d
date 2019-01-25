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
#include "qdemonrenderdynamicobjectsystem.h"

#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRuntimeRender/qdemonrenderdynamicobject.h>
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

uint qHash(const SDynamicShaderMapKey &inKey)
{
    return inKey.m_HashCode;
}

quint32 SCommand::GetSizeofCommand(const SCommand &inCommand)
{
    switch (inCommand.m_Type) {
    case CommandTypes::AllocateBuffer:
        return sizeof(SAllocateBuffer);
    case CommandTypes::BindBuffer:
        return sizeof(SBindBuffer);
    case CommandTypes::BindTarget:
        return sizeof(SBindTarget);
    case CommandTypes::BindShader:
        return sizeof(SBindShader);
    case CommandTypes::Render:
        return sizeof(SRender);
    case CommandTypes::ApplyBufferValue:
        return sizeof(SApplyBufferValue);
    case CommandTypes::ApplyDepthValue:
        return sizeof(SApplyDepthValue);
    case CommandTypes::ApplyInstanceValue:
        return sizeof(SApplyInstanceValue);
    case CommandTypes::ApplyBlending:
        return sizeof(SApplyBlending);
    case CommandTypes::ApplyRenderState:
        return sizeof(SApplyRenderState);
    case CommandTypes::ApplyBlitFramebuffer:
        return sizeof(SApplyBlitFramebuffer);
    case CommandTypes::ApplyValue:
        return sizeof(SApplyValue)
                + static_cast<const SApplyValue &>(inCommand).m_Value.mSize;
    case CommandTypes::DepthStencil:
        return sizeof(SDepthStencil);
    case CommandTypes::AllocateImage:
        return sizeof(SAllocateImage);
    case CommandTypes::ApplyImageValue:
        return sizeof(SApplyImageValue);
    case CommandTypes::AllocateDataBuffer:
        return sizeof(SAllocateDataBuffer);
    case CommandTypes::ApplyDataBufferValue:
        return sizeof(SApplyDataBufferValue);
    default:
        break;
    }
    Q_ASSERT(false);
    return 0;
}

template <typename TCommandType>
inline void CopyConstructCommandT(quint8 *inDataBuffer, const SCommand &inCommand)
{
    TCommandType *theCommand = (TCommandType *)inDataBuffer;
    theCommand = new (theCommand)
            TCommandType(static_cast<const TCommandType &>(inCommand));
}

void SCommand::CopyConstructCommand(quint8 *inDataBuffer, const SCommand &inCommand)
{
    switch (inCommand.m_Type) {
    case CommandTypes::AllocateBuffer:
        CopyConstructCommandT<SAllocateBuffer>(inDataBuffer, inCommand);
        break;
    case CommandTypes::BindBuffer:
        CopyConstructCommandT<SBindBuffer>(inDataBuffer, inCommand);
        break;
    case CommandTypes::BindTarget:
        CopyConstructCommandT<SBindTarget>(inDataBuffer, inCommand);
        break;
    case CommandTypes::BindShader:
        CopyConstructCommandT<SBindShader>(inDataBuffer, inCommand);
        break;
    case CommandTypes::Render:
        CopyConstructCommandT<SRender>(inDataBuffer, inCommand);
        break;
    case CommandTypes::ApplyBufferValue:
        CopyConstructCommandT<SApplyBufferValue>(inDataBuffer, inCommand);
        break;
    case CommandTypes::ApplyDepthValue:
        CopyConstructCommandT<SApplyDepthValue>(inDataBuffer, inCommand);
        break;
    case CommandTypes::ApplyInstanceValue:
        CopyConstructCommandT<SApplyInstanceValue>(inDataBuffer, inCommand);
        break;
    case CommandTypes::ApplyBlending:
        CopyConstructCommandT<SApplyBlending>(inDataBuffer, inCommand);
        break;
    case CommandTypes::ApplyRenderState:
        CopyConstructCommandT<SApplyRenderState>(inDataBuffer, inCommand);
        break;
    case CommandTypes::ApplyBlitFramebuffer:
        CopyConstructCommandT<SApplyBlitFramebuffer>(inDataBuffer, inCommand);
        break;
    case CommandTypes::ApplyValue: {
        CopyConstructCommandT<SApplyValue>(inDataBuffer, inCommand);
        SApplyValue &dest = *reinterpret_cast<SApplyValue *>(inDataBuffer);
        quint8 *destMem = inDataBuffer + sizeof(SApplyValue);
        const SApplyValue &src = static_cast<const SApplyValue &>(inCommand);
        memcpy(destMem, src.m_Value.mData, src.m_Value.mSize);
        dest.m_Value.mData = destMem;
        break;
    }
    case CommandTypes::DepthStencil:
        CopyConstructCommandT<SDepthStencil>(inDataBuffer, inCommand);
        break;
    case CommandTypes::AllocateImage:
        CopyConstructCommandT<SAllocateImage>(inDataBuffer, inCommand);
        break;
    case CommandTypes::ApplyImageValue:
        CopyConstructCommandT<SApplyImageValue>(inDataBuffer, inCommand);
        break;
    case CommandTypes::AllocateDataBuffer:
        CopyConstructCommandT<SAllocateDataBuffer>(inDataBuffer, inCommand);
        break;
    case CommandTypes::ApplyDataBufferValue:
        CopyConstructCommandT<SApplyDataBufferValue>(inDataBuffer, inCommand);
        break;
    default:
        Q_ASSERT(false);
        break;
    }
}
}

namespace {

template <typename TCommandType>
struct SCommandRemapping
{
    template <typename TRemapper>
    static void RemapCommandData(TCommandType &, TRemapper &)
    {
    }
};

template <>
struct SCommandRemapping<dynamic::SAllocateBuffer>
{
    template <typename TRemapper>
    static void RemapCommandData(dynamic::SAllocateBuffer &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_Name);
    }
};

template <>
struct SCommandRemapping<dynamic::SAllocateImage>
{
    template <typename TRemapper>
    static void RemapCommandData(dynamic::SAllocateImage &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_Name);
    }
};

template <>
struct SCommandRemapping<dynamic::SAllocateDataBuffer>
{
    template <typename TRemapper>
    static void RemapCommandData(dynamic::SAllocateDataBuffer &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_Name);
        if (!cmd.m_WrapName.isEmpty())
            remapper.Remap(cmd.m_WrapName);
    }
};

template <>
struct SCommandRemapping<dynamic::SBindBuffer>
{
    template <typename TRemapper>
    static void RemapCommandData(dynamic::SBindBuffer &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_BufferName);
    }
};
template <>
struct SCommandRemapping<dynamic::SBindShader>
{
    template <typename TRemapper>
    static void RemapCommandData(dynamic::SBindShader &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_ShaderPath);
        remapper.Remap(cmd.m_ShaderDefine);
    }
};
template <>
struct SCommandRemapping<dynamic::SApplyInstanceValue>
{
    template <typename TRemapper>
    static void RemapCommandData(dynamic::SApplyInstanceValue &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_PropertyName);
    }
};
template <>
struct SCommandRemapping<dynamic::SApplyBufferValue>
{
    template <typename TRemapper>
    static void RemapCommandData(dynamic::SApplyBufferValue &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_BufferName);
        remapper.Remap(cmd.m_ParamName);
    }
};

template <>
struct SCommandRemapping<dynamic::SApplyDepthValue>
{
    template <typename TRemapper>
    static void RemapCommandData(dynamic::SApplyDepthValue &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_ParamName);
    }
};

template <>
struct SCommandRemapping<dynamic::SApplyBlitFramebuffer>
{
    template <typename TRemapper>
    static void RemapCommandData(dynamic::SApplyBlitFramebuffer &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_SourceBufferName);
        remapper.Remap(cmd.m_DestBufferName);
    }
};

template <>
struct SCommandRemapping<dynamic::SApplyValue>
{
    template <typename TRemapper>
    static void RemapCommandData(dynamic::SApplyValue &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_PropertyName);
    }
};

template <>
struct SCommandRemapping<dynamic::SApplyDataBufferValue>
{
    template <typename TRemapper>
    static void RemapCommandData(dynamic::SApplyDataBufferValue &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_ParamName);
    }
};

template <>
struct SCommandRemapping<dynamic::SDepthStencil>
{
    template <typename TRemapper>
    static void RemapCommandData(dynamic::SDepthStencil &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_BufferName);
    }
};

quint32 Align(quint32 inValue)
{
    if (inValue % 4)
        return inValue + (4 - (inValue % 4));
    return inValue;
}

quint32 Align8(quint32 inValue)
{
    if (inValue % 8)
        return inValue + (8 - (inValue % 8));
    return inValue;
}

inline const char *GetShaderDatatypeName(QDemonRenderShaderDataTypes::Enum inValue)
{
    switch (inValue) {
    case QDemonRenderShaderDataTypes::Unknown:
        return "";
    case QDemonRenderShaderDataTypes::Integer:
        return "qint32";
    case QDemonRenderShaderDataTypes::IntegerVec2:
        return "qint32_2";
    case QDemonRenderShaderDataTypes::IntegerVec3:
        return "qint32_3";
    case QDemonRenderShaderDataTypes::IntegerVec4:
        return "qint32_4";
    case QDemonRenderShaderDataTypes::Boolean:
        return "bool";
    case QDemonRenderShaderDataTypes::BooleanVec2:
        return "bool_2";
    case QDemonRenderShaderDataTypes::BooleanVec3:
        return "bool_3";
    case QDemonRenderShaderDataTypes::BooleanVec4:
        return "bool_4";
    case QDemonRenderShaderDataTypes::Float:
        return "float";
    case QDemonRenderShaderDataTypes::Vec2:
        return "QVector2D";
    case QDemonRenderShaderDataTypes::Vec3:
        return "QVector3D";
    case QDemonRenderShaderDataTypes::Vec4:
        return "QVector4D";
    case QDemonRenderShaderDataTypes::UnsignedInteger:
        return "quint32";
    case QDemonRenderShaderDataTypes::UnsignedIntegerVec2:
        return "quint32_2";
    case QDemonRenderShaderDataTypes::UnsignedIntegerVec3:
        return "quint32_3";
    case QDemonRenderShaderDataTypes::UnsignedIntegerVec4:
        return "quint32_4";
    case QDemonRenderShaderDataTypes::Matrix3x3:
        return "QMatrix3x3";
    case QDemonRenderShaderDataTypes::Matrix4x4:
        return "QMatrix4x4";
    case QDemonRenderShaderDataTypes::Texture2D:
        return "QDemonRenderTexture2DPtr";
    case QDemonRenderShaderDataTypes::Texture2DHandle:
        return "QDemonRenderTexture2DHandle";
    case QDemonRenderShaderDataTypes::Texture2DArray:
        return "QDemonRenderTexture2DArrayPtr";
    case QDemonRenderShaderDataTypes::TextureCube:
        return "QDemonRenderTextureCubePtr";
    case QDemonRenderShaderDataTypes::TextureCubeHandle:
        return "QDemonRenderTextureCubeHandle";
    case QDemonRenderShaderDataTypes::Image2D:
        return "QDemonRenderImage2DPtr";
    case QDemonRenderShaderDataTypes::DataBuffer:
        return "QDemonRenderDataBufferPtr";
    }
    Q_ASSERT(false);
    return "";
}

inline quint32 getSizeofShaderDataType(QDemonRenderShaderDataTypes::Enum value)
{
    switch (value) {
    case QDemonRenderShaderDataTypes::Unknown:
        return 0;
    case QDemonRenderShaderDataTypes::Integer:
        return sizeof(qint32);
    case QDemonRenderShaderDataTypes::IntegerVec2:
        return sizeof(qint32_2);
    case QDemonRenderShaderDataTypes::IntegerVec3:
        return sizeof(qint32_3);
    case QDemonRenderShaderDataTypes::IntegerVec4:
        return sizeof(qint32_4);
    case QDemonRenderShaderDataTypes::Boolean:
        return sizeof(bool);
    case QDemonRenderShaderDataTypes::BooleanVec2:
        return sizeof(bool_2);
    case QDemonRenderShaderDataTypes::BooleanVec3:
        return sizeof(bool_3);
    case QDemonRenderShaderDataTypes::BooleanVec4:
        return sizeof(bool_4);
    case QDemonRenderShaderDataTypes::Float:
        return sizeof(float);
    case QDemonRenderShaderDataTypes::Vec2:
        return sizeof(QVector2D);
    case QDemonRenderShaderDataTypes::Vec3:
        return sizeof(QVector3D);
    case QDemonRenderShaderDataTypes::Vec4:
        return sizeof(QVector4D);
    case QDemonRenderShaderDataTypes::UnsignedInteger:
        return sizeof(quint32);
    case QDemonRenderShaderDataTypes::UnsignedIntegerVec2:
        return sizeof(quint32_2);
    case QDemonRenderShaderDataTypes::UnsignedIntegerVec3:
        return sizeof(quint32_3);
    case QDemonRenderShaderDataTypes::UnsignedIntegerVec4:
        return sizeof(quint32_4);
    case QDemonRenderShaderDataTypes::Matrix3x3:
        return sizeof(QMatrix3x3);
    case QDemonRenderShaderDataTypes::Matrix4x4:
        return sizeof(QMatrix4x4);
    case QDemonRenderShaderDataTypes::Texture2D:
        return sizeof(QDemonRenderTexture2DPtr);
    case QDemonRenderShaderDataTypes::Texture2DHandle:
        return sizeof(QDemonRenderTexture2DHandle);
    case QDemonRenderShaderDataTypes::Texture2DArray:
        return sizeof(QDemonRenderTexture2DArrayPtr);
    case QDemonRenderShaderDataTypes::TextureCube:
        return sizeof(QDemonRenderTextureCubePtr);
    case QDemonRenderShaderDataTypes::TextureCubeHandle:
        return sizeof(QDemonRenderTextureCubeHandle);
    case QDemonRenderShaderDataTypes::Image2D:
        return sizeof(QDemonRenderImage2DPtr);
    case QDemonRenderShaderDataTypes::DataBuffer:
        return sizeof(QDemonRenderDataBufferPtr);
    }
    Q_ASSERT(false);
    return 0;
}

struct SDynamicObjectShaderInfo
{
    QString m_Type; ///< shader type (GLSL or HLSL)
    QString m_Version; ///< shader version (e.g. 330 vor GLSL)
    bool m_HasGeomShader;
    bool m_IsComputeShader;

    SDynamicObjectShaderInfo()
        : m_HasGeomShader(false)
        , m_IsComputeShader(false)
    {
    }
    SDynamicObjectShaderInfo(QString inType, QString inVersion,
                             bool inHasGeomShader, bool inIsComputeShader)
        : m_Type(inType)
        , m_Version(inVersion)
        , m_HasGeomShader(inHasGeomShader)
        , m_IsComputeShader(inIsComputeShader)
    {
    }
};

struct SDynamicObjClassImpl : public IDynamicObjectClass
{
    QString m_Id;
    QDemonConstDataRef<dynamic::SPropertyDefinition> m_PropertyDefinitions;
    quint32 m_PropertySectionByteSize;
    quint32 m_BaseObjectSize;
    GraphObjectTypes::Enum m_GraphObjectType;
    quint8 *m_PropertyDefaultData;
    QDemonConstDataRef<dynamic::SCommand *> m_RenderCommands;
    bool m_RequiresDepthTexture;
    bool m_RequiresCompilation;
    QDemonRenderTextureFormats::Enum m_OutputFormat;

    SDynamicObjClassImpl(QString id,
                         QDemonConstDataRef<dynamic::SPropertyDefinition> definitions,
                         quint32 propertySectionByteSize,
                         quint32 baseObjectSize,
                         GraphObjectTypes::Enum objectType,
                         quint8 *propDefaultData,
                         bool inRequiresDepthTexture = false,
                         QDemonRenderTextureFormats::Enum inOutputFormat = QDemonRenderTextureFormats::RGBA8)
        : m_Id(id)
        , m_PropertyDefinitions(definitions)
        , m_PropertySectionByteSize(propertySectionByteSize)
        , m_BaseObjectSize(baseObjectSize)
        , m_GraphObjectType(objectType)
        , m_PropertyDefaultData(propDefaultData)
        , m_RequiresDepthTexture(inRequiresDepthTexture)
        , m_RequiresCompilation(false)
        , m_OutputFormat(inOutputFormat)
    {
    }

    ~SDynamicObjClassImpl() override
    {
        if (m_PropertyDefinitions.size()) {
            for (quint32 idx = 0, end = m_PropertyDefinitions.size(); idx < end; ++idx) {
                if (m_PropertyDefinitions[idx].m_EnumValueNames.size()) // ### You can't free a QString like this!
                    ::free((void *)m_PropertyDefinitions[idx].m_EnumValueNames.begin());
            }
        }
        ReleaseCommands();
    }

    template <typename TRemapperType>
    static void RemapCommand(dynamic::SCommand &inCommand, TRemapperType &inRemapper)
    {
        switch (inCommand.m_Type) {
#define QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(type)                                              \
        case dynamic::CommandTypes::type:                                                                       \
    SCommandRemapping<dynamic::S##type>::RemapCommandData(static_cast<dynamic::S##type &>(inCommand),            \
    inRemapper);                                  \
    break;
        QDEMON_RENDER_EFFECTS_ITERATE_COMMAND_TYPES
        #undef QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES
                default:
            Q_ASSERT(false);
        break;
        }
    }
    template <typename TRemapper>
    void SetupThisObjectFromMemory(TRemapper &inRemapper, quint8 *inCommandStart, quint32 numEffectCommands)
    {
        quint8 *theCommandPtrBegin = inCommandStart;
        quint32 theCommandOffset = 0;
        for (quint32 idx = 0; idx < numEffectCommands; ++idx) {
            dynamic::SCommand *theCommand = reinterpret_cast<dynamic::SCommand *>(inCommandStart + theCommandOffset);
            theCommandOffset += dynamic::SCommand::GetSizeofCommand(*theCommand);
        }
        dynamic::SCommand **theCommandPtrStart =
                reinterpret_cast<dynamic::SCommand **>(theCommandPtrBegin + theCommandOffset);
        m_RenderCommands = QDemonConstDataRef<dynamic::SCommand *>(theCommandPtrStart, numEffectCommands);
        // Now run through the commands, fixup strings and setup the command ptrs
        theCommandOffset = 0;
        for (quint32 idx = 0; idx < numEffectCommands; ++idx) {
            dynamic::SCommand *theCommand =
                    reinterpret_cast<dynamic::SCommand *>(theCommandPtrBegin + theCommandOffset);
            theCommandPtrStart[idx] = theCommand;
            RemapCommand(*theCommand, inRemapper);
            theCommandOffset += dynamic::SCommand::GetSizeofCommand(*theCommand);
        }
    }

    void ReleaseCommands()
    {
        if (m_RenderCommands.size()) {
            ::free(const_cast<dynamic::SCommand *>(*m_RenderCommands.begin()));
            m_RenderCommands = QDemonConstDataRef<dynamic::SCommand *>();
        }
    }

    QString GetId() const override { return m_Id; }
    QDemonConstDataRef<dynamic::SPropertyDefinition> GetProperties() const override
    {
        return m_PropertyDefinitions;
    }
    quint32 GetPropertySectionByteSize() const override { return m_PropertySectionByteSize; }
    const quint8 *GetDefaultValueBuffer() const override { return m_PropertyDefaultData; }
    quint32 GetBaseObjectSize() const override { return m_BaseObjectSize; }
    GraphObjectTypes::Enum GraphObjectType() const override { return m_GraphObjectType; }
    const dynamic::SPropertyDefinition *FindDefinition(QString &str) const
    {
        for (quint32 idx = 0, end = m_PropertyDefinitions.size(); idx < end; ++idx) {
            const dynamic::SPropertyDefinition &def(m_PropertyDefinitions[idx]);
            if (def.m_Name == str)
                return &def;
        }
        return nullptr;
    }
    const dynamic::SPropertyDefinition *FindPropertyByName(QString inName) const override
    {
        return FindDefinition(inName);
    }
    QDemonConstDataRef<dynamic::SCommand *> GetRenderCommands() const override
    {
        return m_RenderCommands;
    }
    bool RequiresDepthTexture() const override { return m_RequiresDepthTexture; }
    void SetRequiresDepthTexture(bool inVal) override { m_RequiresDepthTexture = inVal; }
    virtual bool RequiresCompilation() const override { return m_RequiresCompilation; }
    virtual void SetRequiresCompilation(bool inVal) override { m_RequiresCompilation = inVal; }
    QDemonRenderTextureFormats::Enum GetOutputTextureFormat() const override { return m_OutputFormat; }
};

struct SDataRemapper
{
    template <typename TRemapper>
    void Remap(quint8 *inData, dynamic::SPropertyDefinition &item, TRemapper &remapper)
    {
        switch (item.m_DataType) {
        default:
            break; // no remapping necessary
        case QDemonRenderShaderDataTypes::Texture2D:
            QString *realData = reinterpret_cast<QString *>(inData);
            remapper.Remap(*realData);
            break;
        }
    }
};

}

namespace {

typedef QHash<QString, QSharedPointer<SDynamicObjClassImpl>> TStringClassMap;
typedef QHash<QString, QByteArray> TPathDataMap;
typedef QHash<QString, SDynamicObjectShaderInfo> TShaderInfoMap;
typedef QSet<QString> TPathSet;
typedef QHash<dynamic::SDynamicShaderMapKey, TShaderAndFlags> TShaderMap;

struct SDynamicObjectSystemCoreImpl : public IDynamicObjectSystem
{
};

struct SDynamicObjectSystemImpl : public IDynamicObjectSystem
{
    IQDemonRenderContextCore *m_CoreContext;
    IQDemonRenderContext *m_Context;
    TStringClassMap m_Classes;
    TPathDataMap m_ExpandedFiles;
    QString m_ShaderKeyBuilder;
    TShaderMap m_ShaderMap;
    TShaderInfoMap m_ShaderInfoMap;
    QString m_IncludePath;
    QString m_IncludeSearch;
    QString m_VertShader;
    QString m_FragShader;
    QString m_GeometryShader;
    QString m_shaderLibraryVersion;
    QString m_shaderLibraryPlatformDirectory;
    mutable QMutex m_PropertyLoadMutex;

    SDynamicObjectSystemImpl(IQDemonRenderContextCore *inCore)
        : m_CoreContext(inCore)
        , m_Context(nullptr)
        , m_PropertyLoadMutex()
    {
        m_IncludeSearch = QStringLiteral("#include \"");
    }

    virtual ~SDynamicObjectSystemImpl()
    {
    }

    bool IsRegistered(QString inStr) override
    {
        return m_Classes.find(inStr) != m_Classes.end();
    }

    bool Register(QString inName,
                  QDemonConstDataRef<dynamic::SPropertyDeclaration> inProperties, quint32 inBaseObjectSize,
                  GraphObjectTypes::Enum inGraphObjectType) override
    {
        if (IsRegistered(inName)) {
            Q_ASSERT(false);
            return false;
        }
        QVector<dynamic::SPropertyDefinition> definitions;
        quint32 theCurrentOffset = 0;
        for (quint32 idx = 0, end = inProperties.size(); idx < end; ++idx) {
            const dynamic::SPropertyDeclaration &thePropDec = inProperties[idx];
            QString thePropName(QString::fromLocal8Bit(thePropDec.m_Name));
            quint32 propSize = getSizeofShaderDataType(thePropDec.m_DataType);
            definitions.push_back(dynamic::SPropertyDefinition(thePropName, thePropDec.m_DataType,
                                                      theCurrentOffset, propSize));
            theCurrentOffset += propSize;
            theCurrentOffset = Align(theCurrentOffset);
        }
        quint32 dataSectionSize = theCurrentOffset;
        quint32 clsSize = Align(sizeof(SDynamicObjClassImpl));
        quint32 defSize = Align(sizeof(dynamic::SPropertyDefinition) * inProperties.size());
        quint32 defaultSize = dataSectionSize;
        quint32 allocSize = clsSize + defSize + defaultSize;
        quint8 *allocData = reinterpret_cast<quint8 *>(::malloc(allocSize));
        quint8 *defData = allocData + clsSize;
        quint8 *defaultData = defData + defSize;
        dynamic::SPropertyDefinition *defPtr = reinterpret_cast<dynamic::SPropertyDefinition *>(defData);
        if (defSize)
            ::memcpy(defPtr, definitions.data(), defSize);
        if (defaultSize)
            memZero(defaultData, defaultSize);
        QSharedPointer<SDynamicObjClassImpl> theClass(new (allocData)
                SDynamicObjClassImpl(inName, toDataRef(defPtr, inProperties.size()),
                                     dataSectionSize, inBaseObjectSize, inGraphObjectType, defaultData));
        m_Classes.insert(inName, theClass);
        return true;
    }

    bool Unregister(QString inName) override {
        if (!IsRegistered(inName)) {
            Q_ASSERT(false);
            return false;
        }
        TStringClassMap::iterator iter = m_Classes.find(inName);
        if (iter != m_Classes.end())
            m_Classes.erase(iter);
        return true;
    }

    QSharedPointer<SDynamicObjClassImpl> FindClass(QString inName)
    {
        TStringClassMap::iterator iter = m_Classes.find(inName);
        if (iter != m_Classes.end())
            return iter.value();
        return nullptr;
    }

    QPair<const dynamic::SPropertyDefinition *, QSharedPointer<SDynamicObjClassImpl> >
    FindProperty(QString inName, QString inPropName)
    {
        QSharedPointer<SDynamicObjClassImpl> cls = FindClass(inName);
        if (cls) {
            const dynamic::SPropertyDefinition *def = cls->FindDefinition(inPropName);
            if (def)
                return QPair<const dynamic::SPropertyDefinition *, QSharedPointer<SDynamicObjClassImpl> >(def, cls);
        }
        return QPair<const dynamic::SPropertyDefinition *, QSharedPointer<SDynamicObjClassImpl> >(nullptr, nullptr);
    }

    void SetPropertyDefaultValue(QString inName, QString inPropName,
                                 QDemonConstDataRef<quint8> inDefaultData) override
    {
        QPair<const dynamic::SPropertyDefinition *, QSharedPointer<SDynamicObjClassImpl> > def =
                FindProperty(inName, inPropName);
        if (def.first && inDefaultData.size() >= def.first->m_ByteSize) {
            ::memcpy(def.second->m_PropertyDefaultData + def.first->m_Offset, inDefaultData.begin(),
                    def.first->m_ByteSize);
        } else {
            Q_ASSERT(false);
        }
    }

    void SetPropertyEnumNames(QString inName, QString inPropName,
                              QDemonConstDataRef<QString> inNames) override
    {

        QPair<const dynamic::SPropertyDefinition *, QSharedPointer<SDynamicObjClassImpl> > def =
                FindProperty(inName, inPropName);
        dynamic::SPropertyDefinition *theDefinitionPtr = const_cast<dynamic::SPropertyDefinition *>(def.first);
        if (theDefinitionPtr == nullptr) {
            Q_ASSERT(false);
            return;
        }
        if (theDefinitionPtr->m_EnumValueNames.size()) {
            ::free((void *)theDefinitionPtr->m_EnumValueNames.begin());
            theDefinitionPtr->m_EnumValueNames = QDemonConstDataRef<QString>();
        }
        theDefinitionPtr->m_IsEnumProperty = true;
        if (inNames.size()) {
            QString *theNameValues = new QString[inName.size()];
            ::memcpy(theNameValues, inNames.begin(), inNames.size() * sizeof(QString));
            theDefinitionPtr->m_EnumValueNames = QDemonConstDataRef<QString>(theNameValues, inNames.size());
        }
    }

    virtual QDemonConstDataRef<QString>
    GetPropertyEnumNames(QString inName, QString inPropName) const override
    {
        QPair<const dynamic::SPropertyDefinition *, QSharedPointer<SDynamicObjClassImpl> > def =
                const_cast<SDynamicObjectSystemImpl &>(*this).FindProperty(inName, inPropName);
        if (def.first)
            return def.first->m_EnumValueNames;
        return QDemonConstDataRef<QString>();
    }

    // Called during loading which is pretty heavily multithreaded.
    virtual QDemonConstDataRef<dynamic::SPropertyDefinition>
    GetProperties(QString inName) const override
    {
        QMutexLocker locker(&m_PropertyLoadMutex);
        QSharedPointer<SDynamicObjClassImpl> cls = const_cast<SDynamicObjectSystemImpl &>(*this).FindClass(inName);
        if (cls)
            return cls->m_PropertyDefinitions;
        return QDemonConstDataRef<dynamic::SPropertyDefinition>();
    }

    void SetPropertyTextureSettings(QString inName, QString inPropName,
                                    QString inPropPath,
                                    QDemonRenderTextureTypeValue::Enum inTexType,
                                    QDemonRenderTextureCoordOp::Enum inCoordOp,
                                    QDemonRenderTextureMagnifyingOp::Enum inMagFilterOp,
                                    QDemonRenderTextureMinifyingOp::Enum inMinFilterOp) override
    {
        QPair<const dynamic::SPropertyDefinition *, QSharedPointer<SDynamicObjClassImpl> > def =
                FindProperty(inName, inPropName);
        dynamic::SPropertyDefinition *theDefinitionPtr = const_cast<dynamic::SPropertyDefinition *>(def.first);
        if (theDefinitionPtr == nullptr) {
            Q_ASSERT(false);
            return;
        }
        theDefinitionPtr->m_ImagePath = inPropPath;
        theDefinitionPtr->m_TexUsageType = inTexType;
        theDefinitionPtr->m_CoordOp = inCoordOp;
        theDefinitionPtr->m_MagFilterOp = inMagFilterOp;
        theDefinitionPtr->m_MinFilterOp = inMinFilterOp;
    }

    IDynamicObjectClass *GetDynamicObjectClass(QString inName) override
    {
        // TODO: Should probably shared pointer
        return FindClass(inName).data();
    }

    void SetRenderCommands(QString inClassName,
                           QDemonConstDataRef<dynamic::SCommand *> inCommands) override
    {
        QSharedPointer<SDynamicObjClassImpl> theClass =
                const_cast<SDynamicObjectSystemImpl &>(*this).FindClass(inClassName);
        if (theClass == nullptr) {
            Q_ASSERT(false);
            return;
        }
        theClass->ReleaseCommands();
        quint32 commandAllocationSize = 0;
        for (quint32 idx = 0, end = inCommands.size(); idx < end; ++idx) {
            quint32 commandSize = Align(dynamic::SCommand::GetSizeofCommand(*inCommands[idx]));
            commandAllocationSize += commandSize;
        }
        quint32 commandPtrSize = inCommands.size() * sizeof(dynamic::SCommand *);
        quint32 totalAllocationSize = Align8(commandAllocationSize) + commandPtrSize;
        quint8 *theCommandDataBegin = (quint8 *)::malloc(totalAllocationSize);
        quint8 *theCurrentCommandData(theCommandDataBegin);
        dynamic::SCommand **theCommandPtrBegin = reinterpret_cast<dynamic::SCommand **>(theCommandDataBegin + Align8(commandAllocationSize));
        dynamic::SCommand **theCurrentCommandPtr = theCommandPtrBegin;
        memZero(theCommandDataBegin, totalAllocationSize);

        theClass->m_RequiresDepthTexture = false;
        for (quint32 idx = 0, end = inCommands.size(); idx < end; ++idx) {
            dynamic::SCommand &theCommand(*inCommands[idx]);
            quint32 theCommandSize = dynamic::SCommand::GetSizeofCommand(theCommand);
            dynamic::SCommand::CopyConstructCommand(theCurrentCommandData, theCommand);
            if (theCommand.m_Type == dynamic::CommandTypes::ApplyDepthValue)
                theClass->m_RequiresDepthTexture = true;
            if (theCommand.m_Type == dynamic::CommandTypes::BindTarget) {
                dynamic::SBindTarget *bt = reinterpret_cast<dynamic::SBindTarget *>(&theCommand);
                theClass->m_OutputFormat = bt->m_OutputFormat;
            }

            *theCurrentCommandPtr = reinterpret_cast<dynamic::SCommand *>(theCurrentCommandData);
            ++theCurrentCommandPtr;
            theCurrentCommandData += Align(theCommandSize);
        }
        Q_ASSERT(theCurrentCommandData - theCommandDataBegin == (int)commandAllocationSize);
        Q_ASSERT((quint8 *)theCurrentCommandPtr - theCommandDataBegin == (int)totalAllocationSize);
        theClass->m_RenderCommands =
                QDemonConstDataRef<dynamic::SCommand *>(theCommandPtrBegin, inCommands.size());
    }

    virtual QDemonConstDataRef<dynamic::SCommand *>
    GetRenderCommands(QString inClassName) const override
    {
        QSharedPointer<SDynamicObjClassImpl> cls =
                const_cast<SDynamicObjectSystemImpl &>(*this).FindClass(inClassName);
        if (cls)
            return cls->m_RenderCommands;
        return QDemonConstDataRef<dynamic::SCommand *>();
    }

    SDynamicObject *CreateInstance(QString inClassName) override
    {
        QSharedPointer<SDynamicObjClassImpl> theClass = FindClass(inClassName);
        if (!theClass) {
            Q_ASSERT(false);
            return nullptr;
        }
        quint32 totalObjectSize = theClass->m_BaseObjectSize + theClass->m_PropertySectionByteSize;
        SDynamicObject *retval = reinterpret_cast<SDynamicObject *>(::malloc(totalObjectSize));
        new (retval)
                SDynamicObject(theClass->m_GraphObjectType, inClassName,
                               theClass->m_PropertySectionByteSize, theClass->m_BaseObjectSize);
        ::memcpy(retval->GetDataSectionBegin(), theClass->m_PropertyDefaultData,
                theClass->m_PropertySectionByteSize);
        return retval;
    }

    void SetShaderData(QString inPath, const char *inData,
                       const char *inShaderType, const char *inShaderVersion,
                       bool inHasGeomShader, bool inIsComputeShader) override
    {
        inData = inData ? inData : "";
        auto foundIt = m_ExpandedFiles.find(inPath);
        const QByteArray newData(inData);
        if (foundIt != m_ExpandedFiles.end())
            foundIt.value() = newData;
        else
            m_ExpandedFiles.insert(inPath, newData);

        // set shader type and version if available
        if (inShaderType || inShaderVersion || inHasGeomShader || inIsComputeShader) {
            // UdoL TODO: Add this to the load / save setction
            // In addition we should merge the source code into SDynamicObjectShaderInfo as well
            SDynamicObjectShaderInfo &theShaderInfo = m_ShaderInfoMap.insert(inPath, SDynamicObjectShaderInfo()).value();
            theShaderInfo.m_Type = QString::fromLocal8Bit(nonNull(inShaderType));
            theShaderInfo.m_Version = QString::fromLocal8Bit(nonNull(inShaderVersion));
            theShaderInfo.m_HasGeomShader = inHasGeomShader;
            theShaderInfo.m_IsComputeShader = inIsComputeShader;
        }

        return;
    }

    QString GetShaderCacheKey(const char *inId, const char *inProgramMacro, const dynamic::SDynamicShaderProgramFlags &inFlags)
    {
        m_ShaderKeyBuilder = QString::fromLocal8Bit(inId);
        if (inProgramMacro && *inProgramMacro) {
            m_ShaderKeyBuilder.append("#");
            m_ShaderKeyBuilder.append(inProgramMacro);
        }
        if (inFlags.IsTessellationEnabled()) {
            m_ShaderKeyBuilder.append("#");
            m_ShaderKeyBuilder.append(TessModeValues::toString(inFlags.m_TessMode));
        }
        if (inFlags.IsGeometryShaderEnabled() && inFlags.m_WireframeMode) {
            m_ShaderKeyBuilder.append("#");
            m_ShaderKeyBuilder.append(inFlags.wireframeToString(inFlags.m_WireframeMode));
        }

        return m_ShaderKeyBuilder;
    }

    void InsertShaderHeaderInformation(QString &theReadBuffer,
                                       const char *inPathToEffect) override
    {
        DoInsertShaderHeaderInformation(theReadBuffer, inPathToEffect);
    }

    void DoInsertShaderHeaderInformation(QString &theReadBuffer,
                                         const char *inPathToEffect)
    {
        // Now do search and replace for the headers
        for (int thePos = theReadBuffer.indexOf(m_IncludeSearch); thePos != -1; thePos = theReadBuffer.indexOf(m_IncludeSearch, thePos + 1)) {
            QString::size_type theEndQuote = theReadBuffer.indexOf('\"', thePos + m_IncludeSearch.size() + 1);
            // Indicates an unterminated include file.
            if (theEndQuote == -1) {
                qCCritical(INVALID_OPERATION, "Unterminated include in file: %s", inPathToEffect);
                theReadBuffer.clear();
                break;
            }
            QString::size_type theActualBegin = thePos + m_IncludeSearch.size();
//            QString::iterator theIncludeBegin = theReadBuffer.begin() + theActualBegin;
//            QString::iterator theIncludeEnd = theReadBuffer.begin() + theEndQuote;
            // TODO: Make sure that we're off here
            QStringRef theInclude = theReadBuffer.midRef(theActualBegin, theEndQuote);
            m_IncludePath.clear();
            m_IncludePath.append(theInclude);
            // If we haven't included the file yet this round
            QString theIncludeBuffer;
            const char *theHeader = DoLoadShader(m_IncludePath.toLocal8Bit(), theIncludeBuffer);
//            quint32 theLen = (quint32)strlen(theHeader);
//            theReadBuffer = theReadBuffer.replace(theReadBuffer.begin() + thePos, theReadBuffer.begin() + theEndQuote + 1, theHeader, theLen);
            theReadBuffer = theReadBuffer.replace(thePos, theEndQuote + 1, QString::fromLatin1(theHeader));
        }
    }

    const char *DoLoadShader(const char *inPathToEffect, QString &outShaderData)
    {
        auto theInsert = m_ExpandedFiles.find(QString::fromLocal8Bit(inPathToEffect));
        const bool found = (theInsert != m_ExpandedFiles.end());
        if (found)
            *theInsert = QByteArray();
        else
            theInsert = m_ExpandedFiles.insert(QString::fromLocal8Bit(inPathToEffect), QByteArray());

        QString &theReadBuffer(outShaderData);
        if (found) {
            const QString defaultDir = m_Context->GetDynamicObjectSystem()->GetShaderCodeLibraryDirectory();
            const QString platformDir = m_Context->GetDynamicObjectSystem()->shaderCodeLibraryPlatformDirectory();
            const QString ver = m_Context->GetDynamicObjectSystem()->shaderCodeLibraryVersion();

            QString fullPath;
            QSharedPointer<QIODevice> theStream;
            if (!platformDir.isEmpty()) {
                QTextStream stream(&fullPath);
                stream << platformDir << QLatin1Char('/') << inPathToEffect;
                theStream = m_CoreContext->GetInputStreamFactory()->GetStreamForFile(fullPath.toLatin1().data());
            }

            if (theStream.isNull()) {
                fullPath.clear();
                QTextStream stream(&fullPath);
                stream << defaultDir << QLatin1Char('/') << ver << QLatin1Char('/')
                       << inPathToEffect;
                theStream = m_CoreContext->GetInputStreamFactory()->GetStreamForFile(fullPath.toLatin1().data());
                if (theStream.isNull()) {
                    fullPath.clear();
                    QTextStream stream(&fullPath);
                    stream << defaultDir << QLatin1Char('/') << inPathToEffect;
                    theStream = m_CoreContext->GetInputStreamFactory()->GetStreamForFile(fullPath.toLatin1().data());
                }
            }
            if (!theStream.isNull()) {
                quint8 readBuf[1024];
                quint32 amountRead = 0;
                do {
                    amountRead = theStream->read((char *)readBuf, 1024);
                    if (amountRead)
                        theReadBuffer.append(QByteArray::fromRawData((const char *)readBuf, amountRead));
                } while (amountRead);
            } else {
                qCCritical(INVALID_OPERATION, "Failed to find include file %s", inPathToEffect);
                Q_ASSERT(false);
            }
            theInsert.value() = theReadBuffer.toLatin1();
        } else {
            theReadBuffer = QString::fromLatin1(theInsert.value());
        }
        DoInsertShaderHeaderInformation(theReadBuffer, inPathToEffect);
        return theReadBuffer.toLocal8Bit();
    }

//    void Save(SWriteBuffer &ioBuffer, const SStrRemapMap &inRemapMap, const char *inProjectDir) const override
//    {
//        quint32 startOffset = ioBuffer.size();
//        ioBuffer.write((quint32)m_ExpandedFiles.size());
//        for (TPathDataMap::const_iterator theIter = m_ExpandedFiles.begin();
//             theIter != m_ExpandedFiles.end(); ++theIter) {
//            QString thePath(theIter->first);
//            char *theData = theIter->second;
//            theData = theData ? theData : (char *)"";
//            quint32 theLen = (quint32)strlen(theData);
//            thePath.Remap(inRemapMap);
//            ioBuffer.write(thePath);
//            ioBuffer.write(theLen + 1);
//            ioBuffer.write(theData, theLen + 1);
//            ioBuffer.align(sizeof(void *));
//        }
//        ioBuffer.write((quint32)m_Classes.size());
//        SStringSaveRemapper theRemapper(m_Allocator, inRemapMap, inProjectDir,
//                                        m_CoreContext.GetStringTable());
//        for (TStringClassMap::const_iterator iter = m_Classes.begin(), end = m_Classes.end();
//             iter != end; ++iter) {
//            const SDynamicObjClassImpl *theClass = iter->second;
//            ioBuffer.align(4);
//            quint32 objOffset = ioBuffer.size();
//            quint32 classOffset = objOffset - startOffset;
//            (void)classOffset;
//            quint32 defOffset = objOffset + sizeof(SDynamicObjClassImpl);
//            quint32 dataOffset =
//                    defOffset + theClass->m_PropertyDefinitions.size() * sizeof(SPropertyDefinition);
//            quint32 dataEnd = dataOffset + theClass->m_PropertySectionByteSize;
//            quint32 writeAmount = dataEnd - objOffset;
//            ioBuffer.write((const quint8 *)theClass, writeAmount);
//            ioBuffer.align(4);
//            quint32 cmdOffset = 0;
//            quint8 *writeCommandStart = nullptr;
//            if (theClass->m_RenderCommands.size()) {
//                // We know commands are allocated in a block.
//                const SCommand &firstCommand = *theClass->m_RenderCommands[0];
//                const quint8 *commandStart = reinterpret_cast<const quint8 *>(&firstCommand);
//                const SCommand &lastCommand(
//                            *theClass->m_RenderCommands[theClass->m_RenderCommands.size() - 1]);
//                const quint8 *commandEnd = reinterpret_cast<const quint8 *>(&lastCommand)
//                        + SCommand::GetSizeofCommand(lastCommand);
//                cmdOffset = ioBuffer.size();
//                ioBuffer.write(commandStart, (quint32)(commandEnd - commandStart));
//                // Write location of the actual storage for the command ptr array.
//                ioBuffer.writeZeros(theClass->m_RenderCommands.size() * sizeof(SCommand **));
//            }
//            ioBuffer.align(4);
//            if (cmdOffset)
//                writeCommandStart = ioBuffer.begin() + cmdOffset;

//            SDynamicObjClassImpl *writeClass =
//                    (SDynamicObjClassImpl *)(ioBuffer.begin() + objOffset);
//            writeClass->m_Id.Remap(inRemapMap);
//            writeClass->SetupThisObjectFromMemory(m_Allocator, theRemapper, writeCommandStart,
//                                                  theClass->m_RenderCommands.size());
//            for (quint32 idx = 0, end = theClass->m_PropertyDefinitions.size(); idx < end; ++idx) {
//                // Moved into the loop because writing the enumerations may resize the data buffer.
//                SPropertyDefinition *theDefinitions =
//                        (SPropertyDefinition *)(ioBuffer.begin() + defOffset);
//                theDefinitions[idx].m_Name.Remap(inRemapMap);
//                const SPropertyDefinition &theDefinition(theClass->m_PropertyDefinitions[idx]);
//                if (theDefinitions[idx].m_EnumValueNames.size()) {
//                    quint32 enumOffset = ioBuffer.size();
//                    ioBuffer.write(theDefinition.m_EnumValueNames.begin(),
//                                   theDefinition.m_EnumValueNames.size()
//                                   * sizeof(QString));
//                    QString *strPtr =
//                            (QString *)(ioBuffer.begin() + enumOffset);
//                    for (quint32 enumIdx = 0, enumEnd = theDefinition.m_EnumValueNames.size();
//                         enumIdx < enumEnd; ++enumIdx)
//                        strPtr[enumIdx].Remap(inRemapMap);
//                }
//                if (theDefinition.m_DataType == QDemonRenderShaderDataTypes::QDemonRenderTexture2DPtr) {
//                    quint8 *theDataPtr = ioBuffer.begin() + dataOffset;
//                    QString *realData =
//                            reinterpret_cast<QString *>(theDataPtr + theDefinition.m_Offset);
//                    realData->Remap(inRemapMap);
//                }
//            }
//        }

//        // Write out meta information about the shader system
//        quint32 numShaderInfos = (quint32)m_ShaderInfoMap.size();
//        ioBuffer.write(numShaderInfos);
//        for (TShaderInfoMap::const_iterator iter = m_ShaderInfoMap.begin(),
//             end = m_ShaderInfoMap.end();
//             iter != end; ++iter) {
//            QString infoName = iter->first;
//            infoName.Remap(inRemapMap);
//            ioBuffer.write(infoName);
//            const SDynamicObjectShaderInfo &theInfo = iter->second;
//            QString infoType(theInfo.m_Type);
//            QString infoVersion(theInfo.m_Version);
//            infoType.Remap(inRemapMap);
//            infoVersion.Remap(inRemapMap);
//            ioBuffer.write(infoType);
//            ioBuffer.write(infoVersion);
//        }
//    }

//    void Load(QDemonDataRef<quint8> inData, CStrTableOrDataRef inStrDataBlock,
//              const char *inProjectDir) override
//    {
//        m_Allocator.m_PreAllocatedBlock = inData;
//        m_Allocator.m_OwnsMemory = false;
//        TStr workspaceStr(ForwardingAllocator(m_Foundation.getAllocator(), "ProjPath"));
//        SDataReader theReader(inData.begin(), inData.end());
//        quint32 numMappedPaths = theReader.LoadRef<quint32>();
//        for (quint32 idx = 0, end = numMappedPaths; idx < end; ++idx) {
//            QString theStr(theReader.LoadRef<QString>());
//            quint32 theCharLen = theReader.LoadRef<quint32>();
//            char *thePathData = reinterpret_cast<char *>(theReader.m_CurrentPtr);
//            theReader.m_CurrentPtr += theCharLen;
//            theReader.Align();
//            theStr.Remap(inStrDataBlock);
//            m_ExpandedFiles.insert(theStr, thePathData);
//        }
//        SStringLoadRemapper theRemapper(m_Allocator, inStrDataBlock, inProjectDir,
//                                        m_CoreContext.GetStringTable());
//        quint32 numClasses = theReader.LoadRef<quint32>();
//        for (quint32 idx = 0, end = numClasses; idx < end; ++idx) {
//            theReader.Align(4);
//            size_t classOffset = static_cast<quint32>(theReader.m_CurrentPtr - inData.mData);
//            (void)classOffset;
//            SDynamicObjClassImpl *theClass = (SDynamicObjClassImpl *)theReader.m_CurrentPtr;
//            theClass->m_Allocator = &m_Allocator;
//            theReader.m_CurrentPtr += sizeof(SDynamicObjClassImpl);
//            SPropertyDefinition *theDefinitions = (SPropertyDefinition *)theReader.m_CurrentPtr;
//            theReader.m_CurrentPtr +=
//                    theClass->m_PropertyDefinitions.size() * sizeof(SPropertyDefinition);
//            quint8 *theDataBuffer = theReader.m_CurrentPtr;
//            theReader.m_CurrentPtr += theClass->m_PropertySectionByteSize;
//            theClass->m_Id.Remap(inStrDataBlock);
//            theClass->m_PropertyDefinitions = QDemonConstDataRef<SPropertyDefinition>(
//                        theDefinitions, theClass->m_PropertyDefinitions.size());
//            theClass->m_PropertyDefaultData = theDataBuffer;
//            theReader.Align(4);
//            quint8 *theCommandStart = theReader.m_CurrentPtr;

//            quint32 numRenderCommands = theClass->m_RenderCommands.size();
//            new (theClass) SDynamicObjClassImpl(
//                        m_Allocator, theClass->m_Id, theClass->m_PropertyDefinitions,
//                        theClass->m_PropertySectionByteSize, theClass->m_BaseObjectSize,
//                        theClass->m_GraphObjectType, theClass->m_PropertyDefaultData,
//                        theClass->m_RequiresDepthTexture, theClass->m_OutputFormat);

//            theClass->SetupThisObjectFromMemory(m_Allocator, theRemapper, theCommandStart,
//                                                numRenderCommands);

//            if (theClass->m_RenderCommands.size()) {
//                const SCommand &theLastCommand =
//                        *theClass->m_RenderCommands[theClass->m_RenderCommands.size() - 1];
//                const quint8 *theCommandEnd = reinterpret_cast<const quint8 *>(&theLastCommand)
//                        + SCommand::GetSizeofCommand(theLastCommand);
//                theReader.m_CurrentPtr = const_cast<quint8 *>(theCommandEnd);
//                theReader.m_CurrentPtr += theClass->m_RenderCommands.size() * sizeof(SCommand **);
//            }
//            theReader.Align(4);

//            for (quint32 defIdx = 0, defEnd = theClass->m_PropertyDefinitions.size(); defIdx < defEnd;
//                 ++defIdx) {
//                SPropertyDefinition &theDef(theDefinitions[defIdx]);
//                theDef.m_Name.Remap(inStrDataBlock);
//                if (theDef.m_EnumValueNames.size()) {
//                    QString *theNames = (QString *)theReader.m_CurrentPtr;
//                    theReader.m_CurrentPtr +=
//                            theDef.m_EnumValueNames.size() * sizeof(QString);
//                    theDef.m_EnumValueNames =
//                            QDemonDataRef<QString>(theNames, theDef.m_EnumValueNames.size());
//                    for (quint32 enumIdx = 0, enumEnd = theDef.m_EnumValueNames.size();
//                         enumIdx < enumEnd; ++enumIdx)
//                        theNames[enumIdx].Remap(inStrDataBlock);
//                }
//                if (theDef.m_DataType == QDemonRenderShaderDataTypes::QDemonRenderTexture2DPtr) {
//                    QString *realData =
//                            reinterpret_cast<QString *>(theDataBuffer + theDef.m_Offset);
//                    realData->Remap(inStrDataBlock);
//                }
//            }
//            m_Classes.insert(theClass->m_Id, theClass);
//        }
//        quint32 theNumShaderInfos = theReader.LoadRef<quint32>();
//        for (quint32 idx = 0, end = theNumShaderInfos; idx < end; ++idx) {
//            QString name, type, version;
//            name = theReader.LoadRef<QString>();
//            type = theReader.LoadRef<QString>();
//            version = theReader.LoadRef<QString>();
//            name.Remap(inStrDataBlock);
//            type.Remap(inStrDataBlock);
//            version.Remap(inStrDataBlock);
//            SDynamicObjectShaderInfo &theInfo =
//                    m_ShaderInfoMap.insert(name, SDynamicObjectShaderInfo())
//                    .first->second;
//            theInfo.m_Type = type;
//            theInfo.m_Version = version;
//        }
//    }

    QSharedPointer<IDynamicObjectSystem> CreateDynamicSystem(IQDemonRenderContext *rc) override
    {
        m_Context = rc;
        return sharedFromThis();
    }

    QStringList getParameters(const QString &str, int begin, int end)
    {
        const QString s = str.mid(begin, end - begin + 1);
        return s.split(",");
    }

    void insertSnapperDirectives(QString &str)
    {
        int beginIndex = 0;
        // Snapper macros:
        //  #define SNAPPER_SAMPLER2D(propName, propNiceName, texFilter, texWrap, showUI ) \
        //      uniform sampler2D propName;                                     \
        //      uniform int flag##propName;                                     \
        //      uniform vec4 propName##Info;                                    \
        //      vec4 texture2D_##propName(vec2 uv)                              \
        //      {                                                               \
        //          return GetTextureValue( propName, uv, propName##Info.z );   \
        //      }
        //
        //  #define SNAPPER_SAMPLER2DWITHDEFAULT(propName, propNiceName, texFilter, texWrap, defaultPath, showUI ) \
        //      SNAPPER_SAMPLER2D( propName, propNiceName, texFilter, texWrap, showUI )
        //
        //  #define SNAPPER_SAMPLERCUBE(propName, propNiceName, texFilter, texWrap ) \
        //      uniform samplerCube propName;   \
        //      uniform vec2 propName##UVRange; \
        //      uniform int flag##propName;     \
        //      uniform vec2 propName##Size;
        QString snapperSampler = QStringLiteral("SNAPPER_SAMPLER2D(");
        QString snapperSamplerDefault = QStringLiteral("SNAPPER_SAMPLER2DWITHDEFAULT(");
        QString snapperSamplerCube = QStringLiteral("SNAPPER_SAMPLERCUBE(");
        QString endingBracket = QStringLiteral(")");

        while ((beginIndex = str.indexOf(snapperSampler, beginIndex)) >= 0) {
            int endIndex = str.indexOf(endingBracket, beginIndex);
            const QStringList list = getParameters(str, beginIndex + snapperSampler.length(),
                                                   endIndex);
            str.remove(beginIndex, endIndex - beginIndex + 1);
            if (list.size() == 5) {
                QString insertStr;
                QTextStream stream(&insertStr);
                stream << "uniform sampler2D " << list[0] << ";\n";
                stream << "uniform int flag" << list[0] << ";\n";
                stream << "vec4 " << list[0] << "Info;\n";
                stream << "vec4 texture2D_" << list[0] << "(vec2 uv) "
                       << "{ return GetTextureValue( " << list[0] << ", uv, "
                       << list[0] <<"Info.z ); }\n";
                str.insert(beginIndex, insertStr);
            }
        }
        beginIndex = 0;
        while ((beginIndex = str.indexOf(snapperSamplerDefault, beginIndex)) >= 0) {
            int endIndex = str.indexOf(endingBracket, beginIndex);
            const QStringList list = getParameters(str, beginIndex + snapperSamplerDefault.length(),
                                                   endIndex);
            str.remove(beginIndex, endIndex - beginIndex + 1);
            if (list.size() == 5) {
                QString insertStr;
                QTextStream stream(&insertStr);
                stream << "uniform sampler2D " << list[0] << ";\n";
                stream << "uniform int flag" << list[0] << ";\n";
                stream << "vec4 " << list[0] << "Info;\n";
                stream << "vec4 texture2D_" << list[0] << "(vec2 uv) "
                       << "{ return GetTextureValue( " << list[0] << ", uv, "
                       << list[0] <<"Info.z ); }\n";
                str.insert(beginIndex, insertStr);
            }
        }
        beginIndex = 0;
        while ((beginIndex = str.indexOf(snapperSamplerCube, beginIndex)) >= 0) {
            int endIndex = str.indexOf(endingBracket, beginIndex);
            const QStringList list = getParameters(str, beginIndex + snapperSamplerCube.length(),
                                                   endIndex);
            str.remove(beginIndex, endIndex - beginIndex + 1);
            if (list.size() == 4) {
                QString insertStr;
                QTextStream stream(&insertStr);
                stream << "uniform samplerCube " << list[0] << ";\n";
                stream << "uniform vec2 "<< list[0] << "UVRange;\n";
                stream << "uniform int flag" << list[0] << ";\n";
                stream << "uniform vec2 "<< list[0] << "Size;\n";
                str.insert(beginIndex, insertStr);
            }
        }
    }

    QSharedPointer<QDemonRenderShaderProgram> CompileShader(QString inId, const char *inProgramSource,
                                                            const char *inGeomSource,
                                                            QString inProgramMacroName,
                                                            TShaderFeatureSet inFeatureSet,
                                                            const dynamic::SDynamicShaderProgramFlags &inFlags,
                                                            bool inForceCompilation = false)
    {
        m_VertShader.clear();
        m_FragShader.clear();
        m_GeometryShader.clear();
        SShaderCacheProgramFlags theFlags;

        m_VertShader.append("#define VERTEX_SHADER\n");
        m_FragShader.append("#define FRAGMENT_SHADER\n");

        if (!inProgramMacroName.isEmpty()) {
            m_VertShader.append("#define ");
            m_VertShader.append(inProgramMacroName.toLatin1());
            m_VertShader.append("\n");

            m_FragShader.append("#define ");
            m_FragShader.append(inProgramMacroName.toLatin1());
            m_FragShader.append("\n");
        }

        if (inGeomSource && inFlags.IsGeometryShaderEnabled()) {
            theFlags.SetGeometryShaderEnabled(true);

            m_GeometryShader.append("#define GEOMETRY_SHADER 1\n");
            m_GeometryShader.append(inGeomSource);

            m_VertShader.append("#define GEOMETRY_SHADER 1\n");
        } else if (inFlags.IsGeometryShaderEnabled()) {
            theFlags.SetGeometryShaderEnabled(true);
            m_GeometryShader.append("#define USER_GEOMETRY_SHADER 1\n");
            m_GeometryShader.append(inProgramSource);
            m_VertShader.append("#define GEOMETRY_SHADER 0\n");
            m_FragShader.append("#define GEOMETRY_WIREFRAME_SHADER 0\n");
        } else {
            m_VertShader.append("#define GEOMETRY_SHADER 0\n");
            m_FragShader.append("#define GEOMETRY_WIREFRAME_SHADER 0\n");
        }

        if (strstr(inProgramSource, "SNAPPER_SAMPLER")) {
            QString programSource(inProgramSource);
            insertSnapperDirectives(programSource);
            QByteArray data = programSource.toLatin1();
            const char *source = data.constData();

            m_VertShader.append(source);
            m_FragShader.append(source);
        } else {
            m_VertShader.append(inProgramSource);
            m_FragShader.append(inProgramSource);
        }

        QSharedPointer<IShaderCache> theShaderCache = m_Context->GetShaderCache();

        QString theKey = GetShaderCacheKey(inId.toLocal8Bit(), inProgramMacroName.toLocal8Bit(), inFlags);
        if (inForceCompilation) {
            return theShaderCache->ForceCompileProgram(theKey, m_VertShader.toLocal8Bit(),
                                                      m_FragShader.toLocal8Bit(), nullptr, nullptr,
                                                      m_GeometryShader.toLocal8Bit(), theFlags,
                                                      inFeatureSet, false);
        }

        return theShaderCache->CompileProgram(theKey, m_VertShader.toLocal8Bit(), m_FragShader.toLocal8Bit(),
                                             nullptr, nullptr, m_GeometryShader.toLocal8Bit(), theFlags,
                                             inFeatureSet);
    }

    // This just returns the custom material shader source without compiling
    const char *GetShaderSource(QString inPath, QString &inBuffer) override
    {
        inBuffer.clear();
        inBuffer.append("#define FRAGMENT_SHADER\n");

        const char *source = DoLoadShader(inPath.toLocal8Bit(), inBuffer);
        return source;
    }

    TShaderAndFlags GetShaderProgram(QString inPath,
                                     QString inProgramMacro,
                                     TShaderFeatureSet inFeatureSet,
                                     const dynamic::SDynamicShaderProgramFlags &inFlags,
                                     bool inForceCompilation) override
    {
        dynamic::SDynamicShaderMapKey shaderMapKey(TStrStrPair(inPath, inProgramMacro), inFeatureSet, inFlags.m_TessMode, inFlags.m_WireframeMode);
        auto theInserter = m_ShaderMap.find(shaderMapKey);
        const bool found = (theInserter != m_ShaderMap.end());

        if (!found)
            theInserter = m_ShaderMap.insert(shaderMapKey, TShaderAndFlags());

        // TODO: This looks funky (if found)...
        if (found || inForceCompilation) {
            QSharedPointer<QDemonRenderShaderProgram> theProgram = m_Context->GetShaderCache()->GetProgram(
                        GetShaderCacheKey(inPath.toLocal8Bit(), inProgramMacro.toLocal8Bit(), inFlags), inFeatureSet);
            dynamic::SDynamicShaderProgramFlags theFlags(inFlags);
            if (!theProgram || inForceCompilation) {
                SDynamicObjectShaderInfo &theShaderInfo =
                        m_ShaderInfoMap.insert(inPath, SDynamicObjectShaderInfo()).value();
                if (theShaderInfo.m_IsComputeShader == false) {
                    QString theShaderBuffer;
                    const char *programSource = DoLoadShader(inPath.toLocal8Bit(), theShaderBuffer);
                    if (theShaderInfo.m_HasGeomShader)
                        theFlags.SetGeometryShaderEnabled(true);
                    theProgram = CompileShader(inPath, programSource, nullptr, inProgramMacro, inFeatureSet, theFlags, inForceCompilation);
                } else {
                    QString theShaderBuffer;
                    const char *shaderVersionStr = "#version 430\n";
                    if ((quint32)m_Context->GetRenderContext()->GetRenderContextType()
                            == QDemonRenderContextValues::GLES3PLUS)
                        shaderVersionStr = "#version 310 es\n";
                    DoLoadShader(inPath.toLocal8Bit(), theShaderBuffer);
                    theShaderBuffer.insert(0, shaderVersionStr);
                    const char *programSource = theShaderBuffer.toLocal8Bit();
                    quint32 len = (quint32)strlen(nonNull(programSource)) + 1;
                    theProgram = m_Context->GetRenderContext()->CompileComputeSource(inPath.toLocal8Bit(), QDemonConstDataRef<qint8>((qint8 *)programSource, len)).mShader;
                }
            }
            theInserter.value() = TShaderAndFlags(theProgram, theFlags);
        }
        return theInserter.value();
    }

    TShaderAndFlags GetDepthPrepassShader(QString inPath,
                                          QString inPMacro,
                                          TShaderFeatureSet inFeatureSet) override
    {
        SDynamicObjectShaderInfo &theShaderInfo =
                m_ShaderInfoMap.insert(inPath, SDynamicObjectShaderInfo()).value();
        if (theShaderInfo.m_HasGeomShader == false)
            return TShaderAndFlags();
        // else, here we go...
        dynamic::SDynamicShaderProgramFlags theFlags;
        m_ShaderKeyBuilder = inPMacro;
        m_ShaderKeyBuilder.append(QStringLiteral("depthprepass"));

        QString theProgramMacro = m_ShaderKeyBuilder;

        const dynamic::SDynamicShaderMapKey shaderMapKey(TStrStrPair(inPath, theProgramMacro), inFeatureSet, theFlags.m_TessMode, theFlags.m_WireframeMode);
        const TShaderAndFlags shaderFlags;
        auto theInserter = m_ShaderMap.find(shaderMapKey);
        const bool found = theInserter != m_ShaderMap.end();
        if (found) {
            QSharedPointer<QDemonRenderShaderProgram> theProgram = m_Context->GetShaderCache()->GetProgram(GetShaderCacheKey(inPath.toLocal8Bit(), theProgramMacro.toLocal8Bit(), theFlags), inFeatureSet);
            dynamic::SDynamicShaderProgramFlags flags(theFlags);
            if (!theProgram) {
                QString theShaderBuffer;
                const char *geomSource = DoLoadShader(inPath.toLocal8Bit(), theShaderBuffer);
                SShaderVertexCodeGenerator vertexShader(m_Context->GetRenderContext()->GetRenderContextType());
                SShaderFragmentCodeGenerator fragmentShader(vertexShader, m_Context->GetRenderContext()->GetRenderContextType());

                vertexShader.AddAttribute("attr_pos", "vec3");
                vertexShader.AddUniform("model_view_projection", "mat4");
                vertexShader.Append("void main() {");
                vertexShader.Append("\tgl_Position = model_view_projection * vec4(attr_pos, 1.0);");
                vertexShader.Append("}");
                fragmentShader.Append("void main() {");
                fragmentShader.Append("\tfragOutput = vec4(0.0, 0.0, 0.0, 0.0);");
                fragmentShader.Append("}");
                const char *vertexSource = vertexShader.BuildShaderSource().toLocal8Bit().constData();
                const char *fragmentSource = fragmentShader.BuildShaderSource().toLocal8Bit().constData();

                QString programBuffer;
                programBuffer = QStringLiteral("#ifdef VERTEX_SHADER\n");
                programBuffer.append(vertexSource);
                programBuffer.append("\n#endif\n");
                programBuffer.append("\n#ifdef FRAGMENT_SHADER\n");
                programBuffer.append(fragmentSource);
                programBuffer.append("\n#endif");
                flags.SetGeometryShaderEnabled(true);
                theProgram = CompileShader(inPath, programBuffer.toLatin1(), geomSource,
                                           theProgramMacro, inFeatureSet, flags);
            }
            theInserter.value() = TShaderAndFlags(theProgram, flags);
        }
        return theInserter.value();
    }

    void setShaderCodeLibraryVersion(const QString &version) override
    {
        m_shaderLibraryVersion = version;
    }

    QString shaderCodeLibraryVersion() override
    {
        return m_shaderLibraryVersion;
    }

    virtual void setShaderCodeLibraryPlatformDirectory(const QString &directory) override
    {
        m_shaderLibraryPlatformDirectory = directory;
    }

    virtual QString shaderCodeLibraryPlatformDirectory() override
    {
        return m_shaderLibraryPlatformDirectory;
    }
};
}

QSharedPointer<IDynamicObjectSystemCore> IDynamicObjectSystemCore::CreateDynamicSystemCore(IQDemonRenderContextCore *rc)
{
    return QSharedPointer<SDynamicObjectSystemImpl>(new SDynamicObjectSystemImpl(rc));
}

QT_END_NAMESPACE
