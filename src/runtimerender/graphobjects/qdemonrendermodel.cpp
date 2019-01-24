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
#include <QtDemonRuntimeRender/qdemonrendermaterialhelpers.h>
#include <QtDemonRuntimeRender/qdemonrenderbuffermanager.h>
#include <QtDemonRuntimeRender/qdemonrendermesh.h>

QT_BEGIN_NAMESPACE

SModel::SModel()
    : SNode(GraphObjectTypes::Model)
    , m_FirstMaterial(nullptr)
    , m_SkeletonRoot(-1)
    , m_TessellationMode(TessModeValues::NoTess)
    , m_EdgeTess(1.0)
    , m_InnerTess(1.0)
    , m_WireframeMode(false)
{
}

void SModel::AddMaterial(SGraphObject &inMaterial)
{
    if (m_FirstMaterial == nullptr)
        m_FirstMaterial = &inMaterial;
    else {
        SGraphObject *lastMaterial;
        // empty loop intentional
        for (lastMaterial = m_FirstMaterial; lastMaterial && GetNextMaterialSibling(lastMaterial);
             lastMaterial = GetNextMaterialSibling(lastMaterial)) {
        }
        SetNextMaterialSibling(*lastMaterial, &inMaterial);
    }
    if (inMaterial.m_Type == GraphObjectTypes::DefaultMaterial)
        static_cast<SDefaultMaterial &>(inMaterial).m_Parent = this;
}

QDemonBounds3 SModel::GetModelBounds(QSharedPointer<IBufferManager> inManager) const
{
    QDemonBounds3 retval;
    retval.setEmpty();
    SRenderMesh *theMesh = inManager->LoadMesh(m_MeshPath);
    if (theMesh) {
        for (quint32 idx = 0, end = theMesh->m_Subsets.size(); idx < end; ++idx)
            retval.include(theMesh->m_Subsets[idx].m_Bounds);
    }
    return retval;
}

QT_END_NAMESPACE
