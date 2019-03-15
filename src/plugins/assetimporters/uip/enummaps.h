#ifndef ENUMMAPS_H
#define ENUMMAPS_H

#include "uippresentation.h"

QT_BEGIN_NAMESPACE

struct EnumNameMap
{
    int value;
    const char *str;
};

template <typename T>
struct EnumParseMap
{
};

template <>
struct EnumParseMap<UipPresentation::Rotation>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<Node::RotationOrder>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<Node::Orientation>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<Slide::PlayMode>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<Slide::InitialPlayState>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<Slide::PlayThrough>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<AnimationTrack::AnimationType>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<LayerNode::ProgressiveAA>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<LayerNode::MultisampleAA>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<LayerNode::LayerBackground>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<LayerNode::BlendType>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<LayerNode::HorizontalFields>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<LayerNode::Units>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<LayerNode::VerticalFields>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<Image::MappingMode>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<Image::TilingMode>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<ModelNode::Tessellation>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<CameraNode::ScaleMode>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<CameraNode::ScaleAnchor>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<LightNode::LightType>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<DefaultMaterial::ShaderLighting>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<DefaultMaterial::BlendMode>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<DefaultMaterial::SpecularModel>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<TextNode::HorizontalAlignment>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<TextNode::VerticalAlignment>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<TextNode::WordWrap>
{
    static EnumNameMap *get();
};

template <>
struct EnumParseMap<TextNode::Elide>
{
    static EnumNameMap *get();
};

class EnumMap
{
public:
    template <typename T>
    static bool enumFromStr(const QStringRef &str, T *v) {
        QByteArray ba = str.toUtf8();
        EnumNameMap *nameMap = EnumParseMap<T>::get();
        for ( ; nameMap->str; ++nameMap) {
            if (!strcmp(nameMap->str, ba.constData())) {
                *v = static_cast<T>(nameMap->value);
                return true;
            }
        }
        return false;
    }
    template <typename T>
    static const char *strFromEnum(T v) {
        EnumNameMap *nameMap = EnumParseMap<T>::get();
        for ( ; nameMap->str; ++nameMap) {
            if (nameMap->value == v)
                return nameMap->str;
        }
        return nullptr;
    }
};

QT_END_NAMESPACE
#endif // ENUMMAPS_H
