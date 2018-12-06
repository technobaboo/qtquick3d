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

#include <qdemonrendergraphobjectserializer.h>
#include <QtDemonRuntimeRender/qdemonrenderpresentation.h>
#include <QtDemonRuntimeRender/qdemonrendernode.h>
#include <QtDemonRuntimeRender/qdemonrenderscene.h>
#include <QtDemonRuntimeRender/qdemonrenderlayer.h>
#include <QtDemonRuntimeRender/qdemonrendermodel.h>
#include <QtDemonRuntimeRender/qdemonrendertext.h>
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterial.h>
#include <QtDemonRuntimeRender/qdemonrenderimage.h>
#include <QtDemonRuntimeRender/qdemonrendereffect.h>
#include <QtDemonRuntimeRender/qdemonrendercamera.h>
#include <QtDemonRuntimeRender/qdemonrenderlight.h>
#include <QtDemonRuntimeRender/qdemonrendercustommaterial.h>
#include <Qt3DSFoundation.h>
#include <Qt3DSBroadcastingAllocator.h>
#include <Qt3DSContainers.h>
#include <qdemonrendereffectsystem.h>
#include <SerializationTypes.h>
#include <QtDemonRuntimeRender/qdemonrenderstring.h>
#include <FileTools.h>
#include <qdemonrenderplugingraphobject.h>
#include <QtDemonRuntimeRender/qdemonrenderreferencedmaterial.h>
#include <QtDemonRuntimeRender/qdemonrenderpath.h>
#include <QtDemonRuntimeRender/qdemonrenderpathsubpath.h>
#include <qdemonrenderpathmanager.h>


QT_BEGIN_NAMESPACE

using namespace dynamic;

namespace {
typedef nvhash_set<void *> TPtrSet;

void Align(MemoryBuffer<> &inBuffer)
{
    inBuffer.align(sizeof(void *));
}
typedef nvvector<eastl::pair<GraphObjectTypes::Enum, quint32>> TObjectFileStatList;
typedef SPtrOffsetMap TPtrOffsetMap;

struct SSerializerWriteContext
{
    SPtrOffsetMap &m_OffsetMap;
    SWriteBuffer &m_MemoryBuffer;
    const SStrRemapMap &m_StrRemapMap;
    quint32 m_DataBlockStart;
    IDynamicObjectSystem &m_DynamicObjectSystem;
    IPathManager &m_PathManager;
    TObjectFileStatList &m_FileSizeStats;
    CRenderString m_PathMapper;
    CRenderString m_BasePath;
    CRenderString m_RelativePath;
    IStringTable &m_StringTable;
    SSerializerWriteContext(SPtrOffsetMap &inOffsetMap, SWriteBuffer &inWriteBuffer,
                            const SStrRemapMap &inStrMap, quint32 inDataBlockStart,
                            IDynamicObjectSystem &inDynamicObjectSystem,
                            IPathManager &inPathManager, TObjectFileStatList &inStats,
                            NVAllocatorCallback &inAllocator, const char8_t *inProjectDirectory,
                            IStringTable &inStringTable)
        : m_OffsetMap(inOffsetMap)
        , m_MemoryBuffer(inWriteBuffer)
        , m_StrRemapMap(inStrMap)
        , m_DataBlockStart(inDataBlockStart)
        , m_DynamicObjectSystem(inDynamicObjectSystem)
        , m_PathManager(inPathManager)
        , m_FileSizeStats(inStats)
        , m_StringTable(inStringTable)
    {
        Q_UNUSED(inAllocator)
        m_BasePath.assign(inProjectDirectory);
    }

    bool HasWrittenObject(const void *inObject) { return m_OffsetMap.contains(inObject); }

    quint32 &GetStatEntry(GraphObjectTypes::Enum inType) const
    {
        for (quint32 idx = 0, end = m_FileSizeStats.size(); idx < end; ++idx)
            if (m_FileSizeStats[idx].first == inType)
                return m_FileSizeStats[idx].second;
        m_FileSizeStats.push_back(eastl::make_pair(inType, (quint32)0));
        return m_FileSizeStats.back().second;
    }

    template <typename TObjType>
    void AddPtrOffset(const TObjType *inObject)
    {
        quint32 objOffset = m_MemoryBuffer.size() - m_DataBlockStart;
        m_OffsetMap.insert(eastl::make_pair(inObject, objOffset));
// In debug we keep stats on how much each type of object
// contributes to the file size.
#ifdef _DEBUG
        GetStatEntry(inObject->m_Type) += sizeof(TObjType);
#endif
    }

    void Remap(CRegisteredString &inStr) { inStr.Remap(m_StrRemapMap); }

    template <typename TObjType>
    void Remap(TObjType *&inPtr)
    {
        if (inPtr) {
            TPtrOffsetMap::iterator theIter = m_OffsetMap.find(inPtr);
            if (theIter != m_OffsetMap.end())
                inPtr = reinterpret_cast<TObjType *>(theIter->second);
            else {
                Q_ASSERT(false);
            }
        }
    }

    void RemapMaterial(SGraphObject *&inPtr) { Remap(inPtr); }

    template <typename TObjType>
    void NullPtr(TObjType *&inPtr)
    {
        inPtr = nullptr;
    }
};

///////////////////////////////////////////////////////////////////////
// --** Reading the scene graph is heavily threaded when we are loading
//		multiple presentations in one application --**
///////////////////////////////////////////////////////////////////////
struct SSerializerReadContext : public SDataReader
{
    IPathManagerCore &m_PathManager;
    IDynamicObjectSystemCore &m_DynamicObjectSystem;
    QDemonDataRef<quint8> m_DataBlock;
    QDemonDataRef<quint8> m_StrTableBlock;
    CRenderString m_PathMapper;
    const char8_t *m_ProjectDirectory;

    SSerializerReadContext(IPathManagerCore &inPathManager, IDynamicObjectSystemCore &inDynSystem,
                           QDemonDataRef<quint8> inDataBlock, QDemonDataRef<quint8> inStrTable,
                           NVAllocatorCallback &inAllocator, const char8_t *inProjectDirectory)
        : SDataReader(inDataBlock.begin(), inDataBlock.end())
        , m_PathManager(inPathManager)
        , m_DynamicObjectSystem(inDynSystem)
        , m_DataBlock(inDataBlock)
        , m_StrTableBlock(inStrTable)
        , m_ProjectDirectory(inProjectDirectory)
    {
        Q_UNUSED(inAllocator)
    }
    void Remap(CRegisteredString &inStr) { inStr.Remap(m_StrTableBlock); }
    template <typename TObjType>
    void Remap(TObjType *&inPtr)
    {
        if (inPtr) {
            TObjType *purePtr = inPtr;
            size_t ptrValue = reinterpret_cast<size_t>(purePtr);
            if (ptrValue < m_DataBlock.size())
                inPtr = reinterpret_cast<TObjType *>(m_DataBlock.begin() + ptrValue);
            else {
                Q_ASSERT(false);
                inPtr = nullptr;
            }
        }
    }
    void RemapMaterial(SGraphObject *&inPtr) { Remap(inPtr); }
    // Nulling out pointers was done on write, so we don't do it here.
    template <typename TObjType>
    void NullPtr(TObjType *&)
    {
    }
};

template <typename TObjType>
struct SGraphObjectSerializerImpl
{
    static TObjType *Write(const TObjType &ioObject, SSerializerWriteContext &outSavedBuffer);
    static void Remap(TObjType &inObject, SSerializerWriteContext &inRemapContext);
    static TObjType *Read(SSerializerReadContext &inReadContext);
};

struct SWriteRemapper
{
    SSerializerWriteContext &m_WriteBuffer;
    SWriteRemapper(SSerializerWriteContext &buf)
        : m_WriteBuffer(buf)
    {
    }
    // This will happen later
    void Remap(const CRegisteredString &) {}
    void RemapPath(const CRegisteredString &) {}

    // We ignore objects that are saved out explicitly below.
    void Remap(const SScene *) {}

    void Remap(const SLayer *) {}
    // Nodes are ignored because we save them out *not* in depth first order,
    // with models, text, lights and camera saved out contiguously for in-memory
    // traversal.
    void Remap(const SNode *) {}
#ifdef _INTEGRITYPLATFORM
    // explicit specialization of class "<unnamed>::SGraphObjectSerializerImpl<SCustomMaterial>"
    // must precede its first use struct SGraphObjectSerializerImpl<SCustomMaterial>
    template <typename TObjType>
    void Remap(const TObjType *inObj);
#else
    template <typename TObjType>
    void Remap(const TObjType *inObj)
    {
        if (inObj)
            SGraphObjectSerializerImpl<TObjType>::Write(*inObj, m_WriteBuffer);
    }
#endif

    void RemapMaterial(const SGraphObject *inObj)
    {
        if (inObj) {
            if (inObj->m_Type == GraphObjectTypes::DefaultMaterial)
                Remap(static_cast<const SDefaultMaterial *>(inObj));
            else if (inObj->m_Type == GraphObjectTypes::CustomMaterial)
                Remap(static_cast<const SCustomMaterial *>(inObj));
            else if (inObj->m_Type == GraphObjectTypes::ReferencedMaterial)
                Remap(static_cast<const SReferencedMaterial *>(inObj));
            else {
                Q_ASSERT(false);
            }
        }
    }
    template <typename TObjType>
    void NullPtr(const TObjType *)
    {
    }
};

void PrepareFirstPass(const SNode &inNode, nvvector<const SNode *> &ioLightCameras,
                      nvvector<const SNode *> &ioRenderable)
{
    if (GraphObjectTypes::IsRenderableType(inNode.m_Type))
        ioRenderable.push_back(&inNode);
    else if (GraphObjectTypes::IsLightCameraType(inNode.m_Type))
        ioLightCameras.push_back(&inNode);

    for (const SNode *theChild = inNode.m_FirstChild; theChild; theChild = theChild->m_NextSibling)
        PrepareFirstPass(*theChild, ioLightCameras, ioRenderable);
}

template <typename TObject>
TObject *WriteGenericGraphObjectNoRemap(const TObject &ioObject,
                                        SSerializerWriteContext &outSavedBuffer)
{
    if (outSavedBuffer.HasWrittenObject(&ioObject))
        return nullptr;

    outSavedBuffer.AddPtrOffset(&ioObject);
    quint32 theOffset = outSavedBuffer.m_MemoryBuffer.size();
    outSavedBuffer.m_MemoryBuffer.write(ioObject);
    // Probably the buffer stays aligned but we want to work to keep it that way.
    Align(outSavedBuffer.m_MemoryBuffer);
    return reinterpret_cast<TObject *>(outSavedBuffer.m_MemoryBuffer.begin() + theOffset);
}

template <typename TObject>
TObject *WriteGenericGraphObject(const TObject &ioObject, SSerializerWriteContext &outSavedBuffer)
{
    TObject *theObject = WriteGenericGraphObjectNoRemap(ioObject, outSavedBuffer);
    if (theObject) // The object may have already been written.
    {
        // Write mappers just follow pointers and ensure all the associated objects
        // are written out.
        SWriteRemapper theWriteRemapper(outSavedBuffer);
        const_cast<TObject &>(ioObject).Remap(theWriteRemapper);
    }
    return theObject;
}

template <typename TObject>
TObject *ReadGenericGraphObject(SSerializerReadContext &inReadContext)
{
    TObject *retval = inReadContext.Load<TObject>();
    inReadContext.Align();
    if (retval) {
        retval->Remap(inReadContext);
    }
    return retval;
}

template <typename TObjType>
TObjType *SGraphObjectSerializerImpl<TObjType>::Write(const TObjType &ioObject,
                                                      SSerializerWriteContext &outSavedBuffer)
{
    return WriteGenericGraphObject(ioObject, outSavedBuffer);
}
template <typename TObjType>
void SGraphObjectSerializerImpl<TObjType>::Remap(TObjType &ioObject,
                                                 SSerializerWriteContext &inRemapContext)
{
    return ioObject.Remap(inRemapContext);
}

template <typename TObjType>
TObjType *SGraphObjectSerializerImpl<TObjType>::Read(SSerializerReadContext &inReadContext)
{
    return ReadGenericGraphObject<TObjType>(inReadContext);
}

void RemapProperties(SDynamicObject &ioObject, SSerializerWriteContext &outSavedBuffer,
                     CRegisteredString inClassName)
{
    QDemonConstDataRef<SPropertyDefinition> theObjectProps =
        outSavedBuffer.m_DynamicObjectSystem.GetProperties(inClassName);
    for (quint32 idx = 0, end = theObjectProps.size(); idx < end; ++idx) {
        const SPropertyDefinition &theDef(theObjectProps[idx]);
        if (theDef.m_DataType == QDemonRenderShaderDataTypes::QDemonRenderTexture2DPtr) {
            CRegisteredString *theStr = reinterpret_cast<CRegisteredString *>(
                ioObject.GetDataSectionBegin() + theDef.m_Offset);
            outSavedBuffer.Remap(*theStr);
        }
    }
}

void RemapProperties(SDynamicObject &ioObject, SSerializerReadContext &inReadContext)
{
    // CN - !!Note this call is done on multiple threads simultaneously.  I added a mutex just to be
    // sure even though
    // this is a read-only call; I am not certain how good the arm memory locking is when it is
    // completely unprotected.
    QDemonConstDataRef<SPropertyDefinition> theProperties =
        inReadContext.m_DynamicObjectSystem.GetProperties(ioObject.m_ClassName);
    for (quint32 idx = 0, end = theProperties.size(); idx < end; ++idx) {
        const SPropertyDefinition &theDefinition(theProperties[idx]);
        if (theDefinition.m_DataType == QDemonRenderShaderDataTypes::QDemonRenderTexture2DPtr) {
            CRegisteredString *theString = reinterpret_cast<CRegisteredString *>(
                ioObject.GetDataSectionBegin() + theDefinition.m_Offset);
            inReadContext.Remap(*theString);
        }
    }
}

template <>
struct SGraphObjectSerializerImpl<SEffect>
{
    static SGraphObject *Write(const SEffect &ioObject, SSerializerWriteContext &outSavedBuffer)
    {
        size_t itemOffset = outSavedBuffer.m_MemoryBuffer.size();
        SEffect *theNewEffect =
            static_cast<SEffect *>(WriteGenericGraphObjectNoRemap(ioObject, outSavedBuffer));
        if (theNewEffect) {
            theNewEffect->m_Context = nullptr;
            // Writing it out is easy.  Reading it back in means we have to have a correctly setup
            // IEffectManager so we
            // can remap strings.
            outSavedBuffer.m_MemoryBuffer.write(ioObject.GetDataSectionBegin(),
                                                ioObject.m_DataSectionByteSize);
            Align(outSavedBuffer.m_MemoryBuffer);
            SWriteRemapper theWriteRemapper(outSavedBuffer);
            // Write any connected objects.
            theNewEffect =
                reinterpret_cast<SEffect *>(outSavedBuffer.m_MemoryBuffer.begin() + itemOffset);
            theNewEffect->Remap(theWriteRemapper);
        }
        return theNewEffect;
    }

    static void Remap(SEffect &ioObject, SSerializerWriteContext &outSavedBuffer)
    {
        CRegisteredString theClassName = ioObject.m_ClassName;
        ioObject.Remap(outSavedBuffer);
        RemapProperties(ioObject, outSavedBuffer, theClassName);
    }

    static SEffect *Read(SSerializerReadContext &inReadContext)
    {
        SEffect *theEffect = ReadGenericGraphObject<SEffect>(inReadContext);
        if (theEffect) {
            inReadContext.m_CurrentPtr += theEffect->m_DataSectionByteSize;
            inReadContext.Align();
            RemapProperties(*theEffect, inReadContext);
        }
        return theEffect;
    }
};

template <>
struct SGraphObjectSerializerImpl<SCustomMaterial>
{
    static SGraphObject *Write(const SCustomMaterial &ioObject,
                               SSerializerWriteContext &outSavedBuffer)
    {
        size_t itemOffset = outSavedBuffer.m_MemoryBuffer.size();
        SCustomMaterial *theNewObject = static_cast<SCustomMaterial *>(
            WriteGenericGraphObjectNoRemap(ioObject, outSavedBuffer));
        if (theNewObject) {
            // Writing it out is easy.  Reading it back in means we have to have a correctly setup
            // IEffectManager so we
            // can remap strings.
            outSavedBuffer.m_MemoryBuffer.write(ioObject.GetDataSectionBegin(),
                                                ioObject.m_DataSectionByteSize);
            Align(outSavedBuffer.m_MemoryBuffer);
            theNewObject = reinterpret_cast<SCustomMaterial *>(outSavedBuffer.m_MemoryBuffer.begin()
                                                               + itemOffset);
            SWriteRemapper theWriteRemapper(outSavedBuffer);
            // Write any connected objects.
            theNewObject->Remap(theWriteRemapper);
        }
        return theNewObject;
    }

    static void Remap(SCustomMaterial &ioObject, SSerializerWriteContext &outSavedBuffer)
    {
        CRegisteredString theClassName(ioObject.m_ClassName);
        ioObject.Remap(outSavedBuffer);
        RemapProperties(ioObject, outSavedBuffer, theClassName);
    }

    static SCustomMaterial *Read(SSerializerReadContext &inReadContext)
    {
        SCustomMaterial *theMaterial = ReadGenericGraphObject<SCustomMaterial>(inReadContext);
        if (theMaterial) {
            inReadContext.m_CurrentPtr += theMaterial->m_DataSectionByteSize;
            inReadContext.Align();
            RemapProperties(*theMaterial, inReadContext);
        }
        return theMaterial;
    }
};

#ifdef _INTEGRITYPLATFORM
    template <typename TObjType>
    void SWriteRemapper::Remap(const TObjType *inObj)
    {
        if (inObj)
            SGraphObjectSerializerImpl<TObjType>::Write(*inObj, m_WriteBuffer);
    }
#endif

template <>
struct SGraphObjectSerializerImpl<SPathSubPath>
{
    static SGraphObject *Write(const SPathSubPath &ioObject,
                               SSerializerWriteContext &outSavedBuffer)
    {
        SPathSubPath *theObject = WriteGenericGraphObjectNoRemap(ioObject, outSavedBuffer);
        if (theObject) // The object may have already been written.
        {
            QDemonConstDataRef<SPathAnchorPoint> thePoints =
                outSavedBuffer.m_PathManager.GetPathSubPathBuffer(ioObject);
            outSavedBuffer.m_MemoryBuffer.write((quint32)thePoints.size());
            outSavedBuffer.m_MemoryBuffer.write(thePoints.begin(), thePoints.size());
            // Write mappers just follow pointers and ensure all the associated objects
            // are written out.
            SWriteRemapper theWriteRemapper(outSavedBuffer);
            const_cast<SPathSubPath &>(ioObject).Remap(theWriteRemapper);
        }
        return theObject;
    }

    static void Remap(SPathSubPath &ioObject, SSerializerWriteContext &outSavedBuffer)
    {
        ioObject.Remap(outSavedBuffer);
    }

    static SPathSubPath *Read(SSerializerReadContext &inReadContext)
    {
        SPathSubPath *theSubPath = ReadGenericGraphObject<SPathSubPath>(inReadContext);
        if (theSubPath) {
            quint32 numPoints = *inReadContext.Load<quint32>();
            SPathAnchorPoint *theAnchorPointBuffer =
                reinterpret_cast<SPathAnchorPoint *>(inReadContext.m_CurrentPtr);
            inReadContext.m_CurrentPtr += sizeof(SPathAnchorPoint) * numPoints;

            // CN - !!Note this call is done on multiple threads simultaneously.  I added a mutex to
            // the path manager object
            // so this exact call is always protected.  This absolutely caused crashing when it was
            // not protected approriately.
            inReadContext.m_PathManager.SetPathSubPathData(
                *theSubPath, toConstDataRef(theAnchorPointBuffer, numPoints));
        }
        return theSubPath;
    }
};

void WriteGraphObject(const SGraphObject &inObject, SSerializerWriteContext &outSavedBuffer)
{
    SGraphObject *newObject = nullptr;
    switch (inObject.m_Type) {
#define QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE(type)                                                   \
    case GraphObjectTypes::type:                                                                   \
        newObject = SGraphObjectSerializerImpl<S##type>::Write(                                    \
            static_cast<const S##type &>(inObject), outSavedBuffer);                               \
        break;
        QDEMON_RENDER_ITERATE_GRAPH_OBJECT_TYPES
#undef QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE
    default:
        Q_ASSERT(false);
        break;
    }
}

void WriteNodeList(nvvector<const SNode *> &inList, SSerializerWriteContext &outSavedBuffer)
{
    for (quint32 idx = 0, end = inList.size(); idx < end; ++idx)
        WriteGraphObject(*inList[idx], outSavedBuffer);
}

// Now write everything you haven't written so far, skip writing renderables or cameras or lights
void WriteNonRenderableNonCLNode(const SNode &inNode, SSerializerWriteContext &outSavedBuffer)
{
    if (GraphObjectTypes::IsLightCameraType(inNode.m_Type) == false
        && GraphObjectTypes::IsRenderableType(inNode.m_Type) == false) {
        WriteGraphObject(inNode, outSavedBuffer);
    }
    for (const SNode *theChild = inNode.m_FirstChild; theChild; theChild = theChild->m_NextSibling)
        WriteNonRenderableNonCLNode(*theChild, outSavedBuffer);
}

SGraphObject *ReadGraphObject(SSerializerReadContext &inContext)
{
    if (inContext.m_CurrentPtr + sizeof(SGraphObject) < inContext.m_EndPtr) {
        SGraphObject *theObject = reinterpret_cast<SGraphObject *>(inContext.m_CurrentPtr);
        switch (theObject->m_Type) {
#define QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE(type)                                                   \
    case GraphObjectTypes::type:                                                                   \
        SGraphObjectSerializerImpl<S##type>::Read(inContext);                                      \
        break;
            QDEMON_RENDER_ITERATE_GRAPH_OBJECT_TYPES
#undef QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE
        default:
            Q_ASSERT(false);
            theObject = nullptr;
            break;
        }
        return theObject;
    }
    return nullptr;
}
}

void SGraphObjectSerializer::Save(NVFoundationBase &inFoundation,
                                  const SPresentation &inPresentation,
                                  SWriteBuffer &outSavedData,
                                  IDynamicObjectSystem &inDynamicObjectSystem,
                                  IPathManager &inPathManager, SPtrOffsetMap &outSceneGraphOffsets,
                                  IStringTable &inStringTable,
                                  QDemonDataRef<SGraphObject *> inExtraGraphObjects)
{
    using namespace foundation;
    nvvector<const SNode *> theLightCameraList(inFoundation.getAllocator(),
                                               "SGraphObjectSerializer::theLightCameraList");
    nvvector<const SNode *> theRenderableList(inFoundation.getAllocator(),
                                              "SGraphObjectSerializer::theRenderableList");
    TObjectFileStatList theStatList(inFoundation.getAllocator(),
                                    "SGraphObjectSerializer::FileSizeStats");
    // We want to save out the scene graph in the order we are going to traverse it normally.
    // This is reverse depth first for the lights, cameras, and renderables and depth first for
    // everything else so we go
    // in two passes per layer.
    // We expect the incoming data buffer to be aligned already.
    Q_ASSERT(outSavedData.size() % 4 == 0);
    if (inPresentation.m_Scene) {
        quint32 theDataSectionStart = outSavedData.size();
        outSavedData.writeZeros(4);
        SSerializerWriteContext theWriteContext(
            outSceneGraphOffsets, outSavedData, inStringTable.GetRemapMap(), theDataSectionStart,
            inDynamicObjectSystem, inPathManager, theStatList, inFoundation.getAllocator(),
            inPresentation.m_PresentationDirectory, inStringTable);
        // First pass, just write out the data.
        WriteGraphObject(inPresentation, theWriteContext);
        WriteGraphObject(*inPresentation.m_Scene, theWriteContext);
        for (const SLayer *theLayer = inPresentation.m_Scene->m_FirstChild; theLayer;
             theLayer = static_cast<const SLayer *>(theLayer->m_NextSibling)) {
            theLightCameraList.clear();
            theRenderableList.clear();
            PrepareFirstPass(*theLayer, theLightCameraList, theRenderableList);
            eastl::reverse(theLightCameraList.begin(), theLightCameraList.end());
            eastl::reverse(theRenderableList.begin(), theRenderableList.end());
            WriteNodeList(theLightCameraList, theWriteContext);
            WriteNodeList(theRenderableList, theWriteContext);
        }
        // Now just write everything *but* renderable objects and cameras.
        for (const SLayer *theLayer = inPresentation.m_Scene->m_FirstChild; theLayer;
             theLayer = static_cast<const SLayer *>(theLayer->m_NextSibling)) {
            WriteNonRenderableNonCLNode(*theLayer, theWriteContext);
        }
        // Write out any extra objects we haven't covered yet.
        for (quint32 idx = 0, end = inExtraGraphObjects.size(); idx < end; ++idx)
            WriteGraphObject(*inExtraGraphObjects[idx], theWriteContext);

        quint32 theNumObjects = theWriteContext.m_OffsetMap.size();
        quint32 *theCountPtr = reinterpret_cast<quint32 *>(outSavedData.begin() + theDataSectionStart);
        *theCountPtr = theNumObjects;

        // Second pass, perform remapping on all the objects to change their pointers to offsets
        for (SPtrOffsetMap::iterator theIter = outSceneGraphOffsets.begin(),
                                     theEnd = outSceneGraphOffsets.end();
             theIter != theEnd; ++theIter) {
            quint8 *theDataPtr = outSavedData.begin() + theDataSectionStart + theIter->second;
            SGraphObject *theGraphObj = reinterpret_cast<SGraphObject *>(theDataPtr);
            switch (theGraphObj->m_Type) {
#define QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE(type)                                                   \
    case GraphObjectTypes::type:                                                                   \
        SGraphObjectSerializerImpl<S##type>::Remap(static_cast<S##type &>(*theGraphObj),           \
                                                   theWriteContext);                               \
        break;
                QDEMON_RENDER_ITERATE_GRAPH_OBJECT_TYPES
#undef QDEMON_RENDER_HANDL_GRAPH_OBJECT_TYPE
            default:
                Q_ASSERT(false);
                break;
            }
        }
    }
#ifdef _DEBUG
    qCDebug(TRACE_INFO, "--File size stats:");
    // Tell the users how much space is used in the file based on object type:
    for (quint32 idx = 0, end = theStatList.size(); idx < end; ++idx) {
        const char *theObjType = GraphObjectTypes::GetObjectTypeName(theStatList[idx].first);
        qCDebug(TRACE_INFO, "%s - %d bytes:", theObjType, theStatList[idx].second);
    }
    qCDebug(TRACE_INFO, "--End file size stats:");
#endif
};

SPresentation *SGraphObjectSerializer::Load(QDemonDataRef<quint8> inData, QDemonDataRef<quint8> inStrDataBlock,
                                            IDynamicObjectSystemCore &inDynamicObjectSystem,
                                            IPathManagerCore &inPathManager,
                                            NVAllocatorCallback &inAllocator,
                                            const char8_t *inProjectDirectory)
{
    SSerializerReadContext theReadContext(inPathManager, inDynamicObjectSystem, inData,
                                          inStrDataBlock, inAllocator, inProjectDirectory);
    SPresentation *retval = nullptr;
    if (inData.size() < 4) {
        Q_ASSERT(false);
        return nullptr;
    }
    quint32 theNumObjects = theReadContext.LoadRef<quint32>();
    for (quint32 idx = 0, end = theNumObjects; idx < end; ++idx) {
        SGraphObject *theObject = ReadGraphObject(theReadContext);
        if (theObject) {
            if (theObject->m_Type == GraphObjectTypes::Presentation)
                retval = static_cast<SPresentation *>(theObject);
        } else {
            Q_ASSERT(false);
        }
    }
    return retval;
}

QT_END_NAMESPACE
