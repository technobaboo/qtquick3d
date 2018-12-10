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
#ifdef _WIN32
#pragma warning(disable : 4201) // nonstandard extension used : nameless struct/union
#endif

#include <QtDemonRuntimeRender/qdemonrenderbuffermanager.h>
#include <Qt3DSAllocator.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <Qt3DSAtomic.h>
#include <EASTL/hash_map.h>
#include <FileTools.h>
#include <Qt3DSImportMesh.h>
#include <QtDemonRuntimeRender/qdemonrendermesh.h>
#include <Qt3DSAllocatorCallback.h>
#include <QtDemonRuntimeRender/qdemonrenderloadedtexture.h>
#include <Qt3DSFoundation.h>
#include <qdemonrenderinputstreamfactory.h>
#include <qdemonrenderimagescaler.h>
#include <qdemontextrenderer.h>
#include <Qt3DSPerfTimer.h>
#include <Qt3DSMutex.h>
#include <QtDemonRuntimeRender/qdemonrenderprefiltertexture.h>
#include <QtCore/qdir.h>

QT_BEGIN_NAMESPACE

namespace {

typedef eastl::basic_string<char, ForwardingAllocator> TStr;
struct StrHasher
{
    size_t operator()(const TStr &str) const
    {
        return hash<const char *>()((const char *)str.c_str());
    }
};

struct StrEq
{
    bool operator()(const TStr &lhs, const TStr &rhs) const { return lhs == rhs; }
};

struct SImageEntry : public SImageTextureData
{
    bool m_Loaded;
    SImageEntry()
        : m_Loaded(false)
    {
    }
    ~SImageEntry()
    {
        // if (m_BSDFMipMap)
        //     m_BSDFMipMap->release();
    }
};

struct SPrimitiveEntry
{
    // Name of the primitive as it will be in the UIP file
    QString m_PrimitiveName;
    // Name of the primitive file on the filesystem
    QString m_FileName;
};

struct SBufferManager : public IBufferManager
{
    typedef eastl::hash_set<QString, eastl::hash<QString>,
    eastl::equal_to<QString>, ForwardingAllocator>
    TStringSet;
    typedef QHash<QString, SImageEntry> TImageMap;
    typedef QHash<QString, SRenderMesh *> TMeshMap;
    typedef QHash<QString, QString> TAliasImageMap;

    QDemonScopedRefCounted<QDemonRenderContext> m_Context;
    QDemonScopedRefCounted<IInputStreamFactory> m_InputStreamFactory;
    IPerfTimer &m_PerfTimer;
    TStr m_PathBuilder;
    TImageMap m_ImageMap;
    Mutex m_LoadedImageSetMutex;
    TStringSet m_LoadedImageSet;
    TAliasImageMap m_AliasImageMap;
    TMeshMap m_MeshMap;
    SPrimitiveEntry m_PrimitiveNames[5];
    QVector<QDemonRenderVertexBufferEntry> m_EntryBuffer;
    bool m_GPUSupportsDXT;
    static const char *GetPrimitivesDirectory() { return "res//primitives"; }

    SBufferManager(QDemonRenderContext &ctx,
                   IInputStreamFactory &inInputStreamFactory, IPerfTimer &inTimer)
        : m_Context(ctx)
        , m_StrTable(strTable)
        , m_InputStreamFactory(inInputStreamFactory)
        , m_PerfTimer(inTimer)
        , m_GPUSupportsDXT(ctx.AreDXTImagesSupported())
    {
    }
    virtual ~SBufferManager() { Clear(); }

    QString CombineBaseAndRelative(const char *inBase,
                                             const char *inRelative) override
    {
        CFileTools::CombineBaseAndRelative(inBase, inRelative, m_PathBuilder);
        return QString::fromLocal8Bit(m_PathBuilder.c_str());
    }

    void SetImageHasTransparency(QString inImagePath, bool inHasTransparency) override
    {
        pair<TImageMap::iterator, bool> theImage =
                m_ImageMap.insert(make_pair(inImagePath, SImageEntry()));
        theImage.first->second.m_TextureFlags.SetHasTransparency(inHasTransparency);
    }

    bool GetImageHasTransparency(QString inSourcePath) const override
    {
        TImageMap::const_iterator theIter = m_ImageMap.find(inSourcePath);
        if (theIter != m_ImageMap.end())
            return theIter->second.m_TextureFlags.HasTransparency();
        return false;
    }

    void SetImageTransparencyToFalseIfNotSet(QString inSourcePath) override
    {
        pair<TImageMap::iterator, bool> theImage =
                m_ImageMap.insert(make_pair(inSourcePath, SImageEntry()));
        // If we did actually insert something
        if (theImage.second)
            theImage.first->second.m_TextureFlags.SetHasTransparency(false);
    }

    void SetInvertImageUVCoords(QString inImagePath, bool inShouldInvertCoords) override
    {
        pair<TImageMap::iterator, bool> theImage =
                m_ImageMap.insert(make_pair(inImagePath, SImageEntry()));
        theImage.first->second.m_TextureFlags.SetInvertUVCoords(inShouldInvertCoords);
    }

    bool IsImageLoaded(QString inSourcePath) override
    {
        Mutex::ScopedLock __locker(m_LoadedImageSetMutex);
        return m_LoadedImageSet.find(inSourcePath) != m_LoadedImageSet.end();
    }

    bool AliasImagePath(QString inSourcePath, QString inAliasPath,
                        bool inIgnoreIfLoaded) override
    {
        if (inSourcePath.IsValid() == false || inAliasPath.IsValid() == false)
            return false;
        // If the image is loaded then we ignore this call in some cases.
        if (inIgnoreIfLoaded && IsImageLoaded(inSourcePath))
            return false;
        m_AliasImageMap.insert(inSourcePath, inAliasPath);
        return true;
    }

    void UnaliasImagePath(QString inSourcePath) override
    {
        m_AliasImageMap.erase(inSourcePath);
    }

    QString GetImagePath(QString inSourcePath) override
    {
        TAliasImageMap::iterator theAliasIter = m_AliasImageMap.find(inSourcePath);
        if (theAliasIter != m_AliasImageMap.end())
            return theAliasIter->second;
        return inSourcePath;
    }

    static inline int wrapMod(int a, int base)
    {
        int ret = a % base;
        if (ret < 0)
            ret += base;
        return ret;
    }

    static inline void getWrappedCoords(int &sX, int &sY, int width, int height)
    {
        if (sY < 0) {
            sX -= width >> 1;
            sY = -sY;
        }
        if (sY >= height) {
            sX += width >> 1;
            sY = height - sY;
        }
        sX = wrapMod(sX, width);
        sY = wrapMod(sY, height);
    }

    SImageTextureData LoadRenderImage(QString inImagePath,
                                      SLoadedTexture &inLoadedImage,
                                      bool inForceScanForTransparency, bool inBsdfMipmaps) override
    {
        SStackPerfTimer __perfTimer(m_PerfTimer, "Image Upload");
        {
            Mutex::ScopedLock __mapLocker(m_LoadedImageSetMutex);
            m_LoadedImageSet.insert(inImagePath);
        }
        pair<TImageMap::iterator, bool> theImage =
                m_ImageMap.insert(make_pair(inImagePath, SImageEntry()));
        bool wasInserted = theImage.second;
        theImage.first->second.m_Loaded = true;
        // inLoadedImage.EnsureMultiplerOfFour( m_Context->GetFoundation(), inImagePath.c_str() );

        QDemonRenderTexture2D *theTexture = m_Context->CreateTexture2D();
        if (inLoadedImage.data) {
            QDemonRenderTextureFormats::Enum destFormat = inLoadedImage.format;
            if (inBsdfMipmaps) {
                if (m_Context->GetRenderContextType() == QDemonRenderContextValues::GLES2)
                    destFormat = QDemonRenderTextureFormats::RGBA8;
                else
                    destFormat = QDemonRenderTextureFormats::RGBA16F;
            }
            else {
                theTexture->SetTextureData(
                            QDemonDataRef<quint8>((quint8 *)inLoadedImage.data, inLoadedImage.dataSizeInBytes), 0,
                            inLoadedImage.width, inLoadedImage.height, inLoadedImage.format, destFormat);
            }

            if (inBsdfMipmaps
                    && QDemonRenderTextureFormats::isUncompressedTextureFormat(inLoadedImage.format)) {
                theTexture->SetMinFilter(QDemonRenderTextureMinifyingOp::LinearMipmapLinear);
                Qt3DSRenderPrefilterTexture *theBSDFMipMap = theImage.first->second.m_BSDFMipMap;
                if (theBSDFMipMap == nullptr) {
                    theBSDFMipMap = Qt3DSRenderPrefilterTexture::Create(
                                m_Context, inLoadedImage.width, inLoadedImage.height, *theTexture,
                                destFormat, m_Context->GetFoundation());
                    theImage.first->second.m_BSDFMipMap = theBSDFMipMap;
                }

                if (theBSDFMipMap) {
                    theBSDFMipMap->Build(inLoadedImage.data, inLoadedImage.dataSizeInBytes,
                                         inLoadedImage.format);
                }
            }
        } else if (inLoadedImage.dds) {
            theImage.first->second.m_Texture = theTexture;
            bool supportsDXT = m_GPUSupportsDXT;
            bool isDXT = QDemonRenderTextureFormats::isCompressedTextureFormat(inLoadedImage.format);
            bool requiresDecompression = (supportsDXT == false && isDXT) || false;
            // test code for DXT decompression
            // if ( isDXT ) requiresDecompression = true;
            if (requiresDecompression) {
                qCWarning(WARNING, PERF_INFO,
                          "Image %s is DXT format which is unsupported by "
                          "the graphics subsystem, decompressing in CPU",
                          inImagePath.c_str());
            }
            STextureData theDecompressedImage;
            for (int idx = 0; idx < inLoadedImage.dds->numMipmaps; ++idx) {
                if (inLoadedImage.dds->mipwidth[idx] && inLoadedImage.dds->mipheight[idx]) {
                    if (requiresDecompression == false) {
                        theTexture->SetTextureData(
                                    toU8DataRef((char *)inLoadedImage.dds->data[idx],
                                                (quint32)inLoadedImage.dds->size[idx]),
                                    (quint8)idx, (quint32)inLoadedImage.dds->mipwidth[idx],
                                    (quint32)inLoadedImage.dds->mipheight[idx], inLoadedImage.format);
                    } else {
                        theDecompressedImage =
                                inLoadedImage.DecompressDXTImage(idx, &theDecompressedImage);

                        if (theDecompressedImage.data) {
                            theTexture->SetTextureData(
                                        toU8DataRef((char *)theDecompressedImage.data,
                                                    (quint32)theDecompressedImage.dataSizeInBytes),
                                        (quint8)idx, (quint32)inLoadedImage.dds->mipwidth[idx],
                                        (quint32)inLoadedImage.dds->mipheight[idx],
                                        theDecompressedImage.format);
                        }
                    }
                }
            }
            if (theDecompressedImage.data)
                inLoadedImage.ReleaseDecompressedTexture(theDecompressedImage);
        }
        if (wasInserted == true || inForceScanForTransparency)
            theImage.first->second.m_TextureFlags.SetHasTransparency(
                        inLoadedImage.ScanForTransparency());
        theImage.first->second.m_Texture = theTexture;
        return theImage.first->second;
    }

    SImageTextureData LoadRenderImage(QString inImagePath,
                                      bool inForceScanForTransparency, bool inBsdfMipmaps) override
    {
        inImagePath = GetImagePath(inImagePath);

        if (!inImagePath.IsValid())
            return SImageEntry();

        TImageMap::iterator theIter = m_ImageMap.find(inImagePath);
        if (theIter == m_ImageMap.end() && inImagePath.IsValid()) {
            NVScopedReleasable<SLoadedTexture> theLoadedImage;
            {
                SStackPerfTimer __perfTimer(m_PerfTimer, "Image Decompression");
                theLoadedImage = SLoadedTexture::Load(
                            inImagePath.c_str(), m_Context->GetFoundation(), *m_InputStreamFactory,
                            true, m_Context->GetRenderContextType());
                // Hackish solution to custom materials not finding their textures if they are used
                // in sub-presentations. Note: Runtime 1 is going to be removed in Qt 3D Studio 2.x,
                // so this should be ok.
                if (!theLoadedImage) {
                    if (QDir(inImagePath.c_str()).isRelative()) {
                        QString searchPath = inImagePath.c_str();
                        if (searchPath.startsWith(QLatin1String("./")))
                            searchPath.prepend(QLatin1String("."));
                        int loops = 0;
                        while (!theLoadedImage && ++loops <= 3) {
                            theLoadedImage = SLoadedTexture::Load(
                                        searchPath.toUtf8(), m_Context->GetFoundation(),
                                        *m_InputStreamFactory, true,
                                        m_Context->GetRenderContextType());
                            searchPath.prepend(QLatin1String("../"));
                        }
                    } else {
                        // Some textures, for example environment maps for custom materials,
                        // have absolute path at this point. It points to the wrong place with
                        // the new project structure, so we need to split it up and construct
                        // the new absolute path here.
                        QString wholePath = inImagePath.c_str();
                        QStringList splitPath = wholePath.split(QLatin1String("../"));
                        if (splitPath.size() > 1) {
                            QString searchPath = splitPath.at(0) + splitPath.at(1);
                            int loops = 0;
                            while (!theLoadedImage && ++loops <= 3) {
                                theLoadedImage = SLoadedTexture::Load(
                                            searchPath.toUtf8(), m_Context->GetFoundation(),
                                            *m_InputStreamFactory, true,
                                            m_Context->GetRenderContextType());
                                searchPath = splitPath.at(0);
                                for (int i = 0; i < loops; i++)
                                    searchPath.append(QLatin1String("../"));
                                searchPath.append(splitPath.at(1));
                            }
                        }
                    }
                }
            }

            if (theLoadedImage) {
                return LoadRenderImage(inImagePath, *theLoadedImage, inForceScanForTransparency,
                                       inBsdfMipmaps);
            } else {
                // We want to make sure that bad path fails once and doesn't fail over and over
                // again
                // which could slow down the system quite a bit.
                pair<TImageMap::iterator, bool> theImage =
                        m_ImageMap.insert(make_pair(inImagePath, SImageEntry()));
                theImage.first->second.m_Loaded = true;
                qCWarning(WARNING, "Failed to load image: %s", inImagePath.c_str());
                theIter = theImage.first;
            }
        }
        return theIter->second;
    }

    qt3dsimp::SMultiLoadResult LoadPrimitive(const char *inRelativePath)
    {
        QString theName(QString::fromLocal8Bit(inRelativePath));
        if (m_PrimitiveNames[0].m_PrimitiveName.IsValid() == false) {
            m_PrimitiveNames[0].m_PrimitiveName = QStringLiteral("#Rectangle");
            m_PrimitiveNames[0].m_FileName = QStringLiteral("Rectangle.mesh");
            m_PrimitiveNames[1].m_PrimitiveName = QStringLiteral("#Sphere");
            m_PrimitiveNames[1].m_FileName = QStringLiteral("Sphere.mesh");
            m_PrimitiveNames[2].m_PrimitiveName = QStringLiteral("#Cube");
            m_PrimitiveNames[2].m_FileName = QStringLiteral("Cube.mesh");
            m_PrimitiveNames[3].m_PrimitiveName = QStringLiteral("#Cone");
            m_PrimitiveNames[3].m_FileName = QStringLiteral("Cone.mesh");
            m_PrimitiveNames[4].m_PrimitiveName = QStringLiteral("#Cylinder");
            m_PrimitiveNames[4].m_FileName = QStringLiteral("Cylinder.mesh");
        }
        for (size_t idx = 0; idx < 5; ++idx) {
            if (m_PrimitiveNames[idx].m_PrimitiveName == theName) {
                CFileTools::CombineBaseAndRelative(GetPrimitivesDirectory(),
                                                   m_PrimitiveNames[idx].m_FileName, m_PathBuilder);
                quint32 id = 1;
                QDemonScopedRefCounted<IRefCountedInputStream> theInStream(
                            m_InputStreamFactory->GetStreamForFile(m_PathBuilder.c_str()));
                if (theInStream)
                    return qt3dsimp::Mesh::LoadMulti(m_Context->GetAllocator(), *theInStream, id);
                else {
                    qCCritical(INTERNAL_ERROR, "Unable to find mesh primitive %s",
                               m_PathBuilder.c_str());
                    return qt3dsimp::SMultiLoadResult();
                }
            }
        }
        return qt3dsimp::SMultiLoadResult();
    }

    virtual QDemonConstDataRef<quint8> CreatePackedPositionDataArray(qt3dsimp::SMultiLoadResult *inResult)
    {
        // we assume a position consists of 3 floats
        quint32 vertexCount = inResult->m_Mesh->m_VertexBuffer.m_Data.size()
                / inResult->m_Mesh->m_VertexBuffer.m_Stride;
        quint32 dataSize = vertexCount * 3 * sizeof(float);
        float *posData = (float *)QDEMON_ALLOC(m_Context->GetAllocator(), dataSize,
                                               "SRenderMesh::CreatePackedPositionDataArray");
        quint8 *baseOffset = reinterpret_cast<quint8 *>(inResult->m_Mesh);
        // copy position data
        if (posData) {
            float *srcData = (float *)inResult->m_Mesh->m_VertexBuffer.m_Data.begin(baseOffset);
            quint32 srcStride = inResult->m_Mesh->m_VertexBuffer.m_Stride / sizeof(float);
            float *dstData = posData;
            quint32 dstStride = 3;

            for (quint32 i = 0; i < vertexCount; ++i) {
                dstData[0] = srcData[0];
                dstData[1] = srcData[1];
                dstData[2] = srcData[2];

                dstData += dstStride;
                srcData += srcStride;
            }

            return toConstDataRef((const quint8 *)posData, dataSize);
        }

        return QDemonConstDataRef<quint8>();
    }

    SRenderMesh *LoadMesh(QString inMeshPath) override
    {
        if (inMeshPath.IsValid() == false)
            return nullptr;
        pair<TMeshMap::iterator, bool> theMesh =
                m_MeshMap.insert(make_pair(inMeshPath, (SRenderMesh *)nullptr));
        if (theMesh.second == true) {
            // check to see if this is primitive

            qt3dsimp::SMultiLoadResult theResult = LoadPrimitive(inMeshPath);

            // Attempt a load from the filesystem if this mesh isn't a primitive.
            if (theResult.m_Mesh == nullptr) {
                m_PathBuilder = inMeshPath;
                TStr::size_type pound = m_PathBuilder.rfind('#');
                quint32 id = 0;
                if (pound != TStr::npos) {
                    id = atoi(m_PathBuilder.c_str() + pound + 1);
                    m_PathBuilder.erase(m_PathBuilder.begin() + pound, m_PathBuilder.end());
                }
                QDemonScopedRefCounted<IRefCountedInputStream> theStream(
                            m_InputStreamFactory->GetStreamForFile(m_PathBuilder.c_str()));
                if (theStream) {
                    theResult = qt3dsimp::Mesh::LoadMulti(m_Context->GetAllocator(), *theStream, id);
                }
                if (theResult.m_Mesh == nullptr)
                    qCWarning(WARNING, "Failed to load mesh: %s", m_PathBuilder.c_str());
            }

            if (theResult.m_Mesh) {
                SRenderMesh *theNewMesh = new SRenderMesh(
                            QDemonRenderDrawMode::Triangles,
                            QDemonRenderWinding::CounterClockwise, theResult.m_Id,
                            m_Context->GetAllocator());
                quint8 *baseAddress = reinterpret_cast<quint8 *>(theResult.m_Mesh);
                theMesh.first->second = theNewMesh;
                QDemonConstDataRef<quint8> theVBufData(
                            theResult.m_Mesh->m_VertexBuffer.m_Data.begin(baseAddress),
                            theResult.m_Mesh->m_VertexBuffer.m_Data.size());

                QDemonRenderVertexBuffer *theVertexBuffer = m_Context->CreateVertexBuffer(
                            QDemonRenderBufferUsageType::Static,
                            theResult.m_Mesh->m_VertexBuffer.m_Data.m_Size,
                            theResult.m_Mesh->m_VertexBuffer.m_Stride, theVBufData);

                // create a tight packed position data VBO
                // this should improve our depth pre pass rendering
                QDemonRenderVertexBuffer *thePosVertexBuffer = nullptr;
                QDemonConstDataRef<quint8> posData = CreatePackedPositionDataArray(&theResult);
                if (posData.size()) {
                    thePosVertexBuffer =
                            m_Context->CreateVertexBuffer(QDemonRenderBufferUsageType::Static,
                                                          posData.size(), 3 * sizeof(float), posData);
                }

                QDemonRenderIndexBuffer *theIndexBuffer = nullptr;
                if (theResult.m_Mesh->m_IndexBuffer.m_Data.size()) {
                    using QDemonRenderComponentTypes;
                    quint32 theIndexBufferSize = theResult.m_Mesh->m_IndexBuffer.m_Data.size();
                    QDemonRenderComponentTypes::Enum bufComponentType =
                            theResult.m_Mesh->m_IndexBuffer.m_ComponentType;
                    quint32 sizeofType =
                            QDemonRenderComponentTypes::getSizeofType(bufComponentType);

                    if (sizeofType == 2 || sizeofType == 4) {
                        // Ensure type is unsigned; else things will fail in rendering pipeline.
                        if (bufComponentType == QDemonRenderComponentTypes::qint16)
                            bufComponentType = QDemonRenderComponentTypes::quint16;
                        if (bufComponentType == QDemonRenderComponentTypes::qint32)
                            bufComponentType = QDemonRenderComponentTypes::quint32;

                        QDemonConstDataRef<quint8> theIBufData(
                                    theResult.m_Mesh->m_IndexBuffer.m_Data.begin(baseAddress),
                                    theResult.m_Mesh->m_IndexBuffer.m_Data.size());
                        theIndexBuffer = m_Context->CreateIndexBuffer(
                                    QDemonRenderBufferUsageType::Static, bufComponentType,
                                    theIndexBufferSize, theIBufData);
                    } else {
                        Q_ASSERT(false);
                    }
                }
                QVector<QDemonRenderVertexBufferEntry> &theEntryBuffer(m_EntryBuffer);
                theEntryBuffer.resize(theResult.m_Mesh->m_VertexBuffer.m_Entries.size());
                for (quint32 entryIdx = 0,
                     entryEnd = theResult.m_Mesh->m_VertexBuffer.m_Entries.size();
                     entryIdx < entryEnd; ++entryIdx) {
                    theEntryBuffer[entryIdx] =
                            theResult.m_Mesh->m_VertexBuffer.m_Entries.index(baseAddress, entryIdx)
                            .ToVertexBufferEntry(baseAddress);
                }
                // create our attribute layout
                QDemonRenderAttribLayout *theAttribLayout =
                        m_Context->CreateAttributeLayout(theEntryBuffer);
                // create our attribute layout for depth pass
                QDemonRenderVertexBufferEntry theEntries[] = {
                    QDemonRenderVertexBufferEntry(
                    "attr_pos", QDemonRenderComponentTypes::float, 3),
                };
                QDemonRenderAttribLayout *theAttribLayoutDepth =
                        m_Context->CreateAttributeLayout(toConstDataRef(theEntries, 1));

                // create input assembler object
                quint32 strides = theResult.m_Mesh->m_VertexBuffer.m_Stride;
                quint32 offsets = 0;
                QDemonRenderInputAssembler *theInputAssembler = m_Context->CreateInputAssembler(
                            theAttribLayout, toConstDataRef(&theVertexBuffer, 1), theIndexBuffer,
                            toConstDataRef(&strides, 1), toConstDataRef(&offsets, 1),
                            theResult.m_Mesh->m_DrawMode);

                // create depth input assembler object
                quint32 posStrides = (thePosVertexBuffer) ? 3 * sizeof(float) : strides;
                QDemonRenderInputAssembler *theInputAssemblerDepth = m_Context->CreateInputAssembler(
                            theAttribLayoutDepth,
                            toConstDataRef((thePosVertexBuffer) ? &thePosVertexBuffer : &theVertexBuffer,
                                           1),
                            theIndexBuffer, toConstDataRef(&posStrides, 1), toConstDataRef(&offsets, 1),
                            theResult.m_Mesh->m_DrawMode);

                QDemonRenderInputAssembler *theInputAssemblerPoints = m_Context->CreateInputAssembler(
                            theAttribLayoutDepth,
                            toConstDataRef((thePosVertexBuffer) ? &thePosVertexBuffer : &theVertexBuffer,
                                           1),
                            nullptr, toConstDataRef(&posStrides, 1), toConstDataRef(&offsets, 1),
                            QDemonRenderDrawMode::Points);

                if (!theInputAssembler || !theInputAssemblerDepth || !theInputAssemblerPoints) {
                    Q_ASSERT(false);
                    return nullptr;
                }
                theNewMesh->m_Joints.resize(theResult.m_Mesh->m_Joints.size());
                for (quint32 jointIdx = 0, jointEnd = theResult.m_Mesh->m_Joints.size();
                     jointIdx < jointEnd; ++jointIdx) {
                    const qt3dsimp::Joint &theImportJoint(
                                theResult.m_Mesh->m_Joints.index(baseAddress, jointIdx));
                    SRenderJoint &theNewJoint(theNewMesh->m_Joints[jointIdx]);
                    theNewJoint.m_JointID = theImportJoint.m_JointID;
                    theNewJoint.m_ParentID = theImportJoint.m_ParentID;
                    ::memcpy(theNewJoint.m_invBindPose, theImportJoint.m_invBindPose,
                            16 * sizeof(float));
                    ::memcpy(theNewJoint.m_localToGlobalBoneSpace,
                            theImportJoint.m_localToGlobalBoneSpace, 16 * sizeof(float));
                }

                for (quint32 subsetIdx = 0, subsetEnd = theResult.m_Mesh->m_Subsets.size();
                     subsetIdx < subsetEnd; ++subsetIdx) {
                    SRenderSubset theSubset(m_Context->GetAllocator());
                    const qt3dsimp::MeshSubset &source(
                                theResult.m_Mesh->m_Subsets.index(baseAddress, subsetIdx));
                    theSubset.m_Bounds = source.m_Bounds;
                    theSubset.m_Count = source.m_Count;
                    theSubset.m_Offset = source.m_Offset;
                    theSubset.m_Joints = theNewMesh->m_Joints;
                    theSubset.m_Name = QString::fromLocal8Bit(source.m_Name.begin(baseAddress));
                    //theVertexBuffer->addRef();
                    theSubset.m_VertexBuffer = theVertexBuffer;
                    if (thePosVertexBuffer) {
                        //thePosVertexBuffer->addRef();
                        theSubset.m_PosVertexBuffer = thePosVertexBuffer;
                    }
                    if (theIndexBuffer) {
                        //theIndexBuffer->addRef();
                        theSubset.m_IndexBuffer = theIndexBuffer;
                    }
                    theSubset.m_InputAssembler = theInputAssembler;
                    theSubset.m_InputAssemblerDepth = theInputAssemblerDepth;
                    theSubset.m_InputAssemblerPoints = theInputAssemblerPoints;
                    theSubset.m_PrimitiveType = theResult.m_Mesh->m_DrawMode;
                    //theInputAssembler->addRef();
                    //theInputAssemblerDepth->addRef();
                    //theSubset.m_InputAssemblerPoints->addRef();
                    theNewMesh->m_Subsets.push_back(theSubset);
                }
                // If we want to, we can an in a quite stupid way break up modes into sub-subsets.
                // These are assumed to use the same material as the outer subset but have fewer tris
                // and should have a more exact bounding box.  This sort of thing helps with using the frustum
                // culling
                // system but it is really done incorrectly.  It should be done via some sort of oct-tree mechanism
                // and it
                // so that the sub-subsets spatially sorted and it should only be done upon save-to-binary with the
                // results
                // saved out to disk.  As you can see, doing it properly requires some real engineering effort so it
                // is somewhat
                // unlikely it will ever happen.  Or it could be done on import if someone really wants to change
                // the mesh buffer
                // format.  Either way it isn't going to happen here and it isn't going to happen this way but this
                // is a working
                // example of using the technique.
#ifdef QDEMON_RENDER_GENERATE_SUB_SUBSETS
                QDemonOption<QDemonRenderVertexBufferEntry> thePosAttrOpt =
                        theVertexBuffer->GetEntryByName("attr_pos");
                bool hasPosAttr = thePosAttrOpt.hasValue()
                        && thePosAttrOpt->m_ComponentType == QDemonRenderComponentTypes::float
                        && thePosAttrOpt->m_NumComponents == 3;

                for (size_t subsetIdx = 0, subsetEnd = theNewMesh->m_Subsets.size();
                     subsetIdx < subsetEnd; ++subsetIdx) {
                    SRenderSubset &theOuterSubset = theNewMesh->m_Subsets[subsetIdx];
                    if (theOuterSubset.m_Count && theIndexBuffer
                            && theIndexBuffer->GetComponentType()
                            == QDemonRenderComponentTypes::quint16
                            && theNewMesh->m_DrawMode == QDemonRenderDrawMode::Triangles && hasPosAttr) {
                        // Num tris in a sub subset.
                        quint32 theSubsetSize = 3334 * 3; // divisible by three.
                        size_t theNumSubSubsets =
                                ((theOuterSubset.m_Count - 1) / theSubsetSize) + 1;
                        quint32 thePosAttrOffset = thePosAttrOpt->m_FirstItemOffset;
                        const quint8 *theVertData = theResult.m_Mesh->m_VertexBuffer.m_Data.begin();
                        const quint8 *theIdxData = theResult.m_Mesh->m_IndexBuffer.m_Data.begin();
                        quint32 theVertStride = theResult.m_Mesh->m_VertexBuffer.m_Stride;
                        quint32 theOffset = theOuterSubset.m_Offset;
                        quint32 theCount = theOuterSubset.m_Count;
                        for (size_t subSubsetIdx = 0, subSubsetEnd = theNumSubSubsets;
                             subSubsetIdx < subSubsetEnd; ++subSubsetIdx) {
                            SRenderSubsetBase theBase;
                            theBase.m_Offset = theOffset;
                            theBase.m_Count = NVMin(theSubsetSize, theCount);
                            theBase.m_Bounds.setEmpty();
                            theCount -= theBase.m_Count;
                            theOffset += theBase.m_Count;
                            // Create new bounds.
                            // Offset is in item size, not bytes.
                            const quint16 *theSubsetIdxData =
                                    reinterpret_cast<const quint16 *>(theIdxData + theBase.m_Offset * 2);
                            for (size_t theIdxIdx = 0, theIdxEnd = theBase.m_Count;
                                 theIdxIdx < theIdxEnd; ++theIdxIdx) {
                                quint32 theVertOffset = theSubsetIdxData[theIdxIdx] * theVertStride;
                                theVertOffset += thePosAttrOffset;
                                QVector3D thePos = *(
                                            reinterpret_cast<const QVector3D *>(theVertData + theVertOffset));
                                theBase.m_Bounds.include(thePos);
                            }
                            theOuterSubset.m_SubSubsets.push_back(theBase);
                        }
                    } else {
                        SRenderSubsetBase theBase;
                        theBase.m_Bounds = theOuterSubset.m_Bounds;
                        theBase.m_Count = theOuterSubset.m_Count;
                        theBase.m_Offset = theOuterSubset.m_Offset;
                        theOuterSubset.m_SubSubsets.push_back(theBase);
                    }
                }
#endif
                if (posData.size())
                    m_Context->GetAllocator().deallocate((void *)posData.begin());

                m_Context->GetAllocator().deallocate(theResult.m_Mesh);
            }
        }
        return theMesh.first->second;
    }

    SRenderMesh *CreateMesh(Qt3DSBCharPtr inSourcePath, quint8 *inVertData, quint32 inNumVerts,
                            quint32 inVertStride, quint32 *inIndexData, quint32 inIndexCount,
                            QDemonBounds3 inBounds) override
    {
        QString sourcePath = QString::fromLocal8Bit(inSourcePath);

        // QPair<QString, SRenderMesh*> thePair(sourcePath, (SRenderMesh*)nullptr);
        QPair<TMeshMap::iterator, bool> theMesh;
        // Make sure there isn't already a buffer entry for this mesh.
        if (m_MeshMap.contains(sourcePath)) {
            theMesh = QPair<TMeshMap::iterator, bool>(m_MeshMap.find(sourcePath), true);
        } else {
            theMesh = m_MeshMap.insert(make_pair(sourcePath, (SRenderMesh *)nullptr));
        }

        if (theMesh.second == true) {
            SRenderMesh *theNewMesh = new SRenderMesh(
                        QDemonRenderDrawMode::Triangles,
                        QDemonRenderWinding::CounterClockwise, 0, m_Context->GetAllocator());

            // If we failed to create the RenderMesh, return a failure.
            if (!theNewMesh) {
                Q_ASSERT(false);
                return nullptr;
            }

            // Get rid of any old mesh that was sitting here and fill it with a new one.
            // NOTE : This is assuming that the source of our mesh data doesn't do its own memory
            // management and always returns new buffer pointers every time.
            // Don't know for sure if that's what we'll get from our intended sources, but that's
            // easily
            // adjustable by looking for matching pointers in the Subsets.
            if (theNewMesh && theMesh.first->second != nullptr) {
                delete theMesh.first->second;
                theMesh.first->second = nullptr;
            }

            theMesh.first->second = theNewMesh;
            quint32 vertDataSize = inNumVerts * inVertStride;
            QDemonConstDataRef<quint8> theVBufData(inVertData, vertDataSize);
            // QDemonConstDataRef<quint8> theVBufData( theResult.m_Mesh->m_VertexBuffer.m_Data.begin(
            // baseAddress )
            //		, theResult.m_Mesh->m_VertexBuffer.m_Data.size() );

            QDemonRenderVertexBuffer *theVertexBuffer =
                    m_Context->CreateVertexBuffer(QDemonRenderBufferUsageType::Static,
                                                  vertDataSize, inVertStride, theVBufData);
            QDemonRenderIndexBuffer *theIndexBuffer = nullptr;
            if (inIndexData != nullptr && inIndexCount > 3) {
                QDemonConstDataRef<quint8> theIBufData((quint8 *)inIndexData, inIndexCount * sizeof(quint32));
                theIndexBuffer =
                        m_Context->CreateIndexBuffer(QDemonRenderBufferUsageType::Static,
                                                     QDemonRenderComponentTypes::quint32,
                                                     inIndexCount * sizeof(quint32), theIBufData);
            }

            // WARNING
            // Making an assumption here about the contents of the stream
            // PKC TODO : We may have to consider some other format.
            QDemonRenderVertexBufferEntry theEntries[] = {
                QDemonRenderVertexBufferEntry("attr_pos",
                QDemonRenderComponentTypes::float, 3),
                QDemonRenderVertexBufferEntry(
                "attr_uv", QDemonRenderComponentTypes::float, 2, 12),
                QDemonRenderVertexBufferEntry(
                "attr_norm", QDemonRenderComponentTypes::float, 3, 18),
            };

            // create our attribute layout
            QDemonRenderAttribLayout *theAttribLayout =
                    m_Context->CreateAttributeLayout(toConstDataRef(theEntries, 3));
            /*
            // create our attribute layout for depth pass
            QDemonRenderVertexBufferEntry theEntriesDepth[] = {
                    QDemonRenderVertexBufferEntry( "attr_pos",
            QDemonRenderComponentTypes::float, 3 ),
            };
            QDemonRenderAttribLayout* theAttribLayoutDepth = m_Context->CreateAttributeLayout(
            toConstDataRef( theEntriesDepth, 1 ) );
            */
            // create input assembler object
            quint32 strides = inVertStride;
            quint32 offsets = 0;
            QDemonRenderInputAssembler *theInputAssembler = m_Context->CreateInputAssembler(
                        theAttribLayout, toConstDataRef(&theVertexBuffer, 1), theIndexBuffer,
                        toConstDataRef(&strides, 1), toConstDataRef(&offsets, 1),
                        QDemonRenderDrawMode::Triangles);

            if (!theInputAssembler) {
                Q_ASSERT(false);
                return nullptr;
            }

            // Pull out just the mesh object name from the total path
            QString fullName(inSourcePath);
            QString subName(inSourcePath);
            if (fullName.rfind("#") != QString::npos) {
                subName = fullName.substr(fullName.rfind("#"), QString::npos);
            }

            theNewMesh->m_Joints.clear();
            SRenderSubset theSubset(m_Context->GetAllocator());
            theSubset.m_Bounds = inBounds;
            theSubset.m_Count = inIndexCount;
            theSubset.m_Offset = 0;
            theSubset.m_Joints = theNewMesh->m_Joints;
            theSubset.m_Name = QString::fromLocal8Bit(subName.c_str());
            //theVertexBuffer->addRef();
            theSubset.m_VertexBuffer = theVertexBuffer;
            theSubset.m_PosVertexBuffer = nullptr;
            // if (theIndexBuffer)
            //     theIndexBuffer->addRef();
            theSubset.m_IndexBuffer = theIndexBuffer;
            theSubset.m_InputAssembler = theInputAssembler;
            theSubset.m_InputAssemblerDepth = theInputAssembler;
            theSubset.m_InputAssemblerPoints = theInputAssembler;
            theSubset.m_PrimitiveType = QDemonRenderDrawMode::Triangles;
            //theSubset.m_InputAssembler->addRef();
            //theSubset.m_InputAssemblerDepth->addRef();
            //theSubset.m_InputAssemblerPoints->addRef();
            theNewMesh->m_Subsets.push_back(theSubset);
        }

        return theMesh.first->second;
    }

    void ReleaseMesh(SRenderMesh &inMesh)
    {
        for (quint32 subsetIdx = 0, subsetEnd = inMesh.m_Subsets.size(); subsetIdx < subsetEnd;
             ++subsetIdx) {
            //inMesh.m_Subsets[subsetIdx].m_VertexBuffer->release();
            // if (inMesh.m_Subsets[subsetIdx].m_PosVertexBuffer) // can be nullptr
            //     inMesh.m_Subsets[subsetIdx].m_PosVertexBuffer->release();
            // if (inMesh.m_Subsets[subsetIdx].m_IndexBuffer) // can be nullptr
            //     inMesh.m_Subsets[subsetIdx].m_IndexBuffer->release();
            // inMesh.m_Subsets[subsetIdx].m_InputAssembler->release();
            // inMesh.m_Subsets[subsetIdx].m_InputAssemblerDepth->release();
            // if (inMesh.m_Subsets[subsetIdx].m_InputAssemblerPoints)
            //     inMesh.m_Subsets[subsetIdx].m_InputAssemblerPoints->release();
        }
        delete &inMesh;
    }
    void ReleaseTexture(SImageEntry &inEntry)
    {
        // if (inEntry.m_Texture)
        //     inEntry.m_Texture->release();
    }
    void Clear() override
    {
        for (TMeshMap::iterator iter = m_MeshMap.begin(), end = m_MeshMap.end(); iter != end;
             ++iter) {
            SRenderMesh *theMesh = iter->second;
            if (theMesh)
                ReleaseMesh(*theMesh);
        }
        m_MeshMap.clear();
        for (TImageMap::iterator iter = m_ImageMap.begin(), end = m_ImageMap.end(); iter != end;
             ++iter) {
            SImageEntry &theEntry = iter->second;
            ReleaseTexture(theEntry);
        }
        m_ImageMap.clear();
        m_AliasImageMap.clear();
        {
            Mutex::ScopedLock __locker(m_LoadedImageSetMutex);
            m_LoadedImageSet.clear();
        }
    }
    void InvalidateBuffer(QString inSourcePath) override
    {
        {
            TMeshMap::iterator iter = m_MeshMap.find(inSourcePath);
            if (iter != m_MeshMap.end()) {
                if (iter->second)
                    ReleaseMesh(*iter->second);
                m_MeshMap.erase(iter);
                return;
            }
        }
        {
            TImageMap::iterator iter = m_ImageMap.find(inSourcePath);
            if (iter != m_ImageMap.end()) {
                SImageEntry &theEntry = iter->second;
                ReleaseTexture(theEntry);
                m_ImageMap.erase(inSourcePath);
                {
                    Mutex::ScopedLock __locker(m_LoadedImageSetMutex);
                    m_LoadedImageSet.erase(inSourcePath);
                }
            }
        }
    }
};
}

IBufferManager &IBufferManager::Create(QDemonRenderContext &inRenderContext,
                                       IInputStreamFactory &inFactory, IPerfTimer &inPerfTimer)
{
    return *new SBufferManager(inRenderContext, inFactory, inPerfTimer);
}

QT_END_NAMESPACE
