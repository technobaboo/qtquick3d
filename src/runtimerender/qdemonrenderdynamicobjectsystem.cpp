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

uint qHash(const QDemonDynamicShaderMapKey &inKey)
{
    return inKey.m_hashCode;
}

quint32 QDemonCommand::getSizeofCommand(const QDemonCommand &inCommand)
{
    switch (inCommand.m_type) {
    case CommandTypes::AllocateBuffer:
        return sizeof(QDemonAllocateBuffer);
    case CommandTypes::BindBuffer:
        return sizeof(QDemonBindBuffer);
    case CommandTypes::BindTarget:
        return sizeof(QDemonBindTarget);
    case CommandTypes::BindShader:
        return sizeof(QDemonBindShader);
    case CommandTypes::Render:
        return sizeof(QDemonRender);
    case CommandTypes::ApplyBufferValue:
        return sizeof(QDemonApplyBufferValue);
    case CommandTypes::ApplyDepthValue:
        return sizeof(QDemonApplyDepthValue);
    case CommandTypes::ApplyInstanceValue:
        return sizeof(QDemonApplyInstanceValue);
    case CommandTypes::ApplyBlending:
        return sizeof(QDemonApplyBlending);
    case CommandTypes::ApplyRenderState:
        return sizeof(QDemonApplyRenderState);
    case CommandTypes::ApplyBlitFramebuffer:
        return sizeof(QDemonApplyBlitFramebuffer);
    case CommandTypes::ApplyValue:
        return sizeof(QDemonApplyValue)
                + static_cast<const QDemonApplyValue &>(inCommand).m_value.mSize;
    case CommandTypes::DepthStencil:
        return sizeof(QDemonDepthStencil);
    case CommandTypes::AllocateImage:
        return sizeof(QDemonAllocateImage);
    case CommandTypes::ApplyImageValue:
        return sizeof(QDemonApplyImageValue);
    case CommandTypes::AllocateDataBuffer:
        return sizeof(QDemonAllocateDataBuffer);
    case CommandTypes::ApplyDataBufferValue:
        return sizeof(QDemonApplyDataBufferValue);
    default:
        break;
    }
    Q_ASSERT(false);
    return 0;
}

template <typename TCommandType>
inline void CopyConstructCommandT(quint8 *inDataBuffer, const QDemonCommand &inCommand)
{
    TCommandType *theCommand = (TCommandType *)inDataBuffer;
    theCommand = new (theCommand)
            TCommandType(static_cast<const TCommandType &>(inCommand));
}

void QDemonCommand::copyConstructCommand(quint8 *inDataBuffer, const QDemonCommand &inCommand)
{
    switch (inCommand.m_type) {
    case CommandTypes::AllocateBuffer:
        CopyConstructCommandT<QDemonAllocateBuffer>(inDataBuffer, inCommand);
        break;
    case CommandTypes::BindBuffer:
        CopyConstructCommandT<QDemonBindBuffer>(inDataBuffer, inCommand);
        break;
    case CommandTypes::BindTarget:
        CopyConstructCommandT<QDemonBindTarget>(inDataBuffer, inCommand);
        break;
    case CommandTypes::BindShader:
        CopyConstructCommandT<QDemonBindShader>(inDataBuffer, inCommand);
        break;
    case CommandTypes::Render:
        CopyConstructCommandT<QDemonRender>(inDataBuffer, inCommand);
        break;
    case CommandTypes::ApplyBufferValue:
        CopyConstructCommandT<QDemonApplyBufferValue>(inDataBuffer, inCommand);
        break;
    case CommandTypes::ApplyDepthValue:
        CopyConstructCommandT<QDemonApplyDepthValue>(inDataBuffer, inCommand);
        break;
    case CommandTypes::ApplyInstanceValue:
        CopyConstructCommandT<QDemonApplyInstanceValue>(inDataBuffer, inCommand);
        break;
    case CommandTypes::ApplyBlending:
        CopyConstructCommandT<QDemonApplyBlending>(inDataBuffer, inCommand);
        break;
    case CommandTypes::ApplyRenderState:
        CopyConstructCommandT<QDemonApplyRenderState>(inDataBuffer, inCommand);
        break;
    case CommandTypes::ApplyBlitFramebuffer:
        CopyConstructCommandT<QDemonApplyBlitFramebuffer>(inDataBuffer, inCommand);
        break;
    case CommandTypes::ApplyValue: {
        CopyConstructCommandT<QDemonApplyValue>(inDataBuffer, inCommand);
        QDemonApplyValue &dest = *reinterpret_cast<QDemonApplyValue *>(inDataBuffer);
        quint8 *destMem = inDataBuffer + sizeof(QDemonApplyValue);
        const QDemonApplyValue &src = static_cast<const QDemonApplyValue &>(inCommand);
        memcpy(destMem, src.m_value.mData, src.m_value.mSize);
        dest.m_value.mData = destMem;
        break;
    }
    case CommandTypes::DepthStencil:
        CopyConstructCommandT<QDemonDepthStencil>(inDataBuffer, inCommand);
        break;
    case CommandTypes::AllocateImage:
        CopyConstructCommandT<QDemonAllocateImage>(inDataBuffer, inCommand);
        break;
    case CommandTypes::ApplyImageValue:
        CopyConstructCommandT<QDemonApplyImageValue>(inDataBuffer, inCommand);
        break;
    case CommandTypes::AllocateDataBuffer:
        CopyConstructCommandT<QDemonAllocateDataBuffer>(inDataBuffer, inCommand);
        break;
    case CommandTypes::ApplyDataBufferValue:
        CopyConstructCommandT<QDemonApplyDataBufferValue>(inDataBuffer, inCommand);
        break;
    default:
        Q_ASSERT(false);
        break;
    }
}
}

namespace {

template <typename TCommandType>
struct QDemonCommandRemapping
{
    template <typename TRemapper>
    static void remapCommandData(TCommandType &, TRemapper &)
    {
    }
};

template <>
struct QDemonCommandRemapping<dynamic::QDemonAllocateBuffer>
{
    template <typename TRemapper>
    static void remapCommandData(dynamic::QDemonAllocateBuffer &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_name);
    }
};

template <>
struct QDemonCommandRemapping<dynamic::QDemonAllocateImage>
{
    template <typename TRemapper>
    static void remapCommandData(dynamic::QDemonAllocateImage &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_name);
    }
};

template <>
struct QDemonCommandRemapping<dynamic::QDemonAllocateDataBuffer>
{
    template <typename TRemapper>
    static void remapCommandData(dynamic::QDemonAllocateDataBuffer &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_name);
        if (!cmd.m_wrapName.isEmpty())
            remapper.Remap(cmd.m_wrapName);
    }
};

template <>
struct QDemonCommandRemapping<dynamic::QDemonBindBuffer>
{
    template <typename TRemapper>
    static void remapCommandData(dynamic::QDemonBindBuffer &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_bufferName);
    }
};
template <>
struct QDemonCommandRemapping<dynamic::QDemonBindShader>
{
    template <typename TRemapper>
    static void remapCommandData(dynamic::QDemonBindShader &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_shaderPath);
        remapper.Remap(cmd.m_shaderDefine);
    }
};
template <>
struct QDemonCommandRemapping<dynamic::QDemonApplyInstanceValue>
{
    template <typename TRemapper>
    static void remapCommandData(dynamic::QDemonApplyInstanceValue &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_propertyName);
    }
};
template <>
struct QDemonCommandRemapping<dynamic::QDemonApplyBufferValue>
{
    template <typename TRemapper>
    static void remapCommandData(dynamic::QDemonApplyBufferValue &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_bufferName);
        remapper.Remap(cmd.m_paramName);
    }
};

template <>
struct QDemonCommandRemapping<dynamic::QDemonApplyDepthValue>
{
    template <typename TRemapper>
    static void remapCommandData(dynamic::QDemonApplyDepthValue &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_paramName);
    }
};

template <>
struct QDemonCommandRemapping<dynamic::QDemonApplyBlitFramebuffer>
{
    template <typename TRemapper>
    static void remapCommandData(dynamic::QDemonApplyBlitFramebuffer &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_sourceBufferName);
        remapper.Remap(cmd.m_destBufferName);
    }
};

template <>
struct QDemonCommandRemapping<dynamic::QDemonApplyValue>
{
    template <typename TRemapper>
    static void remapCommandData(dynamic::QDemonApplyValue &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_propertyName);
    }
};

template <>
struct QDemonCommandRemapping<dynamic::QDemonApplyDataBufferValue>
{
    template <typename TRemapper>
    static void remapCommandData(dynamic::QDemonApplyDataBufferValue &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_paramName);
    }
};

template <>
struct QDemonCommandRemapping<dynamic::QDemonDepthStencil>
{
    template <typename TRemapper>
    static void remapCommandData(dynamic::QDemonDepthStencil &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_bufferName);
    }
};

quint32 align(quint32 inValue)
{
    if (inValue % 4)
        return inValue + (4 - (inValue % 4));
    return inValue;
}

quint32 align8(quint32 inValue)
{
    if (inValue % 8)
        return inValue + (8 - (inValue % 8));
    return inValue;
}

inline const char *getShaderDatatypeName(QDemonRenderShaderDataTypes::Enum inValue)
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

struct QDemonDynamicObjectShaderInfo
{
    QString m_type; ///< shader type (GLSL or HLSL)
    QString m_version; ///< shader version (e.g. 330 vor GLSL)
    bool m_hasGeomShader;
    bool m_isComputeShader;

    QDemonDynamicObjectShaderInfo()
        : m_hasGeomShader(false)
        , m_isComputeShader(false)
    {
    }
    QDemonDynamicObjectShaderInfo(QString inType, QString inVersion,
                             bool inHasGeomShader, bool inIsComputeShader)
        : m_type(inType)
        , m_version(inVersion)
        , m_hasGeomShader(inHasGeomShader)
        , m_isComputeShader(inIsComputeShader)
    {
    }
};

struct QDemonDynamicObjClassImpl : public QDemonDynamicObjectClassInterface
{
    QString m_id;
    QDemonConstDataRef<dynamic::QDemonPropertyDefinition> m_propertyDefinitions;
    quint32 m_propertySectionByteSize;
    quint32 m_baseObjectSize;
    QDemonGraphObjectTypes::Enum m_graphObjectType;
    quint8 *m_propertyDefaultData;
    QDemonConstDataRef<dynamic::QDemonCommand *> m_renderCommands;
    bool m_requiresDepthTexture;
    bool m_requiresCompilation;
    QDemonRenderTextureFormats::Enum m_outputFormat;

    QDemonDynamicObjClassImpl(QString id,
                              QDemonConstDataRef<dynamic::QDemonPropertyDefinition> definitions,
                              quint32 propertySectionByteSize,
                              quint32 baseObjectSize,
                              QDemonGraphObjectTypes::Enum objectType,
                              quint8 *propDefaultData,
                              bool inRequiresDepthTexture = false,
                              QDemonRenderTextureFormats::Enum inOutputFormat = QDemonRenderTextureFormats::RGBA8)
        : m_id(id)
        , m_propertyDefinitions(definitions)
        , m_propertySectionByteSize(propertySectionByteSize)
        , m_baseObjectSize(baseObjectSize)
        , m_graphObjectType(objectType)
        , m_propertyDefaultData(propDefaultData)
        , m_requiresDepthTexture(inRequiresDepthTexture)
        , m_requiresCompilation(false)
        , m_outputFormat(inOutputFormat)
    {
    }

    ~QDemonDynamicObjClassImpl() override
    {
        if (m_propertyDefinitions.size()) {
            for (quint32 idx = 0, end = m_propertyDefinitions.size(); idx < end; ++idx) {
                if (m_propertyDefinitions[idx].enumValueNames.size()) // ### You can't free a QString like this!
                    ::free((void *)m_propertyDefinitions[idx].enumValueNames.begin());
            }
        }
        releaseCommands();
    }

    template <typename TRemapperType>
    static void remapCommand(dynamic::QDemonCommand &inCommand, TRemapperType &inRemapper)
    {
        switch (inCommand.m_type) {
        // TODO: ...
#define QDEMON_RENDER_EFFECTS_HANDLE_COMMAND_TYPES(type)                                              \
        case dynamic::CommandTypes::type:                                                                       \
    QDemonCommandRemapping<dynamic::QDemon##type>::remapCommandData(static_cast<dynamic::QDemon##type &>(inCommand),            \
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
    void setupThisObjectFromMemory(TRemapper &inRemapper, quint8 *inCommandStart, quint32 numEffectCommands)
    {
        quint8 *theCommandPtrBegin = inCommandStart;
        quint32 theCommandOffset = 0;
        for (quint32 idx = 0; idx < numEffectCommands; ++idx) {
            dynamic::QDemonCommand *theCommand = reinterpret_cast<dynamic::QDemonCommand *>(inCommandStart + theCommandOffset);
            theCommandOffset += dynamic::QDemonCommand::getSizeofCommand(*theCommand);
        }
        dynamic::QDemonCommand **theCommandPtrStart =
                reinterpret_cast<dynamic::QDemonCommand **>(theCommandPtrBegin + theCommandOffset);
        m_renderCommands = QDemonConstDataRef<dynamic::QDemonCommand *>(theCommandPtrStart, numEffectCommands);
        // Now run through the commands, fixup strings and setup the command ptrs
        theCommandOffset = 0;
        for (quint32 idx = 0; idx < numEffectCommands; ++idx) {
            dynamic::QDemonCommand *theCommand =
                    reinterpret_cast<dynamic::QDemonCommand *>(theCommandPtrBegin + theCommandOffset);
            theCommandPtrStart[idx] = theCommand;
            remapCommand(*theCommand, inRemapper);
            theCommandOffset += dynamic::QDemonCommand::getSizeofCommand(*theCommand);
        }
    }

    void releaseCommands()
    {
        if (m_renderCommands.size()) {
            ::free(const_cast<dynamic::QDemonCommand *>(*m_renderCommands.begin()));
            m_renderCommands = QDemonConstDataRef<dynamic::QDemonCommand *>();
        }
    }

    QString getId() const override { return m_id; }
    QDemonConstDataRef<dynamic::QDemonPropertyDefinition> getProperties() const override
    {
        return m_propertyDefinitions;
    }
    quint32 getPropertySectionByteSize() const override { return m_propertySectionByteSize; }
    const quint8 *getDefaultValueBuffer() const override { return m_propertyDefaultData; }
    quint32 getBaseObjectSize() const override { return m_baseObjectSize; }
    QDemonGraphObjectTypes::Enum graphObjectType() const override { return m_graphObjectType; }
    const dynamic::QDemonPropertyDefinition *FindDefinition(QString &str) const
    {
        for (quint32 idx = 0, end = m_propertyDefinitions.size(); idx < end; ++idx) {
            const dynamic::QDemonPropertyDefinition &def(m_propertyDefinitions[idx]);
            if (def.name == str)
                return &def;
        }
        return nullptr;
    }
    const dynamic::QDemonPropertyDefinition *findPropertyByName(QString inName) const override
    {
        return FindDefinition(inName);
    }
    QDemonConstDataRef<dynamic::QDemonCommand *> getRenderCommands() const override
    {
        return m_renderCommands;
    }
    bool requiresDepthTexture() const override { return m_requiresDepthTexture; }
    void setRequiresDepthTexture(bool inVal) override { m_requiresDepthTexture = inVal; }
    virtual bool requiresCompilation() const override { return m_requiresCompilation; }
    virtual void setRequiresCompilation(bool inVal) override { m_requiresCompilation = inVal; }
    QDemonRenderTextureFormats::Enum getOutputTextureFormat() const override { return m_outputFormat; }
};

struct QDemonDataRemapper
{
    template <typename TRemapper>
    void remap(quint8 *inData, dynamic::QDemonPropertyDefinition &item, TRemapper &remapper)
    {
        switch (item.dataType) {
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

typedef QHash<QString, QDemonRef<QDemonDynamicObjClassImpl>> TStringClassMap;
typedef QHash<QString, QByteArray> TPathDataMap;
typedef QHash<QString, QDemonDynamicObjectShaderInfo> TShaderInfoMap;
typedef QSet<QString> TPathSet;
typedef QHash<dynamic::QDemonDynamicShaderMapKey, TShaderAndFlags> TShaderMap;

struct QDemonDynamicObjectSystemCoreImpl : public QDemonDynamicObjectSystemInterface
{
};

struct QDemonDynamicObjectSystemImpl : public QDemonDynamicObjectSystemInterface
{
    QDemonRenderContextCoreInterface *m_coreContext;
    QDemonRenderContextInterface *m_context;
    TStringClassMap m_classes;
    TPathDataMap m_expandedFiles;
    QString m_shaderKeyBuilder;
    TShaderMap m_shaderMap;
    TShaderInfoMap m_shaderInfoMap;
    QString m_includePath;
    QString m_includeSearch;
    QString m_vertShader;
    QString m_fragShader;
    QString m_geometryShader;
    QString m_shaderLibraryVersion;
    QString m_shaderLibraryPlatformDirectory;
    mutable QMutex m_propertyLoadMutex;

    QDemonDynamicObjectSystemImpl(QDemonRenderContextCoreInterface *inCore)
        : m_coreContext(inCore)
        , m_context(nullptr)
        , m_propertyLoadMutex()
    {
        m_includeSearch = QStringLiteral("#include \"");
    }

    virtual ~QDemonDynamicObjectSystemImpl()
    {
    }

    bool isRegistered(QString inStr) override
    {
        return m_classes.find(inStr) != m_classes.end();
    }

    bool doRegister(QString inName,
                    QDemonConstDataRef<dynamic::QDemonPropertyDeclaration> inProperties,
                    quint32 inBaseObjectSize,
                    QDemonGraphObjectTypes::Enum inGraphObjectType) override
    {
        if (isRegistered(inName)) {
            Q_ASSERT(false);
            return false;
        }
        QVector<dynamic::QDemonPropertyDefinition> definitions;
        quint32 theCurrentOffset = 0;
        for (quint32 idx = 0, end = inProperties.size(); idx < end; ++idx) {
            const dynamic::QDemonPropertyDeclaration &thePropDec = inProperties[idx];
            QString thePropName(QString::fromLocal8Bit(thePropDec.name));
            quint32 propSize = getSizeofShaderDataType(thePropDec.dataType);
            definitions.push_back(dynamic::QDemonPropertyDefinition(thePropName, thePropDec.dataType,
                                                      theCurrentOffset, propSize));
            theCurrentOffset += propSize;
            theCurrentOffset = align(theCurrentOffset);
        }
        quint32 dataSectionSize = theCurrentOffset;
        quint32 clsSize = align(sizeof(QDemonDynamicObjClassImpl));
        quint32 defSize = align(sizeof(dynamic::QDemonPropertyDefinition) * inProperties.size());
        quint32 defaultSize = dataSectionSize;
        quint32 allocSize = clsSize + defSize + defaultSize;
        quint8 *allocData = reinterpret_cast<quint8 *>(::malloc(allocSize));
        quint8 *defData = allocData + clsSize;
        quint8 *defaultData = defData + defSize;
        dynamic::QDemonPropertyDefinition *defPtr = reinterpret_cast<dynamic::QDemonPropertyDefinition *>(defData);
        if (defSize)
            ::memcpy(defPtr, definitions.data(), defSize);
        if (defaultSize)
            memZero(defaultData, defaultSize);
        QDemonRef<QDemonDynamicObjClassImpl> theClass(new (allocData)
                QDemonDynamicObjClassImpl(inName, toDataRef(defPtr, inProperties.size()),
                                     dataSectionSize, inBaseObjectSize, inGraphObjectType, defaultData));
        m_classes.insert(inName, theClass);
        return true;
    }

    bool unregister(QString inName) override {
        if (!isRegistered(inName)) {
            Q_ASSERT(false);
            return false;
        }
        TStringClassMap::iterator iter = m_classes.find(inName);
        if (iter != m_classes.end())
            m_classes.erase(iter);
        return true;
    }

    QDemonRef<QDemonDynamicObjClassImpl> findClass(QString inName)
    {
        TStringClassMap::iterator iter = m_classes.find(inName);
        if (iter != m_classes.end())
            return iter.value();
        return nullptr;
    }

    QPair<const dynamic::QDemonPropertyDefinition *, QDemonRef<QDemonDynamicObjClassImpl> >
    findProperty(QString inName, QString inPropName)
    {
        QDemonRef<QDemonDynamicObjClassImpl> cls = findClass(inName);
        if (cls) {
            const dynamic::QDemonPropertyDefinition *def = cls->FindDefinition(inPropName);
            if (def)
                return QPair<const dynamic::QDemonPropertyDefinition *, QDemonRef<QDemonDynamicObjClassImpl> >(def, cls);
        }
        return QPair<const dynamic::QDemonPropertyDefinition *, QDemonRef<QDemonDynamicObjClassImpl> >(nullptr, nullptr);
    }

    void setPropertyDefaultValue(QString inName, QString inPropName,
                                 QDemonConstDataRef<quint8> inDefaultData) override
    {
        QPair<const dynamic::QDemonPropertyDefinition *, QDemonRef<QDemonDynamicObjClassImpl> > def =
                findProperty(inName, inPropName);
        if (def.first && inDefaultData.size() >= def.first->byteSize) {
            ::memcpy(def.second->m_propertyDefaultData + def.first->offset, inDefaultData.begin(),
                    def.first->byteSize);
        } else {
            Q_ASSERT(false);
        }
    }

    void setPropertyEnumNames(QString inName, QString inPropName,
                              QDemonConstDataRef<QString> inNames) override
    {

        QPair<const dynamic::QDemonPropertyDefinition *, QDemonRef<QDemonDynamicObjClassImpl> > def =
                findProperty(inName, inPropName);
        dynamic::QDemonPropertyDefinition *theDefinitionPtr = const_cast<dynamic::QDemonPropertyDefinition *>(def.first);
        if (theDefinitionPtr == nullptr) {
            Q_ASSERT(false);
            return;
        }
        if (theDefinitionPtr->enumValueNames.size()) {
            ::free((void *)theDefinitionPtr->enumValueNames.begin());
            theDefinitionPtr->enumValueNames = QDemonConstDataRef<QString>();
        }
        theDefinitionPtr->isEnumProperty = true;
        if (inNames.size()) {
            QString *theNameValues = new QString[inName.size()];
            ::memcpy(theNameValues, inNames.begin(), inNames.size() * sizeof(QString));
            theDefinitionPtr->enumValueNames = QDemonConstDataRef<QString>(theNameValues, inNames.size());
        }
    }

    virtual QDemonConstDataRef<QString> getPropertyEnumNames(QString inName,
                                                             QString inPropName) const override
    {
        QPair<const dynamic::QDemonPropertyDefinition *, QDemonRef<QDemonDynamicObjClassImpl> > def =
                const_cast<QDemonDynamicObjectSystemImpl &>(*this).findProperty(inName, inPropName);
        if (def.first)
            return def.first->enumValueNames;
        return QDemonConstDataRef<QString>();
    }

    // Called during loading which is pretty heavily multithreaded.
    virtual QDemonConstDataRef<dynamic::QDemonPropertyDefinition> getProperties(QString inName) const override
    {
        QMutexLocker locker(&m_propertyLoadMutex);
        QDemonRef<QDemonDynamicObjClassImpl> cls = const_cast<QDemonDynamicObjectSystemImpl &>(*this).findClass(inName);
        if (cls)
            return cls->m_propertyDefinitions;
        return QDemonConstDataRef<dynamic::QDemonPropertyDefinition>();
    }

    void setPropertyTextureSettings(QString inName, QString inPropName,
                                    QString inPropPath,
                                    QDemonRenderTextureTypeValue::Enum inTexType,
                                    QDemonRenderTextureCoordOp::Enum inCoordOp,
                                    QDemonRenderTextureMagnifyingOp::Enum inMagFilterOp,
                                    QDemonRenderTextureMinifyingOp::Enum inMinFilterOp) override
    {
        QPair<const dynamic::QDemonPropertyDefinition *, QDemonRef<QDemonDynamicObjClassImpl> > def =
                findProperty(inName, inPropName);
        dynamic::QDemonPropertyDefinition *theDefinitionPtr = const_cast<dynamic::QDemonPropertyDefinition *>(def.first);
        if (theDefinitionPtr == nullptr) {
            Q_ASSERT(false);
            return;
        }
        theDefinitionPtr->imagePath = inPropPath;
        theDefinitionPtr->texUsageType = inTexType;
        theDefinitionPtr->coordOp = inCoordOp;
        theDefinitionPtr->magFilterOp = inMagFilterOp;
        theDefinitionPtr->minFilterOp = inMinFilterOp;
    }

    QDemonDynamicObjectClassInterface *getDynamicObjectClass(QString inName) override
    {
        // TODO: Should probably shared pointer
        return findClass(inName).data();
    }

    void setRenderCommands(QString inClassName,
                           QDemonConstDataRef<dynamic::QDemonCommand *> inCommands) override
    {
        QDemonRef<QDemonDynamicObjClassImpl> theClass =
                const_cast<QDemonDynamicObjectSystemImpl &>(*this).findClass(inClassName);
        if (theClass == nullptr) {
            Q_ASSERT(false);
            return;
        }
        theClass->releaseCommands();
        quint32 commandAllocationSize = 0;
        for (quint32 idx = 0, end = inCommands.size(); idx < end; ++idx) {
            quint32 commandSize = align(dynamic::QDemonCommand::getSizeofCommand(*inCommands[idx]));
            commandAllocationSize += commandSize;
        }
        quint32 commandPtrSize = inCommands.size() * sizeof(dynamic::QDemonCommand *);
        quint32 totalAllocationSize = align8(commandAllocationSize) + commandPtrSize;
        quint8 *theCommandDataBegin = (quint8 *)::malloc(totalAllocationSize);
        quint8 *theCurrentCommandData(theCommandDataBegin);
        dynamic::QDemonCommand **theCommandPtrBegin = reinterpret_cast<dynamic::QDemonCommand **>(theCommandDataBegin + align8(commandAllocationSize));
        dynamic::QDemonCommand **theCurrentCommandPtr = theCommandPtrBegin;
        memZero(theCommandDataBegin, totalAllocationSize);

        theClass->m_requiresDepthTexture = false;
        for (quint32 idx = 0, end = inCommands.size(); idx < end; ++idx) {
            dynamic::QDemonCommand &theCommand(*inCommands[idx]);
            quint32 theCommandSize = dynamic::QDemonCommand::getSizeofCommand(theCommand);
            dynamic::QDemonCommand::copyConstructCommand(theCurrentCommandData, theCommand);
            if (theCommand.m_type == dynamic::CommandTypes::ApplyDepthValue)
                theClass->m_requiresDepthTexture = true;
            if (theCommand.m_type == dynamic::CommandTypes::BindTarget) {
                dynamic::QDemonBindTarget *bt = reinterpret_cast<dynamic::QDemonBindTarget *>(&theCommand);
                theClass->m_outputFormat = bt->m_outputFormat;
            }

            *theCurrentCommandPtr = reinterpret_cast<dynamic::QDemonCommand *>(theCurrentCommandData);
            ++theCurrentCommandPtr;
            theCurrentCommandData += align(theCommandSize);
        }
        Q_ASSERT(theCurrentCommandData - theCommandDataBegin == (int)commandAllocationSize);
        Q_ASSERT((quint8 *)theCurrentCommandPtr - theCommandDataBegin == (int)totalAllocationSize);
        theClass->m_renderCommands =
                QDemonConstDataRef<dynamic::QDemonCommand *>(theCommandPtrBegin, inCommands.size());
    }

    virtual QDemonConstDataRef<dynamic::QDemonCommand *> getRenderCommands(QString inClassName) const override
    {
        QDemonRef<QDemonDynamicObjClassImpl> cls =
                const_cast<QDemonDynamicObjectSystemImpl &>(*this).findClass(inClassName);
        if (cls)
            return cls->m_renderCommands;
        return QDemonConstDataRef<dynamic::QDemonCommand *>();
    }

    QDemonDynamicObject *createInstance(QString inClassName) override
    {
        QDemonRef<QDemonDynamicObjClassImpl> theClass = findClass(inClassName);
        if (!theClass) {
            Q_ASSERT(false);
            return nullptr;
        }
        quint32 totalObjectSize = theClass->m_baseObjectSize + theClass->m_propertySectionByteSize;
        QDemonDynamicObject *retval = reinterpret_cast<QDemonDynamicObject *>(::malloc(totalObjectSize));
        new (retval)
                QDemonDynamicObject(theClass->m_graphObjectType, inClassName,
                               theClass->m_propertySectionByteSize, theClass->m_baseObjectSize);
        ::memcpy(retval->getDataSectionBegin(), theClass->m_propertyDefaultData,
                theClass->m_propertySectionByteSize);
        return retval;
    }

    void setShaderData(QString inPath, const char *inData,
                       const char *inShaderType, const char *inShaderVersion,
                       bool inHasGeomShader, bool inIsComputeShader) override
    {
        inData = inData ? inData : "";
        auto foundIt = m_expandedFiles.find(inPath);
        const QByteArray newData(inData);
        if (foundIt != m_expandedFiles.end())
            foundIt.value() = newData;
        else
            m_expandedFiles.insert(inPath, newData);

        // set shader type and version if available
        if (inShaderType || inShaderVersion || inHasGeomShader || inIsComputeShader) {
            // UdoL TODO: Add this to the load / save setction
            // In addition we should merge the source code into SDynamicObjectShaderInfo as well
            QDemonDynamicObjectShaderInfo &theShaderInfo = m_shaderInfoMap.insert(inPath, QDemonDynamicObjectShaderInfo()).value();
            theShaderInfo.m_type = QString::fromLocal8Bit(nonNull(inShaderType));
            theShaderInfo.m_version = QString::fromLocal8Bit(nonNull(inShaderVersion));
            theShaderInfo.m_hasGeomShader = inHasGeomShader;
            theShaderInfo.m_isComputeShader = inIsComputeShader;
        }

        return;
    }

    QString getShaderCacheKey(const char *inId, const char *inProgramMacro, const dynamic::QDemonDynamicShaderProgramFlags &inFlags)
    {
        m_shaderKeyBuilder = QString::fromLocal8Bit(inId);
        if (inProgramMacro && *inProgramMacro) {
            m_shaderKeyBuilder.append("#");
            m_shaderKeyBuilder.append(inProgramMacro);
        }
        if (inFlags.isTessellationEnabled()) {
            m_shaderKeyBuilder.append("#");
            m_shaderKeyBuilder.append(TessModeValues::toString(inFlags.tessMode));
        }
        if (inFlags.isGeometryShaderEnabled() && inFlags.wireframeMode) {
            m_shaderKeyBuilder.append("#");
            m_shaderKeyBuilder.append(inFlags.wireframeToString(inFlags.wireframeMode));
        }

        return m_shaderKeyBuilder;
    }

    void insertShaderHeaderInformation(QString &theReadBuffer,
                                       const char *inPathToEffect) override
    {
        doInsertShaderHeaderInformation(theReadBuffer, inPathToEffect);
    }

    void doInsertShaderHeaderInformation(QString &theReadBuffer,
                                         const QString &inPathToEffect)
    {
        // Now do search and replace for the headers
        for (int thePos = theReadBuffer.indexOf(m_includeSearch); thePos != -1; thePos = theReadBuffer.indexOf(m_includeSearch, thePos + 1)) {
            int theEndQuote = theReadBuffer.indexOf('\"', thePos + m_includeSearch.size() + 1);
            // Indicates an unterminated include file.
            if (theEndQuote == -1) {
                qCCritical(INVALID_OPERATION, "Unterminated include in file: %s", qPrintable(inPathToEffect));
                theReadBuffer.clear();
                break;
            }
            int theActualBegin = thePos + m_includeSearch.size();
            QString theInclude = theReadBuffer.mid(theActualBegin, theEndQuote - theActualBegin);
            m_includePath = theInclude;
            // If we haven't included the file yet this round
            QString theHeader = doLoadShader(m_includePath);
//            quint32 theLen = (quint32)strlen(theHeader);
//            theReadBuffer = theReadBuffer.replace(theReadBuffer.begin() + thePos, theReadBuffer.begin() + theEndQuote + 1, theHeader, theLen);
            theReadBuffer = theReadBuffer.replace(thePos, (theEndQuote + 1) - thePos, theHeader);
        }
    }

    QString doLoadShader(const QString &inPathToEffect)
    {
        auto theInsert = m_expandedFiles.find(inPathToEffect);
        const bool found = (theInsert != m_expandedFiles.end());
//        if (found)
//            *theInsert = QByteArray();
//        else
//            theInsert = m_expandedFiles.insert(inPathToEffect, QByteArray());

        QString theReadBuffer;
        if (!found) {
            const QString defaultDir = m_context->getDynamicObjectSystem()->getShaderCodeLibraryDirectory();
            const QString platformDir = m_context->getDynamicObjectSystem()->shaderCodeLibraryPlatformDirectory();
            const QString ver = m_context->getDynamicObjectSystem()->shaderCodeLibraryVersion();

            QString fullPath;
            QSharedPointer<QIODevice> theStream;
            if (!platformDir.isEmpty()) {
                QTextStream stream(&fullPath);
                stream << platformDir << QLatin1Char('/') << inPathToEffect;
                theStream = m_coreContext->getInputStreamFactory()->getStreamForFile(fullPath.toLatin1().data(), true);
            }

            if (theStream.isNull()) {
                fullPath.clear();
                QTextStream stream(&fullPath);
                stream << defaultDir << QLatin1Char('/') << ver << QLatin1Char('/')
                       << inPathToEffect;
                theStream = m_coreContext->getInputStreamFactory()->getStreamForFile(fullPath.toLatin1().data(), true);
                if (theStream.isNull()) {
                    fullPath.clear();
                    QTextStream stream(&fullPath);
                    stream << defaultDir << QLatin1Char('/') << inPathToEffect;
                    theStream = m_coreContext->getInputStreamFactory()->getStreamForFile(fullPath.toLatin1().data(), false);
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
                qCCritical(INVALID_OPERATION, "Failed to find include file %s", qPrintable(inPathToEffect));
                Q_ASSERT(false);
            }
            theInsert = m_expandedFiles.insert(inPathToEffect, theReadBuffer.toLatin1());
        } else {
            theReadBuffer = QString::fromLatin1(theInsert.value());
        }
        doInsertShaderHeaderInformation(theReadBuffer, inPathToEffect);
        return theReadBuffer;
    }

//    void save(SWriteBuffer &ioBuffer, const SStrRemapMap &inRemapMap, const char *inProjectDir) const override
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

//    void load(QDemonDataRef<quint8> inData, CStrTableOrDataRef inStrDataBlock,
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

    void setContextInterface(QDemonRenderContextInterface *rc) override
    {
        m_context = rc;
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

    QDemonRef<QDemonRenderShaderProgram> compileShader(QString inId, const char *inProgramSource,
                                                            const char *inGeomSource,
                                                            QString inProgramMacroName,
                                                            TShaderFeatureSet inFeatureSet,
                                                            const dynamic::QDemonDynamicShaderProgramFlags &inFlags,
                                                            bool inForceCompilation = false)
    {
        m_vertShader.clear();
        m_fragShader.clear();
        m_geometryShader.clear();
        QDemonShaderCacheProgramFlags theFlags;

        m_vertShader.append("#define VERTEX_SHADER\n");
        m_fragShader.append("#define FRAGMENT_SHADER\n");

        if (!inProgramMacroName.isEmpty()) {
            m_vertShader.append("#define ");
            m_vertShader.append(inProgramMacroName.toLatin1());
            m_vertShader.append("\n");

            m_fragShader.append("#define ");
            m_fragShader.append(inProgramMacroName.toLatin1());
            m_fragShader.append("\n");
        }

        if (inGeomSource && inFlags.isGeometryShaderEnabled()) {
            theFlags.setGeometryShaderEnabled(true);

            m_geometryShader.append("#define GEOMETRY_SHADER 1\n");
            m_geometryShader.append(inGeomSource);

            m_vertShader.append("#define GEOMETRY_SHADER 1\n");
        } else if (inFlags.isGeometryShaderEnabled()) {
            theFlags.setGeometryShaderEnabled(true);
            m_geometryShader.append("#define USER_GEOMETRY_SHADER 1\n");
            m_geometryShader.append(inProgramSource);
            m_vertShader.append("#define GEOMETRY_SHADER 0\n");
            m_fragShader.append("#define GEOMETRY_WIREFRAME_SHADER 0\n");
        } else {
            m_vertShader.append("#define GEOMETRY_SHADER 0\n");
            m_fragShader.append("#define GEOMETRY_WIREFRAME_SHADER 0\n");
        }

        if (strstr(inProgramSource, "SNAPPER_SAMPLER")) {
            QString programSource(inProgramSource);
            insertSnapperDirectives(programSource);
            QByteArray data = programSource.toLatin1();
            const char *source = data.constData();

            m_vertShader.append(source);
            m_fragShader.append(source);
        } else {
            m_vertShader.append(inProgramSource);
            m_fragShader.append(inProgramSource);
        }

        QDemonRef<QDemonShaderCacheInterface> theShaderCache = m_context->getShaderCache();

        QString theKey = getShaderCacheKey(inId.toLocal8Bit(), inProgramMacroName.toLocal8Bit(), inFlags);
        if (inForceCompilation) {
            return theShaderCache->forceCompileProgram(theKey, m_vertShader.toLocal8Bit(),
                                                      m_fragShader.toLocal8Bit(), nullptr, nullptr,
                                                      m_geometryShader.toLocal8Bit(), theFlags,
                                                      inFeatureSet, false);
        }

        return theShaderCache->compileProgram(theKey, m_vertShader.toLocal8Bit(), m_fragShader.toLocal8Bit(),
                                             nullptr, nullptr, m_geometryShader.toLocal8Bit(), theFlags,
                                             inFeatureSet);
    }

    // This just returns the custom material shader source without compiling
    QString getShaderSource(QString inPath) override
    {
        QString source;
        source.append(QStringLiteral("#define FRAGMENT_SHADER\n"));

        source.append(doLoadShader(inPath));
        return source;
    }

    TShaderAndFlags getShaderProgram(QString inPath,
                                     QString inProgramMacro,
                                     TShaderFeatureSet inFeatureSet,
                                     const dynamic::QDemonDynamicShaderProgramFlags &inFlags,
                                     bool inForceCompilation) override
    {
        dynamic::QDemonDynamicShaderMapKey shaderMapKey(TStrStrPair(inPath, inProgramMacro), inFeatureSet, inFlags.tessMode, inFlags.wireframeMode);
        auto theInserter = m_shaderMap.find(shaderMapKey);
        const bool found = (theInserter != m_shaderMap.end());

        if (!found)
            theInserter = m_shaderMap.insert(shaderMapKey, TShaderAndFlags());

        // TODO: This looks funky (if found)...
        if (found || inForceCompilation) {
            QDemonRef<QDemonRenderShaderProgram> theProgram = m_context->getShaderCache()->getProgram(getShaderCacheKey(inPath.toLocal8Bit(), inProgramMacro.toLocal8Bit(), inFlags), inFeatureSet);
            dynamic::QDemonDynamicShaderProgramFlags theFlags(inFlags);
            if (!theProgram || inForceCompilation) {
                QDemonDynamicObjectShaderInfo &theShaderInfo =
                        m_shaderInfoMap.insert(inPath, QDemonDynamicObjectShaderInfo()).value();
                if (theShaderInfo.m_isComputeShader == false) {
                    QString programSource = doLoadShader(inPath);
                    if (theShaderInfo.m_hasGeomShader)
                        theFlags.setGeometryShaderEnabled(true);
                    theProgram = compileShader(inPath, programSource.toLocal8Bit().constData(), nullptr, inProgramMacro, inFeatureSet, theFlags, inForceCompilation);
                } else {
                    QString theShaderBuffer;
                    QString shaderVersionStr = QStringLiteral("#version 430\n");
                    if ((quint32)m_context->getRenderContext()->getRenderContextType()
                            == QDemonRenderContextValues::GLES3PLUS)
                        shaderVersionStr = QStringLiteral("#version 310 es\n");
                    theShaderBuffer = doLoadShader(inPath);
                    theShaderBuffer.insert(0, shaderVersionStr);
                    const char *programSource = theShaderBuffer.toLocal8Bit();
                    quint32 len = (quint32)strlen(nonNull(programSource)) + 1;
                    theProgram = m_context->getRenderContext()->compileComputeSource(inPath.toLocal8Bit(), QDemonConstDataRef<qint8>((qint8 *)programSource, len)).m_shader;
                }
            }
            theInserter.value() = TShaderAndFlags(theProgram, theFlags);
        }
        return theInserter.value();
    }

    TShaderAndFlags getDepthPrepassShader(QString inPath,
                                          QString inPMacro,
                                          TShaderFeatureSet inFeatureSet) override
    {
        QDemonDynamicObjectShaderInfo &theShaderInfo =
                m_shaderInfoMap.insert(inPath, QDemonDynamicObjectShaderInfo()).value();
        if (theShaderInfo.m_hasGeomShader == false)
            return TShaderAndFlags();
        // else, here we go...
        dynamic::QDemonDynamicShaderProgramFlags theFlags;
        m_shaderKeyBuilder = inPMacro;
        m_shaderKeyBuilder.append(QStringLiteral("depthprepass"));

        QString theProgramMacro = m_shaderKeyBuilder;

        const dynamic::QDemonDynamicShaderMapKey shaderMapKey(TStrStrPair(inPath, theProgramMacro), inFeatureSet, theFlags.tessMode, theFlags.wireframeMode);
        const TShaderAndFlags shaderFlags;
        auto theInserter = m_shaderMap.find(shaderMapKey);
        const bool found = theInserter != m_shaderMap.end();
        if (found) {
            QDemonRef<QDemonRenderShaderProgram> theProgram = m_context->getShaderCache()->getProgram(getShaderCacheKey(inPath.toLocal8Bit(), theProgramMacro.toLocal8Bit(), theFlags), inFeatureSet);
            dynamic::QDemonDynamicShaderProgramFlags flags(theFlags);
            if (!theProgram) {
                QString geomSource = doLoadShader(inPath);
                QDemonShaderVertexCodeGenerator vertexShader(m_context->getRenderContext()->getRenderContextType());
                QDemonShaderFragmentCodeGenerator fragmentShader(vertexShader, m_context->getRenderContext()->getRenderContextType());

                vertexShader.addAttribute("attr_pos", "vec3");
                vertexShader.addUniform("model_view_projection", "mat4");
                vertexShader.append("void main() {");
                vertexShader.append("\tgl_Position = model_view_projection * vec4(attr_pos, 1.0);");
                vertexShader.append("}");
                fragmentShader.append("void main() {");
                fragmentShader.append("\tfragOutput = vec4(0.0, 0.0, 0.0, 0.0);");
                fragmentShader.append("}");
                const char *vertexSource = vertexShader.buildShaderSource().toLocal8Bit().constData();
                const char *fragmentSource = fragmentShader.buildShaderSource().toLocal8Bit().constData();

                QString programBuffer;
                programBuffer = QStringLiteral("#ifdef VERTEX_SHADER\n");
                programBuffer.append(vertexSource);
                programBuffer.append("\n#endif\n");
                programBuffer.append("\n#ifdef FRAGMENT_SHADER\n");
                programBuffer.append(fragmentSource);
                programBuffer.append("\n#endif");
                flags.setGeometryShaderEnabled(true);
                theProgram = compileShader(inPath, programBuffer.toLatin1(), geomSource.toLocal8Bit().constData(),
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

QDemonRef<QDemonDynamicObjectSystemInterface> QDemonDynamicObjectSystemInterface::createDynamicSystem(QDemonRenderContextCoreInterface *rc)
{
    return QDemonRef<QDemonDynamicObjectSystemImpl>(new QDemonDynamicObjectSystemImpl(rc));
}

QDemonDynamicObjectSystemInterface::~QDemonDynamicObjectSystemInterface()
{

}

QT_END_NAMESPACE
