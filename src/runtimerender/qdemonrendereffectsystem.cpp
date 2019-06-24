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
#include <QtDemonRuntimeRender/qdemonrendercamera.h>
#include <QtDemonRuntimeRender/qdemonrenderbuffermanager.h>
#include <QtDemonRuntimeRender/qdemonoffscreenrendermanager.h>
#include <QtDemonRuntimeRender/qdemonrendershadercache.h>
#include <QtDemonRuntimeRender/qdemonrendererutil.h>
#include <QtDemonRuntimeRender/qdemonrenderprefiltertexture.h>
#include <qdemonoffscreenrenderkey.h>
#include <qdemonrenderdynamicobjectsystemutil.h>

QT_BEGIN_NAMESPACE

using namespace dynamic;

// None of this code will work if the size of void* changes because that would mean that
// the alignment of some of the objects isn't 4 bytes but would be 8 bytes.

/*
                ApplyBufferValue,
                //Apply the depth buffer as an input texture.
                ApplyDepthValue,
                Render, //Render to current FBO
                */

struct QDemonAllocatedBufferEntry
{
    QAtomicInt ref;
    QByteArray name;
    QDemonRef<QDemonRenderFrameBuffer> frameBuffer;
    QDemonRef<QDemonRenderTexture2D> texture;
    QDemonAllocateBufferFlags flags;
    bool needsClear;

    QDemonAllocatedBufferEntry(const QByteArray &inName, QDemonRenderFrameBuffer &inFb, QDemonRenderTexture2D &inTexture, QDemonAllocateBufferFlags inFlags)
        : name(inName), frameBuffer(&inFb), texture(&inTexture), flags(inFlags), needsClear(true)
    {
    }
    QDemonAllocatedBufferEntry() = default;
};

struct QDemonAllocatedImageEntry
{
    QAtomicInt ref;
    QByteArray name;
    QDemonRef<QDemonRenderImage2D> image;
    QDemonRef<QDemonRenderTexture2D> texture;
    QDemonAllocateBufferFlags flags;

    QDemonAllocatedImageEntry(const QByteArray &inName, QDemonRenderImage2D &inImage, QDemonRenderTexture2D &inTexture, QDemonAllocateBufferFlags inFlags)
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
    QByteArray name;
    QDemonRef<QDemonRenderDataBuffer> dataBuffer;
    QDemonRenderBufferType bufferType;
    QDemonByteRef bufferData;
    QDemonAllocateBufferFlags flags;
    bool needsClear;

    QDemonAllocatedDataBufferEntry(const QByteArray &inName,
                                   QDemonRenderDataBuffer &inDataBuffer,
                                   QDemonRenderBufferType inType,
                                   const QDemonByteRef &data,
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

    void set(const QDemonRef<QDemonRenderTexture2D> &inTexture, bool inNeedsAlphaMultiply, const QDemonRenderEffect::TextureProperty *inDefinition)
    {
        float theMixValue(inNeedsAlphaMultiply ? 0.0f : 1.0f);
        if (inTexture && inDefinition) {
            inTexture->setMagFilter(inDefinition->magFilterType);
            inTexture->setMinFilter(static_cast<QDemonRenderTextureMinifyingOp>(inDefinition->magFilterType));
            inTexture->setTextureWrapS(inDefinition->clampType);
            inTexture->setTextureWrapT(inDefinition->clampType);
        }
        texture.set(inTexture.data());
        if (inTexture) {
            QDemonTextureDetails theDetails(inTexture->textureDetails());
            textureData.set(QVector4D((float)theDetails.width, (float)theDetails.height, theMixValue, 0.0f));
            // I have no idea what these flags do.
            textureFlags.set(1);
        } else {
            textureFlags.set(0);
        }
    }

    static QDemonTextureEntry createTextureEntry(const QDemonRef<QDemonRenderShaderProgram> &inShader,
                                                 const QByteArray &inStem,
                                                 QString &inBuilder,
                                                 QString &inBuilder2)
    {
        inBuilder = QString::fromLatin1(inStem);
        inBuilder.append(QString::fromLatin1("Info"));
        inBuilder2 = QString::fromLatin1("flag");
        inBuilder2.append(QString::fromLatin1(inStem));
        return QDemonTextureEntry(inShader, inStem, inBuilder.toLocal8Bit(), inBuilder2.toLocal8Bit());
    }
};

typedef QPair<QByteArray, QDemonRef<QDemonTextureEntry>> TNamedTextureEntry;
typedef QPair<QByteArray, QDemonRef<QDemonImageEntry>> TNamedImageEntry;
typedef QPair<QByteArray, QDemonRef<QDemonDataBufferEntry>> TNamedDataBufferEntry;

struct QDemonEffectClass
{
    QAtomicInt ref;
    QDemonDynamicObjectClass *dynamicClass;

    QDemonEffectClass(QDemonDynamicObjectClass &dynClass) : dynamicClass(&dynClass) {}
};

struct QDemonEffectContext
{
    QAtomicInt ref;
    QByteArray m_className;
    QDemonRenderContextInterface *m_context;
    QDemonRef<QDemonResourceManager> m_resourceManager;
    QVector<QDemonAllocatedBufferEntry> m_allocatedBuffers;
    QVector<QDemonAllocatedImageEntry> m_allocatedImages;
    QVector<QDemonAllocatedDataBufferEntry> m_allocatedDataBuffers;
    QVector<TNamedTextureEntry> m_textureEntries;
    QVector<TNamedImageEntry> m_imageEntries;
    QVector<TNamedDataBufferEntry> m_dataBufferEntries;

    QDemonEffectContext(const QByteArray &inName, QDemonRenderContextInterface *ctx, const QDemonRef<QDemonResourceManager> &inManager)
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

    qint32 findBuffer(const QByteArray &inName)
    {
        for (qint32 idx = 0, end = m_allocatedBuffers.size(); idx < end; ++idx)
            if (m_allocatedBuffers[idx].name == inName)
                return idx;
        return m_allocatedBuffers.size();
    }

    qint32 findImage(const QByteArray &inName)
    {
        for (qint32 idx = 0, end = m_allocatedImages.size(); idx < end; ++idx)
            if (m_allocatedImages[idx].name == inName)
                return idx;

        return m_allocatedImages.size();
    }

    qint32 findDataBuffer(const QByteArray &inName)
    {
        for (qint32 idx = 0, end = m_allocatedDataBuffers.size(); idx < end; ++idx) {
            if (m_allocatedDataBuffers[idx].name == inName)
                return idx;
        }

        return m_allocatedDataBuffers.size();
    }

    void setTexture(const QDemonRef<QDemonRenderShaderProgram> &inShader,
                    const QByteArray &inPropName,
                    const QDemonRef<QDemonRenderTexture2D> &inTexture,
                    bool inNeedsMultiply,
                    QString &inStringBuilder,
                    QString &inStringBuilder2,
                    const QDemonRenderEffect::TextureProperty *inPropDec = nullptr)
    {
        QDemonRef<QDemonTextureEntry> theTextureEntry;
        for (qint32 idx = 0, end = m_textureEntries.size(); idx < end && theTextureEntry == nullptr; ++idx) {
            if (m_textureEntries[idx].first == inPropName && m_textureEntries[idx].second->shader == inShader)
                theTextureEntry = m_textureEntries[idx].second;
        }
        if (theTextureEntry == nullptr) {
            QDemonRef<QDemonTextureEntry> theNewEntry(new QDemonTextureEntry(
                    QDemonTextureEntry::createTextureEntry(inShader, inPropName, inStringBuilder, inStringBuilder2)));
            m_textureEntries.push_back(QPair<QByteArray, QDemonRef<QDemonTextureEntry>>(inPropName, theNewEntry));
            theTextureEntry = theNewEntry;
        }
        theTextureEntry->set(inTexture, inNeedsMultiply, inPropDec);
    }

    void setImage(const QDemonRef<QDemonRenderShaderProgram> &inShader, const QByteArray &inPropName, const QDemonRef<QDemonRenderImage2D> &inImage)
    {
        QDemonRef<QDemonImageEntry> theImageEntry;
        for (qint32 idx = 0, end = m_imageEntries.size(); idx < end && theImageEntry == nullptr; ++idx) {
            if (m_imageEntries[idx].first == inPropName && m_imageEntries[idx].second->shader == inShader)
                theImageEntry = m_imageEntries[idx].second;
        }
        if (theImageEntry == nullptr) {
            QDemonRef<QDemonImageEntry> theNewEntry(
                    new QDemonImageEntry(QDemonImageEntry::createImageEntry(inShader, inPropName)));
            m_imageEntries.push_back(QPair<QByteArray, QDemonRef<QDemonImageEntry>>(inPropName, theNewEntry));
            theImageEntry = theNewEntry;
        }

        theImageEntry->set(inImage.data());
    }

    void setDataBuffer(const QDemonRef<QDemonRenderShaderProgram> &inShader,
                       const QByteArray &inPropName,
                       const QDemonRef<QDemonRenderDataBuffer> &inBuffer)
    {
        QDemonRef<QDemonDataBufferEntry> theDataBufferEntry;
        for (qint32 idx = 0, end = m_dataBufferEntries.size(); idx < end && theDataBufferEntry == nullptr; ++idx) {
            if (m_dataBufferEntries[idx].first == inPropName && m_dataBufferEntries[idx].second->shader == inShader)
                theDataBufferEntry = m_dataBufferEntries[idx].second;
        }
        if (theDataBufferEntry == nullptr) {
            QDemonRef<QDemonDataBufferEntry> theNewEntry(new QDemonDataBufferEntry(
                    QDemonDataBufferEntry::createDataBufferEntry(inShader, inPropName)));
            m_dataBufferEntries.push_back(QPair<QByteArray, QDemonRef<QDemonDataBufferEntry>>(inPropName, theNewEntry));
            theDataBufferEntry = theNewEntry;
        }

        theDataBufferEntry->set(inBuffer.data());
    }
};

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
    QDemonEffectShader(const QDemonRef<QDemonRenderShaderProgram> &inShader);
};

QDemonEffectShader::QDemonEffectShader(const QDemonRef<QDemonRenderShaderProgram> &inShader)
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

QDemonEffectSystem::QDemonEffectSystem(QDemonRenderContextInterface *inContext) : m_context(inContext)
{
    init();
}

QDemonEffectSystem::~QDemonEffectSystem()
{
    for (qint32 idx = 0, end = m_contexts.size(); idx < end; ++idx)
        delete m_contexts[idx].data();
    m_contexts.clear();
}

QDemonEffectContext &QDemonEffectSystem::getEffectContext(QDemonRenderEffect &inEffect)
{
    if (inEffect.m_context == nullptr) {
        inEffect.m_context = new QDemonEffectContext(inEffect.className, m_context, m_resourceManager);
        m_contexts.push_back(inEffect.m_context);
    }
    return *inEffect.m_context;
}

const QDemonRef<QDemonEffectClass> QDemonEffectSystem::getEffectClass(const QByteArray &inStr) const
{
    const auto theIter = m_effectClasses.constFind(inStr);
    if (theIter != m_effectClasses.cend())
        return theIter.value();
    return nullptr;
}

bool QDemonEffectSystem::isEffectRegistered(const QByteArray &inStr) { return getEffectClass(inStr) != nullptr; }

QVector<QByteArray> QDemonEffectSystem::getRegisteredEffects()
{
    m_effectList.clear();
    auto theIter = m_effectClasses.cbegin();
    const auto theEnd = m_effectClasses.cend();
    for (; theIter != theEnd; ++theIter)
        m_effectList.push_back(theIter.key());
    return m_effectList;
}

void QDemonEffectSystem::setEffectPropertyDefaultValue(QString inName, QString inPropName, QDemonByteView inDefaultData)
{
    Q_UNUSED(inName)
    Q_UNUSED(inPropName)
    Q_UNUSED(inDefaultData)
    Q_ASSERT(0);
    //        m_context->dynamicObjectSystem()->setPropertyDefaultValue(inName, inPropName, inDefaultData);
}

void QDemonEffectSystem::setEffectPropertyEnumNames(QString inName, QString inPropName, QDemonDataView<QString> inNames)
{
    Q_UNUSED(inName)
    Q_UNUSED(inPropName)
    Q_UNUSED(inNames)
    Q_ASSERT(0);
    //        m_context->dynamicObjectSystem()->setPropertyEnumNames(inName, inPropName, inNames);
}

bool QDemonEffectSystem::registerEffect(const QByteArray &inName)
{
    Q_UNUSED(inName)
    Q_ASSERT(0);
//    if (isEffectRegistered(inName))
//        return false;
//    m_context->dynamicObjectSystem()->doRegister(inName, inProperties, sizeof(QDemonRenderEffect), QDemonRenderGraphObject::Type::Effect);
//    auto theClass = m_context->dynamicObjectSystem()->dynamicObjectClass(inName);
//    QDemonRef<QDemonEffectClass> theEffect(new QDemonEffectClass(*theClass));
//    m_effectClasses.insert(inName, theEffect);
//    return true;
return false;
}

bool QDemonEffectSystem::unregisterEffect(const QByteArray &inName)
{
    Q_UNUSED(inName)
    Q_ASSERT(0);
    //        if (!isEffectRegistered(inName))
    //            return false;

    //        m_context->dynamicObjectSystem()->unregister(inName);

    //        TEffectClassMap::iterator iter = m_effectClasses.find(inName);
    //        if (iter != m_effectClasses.end())
    //            m_effectClasses.erase(iter);

    //        for (quint32 idx = 0, end = m_contexts.size(); idx < end; ++idx) {
    //            if (m_contexts[idx]->m_className == inName)
    //                releaseEffectContext(m_contexts[idx].data());
    //        }
    //        return true;
    return false;
}

void QDemonEffectSystem::setEffectPropertyTextureSettings(QString inName,
                                                          QString inPropName,
                                                          QString inPropPath,
                                                          QDemonRenderTextureTypeValue inTexType,
                                                          QDemonRenderTextureCoordOp inCoordOp,
                                                          QDemonRenderTextureMagnifyingOp inMagFilterOp,
                                                          QDemonRenderTextureMinifyingOp inMinFilterOp)
{
    Q_UNUSED(inName)
    Q_UNUSED(inPropName)
    Q_UNUSED(inPropPath)
    Q_UNUSED(inTexType)
    Q_UNUSED(inCoordOp)
    Q_UNUSED(inMagFilterOp)
    Q_UNUSED(inMinFilterOp)
    Q_ASSERT(0);
    //        m_context->dynamicObjectSystem()
    //                ->setPropertyTextureSettings(inName, inPropName, inPropPath, inTexType, inCoordOp, inMagFilterOp, inMinFilterOp);
}

void QDemonEffectSystem::setEffectRequiresDepthTexture(const QByteArray &inEffectName, bool inValue)
{
    Q_UNUSED(inEffectName)
    Q_UNUSED(inValue)
    Q_ASSERT(0);
    //        auto theClass = getEffectClass(inEffectName);
    //        if (theClass == nullptr) {
    //            Q_ASSERT(false);
    //            return;
    //        }
    //        theClass->dynamicClass->setRequiresDepthTexture(inValue);
}

bool QDemonEffectSystem::doesEffectRequireDepthTexture(const QByteArray &inEffectName) const
{
    Q_UNUSED(inEffectName)
    Q_ASSERT(0);
    //        const auto theClass = getEffectClass(inEffectName);
    //        if (theClass == nullptr) {
    //            Q_ASSERT(false);
    //            return false;
    //        }
    //        return theClass->dynamicClass->requiresDepthTexture();
    return false;
}

void QDemonEffectSystem::setEffectRequiresCompilation(const QByteArray &inEffectName, bool inValue)
{
    Q_UNUSED(inValue)
    Q_ASSERT(0);
    auto theClass = getEffectClass(inEffectName);
    if (theClass == nullptr) {
        Q_ASSERT(false);
        return;
    }
//    theClass->dynamicClass->setRequiresCompilation(inValue);
}

bool QDemonEffectSystem::doesEffectRequireCompilation(const QByteArray &inEffectName) const
{
    Q_UNUSED(inEffectName)
    Q_ASSERT(0);
    //        const auto theClass = getEffectClass(inEffectName);
    //        if (theClass == nullptr) {
    //            Q_ASSERT(false);
    //            return false;
    //        }
    //        return theClass->dynamicClass->requiresCompilation();
    return false;
}

QDemonRenderEffect *QDemonEffectSystem::createEffectInstance(const QByteArray &inEffectName)
{
    auto theClass = getEffectClass(inEffectName);
    if (theClass == nullptr)
        return nullptr;
    //        StaticAssert<(sizeof(SEffect) % 4 == 0)>::valid_expression();

    //        QDemonRenderEffect *theEffect = static_cast<QDemonRenderEffect *>(
    //                m_context->dynamicObjectSystem()->createInstance(inEffectName));
    //        theEffect->initialize();
    //        return theEffect;
    return nullptr;
}

void QDemonEffectSystem::allocateBuffer(QDemonRenderEffect &inEffect, const QDemonAllocateBuffer &inCommand, quint32 inFinalWidth, quint32 inFinalHeight, QDemonRenderTextureFormat inSourceTextureFormat)
{
    // Check to see if it is already allocated and if it is, is it the correct size. If both of
    // these assumptions hold, then we are good.
    QDemonRef<QDemonRenderTexture2D> theBufferTexture;
    const qint32 theWidth = QDemonRendererUtil::nextMultipleOf4((quint32)(inFinalWidth * inCommand.m_sizeMultiplier));
    const qint32 theHeight = QDemonRendererUtil::nextMultipleOf4((quint32)(inFinalHeight * inCommand.m_sizeMultiplier));
    QDemonRenderTextureFormat resultFormat = inCommand.m_format;
    if (resultFormat == QDemonRenderTextureFormat::Unknown)
        resultFormat = inSourceTextureFormat;

    if (inEffect.m_context) {
        QDemonEffectContext &theContext(*inEffect.m_context);
        // size intentionally requiried every loop;
        qint32 bufferIdx = theContext.findBuffer(inCommand.m_name);
        if (bufferIdx < theContext.m_allocatedBuffers.size()) {
            QDemonAllocatedBufferEntry &theEntry(theContext.m_allocatedBuffers[bufferIdx]);
            QDemonTextureDetails theDetails = theEntry.texture->textureDetails();
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
        theTexture->setMinFilter(static_cast<QDemonRenderTextureMinifyingOp>(inCommand.m_filterOp));
        theTexture->setTextureWrapS(inCommand.m_texCoordOp);
        theTexture->setTextureWrapT(inCommand.m_texCoordOp);
        theFB->attach(QDemonRenderFrameBufferAttachment::Color0, theTexture);
        theContext.m_allocatedBuffers.push_back(
                    QDemonAllocatedBufferEntry(inCommand.m_name, *theFB, *theTexture, inCommand.m_bufferFlags));
        theBufferTexture = theTexture;
    }
}

void QDemonEffectSystem::allocateImage(QDemonRenderEffect &inEffect, const QDemonAllocateImage &inCommand, quint32 inFinalWidth, quint32 inFinalHeight)
{
    QDemonRef<QDemonRenderImage2D> theImage;
    qint32 theWidth = QDemonRendererUtil::nextMultipleOf4((quint32)(inFinalWidth * inCommand.m_sizeMultiplier));
    qint32 theHeight = QDemonRendererUtil::nextMultipleOf4((quint32)(inFinalHeight * inCommand.m_sizeMultiplier));

    Q_ASSERT(inCommand.m_format != QDemonRenderTextureFormat::Unknown);

    if (inEffect.m_context) {
        QDemonEffectContext &theContext(*inEffect.m_context);
        // size intentionally requiried every loop;
        qint32 imageIdx = theContext.findImage(inCommand.m_name);
        if (imageIdx < theContext.m_allocatedImages.size()) {
            QDemonAllocatedImageEntry &theEntry(theContext.m_allocatedImages[imageIdx]);
            QDemonTextureDetails theDetails = theEntry.texture->textureDetails();
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
        theTexture->setMinFilter(static_cast<QDemonRenderTextureMinifyingOp>(inCommand.m_filterOp));
        theTexture->setTextureWrapS(inCommand.m_texCoordOp);
        theTexture->setTextureWrapT(inCommand.m_texCoordOp);
        auto theImage = (m_resourceManager->allocateImage2D(theTexture, inCommand.m_access));
        theContext.m_allocatedImages.push_back(
                    QDemonAllocatedImageEntry(inCommand.m_name, *theImage, *theTexture, inCommand.m_bufferFlags));
    }
}

void QDemonEffectSystem::allocateDataBuffer(QDemonRenderEffect &inEffect, const QDemonAllocateDataBuffer &inCommand)
{
    const qint32 theBufferSize = qint32(inCommand.m_size);
    Q_ASSERT(theBufferSize > 0);
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
        const auto &theRenderContext(m_context->renderContext());
        quint8 *initialData = (quint8 *)::malloc(theBufferSize);
        QDemonByteRef data((quint8 *)initialData, theBufferSize);
        memset(initialData, 0x0L, theBufferSize);
        if (inCommand.m_dataBufferType == QDemonRenderBufferType::Storage) {
            theDataBuffer = new QDemonRenderStorageBuffer(theRenderContext, inCommand.m_name,
                                                          QDemonRenderBufferUsageType::Dynamic,
                                                          data,
                                                          nullptr);
        } else if (inCommand.m_dataBufferType == QDemonRenderBufferType::DrawIndirect) {
            Q_ASSERT(theBufferSize == sizeof(DrawArraysIndirectCommand));
            // init a draw call
            quint32 *pIndirectDrawCall = (quint32 *)initialData;
            // vertex count we draw points right now only
            // the rest we fill in by GPU
            pIndirectDrawCall[0] = 1;
            theDataBuffer = new QDemonRenderDrawIndirectBuffer(theRenderContext, QDemonRenderBufferUsageType::Dynamic, data);
        } else
            Q_ASSERT(false);

        theContext.m_allocatedDataBuffers.push_back(QDemonAllocatedDataBufferEntry(inCommand.m_name,
                                                                                   *theDataBuffer,
                                                                                   inCommand.m_dataBufferType,
                                                                                   data,
                                                                                   inCommand.m_bufferFlags));

        // create wrapper buffer
        if (inCommand.m_dataBufferWrapType == QDemonRenderBufferType::Storage
                && !inCommand.m_wrapName.isEmpty() && theDataBuffer) {
            theDataWrapBuffer = new QDemonRenderStorageBuffer(theRenderContext, inCommand.m_wrapName,
                                                              QDemonRenderBufferUsageType::Dynamic,
                                                              data,
                                                              theDataBuffer.data());
            theContext.m_allocatedDataBuffers.push_back(QDemonAllocatedDataBufferEntry(inCommand.m_wrapName,
                                                                                       *theDataWrapBuffer,
                                                                                       inCommand.m_dataBufferWrapType,
                                                                                       QDemonByteRef(),
                                                                                       inCommand.m_bufferFlags));
        }
        ::free(initialData);
    }
}

QDemonRef<QDemonRenderTexture2D> QDemonEffectSystem::findTexture(QDemonRenderEffect *inEffect, const QByteArray &inName)
{
    if (inEffect->m_context) {
        QDemonEffectContext &theContext(*inEffect->m_context);
        qint32 bufferIdx = theContext.findBuffer(inName);
        if (bufferIdx < theContext.m_allocatedBuffers.size())
            return theContext.m_allocatedBuffers[bufferIdx].texture;
    }
    Q_ASSERT(false);
    return nullptr;
}

QDemonRef<QDemonRenderFrameBuffer> QDemonEffectSystem::bindBuffer(QDemonRenderEffect &inEffect, const QDemonBindBuffer &inCommand, QMatrix4x4 &outMVP, QVector2D &outDestSize)
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
                   inEffect.className,
                   inCommand.m_bufferName.constData());
        QString errorMsg = QObject::tr("Failed to compile \"%1\" effect.\nConsider"
                                       " removing it from the presentation.")
                                   .arg(QString::fromLatin1(inEffect.className));
        // TODO:
        //            QDEMON_ALWAYS_ASSERT_MESSAGE(errorMsg.toUtf8());
        outMVP = QMatrix4x4();
        return nullptr;
    }

    if (theTexture) {
        QDemonRenderCamera::setupOrthographicCameraForOffscreenRender(*theTexture, outMVP);
        QDemonTextureDetails theDetails(theTexture->textureDetails());
        m_context->renderContext()->setViewport(QRect(0, 0, (quint32)theDetails.width, (quint32)theDetails.height));
        outDestSize = QVector2D((float)theDetails.width, (float)theDetails.height);
    }

    return theBuffer;
}

QDemonRef<QDemonEffectShader> QDemonEffectSystem::bindShader(const QByteArray &inEffectId, const QDemonBindShader &inCommand)
{
    auto theClass = getEffectClass(inEffectId);
    if (!theClass) {
        Q_ASSERT(false);
        return nullptr;
    }

    Q_ASSERT(0);
    bool forceCompilation = true; // theClass->dynamicClass->requiresCompilation();

    auto key = TStrStrPair(inCommand.m_shaderPath, inCommand.m_shaderDefine);
    auto theInsertResult = m_shaderMap.find(key);
    const bool found = (theInsertResult != m_shaderMap.end());
    if (!found)
        theInsertResult = m_shaderMap.insert(key, QDemonRef<QDemonEffectShader>());

    if (found || forceCompilation) {
        auto theProgram = m_context->dynamicObjectSystem()
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
        const auto &theContext(m_context->renderContext());
        theContext->setActiveShader(theInsertResult.value()->m_shader);
    }

    return theInsertResult.value();
}

void QDemonEffectSystem::doApplyInstanceValue(QDemonRenderEffect *inEffect,
                                              const QByteArray &inPropertyName,
                                              const QVariant &propertyValue,
                                              QDemonRenderShaderDataType inPropertyType,
                                              const QDemonRef<QDemonRenderShaderProgram> &inShader)
{
    auto theConstant = inShader->shaderConstant(inPropertyName);
    if (theConstant) {
        if (theConstant->getShaderConstantType() == inPropertyType) {
            if (inPropertyType == QDemonRenderShaderDataType::Texture2D) {
                const auto &theBufferManager(m_context->bufferManager());
                auto theOffscreenRenderer = m_context->offscreenRenderManager();
                bool needsAlphaMultiply = true;

                const QDemonRenderEffect::TextureProperty *textureProperty = reinterpret_cast<QDemonRenderEffect::TextureProperty *>(propertyValue.value<void *>());
                QDemonRenderImage *image = textureProperty->texImage;
                if (image) {
                    const QString &imageSource = image->m_imagePath;
                    QDemonRef<QDemonRenderTexture2D> theTexture;
                    if (!imageSource.isEmpty()) {
                        if (theOffscreenRenderer->hasOffscreenRenderer(imageSource)) {
                            QDemonOffscreenRenderResult theResult = theOffscreenRenderer->getRenderedItem(imageSource);
                            needsAlphaMultiply = false;
                            theTexture = theResult.texture;
                        } else {
                            QDemonRenderImageTextureData theTextureData = theBufferManager->loadRenderImage(imageSource);
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
                                                           textureProperty);
                }
            } else if (inPropertyType == QDemonRenderShaderDataType::Image2D) {
                // TODO:
                //                    StaticAssert<sizeof(QString)
                //                            == sizeof(QDemonRenderTexture2DPtr)>::valid_expression();
                QDemonRef<QDemonRenderImage2D> theImage;
                getEffectContext(*inEffect).setImage(inShader, inPropertyName, theImage);
            } else if (inPropertyType == QDemonRenderShaderDataType::DataBuffer) {
                // we don't handle this here
            } else {
                switch (inPropertyType) {
                case QDemonRenderShaderDataType::Integer:
                    inShader->setPropertyValue(theConstant.data(), propertyValue.toInt());
                    break;
                case QDemonRenderShaderDataType::IntegerVec2:
                    inShader->setPropertyValue(theConstant.data(), propertyValue.value<qint32_2>());
                    break;
                case QDemonRenderShaderDataType::IntegerVec3:
                    inShader->setPropertyValue(theConstant.data(), propertyValue.value<qint32_3>());
                    break;
                case QDemonRenderShaderDataType::IntegerVec4:
                    inShader->setPropertyValue(theConstant.data(), propertyValue.value<qint32_4>());
                    break;
                case QDemonRenderShaderDataType::Boolean:
                    inShader->setPropertyValue(theConstant.data(), propertyValue.value<bool>());
                    break;
                case QDemonRenderShaderDataType::BooleanVec2:
                    inShader->setPropertyValue(theConstant.data(), propertyValue.value<bool_2>());
                    break;
                case QDemonRenderShaderDataType::BooleanVec3:
                    inShader->setPropertyValue(theConstant.data(), propertyValue.value<bool_3>());
                    break;
                case QDemonRenderShaderDataType::BooleanVec4:
                    inShader->setPropertyValue(theConstant.data(), propertyValue.value<bool_4>());
                    break;
                case QDemonRenderShaderDataType::Float:
                    inShader->setPropertyValue(theConstant.data(), propertyValue.value<float>());
                    break;
                case QDemonRenderShaderDataType::Vec2:
                    inShader->setPropertyValue(theConstant.data(), propertyValue.value<QVector2D>());
                    break;
                case QDemonRenderShaderDataType::Vec3:
                    inShader->setPropertyValue(theConstant.data(), propertyValue.value<QVector3D>());
                    break;
                case QDemonRenderShaderDataType::Vec4:
                    inShader->setPropertyValue(theConstant.data(), propertyValue.value<QVector4D>());
                    break;
                case QDemonRenderShaderDataType::UnsignedInteger:
                    inShader->setPropertyValue(theConstant.data(), propertyValue.value<quint32>());
                    break;
                case QDemonRenderShaderDataType::UnsignedIntegerVec2:
                    inShader->setPropertyValue(theConstant.data(), propertyValue.value<quint32_2>());
                    break;
                case QDemonRenderShaderDataType::UnsignedIntegerVec3:
                    inShader->setPropertyValue(theConstant.data(), propertyValue.value<quint32_3>());
                    break;
                case QDemonRenderShaderDataType::UnsignedIntegerVec4:
                    inShader->setPropertyValue(theConstant.data(), propertyValue.value<quint32_4>());
                    break;
                case QDemonRenderShaderDataType::Matrix3x3:
                    inShader->setPropertyValue(theConstant.data(), propertyValue.value<QMatrix3x3>());
                    break;
                case QDemonRenderShaderDataType::Matrix4x4:
                    inShader->setPropertyValue(theConstant.data(), propertyValue.value<QMatrix4x4>());
                    break;
                case QDemonRenderShaderDataType::Texture2D:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderTexture2D **>(propertyValue.value<void *>())));
                    break;
                case QDemonRenderShaderDataType::Texture2DHandle:
                    inShader->setPropertyValue(theConstant.data(),
                                               *(reinterpret_cast<QDemonRenderTexture2D ***>(propertyValue.value<void *>())));
                    break;
                case QDemonRenderShaderDataType::Texture2DArray:
                    inShader->setPropertyValue(theConstant.data(),
                                               *(reinterpret_cast<QDemonRenderTexture2DArray **>(propertyValue.value<void *>())));
                    break;
                case QDemonRenderShaderDataType::TextureCube:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderTextureCube **>(propertyValue.value<void *>())));
                    break;
                case QDemonRenderShaderDataType::TextureCubeHandle:
                    inShader->setPropertyValue(theConstant.data(),
                                               *(reinterpret_cast<QDemonRenderTextureCube ***>(propertyValue.value<void *>())));
                    break;
                case QDemonRenderShaderDataType::Image2D:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderImage2D **>(propertyValue.value<void *>())));
                    break;
                case QDemonRenderShaderDataType::DataBuffer:
                    inShader->setPropertyValue(theConstant.data(), *(reinterpret_cast<QDemonRenderDataBuffer **>(propertyValue.value<void *>())));
                    break;
                default:
                    Q_UNREACHABLE();
                }
            }

        } else {
            qCCritical(INVALID_OPERATION,
                       "Effect ApplyInstanceValue command datatype "
                       "and shader datatypes differ for property %s",
                       inPropertyName.constData());
            Q_ASSERT(false);
        }
    }
}

void QDemonEffectSystem::applyInstanceValue(QDemonRenderEffect *inEffect,
                                            const QDemonRef<QDemonEffectClass> &inClass,
                                            const QDemonRef<QDemonRenderShaderProgram> &inShader,
                                            const QDemonApplyInstanceValue &inCommand)
{
    Q_UNUSED(inEffect)
    Q_UNUSED(inClass)
    Q_UNUSED(inShader)
    Q_UNUSED(inCommand)
    Q_ASSERT(0);
    // sanity check
    //        if (!inCommand.m_propertyName.isEmpty()) {
    //            bool canGetData = inCommand.m_valueOffset + getSizeofShaderDataType(inCommand.m_valueType) <= inEffect->dataSectionByteSize;
    //            if (canGetData == false) {
    //                Q_ASSERT(false);
    //                return;
    //            }
    //            quint8 *dataPtr = inEffect->getDataSectionBegin() + inCommand.m_valueOffset;
    //            const QDemonPropertyDefinition *theDefinition = inClass->dynamicClass->findPropertyByName(inCommand.m_propertyName);
    //            if (theDefinition)
    //                doApplyInstanceValue(inEffect, dataPtr, inCommand.m_propertyName, inCommand.m_valueType, inShader, *theDefinition);
    //        } else {
    //            QDemonDataView<QDemonPropertyDefinition> theDefs = inClass->dynamicClass->getProperties();
    //            for (quint32 idx = 0, end = theDefs.size(); idx < end; ++idx) {
    //                const QDemonPropertyDefinition &theDefinition(theDefs[idx]);
    //                auto theConstant = inShader->shaderConstant(theDefinition.name);

    //                // This is fine, the property wasn't found and we continue, no problem.
    //                if (!theConstant)
    //                    continue;
    //                quint8 *dataPtr = inEffect->getDataSectionBegin() + theDefinition.offset;
    //                doApplyInstanceValue(inEffect, dataPtr, theDefinition.name, theDefinition.dataType, inShader, theDefinition);
    //            }
    //        }
}

void QDemonEffectSystem::applyValue(QDemonRenderEffect *inEffect, const QDemonRef<QDemonEffectClass> &inClass, const QDemonRef<QDemonRenderShaderProgram> &inShader, const QDemonApplyValue &inCommand)
{
    Q_UNUSED(inEffect)
    Q_UNUSED(inClass)
    Q_UNUSED(inShader)
    Q_UNUSED(inCommand)
    Q_ASSERT(0);
    //        if (!inCommand.m_propertyName.isEmpty()) {
    //            quint8 *dataPtr = inCommand.m_value.mData;
    //            const QDemonPropertyDefinition *theDefinition = inClass->dynamicClass->findPropertyByName(inCommand.m_propertyName);
    //            if (theDefinition)
    //                doApplyInstanceValue(inEffect, dataPtr, inCommand.m_propertyName, inCommand.m_valueType, inShader, *theDefinition);
    //        }
}

bool QDemonEffectSystem::applyBlending(const QDemonApplyBlending &inCommand)
{
    const auto &theContext(m_context->renderContext());

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

QDemonEffectTextureData QDemonEffectSystem::applyBufferValue(QDemonRenderEffect *inEffect, const QDemonRef<QDemonRenderShaderProgram> &inShader, const QDemonApplyBufferValue &inCommand, const QDemonRef<QDemonRenderTexture2D> &inSourceTexture, const QDemonEffectTextureData &inCurrentSourceTexture)
{
    QDemonEffectTextureData theTextureToBind;
    if (!inCommand.m_bufferName.isEmpty()) {
        if (inEffect->m_context) {
            QDemonEffectContext &theContext(*inEffect->m_context);
            qint32 bufferIdx = theContext.findBuffer(inCommand.m_bufferName);
            if (bufferIdx < theContext.m_allocatedBuffers.size()) {
                QDemonAllocatedBufferEntry &theEntry(theContext.m_allocatedBuffers[bufferIdx]);
                if (theEntry.needsClear) {
                    auto theRenderContext(m_context->renderContext());

                    theRenderContext->setRenderTarget(theEntry.frameBuffer);
                    // Note that depth/stencil buffers need an explicit clear in their bind
                    // commands in order to ensure
                    // we clear the least amount of information possible.

                    if (theEntry.texture) {
                        QDemonRenderTextureFormat theTextureFormat = theEntry.texture->textureDetails().format;
                        if (theTextureFormat != QDemonRenderTextureFormat::Depth16
                                && theTextureFormat != QDemonRenderTextureFormat::Depth24
                                && theTextureFormat != QDemonRenderTextureFormat::Depth32
                                && theTextureFormat != QDemonRenderTextureFormat::Depth24Stencil8) {
                            QDemonRenderContextScopedProperty<QVector4D> __clearColor(*theRenderContext,
                                                                                      &QDemonRenderContext::clearColor,
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
                       inEffect->className,
                       inCommand.m_bufferName.constData());
            Q_ASSERT(false);
        }
    } else { // no name means bind the source
        theTextureToBind = QDemonEffectTextureData(inSourceTexture, false);
    }

    if (!inCommand.m_paramName.isEmpty()) {
        auto theConstant = inShader->shaderConstant(inCommand.m_paramName);

        if (theConstant) {
            if (theConstant->getShaderConstantType() != QDemonRenderShaderDataType::Texture2D) {
                qCCritical(INVALID_OPERATION,
                           "Effect %s: Binding buffer to parameter %s that is not a texture",
                           inEffect->className,
                           inCommand.m_paramName.constData());
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

void QDemonEffectSystem::applyDepthValue(QDemonRenderEffect *inEffect, const QDemonRef<QDemonRenderShaderProgram> &inShader, const QDemonApplyDepthValue &inCommand, const QDemonRef<QDemonRenderTexture2D> &inTexture)
{
    auto theConstant = inShader->shaderConstant(inCommand.m_paramName);

    if (theConstant) {
        if (theConstant->getShaderConstantType() != QDemonRenderShaderDataType::Texture2D) {
            qCCritical(INVALID_OPERATION,
                       "Effect %s: Binding buffer to parameter %s that is not a texture",
                       inEffect->className,
                       inCommand.m_paramName.constData());
            Q_ASSERT(false);
        } else {
            getEffectContext(*inEffect).setTexture(inShader, inCommand.m_paramName, inTexture, false, m_textureStringBuilder, m_textureStringBuilder2);
        }
    }
}

void QDemonEffectSystem::applyImageValue(QDemonRenderEffect *inEffect, const QDemonRef<QDemonRenderShaderProgram> &inShader, const QDemonApplyImageValue &inCommand)
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
                   inEffect->className,
                   inCommand.m_imageName.constData());
        Q_ASSERT(false);
    }

    if (!inCommand.m_paramName.isEmpty()) {
        auto theConstant = inShader->shaderConstant(inCommand.m_paramName);

        if (theConstant) {
            if (inCommand.m_needSync) {
                QDemonRenderBufferBarrierFlags flags(QDemonRenderBufferBarrierValues::TextureFetch
                                                     | QDemonRenderBufferBarrierValues::TextureUpdate);
                inShader->renderContext()->setMemoryBarrier(flags);
            }

            if (theConstant->getShaderConstantType() == QDemonRenderShaderDataType::Image2D && !inCommand.m_bindAsTexture) {
                getEffectContext(*inEffect).setImage(inShader, inCommand.m_paramName, theImageToBind.image);
            } else if (theConstant->getShaderConstantType() == QDemonRenderShaderDataType::Texture2D && inCommand.m_bindAsTexture) {
                getEffectContext(*inEffect).setTexture(inShader, inCommand.m_paramName, theImageToBind.texture, false, m_textureStringBuilder, m_textureStringBuilder2);
            } else {
                qCCritical(INVALID_OPERATION,
                           "Effect %s: Binding buffer to parameter %s that is not a texture",
                           inEffect->className,
                           inCommand.m_paramName.constData());
                Q_ASSERT(false);
            }
        }
    }
}

void QDemonEffectSystem::applyDataBufferValue(QDemonRenderEffect *inEffect, const QDemonRef<QDemonRenderShaderProgram> &inShader, const QDemonApplyDataBufferValue &inCommand)
{
    QDemonAllocatedDataBufferEntry theBufferToBind;
    if (!inCommand.m_paramName.isEmpty()) {
        if (inEffect->m_context) {
            QDemonEffectContext &theContext(*inEffect->m_context);
            qint32 bufferIdx = theContext.findDataBuffer(inCommand.m_paramName);
            if (bufferIdx < theContext.m_allocatedDataBuffers.size()) {
                theBufferToBind = QDemonAllocatedDataBufferEntry(theContext.m_allocatedDataBuffers[bufferIdx]);
                if (theBufferToBind.needsClear) {
                    QDemonByteRef pData = theBufferToBind.dataBuffer->mapBuffer();
                    memset(pData.begin(), 0x0L, theBufferToBind.bufferData.size());
                    theBufferToBind.dataBuffer->unmapBuffer();
                    theBufferToBind.needsClear = false;
                }
            }
        }

        if (theBufferToBind.dataBuffer == nullptr) {
            qCCritical(INVALID_OPERATION,
                       "Effect %s: Failed to find buffer %s for bind",
                       inEffect->className,
                       inCommand.m_paramName.constData());
            Q_ASSERT(false);
        }

        auto theConstant = inShader->shaderBuffer(inCommand.m_paramName);

        if (theConstant) {
            getEffectContext(*inEffect).setDataBuffer(inShader, inCommand.m_paramName, theBufferToBind.dataBuffer);
        } else if (theBufferToBind.bufferType == QDemonRenderBufferType::DrawIndirect) {
            // since we filled part of this buffer on the GPU we need a sync before usage
            QDemonRenderBufferBarrierFlags flags(QDemonRenderBufferBarrierValues::CommandBuffer);
            inShader->renderContext()->setMemoryBarrier(flags);
        }
    }
}

void QDemonEffectSystem::applyRenderStateValue(QDemonRenderFrameBuffer *inTarget, const QDemonRef<QDemonRenderTexture2D> &inDepthStencilTexture, const QDemonApplyRenderState &theCommand)
{
    const auto &theContext(m_context->renderContext());
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

bool QDemonEffectSystem::compareDepthStencilState(QDemonRenderDepthStencilState &inState, QDemonDepthStencil &inStencil)
{
    QDemonRenderStencilFunction theFunction = inState.stencilFunction(QDemonRenderFace::Front);
    QDemonRenderStencilOperation theOperation = inState.stencilOperation(QDemonRenderFace::Front);

    return theFunction.m_function == inStencil.m_stencilFunction && theFunction.m_mask == inStencil.m_mask
            && theFunction.m_referenceValue == inStencil.m_reference && theOperation.m_stencilFail == inStencil.m_stencilFailOperation
            && theOperation.m_depthFail == inStencil.m_depthFailOperation
            && theOperation.m_depthPass == inStencil.m_depthPassOperation;
}

void QDemonEffectSystem::renderPass(QDemonEffectShader &inShader, const QMatrix4x4 &inMVP, const QDemonEffectTextureData &inSourceTexture, const QDemonRef<QDemonRenderFrameBuffer> &inFrameBuffer, QVector2D &inDestSize, const QVector2D &inCameraClipRange, const QDemonRef<QDemonRenderTexture2D> &inDepthStencil, QDemonOption<QDemonDepthStencil> inDepthStencilCommand, bool drawIndirect)
{
    const auto &theContext(m_context->renderContext());
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
                QDemonRenderStencilFunction theFunctionArg(theDepthStencil.m_stencilFunction,
                                                           theDepthStencil.m_reference,
                                                           theDepthStencil.m_mask);
                QDemonRenderStencilOperation theOpArg(theDepthStencil.m_stencilFailOperation,
                                                      theDepthStencil.m_depthFailOperation,
                                                      theDepthStencil.m_depthPassOperation);
                targetState = new QDemonRenderDepthStencilState(theContext,
                                                                theContext->isDepthTestEnabled(),
                                                                theContext->isDepthWriteEnabled(),
                                                                theContext->depthFunction(),
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
        inShader.m_appFrame.set((float)m_context->frameCount());
    if (inShader.m_fps.isValid())
        inShader.m_fps.set((float)m_context->getFPS().first);
    if (inShader.m_cameraClipRange.isValid())
        inShader.m_cameraClipRange.set(inCameraClipRange);

    if (!drawIndirect)
        m_context->renderer()->renderQuad();
    else
        m_context->renderer()->renderPointsIndirect();

    if (inDepthStencil && inFrameBuffer) {
        inFrameBuffer->attach(QDemonRenderFrameBufferAttachment::DepthStencil, QDemonRenderTextureOrRenderBuffer());
        theContext->setDepthStencilState(m_defaultStencilState);
    }
}

void QDemonEffectSystem::doRenderEffect(QDemonRenderEffect *inEffect,
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
    const auto &theContext = m_context->renderContext();

    // Context variables that are updated during the course of a pass.
    QDemonEffectTextureData theCurrentSourceTexture(inSourceTexture, false);
    QDemonRef<QDemonRenderTexture2D> theCurrentDepthStencilTexture;
    QDemonRef<QDemonRenderFrameBuffer> theCurrentRenderTarget(inTarget);
    QDemonRef<QDemonEffectShader> theCurrentShader;
    QRect theOriginalViewport(theContext->viewport());
    bool wasScissorEnabled = theContext->isScissorTestEnabled();
    bool wasBlendingEnabled = theContext->isBlendingEnabled();
    // save current blending setup
    QDemonRenderBlendFunctionArgument theBlendFunc = theContext->blendFunction();
    QDemonRenderBlendEquationArgument theBlendEqu = theContext->blendEquation();
    bool intermediateBlendingEnabled = false;
    QDemonTextureDetails theDetails(inSourceTexture->textureDetails());
    const qint32 theFinalWidth = theDetails.width;
    const qint32 theFinalHeight = theDetails.height;
    QVector2D theDestSize;
    {
        // Ensure no matter the command run goes we replace the rendering system to some
        // semblance of the approprate
        // setting.
        QDemonRenderContextScopedProperty<QDemonRef<QDemonRenderFrameBuffer>> __framebuffer(*theContext,
                                                                                            &QDemonRenderContext::renderTarget,
                                                                                            &QDemonRenderContext::setRenderTarget);
        QDemonRenderContextScopedProperty<QRect> __viewport(*theContext,
                                                            &QDemonRenderContext::viewport,
                                                            &QDemonRenderContext::setViewport);
        QDemonRenderContextScopedProperty<bool> __scissorEnabled(*theContext,
                                                                 &QDemonRenderContext::isScissorTestEnabled,
                                                                 &QDemonRenderContext::setScissorTestEnabled);
        QDemonRenderContextScopedProperty<bool> __stencilTest(*theContext,
                                                              &QDemonRenderContext::isStencilTestEnabled,
                                                              &QDemonRenderContext::setStencilTestEnabled);
        QDemonRenderContextScopedProperty<QDemonRenderBoolOp> __depthFunction(*theContext,
                                                                              &QDemonRenderContext::depthFunction,
                                                                              &QDemonRenderContext::setDepthFunction);
        QDemonOption<QDemonDepthStencil> theCurrentDepthStencil;

        theContext->setScissorTestEnabled(false);
        theContext->setBlendingEnabled(false);
        theContext->setCullingEnabled(false);
        theContext->setDepthTestEnabled(false);
        theContext->setDepthWriteEnabled(false);

        QMatrix4x4 theMVP;
        const auto &theCommands = inEffect->commands;
        for (const auto &theCommand : theCommands) {
            switch (theCommand->m_type) {
            case CommandType::AllocateBuffer:
                allocateBuffer(*inEffect,
                               static_cast<const QDemonAllocateBuffer &>(*theCommand),
                               theFinalWidth,
                               theFinalHeight,
                               theDetails.format);
                break;

            case CommandType::AllocateImage:
                allocateImage(*inEffect, static_cast<const QDemonAllocateImage &>(*theCommand), theFinalWidth, theFinalHeight);
                break;

            case CommandType::AllocateDataBuffer:
                allocateDataBuffer(*inEffect, static_cast<const QDemonAllocateDataBuffer &>(*theCommand));
                break;

            case CommandType::BindBuffer:
                theCurrentRenderTarget = bindBuffer(*inEffect, static_cast<const QDemonBindBuffer &>(*theCommand), theMVP, theDestSize);
                break;

            case CommandType::BindTarget: {
                m_context->renderContext()->setRenderTarget(inTarget);
                theCurrentRenderTarget = inTarget;
                theMVP = inMVP;
                theContext->setViewport(theOriginalViewport);
                theDestSize = QVector2D(float(theFinalWidth), float(theFinalHeight));
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
            case CommandType::BindShader:
                theCurrentShader = bindShader(inEffect->className, static_cast<const QDemonBindShader &>(*theCommand));
                break;
            case CommandType::ApplyInstanceValue:
                if (theCurrentShader)
                    applyInstanceValue(inEffect,
                                       inClass,
                                       theCurrentShader->m_shader,
                                       static_cast<const QDemonApplyInstanceValue &>(*theCommand));
                break;
            case CommandType::ApplyValue:
                if (theCurrentShader)
                    applyValue(inEffect, inClass, theCurrentShader->m_shader, static_cast<const QDemonApplyValue &>(*theCommand));
                break;
            case CommandType::ApplyBlending:
                intermediateBlendingEnabled = applyBlending(static_cast<const QDemonApplyBlending &>(*theCommand));
                break;
            case CommandType::ApplyBufferValue:
                if (theCurrentShader)
                    theCurrentSourceTexture = applyBufferValue(inEffect,
                                                               theCurrentShader->m_shader,
                                                               static_cast<const QDemonApplyBufferValue &>(*theCommand),
                                                               inSourceTexture,
                                                               theCurrentSourceTexture);
                break;
            case CommandType::ApplyDepthValue:
                if (theCurrentShader)
                    applyDepthValue(inEffect, theCurrentShader->m_shader, static_cast<const QDemonApplyDepthValue &>(*theCommand), inDepthTexture);
                if (!inDepthTexture) {
                    qCCritical(INVALID_OPERATION,
                               "Depth value command detected but no "
                               "depth buffer provided for effect %s",
                               inEffect->className);
                    Q_ASSERT(false);
                }
                break;
            case CommandType::ApplyImageValue:
                if (theCurrentShader)
                    applyImageValue(inEffect, theCurrentShader->m_shader, static_cast<const QDemonApplyImageValue &>(*theCommand));
                break;
            case CommandType::ApplyDataBufferValue:
                if (theCurrentShader)
                    applyDataBufferValue(inEffect,
                                         theCurrentShader->m_shader,
                                         static_cast<const QDemonApplyDataBufferValue &>(*theCommand));
                break;
            case CommandType::DepthStencil: {
                const QDemonDepthStencil &theDepthStencil = static_cast<const QDemonDepthStencil &>(*theCommand);
                theCurrentDepthStencilTexture = findTexture(inEffect, theDepthStencil.m_bufferName);
                if (theCurrentDepthStencilTexture)
                    theCurrentDepthStencil = theDepthStencil;
            } break;
            case CommandType::Render:
                if (theCurrentShader && theCurrentSourceTexture.texture) {
                    renderPass(*theCurrentShader,
                               theMVP,
                               theCurrentSourceTexture,
                               theCurrentRenderTarget,
                               theDestSize,
                               inCameraClipRange,
                               theCurrentDepthStencilTexture,
                               theCurrentDepthStencil,
                               static_cast<const QDemonRender &>(*theCommand).m_drawIndirect);
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
            case CommandType::ApplyRenderState:
                applyRenderStateValue(theCurrentRenderTarget.data(),
                                      inDepthStencilTexture,
                                      static_cast<const QDemonApplyRenderState &>(*theCommand));
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

QDemonRef<QDemonRenderTexture2D> QDemonEffectSystem::renderEffect(QDemonEffectRenderArgument inRenderArgument)
{
    auto theClass = getEffectClass(inRenderArgument.m_effect->className);
    if (!theClass) {
        Q_ASSERT(false);
        return nullptr;
    }
    QMatrix4x4 theMVP;
    QDemonRenderCamera::setupOrthographicCameraForOffscreenRender(*inRenderArgument.m_colorBuffer, theMVP);
    // setup a render target
    const auto &theContext(m_context->renderContext());
    const auto &theManager(m_context->resourceManager());
    QDemonRenderContextScopedProperty<QDemonRef<QDemonRenderFrameBuffer>> __framebuffer(*theContext,
                                                                                        &QDemonRenderContext::renderTarget,
                                                                                        &QDemonRenderContext::setRenderTarget);
    QDemonTextureDetails theDetails(inRenderArgument.m_colorBuffer->textureDetails());
    quint32 theFinalWidth = QDemonRendererUtil::nextMultipleOf4((quint32)(theDetails.width));
    quint32 theFinalHeight = QDemonRendererUtil::nextMultipleOf4((quint32)(theDetails.height));
    auto theBuffer = theManager->allocateFrameBuffer();
    // UdoL Some Effects may need to run before HDR tonemap. This means we need to keep the
    // input format
    QDemonRenderTextureFormat theOutputFormat = QDemonRenderTextureFormat::RGBA8;
    // TODO:
    //        if (theClass->dynamicClass->getOutputTextureFormat() == QDemonRenderTextureFormat::Unknown)
    //            theOutputFormat = theDetails.format;
    auto theTargetTexture = theManager->allocateTexture2D(theFinalWidth, theFinalHeight, theOutputFormat);
    theBuffer->attach(QDemonRenderFrameBufferAttachment::Color0, theTargetTexture);
    theContext->setRenderTarget(theBuffer);
    QDemonRenderContextScopedProperty<QRect> __viewport(*theContext,
                                                        &QDemonRenderContext::viewport,
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
                   m_context->renderContext()->renderTarget(),
                   false,
                   inRenderArgument.m_depthTexture,
                   inRenderArgument.m_depthStencilBuffer,
                   inRenderArgument.m_cameraClipRange);

    theBuffer->attach(QDemonRenderFrameBufferAttachment::Color0, QDemonRenderTextureOrRenderBuffer());
    theManager->release(theBuffer);
    return theTargetTexture;
}

bool QDemonEffectSystem::renderEffect(QDemonEffectRenderArgument inRenderArgument, QMatrix4x4 &inMVP, bool inEnableBlendWhenRenderToTarget)
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
                   m_context->renderContext()->renderTarget(),
                   inEnableBlendWhenRenderToTarget,
                   inRenderArgument.m_depthTexture,
                   inRenderArgument.m_depthStencilBuffer,
                   inRenderArgument.m_cameraClipRange);
    return true;
}

void QDemonEffectSystem::releaseEffectContext(QDemonEffectContext *inContext)
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

void QDemonEffectSystem::resetEffectFrameData(QDemonEffectContext &inContext)
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

void QDemonEffectSystem::setShaderData(const QByteArray &path, const char *data, const char *inShaderType, const char *inShaderVersion, bool inHasGeomShader, bool inIsComputeShader)
{
    m_context->dynamicObjectSystem()->setShaderData(path, data, inShaderType, inShaderVersion, inHasGeomShader, inIsComputeShader);
}

void QDemonEffectSystem::init()
{
    const auto &theContext(m_context->renderContext());

    m_resourceManager = m_context->resourceManager();

    // create default stencil state
    QDemonRenderStencilFunction stencilDefaultFunc(QDemonRenderBoolOp::AlwaysTrue, 0x0, 0xFF);
    QDemonRenderStencilOperation stencilDefaultOp(QDemonRenderStencilOp::Keep,
                                                  QDemonRenderStencilOp::Keep,
                                                  QDemonRenderStencilOp::Keep);
    m_defaultStencilState = new QDemonRenderDepthStencilState(theContext,
                                                              theContext->isDepthTestEnabled(),
                                                              theContext->isDepthWriteEnabled(),
                                                              theContext->depthFunction(),
                                                              theContext->isStencilTestEnabled(),
                                                              stencilDefaultFunc,
                                                              stencilDefaultFunc,
                                                              stencilDefaultOp,
                                                              stencilDefaultOp);
}

QDemonRef<QDemonResourceManager> QDemonEffectSystem::getResourceManager() { return m_resourceManager; }

QDemonEffectTextureData::QDemonEffectTextureData(const QDemonRef<QDemonRenderTexture2D> &inTexture, bool inNeedsMultiply)
    : texture(inTexture), needsAlphaMultiply(inNeedsMultiply)
{
}

QT_END_NAMESPACE
