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
#ifndef QDEMON_RENDER_DYNAMIC_OBJECT_SYSTEM_UTIL_H
#define QDEMON_RENDER_DYNAMIC_OBJECT_SYSTEM_UTIL_H

#include <StringTable.h>
#include <Qt3DSAllocatorCallback.h>
#include <QtDemonRuntimeRender/qdemonrenderstring.h>

QT_BEGIN_NAMESPACE
namespace dynamic {

struct SStringLoadRemapper
{
    CStrTableOrDataRef m_StrData;
    QString m_PathMapper;
    const char *m_ProjectDir;
    SStringLoadRemapper(CStrTableOrDataRef inData, const char *inProjectDir)
        : m_StrData(inData)
        , m_ProjectDir(inProjectDir)
    {
    }
    void Remap(QString &inStr) { inStr.Remap(m_StrData); }
};

struct SStringSaveRemapper
{
    const SStrRemapMap &m_Map;
    QString m_RelativeBuffer;
    QString m_ProjectDir;
    QString m_FinalBuffer;
    SStringSaveRemapper(const SStrRemapMap &map, const char *inProjectDir)
        : m_Map(map)
        , m_StringTable(inStrTable)
    {
        m_ProjectDir.assign(inProjectDir);
    }
    void Remap(QString &inStr) { inStr.Remap(m_Map); }
};

inline quint32 Align(quint32 inValue)
{
    if (inValue % 4)
        return inValue + (4 - (inValue % 4));
    return inValue;
}

inline quint32 Align8(quint32 inValue)
{
    if (inValue % 8)
        return inValue + (8 - (inValue % 8));
    return inValue;
}

inline quint32 getSizeofShaderDataType(QDemonRenderShaderDataTypes::Enum value)
{
    using namespace qt3ds;
    using namespace render;
    switch (value) {
#define HANDLE_QDEMON_SHADER_DATA_TYPE(x)                                                              \
    case QDemonRenderShaderDataTypes::x:                                                               \
    return sizeof(x);
    ITERATE_QDEMON_SHADER_DATA_TYPES
        #undef HANDLE_QDEMON_SHADER_DATA_TYPE
            default:
        break;
    }
    Q_ASSERT(false);
    return 0;
}

inline const char *GetShaderDatatypeName(QDemonRenderShaderDataTypes::Enum inValue)
{
    switch (inValue) {
#define HANDLE_QDEMON_SHADER_DATA_TYPE(type)                                                           \
    case QDemonRenderShaderDataTypes::type:                                                            \
    return #type;
    ITERATE_QDEMON_SHADER_DATA_TYPES
        #undef HANDLE_QDEMON_SHADER_DATA_TYPE
            default:
        break;
    }
    Q_ASSERT(false);
    return "";
}
}
QT_END_NAMESPACE

#endif
