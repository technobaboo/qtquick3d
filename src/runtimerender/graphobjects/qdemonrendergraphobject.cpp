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

#include "qdemonrendergraphobject.h"
#include <QtDemonRuntimeRender/qdemonrendercustommaterial.h>
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterial.h>
#include <QtDemonRuntimeRender/qdemonrenderreferencedmaterial.h>

QT_BEGIN_NAMESPACE

QDemonRenderGraphObject::~QDemonRenderGraphObject() {}

QDemonRenderGraphObject *QDemonRenderGraphObject::nextMaterialSibling()
{
    if (isMaterial() == false) {
        Q_ASSERT(false);
        return nullptr;
    }
    if (type == QDemonRenderGraphObject::Type::CustomMaterial)
        return static_cast<QDemonRenderCustomMaterial *>(this)->m_nextSibling;
    else if (type == QDemonRenderGraphObject::Type::DefaultMaterial)
        return static_cast<QDemonRenderDefaultMaterial *>(this)->nextSibling;
    else
        return static_cast<QDemonRenderReferencedMaterial *>(this)->m_nextSibling;
}

void QDemonRenderGraphObject::setNextMaterialSibling(QDemonRenderGraphObject *sibling)
{
    if (!isMaterial()) {
        Q_ASSERT(false);
        return;
    }
    if (type == QDemonRenderGraphObject::Type::CustomMaterial)
        static_cast<QDemonRenderCustomMaterial *>(this)->m_nextSibling = sibling;
    else if (type == QDemonRenderGraphObject::Type::DefaultMaterial)
        static_cast<QDemonRenderDefaultMaterial *>(this)->nextSibling = sibling;
    else
        static_cast<QDemonRenderReferencedMaterial *>(this)->m_nextSibling = sibling;
}

QT_END_NAMESPACE
