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
#include <qdemonrendereffectsystem.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <qdemonrenderinputstreamfactory.h>
//#include <QtDemonRuntimeRender/qdemonrenderstring.h>
#include <QtDemonRuntimeRender/qdemonrendereffect.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcemanager.h>
#include <qdemonrenderdynamicobjectsystemcommands.h>
#include <QtDemonRender/qdemonrenderframebuffer.h>
#include <QtDemonRender/qdemonrendershaderconstant.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRuntimeRender/qdemonrenderer.h>
#include <qdemontextrenderer.h>
#include <QtDemonRuntimeRender/qdemonrendercamera.h>
#include <QtDemonRuntimeRender/qdemonrenderbuffermanager.h>
#include <QtDemonRuntimeRender/qdemonoffscreenrendermanager.h>
#include <QtDemonRuntimeRender/qdemonrendershadercache.h>
#include <qdemonoffscreenrenderkey.h>
#include <qdemonrenderdynamicobjectsystemutil.h>


QT_BEGIN_NAMESPACE

using namespace dynamic;

// None of this code will work if the size of void* changes because that would mean that
// the alignment of some of the objects isn't 4 bytes but would be 8 bytes.

typedef QPair<QString, QString> TStrStrPair;

namespace {

/*
                ApplyBufferValue,
                //Apply the depth buffer as an input texture.
                ApplyDepthValue,
                Render, //Render to current FBO
                */

struct SEffectClass
{
    IDynamicObjectClass *m_DynamicClass;

    SEffectClass(IDynamicObjectClass &dynClass)
        : m_DynamicClass(&dynClass)
    {
    }

    void SetupThisObjectFromMemory(IDynamicObjectClass &inClass)
    {
        m_DynamicClass = &inClass;
    }
};

struct SAllocatedBufferEntry
{
    QString m_Name;
    QSharedPointer<QDemonRenderFrameBuffer> m_FrameBuffer;
    QSharedPointer<QDemonRenderTexture2D> m_Texture;
    SAllocateBufferFlags m_Flags;
    bool m_NeedsClear;

    SAllocatedBufferEntry(QString inName, QDemonRenderFrameBuffer &inFb,
                          QDemonRenderTexture2D &inTexture, SAllocateBufferFlags inFlags)
        : m_Name(inName)
        , m_FrameBuffer(&inFb)
        , m_Texture(&inTexture)
        , m_Flags(inFlags)
        , m_NeedsClear(true)
    {
    }
    SAllocatedBufferEntry() {}
};

struct SAllocatedImageEntry
{
    QString m_Name;
    QSharedPointer<QDemonRenderImage2D> m_Image;
    QSharedPointer<QDemonRenderTexture2D> m_Texture;
    SAllocateBufferFlags m_Flags;

    SAllocatedImageEntry(QString inName, QDemonRenderImage2D &inImage,
                         QDemonRenderTexture2D &inTexture, SAllocateBufferFlags inFlags)
        : m_Name(inName)
        , m_Image(&inImage)
        , m_Texture(&inTexture)
        , m_Flags(inFlags)
    {
    }
    SAllocatedImageEntry() {}
};

struct SImageEntry
{
    QSharedPointer<QDemonRenderShaderProgram> m_Shader;
    QDemonRenderCachedShaderProperty<QDemonRenderImage2D *> m_Image;

    SImageEntry(QSharedPointer<QDemonRenderShaderProgram> inShader, const char *inImageName)
        : m_Shader(inShader)
        , m_Image(inImageName, inShader)
    {
    }

    void Set(QDemonRenderImage2D *inImage) { m_Image.Set(inImage); }

    static SImageEntry CreateImageEntry(QSharedPointer<QDemonRenderShaderProgram> inShader, const char *inStem)
    {
        return SImageEntry(inShader, inStem);
    }
};

struct SAllocatedDataBufferEntry
{
    QString m_Name;
    QSharedPointer<QDemonRenderDataBuffer> m_DataBuffer;
    QDemonRenderBufferBindValues::Enum m_BufferType;
    QDemonDataRef<quint8> m_BufferData;
    SAllocateBufferFlags m_Flags;
    bool m_NeedsClear;

    SAllocatedDataBufferEntry(QString inName,
                              QDemonRenderDataBuffer &inDataBuffer,
                              QDemonRenderBufferBindValues::Enum inType, QDemonDataRef<quint8> data,
                              SAllocateBufferFlags inFlags)
        : m_Name(inName)
        , m_DataBuffer(&inDataBuffer)
        , m_BufferType(inType)
        , m_BufferData(data)
        , m_Flags(inFlags)
        , m_NeedsClear(false)
    {
    }
    SAllocatedDataBufferEntry() {}
};

struct SDataBufferEntry
{
    QSharedPointer<QDemonRenderShaderProgram> m_Shader;
    QDemonRenderCachedShaderBuffer<QDemonRenderShaderBufferBase> m_DataBuffer;

    SDataBufferEntry(QSharedPointer<QDemonRenderShaderProgram> inShader, const char *inBufferName)
        : m_Shader(inShader)
        , m_DataBuffer(inBufferName, inShader)
    {
    }

    void Set(QDemonRenderDataBuffer *inBuffer)
    {
        if (inBuffer)
            inBuffer->Bind();

        m_DataBuffer.Set();
    }

    static SDataBufferEntry CreateDataBufferEntry(QSharedPointer<QDemonRenderShaderProgram> inShader,
                                                  const char *inStem)
    {
        return SDataBufferEntry(inShader, inStem);
    }
};

struct SEffectTextureData
{
    QSharedPointer<QDemonRenderTexture2D> m_Texture;
    bool m_NeedsAlphaMultiply;
    SEffectTextureData(QSharedPointer<QDemonRenderTexture2D> inTexture, bool inNeedsMultiply)
        : m_Texture(inTexture)
        , m_NeedsAlphaMultiply(inNeedsMultiply)
    {
    }
    SEffectTextureData()
        : m_Texture(nullptr)
        , m_NeedsAlphaMultiply(false)
    {
    }
};

struct STextureEntry
{
    QSharedPointer<QDemonRenderShaderProgram> m_Shader;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> m_Texture;
    QDemonRenderCachedShaderProperty<QVector4D> m_TextureData;
    QDemonRenderCachedShaderProperty<qint32> m_TextureFlags;

    STextureEntry(QSharedPointer<QDemonRenderShaderProgram> inShader,
                  const char *inTexName,
                  const char *inDataName,
                  const char *inFlagName)
        : m_Shader(inShader)
        , m_Texture(inTexName, inShader)
        , m_TextureData(inDataName, inShader)
        , m_TextureFlags(inFlagName, inShader)
    {
    }

    void Set(QSharedPointer<QDemonRenderTexture2D> inTexture, bool inNeedsAlphaMultiply,
             const SPropertyDefinition *inDefinition)
    {
        float theMixValue(inNeedsAlphaMultiply ? 0.0f : 1.0f);
        if (inTexture && inDefinition) {
            inTexture->SetMagFilter(inDefinition->m_MagFilterOp);
            inTexture->SetMinFilter(
                        static_cast<QDemonRenderTextureMinifyingOp::Enum>(inDefinition->m_MagFilterOp));
            inTexture->SetTextureWrapS(inDefinition->m_CoordOp);
            inTexture->SetTextureWrapT(inDefinition->m_CoordOp);
        }
        m_Texture.Set(inTexture.data());
        if (inTexture) {
            STextureDetails theDetails(inTexture->GetTextureDetails());
            m_TextureData.Set(
                        QVector4D((float)theDetails.m_Width, (float)theDetails.m_Height, theMixValue, 0.0f));
            // I have no idea what these flags do.
            m_TextureFlags.Set(1);
        } else
            m_TextureFlags.Set(0);
    }

    static STextureEntry CreateTextureEntry(QSharedPointer<QDemonRenderShaderProgram> inShader,
                                            const char *inStem,
                                            QString &inBuilder,
                                            QString &inBuilder2)
    {
        inBuilder = inStem;
        inBuilder.append("Info");
        inBuilder2 = "flag";
        inBuilder2.append(inStem);
        return STextureEntry(inShader, inStem, inBuilder.toLocal8Bit(), inBuilder2.toLocal8Bit());
    }
};

typedef QPair<QString, QSharedPointer<STextureEntry>> TNamedTextureEntry;
typedef QPair<QString, QSharedPointer<SImageEntry>> TNamedImageEntry;
typedef QPair<QString, QSharedPointer<SDataBufferEntry>> TNamedDataBufferEntry;
}

struct SEffectContext
{
    QString m_ClassName;
    IQDemonRenderContext *m_Context;
    QSharedPointer<IResourceManager> m_ResourceManager;
    QVector<SAllocatedBufferEntry> m_AllocatedBuffers;
    QVector<SAllocatedImageEntry> m_AllocatedImages;
    QVector<SAllocatedDataBufferEntry> m_AllocatedDataBuffers;
    QVector<TNamedTextureEntry> m_TextureEntries;
    QVector<TNamedImageEntry> m_ImageEntries;
    QVector<TNamedDataBufferEntry> m_DataBufferEntries;

    SEffectContext(QString inName, IQDemonRenderContext *ctx, QSharedPointer<IResourceManager> inManager)
        : m_ClassName(inName)
        , m_Context(ctx)
        , m_ResourceManager(inManager)
    {
    }

    ~SEffectContext()
    {
        while (m_AllocatedBuffers.size())
            ReleaseBuffer(0);

        while (m_AllocatedImages.size())
            ReleaseImage(0);

        while (m_AllocatedDataBuffers.size())
            ReleaseDataBuffer(0);
    }

    void ReleaseBuffer(quint32 inIdx)
    {
        SAllocatedBufferEntry &theEntry(m_AllocatedBuffers[inIdx]);
        theEntry.m_FrameBuffer->Attach(QDemonRenderFrameBufferAttachments::Color0,
                                       QDemonRenderTextureOrRenderBuffer());
        m_ResourceManager->Release(theEntry.m_FrameBuffer);
        m_ResourceManager->Release(theEntry.m_Texture);
        { // replace_with_last
            m_AllocatedBuffers[inIdx] = m_AllocatedBuffers.back();
            m_AllocatedBuffers.pop_back();
        }
    }

    void ReleaseImage(quint32 inIdx)
    {
        SAllocatedImageEntry &theEntry(m_AllocatedImages[inIdx]);
        m_ResourceManager->Release(theEntry.m_Image);
        m_ResourceManager->Release(theEntry.m_Texture);
        { // replace_with_last
            m_AllocatedImages[inIdx] = m_AllocatedImages.back();
            m_AllocatedImages.pop_back();
        }
    }

    void ReleaseDataBuffer(quint32 inIdx)
    {
        SAllocatedDataBufferEntry &theEntry(m_AllocatedDataBuffers[inIdx]);
        ::free(theEntry.m_BufferData.begin());
        { // replace_with_last
            m_AllocatedDataBuffers[inIdx] = m_AllocatedDataBuffers.back();
            m_AllocatedDataBuffers.pop_back();
        }
    }

    quint32 FindBuffer(QString inName)
    {
        for (quint32 idx = 0, end = m_AllocatedBuffers.size(); idx < end; ++idx)
            if (m_AllocatedBuffers[idx].m_Name == inName)
                return idx;
        return m_AllocatedBuffers.size();
    }

    quint32 FindImage(QString inName)
    {
        for (quint32 idx = 0, end = m_AllocatedImages.size(); idx < end; ++idx)
            if (m_AllocatedImages[idx].m_Name == inName)
                return idx;

        return m_AllocatedImages.size();
    }

    quint32 FindDataBuffer(QString inName)
    {
        for (quint32 idx = 0, end = m_AllocatedDataBuffers.size(); idx < end; ++idx) {
            if (m_AllocatedDataBuffers[idx].m_Name == inName)
                return idx;
        }

        return m_AllocatedDataBuffers.size();
    }

    void SetTexture(QSharedPointer<QDemonRenderShaderProgram> inShader, QString inPropName,
                    QSharedPointer<QDemonRenderTexture2D> inTexture, bool inNeedsMultiply,
                    QString &inStringBuilder, QString &inStringBuilder2,
                    const SPropertyDefinition *inPropDec = nullptr)
    {
        QSharedPointer<STextureEntry> theTextureEntry;
        for (quint32 idx = 0, end = m_TextureEntries.size(); idx < end && theTextureEntry == nullptr;
             ++idx) {
            if (m_TextureEntries[idx].first == inPropName
                    && m_TextureEntries[idx].second->m_Shader == inShader)
                theTextureEntry = m_TextureEntries[idx].second;
        }
        if (theTextureEntry == nullptr) {
            QSharedPointer<STextureEntry> theNewEntry(new STextureEntry(STextureEntry::CreateTextureEntry(inShader, inPropName.toLatin1(), inStringBuilder, inStringBuilder2)));
            m_TextureEntries.push_back(QPair<QString, QSharedPointer<STextureEntry>>(inPropName, theNewEntry));
            theTextureEntry = theNewEntry;
        }
        theTextureEntry->Set(inTexture, inNeedsMultiply, inPropDec);
    }

    void SetImage(QSharedPointer<QDemonRenderShaderProgram> inShader, QString inPropName,
                  QSharedPointer<QDemonRenderImage2D> inImage)
    {
        QSharedPointer<SImageEntry> theImageEntry;
        for (quint32 idx = 0, end = m_ImageEntries.size(); idx < end && theImageEntry == nullptr;
             ++idx) {
            if (m_ImageEntries[idx].first == inPropName
                    && m_ImageEntries[idx].second->m_Shader == inShader)
                theImageEntry = m_ImageEntries[idx].second;
        }
        if (theImageEntry == nullptr) {
            QSharedPointer<SImageEntry> theNewEntry(new SImageEntry(SImageEntry::CreateImageEntry(inShader, inPropName.toLatin1())));
            m_ImageEntries.push_back(QPair<QString, QSharedPointer<SImageEntry>>(inPropName, theNewEntry));
            theImageEntry = theNewEntry;
        }

        theImageEntry->Set(inImage.data());
    }

    void SetDataBuffer(QSharedPointer<QDemonRenderShaderProgram> inShader, QString inPropName,
                       QSharedPointer<QDemonRenderDataBuffer> inBuffer)
    {
        QSharedPointer<SDataBufferEntry> theDataBufferEntry;
        for (quint32 idx = 0, end = m_DataBufferEntries.size();
             idx < end && theDataBufferEntry == nullptr; ++idx) {
            if (m_DataBufferEntries[idx].first == inPropName
                    && m_DataBufferEntries[idx].second->m_Shader == inShader)
                theDataBufferEntry = m_DataBufferEntries[idx].second;
        }
        if (theDataBufferEntry == nullptr) {
            QSharedPointer<SDataBufferEntry> theNewEntry(
                    new SDataBufferEntry(
                        SDataBufferEntry::CreateDataBufferEntry(inShader, inPropName.toLatin1())));
            m_DataBufferEntries.push_back(QPair<QString, QSharedPointer<SDataBufferEntry>>(inPropName, theNewEntry));
            theDataBufferEntry = theNewEntry;
        }

        theDataBufferEntry->Set(inBuffer.data());
    }
};

namespace {

/* We setup some shared state on the effect shaders */
struct SEffectShader
{
    QSharedPointer<QDemonRenderShaderProgram> m_Shader;
    QDemonRenderCachedShaderProperty<QMatrix4x4> m_MVP;
    QDemonRenderCachedShaderProperty<QVector2D> m_FragColorAlphaSettings;
    QDemonRenderCachedShaderProperty<QVector2D> m_DestSize;
    QDemonRenderCachedShaderProperty<float> m_AppFrame;
    QDemonRenderCachedShaderProperty<float> m_FPS;
    QDemonRenderCachedShaderProperty<QVector2D> m_CameraClipRange;
    STextureEntry m_TextureEntry;
    SEffectShader(QSharedPointer<QDemonRenderShaderProgram> inShader)
        : m_Shader(inShader)
        , m_MVP("ModelViewProjectionMatrix", inShader)
        , m_FragColorAlphaSettings("FragColorAlphaSettings", inShader)
        , m_DestSize("DestSize", inShader)
        , m_AppFrame("AppFrame", inShader)
        , m_FPS("FPS", inShader)
        , m_CameraClipRange("CameraClipRange", inShader)
        , m_TextureEntry(inShader, "Texture0", "Texture0Info", "Texture0Flags")
    {
    }
};

struct SEffectSystem : public IEffectSystem, public QEnableSharedFromThis<SEffectSystem>
{
    typedef QHash<QString, char *> TPathDataMap;
    typedef QSet<QString> TPathSet;
    typedef QHash<QString, QSharedPointer<SEffectClass>> TEffectClassMap;
    typedef QHash<TStrStrPair, QSharedPointer<SEffectShader>> TShaderMap;
    typedef QVector<QSharedPointer<SEffectContext>> TContextList;

    IQDemonRenderContextCore * m_CoreContext;
    IQDemonRenderContext *m_Context;
    QSharedPointer<IResourceManager> m_ResourceManager;
    // Keep from dual-including headers.
    TEffectClassMap m_EffectClasses;
    QVector<QString> m_EffectList;
    TContextList m_Contexts;
    QString m_TextureStringBuilder;
    QString m_TextureStringBuilder2;
    TShaderMap m_ShaderMap;
    QSharedPointer<QDemonRenderDepthStencilState> m_DefaultStencilState;
    QVector<QSharedPointer<QDemonRenderDepthStencilState>> m_DepthStencilStates;

    SEffectSystem(IQDemonRenderContextCore * inContext)
        : m_CoreContext(inContext)
        , m_Context(nullptr)
    {
    }

    ~SEffectSystem()
    {
//        for (quint32 idx = 0, end = m_Contexts.size(); idx < end; ++idx)
//            delete m_Contexts[idx];
        m_Contexts.clear();
    }

    SEffectContext &GetEffectContext(SEffect &inEffect)
    {
        if (inEffect.m_Context == nullptr) {
            inEffect.m_Context = QSharedPointer<SEffectContext>(new SEffectContext(inEffect.m_ClassName, m_Context, m_ResourceManager));
            m_Contexts.push_back(inEffect.m_Context);
        }
        return *inEffect.m_Context;
    }

    QSharedPointer<SEffectClass> GetEffectClass(QString inStr)
    {
        TEffectClassMap::iterator theIter = m_EffectClasses.find(inStr);
        if (theIter != m_EffectClasses.end())
            return theIter.value();
        return nullptr;
    }
    const QSharedPointer<SEffectClass> GetEffectClass(QString inStr) const
    {
        return const_cast<SEffectSystem *>(this)->GetEffectClass(inStr);
    }

    bool IsEffectRegistered(QString inStr) override
    {
        return GetEffectClass(inStr) != nullptr;
    }
    QVector<QString> GetRegisteredEffects() override
    {
        m_EffectList.clear();
        for (TEffectClassMap::iterator theIter = m_EffectClasses.begin(),
             theEnd = m_EffectClasses.end();
             theIter != theEnd; ++theIter)
            m_EffectList.push_back(theIter.key());
        return m_EffectList;
    }

    // Registers an effect that runs via a single GLSL file.
    bool RegisterGLSLEffect(QString inName, const char *inPathToEffect,
                            QDemonConstDataRef<SPropertyDeclaration> inProperties) override
    {
        if (IsEffectRegistered(inName))
            return false;

        m_CoreContext->GetDynamicObjectSystemCore()->Register(inName, inProperties, sizeof(SEffect),
                                                            GraphObjectTypes::Effect);
        IDynamicObjectClass &theClass =
                *m_CoreContext->GetDynamicObjectSystemCore()->GetDynamicObjectClass(inName);

        QSharedPointer<SEffectClass> theEffect(new SEffectClass(theClass));
        m_EffectClasses.insert(inName, theEffect);

        // Setup the commands required to run this effect
//        StaticAssert<(sizeof(SBindShader) % 4 == 0)>::valid_expression();
//        StaticAssert<(sizeof(SApplyInstanceValue) % 4 == 0)>::valid_expression();
//        StaticAssert<(sizeof(SRender) % 4 == 0)>::valid_expression();

        quint32 commandAllocationSize = sizeof(SBindTarget);
        commandAllocationSize += sizeof(SBindShader);
        commandAllocationSize += sizeof(SApplyInstanceValue) * inProperties.size();
        commandAllocationSize += sizeof(SRender);

        quint32 commandCount = 3 + inProperties.size();
        quint32 commandPtrAllocationSize = commandCount * sizeof(SCommand *);
        quint32 allocationSize = Align8(commandAllocationSize) + commandPtrAllocationSize;
        quint8 *startBuffer =
                (quint8 *)::malloc(allocationSize);
        quint8 *dataBuffer = startBuffer;
        // Setup the command buffer such that the ptrs to the commands and the commands themselves
        // are
        // all in the same block of memory.  This enables quicker iteration (trivially quicker, most
        // likely)
        // but it also enables simpler memory management (single free).
        // Furthermore, for a single glsl file the effect properties map precisely into the
        // glsl file itself.
        SCommand **theFirstCommandPtr =
                (reinterpret_cast<SCommand **>(dataBuffer + Align8(commandAllocationSize)));
        SCommand **theCommandPtr = theFirstCommandPtr;
        memZero(dataBuffer, commandAllocationSize);

        new (dataBuffer) SBindTarget();
        *theCommandPtr = (SCommand *)dataBuffer;
        ++theCommandPtr;
        dataBuffer += sizeof(SBindTarget);

        new (dataBuffer) SBindShader(QString::fromLocal8Bit(inPathToEffect));
        *theCommandPtr = (SCommand *)dataBuffer;
        ++theCommandPtr;
        dataBuffer += sizeof(SBindShader);

        for (quint32 idx = 0, end = inProperties.size(); idx < end; ++idx) {
            const SPropertyDefinition &theDefinition(
                        theEffect->m_DynamicClass->GetProperties()[idx]);
            new (dataBuffer) SApplyInstanceValue(theDefinition.m_Name, theDefinition.m_DataType,
                                                 theDefinition.m_Offset);
            *theCommandPtr = (SCommand *)dataBuffer;
            ++theCommandPtr;
            dataBuffer += sizeof(SApplyInstanceValue);
        }
        new (dataBuffer) SRender(false);
        *theCommandPtr = (SCommand *)dataBuffer;
        ++theCommandPtr;
        dataBuffer += sizeof(SRender);
        // Ensure we end up *exactly* where we expected to.
        Q_ASSERT(dataBuffer == startBuffer + commandAllocationSize);
        Q_ASSERT(theCommandPtr - theFirstCommandPtr == (int)inProperties.size() + 3);
        m_CoreContext->GetDynamicObjectSystemCore()->SetRenderCommands(
                    inName, QDemonConstDataRef<SCommand *>(theFirstCommandPtr, commandCount));
        ::free(startBuffer);
        return true;
    }

    void SetEffectPropertyDefaultValue(QString inName,
                                       QString inPropName,
                                       QDemonConstDataRef<quint8> inDefaultData) override
    {
        m_CoreContext->GetDynamicObjectSystemCore()->SetPropertyDefaultValue(inName, inPropName,
                                                                           inDefaultData);
    }

    void SetEffectPropertyEnumNames(QString inName, QString inPropName,
                                    QDemonConstDataRef<QString> inNames) override
    {
        m_CoreContext->GetDynamicObjectSystemCore()->SetPropertyEnumNames(inName, inPropName,
                                                                        inNames);
    }

    bool RegisterEffect(QString inName,
                        QDemonConstDataRef<SPropertyDeclaration> inProperties) override
    {
        if (IsEffectRegistered(inName))
            return false;
        m_CoreContext->GetDynamicObjectSystemCore()->Register(inName, inProperties, sizeof(SEffect),
                                                            GraphObjectTypes::Effect);
        auto theClass = m_CoreContext->GetDynamicObjectSystemCore()->GetDynamicObjectClass(inName);
        QSharedPointer<SEffectClass> theEffect(new SEffectClass(*theClass));
        m_EffectClasses.insert(inName, theEffect);
        return true;
    }

    bool UnregisterEffect(QString inName) override
    {
        if (!IsEffectRegistered(inName))
            return false;

        m_CoreContext->GetDynamicObjectSystemCore()->Unregister(inName);

        TEffectClassMap::iterator iter = m_EffectClasses.find(inName);
        if (iter != m_EffectClasses.end())
            m_EffectClasses.erase(iter);

        for (quint32 idx = 0, end = m_Contexts.size(); idx < end; ++idx) {
            if (m_Contexts[idx]->m_ClassName == inName)
                ReleaseEffectContext(m_Contexts[idx].data());
        }
        return true;
    }

    virtual QDemonConstDataRef<QString>
    GetEffectPropertyEnumNames(QString inName, QString inPropName) const override
    {
        const auto theClass = GetEffectClass(inName);
        if (theClass == nullptr) {
            Q_ASSERT(false);
            QDemonConstDataRef<QString>();
        }
        const SPropertyDefinition *theDefinitionPtr =
                theClass->m_DynamicClass->FindPropertyByName(inPropName);
        if (theDefinitionPtr)
            return theDefinitionPtr->m_EnumValueNames;
        return QDemonConstDataRef<QString>();
    }

    virtual QDemonConstDataRef<SPropertyDefinition>
    GetEffectProperties(QString inEffectName) const override
    {
        const auto theClass = GetEffectClass(inEffectName);
        if (theClass)
            return theClass->m_DynamicClass->GetProperties();
        return QDemonConstDataRef<SPropertyDefinition>();
    }

    void SetEffectPropertyTextureSettings(QString inName,
                                          QString inPropName,
                                          QString inPropPath,
                                          QDemonRenderTextureTypeValue::Enum inTexType,
                                          QDemonRenderTextureCoordOp::Enum inCoordOp,
                                          QDemonRenderTextureMagnifyingOp::Enum inMagFilterOp,
                                          QDemonRenderTextureMinifyingOp::Enum inMinFilterOp) override
    {
        m_CoreContext->GetDynamicObjectSystemCore()->SetPropertyTextureSettings(
                    inName, inPropName, inPropPath, inTexType, inCoordOp, inMagFilterOp, inMinFilterOp);
    }

    void SetEffectRequiresDepthTexture(QString inEffectName, bool inValue) override
    {
        auto theClass = GetEffectClass(inEffectName);
        if (theClass == nullptr) {
            Q_ASSERT(false);
            return;
        }
        theClass->m_DynamicClass->SetRequiresDepthTexture(inValue);
    }

    bool DoesEffectRequireDepthTexture(QString inEffectName) const override
    {
        const auto theClass = GetEffectClass(inEffectName);
        if (theClass == nullptr) {
            Q_ASSERT(false);
            return false;
        }
        return theClass->m_DynamicClass->RequiresDepthTexture();
    }

    void SetEffectRequiresCompilation(QString inEffectName, bool inValue) override
    {
        auto theClass = GetEffectClass(inEffectName);
        if (theClass == nullptr) {
            Q_ASSERT(false);
            return;
        }
        theClass->m_DynamicClass->SetRequiresCompilation(inValue);
    }

    bool DoesEffectRequireCompilation(QString inEffectName) const override
    {
        const auto theClass = GetEffectClass(inEffectName);
        if (theClass == nullptr) {
            Q_ASSERT(false);
            return false;
        }
        return theClass->m_DynamicClass->RequiresCompilation();
    }

    void SetEffectCommands(QString inEffectName,
                           QDemonConstDataRef<dynamic::SCommand *> inCommands) override
    {
        m_CoreContext->GetDynamicObjectSystemCore()->SetRenderCommands(inEffectName, inCommands);
    }

    virtual QDemonConstDataRef<dynamic::SCommand *>
    GetEffectCommands(QString inEffectName) const override
    {
        return m_CoreContext->GetDynamicObjectSystemCore()->GetRenderCommands(inEffectName);
    }

    SEffect *CreateEffectInstance(QString inEffectName) override
    {
        auto theClass = GetEffectClass(inEffectName);
        if (theClass == nullptr)
            return nullptr;
//        StaticAssert<(sizeof(SEffect) % 4 == 0)>::valid_expression();

        SEffect *theEffect = static_cast<SEffect *>(m_CoreContext->GetDynamicObjectSystemCore()->CreateInstance(inEffectName));
        theEffect->Initialize();
        return theEffect;
    }

    void AllocateBuffer(SEffect &inEffect, const SAllocateBuffer &inCommand, quint32 inFinalWidth,
                        quint32 inFinalHeight, QDemonRenderTextureFormats::Enum inSourceTextureFormat)
    {
        // Check to see if it is already allocated and if it is, is it the correct size. If both of
        // these assumptions hold, then we are good.
        QSharedPointer<QDemonRenderTexture2D> theBufferTexture;
        quint32 theWidth =
                ITextRenderer::NextMultipleOf4((quint32)(inFinalWidth * inCommand.m_SizeMultiplier));
        quint32 theHeight =
                ITextRenderer::NextMultipleOf4((quint32)(inFinalHeight * inCommand.m_SizeMultiplier));
        QDemonRenderTextureFormats::Enum resultFormat = inCommand.m_Format;
        if (resultFormat == QDemonRenderTextureFormats::Unknown)
            resultFormat = inSourceTextureFormat;

        if (inEffect.m_Context) {
            SEffectContext &theContext(*inEffect.m_Context);
            // size intentionally requiried every loop;
            quint32 bufferIdx = theContext.FindBuffer(inCommand.m_Name);
            if (bufferIdx < theContext.m_AllocatedBuffers.size()) {
                SAllocatedBufferEntry &theEntry(theContext.m_AllocatedBuffers[bufferIdx]);
                STextureDetails theDetails = theEntry.m_Texture->GetTextureDetails();
                if (theDetails.m_Width == theWidth && theDetails.m_Height == theHeight
                        && theDetails.m_Format == resultFormat) {
                    theBufferTexture = theEntry.m_Texture;
                } else {
                    theContext.ReleaseBuffer(bufferIdx);
                }
            }
        }
        if (theBufferTexture == nullptr) {
            SEffectContext &theContext(GetEffectContext(inEffect));
            auto theFB(m_ResourceManager->AllocateFrameBuffer());
            auto theTexture(
                        m_ResourceManager->AllocateTexture2D(theWidth, theHeight, resultFormat));
            theTexture->SetMagFilter(inCommand.m_FilterOp);
            theTexture->SetMinFilter(
                        static_cast<QDemonRenderTextureMinifyingOp::Enum>(inCommand.m_FilterOp));
            theTexture->SetTextureWrapS(inCommand.m_TexCoordOp);
            theTexture->SetTextureWrapT(inCommand.m_TexCoordOp);
            theFB->Attach(QDemonRenderFrameBufferAttachments::Color0, theTexture);
            theContext.m_AllocatedBuffers.push_back(SAllocatedBufferEntry(
                                                        inCommand.m_Name, *theFB, *theTexture, inCommand.m_BufferFlags));
            theBufferTexture = theTexture;
        }
    }

    void AllocateImage(SEffect &inEffect, const SAllocateImage &inCommand, quint32 inFinalWidth,
                       quint32 inFinalHeight)
    {
        QSharedPointer<QDemonRenderImage2D> theImage;
        quint32 theWidth =
                ITextRenderer::NextMultipleOf4((quint32)(inFinalWidth * inCommand.m_SizeMultiplier));
        quint32 theHeight =
                ITextRenderer::NextMultipleOf4((quint32)(inFinalHeight * inCommand.m_SizeMultiplier));

        Q_ASSERT(inCommand.m_Format != QDemonRenderTextureFormats::Unknown);

        if (inEffect.m_Context) {
            SEffectContext &theContext(*inEffect.m_Context);
            // size intentionally requiried every loop;
            quint32 imageIdx = theContext.FindImage(inCommand.m_Name);
            if (imageIdx < theContext.m_AllocatedImages.size()) {
                SAllocatedImageEntry &theEntry(theContext.m_AllocatedImages[imageIdx]);
                STextureDetails theDetails = theEntry.m_Texture->GetTextureDetails();
                if (theDetails.m_Width == theWidth && theDetails.m_Height == theHeight
                        && theDetails.m_Format == inCommand.m_Format) {
                    theImage = theEntry.m_Image;
                } else {
                    theContext.ReleaseImage(imageIdx);
                }
            }
        }

        if (theImage == nullptr) {
            SEffectContext &theContext(GetEffectContext(inEffect));
            // allocate an immutable texture
            auto theTexture(m_ResourceManager->AllocateTexture2D(
                                                  theWidth, theHeight, inCommand.m_Format, 1, true));
            theTexture->SetMagFilter(inCommand.m_FilterOp);
            theTexture->SetMinFilter(
                        static_cast<QDemonRenderTextureMinifyingOp::Enum>(inCommand.m_FilterOp));
            theTexture->SetTextureWrapS(inCommand.m_TexCoordOp);
            theTexture->SetTextureWrapT(inCommand.m_TexCoordOp);
            auto theImage =
                    (m_ResourceManager->AllocateImage2D(theTexture, inCommand.m_Access));
            theContext.m_AllocatedImages.push_back(SAllocatedImageEntry(
                                                       inCommand.m_Name, *theImage, *theTexture, inCommand.m_BufferFlags));
        }
    }

    void AllocateDataBuffer(SEffect &inEffect, const SAllocateDataBuffer &inCommand)
    {
        quint32 theBufferSize = (quint32)inCommand.m_Size;
        Q_ASSERT(theBufferSize);
        QSharedPointer<QDemonRenderDataBuffer> theDataBuffer;
        QSharedPointer<QDemonRenderDataBuffer> theDataWrapBuffer;

        if (inEffect.m_Context) {
            SEffectContext &theContext(*inEffect.m_Context);
            // size intentionally requiried every loop;
            quint32 bufferIdx = theContext.FindDataBuffer(inCommand.m_Name);
            if (bufferIdx < theContext.m_AllocatedDataBuffers.size()) {
                SAllocatedDataBufferEntry &theEntry(theContext.m_AllocatedDataBuffers[bufferIdx]);
                if (theEntry.m_BufferType == inCommand.m_DataBufferType
                        && theEntry.m_BufferData.size() == theBufferSize) {
                    theDataBuffer = theEntry.m_DataBuffer;
                } else {
                    // if type and size don't match something is wrong
                    Q_ASSERT(false);
                }
            }
        }

        if (theDataBuffer == nullptr) {
            SEffectContext &theContext(GetEffectContext(inEffect));
            auto theRenderContext(m_Context->GetRenderContext());
            quint8 *initialData = (quint8 *)::malloc(theBufferSize);
            QDemonDataRef<quint8> data((quint8 *)initialData, theBufferSize);
            memset(initialData, 0x0L, theBufferSize);
            if (inCommand.m_DataBufferType == QDemonRenderBufferBindValues::Storage) {
                theDataBuffer = theRenderContext->CreateStorageBuffer(
                            inCommand.m_Name.toLocal8Bit(), QDemonRenderBufferUsageType::Dynamic, theBufferSize,
                            data, nullptr);
            } else if (inCommand.m_DataBufferType == QDemonRenderBufferBindValues::Draw_Indirect) {
                Q_ASSERT(theBufferSize == sizeof(DrawArraysIndirectCommand));
                // init a draw call
                quint32 *pIndirectDrawCall = (quint32 *)initialData;
                // vertex count we draw points right now only
                // the rest we fill in by GPU
                pIndirectDrawCall[0] = 1;
                theDataBuffer = theRenderContext->CreateDrawIndirectBuffer(
                            QDemonRenderBufferUsageType::Dynamic, theBufferSize, data);
            } else
                Q_ASSERT(false);

            theContext.m_AllocatedDataBuffers.push_back(SAllocatedDataBufferEntry(
                                                            inCommand.m_Name, *theDataBuffer, inCommand.m_DataBufferType, data,
                                                            inCommand.m_BufferFlags));

            // create wrapper buffer
            if (inCommand.m_DataBufferWrapType == QDemonRenderBufferBindValues::Storage
                    && !inCommand.m_WrapName.isEmpty() && theDataBuffer) {
                theDataWrapBuffer = theRenderContext->CreateStorageBuffer(
                            inCommand.m_WrapName.toLocal8Bit(), QDemonRenderBufferUsageType::Dynamic,
                            theBufferSize, data, theDataBuffer.data());
                theContext.m_AllocatedDataBuffers.push_back(SAllocatedDataBufferEntry(
                                                                inCommand.m_WrapName, *theDataWrapBuffer, inCommand.m_DataBufferWrapType,
                                                                QDemonDataRef<quint8>(), inCommand.m_BufferFlags));
            }
        }
    }

    QSharedPointer<QDemonRenderTexture2D> FindTexture(SEffect *inEffect, QString inName)
    {
        if (inEffect->m_Context) {
            SEffectContext &theContext(*inEffect->m_Context);
            quint32 bufferIdx = theContext.FindBuffer(inName);
            if (bufferIdx < theContext.m_AllocatedBuffers.size()) {
                return theContext.m_AllocatedBuffers[bufferIdx].m_Texture;
            }
        }
        Q_ASSERT(false);
        return nullptr;
    }

    QSharedPointer<QDemonRenderFrameBuffer> BindBuffer(SEffect &inEffect,
                                                       const SBindBuffer &inCommand,
                                                       QMatrix4x4 &outMVP,
                                                       QVector2D &outDestSize)
    {
        QSharedPointer<QDemonRenderFrameBuffer> theBuffer;
        QSharedPointer<QDemonRenderTexture2D> theTexture;
        if (inEffect.m_Context) {
            SEffectContext &theContext(*inEffect.m_Context);
            quint32 bufferIdx = theContext.FindBuffer(inCommand.m_BufferName);
            if (bufferIdx < theContext.m_AllocatedBuffers.size()) {
                theBuffer = theContext.m_AllocatedBuffers[bufferIdx].m_FrameBuffer;
                theTexture = theContext.m_AllocatedBuffers[bufferIdx].m_Texture;
                theContext.m_AllocatedBuffers[bufferIdx].m_NeedsClear = false;
            }
        }
        if (theBuffer == nullptr) {
            qCCritical(INVALID_OPERATION, "Effect %s: Failed to find buffer %s for bind",
                       inEffect.m_ClassName.toLatin1().constData(), inCommand.m_BufferName.toLatin1().constData());
            QString errorMsg = QObject::tr("Failed to compile \"%1\" effect.\nConsider"
                                           " removing it from the presentation.")
                    .arg(qPrintable(inEffect.m_ClassName));
            // TODO:
//            QDEMON_ALWAYS_ASSERT_MESSAGE(errorMsg.toUtf8());
            outMVP = QMatrix4x4();
            return nullptr;
        }

        if (theTexture) {
            SCamera::SetupOrthographicCameraForOffscreenRender(*theTexture, outMVP);
            STextureDetails theDetails(theTexture->GetTextureDetails());
            m_Context->GetRenderContext()->SetViewport(
                        QDemonRenderRect(0, 0, (quint32)theDetails.m_Width, (quint32)theDetails.m_Height));
            outDestSize = QVector2D((float)theDetails.m_Width, (float)theDetails.m_Height);
        }

        return theBuffer;
    }

    QSharedPointer<SEffectShader> BindShader(QString &inEffectId, const SBindShader &inCommand)
    {
        auto theClass = GetEffectClass(inEffectId);
        if (!theClass) {
            Q_ASSERT(false);
            return nullptr;
        }

        bool forceCompilation = theClass->m_DynamicClass->RequiresCompilation();

        auto key = TStrStrPair(inCommand.m_ShaderPath, inCommand.m_ShaderDefine);
        auto theInsertResult = m_ShaderMap.find(key);
        const bool found = (theInsertResult != m_ShaderMap.end());
        if (!found)
            theInsertResult = m_ShaderMap.insert(key, QSharedPointer<SEffectShader>());

        if (found || forceCompilation) {
            auto theProgram = m_Context->GetDynamicObjectSystem()
                    ->GetShaderProgram(inCommand.m_ShaderPath, inCommand.m_ShaderDefine,
                                      TShaderFeatureSet(), SDynamicShaderProgramFlags(),
                                      forceCompilation).first;
            if (theProgram)
                theInsertResult.value() = QSharedPointer<SEffectShader>(new SEffectShader(theProgram));
        }
        if (theInsertResult.value()) {
            auto theContext(m_Context->GetRenderContext());
            theContext->SetActiveShader(theInsertResult.value()->m_Shader);
        }

        return theInsertResult.value();
    }

    void DoApplyInstanceValue(SEffect *inEffect, quint8 *inDataPtr, QString inPropertyName,
                              QDemonRenderShaderDataTypes::Enum inPropertyType,
                              QSharedPointer<QDemonRenderShaderProgram> inShader,
                              const SPropertyDefinition &inDefinition)
    {
        auto theConstant = inShader->GetShaderConstant(inPropertyName.toLocal8Bit());
        if (theConstant) {
            if (theConstant->GetShaderConstantType() == inPropertyType) {
                if (inPropertyType == QDemonRenderShaderDataTypes::Texture2D) {
                    // TODO:
//                    StaticAssert<sizeof(QString) == sizeof(QDemonRenderTexture2DPtr)>::valid_expression();
                    QString theStrPtr = QString::fromLatin1(reinterpret_cast<char *>(inDataPtr));
                    auto theBufferManager(m_Context->GetBufferManager());
                    auto theOffscreenRenderer = m_Context->GetOffscreenRenderManager();
                    bool needsAlphaMultiply = true;
                    QSharedPointer<QDemonRenderTexture2D> theTexture;
                    if (!theStrPtr.isEmpty()) {
                        if (theOffscreenRenderer->HasOffscreenRenderer(theStrPtr)) {
                            SOffscreenRenderResult theResult =
                                    theOffscreenRenderer->GetRenderedItem(theStrPtr);
                            needsAlphaMultiply = false;
                            theTexture = theResult.m_Texture;
                        } else {
                            SImageTextureData theTextureData =
                                    theBufferManager->LoadRenderImage(theStrPtr);
                            needsAlphaMultiply = true;
                            theTexture = theTextureData.m_Texture;
                        }
                    }
                    GetEffectContext(*inEffect).SetTexture(
                                inShader, inPropertyName, theTexture, needsAlphaMultiply,
                                m_TextureStringBuilder, m_TextureStringBuilder2, &inDefinition);
                } else if (inPropertyType == QDemonRenderShaderDataTypes::Image2D) {
                    // TODO:
//                    StaticAssert<sizeof(QString)
//                            == sizeof(QDemonRenderTexture2DPtr)>::valid_expression();
                    QSharedPointer<QDemonRenderImage2D> theImage;
                    GetEffectContext(*inEffect).SetImage(inShader, inPropertyName, theImage);
                } else if (inPropertyType == QDemonRenderShaderDataTypes::DataBuffer) {
                    // we don't handle this here
                } else {
                    switch (inPropertyType) {
                    case QDemonRenderShaderDataTypes::Integer:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<qint32 *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::IntegerVec2:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<qint32_2 *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::IntegerVec3:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<qint32_3 *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::IntegerVec4:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<qint32_4 *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::Boolean:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<bool *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::BooleanVec2:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<bool_2 *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::BooleanVec3:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<bool_3 *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::BooleanVec4:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<bool_4 *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::Float:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<float *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::Vec2:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QVector2D *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::Vec3:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QVector3D *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::Vec4:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QVector4D *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::UnsignedInteger:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<quint32 *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::UnsignedIntegerVec2:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<quint32_2 *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::UnsignedIntegerVec3:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<quint32_3 *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::UnsignedIntegerVec4:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<quint32_4 *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::Matrix3x3:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QMatrix3x3 *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::Matrix4x4:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QMatrix4x4 *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::Texture2D:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderTexture2DPtr *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::Texture2DHandle:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderTexture2DHandle *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::Texture2DArray:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderTexture2DArrayPtr *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::TextureCube:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderTextureCubePtr *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::TextureCubeHandle:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderTextureCubeHandle *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::Image2D:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderImage2DPtr *>(inDataPtr))); break;
                    case QDemonRenderShaderDataTypes::DataBuffer:
                        inShader->SetPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderDataBufferPtr *>(inDataPtr))); break;
                    default:
                        Q_ASSERT(false);
                        break;
                    }
                }

            } else {
                qCCritical(INVALID_OPERATION,
                           "Effect ApplyInstanceValue command datatype "
                           "and shader datatypes differ for property %s",
                           qPrintable(inPropertyName));
                Q_ASSERT(false);
            }
        }
    }

    void ApplyInstanceValue(SEffect *inEffect,
                            QSharedPointer<SEffectClass> inClass,
                            QSharedPointer<QDemonRenderShaderProgram> inShader,
                            const SApplyInstanceValue &inCommand)
    {
        // sanity check
        if (!inCommand.m_PropertyName.isEmpty()) {
            bool canGetData =
                    inCommand.m_ValueOffset + getSizeofShaderDataType(inCommand.m_ValueType)
                    <= inEffect->m_DataSectionByteSize;
            if (canGetData == false) {
                Q_ASSERT(false);
                return;
            }
            quint8 *dataPtr = inEffect->GetDataSectionBegin() + inCommand.m_ValueOffset;
            const SPropertyDefinition *theDefinition =
                    inClass->m_DynamicClass->FindPropertyByName(inCommand.m_PropertyName);
            if (theDefinition)
                DoApplyInstanceValue(inEffect, dataPtr, inCommand.m_PropertyName,
                                     inCommand.m_ValueType, inShader, *theDefinition);
        } else {
            QDemonConstDataRef<SPropertyDefinition> theDefs = inClass->m_DynamicClass->GetProperties();
            for (quint32 idx = 0, end = theDefs.size(); idx < end; ++idx) {
                const SPropertyDefinition &theDefinition(theDefs[idx]);
                auto theConstant = inShader->GetShaderConstant(theDefinition.m_Name.toLatin1());

                // This is fine, the property wasn't found and we continue, no problem.
                if (!theConstant)
                    continue;
                quint8 *dataPtr = inEffect->GetDataSectionBegin() + theDefinition.m_Offset;
                DoApplyInstanceValue(inEffect, dataPtr, theDefinition.m_Name,
                                     theDefinition.m_DataType, inShader, theDefinition);
            }
        }
    }

    void ApplyValue(SEffect *inEffect,
                    QSharedPointer<SEffectClass> inClass,
                    QSharedPointer<QDemonRenderShaderProgram> inShader,
                    const SApplyValue &inCommand)
    {
        if (!inCommand.m_PropertyName.isEmpty()) {
            quint8 *dataPtr = inCommand.m_Value.mData;
            const SPropertyDefinition *theDefinition =
                    inClass->m_DynamicClass->FindPropertyByName(inCommand.m_PropertyName);
            if (theDefinition)
                DoApplyInstanceValue(inEffect, dataPtr, inCommand.m_PropertyName,
                                     inCommand.m_ValueType, inShader, *theDefinition);
        }
    }

    bool ApplyBlending(const SApplyBlending &inCommand)
    {
        auto theContext(m_Context->GetRenderContext());

        theContext->SetBlendingEnabled(true);

        QDemonRenderBlendFunctionArgument blendFunc =
                QDemonRenderBlendFunctionArgument(
                    inCommand.m_SrcBlendFunc, inCommand.m_DstBlendFunc, inCommand.m_SrcBlendFunc,
                    inCommand.m_DstBlendFunc);

        QDemonRenderBlendEquationArgument blendEqu(QDemonRenderBlendEquation::Add,
                                                   QDemonRenderBlendEquation::Add);

        theContext->SetBlendFunction(blendFunc);
        theContext->SetBlendEquation(blendEqu);

        return true;
    }

    // This has the potential to change the source texture for the current render pass
    SEffectTextureData ApplyBufferValue(SEffect *inEffect,
                                        QSharedPointer<QDemonRenderShaderProgram> inShader,
                                        const SApplyBufferValue &inCommand,
                                        QSharedPointer<QDemonRenderTexture2D> inSourceTexture,
                                        SEffectTextureData inCurrentSourceTexture)
    {
        SEffectTextureData theTextureToBind;
        if (!inCommand.m_BufferName.isEmpty()) {
            if (inEffect->m_Context) {
                SEffectContext &theContext(*inEffect->m_Context);
                quint32 bufferIdx = theContext.FindBuffer(inCommand.m_BufferName);
                if (bufferIdx < theContext.m_AllocatedBuffers.size()) {
                    SAllocatedBufferEntry &theEntry(theContext.m_AllocatedBuffers[bufferIdx]);
                    if (theEntry.m_NeedsClear) {
                        auto theRenderContext(m_Context->GetRenderContext());

                        theRenderContext->SetRenderTarget(theEntry.m_FrameBuffer);
                        // Note that depth/stencil buffers need an explicit clear in their bind
                        // commands in order to ensure
                        // we clear the least amount of information possible.

                        if (theEntry.m_Texture) {
                            QDemonRenderTextureFormats::Enum theTextureFormat =
                                    theEntry.m_Texture->GetTextureDetails().m_Format;
                            if (theTextureFormat != QDemonRenderTextureFormats::Depth16
                                    && theTextureFormat != QDemonRenderTextureFormats::Depth24
                                    && theTextureFormat != QDemonRenderTextureFormats::Depth32
                                    && theTextureFormat != QDemonRenderTextureFormats::Depth24Stencil8) {
                                QDemonRenderContextScopedProperty<QVector4D> __clearColor(
                                            *theRenderContext, &QDemonRenderContext::GetClearColor,
                                            &QDemonRenderContext::SetClearColor, QVector4D());
                                theRenderContext->Clear(QDemonRenderClearValues::Color);
                            }
                        }
                        theEntry.m_NeedsClear = false;
                    }
                    theTextureToBind = SEffectTextureData(theEntry.m_Texture, false);
                }
            }
            if (theTextureToBind.m_Texture == nullptr) {
                Q_ASSERT(false);
                qCCritical(INVALID_OPERATION, "Effect %s: Failed to find buffer %s for bind",
                           qPrintable(inEffect->m_ClassName), qPrintable(inCommand.m_BufferName));
                Q_ASSERT(false);
            }
        } else // no name means bind the source
            theTextureToBind = SEffectTextureData(inSourceTexture, false);

        if (!inCommand.m_ParamName.isEmpty()) {
            auto theConstant =
                    inShader->GetShaderConstant(inCommand.m_ParamName.toLatin1());

            if (theConstant) {
                if (theConstant->GetShaderConstantType()
                        != QDemonRenderShaderDataTypes::Texture2D) {
                    qCCritical(INVALID_OPERATION,
                               "Effect %s: Binding buffer to parameter %s that is not a texture",
                               qPrintable(inEffect->m_ClassName), qPrintable(inCommand.m_ParamName));
                    Q_ASSERT(false);
                } else {
                    GetEffectContext(*inEffect).SetTexture(
                                inShader, inCommand.m_ParamName, theTextureToBind.m_Texture,
                                theTextureToBind.m_NeedsAlphaMultiply, m_TextureStringBuilder,
                                m_TextureStringBuilder2);
                }
            }
            return inCurrentSourceTexture;
        } else {
            return theTextureToBind;
        }
    }

    void ApplyDepthValue(SEffect *inEffect,
                         QSharedPointer<QDemonRenderShaderProgram> inShader,
                         const SApplyDepthValue &inCommand,
                         QSharedPointer<QDemonRenderTexture2D> inTexture)
    {
        auto theConstant =
                inShader->GetShaderConstant(inCommand.m_ParamName.toLatin1());

        if (theConstant) {
            if (theConstant->GetShaderConstantType()
                    != QDemonRenderShaderDataTypes::Texture2D) {
                qCCritical(INVALID_OPERATION,
                           "Effect %s: Binding buffer to parameter %s that is not a texture",
                           qPrintable(inEffect->m_ClassName), qPrintable(inCommand.m_ParamName));
                Q_ASSERT(false);
            } else {
                GetEffectContext(*inEffect).SetTexture(inShader, inCommand.m_ParamName, inTexture,
                                                      false, m_TextureStringBuilder,
                                                      m_TextureStringBuilder2);
            }
        }
    }

    void ApplyImageValue(SEffect *inEffect,
                         QSharedPointer<QDemonRenderShaderProgram> inShader,
                         const SApplyImageValue &inCommand)
    {
        SAllocatedImageEntry theImageToBind;
        if (!inCommand.m_ImageName.isEmpty()) {
            if (inEffect->m_Context) {
                SEffectContext &theContext(*inEffect->m_Context);
                quint32 bufferIdx = theContext.FindImage(inCommand.m_ImageName);
                if (bufferIdx < theContext.m_AllocatedImages.size()) {
                    theImageToBind = SAllocatedImageEntry(theContext.m_AllocatedImages[bufferIdx]);
                }
            }
        }

        if (theImageToBind.m_Image == nullptr) {
            qCCritical(INVALID_OPERATION, "Effect %s: Failed to find image %s for bind",
                       qPrintable(inEffect->m_ClassName), qPrintable(inCommand.m_ImageName));
            Q_ASSERT(false);
        }

        if (!inCommand.m_ParamName.isEmpty()) {
            auto theConstant =
                    inShader->GetShaderConstant(inCommand.m_ParamName.toLatin1());

            if (theConstant) {
                if (inCommand.m_NeedSync) {
                    QDemonRenderBufferBarrierFlags flags(
                                QDemonRenderBufferBarrierValues::TextureFetch
                                | QDemonRenderBufferBarrierValues::TextureUpdate);
                    inShader->GetRenderContext()->SetMemoryBarrier(flags);
                }

                if (theConstant->GetShaderConstantType()
                        == QDemonRenderShaderDataTypes::Image2D
                        && !inCommand.m_BindAsTexture) {
                    GetEffectContext(*inEffect).SetImage(inShader, inCommand.m_ParamName,
                                                        theImageToBind.m_Image);
                } else if (theConstant->GetShaderConstantType()
                           == QDemonRenderShaderDataTypes::Texture2D
                           && inCommand.m_BindAsTexture) {
                    GetEffectContext(*inEffect).SetTexture(
                                inShader, inCommand.m_ParamName, theImageToBind.m_Texture, false,
                                m_TextureStringBuilder, m_TextureStringBuilder2);
                } else {
                    qCCritical(INVALID_OPERATION,
                               "Effect %s: Binding buffer to parameter %s that is not a texture",
                               qPrintable(inEffect->m_ClassName), qPrintable(inCommand.m_ParamName));
                    Q_ASSERT(false);
                }
            }
        }
    }

    void ApplyDataBufferValue(SEffect *inEffect,
                              QSharedPointer<QDemonRenderShaderProgram> inShader,
                              const SApplyDataBufferValue &inCommand)
    {
        SAllocatedDataBufferEntry theBufferToBind;
        if (!inCommand.m_ParamName.isEmpty()) {
            if (inEffect->m_Context) {
                SEffectContext &theContext(*inEffect->m_Context);
                quint32 bufferIdx = theContext.FindDataBuffer(inCommand.m_ParamName);
                if (bufferIdx < theContext.m_AllocatedDataBuffers.size()) {
                    theBufferToBind =
                            SAllocatedDataBufferEntry(theContext.m_AllocatedDataBuffers[bufferIdx]);
                    if (theBufferToBind.m_NeedsClear) {
                        QDemonDataRef<quint8> pData = theBufferToBind.m_DataBuffer->MapBuffer();
                        memset(pData.begin(), 0x0L, theBufferToBind.m_BufferData.size());
                        theBufferToBind.m_DataBuffer->UnmapBuffer();
                        theBufferToBind.m_NeedsClear = false;
                    }
                }
            }

            if (theBufferToBind.m_DataBuffer == nullptr) {
                qCCritical(INVALID_OPERATION, "Effect %s: Failed to find buffer %s for bind",
                           qPrintable(inEffect->m_ClassName), qPrintable(inCommand.m_ParamName));
                Q_ASSERT(false);
            }

            auto theConstant =
                    inShader->GetShaderBuffer(inCommand.m_ParamName.toLatin1());

            if (theConstant) {
                GetEffectContext(*inEffect).SetDataBuffer(inShader, inCommand.m_ParamName,
                                                         theBufferToBind.m_DataBuffer);
            } else if (theBufferToBind.m_BufferType
                       == QDemonRenderBufferBindValues::Draw_Indirect) {
                // since we filled part of this buffer on the GPU we need a sync before usage
                QDemonRenderBufferBarrierFlags flags(
                            QDemonRenderBufferBarrierValues::CommandBuffer);
                inShader->GetRenderContext()->SetMemoryBarrier(flags);
            }
        }
    }

    void ApplyRenderStateValue(QDemonRenderFrameBuffer *inTarget,
                               QSharedPointer<QDemonRenderTexture2D> inDepthStencilTexture,
                               const SApplyRenderState &theCommand)
    {
        auto theContext(m_Context->GetRenderContext());
        quint32 inState = (quint32)theCommand.m_RenderState;
        bool inEnable = theCommand.m_Enabled;

        switch (inState) {
        case QDemonRenderState::StencilTest: {
            if (inEnable && inTarget) {
                inTarget->Attach(QDemonRenderFrameBufferAttachments::DepthStencil,
                                 inDepthStencilTexture);
            } else if (inTarget) {
                inTarget->Attach(QDemonRenderFrameBufferAttachments::DepthStencil,
                                 QDemonRenderTextureOrRenderBuffer());
            }

            theContext->SetStencilTestEnabled(inEnable);
        } break;
        default:
            Q_ASSERT(false);
            break;
        }
    }

    static bool CompareDepthStencilState(QDemonRenderDepthStencilState &inState,
                                         SDepthStencil &inStencil)
    {
        QDemonRenderStencilFunctionArgument theFunction =
                inState.GetStencilFunc(QDemonRenderFaces::Front);
        QDemonRenderStencilOperationArgument theOperation =
                inState.GetStencilOp(QDemonRenderFaces::Front);

        return theFunction.m_Function == inStencil.m_StencilFunction
                && theFunction.m_Mask == inStencil.m_Mask
                && theFunction.m_ReferenceValue == inStencil.m_Reference
                && theOperation.m_StencilFail == inStencil.m_StencilFailOperation
                && theOperation.m_DepthFail == inStencil.m_DepthFailOperation
                && theOperation.m_DepthPass == inStencil.m_DepthPassOperation;
    }

    void RenderPass(SEffectShader &inShader,
                    const QMatrix4x4 &inMVP,
                    SEffectTextureData inSourceTexture,
                    QSharedPointer<QDemonRenderFrameBuffer> inFrameBuffer,
                    QVector2D &inDestSize,
                    const QVector2D &inCameraClipRange,
                    QSharedPointer<QDemonRenderTexture2D> inDepthStencil,
                    QDemonOption<SDepthStencil> inDepthStencilCommand,
                    bool drawIndirect)
    {
        auto theContext(m_Context->GetRenderContext());
        theContext->SetRenderTarget(inFrameBuffer);
        if (inDepthStencil && inFrameBuffer) {
            inFrameBuffer->Attach(QDemonRenderFrameBufferAttachments::DepthStencil, inDepthStencil);
            if (inDepthStencilCommand.hasValue()) {
                SDepthStencil &theDepthStencil(*inDepthStencilCommand);
                quint32 clearFlags = 0;
                if (theDepthStencil.m_Flags.HasClearStencil())
                    clearFlags |= QDemonRenderClearValues::Stencil;
                if (theDepthStencil.m_Flags.HasClearDepth())
                    clearFlags |= QDemonRenderClearValues::Depth;

                if (clearFlags)
                    theContext->Clear(QDemonRenderClearFlags(clearFlags));

                QSharedPointer<QDemonRenderDepthStencilState> targetState;
                for (quint32 idx = 0, end = m_DepthStencilStates.size();
                     idx < end && targetState == nullptr; ++idx) {
                    QSharedPointer<QDemonRenderDepthStencilState> theState = m_DepthStencilStates[idx];
                    if (CompareDepthStencilState(*theState, theDepthStencil))
                        targetState = theState;
                }
                if (targetState == nullptr) {
                    QDemonRenderStencilFunctionArgument theFunctionArg(
                                theDepthStencil.m_StencilFunction, theDepthStencil.m_Reference,
                                theDepthStencil.m_Mask);
                    QDemonRenderStencilOperationArgument theOpArg(
                                theDepthStencil.m_StencilFailOperation,
                                theDepthStencil.m_DepthFailOperation, theDepthStencil.m_DepthPassOperation);
                    targetState = theContext->CreateDepthStencilState(
                                theContext->IsDepthTestEnabled(), theContext->IsDepthWriteEnabled(),
                                theContext->GetDepthFunction(), true, theFunctionArg, theFunctionArg,
                                theOpArg, theOpArg);
                    m_DepthStencilStates.push_back(targetState);
                }
                theContext->SetDepthStencilState(targetState);
            }
        }

        theContext->SetActiveShader(inShader.m_Shader);
        inShader.m_MVP.Set(inMVP);
        if (inSourceTexture.m_Texture) {
            inShader.m_TextureEntry.Set(inSourceTexture.m_Texture,
                                        inSourceTexture.m_NeedsAlphaMultiply, nullptr);
        } else {
            qCCritical(INTERNAL_ERROR, "Failed to setup pass due to null source texture");
            Q_ASSERT(false);
        }
        inShader.m_FragColorAlphaSettings.Set(QVector2D(1.0f, 0.0f));
        inShader.m_DestSize.Set(inDestSize);
        if (inShader.m_AppFrame.IsValid())
            inShader.m_AppFrame.Set((float)m_Context->GetFrameCount());
        if (inShader.m_FPS.IsValid())
            inShader.m_FPS.Set((float)m_Context->GetFPS().first);
        if (inShader.m_CameraClipRange.IsValid())
            inShader.m_CameraClipRange.Set(inCameraClipRange);

        if (!drawIndirect)
            m_Context->GetRenderer()->RenderQuad();
        else
            m_Context->GetRenderer()->RenderPointsIndirect();

        if (inDepthStencil && inFrameBuffer) {
            inFrameBuffer->Attach(QDemonRenderFrameBufferAttachments::DepthStencil,
                                  QDemonRenderTextureOrRenderBuffer());
            theContext->SetDepthStencilState(m_DefaultStencilState);
        }
    }

    void DoRenderEffect(SEffect *inEffect,
                        QSharedPointer<SEffectClass> inClass,
                        QSharedPointer<QDemonRenderTexture2D> inSourceTexture,
                        QMatrix4x4 &inMVP,
                        QSharedPointer<QDemonRenderFrameBuffer> inTarget,
                        bool inEnableBlendWhenRenderToTarget,
                        QSharedPointer<QDemonRenderTexture2D> inDepthTexture,
                        QSharedPointer<QDemonRenderTexture2D> inDepthStencilTexture,
                        const QVector2D inCameraClipRange)
    {
        // Run through the effect commands and render the effect.
        // QDemonRenderTexture2D* theCurrentTexture(&inSourceTexture);
        auto theContext = m_Context->GetRenderContext();

        // Context variables that are updated during the course of a pass.
        SEffectTextureData theCurrentSourceTexture(inSourceTexture, false);
        QSharedPointer<QDemonRenderTexture2D> theCurrentDepthStencilTexture;
        QSharedPointer<QDemonRenderFrameBuffer> theCurrentRenderTarget(inTarget);
        QSharedPointer<SEffectShader> theCurrentShader;
        QDemonRenderRect theOriginalViewport(theContext->GetViewport());
        bool wasScissorEnabled = theContext->IsScissorTestEnabled();
        bool wasBlendingEnabled = theContext->IsBlendingEnabled();
        // save current blending setup
        QDemonRenderBlendFunctionArgument theBlendFunc = theContext->GetBlendFunction();
        QDemonRenderBlendEquationArgument theBlendEqu = theContext->GetBlendEquation();
        bool intermediateBlendingEnabled = false;
        STextureDetails theDetails(inSourceTexture->GetTextureDetails());
        quint32 theFinalWidth = (quint32)(theDetails.m_Width);
        quint32 theFinalHeight = (quint32)(theDetails.m_Height);
        QVector2D theDestSize;
        {
            // Ensure no matter the command run goes we replace the rendering system to some
            // semblance of the approprate
            // setting.
            QDemonRenderContextScopedProperty<QSharedPointer<QDemonRenderFrameBuffer>> __framebuffer(
                        *theContext, &QDemonRenderContext::GetRenderTarget, &QDemonRenderContext::SetRenderTarget);
            QDemonRenderContextScopedProperty<QDemonRenderRect> __viewport(
                        *theContext, &QDemonRenderContext::GetViewport, &QDemonRenderContext::SetViewport);
            QDemonRenderContextScopedProperty<bool> __scissorEnabled(
                        *theContext, &QDemonRenderContext::IsScissorTestEnabled,
                        &QDemonRenderContext::SetScissorTestEnabled);
            QDemonRenderContextScopedProperty<bool> __stencilTest(
                        *theContext, &QDemonRenderContext::IsStencilTestEnabled,
                        &QDemonRenderContext::SetStencilTestEnabled);
            QDemonRenderContextScopedProperty<QDemonRenderBoolOp::Enum> __depthFunction(
                        *theContext, &QDemonRenderContext::GetDepthFunction, &QDemonRenderContext::SetDepthFunction);
            QDemonOption<SDepthStencil> theCurrentDepthStencil;

            theContext->SetScissorTestEnabled(false);
            theContext->SetBlendingEnabled(false);
            theContext->SetCullingEnabled(false);
            theContext->SetDepthTestEnabled(false);
            theContext->SetDepthWriteEnabled(false);

            QMatrix4x4 theMVP;
            QDemonConstDataRef<dynamic::SCommand *> theCommands =
                    inClass->m_DynamicClass->GetRenderCommands();
            for (quint32 commandIdx = 0, commandEnd = theCommands.size(); commandIdx < commandEnd;
                 ++commandIdx) {
                const SCommand &theCommand(*theCommands[commandIdx]);
                switch (theCommand.m_Type) {
                case CommandTypes::AllocateBuffer:
                    AllocateBuffer(*inEffect, static_cast<const SAllocateBuffer &>(theCommand),
                                   theFinalWidth, theFinalHeight, theDetails.m_Format);
                    break;

                case CommandTypes::AllocateImage:
                    AllocateImage(*inEffect, static_cast<const SAllocateImage &>(theCommand),
                                  theFinalWidth, theFinalHeight);
                    break;

                case CommandTypes::AllocateDataBuffer:
                    AllocateDataBuffer(*inEffect,
                                       static_cast<const SAllocateDataBuffer &>(theCommand));
                    break;

                case CommandTypes::BindBuffer:
                    theCurrentRenderTarget =
                            BindBuffer(*inEffect, static_cast<const SBindBuffer &>(theCommand), theMVP,
                                       theDestSize);
                    break;

                case CommandTypes::BindTarget: {
                    m_Context->GetRenderContext()->SetRenderTarget(inTarget);
                    theCurrentRenderTarget = inTarget;
                    theMVP = inMVP;
                    theContext->SetViewport(theOriginalViewport);
                    theDestSize = QVector2D((float)theFinalWidth, (float)theFinalHeight);
                    // This isn't necessary if we are rendering to an offscreen buffer and not
                    // compositing
                    // with other objects.
                    if (inEnableBlendWhenRenderToTarget) {
                        theContext->SetBlendingEnabled(wasBlendingEnabled);
                        theContext->SetScissorTestEnabled(wasScissorEnabled);
                        // The blending setup was done before we apply the effect
                        theContext->SetBlendFunction(theBlendFunc);
                        theContext->SetBlendEquation(theBlendEqu);
                    }
                } break;
                case CommandTypes::BindShader:
                    theCurrentShader = BindShader(inEffect->m_ClassName,
                                                  static_cast<const SBindShader &>(theCommand));
                    break;
                case CommandTypes::ApplyInstanceValue:
                    if (theCurrentShader)
                        ApplyInstanceValue(inEffect, inClass, theCurrentShader->m_Shader,
                                           static_cast<const SApplyInstanceValue &>(theCommand));
                    break;
                case CommandTypes::ApplyValue:
                    if (theCurrentShader)
                        ApplyValue(inEffect, inClass, theCurrentShader->m_Shader,
                                   static_cast<const SApplyValue &>(theCommand));
                    break;
                case CommandTypes::ApplyBlending:
                    intermediateBlendingEnabled =
                            ApplyBlending(static_cast<const SApplyBlending &>(theCommand));
                    break;
                case CommandTypes::ApplyBufferValue:
                    if (theCurrentShader)
                        theCurrentSourceTexture =
                                ApplyBufferValue(inEffect, theCurrentShader->m_Shader,
                                                 static_cast<const SApplyBufferValue &>(theCommand),
                                                 inSourceTexture, theCurrentSourceTexture);
                    break;
                case CommandTypes::ApplyDepthValue:
                    if (theCurrentShader)
                        ApplyDepthValue(inEffect, theCurrentShader->m_Shader,
                                        static_cast<const SApplyDepthValue &>(theCommand),
                                        inDepthTexture);
                    if (!inDepthTexture) {
                        qCCritical(INVALID_OPERATION,
                                   "Depth value command detected but no "
                                   "depth buffer provided for effect %s",
                                   qPrintable(inEffect->m_ClassName));
                        Q_ASSERT(false);
                    }
                    break;
                case CommandTypes::ApplyImageValue:
                    if (theCurrentShader)
                        ApplyImageValue(inEffect, theCurrentShader->m_Shader,
                                        static_cast<const SApplyImageValue &>(theCommand));
                    break;
                case CommandTypes::ApplyDataBufferValue:
                    if (theCurrentShader)
                        ApplyDataBufferValue(
                                    inEffect, theCurrentShader->m_Shader,
                                    static_cast<const SApplyDataBufferValue &>(theCommand));
                    break;
                case CommandTypes::DepthStencil: {
                    const SDepthStencil &theDepthStencil =
                            static_cast<const SDepthStencil &>(theCommand);
                    theCurrentDepthStencilTexture =
                            FindTexture(inEffect, theDepthStencil.m_BufferName);
                    if (theCurrentDepthStencilTexture)
                        theCurrentDepthStencil = theDepthStencil;
                } break;
                case CommandTypes::Render:
                    if (theCurrentShader && theCurrentSourceTexture.m_Texture) {
                        RenderPass(*theCurrentShader, theMVP, theCurrentSourceTexture,
                                   theCurrentRenderTarget, theDestSize, inCameraClipRange,
                                   theCurrentDepthStencilTexture, theCurrentDepthStencil,
                                   static_cast<const SRender &>(theCommand).m_DrawIndirect);
                    }
                    // Reset the source texture regardless
                    theCurrentSourceTexture = SEffectTextureData(inSourceTexture, false);
                    theCurrentDepthStencilTexture = nullptr;
                    theCurrentDepthStencil = QDemonOption<SDepthStencil>();
                    // reset intermediate blending state
                    if (intermediateBlendingEnabled) {
                        theContext->SetBlendingEnabled(false);
                        intermediateBlendingEnabled = false;
                    }
                    break;
                case CommandTypes::ApplyRenderState:
                    ApplyRenderStateValue(theCurrentRenderTarget.data(), inDepthStencilTexture,
                                          static_cast<const SApplyRenderState &>(theCommand));
                    break;
                default:
                    Q_ASSERT(false);
                    break;
                }
            }

            SetEffectRequiresCompilation(inEffect->m_ClassName, false);

            // reset to default stencil state
            if (inDepthStencilTexture) {
                theContext->SetDepthStencilState(m_DefaultStencilState);
            }

            // Release any per-frame buffers
            if (inEffect->m_Context) {
                SEffectContext &theContext(*inEffect->m_Context);
                // Query for size on every loop intentional
                for (quint32 idx = 0; idx < theContext.m_AllocatedBuffers.size(); ++idx) {
                    if (theContext.m_AllocatedBuffers[idx].m_Flags.IsSceneLifetime() == false) {
                        theContext.ReleaseBuffer(idx);
                        --idx;
                    }
                }
                for (quint32 idx = 0; idx < theContext.m_AllocatedImages.size(); ++idx) {
                    if (theContext.m_AllocatedImages[idx].m_Flags.IsSceneLifetime() == false) {
                        theContext.ReleaseImage(idx);
                        --idx;
                    }
                }
            }
        }
    }

    QSharedPointer<QDemonRenderTexture2D> RenderEffect(SEffectRenderArgument inRenderArgument) override
    {
        auto theClass = GetEffectClass(inRenderArgument.m_Effect->m_ClassName);
        if (!theClass) {
            Q_ASSERT(false);
            return nullptr;
        }
        QMatrix4x4 theMVP;
        SCamera::SetupOrthographicCameraForOffscreenRender(*inRenderArgument.m_ColorBuffer, theMVP);
        // setup a render target
        auto theContext(m_Context->GetRenderContext());
        auto theManager(m_Context->GetResourceManager());
        QDemonRenderContextScopedProperty<QSharedPointer<QDemonRenderFrameBuffer>> __framebuffer(
                    *theContext, &QDemonRenderContext::GetRenderTarget, &QDemonRenderContext::SetRenderTarget);
        STextureDetails theDetails(inRenderArgument.m_ColorBuffer->GetTextureDetails());
        quint32 theFinalWidth = ITextRenderer::NextMultipleOf4((quint32)(theDetails.m_Width));
        quint32 theFinalHeight = ITextRenderer::NextMultipleOf4((quint32)(theDetails.m_Height));
        auto theBuffer = theManager->AllocateFrameBuffer();
        // UdoL Some Effects may need to run before HDR tonemap. This means we need to keep the
        // input format
        QDemonRenderTextureFormats::Enum theOutputFormat = QDemonRenderTextureFormats::RGBA8;
        if (theClass->m_DynamicClass->GetOutputTextureFormat() == QDemonRenderTextureFormats::Unknown)
            theOutputFormat = theDetails.m_Format;
        auto theTargetTexture =
                theManager->AllocateTexture2D(theFinalWidth, theFinalHeight, theOutputFormat);
        theBuffer->Attach(QDemonRenderFrameBufferAttachments::Color0, theTargetTexture);
        theContext->SetRenderTarget(theBuffer);
        QDemonRenderContextScopedProperty<QDemonRenderRect> __viewport(
                    *theContext, &QDemonRenderContext::GetViewport, &QDemonRenderContext::SetViewport,
                    QDemonRenderRect(0, 0, theFinalWidth, theFinalHeight));

        QDemonRenderContextScopedProperty<bool> __scissorEnable(
                    *theContext, &QDemonRenderContext::IsScissorTestEnabled,
                    &QDemonRenderContext::SetScissorTestEnabled, false);

        DoRenderEffect(inRenderArgument.m_Effect, theClass, inRenderArgument.m_ColorBuffer, theMVP,
                       m_Context->GetRenderContext()->GetRenderTarget(), false,
                       inRenderArgument.m_DepthTexture, inRenderArgument.m_DepthStencilBuffer,
                       inRenderArgument.m_CameraClipRange);

        theBuffer->Attach(QDemonRenderFrameBufferAttachments::Color0, QDemonRenderTextureOrRenderBuffer());
        theManager->Release(theBuffer);
        return theTargetTexture;
    }

    // Render the effect to the currently bound render target using this MVP
    bool RenderEffect(SEffectRenderArgument inRenderArgument, QMatrix4x4 &inMVP,
                      bool inEnableBlendWhenRenderToTarget) override
    {
        auto theClass = GetEffectClass(inRenderArgument.m_Effect->m_ClassName);
        if (!theClass) {
            Q_ASSERT(false);
            return false;
        }

        DoRenderEffect(inRenderArgument.m_Effect, theClass, inRenderArgument.m_ColorBuffer, inMVP,
                       m_Context->GetRenderContext()->GetRenderTarget(),
                       inEnableBlendWhenRenderToTarget, inRenderArgument.m_DepthTexture,
                       inRenderArgument.m_DepthStencilBuffer, inRenderArgument.m_CameraClipRange);
        return true;
    }

    void ReleaseEffectContext(SEffectContext *inContext) override
    {
        if (inContext == nullptr)
            return;
        for (quint32 idx = 0, end = m_Contexts.size(); idx < end; ++idx) {
            if (m_Contexts[idx] == inContext) {
                { // replace_with_last
                    m_Contexts[idx] = m_Contexts.back();
                    m_Contexts.pop_back();
                }
            }
        }
    }

    void ResetEffectFrameData(SEffectContext &inContext) override
    { // Query for size on every loop intentional
        for (quint32 idx = 0; idx < inContext.m_AllocatedBuffers.size(); ++idx) {
            SAllocatedBufferEntry &theBuffer(inContext.m_AllocatedBuffers[idx]);
            if (theBuffer.m_Flags.IsSceneLifetime() == true)
                theBuffer.m_NeedsClear = true;
        }
        for (quint32 idx = 0; idx < inContext.m_AllocatedDataBuffers.size(); ++idx) {
            SAllocatedDataBufferEntry &theDataBuffer(inContext.m_AllocatedDataBuffers[idx]);
            if (theDataBuffer.m_Flags.IsSceneLifetime() == true)
                theDataBuffer.m_NeedsClear = true;
        }
    }

    void SetShaderData(QString path, const char *data,
                       const char *inShaderType, const char *inShaderVersion,
                       bool inHasGeomShader, bool inIsComputeShader) override
    {
        m_CoreContext->GetDynamicObjectSystemCore()->SetShaderData(
                    path, data, inShaderType, inShaderVersion, inHasGeomShader, inIsComputeShader);
    }

//    void Save(SWriteBuffer &ioBuffer,
//              const SStrRemapMap &inRemapMap,
//              const char *inProjectDir) const override
//    {
//        ioBuffer.write((quint32)m_EffectClasses.size());
//        SStringSaveRemapper theRemapper(m_Allocator, inRemapMap, inProjectDir,
//                                        m_CoreContext.GetStringTable());
//        for (TEffectClassMap::const_iterator theIter = m_EffectClasses.begin(),
//             end = m_EffectClasses.end();
//             theIter != end; ++theIter) {
//            const SEffectClass &theClass = *theIter->second;
//            QString theClassName = theClass.m_DynamicClass->GetId();
//            theClassName.Remap(inRemapMap);
//            ioBuffer.write(theClassName);
//            // Effect classes do not store any additional data from the dynamic object class.
//            ioBuffer.write(theClass);
//        }
//    }

//    void Load(QDemonDataRef<quint8> inData, CStrTableOrDataRef inStrDataBlock,
//              const char *inProjectDir) override
//    {
//        m_Allocator.m_PreAllocatedBlock = inData;
//        m_Allocator.m_OwnsMemory = false;
//        SDataReader theReader(inData.begin(), inData.end());
//        quint32 numEffectClasses = theReader.LoadRef<quint32>();
//        SStringLoadRemapper theRemapper(m_Allocator, inStrDataBlock, inProjectDir,
//                                        m_CoreContext.GetStringTable());
//        for (quint32 idx = 0, end = numEffectClasses; idx < end; ++idx) {
//            QString theClassName = theReader.LoadRef<QString>();
//            theClassName.Remap(inStrDataBlock);
//            IDynamicObjectClass *theBaseClass =
//                    m_CoreContext.GetDynamicObjectSystemCore().GetDynamicObjectClass(theClassName);
//            if (theBaseClass == nullptr) {
//                Q_ASSERT(false);
//                return;
//            }
//            SEffectClass *theClass = theReader.Load<SEffectClass>();
//            theClass->SetupThisObjectFromMemory(m_Allocator, *theBaseClass);
//            QSharedPointer<SEffectClass> theClassPtr(theClass);
//            m_EffectClasses.insert(theBaseClass->GetId(), theClassPtr);
//        }
//    }

    QSharedPointer<IEffectSystem> GetEffectSystem(IQDemonRenderContext *context) override
    {
        m_Context = context;

        auto theContext(m_Context->GetRenderContext());

        m_ResourceManager = IResourceManager::CreateResourceManager(theContext);

        // create default stencil state
        QDemonRenderStencilFunctionArgument stencilDefaultFunc(
                    QDemonRenderBoolOp::AlwaysTrue, 0x0, 0xFF);
        QDemonRenderStencilOperationArgument stencilDefaultOp(
                    QDemonRenderStencilOp::Keep, QDemonRenderStencilOp::Keep,
                    QDemonRenderStencilOp::Keep);
        m_DefaultStencilState = theContext->CreateDepthStencilState(
                    theContext->IsDepthTestEnabled(), theContext->IsDepthWriteEnabled(),
                    theContext->GetDepthFunction(), theContext->IsStencilTestEnabled(), stencilDefaultFunc,
                    stencilDefaultFunc, stencilDefaultOp, stencilDefaultOp);

        return sharedFromThis();
    }

    QSharedPointer<IResourceManager> GetResourceManager() override
    {
        return m_ResourceManager;
    }
};
}

IEffectSystemCore::~IEffectSystemCore()
{

}

QSharedPointer<IEffectSystemCore> IEffectSystemCore::CreateEffectSystemCore(IQDemonRenderContextCore * inContext)
{
    return QSharedPointer<SEffectSystem>(new SEffectSystem(inContext));
}

QT_END_NAMESPACE
