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

#include <QtDemon/qdemonbounds3.h>

QT_BEGIN_NAMESPACE

struct QDemonRenderMesh;
struct QDemonLoadedTexture;
class QDemonRenderContext;
class QDemonInputStreamFactoryInterface;
class QDemonPerfTimerInterface;

class Q_DEMONRUNTIMERENDER_EXPORT QDemonBufferManagerInterface
{
protected:
    virtual ~QDemonBufferManagerInterface() {}

public:
    // Path manipulation used to get the final path form a base path plus relative extension
    virtual QString combineBaseAndRelative(const char *inBase,
                                           const char *inRelative) = 0;
    virtual void setImageHasTransparency(QString inSourcePath,
                                         bool inHasTransparency) = 0;
    virtual bool getImageHasTransparency(QString inSourcePath) const = 0;
    virtual void setImageTransparencyToFalseIfNotSet(QString inSourcePath) = 0;
    virtual void setInvertImageUVCoords(QString inSourcePath,
                                        bool inShouldInvertCoords) = 0;

    // Returns true if this image has been loaded into memory
    // This call is threadsafe.  Nothing else on this object is guaranteed to be.
    virtual bool isImageLoaded(QString inSourcePath) = 0;

    // Alias one image path with another image path.  Optionally this object will ignore the
    // call if
    // the source path is already loaded.  Aliasing is currently used to allow a default image
    // to be shown
    // in place of an image that is loading offline.
    // Returns true if the image was aliased, false otherwise.
    virtual bool aliasImagePath(QString inSourcePath, QString inAliasPath,
                                bool inIgnoreIfLoaded) = 0;
    virtual void unaliasImagePath(QString inSourcePath) = 0;

    // Returns the given source path unless the source path is aliased; in which case returns
    // the aliased path.
    virtual QString getImagePath(QString inSourcePath) = 0;
    // Returns a texture and a boolean indicating if this texture has transparency in it or not.
    // Can't name this LoadImage because that gets mangled by windows to LoadImageA (uggh)
    // In some cases we need to only scan particular images for transparency.
    virtual QDemonRenderImageTextureData loadRenderImage(QString inImagePath,
                                              QSharedPointer<QDemonLoadedTexture> inTexture,
                                              bool inForceScanForTransparency = false,
                                              bool inBsdfMipmaps = false) = 0;
    virtual QDemonRenderImageTextureData loadRenderImage(QString inSourcePath,
                                              bool inForceScanForTransparency = false,
                                              bool inBsdfMipmaps = false) = 0;
    virtual QDemonRenderMesh *loadMesh(const QString &inSourcePath) = 0;

    virtual QDemonRenderMesh *createMesh(const QString &inSourcePath,
                                    quint8 *inVertData,
                                    quint32 inNumVerts,
                                    quint32 inVertStride,
                                    quint32 *inIndexData,
                                    quint32 inIndexCount,
                                    QDemonBounds3 inBounds) = 0;

    // Remove *all* buffers from the buffer manager;
    virtual void clear() = 0;
    virtual void invalidateBuffer(QString inSourcePath) = 0;

    static QSharedPointer<QDemonBufferManagerInterface> create(QSharedPointer<QDemonRenderContext> inRenderContext,
                                                               QSharedPointer<QDemonInputStreamFactoryInterface> inInputStreamFactory,
                                                               QSharedPointer<QDemonPerfTimerInterface> inTimer);
};
QT_END_NAMESPACE

#endif
