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
#include <QtDemonRuntimeRender/qdemonrenderstring.h>
#include <QtDemonRuntimeRender/qdemonrendereffect.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcemanager.h>
#include <qdemonrenderdynamicobjectsystemcommands.h>
#include <QtDemonRender/qdemonrenderframebuffer.h>
#include <Qt3DSRenderShaderConstant.h>
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
using QDemonRenderContextScopedProperty;
using NVRenderCachedShaderProperty;
using NVRenderCachedShaderBuffer;

// None of this code will work if the size of void* changes because that would mean that
// the alignment of some of the objects isn't 4 bytes but would be 8 bytes.

typedef eastl::pair<CRegisteredString, CRegisteredString> TStrStrPair;

namespace eastl {
template <>
struct hash<TStrStrPair>
{
    size_t operator()(const TStrStrPair &item) const
    {
        return hash<CRegisteredString>()(item.first) ^ hash<CRegisteredString>()(item.second);
    }
};
}

namespace {

/*
                ApplyBufferValue,
                //Apply the depth buffer as an input texture.
                ApplyDepthValue,
                Render, //Render to current FBO
                */

struct SEffectClass
{
    NVAllocatorCallback *m_Allocator;
    IDynamicObjectClass *m_DynamicClass;
    volatile qint32 mRefCount;

    SEffectClass(NVAllocatorCallback &inFnd, IDynamicObjectClass &dynClass)
        : m_Allocator(&inFnd)
        , m_DynamicClass(&dynClass)
        , mRefCount(0)
    {
    }

    QDEMON_IMPLEMENT_REF_COUNT_ADDREF_RELEASE(*m_Allocator)

    void SetupThisObjectFromMemory(NVAllocatorCallback &inAlloc, IDynamicObjectClass &inClass)
    {
        m_Allocator = &inAlloc;
        m_DynamicClass = &inClass;
        mRefCount = 0;
    }
};

struct SAllocatedBufferEntry
{
    CRegisteredString m_Name;
    QDemonScopedRefCounted<QDemonRenderFrameBuffer> m_FrameBuffer;
    QDemonScopedRefCounted<QDemonRenderTexture2D> m_Texture;
    SAllocateBufferFlags m_Flags;
    bool m_NeedsClear;

    SAllocatedBufferEntry(CRegisteredString inName, QDemonRenderFrameBuffer &inFb,
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
    CRegisteredString m_Name;
    QDemonScopedRefCounted<QDemonRenderImage2D> m_Image;
    QDemonScopedRefCounted<QDemonRenderTexture2D> m_Texture;
    SAllocateBufferFlags m_Flags;

    SAllocatedImageEntry(CRegisteredString inName, QDemonRenderImage2D &inImage,
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
    QDemonScopedRefCounted<QDemonRenderShaderProgram> m_Shader;
    NVRenderCachedShaderProperty<QDemonRenderImage2D *> m_Image;
    volatile qint32 mRefCount;

    SImageEntry(QDemonRenderShaderProgram &inShader, const char *inImageName)
        : m_Shader(inShader)
        , m_Image(inImageName, inShader)
        , mRefCount(0)
    {
    }

    QDEMON_IMPLEMENT_REF_COUNT_ADDREF_RELEASE(m_Shader->GetRenderContext().GetAllocator())

    void Set(QDemonRenderImage2D *inImage) { m_Image.Set(inImage); }

    static SImageEntry CreateImageEntry(QDemonRenderShaderProgram &inShader, const char *inStem)
    {
        return SImageEntry(inShader, inStem);
    }
};

struct SAllocatedDataBufferEntry
{
    CRegisteredString m_Name;
    QDemonScopedRefCounted<QDemonRenderDataBuffer> m_DataBuffer;
    QDemonRenderBufferBindValues::Enum m_BufferType;
    QDemonDataRef<quint8> m_BufferData;
    SAllocateBufferFlags m_Flags;
    bool m_NeedsClear;

    SAllocatedDataBufferEntry(CRegisteredString inName,
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
    QDemonScopedRefCounted<QDemonRenderShaderProgram> m_Shader;
    NVRenderCachedShaderBuffer<QDemonRenderShaderBufferBase *> m_DataBuffer;
    volatile qint32 mRefCount;

    SDataBufferEntry(QDemonRenderShaderProgram &inShader, const char *inBufferName)
        : m_Shader(inShader)
        , m_DataBuffer(inBufferName, inShader)
        , mRefCount(0)
    {
    }

    QDEMON_IMPLEMENT_REF_COUNT_ADDREF_RELEASE(m_Shader->GetRenderContext().GetAllocator())

    void Set(QDemonRenderDataBuffer *inBuffer)
    {
        if (inBuffer)
            inBuffer->Bind();

        m_DataBuffer.Set();
    }

    static SDataBufferEntry CreateDataBufferEntry(QDemonRenderShaderProgram &inShader,
                                                  const char *inStem)
    {
        return SDataBufferEntry(inShader, inStem);
    }
};

struct SEffectTextureData
{
    QDemonRenderTexture2D *m_Texture;
    bool m_NeedsAlphaMultiply;
    SEffectTextureData(QDemonRenderTexture2D *inTexture, bool inNeedsMultiply)
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
    QDemonScopedRefCounted<QDemonRenderShaderProgram> m_Shader;
    NVRenderCachedShaderProperty<QDemonRenderTexture2D *> m_Texture;
    NVRenderCachedShaderProperty<QVector4D> m_TextureData;
    NVRenderCachedShaderProperty<qint32> m_TextureFlags;
    volatile qint32 mRefCount;

    STextureEntry(QDemonRenderShaderProgram &inShader, const char *inTexName, const char *inDataName,
                  const char *inFlagName)
        : m_Shader(inShader)
        , m_Texture(inTexName, inShader)
        , m_TextureData(inDataName, inShader)
        , m_TextureFlags(inFlagName, inShader)
        , mRefCount(0)
    {
    }

    QDEMON_IMPLEMENT_REF_COUNT_ADDREF_RELEASE(m_Shader->GetRenderContext().GetAllocator())

    void Set(QDemonRenderTexture2D *inTexture, bool inNeedsAlphaMultiply,
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
        m_Texture.Set(inTexture);
        if (inTexture) {
            STextureDetails theDetails(inTexture->GetTextureDetails());
            m_TextureData.Set(
                        QVector4D((float)theDetails.m_Width, (float)theDetails.m_Height, theMixValue, 0.0f));
            // I have no idea what these flags do.
            m_TextureFlags.Set(1);
        } else
            m_TextureFlags.Set(0);
    }

    static STextureEntry CreateTextureEntry(QDemonRenderShaderProgram &inShader, const char *inStem,
                                            CRenderString &inBuilder, CRenderString &inBuilder2)
    {
        inBuilder.assign(inStem);
        inBuilder.append("Info");
        inBuilder2.assign("flag");
        inBuilder2.append(inStem);
        return STextureEntry(inShader, inStem, inBuilder.c_str(), inBuilder2.c_str());
    }
};

typedef eastl::pair<CRegisteredString, QDemonScopedRefCounted<STextureEntry>> TNamedTextureEntry;
typedef eastl::pair<CRegisteredString, QDemonScopedRefCounted<SImageEntry>> TNamedImageEntry;
typedef eastl::pair<CRegisteredString, QDemonScopedRefCounted<SDataBufferEntry>> TNamedDataBufferEntry;
}

namespace qt3ds {
namespace render {

struct SEffectContext
{
    CRegisteredString m_ClassName;
    IQt3DSRenderContext &m_Context;
    IResourceManager *m_ResourceManager;
    QVector<SAllocatedBufferEntry> m_AllocatedBuffers;
    QVector<SAllocatedImageEntry> m_AllocatedImages;
    QVector<SAllocatedDataBufferEntry> m_AllocatedDataBuffers;
    QVector<TNamedTextureEntry> m_TextureEntries;
    QVector<TNamedImageEntry> m_ImageEntries;
    QVector<TNamedDataBufferEntry> m_DataBufferEntries;

    SEffectContext(CRegisteredString inName, IQt3DSRenderContext &ctx, IResourceManager *inManager)
        : m_ClassName(inName)
        , m_Context(ctx)
        , m_ResourceManager(inManager)
        , m_AllocatedBuffers(ctx.GetAllocator(), "SEffectContext::m_AllocatedBuffers")
        , m_AllocatedImages(ctx.GetAllocator(), "SEffectContext::m_AllocatedImages")
        , m_AllocatedDataBuffers(ctx.GetAllocator(), "SEffectContext::m_AllocatedDataBuffers")
        , m_TextureEntries(ctx.GetAllocator(), "SEffectContext::m_TextureEntries")
        , m_ImageEntries(ctx.GetAllocator(), "SEffectContext::m_ImageEntries")
        , m_DataBufferEntries(ctx.GetAllocator(), "SEffectContext::m_DataBufferEntries")
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
        m_ResourceManager->Release(*theEntry.m_FrameBuffer);
        m_ResourceManager->Release(*theEntry.m_Texture);
        m_AllocatedBuffers.replace_with_last(inIdx);
    }

    void ReleaseImage(quint32 inIdx)
    {
        SAllocatedImageEntry &theEntry(m_AllocatedImages[inIdx]);
        m_ResourceManager->Release(*theEntry.m_Image);
        m_ResourceManager->Release(*theEntry.m_Texture);
        m_AllocatedImages.replace_with_last(inIdx);
    }

    void ReleaseDataBuffer(quint32 inIdx)
    {
        SAllocatedDataBufferEntry &theEntry(m_AllocatedDataBuffers[inIdx]);
        m_Context.GetAllocator().deallocate(theEntry.m_BufferData.begin());

        m_AllocatedDataBuffers.replace_with_last(inIdx);
    }

    quint32 FindBuffer(CRegisteredString inName)
    {
        for (quint32 idx = 0, end = m_AllocatedBuffers.size(); idx < end; ++idx)
            if (m_AllocatedBuffers[idx].m_Name == inName)
                return idx;
        return m_AllocatedBuffers.size();
    }

    quint32 FindImage(CRegisteredString inName)
    {
        for (quint32 idx = 0, end = m_AllocatedImages.size(); idx < end; ++idx)
            if (m_AllocatedImages[idx].m_Name == inName)
                return idx;

        return m_AllocatedImages.size();
    }

    quint32 FindDataBuffer(CRegisteredString inName)
    {
        for (quint32 idx = 0, end = m_AllocatedDataBuffers.size(); idx < end; ++idx) {
            if (m_AllocatedDataBuffers[idx].m_Name == inName)
                return idx;
        }

        return m_AllocatedDataBuffers.size();
    }

    void SetTexture(QDemonRenderShaderProgram &inShader, CRegisteredString inPropName,
                    QDemonRenderTexture2D *inTexture, bool inNeedsMultiply,
                    CRenderString &inStringBuilder, CRenderString &inStringBuilder2,
                    const SPropertyDefinition *inPropDec = nullptr)
    {
        STextureEntry *theTextureEntry(nullptr);
        for (quint32 idx = 0, end = m_TextureEntries.size(); idx < end && theTextureEntry == nullptr;
             ++idx) {
            if (m_TextureEntries[idx].first == inPropName
                    && m_TextureEntries[idx].second->m_Shader.mPtr == &inShader)
                theTextureEntry = m_TextureEntries[idx].second;
        }
        if (theTextureEntry == nullptr) {
            QDemonScopedRefCounted<STextureEntry> theNewEntry = QDEMON_NEW(
                        m_Context.GetAllocator(), STextureEntry)(STextureEntry::CreateTextureEntry(
                                                                     inShader, inPropName, inStringBuilder, inStringBuilder2));
            m_TextureEntries.push_back(eastl::make_pair(inPropName, theNewEntry));
            theTextureEntry = theNewEntry.mPtr;
        }
        theTextureEntry->Set(inTexture, inNeedsMultiply, inPropDec);
    }

    void SetImage(QDemonRenderShaderProgram &inShader, CRegisteredString inPropName,
                  QDemonRenderImage2D *inImage)
    {
        SImageEntry *theImageEntry(nullptr);
        for (quint32 idx = 0, end = m_ImageEntries.size(); idx < end && theImageEntry == nullptr;
             ++idx) {
            if (m_ImageEntries[idx].first == inPropName
                    && m_ImageEntries[idx].second->m_Shader.mPtr == &inShader)
                theImageEntry = m_ImageEntries[idx].second;
        }
        if (theImageEntry == nullptr) {
            QDemonScopedRefCounted<SImageEntry> theNewEntry =
                    QDEMON_NEW(m_Context.GetAllocator(),
                               SImageEntry)(SImageEntry::CreateImageEntry(inShader, inPropName));
            m_ImageEntries.push_back(eastl::make_pair(inPropName, theNewEntry));
            theImageEntry = theNewEntry.mPtr;
        }

        theImageEntry->Set(inImage);
    }

    void SetDataBuffer(QDemonRenderShaderProgram &inShader, CRegisteredString inPropName,
                       QDemonRenderDataBuffer *inBuffer)
    {
        SDataBufferEntry *theDataBufferEntry(nullptr);
        for (quint32 idx = 0, end = m_DataBufferEntries.size();
             idx < end && theDataBufferEntry == nullptr; ++idx) {
            if (m_DataBufferEntries[idx].first == inPropName
                    && m_DataBufferEntries[idx].second->m_Shader.mPtr == &inShader)
                theDataBufferEntry = m_DataBufferEntries[idx].second;
        }
        if (theDataBufferEntry == nullptr) {
            QDemonScopedRefCounted<SDataBufferEntry> theNewEntry =
                    QDEMON_NEW(m_Context.GetAllocator(), SDataBufferEntry)(
                        SDataBufferEntry::CreateDataBufferEntry(inShader, inPropName));
            m_DataBufferEntries.push_back(eastl::make_pair(inPropName, theNewEntry));
            theDataBufferEntry = theNewEntry.mPtr;
        }

        theDataBufferEntry->Set(inBuffer);
    }
};
}
}

namespace {

using NVRenderCachedShaderProperty;
/* We setup some shared state on the effect shaders */
struct SEffectShader
{
    QDemonScopedRefCounted<QDemonRenderShaderProgram> m_Shader;
    NVRenderCachedShaderProperty<QMatrix4x4> m_MVP;
    NVRenderCachedShaderProperty<QVector2D> m_FragColorAlphaSettings;
    NVRenderCachedShaderProperty<QVector2D> m_DestSize;
    NVRenderCachedShaderProperty<float> m_AppFrame;
    NVRenderCachedShaderProperty<float> m_FPS;
    NVRenderCachedShaderProperty<QVector2D> m_CameraClipRange;
    STextureEntry m_TextureEntry;
    volatile qint32 mRefCount;
    SEffectShader(QDemonRenderShaderProgram &inShader)
        : m_Shader(inShader)
        , m_MVP("ModelViewProjectionMatrix", inShader)
        , m_FragColorAlphaSettings("FragColorAlphaSettings", inShader)
        , m_DestSize("DestSize", inShader)
        , m_AppFrame("AppFrame", inShader)
        , m_FPS("FPS", inShader)
        , m_CameraClipRange("CameraClipRange", inShader)
        , m_TextureEntry(inShader, "Texture0", "Texture0Info", "Texture0Flags")
        , mRefCount(0)
    {
    }

    QDEMON_IMPLEMENT_REF_COUNT_ADDREF_RELEASE(m_Shader->GetRenderContext().GetAllocator())
};

struct SEffectSystem : public IEffectSystem
{
    typedef QHash<CRegisteredString, char8_t *> TPathDataMap;
    typedef nvhash_set<CRegisteredString> TPathSet;
    typedef QHash<CRegisteredString, QDemonScopedRefCounted<SEffectClass>> TEffectClassMap;
    typedef QHash<TStrStrPair, QDemonScopedRefCounted<SEffectShader>> TShaderMap;
    typedef QVector<SEffectContext *> TContextList;

    IQt3DSRenderContextCore &m_CoreContext;
    IQt3DSRenderContext *m_Context;
    QDemonScopedRefCounted<IResourceManager> m_ResourceManager;
    mutable SPreAllocatedAllocator m_Allocator;
    // Keep from dual-including headers.
    TEffectClassMap m_EffectClasses;
    QVector<CRegisteredString> m_EffectList;
    TContextList m_Contexts;
    CRenderString m_TextureStringBuilder;
    CRenderString m_TextureStringBuilder2;
    TShaderMap m_ShaderMap;
    QDemonScopedRefCounted<QDemonRenderDepthStencilState> m_DefaultStencilState;
    QVector<QDemonScopedRefCounted<QDemonRenderDepthStencilState>> m_DepthStencilStates;
    volatile qint32 mRefCount;

    SEffectSystem(IQt3DSRenderContextCore &inContext)
        : m_CoreContext(inContext)
        , m_Context(nullptr)
        , m_Allocator(inContext.GetAllocator())
        , m_EffectClasses(inContext.GetAllocator(), "SEffectSystem::m_EffectClasses")
        , m_EffectList(inContext.GetAllocator(), "SEffectSystem::m_EffectList")
        , m_Contexts(inContext.GetAllocator(), "SEffectSystem::m_Contexts")
        , m_ShaderMap(inContext.GetAllocator(), "SEffectSystem::m_ShaderMap")
        , m_DepthStencilStates(inContext.GetAllocator(), "SEffectSystem::m_DepthStencilStates")
        , mRefCount(0)
    {
    }

    ~SEffectSystem()
    {
        for (quint32 idx = 0, end = m_Contexts.size(); idx < end; ++idx)
            NVDelete(m_Allocator, m_Contexts[idx]);
        m_Contexts.clear();
    }

    SEffectContext &GetEffectContext(SEffect &inEffect)
    {
        if (inEffect.m_Context == nullptr) {
            inEffect.m_Context =
                    QDEMON_NEW(m_Allocator, SEffectContext)(inEffect.m_ClassName,
                                                            *m_Context, m_ResourceManager);
            m_Contexts.push_back(inEffect.m_Context);
        }
        return *inEffect.m_Context;
    }

    QDEMON_IMPLEMENT_REF_COUNT_ADDREF_RELEASE_OVERRIDE(m_CoreContext.GetAllocator());

    SEffectClass *GetEffectClass(CRegisteredString inStr)
    {
        TEffectClassMap::iterator theIter = m_EffectClasses.find(inStr);
        if (theIter != m_EffectClasses.end())
            return theIter->second;
        return nullptr;
    }
    const SEffectClass *GetEffectClass(CRegisteredString inStr) const
    {
        return const_cast<SEffectSystem *>(this)->GetEffectClass(inStr);
    }

    bool IsEffectRegistered(CRegisteredString inStr) override
    {
        return GetEffectClass(inStr) != nullptr;
    }
    QDemonConstDataRef<CRegisteredString> GetRegisteredEffects() override
    {
        m_EffectList.clear();
        for (TEffectClassMap::iterator theIter = m_EffectClasses.begin(),
             theEnd = m_EffectClasses.end();
             theIter != theEnd; ++theIter)
            m_EffectList.push_back(theIter->first);
        return m_EffectList;
    }

    // Registers an effect that runs via a single GLSL file.
    bool RegisterGLSLEffect(CRegisteredString inName, const char8_t *inPathToEffect,
                            QDemonConstDataRef<SPropertyDeclaration> inProperties) override
    {
        if (IsEffectRegistered(inName))
            return false;

        m_CoreContext.GetDynamicObjectSystemCore().Register(inName, inProperties, sizeof(SEffect),
                                                            GraphObjectTypes::Effect);
        IDynamicObjectClass &theClass =
                *m_CoreContext.GetDynamicObjectSystemCore().GetDynamicObjectClass(inName);

        SEffectClass *theEffect = QDEMON_NEW(m_Allocator, SEffectClass)(m_Allocator, theClass);
        m_EffectClasses.insert(eastl::make_pair(inName, theEffect));

        // Setup the commands required to run this effect
        StaticAssert<(sizeof(SBindShader) % 4 == 0)>::valid_expression();
        StaticAssert<(sizeof(SApplyInstanceValue) % 4 == 0)>::valid_expression();
        StaticAssert<(sizeof(SRender) % 4 == 0)>::valid_expression();

        quint32 commandAllocationSize = sizeof(SBindTarget);
        commandAllocationSize += sizeof(SBindShader);
        commandAllocationSize += sizeof(SApplyInstanceValue) * inProperties.size();
        commandAllocationSize += sizeof(SRender);

        quint32 commandCount = 3 + inProperties.size();
        quint32 commandPtrAllocationSize = commandCount * sizeof(SCommand *);
        quint32 allocationSize = Align8(commandAllocationSize) + commandPtrAllocationSize;
        quint8 *startBuffer =
                (quint8 *)m_Allocator.allocate(allocationSize, "dynamic::SCommand", __FILE__, __LINE__);
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

        new (dataBuffer) SBindShader(m_CoreContext.GetStringTable().RegisterStr(inPathToEffect));
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
        m_CoreContext.GetDynamicObjectSystemCore().SetRenderCommands(
                    inName, QDemonConstDataRef<SCommand *>(theFirstCommandPtr, commandCount));
        m_Allocator.deallocate(startBuffer);
        return true;
    }

    void SetEffectPropertyDefaultValue(CRegisteredString inName,
                                       CRegisteredString inPropName,
                                       QDemonConstDataRef<quint8> inDefaultData) override
    {
        m_CoreContext.GetDynamicObjectSystemCore().SetPropertyDefaultValue(inName, inPropName,
                                                                           inDefaultData);
    }

    void SetEffectPropertyEnumNames(CRegisteredString inName, CRegisteredString inPropName,
                                    QDemonConstDataRef<CRegisteredString> inNames) override
    {
        m_CoreContext.GetDynamicObjectSystemCore().SetPropertyEnumNames(inName, inPropName,
                                                                        inNames);
    }

    bool RegisterEffect(CRegisteredString inName,
                        QDemonConstDataRef<SPropertyDeclaration> inProperties) override
    {
        if (IsEffectRegistered(inName))
            return false;
        m_CoreContext.GetDynamicObjectSystemCore().Register(inName, inProperties, sizeof(SEffect),
                                                            GraphObjectTypes::Effect);
        IDynamicObjectClass &theClass =
                *m_CoreContext.GetDynamicObjectSystemCore().GetDynamicObjectClass(inName);
        SEffectClass *theEffect = QDEMON_NEW(m_Allocator, SEffectClass)(m_Allocator, theClass);
        m_EffectClasses.insert(eastl::make_pair(inName, theEffect));
        return true;
    }

    bool UnregisterEffect(CRegisteredString inName) override
    {
        if (!IsEffectRegistered(inName))
            return false;

        m_CoreContext.GetDynamicObjectSystemCore().Unregister(inName);

        TEffectClassMap::iterator iter = m_EffectClasses.find(inName);
        if (iter != m_EffectClasses.end())
            m_EffectClasses.erase(iter);

        for (quint32 idx = 0, end = m_Contexts.size(); idx < end; ++idx) {
            if (m_Contexts[idx]->m_ClassName == inName)
                ReleaseEffectContext(m_Contexts[idx]);
        }
        return true;
    }

    virtual QDemonConstDataRef<CRegisteredString>
    GetEffectPropertyEnumNames(CRegisteredString inName, CRegisteredString inPropName) const override
    {
        const SEffectClass *theClass = GetEffectClass(inName);
        if (theClass == nullptr) {
            Q_ASSERT(false);
            QDemonConstDataRef<CRegisteredString>();
        }
        const SPropertyDefinition *theDefinitionPtr =
                theClass->m_DynamicClass->FindPropertyByName(inPropName);
        if (theDefinitionPtr)
            return theDefinitionPtr->m_EnumValueNames;
        return QDemonConstDataRef<CRegisteredString>();
    }

    virtual QDemonConstDataRef<SPropertyDefinition>
    GetEffectProperties(CRegisteredString inEffectName) const override
    {
        const SEffectClass *theClass = GetEffectClass(inEffectName);
        if (theClass)
            return theClass->m_DynamicClass->GetProperties();
        return QDemonConstDataRef<SPropertyDefinition>();
    }

    void SetEffectPropertyTextureSettings(CRegisteredString inName,
                                          CRegisteredString inPropName,
                                          CRegisteredString inPropPath,
                                          QDemonRenderTextureTypeValue::Enum inTexType,
                                          QDemonRenderTextureCoordOp::Enum inCoordOp,
                                          QDemonRenderTextureMagnifyingOp::Enum inMagFilterOp,
                                          QDemonRenderTextureMinifyingOp::Enum inMinFilterOp) override
    {
        m_CoreContext.GetDynamicObjectSystemCore().SetPropertyTextureSettings(
                    inName, inPropName, inPropPath, inTexType, inCoordOp, inMagFilterOp, inMinFilterOp);
    }

    void SetEffectRequiresDepthTexture(CRegisteredString inEffectName, bool inValue) override
    {
        SEffectClass *theClass = GetEffectClass(inEffectName);
        if (theClass == nullptr) {
            Q_ASSERT(false);
            return;
        }
        theClass->m_DynamicClass->SetRequiresDepthTexture(inValue);
    }

    bool DoesEffectRequireDepthTexture(CRegisteredString inEffectName) const override
    {
        const SEffectClass *theClass = GetEffectClass(inEffectName);
        if (theClass == nullptr) {
            Q_ASSERT(false);
            return false;
        }
        return theClass->m_DynamicClass->RequiresDepthTexture();
    }

    void SetEffectRequiresCompilation(CRegisteredString inEffectName, bool inValue) override
    {
        SEffectClass *theClass = GetEffectClass(inEffectName);
        if (theClass == nullptr) {
            Q_ASSERT(false);
            return;
        }
        theClass->m_DynamicClass->SetRequiresCompilation(inValue);
    }

    bool DoesEffectRequireCompilation(CRegisteredString inEffectName) const override
    {
        const SEffectClass *theClass = GetEffectClass(inEffectName);
        if (theClass == nullptr) {
            Q_ASSERT(false);
            return false;
        }
        return theClass->m_DynamicClass->RequiresCompilation();
    }

    void SetEffectCommands(CRegisteredString inEffectName,
                           QDemonConstDataRef<dynamic::SCommand *> inCommands) override
    {
        m_CoreContext.GetDynamicObjectSystemCore().SetRenderCommands(inEffectName, inCommands);
    }

    virtual QDemonConstDataRef<dynamic::SCommand *>
    GetEffectCommands(CRegisteredString inEffectName) const override
    {
        return m_CoreContext.GetDynamicObjectSystemCore().GetRenderCommands(inEffectName);
    }

    SEffect *CreateEffectInstance(CRegisteredString inEffectName,
                                  NVAllocatorCallback &inSceneGraphAllocator) override
    {
        SEffectClass *theClass = GetEffectClass(inEffectName);
        if (theClass == nullptr)
            return nullptr;
        StaticAssert<(sizeof(SEffect) % 4 == 0)>::valid_expression();

        SEffect *theEffect = (SEffect *)m_CoreContext.GetDynamicObjectSystemCore().CreateInstance(
                    inEffectName, inSceneGraphAllocator);
        theEffect->Initialize();
        return theEffect;
    }

    void AllocateBuffer(SEffect &inEffect, const SAllocateBuffer &inCommand, quint32 inFinalWidth,
                        quint32 inFinalHeight, QDemonRenderTextureFormats::Enum inSourceTextureFormat)
    {
        // Check to see if it is already allocated and if it is, is it the correct size. If both of
        // these assumptions hold, then we are good.
        QDemonRenderTexture2D *theBufferTexture = nullptr;
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
            QDemonRenderFrameBuffer *theFB(m_ResourceManager->AllocateFrameBuffer());
            QDemonRenderTexture2D *theTexture(
                        m_ResourceManager->AllocateTexture2D(theWidth, theHeight, resultFormat));
            theTexture->SetMagFilter(inCommand.m_FilterOp);
            theTexture->SetMinFilter(
                        static_cast<QDemonRenderTextureMinifyingOp::Enum>(inCommand.m_FilterOp));
            theTexture->SetTextureWrapS(inCommand.m_TexCoordOp);
            theTexture->SetTextureWrapT(inCommand.m_TexCoordOp);
            theFB->Attach(QDemonRenderFrameBufferAttachments::Color0, *theTexture);
            theContext.m_AllocatedBuffers.push_back(SAllocatedBufferEntry(
                                                        inCommand.m_Name, *theFB, *theTexture, inCommand.m_BufferFlags));
            theBufferTexture = theTexture;
        }
    }

    void AllocateImage(SEffect &inEffect, const SAllocateImage &inCommand, quint32 inFinalWidth,
                       quint32 inFinalHeight)
    {
        QDemonRenderImage2D *theImage = nullptr;
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
            QDemonRenderTexture2D *theTexture(m_ResourceManager->AllocateTexture2D(
                                                  theWidth, theHeight, inCommand.m_Format, 1, true));
            theTexture->SetMagFilter(inCommand.m_FilterOp);
            theTexture->SetMinFilter(
                        static_cast<QDemonRenderTextureMinifyingOp::Enum>(inCommand.m_FilterOp));
            theTexture->SetTextureWrapS(inCommand.m_TexCoordOp);
            theTexture->SetTextureWrapT(inCommand.m_TexCoordOp);
            QDemonRenderImage2D *theImage =
                    (m_ResourceManager->AllocateImage2D(theTexture, inCommand.m_Access));
            theContext.m_AllocatedImages.push_back(SAllocatedImageEntry(
                                                       inCommand.m_Name, *theImage, *theTexture, inCommand.m_BufferFlags));
        }
    }

    void AllocateDataBuffer(SEffect &inEffect, const SAllocateDataBuffer &inCommand)
    {
        quint32 theBufferSize = (quint32)inCommand.m_Size;
        Q_ASSERT(theBufferSize);
        QDemonRenderDataBuffer *theDataBuffer = nullptr;
        QDemonRenderDataBuffer *theDataWrapBuffer = nullptr;

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
            QDemonRenderContext &theRenderContext(m_Context->GetRenderContext());
            quint8 *initialData = (quint8 *)theContext.m_Context.GetAllocator().allocate(
                        theBufferSize, "SEffectContext::AllocateDataBuffer", __FILE__, __LINE__);
            QDemonDataRef<quint8> data((quint8 *)initialData, theBufferSize);
            memset(initialData, 0x0L, theBufferSize);
            if (inCommand.m_DataBufferType == QDemonRenderBufferBindValues::Storage) {
                theDataBuffer = theRenderContext.CreateStorageBuffer(
                            inCommand.m_Name, QDemonRenderBufferUsageType::Dynamic, theBufferSize,
                            data, nullptr);
            } else if (inCommand.m_DataBufferType == QDemonRenderBufferBindValues::Draw_Indirect) {
                Q_ASSERT(theBufferSize == sizeof(DrawArraysIndirectCommand));
                // init a draw call
                quint32 *pIndirectDrawCall = (quint32 *)initialData;
                // vertex count we draw points right now only
                // the rest we fill in by GPU
                pIndirectDrawCall[0] = 1;
                theDataBuffer = theRenderContext.CreateDrawIndirectBuffer(
                            QDemonRenderBufferUsageType::Dynamic, theBufferSize, data);
            } else
                Q_ASSERT(false);

            theContext.m_AllocatedDataBuffers.push_back(SAllocatedDataBufferEntry(
                                                            inCommand.m_Name, *theDataBuffer, inCommand.m_DataBufferType, data,
                                                            inCommand.m_BufferFlags));

            // create wrapper buffer
            if (inCommand.m_DataBufferWrapType == QDemonRenderBufferBindValues::Storage
                    && inCommand.m_WrapName && theDataBuffer) {
                theDataWrapBuffer = theRenderContext.CreateStorageBuffer(
                            inCommand.m_WrapName, QDemonRenderBufferUsageType::Dynamic,
                            theBufferSize, data, theDataBuffer);
                theContext.m_AllocatedDataBuffers.push_back(SAllocatedDataBufferEntry(
                                                                inCommand.m_WrapName, *theDataWrapBuffer, inCommand.m_DataBufferWrapType,
                                                                QDemonDataRef<quint8>(), inCommand.m_BufferFlags));
            }
        }
    }

    QDemonRenderTexture2D *FindTexture(SEffect &inEffect, CRegisteredString inName)
    {
        if (inEffect.m_Context) {
            SEffectContext &theContext(*inEffect.m_Context);
            quint32 bufferIdx = theContext.FindBuffer(inName);
            if (bufferIdx < theContext.m_AllocatedBuffers.size()) {
                return theContext.m_AllocatedBuffers[bufferIdx].m_Texture;
            }
        }
        Q_ASSERT(false);
        return nullptr;
    }

    QDemonRenderFrameBuffer *BindBuffer(SEffect &inEffect, const SBindBuffer &inCommand,
                                        QMatrix4x4 &outMVP, QVector2D &outDestSize)
    {
        QDemonRenderFrameBuffer *theBuffer = nullptr;
        QDemonRenderTexture2D *theTexture = nullptr;
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
                       inEffect.m_ClassName.c_str(), inCommand.m_BufferName.c_str());
            QString errorMsg = QObject::tr("Failed to compile \"%1\" effect.\nConsider"
                                           " removing it from the presentation.")
                    .arg(inEffect.m_ClassName.c_str());
            QDEMON_ALWAYS_ASSERT_MESSAGE(errorMsg.toUtf8());
            outMVP = QMatrix4x4::createIdentity();
            return nullptr;
        }

        if (theTexture) {
            SCamera::SetupOrthographicCameraForOffscreenRender(*theTexture, outMVP);
            STextureDetails theDetails(theTexture->GetTextureDetails());
            m_Context->GetRenderContext().SetViewport(
                        QDemonRenderRect(0, 0, (quint32)theDetails.m_Width, (quint32)theDetails.m_Height));
            outDestSize = QVector2D((float)theDetails.m_Width, (float)theDetails.m_Height);
        }

        return theBuffer;
    }

    SEffectShader *BindShader(CRegisteredString &inEffectId, const SBindShader &inCommand)
    {
        SEffectClass *theClass = GetEffectClass(inEffectId);
        if (!theClass) {
            Q_ASSERT(false);
            return nullptr;
        }

        bool forceCompilation = theClass->m_DynamicClass->RequiresCompilation();

        eastl::pair<const TStrStrPair, QDemonScopedRefCounted<SEffectShader>> theInserter(
                    TStrStrPair(inCommand.m_ShaderPath, inCommand.m_ShaderDefine),
                    QDemonScopedRefCounted<SEffectShader>());
        eastl::pair<TShaderMap::iterator, bool> theInsertResult(m_ShaderMap.insert(theInserter));

        if (theInsertResult.second || forceCompilation) {
            QDemonRenderShaderProgram *theProgram =
                    m_Context->GetDynamicObjectSystem()
                    .GetShaderProgram(inCommand.m_ShaderPath, inCommand.m_ShaderDefine,
                                      TShaderFeatureSet(), SDynamicShaderProgramFlags(),
                                      forceCompilation).first;
            if (theProgram)
                theInsertResult.first->second = QDEMON_NEW(m_Allocator, SEffectShader)(*theProgram);
        }
        if (theInsertResult.first->second) {
            QDemonRenderContext &theContext(m_Context->GetRenderContext());
            theContext.SetActiveShader(theInsertResult.first->second->m_Shader);
        }

        return theInsertResult.first->second;
    }

    void DoApplyInstanceValue(SEffect &inEffect, quint8 *inDataPtr, CRegisteredString inPropertyName,
                              QDemonRenderShaderDataTypes::Enum inPropertyType,
                              QDemonRenderShaderProgram &inShader,
                              const SPropertyDefinition &inDefinition)
    {
        QDemonRenderShaderConstantBase *theConstant =
                inShader.GetShaderConstant(inPropertyName);
        using namespace render;
        if (theConstant) {
            if (theConstant->GetShaderConstantType() == inPropertyType) {
                if (inPropertyType == QDemonRenderShaderDataTypes::QDemonRenderTexture2DPtr) {
                    StaticAssert<sizeof(CRegisteredString)
                            == sizeof(QDemonRenderTexture2DPtr)>::valid_expression();
                    CRegisteredString *theStrPtr = reinterpret_cast<CRegisteredString *>(inDataPtr);
                    IBufferManager &theBufferManager(m_Context->GetBufferManager());
                    IOffscreenRenderManager &theOffscreenRenderer(
                                m_Context->GetOffscreenRenderManager());
                    bool needsAlphaMultiply = true;
                    QDemonRenderTexture2D *theTexture = nullptr;
                    if (theStrPtr->IsValid()) {
                        if (theOffscreenRenderer.HasOffscreenRenderer(*theStrPtr)) {
                            SOffscreenRenderResult theResult =
                                    theOffscreenRenderer.GetRenderedItem(*theStrPtr);
                            needsAlphaMultiply = false;
                            theTexture = theResult.m_Texture;
                        } else {
                            SImageTextureData theTextureData =
                                    theBufferManager.LoadRenderImage(*theStrPtr);
                            needsAlphaMultiply = true;
                            theTexture = theTextureData.m_Texture;
                        }
                    }
                    GetEffectContext(inEffect).SetTexture(
                                inShader, inPropertyName, theTexture, needsAlphaMultiply,
                                m_TextureStringBuilder, m_TextureStringBuilder2, &inDefinition);
                } else if (inPropertyType == QDemonRenderShaderDataTypes::QDemonRenderImage2DPtr) {
                    StaticAssert<sizeof(CRegisteredString)
                            == sizeof(QDemonRenderTexture2DPtr)>::valid_expression();
                    QDemonRenderImage2D *theImage = nullptr;
                    GetEffectContext(inEffect).SetImage(inShader, inPropertyName, theImage);
                } else if (inPropertyType == QDemonRenderShaderDataTypes::QDemonRenderDataBufferPtr) {
                    // we don't handle this here
                } else {
                    switch (inPropertyType) {
#define HANDLE_QDEMON_SHADER_DATA_TYPE(type)                                                           \
                    case QDemonRenderShaderDataTypes::type:                                                            \
    inShader.SetPropertyValue(theConstant, *(reinterpret_cast<type *>(inDataPtr)));            \
    break;
                    ITERATE_QDEMON_SHADER_DATA_TYPES
        #undef HANDLE_QDEMON_SHADER_DATA_TYPE
                            default:
                        Q_ASSERT(false);
                    break;
                    }
                }
            } else {
                qCCritical(INVALID_OPERATION,
                           "Effect ApplyInstanceValue command datatype "
                           "and shader datatypes differ for property %s",
                           inPropertyName.c_str());
                Q_ASSERT(false);
            }
        }
    }

    void ApplyInstanceValue(SEffect &inEffect, SEffectClass &inClass,
                            QDemonRenderShaderProgram &inShader, const SApplyInstanceValue &inCommand)
    {
        // sanity check
        if (inCommand.m_PropertyName.IsValid()) {
            bool canGetData =
                    inCommand.m_ValueOffset + getSizeofShaderDataType(inCommand.m_ValueType)
                    <= inEffect.m_DataSectionByteSize;
            if (canGetData == false) {
                Q_ASSERT(false);
                return;
            }
            quint8 *dataPtr = inEffect.GetDataSectionBegin() + inCommand.m_ValueOffset;
            const SPropertyDefinition *theDefinition =
                    inClass.m_DynamicClass->FindPropertyByName(inCommand.m_PropertyName);
            if (theDefinition)
                DoApplyInstanceValue(inEffect, dataPtr, inCommand.m_PropertyName,
                                     inCommand.m_ValueType, inShader, *theDefinition);
        } else {
            QDemonConstDataRef<SPropertyDefinition> theDefs = inClass.m_DynamicClass->GetProperties();
            for (quint32 idx = 0, end = theDefs.size(); idx < end; ++idx) {
                const SPropertyDefinition &theDefinition(theDefs[idx]);
                QDemonRenderShaderConstantBase *theConstant =
                        inShader.GetShaderConstant(theDefinition.m_Name);

                // This is fine, the property wasn't found and we continue, no problem.
                if (!theConstant)
                    continue;
                quint8 *dataPtr = inEffect.GetDataSectionBegin() + theDefinition.m_Offset;
                DoApplyInstanceValue(inEffect, dataPtr, theDefinition.m_Name,
                                     theDefinition.m_DataType, inShader, theDefinition);
            }
        }
    }

    void ApplyValue(SEffect &inEffect, SEffectClass &inClass, QDemonRenderShaderProgram &inShader,
                    const SApplyValue &inCommand)
    {
        if (inCommand.m_PropertyName.IsValid()) {
            quint8 *dataPtr = inCommand.m_Value.mData;
            const SPropertyDefinition *theDefinition =
                    inClass.m_DynamicClass->FindPropertyByName(inCommand.m_PropertyName);
            if (theDefinition)
                DoApplyInstanceValue(inEffect, dataPtr, inCommand.m_PropertyName,
                                     inCommand.m_ValueType, inShader, *theDefinition);
        }
    }

    bool ApplyBlending(const SApplyBlending &inCommand)
    {
        QDemonRenderContext &theContext(m_Context->GetRenderContext());

        theContext.SetBlendingEnabled(true);

        QDemonRenderBlendFunctionArgument blendFunc =
                QDemonRenderBlendFunctionArgument(
                    inCommand.m_SrcBlendFunc, inCommand.m_DstBlendFunc, inCommand.m_SrcBlendFunc,
                    inCommand.m_DstBlendFunc);

        QDemonRenderBlendEquationArgument blendEqu(QDemonRenderBlendEquation::Add,
                                                   QDemonRenderBlendEquation::Add);

        theContext.SetBlendFunction(blendFunc);
        theContext.SetBlendEquation(blendEqu);

        return true;
    }

    // This has the potential to change the source texture for the current render pass
    SEffectTextureData ApplyBufferValue(SEffect &inEffect, QDemonRenderShaderProgram &inShader,
                                        const SApplyBufferValue &inCommand,
                                        QDemonRenderTexture2D &inSourceTexture,
                                        SEffectTextureData inCurrentSourceTexture)
    {
        SEffectTextureData theTextureToBind;
        if (inCommand.m_BufferName.IsValid()) {
            if (inEffect.m_Context) {
                SEffectContext &theContext(*inEffect.m_Context);
                quint32 bufferIdx = theContext.FindBuffer(inCommand.m_BufferName);
                if (bufferIdx < theContext.m_AllocatedBuffers.size()) {
                    SAllocatedBufferEntry &theEntry(theContext.m_AllocatedBuffers[bufferIdx]);
                    if (theEntry.m_NeedsClear) {
                        QDemonRenderContext &theRenderContext(m_Context->GetRenderContext());

                        theRenderContext.SetRenderTarget(theEntry.m_FrameBuffer);
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
                                            theRenderContext, &QDemonRenderContext::GetClearColor,
                                            &QDemonRenderContext::SetClearColor, QVector4D(0.0f));
                                theRenderContext.Clear(QDemonRenderClearValues::Color);
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
                           inEffect.m_ClassName.c_str(), inCommand.m_BufferName.c_str());
                Q_ASSERT(false);
            }
        } else // no name means bind the source
            theTextureToBind = SEffectTextureData(&inSourceTexture, false);

        if (inCommand.m_ParamName.IsValid()) {
            QDemonRenderShaderConstantBase *theConstant =
                    inShader.GetShaderConstant(inCommand.m_ParamName);

            if (theConstant) {
                if (theConstant->GetShaderConstantType()
                        != QDemonRenderShaderDataTypes::QDemonRenderTexture2DPtr) {
                    qCCritical(INVALID_OPERATION,
                               "Effect %s: Binding buffer to parameter %s that is not a texture",
                               inEffect.m_ClassName.c_str(), inCommand.m_ParamName.c_str());
                    Q_ASSERT(false);
                } else {
                    GetEffectContext(inEffect).SetTexture(
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

    void ApplyDepthValue(SEffect &inEffect, QDemonRenderShaderProgram &inShader,
                         const SApplyDepthValue &inCommand, QDemonRenderTexture2D *inTexture)
    {
        QDemonRenderShaderConstantBase *theConstant =
                inShader.GetShaderConstant(inCommand.m_ParamName);

        if (theConstant) {
            if (theConstant->GetShaderConstantType()
                    != QDemonRenderShaderDataTypes::QDemonRenderTexture2DPtr) {
                qCCritical(INVALID_OPERATION,
                           "Effect %s: Binding buffer to parameter %s that is not a texture",
                           inEffect.m_ClassName.c_str(), inCommand.m_ParamName.c_str());
                Q_ASSERT(false);
            } else {
                GetEffectContext(inEffect).SetTexture(inShader, inCommand.m_ParamName, inTexture,
                                                      false, m_TextureStringBuilder,
                                                      m_TextureStringBuilder2);
            }
        }
    }

    void ApplyImageValue(SEffect &inEffect, QDemonRenderShaderProgram &inShader,
                         const SApplyImageValue &inCommand)
    {
        SAllocatedImageEntry theImageToBind;
        if (inCommand.m_ImageName.IsValid()) {
            if (inEffect.m_Context) {
                SEffectContext &theContext(*inEffect.m_Context);
                quint32 bufferIdx = theContext.FindImage(inCommand.m_ImageName);
                if (bufferIdx < theContext.m_AllocatedImages.size()) {
                    theImageToBind = SAllocatedImageEntry(theContext.m_AllocatedImages[bufferIdx]);
                }
            }
        }

        if (theImageToBind.m_Image == nullptr) {
            qCCritical(INVALID_OPERATION, "Effect %s: Failed to find image %s for bind",
                       inEffect.m_ClassName.c_str(), inCommand.m_ImageName.c_str());
            Q_ASSERT(false);
        }

        if (inCommand.m_ParamName.IsValid()) {
            QDemonRenderShaderConstantBase *theConstant =
                    inShader.GetShaderConstant(inCommand.m_ParamName);

            if (theConstant) {
                if (inCommand.m_NeedSync) {
                    QDemonRenderBufferBarrierFlags flags(
                                QDemonRenderBufferBarrierValues::TextureFetch
                                | QDemonRenderBufferBarrierValues::TextureUpdate);
                    inShader.GetRenderContext().SetMemoryBarrier(flags);
                }

                if (theConstant->GetShaderConstantType()
                        == QDemonRenderShaderDataTypes::QDemonRenderImage2DPtr
                        && !inCommand.m_BindAsTexture) {
                    GetEffectContext(inEffect).SetImage(inShader, inCommand.m_ParamName,
                                                        theImageToBind.m_Image);
                } else if (theConstant->GetShaderConstantType()
                           == QDemonRenderShaderDataTypes::QDemonRenderTexture2DPtr
                           && inCommand.m_BindAsTexture) {
                    GetEffectContext(inEffect).SetTexture(
                                inShader, inCommand.m_ParamName, theImageToBind.m_Texture, false,
                                m_TextureStringBuilder, m_TextureStringBuilder2);
                } else {
                    qCCritical(INVALID_OPERATION,
                               "Effect %s: Binding buffer to parameter %s that is not a texture",
                               inEffect.m_ClassName.c_str(), inCommand.m_ParamName.c_str());
                    Q_ASSERT(false);
                }
            }
        }
    }

    void ApplyDataBufferValue(SEffect &inEffect, QDemonRenderShaderProgram &inShader,
                              const SApplyDataBufferValue &inCommand)
    {
        SAllocatedDataBufferEntry theBufferToBind;
        if (inCommand.m_ParamName.IsValid()) {
            if (inEffect.m_Context) {
                SEffectContext &theContext(*inEffect.m_Context);
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
                           inEffect.m_ClassName.c_str(), inCommand.m_ParamName.c_str());
                Q_ASSERT(false);
            }

            QDemonRenderShaderBufferBase *theConstant =
                    inShader.GetShaderBuffer(inCommand.m_ParamName);

            if (theConstant) {
                GetEffectContext(inEffect).SetDataBuffer(inShader, inCommand.m_ParamName,
                                                         theBufferToBind.m_DataBuffer);
            } else if (theBufferToBind.m_BufferType
                       == QDemonRenderBufferBindValues::Draw_Indirect) {
                // since we filled part of this buffer on the GPU we need a sync before usage
                QDemonRenderBufferBarrierFlags flags(
                            QDemonRenderBufferBarrierValues::CommandBuffer);
                inShader.GetRenderContext().SetMemoryBarrier(flags);
            }
        }
    }

    void ApplyRenderStateValue(QDemonRenderFrameBuffer *inTarget,
                               QDemonRenderTexture2D *inDepthStencilTexture,
                               const SApplyRenderState &theCommand)
    {
        QDemonRenderContext &theContext(m_Context->GetRenderContext());
        quint32 inState = (quint32)theCommand.m_RenderState;
        bool inEnable = theCommand.m_Enabled;

        switch (inState) {
        case QDemonRenderState::StencilTest: {
            if (inEnable && inTarget) {
                inTarget->Attach(QDemonRenderFrameBufferAttachments::DepthStencil,
                                 *inDepthStencilTexture);
            } else if (inTarget) {
                inTarget->Attach(QDemonRenderFrameBufferAttachments::DepthStencil,
                                 QDemonRenderTextureOrRenderBuffer());
            }

            theContext.SetStencilTestEnabled(inEnable);
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

    void RenderPass(SEffectShader &inShader, const QMatrix4x4 &inMVP,
                    SEffectTextureData inSourceTexture, QDemonRenderFrameBuffer *inFrameBuffer,
                    QVector2D &inDestSize, const QVector2D &inCameraClipRange,
                    QDemonRenderTexture2D *inDepthStencil, Option<SDepthStencil> inDepthStencilCommand,
                    bool drawIndirect)
    {
        QDemonRenderContext &theContext(m_Context->GetRenderContext());
        theContext.SetRenderTarget(inFrameBuffer);
        if (inDepthStencil && inFrameBuffer) {
            inFrameBuffer->Attach(QDemonRenderFrameBufferAttachments::DepthStencil, *inDepthStencil);
            if (inDepthStencilCommand.hasValue()) {
                SDepthStencil &theDepthStencil(*inDepthStencilCommand);
                quint32 clearFlags = 0;
                if (theDepthStencil.m_Flags.HasClearStencil())
                    clearFlags |= QDemonRenderClearValues::Stencil;
                if (theDepthStencil.m_Flags.HasClearDepth())
                    clearFlags |= QDemonRenderClearValues::Depth;

                if (clearFlags)
                    theContext.Clear(QDemonRenderClearFlags(clearFlags));

                QDemonRenderDepthStencilState *targetState = nullptr;
                for (quint32 idx = 0, end = m_DepthStencilStates.size();
                     idx < end && targetState == nullptr; ++idx) {
                    QDemonRenderDepthStencilState &theState = *m_DepthStencilStates[idx];
                    if (CompareDepthStencilState(theState, theDepthStencil))
                        targetState = &theState;
                }
                if (targetState == nullptr) {
                    QDemonRenderStencilFunctionArgument theFunctionArg(
                                theDepthStencil.m_StencilFunction, theDepthStencil.m_Reference,
                                theDepthStencil.m_Mask);
                    QDemonRenderStencilOperationArgument theOpArg(
                                theDepthStencil.m_StencilFailOperation,
                                theDepthStencil.m_DepthFailOperation, theDepthStencil.m_DepthPassOperation);
                    targetState = theContext.CreateDepthStencilState(
                                theContext.IsDepthTestEnabled(), theContext.IsDepthWriteEnabled(),
                                theContext.GetDepthFunction(), true, theFunctionArg, theFunctionArg,
                                theOpArg, theOpArg);
                    m_DepthStencilStates.push_back(targetState);
                }
                theContext.SetDepthStencilState(targetState);
            }
        }

        theContext.SetActiveShader(inShader.m_Shader);
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
            m_Context->GetRenderer().RenderQuad();
        else
            m_Context->GetRenderer().RenderPointsIndirect();

        if (inDepthStencil && inFrameBuffer) {
            inFrameBuffer->Attach(QDemonRenderFrameBufferAttachments::DepthStencil,
                                  QDemonRenderTextureOrRenderBuffer());
            theContext.SetDepthStencilState(m_DefaultStencilState);
        }
    }

    void DoRenderEffect(SEffect &inEffect, SEffectClass &inClass,
                        QDemonRenderTexture2D &inSourceTexture, QMatrix4x4 &inMVP,
                        QDemonRenderFrameBuffer *inTarget, bool inEnableBlendWhenRenderToTarget,
                        QDemonRenderTexture2D *inDepthTexture, QDemonRenderTexture2D *inDepthStencilTexture,
                        const QVector2D inCameraClipRange)
    {
        // Run through the effect commands and render the effect.
        // QDemonRenderTexture2D* theCurrentTexture(&inSourceTexture);
        QDemonRenderContext &theContext = m_Context->GetRenderContext();

        // Context variables that are updated during the course of a pass.
        SEffectTextureData theCurrentSourceTexture(&inSourceTexture, false);
        QDemonRenderTexture2D *theCurrentDepthStencilTexture = nullptr;
        QDemonRenderFrameBuffer *theCurrentRenderTarget(inTarget);
        SEffectShader *theCurrentShader(nullptr);
        QDemonRenderRect theOriginalViewport(theContext.GetViewport());
        bool wasScissorEnabled = theContext.IsScissorTestEnabled();
        bool wasBlendingEnabled = theContext.IsBlendingEnabled();
        // save current blending setup
        QDemonRenderBlendFunctionArgument theBlendFunc = theContext.GetBlendFunction();
        QDemonRenderBlendEquationArgument theBlendEqu = theContext.GetBlendEquation();
        bool intermediateBlendingEnabled = false;
        STextureDetails theDetails(inSourceTexture.GetTextureDetails());
        quint32 theFinalWidth = (quint32)(theDetails.m_Width);
        quint32 theFinalHeight = (quint32)(theDetails.m_Height);
        QVector2D theDestSize;
        {
            // Ensure no matter the command run goes we replace the rendering system to some
            // semblance of the approprate
            // setting.
            QDemonRenderContextScopedProperty<QDemonRenderFrameBuffer *> __framebuffer(
                        theContext, &QDemonRenderContext::GetRenderTarget, &QDemonRenderContext::SetRenderTarget);
            QDemonRenderContextScopedProperty<QDemonRenderRect> __viewport(
                        theContext, &QDemonRenderContext::GetViewport, &QDemonRenderContext::SetViewport);
            QDemonRenderContextScopedProperty<bool> __scissorEnabled(
                        theContext, &QDemonRenderContext::IsScissorTestEnabled,
                        &QDemonRenderContext::SetScissorTestEnabled);
            QDemonRenderContextScopedProperty<bool> __stencilTest(
                        theContext, &QDemonRenderContext::IsStencilTestEnabled,
                        &QDemonRenderContext::SetStencilTestEnabled);
            QDemonRenderContextScopedProperty<QDemonRenderBoolOp::Enum> __depthFunction(
                        theContext, &QDemonRenderContext::GetDepthFunction, &QDemonRenderContext::SetDepthFunction);
            Option<SDepthStencil> theCurrentDepthStencil;

            theContext.SetScissorTestEnabled(false);
            theContext.SetBlendingEnabled(false);
            theContext.SetCullingEnabled(false);
            theContext.SetDepthTestEnabled(false);
            theContext.SetDepthWriteEnabled(false);

            QMatrix4x4 theMVP(QMatrix4x4::createIdentity());
            QDemonConstDataRef<dynamic::SCommand *> theCommands =
                    inClass.m_DynamicClass->GetRenderCommands();
            for (quint32 commandIdx = 0, commandEnd = theCommands.size(); commandIdx < commandEnd;
                 ++commandIdx) {
                const SCommand &theCommand(*theCommands[commandIdx]);
                switch (theCommand.m_Type) {
                case CommandTypes::AllocateBuffer:
                    AllocateBuffer(inEffect, static_cast<const SAllocateBuffer &>(theCommand),
                                   theFinalWidth, theFinalHeight, theDetails.m_Format);
                    break;

                case CommandTypes::AllocateImage:
                    AllocateImage(inEffect, static_cast<const SAllocateImage &>(theCommand),
                                  theFinalWidth, theFinalHeight);
                    break;

                case CommandTypes::AllocateDataBuffer:
                    AllocateDataBuffer(inEffect,
                                       static_cast<const SAllocateDataBuffer &>(theCommand));
                    break;

                case CommandTypes::BindBuffer:
                    theCurrentRenderTarget =
                            BindBuffer(inEffect, static_cast<const SBindBuffer &>(theCommand), theMVP,
                                       theDestSize);
                    break;

                case CommandTypes::BindTarget: {
                    m_Context->GetRenderContext().SetRenderTarget(inTarget);
                    theCurrentRenderTarget = inTarget;
                    theMVP = inMVP;
                    theContext.SetViewport(theOriginalViewport);
                    theDestSize = QVector2D((float)theFinalWidth, (float)theFinalHeight);
                    // This isn't necessary if we are rendering to an offscreen buffer and not
                    // compositing
                    // with other objects.
                    if (inEnableBlendWhenRenderToTarget) {
                        theContext.SetBlendingEnabled(wasBlendingEnabled);
                        theContext.SetScissorTestEnabled(wasScissorEnabled);
                        // The blending setup was done before we apply the effect
                        theContext.SetBlendFunction(theBlendFunc);
                        theContext.SetBlendEquation(theBlendEqu);
                    }
                } break;
                case CommandTypes::BindShader:
                    theCurrentShader = BindShader(inEffect.m_ClassName,
                                                  static_cast<const SBindShader &>(theCommand));
                    break;
                case CommandTypes::ApplyInstanceValue:
                    if (theCurrentShader)
                        ApplyInstanceValue(inEffect, inClass, *theCurrentShader->m_Shader,
                                           static_cast<const SApplyInstanceValue &>(theCommand));
                    break;
                case CommandTypes::ApplyValue:
                    if (theCurrentShader)
                        ApplyValue(inEffect, inClass, *theCurrentShader->m_Shader,
                                   static_cast<const SApplyValue &>(theCommand));
                    break;
                case CommandTypes::ApplyBlending:
                    intermediateBlendingEnabled =
                            ApplyBlending(static_cast<const SApplyBlending &>(theCommand));
                    break;
                case CommandTypes::ApplyBufferValue:
                    if (theCurrentShader)
                        theCurrentSourceTexture =
                                ApplyBufferValue(inEffect, *theCurrentShader->m_Shader,
                                                 static_cast<const SApplyBufferValue &>(theCommand),
                                                 inSourceTexture, theCurrentSourceTexture);
                    break;
                case CommandTypes::ApplyDepthValue:
                    if (theCurrentShader)
                        ApplyDepthValue(inEffect, *theCurrentShader->m_Shader,
                                        static_cast<const SApplyDepthValue &>(theCommand),
                                        inDepthTexture);
                    if (!inDepthTexture) {
                        qCCritical(INVALID_OPERATION,
                                   "Depth value command detected but no "
                                   "depth buffer provided for effect %s",
                                   inEffect.m_ClassName.c_str());
                        Q_ASSERT(false);
                    }
                    break;
                case CommandTypes::ApplyImageValue:
                    if (theCurrentShader)
                        ApplyImageValue(inEffect, *theCurrentShader->m_Shader,
                                        static_cast<const SApplyImageValue &>(theCommand));
                    break;
                case CommandTypes::ApplyDataBufferValue:
                    if (theCurrentShader)
                        ApplyDataBufferValue(
                                    inEffect, *theCurrentShader->m_Shader,
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
                    theCurrentSourceTexture = SEffectTextureData(&inSourceTexture, false);
                    theCurrentDepthStencilTexture = nullptr;
                    theCurrentDepthStencil = Option<SDepthStencil>();
                    // reset intermediate blending state
                    if (intermediateBlendingEnabled) {
                        theContext.SetBlendingEnabled(false);
                        intermediateBlendingEnabled = false;
                    }
                    break;
                case CommandTypes::ApplyRenderState:
                    ApplyRenderStateValue(theCurrentRenderTarget, inDepthStencilTexture,
                                          static_cast<const SApplyRenderState &>(theCommand));
                    break;
                default:
                    Q_ASSERT(false);
                    break;
                }
            }

            SetEffectRequiresCompilation(inEffect.m_ClassName, false);

            // reset to default stencil state
            if (inDepthStencilTexture) {
                theContext.SetDepthStencilState(m_DefaultStencilState);
            }

            // Release any per-frame buffers
            if (inEffect.m_Context) {
                SEffectContext &theContext(*inEffect.m_Context);
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

    QDemonRenderTexture2D *RenderEffect(SEffectRenderArgument inRenderArgument) override
    {
        SEffectClass *theClass = GetEffectClass(inRenderArgument.m_Effect.m_ClassName);
        if (!theClass) {
            Q_ASSERT(false);
            return nullptr;
        }
        QMatrix4x4 theMVP;
        SCamera::SetupOrthographicCameraForOffscreenRender(inRenderArgument.m_ColorBuffer, theMVP);
        // setup a render target
        QDemonRenderContext &theContext(m_Context->GetRenderContext());
        IResourceManager &theManager(m_Context->GetResourceManager());
        QDemonRenderContextScopedProperty<QDemonRenderFrameBuffer *> __framebuffer(
                    theContext, &QDemonRenderContext::GetRenderTarget, &QDemonRenderContext::SetRenderTarget);
        STextureDetails theDetails(inRenderArgument.m_ColorBuffer.GetTextureDetails());
        quint32 theFinalWidth = ITextRenderer::NextMultipleOf4((quint32)(theDetails.m_Width));
        quint32 theFinalHeight = ITextRenderer::NextMultipleOf4((quint32)(theDetails.m_Height));
        QDemonRenderFrameBuffer *theBuffer = theManager.AllocateFrameBuffer();
        // UdoL Some Effects may need to run before HDR tonemap. This means we need to keep the
        // input format
        QDemonRenderTextureFormats::Enum theOutputFormat = QDemonRenderTextureFormats::RGBA8;
        if (theClass->m_DynamicClass->GetOutputTextureFormat() == QDemonRenderTextureFormats::Unknown)
            theOutputFormat = theDetails.m_Format;
        QDemonRenderTexture2D *theTargetTexture =
                theManager.AllocateTexture2D(theFinalWidth, theFinalHeight, theOutputFormat);
        theBuffer->Attach(QDemonRenderFrameBufferAttachments::Color0, *theTargetTexture);
        theContext.SetRenderTarget(theBuffer);
        QDemonRenderContextScopedProperty<QDemonRenderRect> __viewport(
                    theContext, &QDemonRenderContext::GetViewport, &QDemonRenderContext::SetViewport,
                    QDemonRenderRect(0, 0, theFinalWidth, theFinalHeight));

        QDemonRenderContextScopedProperty<bool> __scissorEnable(
                    theContext, &QDemonRenderContext::IsScissorTestEnabled,
                    &QDemonRenderContext::SetScissorTestEnabled, false);

        DoRenderEffect(inRenderArgument.m_Effect, *theClass, inRenderArgument.m_ColorBuffer, theMVP,
                       m_Context->GetRenderContext().GetRenderTarget(), false,
                       inRenderArgument.m_DepthTexture, inRenderArgument.m_DepthStencilBuffer,
                       inRenderArgument.m_CameraClipRange);

        theBuffer->Attach(QDemonRenderFrameBufferAttachments::Color0, QDemonRenderTextureOrRenderBuffer());
        theManager.Release(*theBuffer);
        return theTargetTexture;
    }

    // Render the effect to the currently bound render target using this MVP
    bool RenderEffect(SEffectRenderArgument inRenderArgument, QMatrix4x4 &inMVP,
                      bool inEnableBlendWhenRenderToTarget) override
    {
        SEffectClass *theClass = GetEffectClass(inRenderArgument.m_Effect.m_ClassName);
        if (!theClass) {
            Q_ASSERT(false);
            return false;
        }

        DoRenderEffect(inRenderArgument.m_Effect, *theClass, inRenderArgument.m_ColorBuffer, inMVP,
                       m_Context->GetRenderContext().GetRenderTarget(),
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
                m_Contexts.replace_with_last(idx);
                NVDelete(m_Allocator, inContext);
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

    void SetShaderData(CRegisteredString path, const char8_t *data,
                       const char8_t *inShaderType, const char8_t *inShaderVersion,
                       bool inHasGeomShader, bool inIsComputeShader) override
    {
        m_CoreContext.GetDynamicObjectSystemCore().SetShaderData(
                    path, data, inShaderType, inShaderVersion, inHasGeomShader, inIsComputeShader);
    }

    void Save(SWriteBuffer &ioBuffer,
              const SStrRemapMap &inRemapMap, const char8_t *inProjectDir) const override
    {
        ioBuffer.write((quint32)m_EffectClasses.size());
        SStringSaveRemapper theRemapper(m_Allocator, inRemapMap, inProjectDir,
                                        m_CoreContext.GetStringTable());
        for (TEffectClassMap::const_iterator theIter = m_EffectClasses.begin(),
             end = m_EffectClasses.end();
             theIter != end; ++theIter) {
            const SEffectClass &theClass = *theIter->second;
            CRegisteredString theClassName = theClass.m_DynamicClass->GetId();
            theClassName.Remap(inRemapMap);
            ioBuffer.write(theClassName);
            // Effect classes do not store any additional data from the dynamic object class.
            ioBuffer.write(theClass);
        }
    }

    void Load(QDemonDataRef<quint8> inData, CStrTableOrDataRef inStrDataBlock,
              const char8_t *inProjectDir) override
    {
        m_Allocator.m_PreAllocatedBlock = inData;
        m_Allocator.m_OwnsMemory = false;
        SDataReader theReader(inData.begin(), inData.end());
        quint32 numEffectClasses = theReader.LoadRef<quint32>();
        SStringLoadRemapper theRemapper(m_Allocator, inStrDataBlock, inProjectDir,
                                        m_CoreContext.GetStringTable());
        for (quint32 idx = 0, end = numEffectClasses; idx < end; ++idx) {
            CRegisteredString theClassName = theReader.LoadRef<CRegisteredString>();
            theClassName.Remap(inStrDataBlock);
            IDynamicObjectClass *theBaseClass =
                    m_CoreContext.GetDynamicObjectSystemCore().GetDynamicObjectClass(theClassName);
            if (theBaseClass == nullptr) {
                Q_ASSERT(false);
                return;
            }
            SEffectClass *theClass = theReader.Load<SEffectClass>();
            theClass->SetupThisObjectFromMemory(m_Allocator, *theBaseClass);
            QDemonScopedRefCounted<SEffectClass> theClassPtr(theClass);
            m_EffectClasses.insert(eastl::make_pair(theBaseClass->GetId(), theClassPtr));
        }
    }

    IEffectSystem &GetEffectSystem(IQt3DSRenderContext &context) override
    {
        m_Context = &context;

        QDemonRenderContext &theContext(m_Context->GetRenderContext());

        m_ResourceManager = &IResourceManager::CreateResourceManager(theContext);

        // create default stencil state
        QDemonRenderStencilFunctionArgument stencilDefaultFunc(
                    QDemonRenderBoolOp::AlwaysTrue, 0x0, 0xFF);
        QDemonRenderStencilOperationArgument stencilDefaultOp(
                    QDemonRenderStencilOp::Keep, QDemonRenderStencilOp::Keep,
                    QDemonRenderStencilOp::Keep);
        m_DefaultStencilState = theContext.CreateDepthStencilState(
                    theContext.IsDepthTestEnabled(), theContext.IsDepthWriteEnabled(),
                    theContext.GetDepthFunction(), theContext.IsStencilTestEnabled(), stencilDefaultFunc,
                    stencilDefaultFunc, stencilDefaultOp, stencilDefaultOp);

        return *this;
    }

    IResourceManager &GetResourceManager() override
    {
        return *m_ResourceManager;
    }
};
}

IEffectSystemCore &IEffectSystemCore::CreateEffectSystemCore(IQt3DSRenderContextCore &inContext)
{
    return *QDEMON_NEW(inContext.GetAllocator(), SEffectSystem)(inContext);
}

QT_END_NAMESPACE
