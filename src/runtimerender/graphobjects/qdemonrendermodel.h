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
#ifndef QDEMON_RENDER_MODEL_H
#define QDEMON_RENDER_MODEL_H

#include <QtDemonRuntimeRender/qdemonrendernode.h>
#include <QtDemonRuntimeRender/qdemonrendertessmodevalues.h>
#include <QtDemonRuntimeRender/qdemonrendermesh.h>

#include <QtDemon/QDemonBounds3>
#include <QtCore/QVector>

QT_BEGIN_NAMESPACE

struct QDemonRenderDefaultMaterial;
class QDemonBufferManager;

struct Q_DEMONRUNTIMERENDER_EXPORT QDemonRenderModel : public QDemonRenderNode
{
    // Complete path to the file;
    //*not* relative to the presentation directory
    QVector<QDemonRenderGraphObject *> materials;
    QDemonRenderMeshPath meshPath;
    qint32 skeletonRoot = -1;
    float edgeTess = 1.0f;
    float innerTess = 1.0f;
    TessModeValues tessellationMode = TessModeValues::NoTess;
    bool wireframeMode = false;

    QDemonRenderModel();

    QDemonBounds3 getModelBounds(const QDemonRef<QDemonBufferManager> &inManager) const;
};
QT_END_NAMESPACE

#endif
