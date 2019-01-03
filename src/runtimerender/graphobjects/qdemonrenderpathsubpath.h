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
#pragma once
#ifndef QDEMON_RENDER_PATH_SEGMENT_H
#define QDEMON_RENDER_PATH_SEGMENT_H

#include <QtDemonRuntimeRender/qdemonrendergraphobject.h>

QT_BEGIN_NAMESPACE
struct SPath;

struct SPathSubPath : public SGraphObject
{
    SPath *m_Path;
    SPathSubPath *m_NextSubPath;
    bool m_Closed;

    SPathSubPath()
        : SGraphObject(GraphObjectTypes::PathSubPath)
        , m_Path(nullptr)
        , m_NextSubPath(nullptr)
        , m_Closed(false)
    {
    }

    // Generic method used during serialization
    // to remap string and object pointers
    template <typename TRemapperType>
    void Remap(TRemapperType &inRemapper)
    {
        SGraphObject::Remap(inRemapper);
        inRemapper.Remap(m_Path);
        inRemapper.Remap(m_NextSubPath);
    }
};
QT_END_NAMESPACE

#endif