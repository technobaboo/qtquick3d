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

struct QDemonEffectClass
{
    QAtomicInt ref;
    QDemonDynamicObjectClassInterface *dynamicClass;

    QDemonEffectClass(QDemonDynamicObjectClassInterface &dynClass) : dynamicClass(&dynClass) {}

    void setupThisObjectFromMemory(QDemonDynamicObjectClassInterface &inClass) { dynamicClass = &inClass; }
};

struct QDemonAllocatedBufferEntry
{
    QAtomicInt ref;
    QString name;
    QDemonRef<QDemonRenderFrameBuffer> frameBuffer;
    QDemonRef<QDemonRenderTexture2D> texture;
    QDemonAllocateBufferFlags flags;
    bool needsClear;

    QDemonAllocatedBufferEntry(const QString &inName, QDemonRenderFrameBuffer &inFb, QDemonRenderTexture2D &inTexture, QDemonAllocateBufferFlags inFlags)
        : name(inName), frameBuffer(&inFb), texture(&inTexture), flags(inFlags), needsClear(true)
    {
    }
    QDemonAllocatedBufferEntry() = default;
};

struct QDemonAllocatedImageEntry
{
    QAtomicInt ref;
    QString name;
    QDemonRef<QDemonRenderImage2D> image;
    QDemonRef<QDemonRenderTexture2D> texture;
    QDemonAllocateBufferFlags flags;

    QDemonAllocatedImageEntry(QString inName, QDemonRenderImage2D &inImage, QDemonRenderTexture2D &inTexture, QDemonAllocateBufferFlags inFlags)
        : name(inName), image(&inImage), texture(&inTexture), flags(inFlags)
    {
    }
    QDemonAllocatedImageEntry() = default;
};

struct QDemonImageEntry
{
    QAtomicInt ref;
    QDemonRef<QDemonRenderShaderProgram> shader;
    QDemonRenderCachedShaderProperty<QDemonRenderImage2D *> image;

    QDemonImageEntry(const QDemonRef<QDemonRenderShaderProgram> &inShader, const char *inImageName)
        : shader(inShader), image(inImageName, inShader)
    {
    }

    void set(QDemonRenderImage2D *inImage) { image.set(inImage); }

    static QDemonImageEntry createImageEntry(const QDemonRef<QDemonRenderShaderProgram> &inShader, const char *inStem)
    {
        return QDemonImageEntry(inShader, inStem);
    }
};

struct QDemonAllocatedDataBufferEntry
{
    QAtomicInt ref;
    QString name;
    QDemonRef<QDemonRenderDataBuffer> dataBuffer;
    QDemonRenderBufferBindValues bufferType;
    QDemonDataRef<quint8> bufferData;
    QDemonAllocateBufferFlags flags;
    bool needsClear;

    QDemonAllocatedDataBufferEntry(const QString &inName,
                                   QDemonRenderDataBuffer &inDataBuffer,
                                   QDemonRenderBufferBindValues inType,
                                   const QDemonDataRef<quint8> &data,
                                   QDemonAllocateBufferFlags inFlags)
        : name(inName), dataBuffer(&inDataBuffer), bufferType(inType), bufferData(data), flags(inFlags), needsClear(false)
    {
    }
    QDemonAllocatedDataBufferEntry() = default;
};

struct QDemonDataBufferEntry
{
    QAtomicInt ref;
    QDemonRef<QDemonRenderShaderProgram> shader;
    QDemonRenderCachedShaderBuffer<QDemonRenderShaderBufferBase> dataBuffer;

    QDemonDataBufferEntry(const QDemonRef<QDemonRenderShaderProgram> &inShader, const char *inBufferName)
        : shader(inShader), dataBuffer(inBufferName, inShader)
    {
    }

    void set(QDemonRenderDataBuffer *inBuffer)
    {
        if (inBuffer)
            inBuffer->bind();

        dataBuffer.set();
    }

    static QDemonDataBufferEntry createDataBufferEntry(const QDemonRef<QDemonRenderShaderProgram> &inShader, const char *inStem)
    {
        return QDemonDataBufferEntry(inShader, inStem);
    }
};

struct QDemonEffectTextureData
{
    QDemonRef<QDemonRenderTexture2D> texture;
    bool needsAlphaMultiply = false;
    QDemonEffectTextureData(const QDemonRef<QDemonRenderTexture2D> &inTexture, bool inNeedsMultiply)
        : texture(inTexture), needsAlphaMultiply(inNeedsMultiply)
    {
    }
    QDemonEffectTextureData() = default;
};

struct QDemonTextureEntry
{
    QAtomicInt ref;
    QDemonRef<QDemonRenderShaderProgram> shader;
    QDemonRenderCachedShaderProperty<QDemonRenderTexture2D *> texture;
    QDemonRenderCachedShaderProperty<QVector4D> textureData;
    QDemonRenderCachedShaderProperty<qint32> textureFlags;

    QDemonTextureEntry(const QDemonRef<QDemonRenderShaderProgram> &inShader, const char *inTexName, const char *inDataName, const char *inFlagName)
        : shader(inShader), texture(inTexName, inShader), textureData(inDataName, inShader), textureFlags(inFlagName, inShader)
    {
    }

    void set(const QDemonRef<QDemonRenderTexture2D> &inTexture, bool inNeedsAlphaMultiply, const QDemonPropertyDefinition *inDefinition)
    {
        float theMixValue(inNeedsAlphaMultiply ? 0.0f : 1.0f);
        if (inTexture && inDefinition) {
            inTexture->setMagFilter(inDefinition->magFilterOp);
            inTexture->setMinFilter(static_cast<QDemonRenderTextureMinifyingOp::Enum>(inDefinition->magFilterOp));
            inTexture->setTextureWrapS(inDefinition->coordOp);
            inTexture->setTextureWrapT(inDefinition->coordOp);
        }
        texture.set(inTexture.data());
        if (inTexture) {
            QDemonTextureDetails theDetails(inTexture->getTextureDetails());
            textureData.set(QVector4D((float)theDetails.width, (float)theDetails.height, theMixValue, 0.0f));
            // I have no idea what these flags do.
            textureFlags.set(1);
        } else {
            textureFlags.set(0);
        }
    }

    static QDemonTextureEntry createTextureEntry(const QDemonRef<QDemonRenderShaderProgram> &inShader,
                                                 const char *inStem,
                                                 QString &inBuilder,
                                                 QString &inBuilder2)
    {
        inBuilder = inStem;
        inBuilder.append("Info");
        inBuilder2 = "flag";
        inBuilder2.append(inStem);
        return QDemonTextureEntry(inShader, inStem, inBuilder.toLocal8Bit(), inBuilder2.toLocal8Bit());
    }
};

typedef QPair<QString, QDemonRef<QDemonTextureEntry>> TNamedTextureEntry;
typedef QPair<QString, QDemonRef<QDemonImageEntry>> TNamedImageEntry;
typedef QPair<QString, QDemonRef<QDemonDataBufferEntry>> TNamedDataBufferEntry;
}

struct QDemonEffectContext
{
    QAtomicInt ref;
    QString m_className;
    QDemonRenderContextInterface *m_context;
    QDemonRef<QDemonResourceManagerInterface> m_resourceManager;
    QVector<QDemonAllocatedBufferEntry> m_allocatedBuffers;
    QVector<QDemonAllocatedImageEntry> m_allocatedImages;
    QVector<QDemonAllocatedDataBufferEntry> m_allocatedDataBuffers;
    QVector<TNamedTextureEntry> m_textureEntries;
    QVector<TNamedImageEntry> m_imageEntries;
    QVector<TNamedDataBufferEntry> m_dataBufferEntries;

    QDemonEffectContext(const QString &inName, QDemonRenderContextInterface *ctx, const QDemonRef<QDemonResourceManagerInterface> &inManager)
        : m_className(inName), m_context(ctx), m_resourceManager(inManager)
    {
    }

    ~QDemonEffectContext()
    {
        while (m_allocatedBuffers.size())
            releaseBuffer(0);

        while (m_allocatedImages.size())
            releaseImage(0);

        while (m_allocatedDataBuffers.size())
            releaseDataBuffer(0);
    }

    void releaseBuffer(qint32 inIdx)
    {
        QDemonAllocatedBufferEntry &theEntry(m_allocatedBuffers[inIdx]);
        theEntry.frameBuffer->attach(QDemonRenderFrameBufferAttachment::Color0, QDemonRenderTextureOrRenderBuffer());
        m_resourceManager->release(theEntry.frameBuffer);
        m_resourceManager->release(theEntry.texture);
        { // replace_with_last
            m_allocatedBuffers[inIdx] = m_allocatedBuffers.back();
            m_allocatedBuffers.pop_back();
        }
    }

    void releaseImage(qint32 inIdx)
    {
        QDemonAllocatedImageEntry &theEntry(m_allocatedImages[inIdx]);
        m_resourceManager->release(theEntry.image);
        m_resourceManager->release(theEntry.texture);
        { // replace_with_last
            m_allocatedImages[inIdx] = m_allocatedImages.back();
            m_allocatedImages.pop_back();
        }
    }

    void releaseDataBuffer(qint32 inIdx)
    {
        QDemonAllocatedDataBufferEntry &theEntry(m_allocatedDataBuffers[inIdx]);
        ::free(theEntry.bufferData.begin());
        { // replace_with_last
            m_allocatedDataBuffers[inIdx] = m_allocatedDataBuffers.back();
            m_allocatedDataBuffers.pop_back();
        }
    }

    qint32 findBuffer(const QString &inName)
    {
        for (qint32 idx = 0, end = m_allocatedBuffers.size(); idx < end; ++idx)
            if (m_allocatedBuffers[idx].name == inName)
                return idx;
        return m_allocatedBuffers.size();
    }

    qint32 findImage(const QString &inName)
    {
        for (qint32 idx = 0, end = m_allocatedImages.size(); idx < end; ++idx)
            if (m_allocatedImages[idx].name == inName)
                return idx;

        return m_allocatedImages.size();
    }

    qint32 findDataBuffer(const QString &inName)
    {
        for (qint32 idx = 0, end = m_allocatedDataBuffers.size(); idx < end; ++idx) {
            if (m_allocatedDataBuffers[idx].name == inName)
                return idx;
        }

        return m_allocatedDataBuffers.size();
    }

    void setTexture(const QDemonRef<QDemonRenderShaderProgram> &inShader,
                    const QString &inPropName,
                    const QDemonRef<QDemonRenderTexture2D> &inTexture,
                    bool inNeedsMultiply,
                    QString &inStringBuilder,
                    QString &inStringBuilder2,
                    const QDemonPropertyDefinition *inPropDec = nullptr)
    {
        QDemonRef<QDemonTextureEntry> theTextureEntry;
        for (qint32 idx = 0, end = m_textureEntries.size(); idx < end && theTextureEntry == nullptr; ++idx) {
            if (m_textureEntries[idx].first == inPropName && m_textureEntries[idx].second->shader == inShader)
                theTextureEntry = m_textureEntries[idx].second;
        }
        if (theTextureEntry == nullptr) {
            QDemonRef<QDemonTextureEntry> theNewEntry(new QDemonTextureEntry(
                    QDemonTextureEntry::createTextureEntry(inShader, inPropName.toLatin1(), inStringBuilder, inStringBuilder2)));
            m_textureEntries.push_back(QPair<QString, QDemonRef<QDemonTextureEntry>>(inPropName, theNewEntry));
            theTextureEntry = theNewEntry;
        }
        theTextureEntry->set(inTexture, inNeedsMultiply, inPropDec);
    }

    void setImage(const QDemonRef<QDemonRenderShaderProgram> &inShader, const QString &inPropName, const QDemonRef<QDemonRenderImage2D> &inImage)
    {
        QDemonRef<QDemonImageEntry> theImageEntry;
        for (qint32 idx = 0, end = m_imageEntries.size(); idx < end && theImageEntry == nullptr; ++idx) {
            if (m_imageEntries[idx].first == inPropName && m_imageEntries[idx].second->shader == inShader)
                theImageEntry = m_imageEntries[idx].second;
        }
        if (theImageEntry == nullptr) {
            QDemonRef<QDemonImageEntry> theNewEntry(
                    new QDemonImageEntry(QDemonImageEntry::createImageEntry(inShader, inPropName.toLatin1())));
            m_imageEntries.push_back(QPair<QString, QDemonRef<QDemonImageEntry>>(inPropName, theNewEntry));
            theImageEntry = theNewEntry;
        }

        theImageEntry->set(inImage.data());
    }

    void setDataBuffer(const QDemonRef<QDemonRenderShaderProgram> &inShader,
                       const QString &inPropName,
                       const QDemonRef<QDemonRenderDataBuffer> &inBuffer)
    {
        QDemonRef<QDemonDataBufferEntry> theDataBufferEntry;
        for (qint32 idx = 0, end = m_dataBufferEntries.size(); idx < end && theDataBufferEntry == nullptr; ++idx) {
            if (m_dataBufferEntries[idx].first == inPropName && m_dataBufferEntries[idx].second->shader == inShader)
                theDataBufferEntry = m_dataBufferEntries[idx].second;
        }
        if (theDataBufferEntry == nullptr) {
            QDemonRef<QDemonDataBufferEntry> theNewEntry(new QDemonDataBufferEntry(
                    QDemonDataBufferEntry::createDataBufferEntry(inShader, inPropName.toLatin1())));
            m_dataBufferEntries.push_back(QPair<QString, QDemonRef<QDemonDataBufferEntry>>(inPropName, theNewEntry));
            theDataBufferEntry = theNewEntry;
        }

        theDataBufferEntry->set(inBuffer.data());
    }
};

namespace {

/* We setup some shared state on the effect shaders */
struct QDemonEffectShader
{
    QAtomicInt ref;
    QDemonRef<QDemonRenderShaderProgram> m_shader;
    QDemonRenderCachedShaderProperty<QMatrix4x4> m_mvp;
    QDemonRenderCachedShaderProperty<QVector2D> m_fragColorAlphaSettings;
    QDemonRenderCachedShaderProperty<QVector2D> m_destSize;
    QDemonRenderCachedShaderProperty<float> m_appFrame;
    QDemonRenderCachedShaderProperty<float> m_fps;
    QDemonRenderCachedShaderProperty<QVector2D> m_cameraClipRange;
    QDemonTextureEntry m_textureEntry;
    QDemonEffectShader(const QDemonRef<QDemonRenderShaderProgram> &inShader)
        : m_shader(inShader)
        , m_mvp("ModelViewProjectionMatrix", inShader)
        , m_fragColorAlphaSettings("FragColorAlphaSettings", inShader)
        , m_destSize("DestSize", inShader)
        , m_appFrame("AppFrame", inShader)
        , m_fps("FPS", inShader)
        , m_cameraClipRange("CameraClipRange", inShader)
        , m_textureEntry(inShader, "Texture0", "Texture0Info", "Texture0Flags")
    {
    }
};

struct QDemonEffectSystem : public QDemonEffectSystemInterface
{
    typedef QHash<QString, char *> TPathDataMap;
    typedef QSet<QString> TPathSet;
    typedef QHash<QString, QDemonRef<QDemonEffectClass>> TEffectClassMap;
    typedef QHash<TStrStrPair, QDemonRef<QDemonEffectShader>> TShaderMap;
    typedef QVector<QDemonRef<QDemonEffectContext>> TContextList;

    QDemonRenderContextCoreInterface *m_coreContext;
    QDemonRenderContextInterface *m_context;
    QDemonRef<QDemonResourceManagerInterface> m_resourceManager;
    // Keep from dual-including headers.
    TEffectClassMap m_effectClasses;
    QVector<QString> m_effectList;
    TContextList m_contexts;
    QString m_textureStringBuilder;
    QString m_textureStringBuilder2;
    TShaderMap m_shaderMap;
    QDemonRef<QDemonRenderDepthStencilState> m_defaultStencilState;
    QVector<QDemonRef<QDemonRenderDepthStencilState>> m_depthStencilStates;

    QDemonEffectSystem(QDemonRenderContextCoreInterface *inContext) : m_coreContext(inContext), m_context(nullptr) {}

    //    ~QDemonEffectSystem() override
    //    {
    ////        for (quint32 idx = 0, end = m_Contexts.size(); idx < end; ++idx)
    ////            delete m_Contexts[idx];
    //        m_Contexts.clear();
    //    }

    QDemonEffectContext &getEffectContext(QDemonRenderEffect &inEffect)
    {
        if (inEffect.m_context == nullptr) {
            inEffect.m_context = QDemonRef<QDemonEffectContext>(new QDemonEffectContext(inEffect.className, m_context, m_resourceManager));
            m_contexts.push_back(inEffect.m_context);
        }
        return *inEffect.m_context;
    }

    QDemonRef<QDemonEffectClass> getEffectClass(const QString &inStr)
    {
        TEffectClassMap::iterator theIter = m_effectClasses.find(inStr);
        if (theIter != m_effectClasses.end())
            return theIter.value();
        return nullptr;
    }
    const QDemonRef<QDemonEffectClass> getEffectClass(const QString &inStr) const
    {
        return const_cast<QDemonEffectSystem *>(this)->getEffectClass(inStr);
    }

    bool isEffectRegistered(QString inStr) override { return getEffectClass(inStr) != nullptr; }
    QVector<QString> getRegisteredEffects() override
    {
        m_effectList.clear();
        for (TEffectClassMap::iterator theIter = m_effectClasses.begin(), theEnd = m_effectClasses.end(); theIter != theEnd; ++theIter)
            m_effectList.push_back(theIter.key());
        return m_effectList;
    }

    // Registers an effect that runs via a single GLSL file.
    bool registerGLSLEffect(QString inName, const char *inPathToEffect, QDemonConstDataRef<QDemonPropertyDeclaration> inProperties) override
    {
        if (isEffectRegistered(inName))
            return false;

        m_coreContext->getDynamicObjectSystemCore()->doRegister(inName, inProperties, sizeof(QDemonRenderEffect), QDemonGraphObjectTypes::Effect);
        QDemonDynamicObjectClassInterface &theClass = *m_coreContext->getDynamicObjectSystemCore()->getDynamicObjectClass(inName);

        QDemonRef<QDemonEffectClass> theEffect(new QDemonEffectClass(theClass));
        m_effectClasses.insert(inName, theEffect);

        // Setup the commands required to run this effect
        //        StaticAssert<(sizeof(SBindShader) % 4 == 0)>::valid_expression();
        //        StaticAssert<(sizeof(SApplyInstanceValue) % 4 == 0)>::valid_expression();
        //        StaticAssert<(sizeof(SRender) % 4 == 0)>::valid_expression();

        quint32 commandAllocationSize = sizeof(QDemonBindTarget);
        commandAllocationSize += sizeof(QDemonBindShader);
        commandAllocationSize += sizeof(QDemonApplyInstanceValue) * inProperties.size();
        commandAllocationSize += sizeof(QDemonRender);

        quint32 commandCount = 3 + inProperties.size();
        quint32 commandPtrAllocationSize = commandCount * sizeof(QDemonCommand *);
        quint32 allocationSize = Align8(commandAllocationSize) + commandPtrAllocationSize;
        quint8 *startBuffer = (quint8 *)::malloc(allocationSize);
        quint8 *dataBuffer = startBuffer;
        // Setup the command buffer such that the ptrs to the commands and the commands themselves
        // are
        // all in the same block of memory.  This enables quicker iteration (trivially quicker, most
        // likely)
        // but it also enables simpler memory management (single free).
        // Furthermore, for a single glsl file the effect properties map precisely into the
        // glsl file itself.
        QDemonCommand **theFirstCommandPtr = (reinterpret_cast<QDemonCommand **>(dataBuffer + Align8(commandAllocationSize)));
        QDemonCommand **theCommandPtr = theFirstCommandPtr;
        memZero(dataBuffer, commandAllocationSize);

        new (dataBuffer) QDemonBindTarget();
        *theCommandPtr = (QDemonCommand *)dataBuffer;
        ++theCommandPtr;
        dataBuffer += sizeof(QDemonBindTarget);

        new (dataBuffer) QDemonBindShader(QString::fromLocal8Bit(inPathToEffect));
        *theCommandPtr = (QDemonCommand *)dataBuffer;
        ++theCommandPtr;
        dataBuffer += sizeof(QDemonBindShader);

        for (quint32 idx = 0, end = inProperties.size(); idx < end; ++idx) {
            const QDemonPropertyDefinition &theDefinition(theEffect->dynamicClass->getProperties()[idx]);
            new (dataBuffer) QDemonApplyInstanceValue(theDefinition.name, theDefinition.dataType, theDefinition.offset);
            *theCommandPtr = (QDemonCommand *)dataBuffer;
            ++theCommandPtr;
            dataBuffer += sizeof(QDemonApplyInstanceValue);
        }
        new (dataBuffer) QDemonRender(false);
        *theCommandPtr = (QDemonCommand *)dataBuffer;
        ++theCommandPtr;
        dataBuffer += sizeof(QDemonRender);
        // Ensure we end up *exactly* where we expected to.
        Q_ASSERT(dataBuffer == startBuffer + commandAllocationSize);
        Q_ASSERT(theCommandPtr - theFirstCommandPtr == (int)inProperties.size() + 3);
        m_coreContext->getDynamicObjectSystemCore()->setRenderCommands(inName, QDemonConstDataRef<QDemonCommand *>(theFirstCommandPtr, commandCount));
        ::free(startBuffer);
        return true;
    }

    void setEffectPropertyDefaultValue(QString inName, QString inPropName, QDemonConstDataRef<quint8> inDefaultData) override
    {
        m_coreContext->getDynamicObjectSystemCore()->setPropertyDefaultValue(inName, inPropName, inDefaultData);
    }

    void setEffectPropertyEnumNames(QString inName, QString inPropName, QDemonConstDataRef<QString> inNames) override
    {
        m_coreContext->getDynamicObjectSystemCore()->setPropertyEnumNames(inName, inPropName, inNames);
    }

    bool registerEffect(QString inName, QDemonConstDataRef<QDemonPropertyDeclaration> inProperties) override
    {
        if (isEffectRegistered(inName))
            return false;
        m_coreContext->getDynamicObjectSystemCore()->doRegister(inName, inProperties, sizeof(QDemonRenderEffect), QDemonGraphObjectTypes::Effect);
        auto theClass = m_coreContext->getDynamicObjectSystemCore()->getDynamicObjectClass(inName);
        QDemonRef<QDemonEffectClass> theEffect(new QDemonEffectClass(*theClass));
        m_effectClasses.insert(inName, theEffect);
        return true;
    }

    bool unregisterEffect(QString inName) override
    {
        if (!isEffectRegistered(inName))
            return false;

        m_coreContext->getDynamicObjectSystemCore()->unregister(inName);

        TEffectClassMap::iterator iter = m_effectClasses.find(inName);
        if (iter != m_effectClasses.end())
            m_effectClasses.erase(iter);

        for (quint32 idx = 0, end = m_contexts.size(); idx < end; ++idx) {
            if (m_contexts[idx]->m_className == inName)
                releaseEffectContext(m_contexts[idx].data());
        }
        return true;
    }

    QDemonConstDataRef<QString> getEffectPropertyEnumNames(QString inName, QString inPropName) const override
    {
        const auto theClass = getEffectClass(inName);
        if (theClass == nullptr) {
            Q_ASSERT(false);
            QDemonConstDataRef<QString>();
        }
        const QDemonPropertyDefinition *theDefinitionPtr = theClass->dynamicClass->findPropertyByName(inPropName);
        if (theDefinitionPtr)
            return theDefinitionPtr->enumValueNames;
        return QDemonConstDataRef<QString>();
    }

    QDemonConstDataRef<QDemonPropertyDefinition> getEffectProperties(QString inEffectName) const override
    {
        const auto theClass = getEffectClass(inEffectName);
        if (theClass)
            return theClass->dynamicClass->getProperties();
        return QDemonConstDataRef<QDemonPropertyDefinition>();
    }

    void setEffectPropertyTextureSettings(QString inName,
                                          QString inPropName,
                                          QString inPropPath,
                                          QDemonRenderTextureTypeValue::Enum inTexType,
                                          QDemonRenderTextureCoordOp::Enum inCoordOp,
                                          QDemonRenderTextureMagnifyingOp::Enum inMagFilterOp,
                                          QDemonRenderTextureMinifyingOp::Enum inMinFilterOp) override
    {
        m_coreContext->getDynamicObjectSystemCore()
                ->setPropertyTextureSettings(inName, inPropName, inPropPath, inTexType, inCoordOp, inMagFilterOp, inMinFilterOp);
    }

    void setEffectRequiresDepthTexture(QString inEffectName, bool inValue) override
    {
        auto theClass = getEffectClass(inEffectName);
        if (theClass == nullptr) {
            Q_ASSERT(false);
            return;
        }
        theClass->dynamicClass->setRequiresDepthTexture(inValue);
    }

    bool doesEffectRequireDepthTexture(QString inEffectName) const override
    {
        const auto theClass = getEffectClass(inEffectName);
        if (theClass == nullptr) {
            Q_ASSERT(false);
            return false;
        }
        return theClass->dynamicClass->requiresDepthTexture();
    }

    void setEffectRequiresCompilation(QString inEffectName, bool inValue) override
    {
        auto theClass = getEffectClass(inEffectName);
        if (theClass == nullptr) {
            Q_ASSERT(false);
            return;
        }
        theClass->dynamicClass->setRequiresCompilation(inValue);
    }

    bool doesEffectRequireCompilation(QString inEffectName) const override
    {
        const auto theClass = getEffectClass(inEffectName);
        if (theClass == nullptr) {
            Q_ASSERT(false);
            return false;
        }
        return theClass->dynamicClass->requiresCompilation();
    }

    void setEffectCommands(QString inEffectName, QDemonConstDataRef<dynamic::QDemonCommand *> inCommands) override
    {
        m_coreContext->getDynamicObjectSystemCore()->setRenderCommands(inEffectName, inCommands);
    }

    QDemonConstDataRef<dynamic::QDemonCommand *> getEffectCommands(QString inEffectName) const override
    {
        return m_coreContext->getDynamicObjectSystemCore()->getRenderCommands(inEffectName);
    }

    QDemonRenderEffect *createEffectInstance(QString inEffectName) override
    {
        auto theClass = getEffectClass(inEffectName);
        if (theClass == nullptr)
            return nullptr;
        //        StaticAssert<(sizeof(SEffect) % 4 == 0)>::valid_expression();

        QDemonRenderEffect *theEffect = static_cast<QDemonRenderEffect *>(
                m_coreContext->getDynamicObjectSystemCore()->createInstance(inEffectName));
        theEffect->initialize();
        return theEffect;
    }

    void allocateBuffer(QDemonRenderEffect &inEffect,
                        const QDemonAllocateBuffer &inCommand,
                        quint32 inFinalWidth,
                        quint32 inFinalHeight,
                        QDemonRenderTextureFormats::Enum inSourceTextureFormat)
    {
        // Check to see if it is already allocated and if it is, is it the correct size. If both of
        // these assumptions hold, then we are good.
        QDemonRef<QDemonRenderTexture2D> theBufferTexture;
        const qint32 theWidth = QDemonTextRendererInterface::nextMultipleOf4((quint32)(inFinalWidth * inCommand.m_sizeMultiplier));
        const qint32 theHeight = QDemonTextRendererInterface::nextMultipleOf4((quint32)(inFinalHeight * inCommand.m_sizeMultiplier));
        QDemonRenderTextureFormats::Enum resultFormat = inCommand.m_format;
        if (resultFormat == QDemonRenderTextureFormats::Unknown)
            resultFormat = inSourceTextureFormat;

        if (inEffect.m_context) {
            QDemonEffectContext &theContext(*inEffect.m_context);
            // size intentionally requiried every loop;
            qint32 bufferIdx = theContext.findBuffer(inCommand.m_name);
            if (bufferIdx < theContext.m_allocatedBuffers.size()) {
                QDemonAllocatedBufferEntry &theEntry(theContext.m_allocatedBuffers[bufferIdx]);
                QDemonTextureDetails theDetails = theEntry.texture->getTextureDetails();
                if (theDetails.width == theWidth && theDetails.height == theHeight && theDetails.format == resultFormat) {
                    theBufferTexture = theEntry.texture;
                } else {
                    theContext.releaseBuffer(bufferIdx);
                }
            }
        }
        if (theBufferTexture == nullptr) {
            QDemonEffectContext &theContext(getEffectContext(inEffect));
            auto theFB(m_resourceManager->allocateFrameBuffer());
            auto theTexture(m_resourceManager->allocateTexture2D(theWidth, theHeight, resultFormat));
            theTexture->setMagFilter(inCommand.m_filterOp);
            theTexture->setMinFilter(static_cast<QDemonRenderTextureMinifyingOp::Enum>(inCommand.m_filterOp));
            theTexture->setTextureWrapS(inCommand.m_texCoordOp);
            theTexture->setTextureWrapT(inCommand.m_texCoordOp);
            theFB->attach(QDemonRenderFrameBufferAttachment::Color0, theTexture);
            theContext.m_allocatedBuffers.push_back(
                    QDemonAllocatedBufferEntry(inCommand.m_name, *theFB, *theTexture, inCommand.m_bufferFlags));
            theBufferTexture = theTexture;
        }
    }

    void allocateImage(QDemonRenderEffect &inEffect, const QDemonAllocateImage &inCommand, quint32 inFinalWidth, quint32 inFinalHeight)
    {
        QDemonRef<QDemonRenderImage2D> theImage;
        qint32 theWidth = QDemonTextRendererInterface::nextMultipleOf4((quint32)(inFinalWidth * inCommand.m_sizeMultiplier));
        qint32 theHeight = QDemonTextRendererInterface::nextMultipleOf4((quint32)(inFinalHeight * inCommand.m_sizeMultiplier));

        Q_ASSERT(inCommand.m_format != QDemonRenderTextureFormats::Unknown);

        if (inEffect.m_context) {
            QDemonEffectContext &theContext(*inEffect.m_context);
            // size intentionally requiried every loop;
            qint32 imageIdx = theContext.findImage(inCommand.m_name);
            if (imageIdx < theContext.m_allocatedImages.size()) {
                QDemonAllocatedImageEntry &theEntry(theContext.m_allocatedImages[imageIdx]);
                QDemonTextureDetails theDetails = theEntry.texture->getTextureDetails();
                if (theDetails.width == theWidth && theDetails.height == theHeight && theDetails.format == inCommand.m_format) {
                    theImage = theEntry.image;
                } else {
                    theContext.releaseImage(imageIdx);
                }
            }
        }

        if (theImage == nullptr) {
            QDemonEffectContext &theContext(getEffectContext(inEffect));
            // allocate an immutable texture
            auto theTexture(m_resourceManager->allocateTexture2D(theWidth, theHeight, inCommand.m_format, 1, true));
            theTexture->setMagFilter(inCommand.m_filterOp);
            theTexture->setMinFilter(static_cast<QDemonRenderTextureMinifyingOp::Enum>(inCommand.m_filterOp));
            theTexture->setTextureWrapS(inCommand.m_texCoordOp);
            theTexture->setTextureWrapT(inCommand.m_texCoordOp);
            auto theImage = (m_resourceManager->allocateImage2D(theTexture, inCommand.m_access));
            theContext.m_allocatedImages.push_back(
                    QDemonAllocatedImageEntry(inCommand.m_name, *theImage, *theTexture, inCommand.m_bufferFlags));
        }
    }

    void allocateDataBuffer(QDemonRenderEffect &inEffect, const QDemonAllocateDataBuffer &inCommand)
    {
        quint32 theBufferSize = (quint32)inCommand.m_size;
        Q_ASSERT(theBufferSize);
        QDemonRef<QDemonRenderDataBuffer> theDataBuffer;
        QDemonRef<QDemonRenderDataBuffer> theDataWrapBuffer;

        if (inEffect.m_context) {
            QDemonEffectContext &theContext(*inEffect.m_context);
            // size intentionally requiried every loop;
            qint32 bufferIdx = theContext.findDataBuffer(inCommand.m_name);
            if (bufferIdx < theContext.m_allocatedDataBuffers.size()) {
                QDemonAllocatedDataBufferEntry &theEntry(theContext.m_allocatedDataBuffers[bufferIdx]);
                if (theEntry.bufferType == inCommand.m_dataBufferType && theEntry.bufferData.size() == theBufferSize) {
                    theDataBuffer = theEntry.dataBuffer;
                } else {
                    // if type and size don't match something is wrong
                    Q_ASSERT(false);
                }
            }
        }

        if (theDataBuffer == nullptr) {
            QDemonEffectContext &theContext(getEffectContext(inEffect));
            auto theRenderContext(m_context->getRenderContext());
            quint8 *initialData = (quint8 *)::malloc(theBufferSize);
            QDemonDataRef<quint8> data((quint8 *)initialData, theBufferSize);
            memset(initialData, 0x0L, theBufferSize);
            if (inCommand.m_dataBufferType == QDemonRenderBufferBindValues::Storage) {
                theDataBuffer = theRenderContext->createStorageBuffer(inCommand.m_name.toLocal8Bit(),
                                                                      QDemonRenderBufferUsageType::Dynamic,
                                                                      theBufferSize,
                                                                      data,
                                                                      nullptr);
            } else if (inCommand.m_dataBufferType == QDemonRenderBufferBindValues::Draw_Indirect) {
                Q_ASSERT(theBufferSize == sizeof(DrawArraysIndirectCommand));
                // init a draw call
                quint32 *pIndirectDrawCall = (quint32 *)initialData;
                // vertex count we draw points right now only
                // the rest we fill in by GPU
                pIndirectDrawCall[0] = 1;
                theDataBuffer = theRenderContext->createDrawIndirectBuffer(QDemonRenderBufferUsageType::Dynamic, theBufferSize, data);
            } else
                Q_ASSERT(false);

            theContext.m_allocatedDataBuffers.push_back(QDemonAllocatedDataBufferEntry(inCommand.m_name,
                                                                                       *theDataBuffer,
                                                                                       inCommand.m_dataBufferType,
                                                                                       data,
                                                                                       inCommand.m_bufferFlags));

            // create wrapper buffer
            if (inCommand.m_dataBufferWrapType == QDemonRenderBufferBindValues::Storage
                && !inCommand.m_wrapName.isEmpty() && theDataBuffer) {
                theDataWrapBuffer = theRenderContext->createStorageBuffer(inCommand.m_wrapName.toLocal8Bit(),
                                                                          QDemonRenderBufferUsageType::Dynamic,
                                                                          theBufferSize,
                                                                          data,
                                                                          theDataBuffer.data());
                theContext.m_allocatedDataBuffers.push_back(QDemonAllocatedDataBufferEntry(inCommand.m_wrapName,
                                                                                           *theDataWrapBuffer,
                                                                                           inCommand.m_dataBufferWrapType,
                                                                                           QDemonDataRef<quint8>(),
                                                                                           inCommand.m_bufferFlags));
            }
        }
    }

    QDemonRef<QDemonRenderTexture2D> findTexture(QDemonRenderEffect *inEffect, const QString &inName)
    {
        if (inEffect->m_context) {
            QDemonEffectContext &theContext(*inEffect->m_context);
            qint32 bufferIdx = theContext.findBuffer(inName);
            if (bufferIdx < theContext.m_allocatedBuffers.size()) {
                return theContext.m_allocatedBuffers[bufferIdx].texture;
            }
        }
        Q_ASSERT(false);
        return nullptr;
    }

    QDemonRef<QDemonRenderFrameBuffer> bindBuffer(QDemonRenderEffect &inEffect,
                                                  const QDemonBindBuffer &inCommand,
                                                  QMatrix4x4 &outMVP,
                                                  QVector2D &outDestSize)
    {
        QDemonRef<QDemonRenderFrameBuffer> theBuffer;
        QDemonRef<QDemonRenderTexture2D> theTexture;
        if (inEffect.m_context) {
            QDemonEffectContext &theContext(*inEffect.m_context);
            qint32 bufferIdx = theContext.findBuffer(inCommand.m_bufferName);
            if (bufferIdx < theContext.m_allocatedBuffers.size()) {
                theBuffer = theContext.m_allocatedBuffers[bufferIdx].frameBuffer;
                theTexture = theContext.m_allocatedBuffers[bufferIdx].texture;
                theContext.m_allocatedBuffers[bufferIdx].needsClear = false;
            }
        }
        if (theBuffer == nullptr) {
            qCCritical(INVALID_OPERATION,
                       "Effect %s: Failed to find buffer %s for bind",
                       inEffect.className.toLatin1().constData(),
                       inCommand.m_bufferName.toLatin1().constData());
            QString errorMsg = QObject::tr("Failed to compile \"%1\" effect.\nConsider"
                                           " removing it from the presentation.")
                                       .arg(qPrintable(inEffect.className));
            // TODO:
            //            QDEMON_ALWAYS_ASSERT_MESSAGE(errorMsg.toUtf8());
            outMVP = QMatrix4x4();
            return nullptr;
        }

        if (theTexture) {
            QDemonRenderCamera::setupOrthographicCameraForOffscreenRender(*theTexture, outMVP);
            QDemonTextureDetails theDetails(theTexture->getTextureDetails());
            m_context->getRenderContext()->setViewport(QRect(0, 0, (quint32)theDetails.width, (quint32)theDetails.height));
            outDestSize = QVector2D((float)theDetails.width, (float)theDetails.height);
        }

        return theBuffer;
    }

    QDemonRef<QDemonEffectShader> bindShader(QString &inEffectId, const QDemonBindShader &inCommand)
    {
        auto theClass = getEffectClass(inEffectId);
        if (!theClass) {
            Q_ASSERT(false);
            return nullptr;
        }

        bool forceCompilation = theClass->dynamicClass->requiresCompilation();

        auto key = TStrStrPair(inCommand.m_shaderPath, inCommand.m_shaderDefine);
        auto theInsertResult = m_shaderMap.find(key);
        const bool found = (theInsertResult != m_shaderMap.end());
        if (!found)
            theInsertResult = m_shaderMap.insert(key, QDemonRef<QDemonEffectShader>());

        if (found || forceCompilation) {
            auto theProgram = m_context->getDynamicObjectSystem()
                                      ->getShaderProgram(inCommand.m_shaderPath,
                                                         inCommand.m_shaderDefine,
                                                         TShaderFeatureSet(),
                                                         QDemonDynamicShaderProgramFlags(),
                                                         forceCompilation)
                                      .first;
            if (theProgram)
                theInsertResult.value() = QDemonRef<QDemonEffectShader>(new QDemonEffectShader(theProgram));
        }
        if (theInsertResult.value()) {
            auto theContext(m_context->getRenderContext());
            theContext->setActiveShader(theInsertResult.value()->m_shader);
        }

        return theInsertResult.value();
    }

    void doApplyInstanceValue(QDemonRenderEffect *inEffect,
                              quint8 *inDataPtr,
                              const QString &inPropertyName,
                              QDemonRenderShaderDataTypes::Enum inPropertyType,
                              const QDemonRef<QDemonRenderShaderProgram> &inShader,
                              const QDemonPropertyDefinition &inDefinition)
    {
        auto theConstant = inShader->getShaderConstant(inPropertyName.toLocal8Bit());
        if (theConstant) {
            if (theConstant->getShaderConstantType() == inPropertyType) {
                if (inPropertyType == QDemonRenderShaderDataTypes::Texture2D) {
                    // TODO:
                    //                    StaticAssert<sizeof(QString) == sizeof(QDemonRenderTexture2DPtr)>::valid_expression();
                    QString theStrPtr = QString::fromLatin1(reinterpret_cast<char *>(inDataPtr));
                    auto theBufferManager(m_context->getBufferManager());
                    auto theOffscreenRenderer = m_context->getOffscreenRenderManager();
                    bool needsAlphaMultiply = true;
                    QDemonRef<QDemonRenderTexture2D> theTexture;
                    if (!theStrPtr.isEmpty()) {
                        if (theOffscreenRenderer->hasOffscreenRenderer(theStrPtr)) {
                            QDemonOffscreenRenderResult theResult = theOffscreenRenderer->getRenderedItem(theStrPtr);
                            needsAlphaMultiply = false;
                            theTexture = theResult.texture;
                        } else {
                            QDemonRenderImageTextureData theTextureData = theBufferManager.loadRenderImage(theStrPtr);
                            needsAlphaMultiply = true;
                            theTexture = theTextureData.m_texture;
                        }
                    }
                    getEffectContext(*inEffect).setTexture(inShader,
                                                           inPropertyName,
                                                           theTexture,
                                                           needsAlphaMultiply,
                                                           m_textureStringBuilder,
                                                           m_textureStringBuilder2,
                                                           &inDefinition);
                } else if (inPropertyType == QDemonRenderShaderDataTypes::Image2D) {
                    // TODO:
                    //                    StaticAssert<sizeof(QString)
                    //                            == sizeof(QDemonRenderTexture2DPtr)>::valid_expression();
                    QDemonRef<QDemonRenderImage2D> theImage;
                    getEffectContext(*inEffect).setImage(inShader, inPropertyName, theImage);
                } else if (inPropertyType == QDemonRenderShaderDataTypes::DataBuffer) {
                    // we don't handle this here
                } else {
                    switch (inPropertyType) {
                    case QDemonRenderShaderDataTypes::Integer:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<qint32 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::IntegerVec2:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<qint32_2 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::IntegerVec3:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<qint32_3 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::IntegerVec4:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<qint32_4 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Boolean:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<bool *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::BooleanVec2:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<bool_2 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::BooleanVec3:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<bool_3 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::BooleanVec4:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<bool_4 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Float:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<float *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Vec2:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QVector2D *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Vec3:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QVector3D *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Vec4:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QVector4D *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::UnsignedInteger:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<quint32 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::UnsignedIntegerVec2:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<quint32_2 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::UnsignedIntegerVec3:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<quint32_3 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::UnsignedIntegerVec4:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<quint32_4 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Matrix3x3:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QMatrix3x3 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Matrix4x4:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QMatrix4x4 *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Texture2D:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderTexture2DPtr *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Texture2DHandle:
                        inShader->setPropertyValue(theConstant.data(),
                                                   *(reinterpret_cast<QDemonRenderTexture2DHandle *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Texture2DArray:
                        inShader->setPropertyValue(theConstant.data(),
                                                   *(reinterpret_cast<QDemonRenderTexture2DArrayPtr *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::TextureCube:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderTextureCubePtr *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::TextureCubeHandle:
                        inShader->setPropertyValue(theConstant.data(),
                                                   *(reinterpret_cast<QDemonRenderTextureCubeHandle *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::Image2D:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderImage2DPtr *>(inDataPtr)));
                        break;
                    case QDemonRenderShaderDataTypes::DataBuffer:
                        inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderDataBufferPtr *>(inDataPtr)));
                        break;
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

    void applyInstanceValue(QDemonRenderEffect *inEffect,
                            const QDemonRef<QDemonEffectClass> &inClass,
                            const QDemonRef<QDemonRenderShaderProgram> &inShader,
                            const QDemonApplyInstanceValue &inCommand)
    {
        // sanity check
        if (!inCommand.m_propertyName.isEmpty()) {
            bool canGetData = inCommand.m_valueOffset + getSizeofShaderDataType(inCommand.m_valueType) <= inEffect->dataSectionByteSize;
            if (canGetData == false) {
                Q_ASSERT(false);
                return;
            }
            quint8 *dataPtr = inEffect->getDataSectionBegin() + inCommand.m_valueOffset;
            const QDemonPropertyDefinition *theDefinition = inClass->dynamicClass->findPropertyByName(inCommand.m_propertyName);
            if (theDefinition)
                doApplyInstanceValue(inEffect, dataPtr, inCommand.m_propertyName, inCommand.m_valueType, inShader, *theDefinition);
        } else {
            QDemonConstDataRef<QDemonPropertyDefinition> theDefs = inClass->dynamicClass->getProperties();
            for (quint32 idx = 0, end = theDefs.size(); idx < end; ++idx) {
                const QDemonPropertyDefinition &theDefinition(theDefs[idx]);
                auto theConstant = inShader->getShaderConstant(theDefinition.name.toLatin1());

                // This is fine, the property wasn't found and we continue, no problem.
                if (!theConstant)
                    continue;
                quint8 *dataPtr = inEffect->getDataSectionBegin() + theDefinition.offset;
                doApplyInstanceValue(inEffect, dataPtr, theDefinition.name, theDefinition.dataType, inShader, theDefinition);
            }
        }
    }

    void applyValue(QDemonRenderEffect *inEffect,
                    const QDemonRef<QDemonEffectClass> &inClass,
                    const QDemonRef<QDemonRenderShaderProgram> &inShader,
                    const QDemonApplyValue &inCommand)
    {
        if (!inCommand.m_propertyName.isEmpty()) {
            quint8 *dataPtr = inCommand.m_value.mData;
            const QDemonPropertyDefinition *theDefinition = inClass->dynamicClass->findPropertyByName(inCommand.m_propertyName);
            if (theDefinition)
                doApplyInstanceValue(inEffect, dataPtr, inCommand.m_propertyName, inCommand.m_valueType, inShader, *theDefinition);
        }
    }

    bool applyBlending(const QDemonApplyBlending &inCommand)
    {
        auto theContext(m_context->getRenderContext());

        theContext->setBlendingEnabled(true);

        QDemonRenderBlendFunctionArgument blendFunc = QDemonRenderBlendFunctionArgument(inCommand.m_srcBlendFunc,
                                                                                        inCommand.m_dstBlendFunc,
                                                                                        inCommand.m_srcBlendFunc,
                                                                                        inCommand.m_dstBlendFunc);

        QDemonRenderBlendEquationArgument blendEqu(QDemonRenderBlendEquation::Add, QDemonRenderBlendEquation::Add);

        theContext->setBlendFunction(blendFunc);
        theContext->setBlendEquation(blendEqu);

        return true;
    }

    // This has the potential to change the source texture for the current render pass
    QDemonEffectTextureData applyBufferValue(QDemonRenderEffect *inEffect,
                                             const QDemonRef<QDemonRenderShaderProgram> &inShader,
                                             const QDemonApplyBufferValue &inCommand,
                                             const QDemonRef<QDemonRenderTexture2D> &inSourceTexture,
                                             const QDemonEffectTextureData &inCurrentSourceTexture)
    {
        QDemonEffectTextureData theTextureToBind;
        if (!inCommand.m_bufferName.isEmpty()) {
            if (inEffect->m_context) {
                QDemonEffectContext &theContext(*inEffect->m_context);
                qint32 bufferIdx = theContext.findBuffer(inCommand.m_bufferName);
                if (bufferIdx < theContext.m_allocatedBuffers.size()) {
                    QDemonAllocatedBufferEntry &theEntry(theContext.m_allocatedBuffers[bufferIdx]);
                    if (theEntry.needsClear) {
                        auto theRenderContext(m_context->getRenderContext());

                        theRenderContext->setRenderTarget(theEntry.frameBuffer);
                        // Note that depth/stencil buffers need an explicit clear in their bind
                        // commands in order to ensure
                        // we clear the least amount of information possible.

                        if (theEntry.texture) {
                            QDemonRenderTextureFormats::Enum theTextureFormat = theEntry.texture->getTextureDetails().format;
                            if (theTextureFormat != QDemonRenderTextureFormats::Depth16
                                && theTextureFormat != QDemonRenderTextureFormats::Depth24
                                && theTextureFormat != QDemonRenderTextureFormats::Depth32
                                && theTextureFormat != QDemonRenderTextureFormats::Depth24Stencil8) {
                                QDemonRenderContextScopedProperty<QVector4D> __clearColor(*theRenderContext,
                                                                                          &QDemonRenderContext::getClearColor,
                                                                                          &QDemonRenderContext::setClearColor,
                                                                                          QVector4D());
                                theRenderContext->clear(QDemonRenderClearValues::Color);
                            }
                        }
                        theEntry.needsClear = false;
                    }
                    theTextureToBind = QDemonEffectTextureData(theEntry.texture, false);
                }
            }
            if (theTextureToBind.texture == nullptr) {
                Q_ASSERT(false);
                qCCritical(INVALID_OPERATION,
                           "Effect %s: Failed to find buffer %s for bind",
                           qPrintable(inEffect->className),
                           qPrintable(inCommand.m_bufferName));
                Q_ASSERT(false);
            }
        } else { // no name means bind the source
            theTextureToBind = QDemonEffectTextureData(inSourceTexture, false);
        }

        if (!inCommand.m_paramName.isEmpty()) {
            auto theConstant = inShader->getShaderConstant(inCommand.m_paramName.toLatin1());

            if (theConstant) {
                if (theConstant->getShaderConstantType() != QDemonRenderShaderDataTypes::Texture2D) {
                    qCCritical(INVALID_OPERATION,
                               "Effect %s: Binding buffer to parameter %s that is not a texture",
                               qPrintable(inEffect->className),
                               qPrintable(inCommand.m_paramName));
                    Q_ASSERT(false);
                } else {
                    getEffectContext(*inEffect).setTexture(inShader,
                                                           inCommand.m_paramName,
                                                           theTextureToBind.texture,
                                                           theTextureToBind.needsAlphaMultiply,
                                                           m_textureStringBuilder,
                                                           m_textureStringBuilder2);
                }
            }
            return inCurrentSourceTexture;
        } else {
            return theTextureToBind;
        }
    }

    void applyDepthValue(QDemonRenderEffect *inEffect,
                         const QDemonRef<QDemonRenderShaderProgram> &inShader,
                         const QDemonApplyDepthValue &inCommand,
                         const QDemonRef<QDemonRenderTexture2D> &inTexture)
    {
        auto theConstant = inShader->getShaderConstant(inCommand.m_paramName.toLatin1());

        if (theConstant) {
            if (theConstant->getShaderConstantType() != QDemonRenderShaderDataTypes::Texture2D) {
                qCCritical(INVALID_OPERATION,
                           "Effect %s: Binding buffer to parameter %s that is not a texture",
                           qPrintable(inEffect->className),
                           qPrintable(inCommand.m_paramName));
                Q_ASSERT(false);
            } else {
                getEffectContext(*inEffect).setTexture(inShader, inCommand.m_paramName, inTexture, false, m_textureStringBuilder, m_textureStringBuilder2);
            }
        }
    }

    void applyImageValue(QDemonRenderEffect *inEffect, const QDemonRef<QDemonRenderShaderProgram> &inShader, const QDemonApplyImageValue &inCommand)
    {
        QDemonAllocatedImageEntry theImageToBind;
        if (!inCommand.m_imageName.isEmpty()) {
            if (inEffect->m_context) {
                QDemonEffectContext &theContext(*inEffect->m_context);
                qint32 bufferIdx = theContext.findImage(inCommand.m_imageName);
                if (bufferIdx < theContext.m_allocatedImages.size()) {
                    theImageToBind = QDemonAllocatedImageEntry(theContext.m_allocatedImages[bufferIdx]);
                }
            }
        }

        if (theImageToBind.image == nullptr) {
            qCCritical(INVALID_OPERATION,
                       "Effect %s: Failed to find image %s for bind",
                       qPrintable(inEffect->className),
                       qPrintable(inCommand.m_imageName));
            Q_ASSERT(false);
        }

        if (!inCommand.m_paramName.isEmpty()) {
            auto theConstant = inShader->getShaderConstant(inCommand.m_paramName.toLatin1());

            if (theConstant) {
                if (inCommand.m_needSync) {
                    QDemonRenderBufferBarrierFlags flags(QDemonRenderBufferBarrierValues::TextureFetch
                                                         | QDemonRenderBufferBarrierValues::TextureUpdate);
                    inShader->getRenderContext()->setMemoryBarrier(flags);
                }

                if (theConstant->getShaderConstantType() == QDemonRenderShaderDataTypes::Image2D && !inCommand.m_bindAsTexture) {
                    getEffectContext(*inEffect).setImage(inShader, inCommand.m_paramName, theImageToBind.image);
                } else if (theConstant->getShaderConstantType() == QDemonRenderShaderDataTypes::Texture2D && inCommand.m_bindAsTexture) {
                    getEffectContext(*inEffect).setTexture(inShader, inCommand.m_paramName, theImageToBind.texture, false, m_textureStringBuilder, m_textureStringBuilder2);
                } else {
                    qCCritical(INVALID_OPERATION,
                               "Effect %s: Binding buffer to parameter %s that is not a texture",
                               qPrintable(inEffect->className),
                               qPrintable(inCommand.m_paramName));
                    Q_ASSERT(false);
                }
            }
        }
    }

    void applyDataBufferValue(QDemonRenderEffect *inEffect,
                              const QDemonRef<QDemonRenderShaderProgram> &inShader,
                              const QDemonApplyDataBufferValue &inCommand)
    {
        QDemonAllocatedDataBufferEntry theBufferToBind;
        if (!inCommand.m_paramName.isEmpty()) {
            if (inEffect->m_context) {
                QDemonEffectContext &theContext(*inEffect->m_context);
                qint32 bufferIdx = theContext.findDataBuffer(inCommand.m_paramName);
                if (bufferIdx < theContext.m_allocatedDataBuffers.size()) {
                    theBufferToBind = QDemonAllocatedDataBufferEntry(theContext.m_allocatedDataBuffers[bufferIdx]);
                    if (theBufferToBind.needsClear) {
                        QDemonDataRef<quint8> pData = theBufferToBind.dataBuffer->mapBuffer();
                        memset(pData.begin(), 0x0L, theBufferToBind.bufferData.size());
                        theBufferToBind.dataBuffer->unmapBuffer();
                        theBufferToBind.needsClear = false;
                    }
                }
            }

            if (theBufferToBind.dataBuffer == nullptr) {
                qCCritical(INVALID_OPERATION,
                           "Effect %s: Failed to find buffer %s for bind",
                           qPrintable(inEffect->className),
                           qPrintable(inCommand.m_paramName));
                Q_ASSERT(false);
            }

            auto theConstant = inShader->getShaderBuffer(inCommand.m_paramName.toLatin1());

            if (theConstant) {
                getEffectContext(*inEffect).setDataBuffer(inShader, inCommand.m_paramName, theBufferToBind.dataBuffer);
            } else if (theBufferToBind.bufferType == QDemonRenderBufferBindValues::Draw_Indirect) {
                // since we filled part of this buffer on the GPU we need a sync before usage
                QDemonRenderBufferBarrierFlags flags(QDemonRenderBufferBarrierValues::CommandBuffer);
                inShader->getRenderContext()->setMemoryBarrier(flags);
            }
        }
    }

    void applyRenderStateValue(QDemonRenderFrameBuffer *inTarget,
                               const QDemonRef<QDemonRenderTexture2D> &inDepthStencilTexture,
                               const QDemonApplyRenderState &theCommand)
    {
        auto theContext(m_context->getRenderContext());
        bool inEnable = theCommand.m_enabled;

        switch (theCommand.m_renderState) {
        case QDemonRenderState::StencilTest: {
            if (inEnable && inTarget) {
                inTarget->attach(QDemonRenderFrameBufferAttachment::DepthStencil, inDepthStencilTexture);
            } else if (inTarget) {
                inTarget->attach(QDemonRenderFrameBufferAttachment::DepthStencil, QDemonRenderTextureOrRenderBuffer());
            }

            theContext->setStencilTestEnabled(inEnable);
        } break;
        default:
            Q_ASSERT(false);
            break;
        }
    }

    static bool compareDepthStencilState(QDemonRenderDepthStencilState &inState, QDemonDepthStencil &inStencil)
    {
        QDemonRenderStencilFunctionArgument theFunction = inState.getStencilFunc(QDemonRenderFace::Front);
        QDemonRenderStencilOperationArgument theOperation = inState.getStencilOp(QDemonRenderFace::Front);

        return theFunction.m_function == inStencil.m_stencilFunction && theFunction.m_mask == inStencil.m_mask
                && theFunction.m_referenceValue == inStencil.m_reference && theOperation.m_stencilFail == inStencil.m_stencilFailOperation
                && theOperation.m_depthFail == inStencil.m_depthFailOperation
                && theOperation.m_depthPass == inStencil.m_depthPassOperation;
    }

    void renderPass(QDemonEffectShader &inShader,
                    const QMatrix4x4 &inMVP,
                    const QDemonEffectTextureData &inSourceTexture,
                    const QDemonRef<QDemonRenderFrameBuffer> &inFrameBuffer,
                    QVector2D &inDestSize,
                    const QVector2D &inCameraClipRange,
                    const QDemonRef<QDemonRenderTexture2D> &inDepthStencil,
                    QDemonOption<QDemonDepthStencil> inDepthStencilCommand,
                    bool drawIndirect)
    {
        auto theContext(m_context->getRenderContext());
        theContext->setRenderTarget(inFrameBuffer);
        if (inDepthStencil && inFrameBuffer) {
            inFrameBuffer->attach(QDemonRenderFrameBufferAttachment::DepthStencil, inDepthStencil);
            if (inDepthStencilCommand.hasValue()) {
                QDemonDepthStencil &theDepthStencil(*inDepthStencilCommand);
                QDemonRenderClearFlags clearFlags;
                if (theDepthStencil.m_glags.hasClearStencil())
                    clearFlags |= QDemonRenderClearValues::Stencil;
                if (theDepthStencil.m_glags.hasClearDepth())
                    clearFlags |= QDemonRenderClearValues::Depth;

                if (clearFlags)
                    theContext->clear(clearFlags);

                QDemonRef<QDemonRenderDepthStencilState> targetState;
                for (quint32 idx = 0, end = m_depthStencilStates.size(); idx < end && targetState == nullptr; ++idx) {
                    QDemonRef<QDemonRenderDepthStencilState> theState = m_depthStencilStates[idx];
                    if (compareDepthStencilState(*theState, theDepthStencil))
                        targetState = theState;
                }
                if (targetState == nullptr) {
                    QDemonRenderStencilFunctionArgument theFunctionArg(theDepthStencil.m_stencilFunction,
                                                                       theDepthStencil.m_reference,
                                                                       theDepthStencil.m_mask);
                    QDemonRenderStencilOperationArgument theOpArg(theDepthStencil.m_stencilFailOperation,
                                                                  theDepthStencil.m_depthFailOperation,
                                                                  theDepthStencil.m_depthPassOperation);
                    targetState = theContext->createDepthStencilState(theContext->isDepthTestEnabled(),
                                                                      theContext->isDepthWriteEnabled(),
                                                                      theContext->getDepthFunction(),
                                                                      true,
                                                                      theFunctionArg,
                                                                      theFunctionArg,
                                                                      theOpArg,
                                                                      theOpArg);
                    m_depthStencilStates.push_back(targetState);
                }
                theContext->setDepthStencilState(targetState);
            }
        }

        theContext->setActiveShader(inShader.m_shader);
        inShader.m_mvp.set(inMVP);
        if (inSourceTexture.texture) {
            inShader.m_textureEntry.set(inSourceTexture.texture, inSourceTexture.needsAlphaMultiply, nullptr);
        } else {
            qCCritical(INTERNAL_ERROR, "Failed to setup pass due to null source texture");
            Q_ASSERT(false);
        }
        inShader.m_fragColorAlphaSettings.set(QVector2D(1.0f, 0.0f));
        inShader.m_destSize.set(inDestSize);
        if (inShader.m_appFrame.isValid())
            inShader.m_appFrame.set((float)m_context->getFrameCount());
        if (inShader.m_fps.isValid())
            inShader.m_fps.set((float)m_context->getFPS().first);
        if (inShader.m_cameraClipRange.isValid())
            inShader.m_cameraClipRange.set(inCameraClipRange);

        if (!drawIndirect)
            m_context->getRenderer()->renderQuad();
        else
            m_context->getRenderer()->renderPointsIndirect();

        if (inDepthStencil && inFrameBuffer) {
            inFrameBuffer->attach(QDemonRenderFrameBufferAttachment::DepthStencil, QDemonRenderTextureOrRenderBuffer());
            theContext->setDepthStencilState(m_defaultStencilState);
        }
    }

    void doRenderEffect(QDemonRenderEffect *inEffect,
                        const QDemonRef<QDemonEffectClass> &inClass,
                        const QDemonRef<QDemonRenderTexture2D> &inSourceTexture,
                        QMatrix4x4 &inMVP,
                        const QDemonRef<QDemonRenderFrameBuffer> &inTarget,
                        bool inEnableBlendWhenRenderToTarget,
                        const QDemonRef<QDemonRenderTexture2D> &inDepthTexture,
                        const QDemonRef<QDemonRenderTexture2D> &inDepthStencilTexture,
                        const QVector2D &inCameraClipRange)
    {
        // Run through the effect commands and render the effect.
        // QDemonRenderTexture2D* theCurrentTexture(&inSourceTexture);
        auto theContext = m_context->getRenderContext();

        // Context variables that are updated during the course of a pass.
        QDemonEffectTextureData theCurrentSourceTexture(inSourceTexture, false);
        QDemonRef<QDemonRenderTexture2D> theCurrentDepthStencilTexture;
        QDemonRef<QDemonRenderFrameBuffer> theCurrentRenderTarget(inTarget);
        QDemonRef<QDemonEffectShader> theCurrentShader;
        QRect theOriginalViewport(theContext->getViewport());
        bool wasScissorEnabled = theContext->isScissorTestEnabled();
        bool wasBlendingEnabled = theContext->isBlendingEnabled();
        // save current blending setup
        QDemonRenderBlendFunctionArgument theBlendFunc = theContext->getBlendFunction();
        QDemonRenderBlendEquationArgument theBlendEqu = theContext->getBlendEquation();
        bool intermediateBlendingEnabled = false;
        QDemonTextureDetails theDetails(inSourceTexture->getTextureDetails());
        quint32 theFinalWidth = (quint32)(theDetails.width);
        quint32 theFinalHeight = (quint32)(theDetails.height);
        QVector2D theDestSize;
        {
            // Ensure no matter the command run goes we replace the rendering system to some
            // semblance of the approprate
            // setting.
            QDemonRenderContextScopedProperty<QDemonRef<QDemonRenderFrameBuffer>> __framebuffer(*theContext,
                                                                                                &QDemonRenderContext::getRenderTarget,
                                                                                                &QDemonRenderContext::setRenderTarget);
            QDemonRenderContextScopedProperty<QRect> __viewport(*theContext,
                                                                &QDemonRenderContext::getViewport,
                                                                &QDemonRenderContext::setViewport);
            QDemonRenderContextScopedProperty<bool> __scissorEnabled(*theContext,
                                                                     &QDemonRenderContext::isScissorTestEnabled,
                                                                     &QDemonRenderContext::setScissorTestEnabled);
            QDemonRenderContextScopedProperty<bool> __stencilTest(*theContext,
                                                                  &QDemonRenderContext::isStencilTestEnabled,
                                                                  &QDemonRenderContext::setStencilTestEnabled);
            QDemonRenderContextScopedProperty<QDemonRenderBoolOp> __depthFunction(*theContext,
                                                                                        &QDemonRenderContext::getDepthFunction,
                                                                                        &QDemonRenderContext::setDepthFunction);
            QDemonOption<QDemonDepthStencil> theCurrentDepthStencil;

            theContext->setScissorTestEnabled(false);
            theContext->setBlendingEnabled(false);
            theContext->setCullingEnabled(false);
            theContext->setDepthTestEnabled(false);
            theContext->setDepthWriteEnabled(false);

            QMatrix4x4 theMVP;
            QDemonConstDataRef<dynamic::QDemonCommand *> theCommands = inClass->dynamicClass->getRenderCommands();
            for (quint32 commandIdx = 0, commandEnd = theCommands.size(); commandIdx < commandEnd; ++commandIdx) {
                const QDemonCommand &theCommand(*theCommands[commandIdx]);
                switch (theCommand.m_type) {
                case CommandTypes::AllocateBuffer:
                    allocateBuffer(*inEffect,
                                   static_cast<const QDemonAllocateBuffer &>(theCommand),
                                   theFinalWidth,
                                   theFinalHeight,
                                   theDetails.format);
                    break;

                case CommandTypes::AllocateImage:
                    allocateImage(*inEffect, static_cast<const QDemonAllocateImage &>(theCommand), theFinalWidth, theFinalHeight);
                    break;

                case CommandTypes::AllocateDataBuffer:
                    allocateDataBuffer(*inEffect, static_cast<const QDemonAllocateDataBuffer &>(theCommand));
                    break;

                case CommandTypes::BindBuffer:
                    theCurrentRenderTarget = bindBuffer(*inEffect, static_cast<const QDemonBindBuffer &>(theCommand), theMVP, theDestSize);
                    break;

                case CommandTypes::BindTarget: {
                    m_context->getRenderContext()->setRenderTarget(inTarget);
                    theCurrentRenderTarget = inTarget;
                    theMVP = inMVP;
                    theContext->setViewport(theOriginalViewport);
                    theDestSize = QVector2D((float)theFinalWidth, (float)theFinalHeight);
                    // This isn't necessary if we are rendering to an offscreen buffer and not
                    // compositing
                    // with other objects.
                    if (inEnableBlendWhenRenderToTarget) {
                        theContext->setBlendingEnabled(wasBlendingEnabled);
                        theContext->setScissorTestEnabled(wasScissorEnabled);
                        // The blending setup was done before we apply the effect
                        theContext->setBlendFunction(theBlendFunc);
                        theContext->setBlendEquation(theBlendEqu);
                    }
                } break;
                case CommandTypes::BindShader:
                    theCurrentShader = bindShader(inEffect->className, static_cast<const QDemonBindShader &>(theCommand));
                    break;
                case CommandTypes::ApplyInstanceValue:
                    if (theCurrentShader)
                        applyInstanceValue(inEffect,
                                           inClass,
                                           theCurrentShader->m_shader,
                                           static_cast<const QDemonApplyInstanceValue &>(theCommand));
                    break;
                case CommandTypes::ApplyValue:
                    if (theCurrentShader)
                        applyValue(inEffect, inClass, theCurrentShader->m_shader, static_cast<const QDemonApplyValue &>(theCommand));
                    break;
                case CommandTypes::ApplyBlending:
                    intermediateBlendingEnabled = applyBlending(static_cast<const QDemonApplyBlending &>(theCommand));
                    break;
                case CommandTypes::ApplyBufferValue:
                    if (theCurrentShader)
                        theCurrentSourceTexture = applyBufferValue(inEffect,
                                                                   theCurrentShader->m_shader,
                                                                   static_cast<const QDemonApplyBufferValue &>(theCommand),
                                                                   inSourceTexture,
                                                                   theCurrentSourceTexture);
                    break;
                case CommandTypes::ApplyDepthValue:
                    if (theCurrentShader)
                        applyDepthValue(inEffect, theCurrentShader->m_shader, static_cast<const QDemonApplyDepthValue &>(theCommand), inDepthTexture);
                    if (!inDepthTexture) {
                        qCCritical(INVALID_OPERATION,
                                   "Depth value command detected but no "
                                   "depth buffer provided for effect %s",
                                   qPrintable(inEffect->className));
                        Q_ASSERT(false);
                    }
                    break;
                case CommandTypes::ApplyImageValue:
                    if (theCurrentShader)
                        applyImageValue(inEffect, theCurrentShader->m_shader, static_cast<const QDemonApplyImageValue &>(theCommand));
                    break;
                case CommandTypes::ApplyDataBufferValue:
                    if (theCurrentShader)
                        applyDataBufferValue(inEffect,
                                             theCurrentShader->m_shader,
                                             static_cast<const QDemonApplyDataBufferValue &>(theCommand));
                    break;
                case CommandTypes::DepthStencil: {
                    const QDemonDepthStencil &theDepthStencil = static_cast<const QDemonDepthStencil &>(theCommand);
                    theCurrentDepthStencilTexture = findTexture(inEffect, theDepthStencil.m_bufferName);
                    if (theCurrentDepthStencilTexture)
                        theCurrentDepthStencil = theDepthStencil;
                } break;
                case CommandTypes::Render:
                    if (theCurrentShader && theCurrentSourceTexture.texture) {
                        renderPass(*theCurrentShader,
                                   theMVP,
                                   theCurrentSourceTexture,
                                   theCurrentRenderTarget,
                                   theDestSize,
                                   inCameraClipRange,
                                   theCurrentDepthStencilTexture,
                                   theCurrentDepthStencil,
                                   static_cast<const QDemonRender &>(theCommand).m_drawIndirect);
                    }
                    // Reset the source texture regardless
                    theCurrentSourceTexture = QDemonEffectTextureData(inSourceTexture, false);
                    theCurrentDepthStencilTexture = nullptr;
                    theCurrentDepthStencil = QDemonOption<QDemonDepthStencil>();
                    // reset intermediate blending state
                    if (intermediateBlendingEnabled) {
                        theContext->setBlendingEnabled(false);
                        intermediateBlendingEnabled = false;
                    }
                    break;
                case CommandTypes::ApplyRenderState:
                    applyRenderStateValue(theCurrentRenderTarget.data(),
                                          inDepthStencilTexture,
                                          static_cast<const QDemonApplyRenderState &>(theCommand));
                    break;
                default:
                    Q_ASSERT(false);
                    break;
                }
            }

            setEffectRequiresCompilation(inEffect->className, false);

            // reset to default stencil state
            if (inDepthStencilTexture)
                theContext->setDepthStencilState(m_defaultStencilState);

            // Release any per-frame buffers
            if (inEffect->m_context) {
                QDemonEffectContext &theContext(*inEffect->m_context);
                // Query for size on every loop intentional
                for (qint32 idx = 0; idx < theContext.m_allocatedBuffers.size(); ++idx) {
                    if (theContext.m_allocatedBuffers[idx].flags.isSceneLifetime() == false) {
                        theContext.releaseBuffer(idx);
                        --idx;
                    }
                }
                for (qint32 idx = 0; idx < theContext.m_allocatedImages.size(); ++idx) {
                    if (theContext.m_allocatedImages[idx].flags.isSceneLifetime() == false) {
                        theContext.releaseImage(idx);
                        --idx;
                    }
                }
            }
        }
    }

    QDemonRef<QDemonRenderTexture2D> renderEffect(QDemonEffectRenderArgument inRenderArgument) override
    {
        auto theClass = getEffectClass(inRenderArgument.m_effect->className);
        if (!theClass) {
            Q_ASSERT(false);
            return nullptr;
        }
        QMatrix4x4 theMVP;
        QDemonRenderCamera::setupOrthographicCameraForOffscreenRender(*inRenderArgument.m_colorBuffer, theMVP);
        // setup a render target
        auto theContext(m_context->getRenderContext());
        auto theManager(m_context->getResourceManager());
        QDemonRenderContextScopedProperty<QDemonRef<QDemonRenderFrameBuffer>> __framebuffer(*theContext,
                                                                                            &QDemonRenderContext::getRenderTarget,
                                                                                            &QDemonRenderContext::setRenderTarget);
        QDemonTextureDetails theDetails(inRenderArgument.m_colorBuffer->getTextureDetails());
        quint32 theFinalWidth = QDemonTextRendererInterface::nextMultipleOf4((quint32)(theDetails.width));
        quint32 theFinalHeight = QDemonTextRendererInterface::nextMultipleOf4((quint32)(theDetails.height));
        auto theBuffer = theManager->allocateFrameBuffer();
        // UdoL Some Effects may need to run before HDR tonemap. This means we need to keep the
        // input format
        QDemonRenderTextureFormats::Enum theOutputFormat = QDemonRenderTextureFormats::RGBA8;
        if (theClass->dynamicClass->getOutputTextureFormat() == QDemonRenderTextureFormats::Unknown)
            theOutputFormat = theDetails.format;
        auto theTargetTexture = theManager->allocateTexture2D(theFinalWidth, theFinalHeight, theOutputFormat);
        theBuffer->attach(QDemonRenderFrameBufferAttachment::Color0, theTargetTexture);
        theContext->setRenderTarget(theBuffer);
        QDemonRenderContextScopedProperty<QRect> __viewport(*theContext,
                                                            &QDemonRenderContext::getViewport,
                                                            &QDemonRenderContext::setViewport,
                                                            QRect(0, 0, theFinalWidth, theFinalHeight));

        QDemonRenderContextScopedProperty<bool> __scissorEnable(*theContext,
                                                                &QDemonRenderContext::isScissorTestEnabled,
                                                                &QDemonRenderContext::setScissorTestEnabled,
                                                                false);

        doRenderEffect(inRenderArgument.m_effect,
                       theClass,
                       inRenderArgument.m_colorBuffer,
                       theMVP,
                       m_context->getRenderContext()->getRenderTarget(),
                       false,
                       inRenderArgument.m_depthTexture,
                       inRenderArgument.m_depthStencilBuffer,
                       inRenderArgument.m_cameraClipRange);

        theBuffer->attach(QDemonRenderFrameBufferAttachment::Color0, QDemonRenderTextureOrRenderBuffer());
        theManager->release(theBuffer);
        return theTargetTexture;
    }

    // Render the effect to the currently bound render target using this MVP
    bool renderEffect(QDemonEffectRenderArgument inRenderArgument, QMatrix4x4 &inMVP, bool inEnableBlendWhenRenderToTarget) override
    {
        auto theClass = getEffectClass(inRenderArgument.m_effect->className);
        if (!theClass) {
            Q_ASSERT(false);
            return false;
        }

        doRenderEffect(inRenderArgument.m_effect,
                       theClass,
                       inRenderArgument.m_colorBuffer,
                       inMVP,
                       m_context->getRenderContext()->getRenderTarget(),
                       inEnableBlendWhenRenderToTarget,
                       inRenderArgument.m_depthTexture,
                       inRenderArgument.m_depthStencilBuffer,
                       inRenderArgument.m_cameraClipRange);
        return true;
    }

    void releaseEffectContext(QDemonEffectContext *inContext) override
    {
        if (inContext == nullptr)
            return;
        for (quint32 idx = 0, end = m_contexts.size(); idx < end; ++idx) {
            if (m_contexts[idx] == inContext) {
                { // replace_with_last
                    m_contexts[idx] = m_contexts.back();
                    m_contexts.pop_back();
                }
            }
        }
    }

    void resetEffectFrameData(QDemonEffectContext &inContext) override
    { // Query for size on every loop intentional
        for (qint32 idx = 0; idx < inContext.m_allocatedBuffers.size(); ++idx) {
            QDemonAllocatedBufferEntry &theBuffer(inContext.m_allocatedBuffers[idx]);
            if (theBuffer.flags.isSceneLifetime() == true)
                theBuffer.needsClear = true;
        }
        for (qint32 idx = 0; idx < inContext.m_allocatedDataBuffers.size(); ++idx) {
            QDemonAllocatedDataBufferEntry &theDataBuffer(inContext.m_allocatedDataBuffers[idx]);
            if (theDataBuffer.flags.isSceneLifetime() == true)
                theDataBuffer.needsClear = true;
        }
    }

    void setShaderData(QString path, const char *data, const char *inShaderType, const char *inShaderVersion, bool inHasGeomShader, bool inIsComputeShader) override
    {
        m_coreContext->getDynamicObjectSystemCore()->setShaderData(path, data, inShaderType, inShaderVersion, inHasGeomShader, inIsComputeShader);
    }

    //    void save(SWriteBuffer &ioBuffer,
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

    //    void load(QDemonDataRef<quint8> inData, CStrTableOrDataRef inStrDataBlock,
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
    //            QDemonRef<SEffectClass> theClassPtr(theClass);
    //            m_EffectClasses.insert(theBaseClass->GetId(), theClassPtr);
    //        }
    //    }

    QDemonRef<QDemonEffectSystemInterface> getEffectSystem(QDemonRenderContextInterface *context) override
    {
        m_context = context;

        auto theContext(m_context->getRenderContext());

        m_resourceManager = QDemonResourceManagerInterface::createResourceManager(theContext);

        // create default stencil state
        QDemonRenderStencilFunctionArgument stencilDefaultFunc(QDemonRenderBoolOp::AlwaysTrue, 0x0, 0xFF);
        QDemonRenderStencilOperationArgument stencilDefaultOp(QDemonRenderStencilOp::Keep,
                                                              QDemonRenderStencilOp::Keep,
                                                              QDemonRenderStencilOp::Keep);
        m_defaultStencilState = theContext->createDepthStencilState(theContext->isDepthTestEnabled(),
                                                                    theContext->isDepthWriteEnabled(),
                                                                    theContext->getDepthFunction(),
                                                                    theContext->isStencilTestEnabled(),
                                                                    stencilDefaultFunc,
                                                                    stencilDefaultFunc,
                                                                    stencilDefaultOp,
                                                                    stencilDefaultOp);

        return this;
    }

    QDemonRef<QDemonResourceManagerInterface> getResourceManager() override { return m_resourceManager; }
};
}

QDemonEffectSystemInterface::~QDemonEffectSystemInterface() = default;

QDemonRef<QDemonEffectSystemInterface> QDemonEffectSystemInterface::createEffectSystem(QDemonRenderContextCoreInterface *inContext)
{
    return QDemonRef<QDemonEffectSystem>(new QDemonEffectSystem(inContext));
}

QDemonEffectRenderArgument::QDemonEffectRenderArgument(QDemonRenderEffect *inEffect,
                                                       const QDemonRef<QDemonRenderTexture2D> &inColorBuffer,
                                                       const QVector2D &inCameraClipRange,
                                                       const QDemonRef<QDemonRenderTexture2D> &inDepthTexture,
                                                       const QDemonRef<QDemonRenderTexture2D> &inDepthBuffer)
    : m_effect(inEffect)
    , m_colorBuffer(inColorBuffer)
    , m_cameraClipRange(inCameraClipRange)
    , m_depthTexture(inDepthTexture)
    , m_depthStencilBuffer(inDepthBuffer)
{
}

QDemonRenderEffect::~QDemonRenderEffect()
{

}

QT_END_NAMESPACE
