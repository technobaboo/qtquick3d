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
#ifndef QDEMON_RENDERABLE_IMAGE_H
#define QDEMON_RENDERABLE_IMAGE_H
#include <Qt3DSRender.h>

namespace qt3ds {
namespace render {

    struct ImageMapTypes
    {
        enum Enum {
            Unknown = 0,
            Diffuse = 1,
            Opacity = 2,
            Specular = 3,
            Emissive = 4,
            Bump = 5,
            SpecularAmountMap = 6,
            Normal = 7,
            Displacement = 8,
            Translucency = 9,
            LightmapIndirect = 10,
            LightmapRadiosity = 11,
            LightmapShadow = 12,
            Roughness = 13,
        };
    };

    /**
     *	Some precomputed information on a given image.  When generating a renderable, the shader
     *	generator goes through all the possible images on a material and for each valid image
     *	computes this renderable image and attaches it to the renderable.
     */
    struct SRenderableImage
    {
        ImageMapTypes::Enum m_MapType;
        SImage &m_Image;
        SRenderableImage *m_NextImage;
        SRenderableImage(ImageMapTypes::Enum inMapType, SImage &inImage)
            : m_MapType(inMapType)
            , m_Image(inImage)
            , m_NextImage(nullptr)
        {
        }
    };
}
}
#endif
