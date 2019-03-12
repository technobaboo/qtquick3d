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
    case CommandType::AllocateBuffer:
        return sizeof(QDemonAllocateBuffer);
    case CommandType::BindBuffer:
        return sizeof(QDemonBindBuffer);
    case CommandType::BindTarget:
        return sizeof(QDemonBindTarget);
    case CommandType::BindShader:
        return sizeof(QDemonBindShader);
    case CommandType::Render:
        return sizeof(QDemonRender);
    case CommandType::ApplyBufferValue:
        return sizeof(QDemonApplyBufferValue);
    case CommandType::ApplyDepthValue:
        return sizeof(QDemonApplyDepthValue);
    case CommandType::ApplyInstanceValue:
        return sizeof(QDemonApplyInstanceValue);
    case CommandType::ApplyBlending:
        return sizeof(QDemonApplyBlending);
    case CommandType::ApplyRenderState:
        return sizeof(QDemonApplyRenderState);
    case CommandType::ApplyBlitFramebuffer:
        return sizeof(QDemonApplyBlitFramebuffer);
    case CommandType::ApplyValue:
        return sizeof(QDemonApplyValue) + static_cast<const QDemonApplyValue &>(inCommand).m_value.mSize;
    case CommandType::DepthStencil:
        return sizeof(QDemonDepthStencil);
    case CommandType::AllocateImage:
        return sizeof(QDemonAllocateImage);
    case CommandType::ApplyImageValue:
        return sizeof(QDemonApplyImageValue);
    case CommandType::AllocateDataBuffer:
        return sizeof(QDemonAllocateDataBuffer);
    case CommandType::ApplyDataBufferValue:
        return sizeof(QDemonApplyDataBufferValue);
    default:
        break;
    }
    Q_ASSERT(false);
    return 0;
}

template<typename TCommandType>
inline void CopyConstructCommandT(quint8 *inDataBuffer, const QDemonCommand &inCommand)
{
    TCommandType *theCommand = (TCommandType *)inDataBuffer;
    theCommand = new (theCommand) TCommandType(static_cast<const TCommandType &>(inCommand));
}

void QDemonCommand::copyConstructCommand(quint8 *inDataBuffer, const QDemonCommand &inCommand)
{
    switch (inCommand.m_type) {
    case CommandType::AllocateBuffer:
        CopyConstructCommandT<QDemonAllocateBuffer>(inDataBuffer, inCommand);
        break;
    case CommandType::BindBuffer:
        CopyConstructCommandT<QDemonBindBuffer>(inDataBuffer, inCommand);
        break;
    case CommandType::BindTarget:
        CopyConstructCommandT<QDemonBindTarget>(inDataBuffer, inCommand);
        break;
    case CommandType::BindShader:
        CopyConstructCommandT<QDemonBindShader>(inDataBuffer, inCommand);
        break;
    case CommandType::Render:
        CopyConstructCommandT<QDemonRender>(inDataBuffer, inCommand);
        break;
    case CommandType::ApplyBufferValue:
        CopyConstructCommandT<QDemonApplyBufferValue>(inDataBuffer, inCommand);
        break;
    case CommandType::ApplyDepthValue:
        CopyConstructCommandT<QDemonApplyDepthValue>(inDataBuffer, inCommand);
        break;
    case CommandType::ApplyInstanceValue:
        CopyConstructCommandT<QDemonApplyInstanceValue>(inDataBuffer, inCommand);
        break;
    case CommandType::ApplyBlending:
        CopyConstructCommandT<QDemonApplyBlending>(inDataBuffer, inCommand);
        break;
    case CommandType::ApplyRenderState:
        CopyConstructCommandT<QDemonApplyRenderState>(inDataBuffer, inCommand);
        break;
    case CommandType::ApplyBlitFramebuffer:
        CopyConstructCommandT<QDemonApplyBlitFramebuffer>(inDataBuffer, inCommand);
        break;
    case CommandType::ApplyValue: {
        CopyConstructCommandT<QDemonApplyValue>(inDataBuffer, inCommand);
        QDemonApplyValue &dest = *reinterpret_cast<QDemonApplyValue *>(inDataBuffer);
        quint8 *destMem = inDataBuffer + sizeof(QDemonApplyValue);
        const QDemonApplyValue &src = static_cast<const QDemonApplyValue &>(inCommand);
        memcpy(destMem, src.m_value.mData, src.m_value.mSize);
        dest.m_value.mData = destMem;
        break;
    }
    case CommandType::DepthStencil:
        CopyConstructCommandT<QDemonDepthStencil>(inDataBuffer, inCommand);
        break;
    case CommandType::AllocateImage:
        CopyConstructCommandT<QDemonAllocateImage>(inDataBuffer, inCommand);
        break;
    case CommandType::ApplyImageValue:
        CopyConstructCommandT<QDemonApplyImageValue>(inDataBuffer, inCommand);
        break;
    case CommandType::AllocateDataBuffer:
        CopyConstructCommandT<QDemonAllocateDataBuffer>(inDataBuffer, inCommand);
        break;
    case CommandType::ApplyDataBufferValue:
        CopyConstructCommandT<QDemonApplyDataBufferValue>(inDataBuffer, inCommand);
        break;
    default:
        Q_ASSERT(false);
        break;
    }
}
}

namespace {

template<typename TCommandType>
struct QDemonCommandRemapping
{
    template<typename TRemapper>
    static void remapCommandData(TCommandType &, TRemapper &)
    {
    }
};

template<>
struct QDemonCommandRemapping<dynamic::QDemonAllocateBuffer>
{
    template<typename TRemapper>
    static void remapCommandData(dynamic::QDemonAllocateBuffer &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_name);
    }
};

template<>
struct QDemonCommandRemapping<dynamic::QDemonAllocateImage>
{
    template<typename TRemapper>
    static void remapCommandData(dynamic::QDemonAllocateImage &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_name);
    }
};

template<>
struct QDemonCommandRemapping<dynamic::QDemonAllocateDataBuffer>
{
    template<typename TRemapper>
    static void remapCommandData(dynamic::QDemonAllocateDataBuffer &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_name);
        if (!cmd.m_wrapName.isEmpty())
            remapper.Remap(cmd.m_wrapName);
    }
};

template<>
struct QDemonCommandRemapping<dynamic::QDemonBindBuffer>
{
    template<typename TRemapper>
    static void remapCommandData(dynamic::QDemonBindBuffer &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_bufferName);
    }
};
template<>
struct QDemonCommandRemapping<dynamic::QDemonBindShader>
{
    template<typename TRemapper>
    static void remapCommandData(dynamic::QDemonBindShader &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_shaderPath);
        remapper.Remap(cmd.m_shaderDefine);
    }
};
template<>
struct QDemonCommandRemapping<dynamic::QDemonApplyInstanceValue>
{
    template<typename TRemapper>
    static void remapCommandData(dynamic::QDemonApplyInstanceValue &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_propertyName);
    }
};
template<>
struct QDemonCommandRemapping<dynamic::QDemonApplyBufferValue>
{
    template<typename TRemapper>
    static void remapCommandData(dynamic::QDemonApplyBufferValue &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_bufferName);
        remapper.Remap(cmd.m_paramName);
    }
};

template<>
struct QDemonCommandRemapping<dynamic::QDemonApplyDepthValue>
{
    template<typename TRemapper>
    static void remapCommandData(dynamic::QDemonApplyDepthValue &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_paramName);
    }
};

template<>
struct QDemonCommandRemapping<dynamic::QDemonApplyBlitFramebuffer>
{
    template<typename TRemapper>
    static void remapCommandData(dynamic::QDemonApplyBlitFramebuffer &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_sourceBufferName);
        remapper.Remap(cmd.m_destBufferName);
    }
};

template<>
struct QDemonCommandRemapping<dynamic::QDemonApplyValue>
{
    template<typename TRemapper>
    static void remapCommandData(dynamic::QDemonApplyValue &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_propertyName);
    }
};

template<>
struct QDemonCommandRemapping<dynamic::QDemonApplyDataBufferValue>
{
    template<typename TRemapper>
    static void remapCommandData(dynamic::QDemonApplyDataBufferValue &cmd, TRemapper &remapper)
    {
        remapper.Remap(cmd.m_paramName);
    }
};

template<>
struct QDemonCommandRemapping<dynamic::QDemonDepthStencil>
{
    template<typename TRemapper>
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

inline const char *getShaderDatatypeName(QDemonRenderShaderDataType inValue)
{
    switch (inValue) {
    case QDemonRenderShaderDataType::Unknown:
        return "";
    case QDemonRenderShaderDataType::Integer:
        return "qint32";
    case QDemonRenderShaderDataType::IntegerVec2:
        return "qint32_2";
    case QDemonRenderShaderDataType::IntegerVec3:
        return "qint32_3";
    case QDemonRenderShaderDataType::IntegerVec4:
        return "qint32_4";
    case QDemonRenderShaderDataType::Boolean:
        return "bool";
    case QDemonRenderShaderDataType::BooleanVec2:
        return "bool_2";
    case QDemonRenderShaderDataType::BooleanVec3:
        return "bool_3";
    case QDemonRenderShaderDataType::BooleanVec4:
        return "bool_4";
    case QDemonRenderShaderDataType::Float:
        return "float";
    case QDemonRenderShaderDataType::Vec2:
        return "QVector2D";
    case QDemonRenderShaderDataType::Vec3:
        return "QVector3D";
    case QDemonRenderShaderDataType::Vec4:
        return "QVector4D";
    case QDemonRenderShaderDataType::UnsignedInteger:
        return "quint32";
    case QDemonRenderShaderDataType::UnsignedIntegerVec2:
        return "quint32_2";
    case QDemonRenderShaderDataType::UnsignedIntegerVec3:
        return "quint32_3";
    case QDemonRenderShaderDataType::UnsignedIntegerVec4:
        return "quint32_4";
    case QDemonRenderShaderDataType::Matrix3x3:
        return "QMatrix3x3";
    case QDemonRenderShaderDataType::Matrix4x4:
        return "QMatrix4x4";
    case QDemonRenderShaderDataType::Texture2D:
        return "QDemonRenderTexture2DPtr";
    case QDemonRenderShaderDataType::Texture2DHandle:
        return "QDemonRenderTexture2DHandle";
    case QDemonRenderShaderDataType::Texture2DArray:
        return "QDemonRenderTexture2DArrayPtr";
    case QDemonRenderShaderDataType::TextureCube:
        return "QDemonRenderTextureCubePtr";
    case QDemonRenderShaderDataType::TextureCubeHandle:
        return "QDemonRenderTextureCubeHandle";
    case QDemonRenderShaderDataType::Image2D:
        return "QDemonRenderImage2DPtr";
    case QDemonRenderShaderDataType::DataBuffer:
        return "QDemonRenderDataBufferPtr";
    }
    Q_ASSERT(false);
    return "";
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

struct QDemonDynamicObjClassImpl : public QDemonDynamicObjectClassInterface
{
    QString m_id;
    QDemonConstDataRef<dynamic::QDemonPropertyDefinition> m_propertyDefinitions;
    quint32 m_propertySectionByteSize;
    quint32 m_baseObjectSize;
    QDemonRenderGraphObject::Type m_graphObjectType;
    quint8 *m_propertyDefaultData;
    QDemonConstDataRef<dynamic::QDemonCommand *> m_renderCommands;
    bool m_requiresDepthTexture;
    bool m_requiresCompilation;
    QDemonRenderTextureFormat m_outputFormat;

    QDemonDynamicObjClassImpl(QString id,
                              QDemonConstDataRef<dynamic::QDemonPropertyDefinition> definitions,
                              quint32 propertySectionByteSize,
                              quint32 baseObjectSize,
                              QDemonRenderGraphObject::Type objectType,
                              quint8 *propDefaultData,
                              bool inRequiresDepthTexture = false,
                              QDemonRenderTextureFormat inOutputFormat = QDemonRenderTextureFormat::RGBA8)
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

    template<typename TRemapperType>
    static void remapCommand(dynamic::QDemonCommand &inCommand, TRemapperType &inRemapper)
    {
        switch (inCommand.m_type) {
        case dynamic::CommandType::AllocateBuffer:
            QDemonCommandRemapping<dynamic::QDemonAllocateBuffer>::remapCommandData(static_cast<dynamic::QDemonAllocateBuffer &>(inCommand),
                                                                                    inRemapper);
            break;
        case dynamic::CommandType::BindTarget:
            QDemonCommandRemapping<dynamic::QDemonBindTarget>::remapCommandData(static_cast<dynamic::QDemonBindTarget &>(inCommand),
                                                                                inRemapper);
            break;
        case dynamic::CommandType::BindBuffer:
            QDemonCommandRemapping<dynamic::QDemonBindBuffer>::remapCommandData(static_cast<dynamic::QDemonBindBuffer &>(inCommand),
                                                                                inRemapper);
            break;
        case dynamic::CommandType::BindShader:
            QDemonCommandRemapping<dynamic::QDemonBindShader>::remapCommandData(static_cast<dynamic::QDemonBindShader &>(inCommand),
                                                                                inRemapper);
            break;
        case dynamic::CommandType::ApplyInstanceValue:
            QDemonCommandRemapping<dynamic::QDemonApplyInstanceValue>::remapCommandData(static_cast<dynamic::QDemonApplyInstanceValue &>(inCommand),
                                                                                        inRemapper);
            break;
        case dynamic::CommandType::ApplyBufferValue:
            QDemonCommandRemapping<dynamic::QDemonApplyBufferValue>::remapCommandData(static_cast<dynamic::QDemonApplyBufferValue &>(inCommand),
                                                                                      inRemapper);
            break;
        case dynamic::CommandType::ApplyDepthValue:
            QDemonCommandRemapping<dynamic::QDemonApplyDepthValue>::remapCommandData(static_cast<dynamic::QDemonApplyDepthValue &>(inCommand),
                                                                                     inRemapper);
            break;
        case dynamic::CommandType::Render:
            QDemonCommandRemapping<dynamic::QDemonRender>::remapCommandData(static_cast<dynamic::QDemonRender &>(inCommand), inRemapper);
            break;
        case dynamic::CommandType::ApplyBlending:
            QDemonCommandRemapping<dynamic::QDemonApplyBlending>::remapCommandData(static_cast<dynamic::QDemonApplyBlending &>(inCommand),
                                                                                   inRemapper);
            break;
        case dynamic::CommandType::ApplyRenderState:
            QDemonCommandRemapping<dynamic::QDemonApplyRenderState>::remapCommandData(static_cast<dynamic::QDemonApplyRenderState &>(inCommand),
                                                                                      inRemapper);
            break;
        case dynamic::CommandType::ApplyBlitFramebuffer:
            QDemonCommandRemapping<dynamic::QDemonApplyBlitFramebuffer>::remapCommandData(static_cast<dynamic::QDemonApplyBlitFramebuffer &>(inCommand),
                                                                                          inRemapper);
            break;
        case dynamic::CommandType::ApplyValue:
            QDemonCommandRemapping<dynamic::QDemonApplyValue>::remapCommandData(static_cast<dynamic::QDemonApplyValue &>(inCommand),
                                                                                inRemapper);
            break;
        case dynamic::CommandType::DepthStencil:
            QDemonCommandRemapping<dynamic::QDemonDepthStencil>::remapCommandData(static_cast<dynamic::QDemonDepthStencil &>(inCommand),
                                                                                  inRemapper);
            break;
        case dynamic::CommandType::AllocateImage:
            QDemonCommandRemapping<dynamic::QDemonAllocateImage>::remapCommandData(static_cast<dynamic::QDemonAllocateImage &>(inCommand),
                                                                                   inRemapper);
            break;
        case dynamic::CommandType::ApplyImageValue:
            QDemonCommandRemapping<dynamic::QDemonApplyImageValue>::remapCommandData(static_cast<dynamic::QDemonApplyImageValue &>(inCommand),
                                                                                     inRemapper);
            break;
        case dynamic::CommandType::AllocateDataBuffer:
            QDemonCommandRemapping<dynamic::QDemonAllocateDataBuffer>::remapCommandData(static_cast<dynamic::QDemonAllocateDataBuffer &>(inCommand),
                                                                                        inRemapper);
            break;
        case dynamic::CommandType::ApplyDataBufferValue:
            QDemonCommandRemapping<dynamic::QDemonApplyDataBufferValue>::remapCommandData(static_cast<dynamic::QDemonApplyDataBufferValue &>(inCommand),
                                                                                          inRemapper);
            break;
        default:
            Q_ASSERT(false);
            break;
        }
    }
    template<typename TRemapper>
    void setupThisObjectFromMemory(TRemapper &inRemapper, quint8 *inCommandStart, quint32 numEffectCommands)
    {
        quint8 *theCommandPtrBegin = inCommandStart;
        quint32 theCommandOffset = 0;
        for (quint32 idx = 0; idx < numEffectCommands; ++idx) {
            dynamic::QDemonCommand *theCommand = reinterpret_cast<dynamic::QDemonCommand *>(inCommandStart + theCommandOffset);
            theCommandOffset += dynamic::QDemonCommand::getSizeofCommand(*theCommand);
        }
        dynamic::QDemonCommand **theCommandPtrStart = reinterpret_cast<dynamic::QDemonCommand **>(theCommandPtrBegin + theCommandOffset);
        m_renderCommands = QDemonConstDataRef<dynamic::QDemonCommand *>(theCommandPtrStart, numEffectCommands);
        // Now run through the commands, fixup strings and setup the command ptrs
        theCommandOffset = 0;
        for (quint32 idx = 0; idx < numEffectCommands; ++idx) {
            dynamic::QDemonCommand *theCommand = reinterpret_cast<dynamic::QDemonCommand *>(theCommandPtrBegin + theCommandOffset);
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
    QDemonRenderGraphObject::Type graphObjectType() const override { return m_graphObjectType; }
    const dynamic::QDemonPropertyDefinition *findDefinition(QString &str) const
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
        return findDefinition(inName);
    }
    QDemonConstDataRef<dynamic::QDemonCommand *> getRenderCommands() const override { return m_renderCommands; }
    bool requiresDepthTexture() const override { return m_requiresDepthTexture; }
    void setRequiresDepthTexture(bool inVal) override { m_requiresDepthTexture = inVal; }
    virtual bool requiresCompilation() const override { return m_requiresCompilation; }
    virtual void setRequiresCompilation(bool inVal) override { m_requiresCompilation = inVal; }
    QDemonRenderTextureFormat getOutputTextureFormat() const override { return m_outputFormat; }
};
}

namespace {

typedef QHash<QString, QDemonRef<QDemonDynamicObjClassImpl>> TStringClassMap;
typedef QHash<QString, QByteArray> TPathDataMap;
typedef QHash<QString, QDemonDynamicObjectShaderInfo> TShaderInfoMap;
typedef QSet<QString> TPathSet;
typedef QHash<dynamic::QDemonDynamicShaderMapKey, TShaderAndFlags> TShaderMap;

static const char *includeSearch = "#include \"";

struct QDemonDynamicObjectSystemImpl : public QDemonDynamicObjectSystemInterface
{
    QDemonRenderContextCoreInterface *m_coreContext;
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

    QDemonDynamicObjectSystemImpl(QDemonRenderContextCoreInterface *inCore)
        : m_coreContext(inCore), m_context(nullptr), m_propertyLoadMutex()
    {
    }

    virtual ~QDemonDynamicObjectSystemImpl() {}

    bool isRegistered(QString inStr) override { return m_classes.find(inStr) != m_classes.end(); }

    bool doRegister(QString inName,
                    QDemonConstDataRef<dynamic::QDemonPropertyDeclaration> inProperties,
                    quint32 inBaseObjectSize,
                    QDemonRenderGraphObject::Type inGraphObjectType) override
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
            quint32 propSize = dynamic::getSizeofShaderDataType(thePropDec.dataType);
            definitions.push_back(dynamic::QDemonPropertyDefinition(thePropName, thePropDec.dataType, theCurrentOffset, propSize));
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
            memset(defaultData, 0, defaultSize);
        QDemonRef<QDemonDynamicObjClassImpl> theClass(
                new (allocData)
                        QDemonDynamicObjClassImpl(inName, toDataRef(defPtr, inProperties.size()), dataSectionSize, inBaseObjectSize, inGraphObjectType, defaultData));
        m_classes.insert(inName, theClass);
        return true;
    }

    bool unregister(QString inName) override
    {
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

    QPair<const dynamic::QDemonPropertyDefinition *, QDemonRef<QDemonDynamicObjClassImpl>> findProperty(QString inName, QString inPropName)
    {
        QDemonRef<QDemonDynamicObjClassImpl> cls = findClass(inName);
        if (cls) {
            const dynamic::QDemonPropertyDefinition *def = cls->findDefinition(inPropName);
            if (def)
                return QPair<const dynamic::QDemonPropertyDefinition *, QDemonRef<QDemonDynamicObjClassImpl>>(def, cls);
        }
        return QPair<const dynamic::QDemonPropertyDefinition *, QDemonRef<QDemonDynamicObjClassImpl>>(nullptr, nullptr);
    }

    void setPropertyDefaultValue(const QString &inName, const QString &inPropName, const QDemonConstDataRef<quint8> &inDefaultData) override
    {
        QPair<const dynamic::QDemonPropertyDefinition *, QDemonRef<QDemonDynamicObjClassImpl>> def = findProperty(inName, inPropName);
        if (def.first && inDefaultData.size() >= qint32(def.first->byteSize)) {
            ::memcpy(def.second->m_propertyDefaultData + def.first->offset, inDefaultData.begin(), def.first->byteSize);
        } else {
            Q_ASSERT(false);
        }
    }

    void setPropertyEnumNames(const QString &inName, const QString &inPropName, const QDemonConstDataRef<QString> &inNames) override
    {

        QPair<const dynamic::QDemonPropertyDefinition *, QDemonRef<QDemonDynamicObjClassImpl>> def = findProperty(inName, inPropName);
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
            // TODO:
            QString *theNameValues = new QString[inName.size()];
            ::memcpy(theNameValues, inNames.begin(), inNames.size() * sizeof(QString));
            theDefinitionPtr->enumValueNames = QDemonConstDataRef<QString>(theNameValues, inNames.size());
        }
    }

    virtual QDemonConstDataRef<QString> getPropertyEnumNames(const QString &inName, const QString &inPropName) const override
    {
        QPair<const dynamic::QDemonPropertyDefinition *, QDemonRef<QDemonDynamicObjClassImpl>>
                def = const_cast<QDemonDynamicObjectSystemImpl &>(*this).findProperty(inName, inPropName);
        if (def.first)
            return def.first->enumValueNames;
        return QDemonConstDataRef<QString>();
    }

    // Called during loading which is pretty heavily multithreaded.
    virtual QDemonConstDataRef<dynamic::QDemonPropertyDefinition> getProperties(const QString &inName) const override
    {
        QMutexLocker locker(&m_propertyLoadMutex);
        QDemonRef<QDemonDynamicObjClassImpl> cls = const_cast<QDemonDynamicObjectSystemImpl &>(*this).findClass(inName);
        if (cls)
            return cls->m_propertyDefinitions;
        return QDemonConstDataRef<dynamic::QDemonPropertyDefinition>();
    }

    void setPropertyTextureSettings(const QString &inName,
                                    const QString &inPropName,
                                    const QString &inPropPath,
                                    QDemonRenderTextureTypeValue inTexType,
                                    QDemonRenderTextureCoordOp inCoordOp,
                                    QDemonRenderTextureMagnifyingOp inMagFilterOp,
                                    QDemonRenderTextureMinifyingOp inMinFilterOp) override
    {
        QPair<const dynamic::QDemonPropertyDefinition *, QDemonRef<QDemonDynamicObjClassImpl>> def = findProperty(inName, inPropName);
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

    QDemonDynamicObjectClassInterface *getDynamicObjectClass(const QString &inName) override
    {
        // TODO: Should probably shared pointer
        return findClass(inName).data();
    }

    void setRenderCommands(const QString &inClassName, const QDemonConstDataRef<dynamic::QDemonCommand *> &inCommands) override
    {
        QDemonRef<QDemonDynamicObjClassImpl> theClass = const_cast<QDemonDynamicObjectSystemImpl &>(*this).findClass(inClassName);
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
        dynamic::QDemonCommand **theCommandPtrBegin = reinterpret_cast<dynamic::QDemonCommand **>(
                theCommandDataBegin + align8(commandAllocationSize));
        dynamic::QDemonCommand **theCurrentCommandPtr = theCommandPtrBegin;
        memset(theCommandDataBegin, 0, totalAllocationSize);

        theClass->m_requiresDepthTexture = false;
        for (quint32 idx = 0, end = inCommands.size(); idx < end; ++idx) {
            dynamic::QDemonCommand &theCommand(*inCommands[idx]);
            quint32 theCommandSize = dynamic::QDemonCommand::getSizeofCommand(theCommand);
            dynamic::QDemonCommand::copyConstructCommand(theCurrentCommandData, theCommand);
            if (theCommand.m_type == dynamic::CommandType::ApplyDepthValue)
                theClass->m_requiresDepthTexture = true;
            if (theCommand.m_type == dynamic::CommandType::BindTarget) {
                dynamic::QDemonBindTarget *bt = reinterpret_cast<dynamic::QDemonBindTarget *>(&theCommand);
                theClass->m_outputFormat = bt->m_outputFormat;
            }

            *theCurrentCommandPtr = reinterpret_cast<dynamic::QDemonCommand *>(theCurrentCommandData);
            ++theCurrentCommandPtr;
            theCurrentCommandData += align(theCommandSize);
        }
        Q_ASSERT(theCurrentCommandData - theCommandDataBegin == (int)commandAllocationSize);
        Q_ASSERT((quint8 *)theCurrentCommandPtr - theCommandDataBegin == (int)totalAllocationSize);
        theClass->m_renderCommands = QDemonConstDataRef<dynamic::QDemonCommand *>(theCommandPtrBegin, inCommands.size());
    }

    virtual QDemonConstDataRef<dynamic::QDemonCommand *> getRenderCommands(const QString &inClassName) const override
    {
        QDemonRef<QDemonDynamicObjClassImpl> cls = const_cast<QDemonDynamicObjectSystemImpl &>(*this).findClass(inClassName);
        if (cls)
            return cls->m_renderCommands;
        return QDemonConstDataRef<dynamic::QDemonCommand *>();
    }

    QDemonRenderDynamicGraphObject *createInstance(const QString &inClassName) override
    {
        QDemonRef<QDemonDynamicObjClassImpl> theClass = findClass(inClassName);
        if (!theClass) {
            Q_ASSERT(false);
            return nullptr;
        }
        quint32 totalObjectSize = theClass->m_baseObjectSize + theClass->m_propertySectionByteSize;
        QDemonRenderDynamicGraphObject *retval = reinterpret_cast<QDemonRenderDynamicGraphObject *>(::malloc(totalObjectSize));
        new (retval) QDemonRenderDynamicGraphObject(theClass->m_graphObjectType, inClassName, theClass->m_propertySectionByteSize, theClass->m_baseObjectSize);
        ::memcpy(retval->getDataSectionBegin(), theClass->m_propertyDefaultData, theClass->m_propertySectionByteSize);
        return retval;
    }

    void setShaderData(const QString &inPath,
                       const char *inData,
                       const char *inShaderType,
                       const char *inShaderVersion,
                       bool inHasGeomShader,
                       bool inIsComputeShader) override
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
            QDemonDynamicObjectShaderInfo
                    &theShaderInfo = m_shaderInfoMap.insert(inPath, QDemonDynamicObjectShaderInfo()).value();
            theShaderInfo.m_type = QString::fromLocal8Bit(nonNull(inShaderType));
            theShaderInfo.m_version = QString::fromLocal8Bit(nonNull(inShaderVersion));
            theShaderInfo.m_hasGeomShader = inHasGeomShader;
            theShaderInfo.m_isComputeShader = inIsComputeShader;
        }

        return;
    }

    QByteArray getShaderCacheKey(const char *inId, const char *inProgramMacro, const dynamic::QDemonDynamicShaderProgramFlags &inFlags)
    {
        QByteArray shaderKey = inId;
        if (inProgramMacro && *inProgramMacro) {
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

    void insertShaderHeaderInformation(QByteArray &theReadBuffer, const char *inPathToEffect) override
    {
        doInsertShaderHeaderInformation(theReadBuffer, inPathToEffect);
    }

    void doInsertShaderHeaderInformation(QByteArray &theReadBuffer, const QString &inPathToEffect)
    {
        // Now do search and replace for the headers
        for (int thePos = theReadBuffer.indexOf(includeSearch); thePos != -1;
             thePos = theReadBuffer.indexOf(includeSearch, thePos + 1)) {
            int theEndQuote = theReadBuffer.indexOf('\"', thePos + strlen(includeSearch) + 1);
            // Indicates an unterminated include file.
            if (theEndQuote == -1) {
                qCCritical(INVALID_OPERATION, "Unterminated include in file: %s", qPrintable(inPathToEffect));
                theReadBuffer.clear();
                break;
            }
            int theActualBegin = thePos + strlen(includeSearch);
            QString theInclude = theReadBuffer.mid(theActualBegin, theEndQuote - theActualBegin);
            // If we haven't included the file yet this round
            QByteArray theHeader = doLoadShader(theInclude);
            //            quint32 theLen = (quint32)strlen(theHeader);
            //            theReadBuffer = theReadBuffer.replace(theReadBuffer.begin() + thePos, theReadBuffer.begin() + theEndQuote + 1, theHeader, theLen);
            theReadBuffer = theReadBuffer.replace(thePos, (theEndQuote + 1) - thePos, theHeader);
        }
    }

    QByteArray doLoadShader(const QString &inPathToEffect)
    {
        auto theInsert = m_expandedFiles.find(inPathToEffect);
        const bool found = (theInsert != m_expandedFiles.end());
        //        if (found)
        //            *theInsert = QByteArray();
        //        else
        //            theInsert = m_expandedFiles.insert(inPathToEffect, QByteArray());

        QByteArray theReadBuffer;
        if (!found) {
            const QString defaultDir = m_context->getDynamicObjectSystem()->getShaderCodeLibraryDirectory();
            const QString platformDir = m_context->getDynamicObjectSystem()->shaderCodeLibraryPlatformDirectory();
            const QString ver = m_context->getDynamicObjectSystem()->shaderCodeLibraryVersion();

            QString fullPath;
            QSharedPointer<QIODevice> theStream;
            if (!platformDir.isEmpty()) {
                QTextStream stream(&fullPath);
                stream << platformDir << QLatin1Char('/') << inPathToEffect;
                theStream = m_coreContext->getInputStreamFactory()->getStreamForFile(fullPath, true);
            }

            if (theStream.isNull()) {
                fullPath.clear();
                QTextStream stream(&fullPath);
                stream << defaultDir << QLatin1Char('/') << ver << QLatin1Char('/') << inPathToEffect;
                theStream = m_coreContext->getInputStreamFactory()->getStreamForFile(fullPath, true);
                if (theStream.isNull()) {
                    fullPath.clear();
                    QTextStream stream(&fullPath);
                    stream << defaultDir << QLatin1Char('/') << inPathToEffect;
                    theStream = m_coreContext->getInputStreamFactory()->getStreamForFile(fullPath, false);
                }
            }
            if (!theStream.isNull()) {
                quint8 readBuf[1024];
                quint32 amountRead = 0;
                do {
                    amountRead = theStream->read((char *)readBuf, 1024);
                    if (amountRead)
                        theReadBuffer.append((const char *)readBuf, amountRead);
                } while (amountRead);
            } else {
                qCCritical(INVALID_OPERATION, "Failed to find include file %s", qPrintable(inPathToEffect));
                Q_ASSERT(false);
            }
            theInsert = m_expandedFiles.insert(inPathToEffect, theReadBuffer);
        } else {
            theReadBuffer = theInsert.value();
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

    void setContextInterface(QDemonRenderContextInterface *rc) override { m_context = rc; }

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

    QDemonRef<QDemonRenderShaderProgram> compileShader(QString inId,
                                                       const char *inProgramSource,
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

        if (inGeomSource && inFlags & ShaderCacheProgramFlagValues::GeometryShaderEnabled) {
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

        QByteArray theKey = getShaderCacheKey(inId.toLocal8Bit(), inProgramMacroName.toLocal8Bit(), inFlags);
        if (inForceCompilation) {
            return theShaderCache->forceCompileProgram(theKey, m_vertShader, m_fragShader, nullptr, nullptr, m_geometryShader, theFlags, inFeatureSet, false);
        }

        return theShaderCache->compileProgram(theKey, m_vertShader, m_fragShader, nullptr, nullptr, m_geometryShader, theFlags, inFeatureSet);
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
            QDemonRef<QDemonRenderShaderProgram> theProgram = m_context->getShaderCache()
                                                                      ->getProgram(getShaderCacheKey(inPath.toLocal8Bit(),
                                                                                                     inProgramMacro.toLocal8Bit(),
                                                                                                     inFlags),
                                                                                   inFeatureSet);
            dynamic::QDemonDynamicShaderProgramFlags theFlags(inFlags);
            if (!theProgram || inForceCompilation) {
                QDemonDynamicObjectShaderInfo
                        &theShaderInfo = m_shaderInfoMap.insert(inPath, QDemonDynamicObjectShaderInfo()).value();
                if (theShaderInfo.m_isComputeShader == false) {
                    QString programSource = doLoadShader(inPath);
                    if (theShaderInfo.m_hasGeomShader)
                        theFlags |= ShaderCacheProgramFlagValues::GeometryShaderEnabled;
                    theProgram = compileShader(inPath, programSource.toLocal8Bit().constData(), nullptr, inProgramMacro, inFeatureSet, theFlags, inForceCompilation);
                } else {
                    QString theShaderBuffer;
                    QString shaderVersionStr = QStringLiteral("#version 430\n");
                    if (m_context->getRenderContext()->renderContextType() == QDemonRenderContextType::GLES3PLUS)
                        shaderVersionStr = QStringLiteral("#version 310 es\n");
                    theShaderBuffer = doLoadShader(inPath);
                    theShaderBuffer.insert(0, shaderVersionStr);
                    const char *programSource = theShaderBuffer.toLocal8Bit();
                    quint32 len = (quint32)strlen(nonNull(programSource)) + 1;
                    theProgram = m_context->getRenderContext()
                                         ->compileComputeSource(inPath.toLocal8Bit(),
                                                                QDemonConstDataRef<qint8>((qint8 *)programSource, len))
                                         .m_shader;
                }
            }
            theInserter.value() = TShaderAndFlags(theProgram, theFlags);
        }
        return theInserter.value();
    }

    TShaderAndFlags getDepthPrepassShader(QString inPath, QString inPMacro, TShaderFeatureSet inFeatureSet) override
    {
        QDemonDynamicObjectShaderInfo &theShaderInfo = m_shaderInfoMap.insert(inPath, QDemonDynamicObjectShaderInfo()).value();
        if (theShaderInfo.m_hasGeomShader == false)
            return TShaderAndFlags();
        // else, here we go...
        dynamic::QDemonDynamicShaderProgramFlags theFlags;
        QByteArray shaderKey = inPMacro.toUtf8();
        shaderKey.append("depthprepass");

        QString theProgramMacro = shaderKey;

        const dynamic::QDemonDynamicShaderMapKey shaderMapKey(TStrStrPair(inPath, theProgramMacro),
                                                              inFeatureSet,
                                                              theFlags.tessMode,
                                                              theFlags.wireframeMode);
        const TShaderAndFlags shaderFlags;
        auto theInserter = m_shaderMap.find(shaderMapKey);
        const bool found = theInserter != m_shaderMap.end();
        if (found) {
            QDemonRef<QDemonRenderShaderProgram> theProgram = m_context->getShaderCache()
                                                                      ->getProgram(getShaderCacheKey(inPath.toLocal8Bit(),
                                                                                                     theProgramMacro.toLocal8Bit(),
                                                                                                     theFlags),
                                                                                   inFeatureSet);
            dynamic::QDemonDynamicShaderProgramFlags flags(theFlags);
            if (!theProgram) {
                QString geomSource = doLoadShader(inPath);
                QDemonShaderVertexCodeGenerator vertexShader(m_context->getRenderContext()->renderContextType());
                QDemonShaderFragmentCodeGenerator fragmentShader(vertexShader,
                                                                 m_context->getRenderContext()->renderContextType());

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

                QString programBuffer;
                programBuffer = QStringLiteral("#ifdef VERTEX_SHADER\n");
                programBuffer.append(vertexSource);
                programBuffer.append("\n#endif\n");
                programBuffer.append("\n#ifdef FRAGMENT_SHADER\n");
                programBuffer.append(fragmentSource);
                programBuffer.append("\n#endif");
                flags |= ShaderCacheProgramFlagValues::GeometryShaderEnabled;
                theProgram = compileShader(inPath, programBuffer.toLatin1(), geomSource.toLocal8Bit().constData(), theProgramMacro, inFeatureSet, flags);
            }
            theInserter.value() = TShaderAndFlags(theProgram, flags);
        }
        return theInserter.value();
    }

    void setShaderCodeLibraryVersion(const QByteArray &version) override { m_shaderLibraryVersion = version; }

    QString shaderCodeLibraryVersion() override { return m_shaderLibraryVersion; }

    virtual void setShaderCodeLibraryPlatformDirectory(const QString &directory) override
    {
        m_shaderLibraryPlatformDirectory = directory;
    }

    virtual QString shaderCodeLibraryPlatformDirectory() override { return m_shaderLibraryPlatformDirectory; }
};
}

QDemonRef<QDemonDynamicObjectSystemInterface> QDemonDynamicObjectSystemInterface::createDynamicSystem(QDemonRenderContextCoreInterface *rc)
{
    return QDemonRef<QDemonDynamicObjectSystemImpl>(new QDemonDynamicObjectSystemImpl(rc));
}

QDemonDynamicObjectSystemInterface::~QDemonDynamicObjectSystemInterface() {}

QT_END_NAMESPACE
