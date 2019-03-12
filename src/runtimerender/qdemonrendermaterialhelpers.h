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
#ifndef QDEMON_RENDER_MATERIAL_HELPERS_H
#define QDEMON_RENDER_MATERIAL_HELPERS_H
#include <QtDemonRuntimeRender/qdemonrendercustommaterial.h>
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterial.h>
#include <QtDemonRuntimeRender/qdemonrenderreferencedmaterial.h>

QT_BEGIN_NAMESPACE

inline bool isMaterial(QDemonRenderGraphObject &obj)
{
    return obj.type == QDemonRenderGraphObject::Type::CustomMaterial || obj.type == QDemonRenderGraphObject::Type::DefaultMaterial
            || obj.type == QDemonRenderGraphObject::Type::ReferencedMaterial;
}

inline bool isMaterial(QDemonRenderGraphObject *obj)
{
    if (obj)
        return isMaterial(*obj);
    return false;
}

inline bool isImage(QDemonRenderGraphObject &obj)
{
    return obj.type == QDemonRenderGraphObject::Type::Image;
}

inline bool isImage(QDemonRenderGraphObject *obj)
{
    if (obj)
        return isImage(*obj);
    return false;
}

inline QDemonRenderGraphObject *getNextMaterialSibling(QDemonRenderGraphObject *obj)
{
    if (obj == nullptr)
        return nullptr;
    if (isMaterial(obj) == false) {
        Q_ASSERT(false);
        return nullptr;
    }
    if (obj->type == QDemonRenderGraphObject::Type::CustomMaterial)
        return static_cast<QDemonRenderCustomMaterial *>(obj)->m_nextSibling;
    else if (obj->type == QDemonRenderGraphObject::Type::DefaultMaterial)
        return static_cast<QDemonRenderDefaultMaterial *>(obj)->nextSibling;
    else
        return static_cast<QDemonRenderReferencedMaterial *>(obj)->m_nextSibling;
}

inline void setNextMaterialSibling(QDemonRenderGraphObject &obj, QDemonRenderGraphObject *sibling)
{
    if (isMaterial(obj) == false) {
        Q_ASSERT(false);
        return;
    }
    if (obj.type == QDemonRenderGraphObject::Type::CustomMaterial)
        static_cast<QDemonRenderCustomMaterial *>(&obj)->m_nextSibling = sibling;
    else if (obj.type == QDemonRenderGraphObject::Type::DefaultMaterial)
        static_cast<QDemonRenderDefaultMaterial *>(&obj)->nextSibling = sibling;
    else
        static_cast<QDemonRenderReferencedMaterial *>(&obj)->m_nextSibling = sibling;
}

QT_END_NAMESPACE

#endif
