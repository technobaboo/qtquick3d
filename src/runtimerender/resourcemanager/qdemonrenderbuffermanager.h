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
#ifndef QDEMON_RENDER_BUFFER_MANAGER_H
#define QDEMON_RENDER_BUFFER_MANAGER_H

#include <QtDemonRuntimeRender/qtdemonruntimerenderglobal.h>
#include <QtDemonRuntimeRender/qdemonrenderimagetexturedata.h>
#include <QtDemonRuntimeRender/qdemonrendermesh.h>
#include <QtDemon/QDemonPerfTimer>

#include <QtDemon/qdemonbounds3.h>
#include <QtCore/qmutex.h>

QT_BEGIN_NAMESPACE

struct QDemonRenderMesh;
struct QDemonLoadedTexture;
class QDemonRenderContext;
class QDemonInputStreamFactory;
namespace QDemonMeshUtilities {
    struct MultiLoadResult;
}

class Q_DEMONRUNTIMERENDER_EXPORT QDemonBufferManager
{
public:
    QAtomicInt ref;
private:
    typedef QSet<QString> StringSet;
    typedef QHash<QString, QDemonRenderImageTextureData> ImageMap;
    typedef QHash<QDemonRenderMeshPath, QDemonRenderMesh *> MeshMap;
    typedef QHash<QString, QString> AliasImageMap;

    QDemonRef<QDemonRenderContext> context;
    QDemonRef<QDemonInputStreamFactory> inputStreamFactory;
    QDemonPerfTimer *perfTimer;
    ImageMap imageMap;
    QMutex loadedImageSetMutex;
    StringSet loadedImageSet;
    AliasImageMap aliasImageMap;
    MeshMap meshMap;
    QVector<QDemonRenderVertexBufferEntry> entryBuffer;
    bool gpuSupportsDXT;

    void clear();

    QDemonMeshUtilities::MultiLoadResult loadPrimitive(const QString &inRelativePath) const;
    QVector<QVector3D> createPackedPositionDataArray(QDemonMeshUtilities::MultiLoadResult *inResult) const;
    static void releaseMesh(QDemonRenderMesh &inMesh);
    static void releaseTexture(QDemonRenderImageTextureData &inEntry);

public:
    QDemonBufferManager(const QDemonRef<QDemonRenderContext> &inRenderContext,
                        const QDemonRef<QDemonInputStreamFactory> &inInputStreamFactory,
                        QDemonPerfTimer *inTimer);
    ~QDemonBufferManager();

    void setImageHasTransparency(QString inSourcePath, bool inHasTransparency);
    bool getImageHasTransparency(QString inSourcePath) const;
    void setImageTransparencyToFalseIfNotSet(QString inSourcePath);
    void setInvertImageUVCoords(QString inSourcePath, bool inShouldInvertCoords);

    // Returns true if this image has been loaded into memory
    // This call is threadsafe.  Nothing else on this object is guaranteed to be.
    bool isImageLoaded(QString inSourcePath);

    // Alias one image path with another image path.  Optionally this object will ignore the
    // call if
    // the source path is already loaded.  Aliasing is currently used to allow a default image
    // to be shown
    // in place of an image that is loading offline.
    // Returns true if the image was aliased, false otherwise.
    bool aliasImagePath(QString inSourcePath, QString inAliasPath, bool inIgnoreIfLoaded);
    void unaliasImagePath(QString inSourcePath);

    // Returns the given source path unless the source path is aliased; in which case returns
    // the aliased path.
    QString getImagePath(const QString &inSourcePath) const;
    // Returns a texture and a boolean indicating if this texture has transparency in it or not.
    // Can't name this LoadImage because that gets mangled by windows to LoadImageA (uggh)
    // In some cases we need to only scan particular images for transparency.
    QDemonRenderImageTextureData loadRenderImage(const QString &inImagePath,
                                                 const QDemonRef<QDemonLoadedTexture> &inTexture,
                                                 bool inForceScanForTransparency = false,
                                                 bool inBsdfMipmaps = false);
    QDemonRenderImageTextureData loadRenderImage(const QString &inSourcePath,
                                                 const QDemonRenderTextureFormat &inFormat = QDemonRenderTextureFormat::Unknown,
                                                 bool inForceScanForTransparency = false,
                                                 bool inBsdfMipmaps = false);
    QDemonRenderMesh *loadMesh(const QDemonRenderMeshPath &inSourcePath);

    QDemonRenderMesh *createMesh(const QString &inSourcePath,
                                         quint8 *inVertData,
                                         quint32 inNumVerts,
                                         quint32 inVertStride,
                                         quint32 *inIndexData,
                                         quint32 inIndexCount,
                                         QDemonBounds3 inBounds);

    // Remove *all* buffers from the buffer manager;

    void invalidateBuffer(const QString &inSourcePath);

};
QT_END_NAMESPACE

#endif
