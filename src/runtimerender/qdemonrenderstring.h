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
#ifndef QDEMON_RENDER_STRING_H
#define QDEMON_RENDER_STRING_H
#include <QtDemonRuntimeRender/qdemonrender.h>
#include <string>

QT_BEGIN_NAMESPACE
// can't name this CString else we will conflict a
class CRenderString : public std::basic_string<char8_t>
{
public:
    typedef std::basic_string<char8_t> TStrType;

    CRenderString()
        : TStrType()
    {
    }
    CRenderString(const CRenderString &inOther)
        : TStrType(inOther)
    {
    }
    CRenderString(const TStrType &inOther)
        : TStrType(inOther)
    {
    }
    CRenderString &operator=(const CRenderString &inOther)
    {
        TStrType::operator=(inOther);
        return *this;
    }
    CRenderString &operator=(const char8_t *inOther)
    {
        TStrType::operator=(inOther);
        return *this;
    }
};
QT_END_NAMESPACE

#endif
