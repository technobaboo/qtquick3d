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
#ifndef QT3DS_RENDER_EXAMPLE_TOOLS_H
#define QT3DS_RENDER_EXAMPLE_TOOLS_H
#include "renderexample.h"

class QDemonRenderExampleTools
{
public:
    static const char *getSimpleVertShader()
    {
        return "uniform mat4 mat_mvp;\n"
               "attribute vec3 attr_pos;			// Vertex pos\n"
               "attribute vec3 attr_norm;			// Vertex pos\n"
               "attribute vec2 attr_uv; 			// UV coords\n"
               "varying vec4 color_to_add;\n"
               "varying vec2 uv_coords;\n"
               "void main()\n"
               "{\n"
               "gl_Position = mat_mvp * vec4(attr_pos, 1.0);\n"
               "color_to_add.xyz = attr_norm * attr_norm;\n"
               "color_to_add.a = 1.0;\n"
               "uv_coords = attr_uv;\n"
               "}\n";
    }

    static const char *getSimpleFragShader()
    {
        return "precision mediump sampler2D;\n"
               "precision mediump float;\n"
               "varying vec4 color_to_add;\n"
               "void main()\n"
               "{\n"
               "gl_FragColor=color_to_add;\n"
               "}\n";
    }

    static const char *getSimpleFragShaderTex()
    {
        return "precision mediump sampler2D;\n"
               "precision mediump float;\n"
               "uniform sampler2D image0;\n"
               "varying vec2 uv_coords;\n"
               "void main()\n"
               "{\n"
               "gl_FragColor=vec4(texture2D( image0, uv_coords ).xyz, 1.0 );\n"
               "}\n";
    }

    static QSharedPointer<QDemonRenderInputAssembler> createBox(QDemonRenderContext *context,
                                                                QSharedPointer<QDemonRenderVertexBuffer> &outVertexBuffer,
                                                                QSharedPointer<QDemonRenderIndexBuffer> &outIndexBuffer);
    static QSharedPointer<QDemonRenderShaderProgram> createSimpleShader(QDemonRenderContext *context);
    static QSharedPointer<QDemonRenderShaderProgram> createSimpleShaderTex(QDemonRenderContext *context);
};

#endif
