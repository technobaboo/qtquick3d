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
#include <QtDemonRuntimeRender/qdemonrendermodel.h>
#include <QtDemonRuntimeRender/qdemonrenderbuffermanager.h>
#include <QtDemonRuntimeRender/qdemonrendermesh.h>
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterial.h>

QT_BEGIN_NAMESPACE

QDemonRenderModel::QDemonRenderModel()
    : QDemonRenderNode(QDemonRenderGraphObject::Type::Model)
    , firstMaterial(nullptr)
    , skeletonRoot(-1)
    , tessellationMode(TessModeValues::NoTess)
    , edgeTess(1.0)
    , innerTess(1.0)
    , wireframeMode(false)
{
}

void QDemonRenderModel::addMaterial(QDemonRenderGraphObject &inMaterial)
{
    if (firstMaterial == nullptr) {
        firstMaterial = &inMaterial;
    } else {
        QDemonRenderGraphObject *lastMaterial = firstMaterial;
        // empty loop intentional
        while (QDemonRenderGraphObject *next = lastMaterial->nextMaterialSibling())
             lastMaterial = next;
        lastMaterial->setNextMaterialSibling(&inMaterial);
    }
    // ### I don't think reparenting is necessary here (I tested without it at least)
    if (inMaterial.type == QDemonRenderGraphObject::Type::DefaultMaterial)
        static_cast<QDemonRenderDefaultMaterial &>(inMaterial).parent = this;
}

QDemonBounds3 QDemonRenderModel::getModelBounds(const QDemonRef<QDemonBufferManager> &inManager) const
{
    QDemonBounds3 retval;
    retval.setEmpty();
    QDemonRenderMesh *theMesh = inManager->loadMesh(meshPath);
    if (theMesh) {
        const auto &subSets = theMesh->subsets;
        for (const auto &subSet : subSets)
            retval.include(subSet.bounds);
    }
    return retval;
}

QT_END_NAMESPACE
