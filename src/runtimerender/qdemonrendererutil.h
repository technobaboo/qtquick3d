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
#ifndef QDEMON_RENDERER_UTIL_H
#define QDEMON_RENDERER_UTIL_H

#include <QtDemonRender/qdemonrenderbasetypes.h>

QT_BEGIN_NAMESPACE

class QDemonResourceManagerInterface;
class QDemonResourceTexture2D;
class QDemonRenderContext;

class QDemonRendererUtil
{
    static const qint16 MAX_SSAA_DIM = 8192; // max render traget size for SSAA mode

public:
    static void resolveMutisampleFBOColorOnly(const QDemonRef<QDemonResourceManagerInterface> &inManager,
                                              QDemonResourceTexture2D &ioResult,
                                              QDemonRenderContext &inRenderContext,
                                              qint32 inWidth,
                                              qint32 inHeight,
                                              QDemonRenderTextureFormat inColorFormat,
                                              const QDemonRef<QDemonRenderFrameBuffer> &inSourceFBO);

    static void resolveSSAAFBOColorOnly(const QDemonRef<QDemonResourceManagerInterface> &inManager,
                                        QDemonResourceTexture2D &ioResult,
                                        qint32 outWidth,
                                        qint32 outHeight,
                                        QDemonRenderContext &inRenderContext,
                                        qint32 inWidth,
                                        qint32 inHeight,
                                        QDemonRenderTextureFormat inColorFormat,
                                        const QDemonRef<QDemonRenderFrameBuffer> &inSourceFBO);

    static void getSSAARenderSize(qint32 inWidth, qint32 inHeight, qint32 &outWidth, qint32 &outHeight);

    static quint32 nextMultipleOf4(quint32 value) {
        return (value + 3) & ~3;
    }
};
QT_END_NAMESPACE

#endif
