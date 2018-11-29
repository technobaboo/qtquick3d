/****************************************************************************
**
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

#ifndef QDEMONOPENGLPREFIX_H
#define QDEMONOPENGLPREFIX_H

#include <QtGui/qtguiglobal.h>
#if defined(QT_OPENGL_ES)
#define GL_GLEXT_PROTOTYPES
#if defined(QT_OPENGL_ES_3_2)
#include <GLES3/gl32.h>
#else
#include <GLES2/gl2.h>
#include <GLES3/gl3.h>
#endif

// Adding this to ensure that platform version of gl2ext.h is included
// before Qt's qopengles2ext.h which is missing symbols we need
#include <GLES2/gl2ext.h>
#endif

QT_BEGIN_NAMESPACE

QT_END_NAMESPACE

#endif // QDEMONOPENGLPREFIX_H
