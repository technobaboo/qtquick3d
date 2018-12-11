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
#ifndef QDEMON_RENDER_UIP_SHARED_TRANSLATION_H
#define QDEMON_RENDER_UIP_SHARED_TRANSLATION_H
#include <QtDemonRuntimeRender/qdemonrenderlight.h>
#include <QtDemonRuntimeRender/qdemonrendercamera.h>
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterial.h>
#include <QtDemonRuntimeRender/qdemonrenderimage.h>
#include <QtDemonRuntimeRender/qdemonrendertext.h>
#include <QtDemonRuntimeRender/qdemonrenderlayer.h>
#include <QtDemonRuntimeRender/qdemonrendermodel.h>
#include <QtDemonRuntimeRender/qdemonrenderpath.h>
#include <QtDemonRuntimeRender/qdemonrenderpresentation.h>

QT_BEGIN_NAMESPACE


template <typename TEnumType>
struct SEnumParseMap
{
};

struct SEnumNameMap
{
    quint32 m_Enum;
    const wchar_t *m_WideName;
    const char *m_Name;
};

template <>
struct SEnumParseMap<RenderLightTypes::Enum>
{
    static SEnumNameMap *GetMap();
};

template <>
struct SEnumParseMap<DefaultMaterialLighting::Enum>
{
    static SEnumNameMap *GetMap();
};

template <>
struct SEnumParseMap<DefaultMaterialBlendMode::Enum>
{
    static SEnumNameMap *GetMap();
};

template <>
struct SEnumParseMap<ImageMappingModes::Enum>
{
    static SEnumNameMap *GetMap();
};

template <>
struct SEnumParseMap<QDemonRenderTextureCoordOp::Enum>
{
    static SEnumNameMap *GetMap();
};

template <>
struct SEnumParseMap<TextHorizontalAlignment::Enum>
{
    static SEnumNameMap *GetMap();
};

template <>
struct SEnumParseMap<TextVerticalAlignment::Enum>
{
    static SEnumNameMap *GetMap();
};

template <>
struct SEnumParseMap<AAModeValues::Enum>
{
    static SEnumNameMap *GetMap();
};

template <>
struct SEnumParseMap<LayerBlendTypes::Enum>
{
    static SEnumNameMap *GetMap();
};

template <>
struct SEnumParseMap<RenderRotationValues::Enum>
{
    static SEnumNameMap *GetMap();
};

template <>
struct SEnumParseMap<CameraScaleModes::Enum>
{
    static SEnumNameMap *GetMap();
};

template <>
struct SEnumParseMap<CameraScaleAnchors::Enum>
{
    static SEnumNameMap *GetMap();
};
template <>
struct SEnumParseMap<HorizontalFieldValues::Enum>
{
    static SEnumNameMap *GetMap();
};
template <>
struct SEnumParseMap<VerticalFieldValues::Enum>
{
    static SEnumNameMap *GetMap();
};
template <>
struct SEnumParseMap<LayerUnitTypes::Enum>
{
    static SEnumNameMap *GetMap();
};

template <>
struct SEnumParseMap<LayerBackground::Enum>
{
    static SEnumNameMap *GetMap();
};

template <>
struct SEnumParseMap<DefaultMaterialSpecularModel::Enum>
{
    static SEnumNameMap *GetMap();
};

template <>
struct SEnumParseMap<TessModeValues::Enum>
{
    static SEnumNameMap *GetMap();
};

template <>
struct SEnumParseMap<PathCapping::Enum>
{
    static SEnumNameMap *GetMap();
};

template <>
struct SEnumParseMap<PathTypes::Enum>
{
    static SEnumNameMap *GetMap();
};

template <>
struct SEnumParseMap<PathPaintStyles::Enum>
{
    static SEnumNameMap *GetMap();
};

#define QDEMON_RENDER_WCHAR_T_XYZs L"XYZ"
#define QDEMON_RENDER_WCHAR_T_YZXs L"YZX"
#define QDEMON_RENDER_WCHAR_T_ZXYs L"ZXY"
#define QDEMON_RENDER_WCHAR_T_XZYs L"XZY"
#define QDEMON_RENDER_WCHAR_T_YXZs L"YXZ"
#define QDEMON_RENDER_WCHAR_T_ZYXs L"ZYX"

#define QDEMON_RENDER_WCHAR_T_XYZr L"XYZr"
#define QDEMON_RENDER_WCHAR_T_YZXr L"YZXr"
#define QDEMON_RENDER_WCHAR_T_ZXYr L"ZXYr"
#define QDEMON_RENDER_WCHAR_T_XZYr L"XZYr"
#define QDEMON_RENDER_WCHAR_T_YXZr L"YXZr"
#define QDEMON_RENDER_WCHAR_T_ZYXr L"ZYXr"

#define QDEMON_RENDER_CHAR_T_XYZs "XYZ"
#define QDEMON_RENDER_CHAR_T_YZXs "YZX"
#define QDEMON_RENDER_CHAR_T_ZXYs "ZXY"
#define QDEMON_RENDER_CHAR_T_XZYs "XZY"
#define QDEMON_RENDER_CHAR_T_YXZs "YXZ"
#define QDEMON_RENDER_CHAR_T_ZYXs "ZYX"

#define QDEMON_RENDER_CHAR_T_XYZr "XYZr"
#define QDEMON_RENDER_CHAR_T_YZXr "YZXr"
#define QDEMON_RENDER_CHAR_T_ZXYr "ZXYr"
#define QDEMON_RENDER_CHAR_T_XZYr "XZYr"
#define QDEMON_RENDER_CHAR_T_YXZr "YXZr"
#define QDEMON_RENDER_CHAR_T_ZYXr "ZYXr"

inline quint32 MapRotationOrder(const wchar_t *inOrderStr)
{
#define MAP_ROTATION_ORDER(name, postfix)                                                          \
    if (wcscmp(inOrderStr, QDEMON_RENDER_WCHAR_T_##name##postfix) == 0) {                             \
    return EulOrd##name##postfix;                                                              \
}
    MAP_ROTATION_ORDER(XYZ, s);
    MAP_ROTATION_ORDER(YZX, s);
    MAP_ROTATION_ORDER(ZXY, s);
    MAP_ROTATION_ORDER(XZY, s);
    MAP_ROTATION_ORDER(YXZ, s);
    MAP_ROTATION_ORDER(ZYX, s);
    MAP_ROTATION_ORDER(XYZ, r);
    MAP_ROTATION_ORDER(YZX, r);
    MAP_ROTATION_ORDER(ZXY, r);
    MAP_ROTATION_ORDER(XZY, r);
    MAP_ROTATION_ORDER(YXZ, r);
    MAP_ROTATION_ORDER(ZYX, r);
#undef MAP_ROTATION_ORDER
    return EulOrdYXZs;
}

inline quint32 MapRotationOrder(const char *inOrderStr)
{
#define MAP_ROTATION_ORDER(name, postfix)                                                          \
    if (strcmp(inOrderStr, QDEMON_RENDER_CHAR_T_##name##postfix) == 0) {                              \
    return EulOrd##name##postfix;                                                              \
}
    MAP_ROTATION_ORDER(XYZ, s);
    MAP_ROTATION_ORDER(YZX, s);
    MAP_ROTATION_ORDER(ZXY, s);
    MAP_ROTATION_ORDER(XZY, s);
    MAP_ROTATION_ORDER(YXZ, s);
    MAP_ROTATION_ORDER(ZYX, s);
    MAP_ROTATION_ORDER(XYZ, r);
    MAP_ROTATION_ORDER(YZX, r);
    MAP_ROTATION_ORDER(ZXY, r);
    MAP_ROTATION_ORDER(XZY, r);
    MAP_ROTATION_ORDER(YXZ, r);
    MAP_ROTATION_ORDER(ZYX, r);
#undef MAP_ROTATION_ORDER
    return EulOrdYXZs;
}

// the goal is to unify the systems that transfer information into the UICRender library.
// There are currently three such systems; the runtime, studio, and the uip loader that loads
// uip files
// directly into the render library.
// To do this, we need to have a mapping between a generic key and a given property on every
// object
// along with some information about what portion of the object model this property affects.

struct QDemonRenderDirtyFlags
{
    enum Enum {
        Unknown = 0,
        Dirty = 1 << 0,
        TransformDirty = 1 << 1,
        TextDirty = 1 << 2,
    };
};

// Now we build out generic macros with no implementation that list all of the properties
// on each struct that we care about.  We will fill in these macros with implementation later.
// Each macro will list the property name along with what dirty operation should get marked
// Global parse tables that list every property used by the system.

#define ITERATE_QDEMON_RENDER_SCENE_PROPERTIES                                                        \
    HANDLE_QDEMON_RENDER_COLOR_PROPERTY(Scene, ClearColor, Dirty)                                     \
    HANDLE_QDEMON_RENDER_PROPERTY(Scene, UseClearColor, Dirty)

#define ITERATE_QDEMON_RENDER_NODE_PROPERTIES                                                         \
    HANDLE_QDEMON_RENDER_TRANSFORM_VEC3_PROPERTY(Node, Rotation, TransformDirty)                      \
    HANDLE_QDEMON_RENDER_VEC3_RADIAN_PROPERTY(Node, Rotation, TransformDirty)                         \
    HANDLE_QDEMON_RENDER_TRANSFORM_VEC3_PROPERTY(Node, Position, TransformDirty)                      \
    HANDLE_QDEMON_RENDER_VEC3_PROPERTY(Node, Position, TransformDirty)                                \
    HANDLE_QDEMON_RENDER_TRANSFORM_VEC3_PROPERTY(Node, Scale, TransformDirty)                         \
    HANDLE_QDEMON_RENDER_VEC3_PROPERTY(Node, Scale, TransformDirty)                                   \
    HANDLE_QDEMON_RENDER_TRANSFORM_VEC3_PROPERTY(Node, Pivot, TransformDirty)                         \
    HANDLE_QDEMON_RENDER_VEC3_PROPERTY(Node, Pivot, TransformDirty)                                   \
    HANDLE_QDEMON_RENDER_OPACITY_PROPERTY(Node, LocalOpacity, TransformDirty)                         \
    HANDLE_QDEMON_ROTATION_ORDER_PROPERTY(Node, RotationOrder, TransformDirty)                        \
    HANDLE_QDEMON_NODE_ORIENTATION_PROPERTY(Node, LeftHanded, TransformDirty)

#define ITERATE_QDEMON_RENDER_LAYER_PROPERTIES                                                        \
    HANDLE_QDEMON_NODE_FLAGS_INVERSE_PROPERTY(Layer, LayerEnableDepthTest, Dirty)                     \
    HANDLE_QDEMON_NODE_FLAGS_INVERSE_PROPERTY(Layer, LayerEnableDepthPrePass, Dirty)                  \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Layer, ProgressiveAAMode, Dirty)                               \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Layer, MultisampleAAMode, Dirty)                               \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, TemporalAAEnabled, Dirty)                                    \
    HANDLE_QDEMON_RENDER_COLOR_PROPERTY(Layer, ClearColor, Dirty)                                     \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Layer, BlendType, Dirty)                                       \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Layer, Background, Dirty)                                      \
    HANDLE_QDEMON_RENDER_SOURCEPATH_PROPERTY(Layer, TexturePath, Dirty)                               \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Layer, HorizontalFieldValues, Dirty)                           \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, Left, Dirty)                                                 \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Layer, LeftUnits, Dirty)                                       \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, Width, Dirty)                                                \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Layer, WidthUnits, Dirty)                                      \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, Right, Dirty)                                                \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Layer, RightUnits, Dirty)                                      \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Layer, VerticalFieldValues, Dirty)                             \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, Top, Dirty)                                                  \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Layer, TopUnits, Dirty)                                        \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, Height, Dirty)                                               \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Layer, HeightUnits, Dirty)                                     \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, Bottom, Dirty)                                               \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Layer, BottomUnits, Dirty)                                     \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, AoStrength, Dirty)                                           \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, AoDistance, Dirty)                                           \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, AoSoftness, Dirty)                                           \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, AoBias, Dirty)                                               \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, AoDither, Dirty)                                             \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, ShadowStrength, Dirty)                                       \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, ShadowDist, Dirty)                                           \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, ShadowSoftness, Dirty)                                       \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, ShadowBias, Dirty)                                           \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, LightProbe, Dirty)                                           \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, ProbeBright, Dirty)                                          \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, FastIbl, Dirty)                                              \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, ProbeHorizon, Dirty)                                         \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, ProbeFov, Dirty)                                             \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, LightProbe2, Dirty)                                          \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, Probe2Fade, Dirty)                                           \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, Probe2Window, Dirty)                                         \
    HANDLE_QDEMON_RENDER_PROPERTY(Layer, Probe2Pos, Dirty)

#define ITERATE_QDEMON_RENDER_CAMERA_PROPERTIES                                                       \
    HANDLE_QDEMON_RENDER_PROPERTY(Camera, ClipNear, Dirty)                                            \
    HANDLE_QDEMON_RENDER_PROPERTY(Camera, ClipFar, Dirty)                                             \
    HANDLE_QDEMON_RENDER_RADIAN_PROPERTY(Camera, FOV, Dirty)                                          \
    HANDLE_QDEMON_RENDER_PROPERTY(Camera, FOVHorizontal, Dirty)                                          \
    HANDLE_QDEMON_NODE_FLAGS_PROPERTY(Camera, Orthographic, Dirty)                                    \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Camera, ScaleMode, Dirty)                                      \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Camera, ScaleAnchor, Dirty)

#define ITERATE_QDEMON_RENDER_LIGHT_PROPERTIES                                                        \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Light, LightType, Dirty)                                       \
    HANDLE_QDEMON_RENDER_PROPERTY(Light, Scope, Dirty)                                                \
    HANDLE_QDEMON_RENDER_COLOR_VEC3_PROPERTY(Light, DiffuseColor, Dirty)                              \
    HANDLE_QDEMON_RENDER_COLOR_PROPERTY(Light, DiffuseColor, Dirty)                                   \
    HANDLE_QDEMON_RENDER_COLOR_VEC3_PROPERTY(Light, SpecularColor, Dirty)                             \
    HANDLE_QDEMON_RENDER_COLOR_PROPERTY(Light, SpecularColor, Dirty)                                  \
    HANDLE_QDEMON_RENDER_COLOR_VEC3_PROPERTY(Light, AmbientColor, Dirty)                              \
    HANDLE_QDEMON_RENDER_COLOR_PROPERTY(Light, AmbientColor, Dirty)                                   \
    HANDLE_QDEMON_RENDER_PROPERTY(Light, Brightness, Dirty)                                           \
    HANDLE_QDEMON_RENDER_PROPERTY(Light, LinearFade, Dirty)                                           \
    HANDLE_QDEMON_RENDER_PROPERTY(Light, ExponentialFade, Dirty)                                      \
    HANDLE_QDEMON_RENDER_PROPERTY(Light, AreaWidth, Dirty)                                            \
    HANDLE_QDEMON_RENDER_PROPERTY(Light, AreaHeight, Dirty)                                           \
    HANDLE_QDEMON_RENDER_PROPERTY(Light, CastShadow, Dirty)                                           \
    HANDLE_QDEMON_RENDER_PROPERTY(Light, ShadowBias, Dirty)                                           \
    HANDLE_QDEMON_RENDER_PROPERTY(Light, ShadowFactor, Dirty)                                         \
    HANDLE_QDEMON_RENDER_PROPERTY(Light, ShadowMapFar, Dirty)                                         \
    HANDLE_QDEMON_RENDER_PROPERTY(Light, ShadowMapFov, Dirty)                                         \
    HANDLE_QDEMON_RENDER_PROPERTY(Light, ShadowFilter, Dirty)

#define ITERATE_QDEMON_RENDER_MODEL_PROPERTIES                                                        \
    HANDLE_QDEMON_RENDER_SOURCEPATH_PROPERTY(Model, MeshPath, Dirty)                                  \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Model, TessellationMode, Dirty)                                \
    HANDLE_QDEMON_RENDER_PROPERTY(Model, EdgeTess, Dirty)                                             \
    HANDLE_QDEMON_RENDER_PROPERTY(Model, InnerTess, Dirty)

#define ITERATE_QDEMON_RENDER_CUSTOM_MATERIAL_PROPERTIES                                              \
    HANDLE_QDEMON_RENDER_PROPERTY(MaterialBase, IblProbe, Dirty)

#define ITERATE_QDEMON_RENDER_LIGHTMAP_PROPERTIES                                                     \
    HANDLE_QDEMON_RENDER_PROPERTY(Lightmaps, LightmapIndirect, Dirty)                                 \
    HANDLE_QDEMON_RENDER_PROPERTY(Lightmaps, LightmapRadiosity, Dirty)                                \
    HANDLE_QDEMON_RENDER_PROPERTY(Lightmaps, LightmapShadow, Dirty)

#define ITERATE_QDEMON_RENDER_MATERIAL_PROPERTIES                                                     \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Material, Lighting, Dirty)                                     \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Material, BlendMode, Dirty)                                    \
    HANDLE_QDEMON_RENDER_PROPERTY(Material, VertexColors, Dirty)                                      \
    HANDLE_QDEMON_RENDER_PROPERTY(MaterialBase, IblProbe, Dirty)                                      \
    HANDLE_QDEMON_RENDER_COLOR_VEC3_PROPERTY(Material, DiffuseColor, Dirty)                           \
    HANDLE_QDEMON_RENDER_COLOR_PROPERTY(Material, DiffuseColor, Dirty)                                \
    HANDLE_QDEMON_RENDER_ARRAY_PROPERTY(Material, DiffuseMaps, 0, Dirty)                              \
    HANDLE_QDEMON_RENDER_ARRAY_PROPERTY(Material, DiffuseMaps, 1, Dirty)                              \
    HANDLE_QDEMON_RENDER_ARRAY_PROPERTY(Material, DiffuseMaps, 2, Dirty)                              \
    HANDLE_QDEMON_RENDER_PROPERTY(Material, EmissivePower, Dirty)                                     \
    HANDLE_QDEMON_RENDER_COLOR_VEC3_PROPERTY(Material, EmissiveColor, Dirty)                          \
    HANDLE_QDEMON_RENDER_COLOR_PROPERTY(Material, EmissiveColor, Dirty)                               \
    HANDLE_QDEMON_RENDER_PROPERTY(Material, EmissiveMap, Dirty)                                       \
    HANDLE_QDEMON_RENDER_PROPERTY(Material, EmissiveMap2, Dirty)                                      \
    HANDLE_QDEMON_RENDER_PROPERTY(Material, SpecularReflection, Dirty)                                \
    HANDLE_QDEMON_RENDER_PROPERTY(Material, SpecularMap, Dirty)                                       \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Material, SpecularModel, Dirty)                                \
    HANDLE_QDEMON_RENDER_COLOR_VEC3_PROPERTY(Material, SpecularTint, Dirty)                           \
    HANDLE_QDEMON_RENDER_COLOR_PROPERTY(Material, SpecularTint, Dirty)                                \
    HANDLE_QDEMON_RENDER_PROPERTY(Material, FresnelPower, Dirty)                                      \
    HANDLE_QDEMON_RENDER_PROPERTY(Material, IOR, Dirty)                                               \
    HANDLE_QDEMON_RENDER_PROPERTY(Material, SpecularAmount, Dirty)                                    \
    HANDLE_QDEMON_RENDER_PROPERTY(Material, SpecularRoughness, Dirty)                                 \
    HANDLE_QDEMON_RENDER_PROPERTY(Material, RoughnessMap, Dirty)                                      \
    HANDLE_QDEMON_RENDER_OPACITY_PROPERTY(Material, Opacity, Dirty)                                   \
    HANDLE_QDEMON_RENDER_PROPERTY(Material, OpacityMap, Dirty)                                        \
    HANDLE_QDEMON_RENDER_PROPERTY(Material, BumpMap, Dirty)                                           \
    HANDLE_QDEMON_RENDER_PROPERTY(Material, BumpAmount, Dirty)                                        \
    HANDLE_QDEMON_RENDER_PROPERTY(Material, NormalMap, Dirty)                                         \
    HANDLE_QDEMON_RENDER_PROPERTY(Material, DisplacementMap, Dirty)                                   \
    HANDLE_QDEMON_RENDER_PROPERTY(Material, DisplaceAmount, Dirty)                                    \
    HANDLE_QDEMON_RENDER_PROPERTY(Material, TranslucencyMap, Dirty)                                   \
    HANDLE_QDEMON_RENDER_PROPERTY(Material, TranslucentFalloff, Dirty)                                \
    HANDLE_QDEMON_RENDER_PROPERTY(Material, DiffuseLightWrap, Dirty)

#define ITERATE_QDEMON_RENDER_REFERENCED_MATERIAL_PROPERTIES                                          \
    HANDLE_QDEMON_RENDER_PROPERTY(Material, ReferencedMaterial, Dirty)

#define ITERATE_QDEMON_RENDER_IMAGE_PROPERTIES                                                        \
    HANDLE_QDEMON_RENDER_SOURCEPATH_PROPERTY(Image, ImagePath, Dirty)                                 \
    HANDLE_QDEMON_RENDER_PROPERTY(Image, OffscreenRendererId, Dirty)                                  \
    HANDLE_QDEMON_RENDER_VEC2_PROPERTY(Image, Scale, TransformDirty)                                  \
    HANDLE_QDEMON_RENDER_VEC2_PROPERTY(Image, Pivot, TransformDirty)                                  \
    HANDLE_QDEMON_RENDER_RADIAN_PROPERTY(Image, Rotation, TransformDirty)                             \
    HANDLE_QDEMON_RENDER_VEC2_PROPERTY(Image, Position, TransformDirty)                               \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Image, MappingMode, Dirty)                                     \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Image, HorizontalTilingMode, Dirty)                            \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Image, VerticalTilingMode, Dirty)

#define ITERATE_QDEMON_RENDER_TEXT_PROPERTIES                                                         \
    HANDLE_QDEMON_RENDER_PROPERTY(Text, Text, TextDirty)                                              \
    HANDLE_QDEMON_RENDER_PROPERTY(Text, Font, TextDirty)                                              \
    HANDLE_QDEMON_RENDER_PROPERTY(Text, FontSize, TextDirty)                                          \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Text, HorizontalAlignment, TextDirty)                          \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Text, VerticalAlignment, TextDirty)                            \
    HANDLE_QDEMON_RENDER_PROPERTY(Text, Leading, TextDirty)                                           \
    HANDLE_QDEMON_RENDER_PROPERTY(Text, Tracking, TextDirty)                                          \
    HANDLE_QDEMON_RENDER_PROPERTY(Text, DropShadow, TextDirty)                                        \
    HANDLE_QDEMON_RENDER_PROPERTY(Text, DropShadowStrength, TextDirty)                                \
    HANDLE_QDEMON_RENDER_PROPERTY(Text, DropShadowOffset, TextDirty)                                  \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Text, DropShadowHorizontalAlignment, TextDirty)                \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Text, DropShadowVerticalAlignment, TextDirty)                  \
    HANDLE_QDEMON_RENDER_COLOR_VEC3_PROPERTY(Text, TextColor, Dirty)                                  \
    HANDLE_QDEMON_RENDER_COLOR_PROPERTY(Text, TextColor, Dirty)                                       \
    HANDLE_QDEMON_RENDER_PROPERTY(Text, EnableAcceleratedFont, Dirty)

#define ITERATE_QDEMON_RENDER_PATH_PROPERTIES                                                         \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Path, PathType, Dirty)                                         \
    HANDLE_QDEMON_RENDER_PROPERTY(Path, Width, Dirty)                                                 \
    HANDLE_QDEMON_RENDER_PROPERTY(Path, LinearError, Dirty)                                           \
    HANDLE_QDEMON_RENDER_PROPERTY(Path, EdgeTessAmount, Dirty)                                        \
    HANDLE_QDEMON_RENDER_PROPERTY(Path, InnerTessAmount, Dirty)                                       \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Path, BeginCapping, Dirty)                                     \
    HANDLE_QDEMON_RENDER_PROPERTY(Path, BeginCapOffset, Dirty)                                        \
    HANDLE_QDEMON_RENDER_PROPERTY(Path, BeginCapOpacity, Dirty)                                       \
    HANDLE_QDEMON_RENDER_PROPERTY(Path, BeginCapWidth, Dirty)                                         \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Path, EndCapping, Dirty)                                       \
    HANDLE_QDEMON_RENDER_PROPERTY(Path, EndCapOffset, Dirty)                                          \
    HANDLE_QDEMON_RENDER_PROPERTY(Path, EndCapOpacity, Dirty)                                         \
    HANDLE_QDEMON_RENDER_PROPERTY(Path, EndCapWidth, Dirty)                                           \
    HANDLE_QDEMON_RENDER_ENUM_PROPERTY(Path, PaintStyle, Dirty)                                       \
    HANDLE_QDEMON_RENDER_SOURCEPATH_PROPERTY(Path, PathBuffer, Dirty)

#define ITERATE_QDEMON_RENDER_PATH_SUBPATH_PROPERTIES                                                 \
    HANDLE_QDEMON_RENDER_PROPERTY(SubPath, Closed, Dirty)

QT_END_NAMESPACE

#endif
