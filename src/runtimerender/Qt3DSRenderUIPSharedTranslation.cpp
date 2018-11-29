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
#include <Qt3DSRenderUIPSharedTranslation.h>

namespace qt3ds {
namespace render {

#define WCHAR_T_Directional L"Directional"
#define WCHAR_T_Point L"Point"
#define WCHAR_T_Area L"Area"
#define WCHAR_T_None L"None"
#define WCHAR_T_Vertex L"Vertex"
#define WCHAR_T_Pixel L"Pixel"
#define WCHAR_T_Normal L"Normal"
#define WCHAR_T_Screen L"Screen"
#define WCHAR_T_Multiply L"Multiply"
#define WCHAR_T_Overlay L"Overlay"
#define WCHAR_T_ColorBurn L"ColorBurn"
#define WCHAR_T_ColorDodge L"ColorDodge"
#define WCHAR_T_Add L"Add"
#define WCHAR_T_Subtract L"Subtract"
#define WCHAR_T_UV_Mapping L"UV Mapping"
#define WCHAR_T_Environmental_Mapping L"Environmental Mapping"
#define WCHAR_T_Light_Probe L"Light Probe"
#define WCHAR_T_No_Tiling L"No Tiling"
#define WCHAR_T_Mirrored L"Mirrored"
#define WCHAR_T_Tiled L"Tiled"
#define WCHAR_T_Left L"Left"
#define WCHAR_T_Center L"Center"
#define WCHAR_T_Right L"Right"
#define WCHAR_T_Top L"Top"
#define WCHAR_T_Middle L"Middle"
#define WCHAR_T_Bottom L"Bottom"
#define WCHAR_T_2x L"2x"
#define WCHAR_T_4x L"4x"
#define WCHAR_T_8x L"8x"
#define WCHAR_T_SSAA L"SSAA"
#define WCHAR_T_NoRotation L"NoRotation"
#define WCHAR_T_Clockwise90 L"90"
#define WCHAR_T_Clockwise180 L"180"
#define WCHAR_T_Clockwise270 L"270"
#define WCHAR_T_Fit L"Fit"
#define WCHAR_T_Same_Size L"Same Size"
#define WCHAR_T_CENTER L"Center"
#define WCHAR_T_North L"N"
#define WCHAR_T_NorthEast L"NE"
#define WCHAR_T_East L"E"
#define WCHAR_T_SouthEast L"SE"
#define WCHAR_T_South L"S"
#define WCHAR_T_SouthWest L"SW"
#define WCHAR_T_West L"W"
#define WCHAR_T_NorthWest L"NW"
#define WCHAR_T_LeftWidth L"Left/Width"
#define WCHAR_T_LeftRight L"Left/Right"
#define WCHAR_T_WidthRight L"Width/Right"
#define WCHAR_T_TopHeight L"Top/Height"
#define WCHAR_T_TopBottom L"Top/Bottom"
#define WCHAR_T_HeightBottom L"Height/Bottom"
#define WCHAR_T_Percent L"percent"
#define WCHAR_T_Pixels L"pixels"
#define WCHAR_T_Fit_Horizontal L"Fit Horizontal"
#define WCHAR_T_Fit_Vertical L"Fit Vertical"
#define WCHAR_T_Default L"Default"
#define WCHAR_T_KGGX L"KGGX"
#define WCHAR_T_KWard L"KWard"
#define WCHAR_T_Transparent L"Transparent"
#define WCHAR_T_Unspecified L"Unspecified"
#define WCHAR_T_Color L"SolidColor"
#define WCHAR_T_Linear L"Linear"
#define WCHAR_T_Phong L"Phong"
#define WCHAR_T_NPatch L"NPatch"
#define WCHAR_T_Taper L"Taper"
#define WCHAR_T_Geometry L"Geometry"
#define WCHAR_T_Painted L"Painted"
#define WCHAR_T_Filled L"Filled"
#define WCHAR_T_Stroked L"Stroked"
#define WCHAR_T_FilledAndStroked L"Filled and Stroked"
#define WCHAR_T_Simple L"Simple"
#define WCHAR_T_Smoke L"Smoke"
#define WCHAR_T_Cloud L"Cloud"
#define WCHAR_T_Fluid L"Fluid"
#define WCHAR_T_User L"User"

#define CHAR_T_Directional "Directional"
#define CHAR_T_Point "Point"
#define CHAR_T_Area "Area"
#define CHAR_T_None "None"
#define CHAR_T_Vertex "Vertex"
#define CHAR_T_Pixel "Pixel"
#define CHAR_T_Normal "Normal"
#define CHAR_T_Screen "Screen"
#define CHAR_T_Multiply "Multiply"
#define CHAR_T_Overlay "Overlay"
#define CHAR_T_ColorBurn "ColorBurn"
#define CHAR_T_ColorDodge "ColorDodge"
#define CHAR_T_Add "Add"
#define CHAR_T_Subtract "Subtract"
#define CHAR_T_UV_Mapping "UV Mapping"
#define CHAR_T_Environmental_Mapping "Environmental Mapping"
#define CHAR_T_Light_Probe "Light Probe"
#define CHAR_T_No_Tiling "No Tiling"
#define CHAR_T_Mirrored "Mirrored"
#define CHAR_T_Tiled "Tiled"
#define CHAR_T_Left "Left"
#define CHAR_T_Center "Center"
#define CHAR_T_Right "Right"
#define CHAR_T_Top "Top"
#define CHAR_T_Middle "Middle"
#define CHAR_T_Bottom "Bottom"
#define CHAR_T_2x "2x"
#define CHAR_T_4x "4x"
#define CHAR_T_8x "8x"
#define CHAR_T_SSAA "SSAA"
#define CHAR_T_NoRotation "NoRotation"
#define CHAR_T_Clockwise90 "90"
#define CHAR_T_Clockwise180 "180"
#define CHAR_T_Clockwise270 "270"
#define CHAR_T_Fit "Fit"
#define CHAR_T_Same_Size "Same Size"
#define CHAR_T_CENTER "Center"
#define CHAR_T_North "N"
#define CHAR_T_NorthEast "NE"
#define CHAR_T_East "E"
#define CHAR_T_SouthEast "SE"
#define CHAR_T_South "S"
#define CHAR_T_SouthWest "SW"
#define CHAR_T_West "W"
#define CHAR_T_NorthWest "NW"
#define CHAR_T_LeftWidth "Left/Width"
#define CHAR_T_LeftRight "Left/Right"
#define CHAR_T_WidthRight "Width/Right"
#define CHAR_T_TopHeight "Top/Height"
#define CHAR_T_TopBottom "Top/Bottom"
#define CHAR_T_HeightBottom "Height/Bottom"
#define CHAR_T_Percent "percent"
#define CHAR_T_Pixels "pixels"
#define CHAR_T_Fit_Horizontal "Fit Horizontal"
#define CHAR_T_Fit_Vertical "Fit Vertical"
#define CHAR_T_Default "Default"
#define CHAR_T_KGGX "KGGX"
#define CHAR_T_KWard "KWard"
#define CHAR_T_Transparent "Transparent"
#define CHAR_T_Unspecified "Unspecified"
#define CHAR_T_Color "SolidColor"
#define CHAR_T_Linear "Linear"
#define CHAR_T_Phong "Phong"
#define CHAR_T_NPatch "NPatch"
#define CHAR_T_Taper "Taper"
#define CHAR_T_Geometry "Geometry"
#define CHAR_T_Painted "Painted"
#define CHAR_T_Filled "Filled"
#define CHAR_T_Stroked "Stroked"
#define CHAR_T_FilledAndStroked "Filled and Stroked"
#define CHAR_T_Simple "Simple"
#define CHAR_T_Smoke "Smoke"
#define CHAR_T_Cloud "Cloud"
#define CHAR_T_Fluid "Fluid"
#define CHAR_T_User "User"

#define DEFINE_NAME_MAP_ENTRY(enumval, name)                                                       \
    {                                                                                              \
        enumval, WCHAR_T_##name, CHAR_T_##name                                                     \
    }
    SEnumNameMap g_LightTypesMap[] = {
        DEFINE_NAME_MAP_ENTRY(RenderLightTypes::Directional, Directional),
        DEFINE_NAME_MAP_ENTRY(RenderLightTypes::Point, Point),
        DEFINE_NAME_MAP_ENTRY(RenderLightTypes::Area, Area),
        { (quint32)-1, nullptr },
    };

    SEnumNameMap g_MaterialLightingMap[] = {
        DEFINE_NAME_MAP_ENTRY(DefaultMaterialLighting::NoLighting, None),
        DEFINE_NAME_MAP_ENTRY(DefaultMaterialLighting::VertexLighting, Vertex),
        DEFINE_NAME_MAP_ENTRY(DefaultMaterialLighting::FragmentLighting, Pixel),
        { (quint32)-1, nullptr },
    };

    SEnumNameMap g_BlendModeMap[] = {
        DEFINE_NAME_MAP_ENTRY(DefaultMaterialBlendMode::Normal, Normal),
        DEFINE_NAME_MAP_ENTRY(DefaultMaterialBlendMode::Screen, Screen),
        DEFINE_NAME_MAP_ENTRY(DefaultMaterialBlendMode::Multiply, Multiply),
        DEFINE_NAME_MAP_ENTRY(DefaultMaterialBlendMode::Overlay, Overlay),
        DEFINE_NAME_MAP_ENTRY(DefaultMaterialBlendMode::ColorBurn, ColorBurn),
        DEFINE_NAME_MAP_ENTRY(DefaultMaterialBlendMode::ColorDodge, ColorDodge),
        { (quint32)-1, nullptr },
    };

    SEnumNameMap g_ImageMappingModeMap[] = {
        DEFINE_NAME_MAP_ENTRY(ImageMappingModes::Normal, UV_Mapping),
        DEFINE_NAME_MAP_ENTRY(ImageMappingModes::Environment, Environmental_Mapping),
        DEFINE_NAME_MAP_ENTRY(ImageMappingModes::LightProbe, Light_Probe),
        { (quint32)-1, nullptr },
    };

    SEnumNameMap g_RenderTextureCoordOpMap[] = {
        DEFINE_NAME_MAP_ENTRY(QDemonRenderTextureCoordOp::ClampToEdge, No_Tiling),
        DEFINE_NAME_MAP_ENTRY(QDemonRenderTextureCoordOp::MirroredRepeat, Mirrored),
        DEFINE_NAME_MAP_ENTRY(QDemonRenderTextureCoordOp::Repeat, Tiled),
        { (quint32)-1, nullptr },
    };

    SEnumNameMap g_TextHorizontalAlignmentMap[] = {
        DEFINE_NAME_MAP_ENTRY(TextHorizontalAlignment::Left, Left),
        DEFINE_NAME_MAP_ENTRY(TextHorizontalAlignment::Center, Center),
        DEFINE_NAME_MAP_ENTRY(TextHorizontalAlignment::Right, Right),
        { (quint32)-1, nullptr },
    };

    SEnumNameMap g_TextVerticalAlignmentMap[] = {
        DEFINE_NAME_MAP_ENTRY(TextVerticalAlignment::Top, Top),
        DEFINE_NAME_MAP_ENTRY(TextVerticalAlignment::Middle, Middle),
        DEFINE_NAME_MAP_ENTRY(TextVerticalAlignment::Bottom, Bottom),
        { (quint32)-1, nullptr },
    };

    SEnumNameMap g_ProgressiveAAValuesMap[] = {
        DEFINE_NAME_MAP_ENTRY(AAModeValues::NoAA, None),
        DEFINE_NAME_MAP_ENTRY(AAModeValues::SSAA, SSAA),
        DEFINE_NAME_MAP_ENTRY(AAModeValues::X2, 2x),
        DEFINE_NAME_MAP_ENTRY(AAModeValues::X4, 4x),
        DEFINE_NAME_MAP_ENTRY(AAModeValues::X8, 8x),
        { (quint32)-1, nullptr },
    };

    SEnumNameMap g_LayerBlendTypesMap[] = {
        DEFINE_NAME_MAP_ENTRY(LayerBlendTypes::Normal, Normal),
        DEFINE_NAME_MAP_ENTRY(LayerBlendTypes::Screen, Screen),
        DEFINE_NAME_MAP_ENTRY(LayerBlendTypes::Multiply, Multiply),
        DEFINE_NAME_MAP_ENTRY(LayerBlendTypes::Add, Add),
        DEFINE_NAME_MAP_ENTRY(LayerBlendTypes::Subtract, Subtract),
        DEFINE_NAME_MAP_ENTRY(LayerBlendTypes::Overlay, Overlay),
        DEFINE_NAME_MAP_ENTRY(LayerBlendTypes::ColorBurn, ColorBurn),
        DEFINE_NAME_MAP_ENTRY(LayerBlendTypes::ColorDodge, ColorDodge),
        { (quint32)-1, nullptr },
    };

    SEnumNameMap g_RenderRotationValuesMap[] = {
        DEFINE_NAME_MAP_ENTRY(RenderRotationValues::NoRotation, None),
        DEFINE_NAME_MAP_ENTRY(RenderRotationValues::Clockwise90, Clockwise90),
        DEFINE_NAME_MAP_ENTRY(RenderRotationValues::Clockwise180, Clockwise180),
        DEFINE_NAME_MAP_ENTRY(RenderRotationValues::Clockwise270, Clockwise270),
        { (quint32)-1, nullptr },
    };

    SEnumNameMap g_CameraScaleModesMap[] = {
        DEFINE_NAME_MAP_ENTRY(CameraScaleModes::Fit, Fit),
        DEFINE_NAME_MAP_ENTRY(CameraScaleModes::SameSize, Same_Size),
        DEFINE_NAME_MAP_ENTRY(CameraScaleModes::FitHorizontal, Fit_Horizontal),
        DEFINE_NAME_MAP_ENTRY(CameraScaleModes::FitVertical, Fit_Vertical),
        { (quint32)-1, nullptr },
    };

    SEnumNameMap g_CameraScaleAnchorsMap[] = {
        DEFINE_NAME_MAP_ENTRY(CameraScaleAnchors::Center, Center),
        DEFINE_NAME_MAP_ENTRY(CameraScaleAnchors::North, North),
        DEFINE_NAME_MAP_ENTRY(CameraScaleAnchors::NorthEast, NorthEast),
        DEFINE_NAME_MAP_ENTRY(CameraScaleAnchors::East, East),
        DEFINE_NAME_MAP_ENTRY(CameraScaleAnchors::SouthEast, SouthEast),
        DEFINE_NAME_MAP_ENTRY(CameraScaleAnchors::South, South),
        DEFINE_NAME_MAP_ENTRY(CameraScaleAnchors::SouthWest, SouthWest),
        DEFINE_NAME_MAP_ENTRY(CameraScaleAnchors::West, West),
        DEFINE_NAME_MAP_ENTRY(CameraScaleAnchors::NorthWest, NorthWest),
        { (quint32)-1, nullptr },
    };

    SEnumNameMap g_HorizontalFieldValuesMap[] = {
        DEFINE_NAME_MAP_ENTRY(HorizontalFieldValues::LeftWidth, LeftWidth),
        DEFINE_NAME_MAP_ENTRY(HorizontalFieldValues::LeftRight, LeftRight),
        DEFINE_NAME_MAP_ENTRY(HorizontalFieldValues::WidthRight, WidthRight),
        { (quint32)-1, nullptr },
    };

    SEnumNameMap g_VerticalFieldValuesMap[] = {
        DEFINE_NAME_MAP_ENTRY(VerticalFieldValues::TopHeight, TopHeight),
        DEFINE_NAME_MAP_ENTRY(VerticalFieldValues::TopBottom, TopBottom),
        DEFINE_NAME_MAP_ENTRY(VerticalFieldValues::HeightBottom, HeightBottom),
        { (quint32)-1, nullptr },
    };

    SEnumNameMap g_LayerUnitTypesMap[] = {
        DEFINE_NAME_MAP_ENTRY(LayerUnitTypes::Percent, Percent),
        DEFINE_NAME_MAP_ENTRY(LayerUnitTypes::Pixels, Pixels),
        { (quint32)-1, nullptr },
    };

    SEnumNameMap g_LayerBackgroundMap[] = {
        DEFINE_NAME_MAP_ENTRY(LayerBackground::Transparent, Transparent),
        DEFINE_NAME_MAP_ENTRY(LayerBackground::Unspecified, Unspecified),
        DEFINE_NAME_MAP_ENTRY(LayerBackground::Color, Color),
        { (quint32)-1, nullptr },
    };

    SEnumNameMap g_SpecularTypesMap[] = {
        DEFINE_NAME_MAP_ENTRY(DefaultMaterialSpecularModel::Default, Default),
        DEFINE_NAME_MAP_ENTRY(DefaultMaterialSpecularModel::KGGX, KGGX),
        DEFINE_NAME_MAP_ENTRY(DefaultMaterialSpecularModel::KWard, KWard),
        { (quint32)-1, nullptr },
    };

    SEnumNameMap g_TessellationValuesMap[] = {
        DEFINE_NAME_MAP_ENTRY(TessModeValues::NoTess, None),
        DEFINE_NAME_MAP_ENTRY(TessModeValues::TessLinear, Linear),
        DEFINE_NAME_MAP_ENTRY(TessModeValues::TessPhong, Phong),
        DEFINE_NAME_MAP_ENTRY(TessModeValues::TessNPatch, NPatch),
        { (quint32)-1, nullptr },
    };

    SEnumNameMap g_PathCappingValuesMap[] = {
        DEFINE_NAME_MAP_ENTRY(PathCapping::Noner, None),
        DEFINE_NAME_MAP_ENTRY(PathCapping::Taper, Taper),
        { (quint32)-1, nullptr },
    };

    SEnumNameMap g_PathTypesMap[] = {
        DEFINE_NAME_MAP_ENTRY(PathTypes::Noner, None),
        DEFINE_NAME_MAP_ENTRY(PathTypes::Painted, Painted),
        DEFINE_NAME_MAP_ENTRY(PathTypes::Geometry, Geometry),
        { (quint32)-1, nullptr },
    };

    SEnumNameMap g_PathPaintStylesMap[] = {
        DEFINE_NAME_MAP_ENTRY(PathPaintStyles::Noner, None),
        DEFINE_NAME_MAP_ENTRY(PathPaintStyles::FilledAndStroked, FilledAndStroked),
        DEFINE_NAME_MAP_ENTRY(PathPaintStyles::Filled, Filled),
        DEFINE_NAME_MAP_ENTRY(PathPaintStyles::Stroked, Stroked),
        { (quint32)-1, nullptr },
    };

    SEnumNameMap *SEnumParseMap<RenderLightTypes::Enum>::GetMap() { return g_LightTypesMap; }

    SEnumNameMap *SEnumParseMap<DefaultMaterialLighting::Enum>::GetMap()
    {
        return g_MaterialLightingMap;
    }

    SEnumNameMap *SEnumParseMap<DefaultMaterialBlendMode::Enum>::GetMap() { return g_BlendModeMap; }

    SEnumNameMap *SEnumParseMap<ImageMappingModes::Enum>::GetMap() { return g_ImageMappingModeMap; }

    SEnumNameMap *SEnumParseMap<QDemonRenderTextureCoordOp::Enum>::GetMap()
    {
        return g_RenderTextureCoordOpMap;
    }

    SEnumNameMap *SEnumParseMap<TextHorizontalAlignment::Enum>::GetMap()
    {
        return g_TextHorizontalAlignmentMap;
    }

    SEnumNameMap *SEnumParseMap<TextVerticalAlignment::Enum>::GetMap()
    {
        return g_TextVerticalAlignmentMap;
    }

    SEnumNameMap *SEnumParseMap<AAModeValues::Enum>::GetMap() { return g_ProgressiveAAValuesMap; }

    SEnumNameMap *SEnumParseMap<LayerBlendTypes::Enum>::GetMap() { return g_LayerBlendTypesMap; }

    SEnumNameMap *SEnumParseMap<RenderRotationValues::Enum>::GetMap()
    {
        return g_RenderRotationValuesMap;
    }

    SEnumNameMap *SEnumParseMap<CameraScaleModes::Enum>::GetMap() { return g_CameraScaleModesMap; }

    SEnumNameMap *SEnumParseMap<CameraScaleAnchors::Enum>::GetMap()
    {
        return g_CameraScaleAnchorsMap;
    }

    SEnumNameMap *SEnumParseMap<HorizontalFieldValues::Enum>::GetMap()
    {
        return g_HorizontalFieldValuesMap;
    }

    SEnumNameMap *SEnumParseMap<VerticalFieldValues::Enum>::GetMap()
    {
        return g_VerticalFieldValuesMap;
    }

    SEnumNameMap *SEnumParseMap<LayerUnitTypes::Enum>::GetMap() { return g_LayerUnitTypesMap; }

    SEnumNameMap *SEnumParseMap<LayerBackground::Enum>::GetMap() { return g_LayerBackgroundMap; }

    SEnumNameMap *SEnumParseMap<DefaultMaterialSpecularModel::Enum>::GetMap()
    {
        return g_SpecularTypesMap;
    }

    SEnumNameMap *SEnumParseMap<TessModeValues::Enum>::GetMap() { return g_TessellationValuesMap; }

    SEnumNameMap *SEnumParseMap<PathCapping::Enum>::GetMap() { return g_PathCappingValuesMap; }

    SEnumNameMap *SEnumParseMap<PathTypes::Enum>::GetMap() { return g_PathTypesMap; }

    SEnumNameMap *SEnumParseMap<PathPaintStyles::Enum>::GetMap() { return g_PathPaintStylesMap; }
}
}
