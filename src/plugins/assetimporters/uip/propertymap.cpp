#include "propertymap.h"

#include "datamodelparser.h"

QT_BEGIN_NAMESPACE

PropertyMap *PropertyMap::instance()
{
    static PropertyMap p;
    return &p;
}

PropertyMap::PropertiesMap* PropertyMap::propertiesForType(GraphObject::Type type)
{
    if (m_properties.contains(type))
        return m_properties[type];

    return nullptr;
}

namespace  {

void insertNodeProperties(PropertyMap::PropertiesMap *node)
{
    node->insert(QStringLiteral("position"), PropertyMap::Property(QStringLiteral("position"), Q3DS::Vector, QVector3D(0, 0, 0)));
    node->insert(QStringLiteral("rotation"), PropertyMap::Property(QStringLiteral("rotation"), Q3DS::Rotation, QVector3D(0, 0, 0)));
    node->insert(QStringLiteral("scale"), PropertyMap::Property(QStringLiteral("scale"), Q3DS::Vector, QVector3D(1.f, 1.f, 1.f)));
    node->insert(QStringLiteral("pivot"), PropertyMap::Property(QStringLiteral("pivot"), Q3DS::Vector, QVector3D(0, 0, 0)));
    node->insert(QStringLiteral("opacity"), PropertyMap::Property(QStringLiteral("opacity"), Q3DS::Float, 1.0f));
    node->insert(QStringLiteral("rotationorder"), PropertyMap::Property(QStringLiteral("rotationorder"), Q3DS::Enum, QStringLiteral("YXZ")));
    node->insert(QStringLiteral("orientation"), PropertyMap::Property(QStringLiteral("orientation"), Q3DS::Enum, QStringLiteral("LeftHanded")));
    node->insert(QStringLiteral("visible"), PropertyMap::Property(QStringLiteral("visible"), Q3DS::Boolean, true));
}

}

PropertyMap::PropertyMap()
{
    // Image
    PropertiesMap *image = new PropertiesMap;
    image->insert(QStringLiteral("scaleu"), Property(QStringLiteral("scaleu"), Q3DS::Float, 1.0f));
    image->insert(QStringLiteral("scalev"), Property(QStringLiteral("scalev"), Q3DS::Float, 1.0f));
    image->insert(QStringLiteral("mappingmode"), Property(QStringLiteral("mappingmode"), Q3DS::Enum, QStringLiteral("Normal")));
    image->insert(QStringLiteral("tilingmodehorz"), Property(QStringLiteral("tilingmodehorz"), Q3DS::Enum, QStringLiteral("ClampToEdge")));
    image->insert(QStringLiteral("tilingmodevert"), Property(QStringLiteral("tilingmodevert"), Q3DS::Enum, QStringLiteral("ClampToEdge")));
    image->insert(QStringLiteral("rotationuv"), Property(QStringLiteral("rotationuv"), Q3DS::Float, 0.0f));
    image->insert(QStringLiteral("positionu"), Property(QStringLiteral("positionu"), Q3DS::Float, 0.0f));
    image->insert(QStringLiteral("positionv"), Property(QStringLiteral("positionv"), Q3DS::Float, 0.0f));
    image->insert(QStringLiteral("pivotu"), Property(QStringLiteral("pivotu"), Q3DS::Float, 0.0f));
    image->insert(QStringLiteral("pivotv"), Property(QStringLiteral("pivotv"), Q3DS::Float, 0.0f));
    m_properties.insert(GraphObject::Image, image);
    // Group
    PropertiesMap *node = new PropertiesMap;;
    insertNodeProperties(node);
    m_properties.insert(GraphObject::Group, node);

    // Layer
    PropertiesMap *layer = new PropertiesMap;
    layer->insert(QStringLiteral("progressiveaa"), Property(QStringLiteral("progressiveAAMode"), Q3DS::Enum, QStringLiteral("NoAA")));
    layer->insert(QStringLiteral("multisampleaa"), Property(QStringLiteral("multisampleAAMode"), Q3DS::Enum, QStringLiteral("NoAA")));
    layer->insert(QStringLiteral("background"), Property(QStringLiteral("backgroundMode"), Q3DS::Enum, QStringLiteral("Transparent")));
    layer->insert(QStringLiteral("clearColor"), Property(QStringLiteral("clearColor"), Q3DS::Color, QColor(0, 0, 0)));
    layer->insert(QStringLiteral("backgroundcolor"), Property(QStringLiteral("backgroundMode"), Q3DS::Enum, QStringLiteral("Transparent")));
    layer->insert(QStringLiteral("blendtype"), Property(QStringLiteral("blendType"), Q3DS::Enum, QStringLiteral("Normal")));
    layer->insert(QStringLiteral("horzfields"), Property(QStringLiteral("horizontalFieldValue"), Q3DS::Enum, QStringLiteral("LeftWidth")));
    layer->insert(QStringLiteral("vertfields"), Property(QStringLiteral("verticalFieldValue"), Q3DS::Enum, QStringLiteral("TopHeight")));
    layer->insert(QStringLiteral("leftunits"), Property(QStringLiteral("leftUnits"), Q3DS::Enum, QStringLiteral("Percent")));
    layer->insert(QStringLiteral("rightunits"), Property(QStringLiteral("rightUnits"), Q3DS::Enum, QStringLiteral("Percent")));
    layer->insert(QStringLiteral("bottomunits"), Property(QStringLiteral("bottomUnits"), Q3DS::Enum, QStringLiteral("Percent")));
    layer->insert(QStringLiteral("topunits"), Property(QStringLiteral("topUnits"), Q3DS::Enum, QStringLiteral("Percent")));
    layer->insert(QStringLiteral("widthunits"), Property(QStringLiteral("widthUnits"), Q3DS::Enum, QStringLiteral("Percent")));
    layer->insert(QStringLiteral("heightunits"), Property(QStringLiteral("heightUnits"), Q3DS::Enum, QStringLiteral("Percent")));
    layer->insert(QStringLiteral("left"), Property(QStringLiteral("left"), Q3DS::Float, 0.0f));
    layer->insert(QStringLiteral("right"), Property(QStringLiteral("right"), Q3DS::Float, 0.0f));
    layer->insert(QStringLiteral("top"), Property(QStringLiteral("top"), Q3DS::Float, 0.0f));
    layer->insert(QStringLiteral("bottom"), Property(QStringLiteral("bottom"), Q3DS::Float, 0.0f));
    layer->insert(QStringLiteral("width"), Property(QStringLiteral("width"), Q3DS::Float, 0.0f));
    layer->insert(QStringLiteral("height"), Property(QStringLiteral("height"), Q3DS::Float, 0.0f));

    layer->insert(QStringLiteral("aostrength"), Property(QStringLiteral("aoStrength"), Q3DS::Float, 0.0f));
    layer->insert(QStringLiteral("aodistance"), Property(QStringLiteral("aoDistance"), Q3DS::Float, 5.0f));
    layer->insert(QStringLiteral("aosoftness"), Property(QStringLiteral("aoSoftness"), Q3DS::Float, 50.0f));
    layer->insert(QStringLiteral("aodither"), Property(QStringLiteral("aoDither"), Q3DS::Boolean, false));
    layer->insert(QStringLiteral("aosamplerate"), Property(QStringLiteral("aoSampleRate"), Q3DS::Long, 2));
    layer->insert(QStringLiteral("aobias"), Property(QStringLiteral("aoBias"), Q3DS::Float, 0.0f));

    layer->insert(QStringLiteral("shadowstrength"), Property(QStringLiteral("shadowStrength"), Q3DS::Float, 0.0f));
    layer->insert(QStringLiteral("shadowdistance"), Property(QStringLiteral("shadowDistance"), Q3DS::Float, 10.0f));
    layer->insert(QStringLiteral("shadowsoftness"), Property(QStringLiteral("shadowSoftness"), Q3DS::Float, 100.0f));
    layer->insert(QStringLiteral("shadowbias"), Property(QStringLiteral("shadowBias"), Q3DS::Float, 0.0f));

    layer->insert(QStringLiteral("probebright"), Property(QStringLiteral("probeBrightness"), Q3DS::Float, 0.0f));
    layer->insert(QStringLiteral("fastibl"), Property(QStringLiteral("fastIBL"), Q3DS::Boolean, true));
    layer->insert(QStringLiteral("probehorizon"), Property(QStringLiteral("probeHorizon"), Q3DS::Float, -1.0f));
    layer->insert(QStringLiteral("probefov"), Property(QStringLiteral("probeFieldOfView"), Q3DS::Float, 180.0f));

    layer->insert(QStringLiteral("probe2fade"), Property(QStringLiteral("probe2Fade"), Q3DS::Float, 1.0f));
    layer->insert(QStringLiteral("probe2window"), Property(QStringLiteral("probe2Window"), Q3DS::Float, 1.0f));
    layer->insert(QStringLiteral("probe2pos"), Property(QStringLiteral("probe2Postion"), Q3DS::Float, 0.5f));

    layer->insert(QStringLiteral("temporalaa"), Property(QStringLiteral("temporalAAEnabled"), Q3DS::Boolean, false));
    m_properties.insert(GraphObject::Layer, layer);

    // Camera
    // Node (properties)
    PropertiesMap *camera = new PropertiesMap;
    insertNodeProperties(camera);

    camera->insert(QStringLiteral("clipnear"), Property(QStringLiteral("clipNear"), Q3DS::Float, 10.0f));
    camera->insert(QStringLiteral("clipfar"), Property(QStringLiteral("clipFar"), Q3DS::Float, 10000.0f));
    camera->insert(QStringLiteral("fov"), Property(QStringLiteral("fieldOfView"), Q3DS::Float, 60.0f));
    camera->insert(QStringLiteral("fovhorizontal"), Property(QStringLiteral("isFieldOFViewHorizontal"), Q3DS::Boolean, false));
    camera->insert(QStringLiteral("scalemode"), Property(QStringLiteral("scaleMode"), Q3DS::Enum, QStringLiteral("Fit")));
    camera->insert(QStringLiteral("scaleanchor"), Property(QStringLiteral("scaleAnchor"), Q3DS::Enum, QStringLiteral("Center")));
    m_properties.insert(GraphObject::Camera, camera);

    // Light
    PropertiesMap *light = new PropertiesMap;
    insertNodeProperties(light);

    light->insert(QStringLiteral("lighttype"), Property(QStringLiteral("lightType"), Q3DS::Enum, QStringLiteral("Directional")));
    light->insert(QStringLiteral("lightdiffuse"), Property(QStringLiteral("diffuseColor"), Q3DS::Color, QColor(255, 255, 255)));
    light->insert(QStringLiteral("lightspecular"), Property(QStringLiteral("specularColor"), Q3DS::Color, QColor(255, 255, 255)));
    light->insert(QStringLiteral("lightambient"), Property(QStringLiteral("ambientColor"), Q3DS::Color, QColor(0, 0, 0)));
    light->insert(QStringLiteral("brightness"), Property(QStringLiteral("brightness"), Q3DS::Float, 100.0f));
    light->insert(QStringLiteral("linearfade"), Property(QStringLiteral("linearFade"), Q3DS::Float, 0.0f));
    light->insert(QStringLiteral("expfade"), Property(QStringLiteral("exponentialFade"), Q3DS::Float, 0.0f));
    light->insert(QStringLiteral("areawidth"), Property(QStringLiteral("areaWidth"), Q3DS::Float, 0.0f));
    light->insert(QStringLiteral("areaheight"), Property(QStringLiteral("areaHeight"), Q3DS::Float, 0.0f));
    light->insert(QStringLiteral("castshadow"), Property(QStringLiteral("castShadow"), Q3DS::Boolean, false));
    light->insert(QStringLiteral("shdwbias"), Property(QStringLiteral("shadowBias"), Q3DS::Float, 0.0f));
    light->insert(QStringLiteral("shdwfactor"), Property(QStringLiteral("shadowFactor"), Q3DS::Float, 5.0f));
    light->insert(QStringLiteral("shdwmapres"), Property(QStringLiteral("shadowMapResolution"), Q3DS::Long, 9));
    light->insert(QStringLiteral("shdwmapfar"), Property(QStringLiteral("shadowMapFar"), Q3DS::Float, 5000.0f));
    light->insert(QStringLiteral("shdwmapfov"), Property(QStringLiteral("shadowMapFieldOfView"), Q3DS::Float, 90.0f));
    light->insert(QStringLiteral("shdwfilter"), Property(QStringLiteral("shadowFilter"), Q3DS::Float, 35.0f));
    m_properties.insert(GraphObject::Light, light);

    // Model
    PropertiesMap *model = new PropertiesMap;
    insertNodeProperties(model);
    model->insert(QStringLiteral("poseroot"), Property(QStringLiteral("skeletonRoot"), Q3DS::Long, -1));
    model->insert(QStringLiteral("tessellation"), Property(QStringLiteral("tesselationMode"), Q3DS::Enum, QStringLiteral("NoTess")));
    model->insert(QStringLiteral("edgetess"), Property(QStringLiteral("edgeTess"), Q3DS::Float, 1.0f));
    model->insert(QStringLiteral("innertess"), Property(QStringLiteral("innerTess"), Q3DS::Float, 1.0f));
    m_properties.insert(GraphObject::Model, model);

    // Component
    PropertiesMap *component = new PropertiesMap;
    insertNodeProperties(component);
    m_properties.insert(GraphObject::Component, component);

    // DefaultMaterial
    PropertiesMap *defaultMaterial = new PropertiesMap;
    defaultMaterial->insert(QStringLiteral("shaderlighting"), Property(QStringLiteral("lighting"), Q3DS::Enum, QStringLiteral("VertexLighting")));
    defaultMaterial->insert(QStringLiteral("blendode"), Property(QStringLiteral("blendMode"), Q3DS::Enum, QStringLiteral("Normal")));
    defaultMaterial->insert(QStringLiteral("diffuse"), Property(QStringLiteral("diffuseColor"), Q3DS::Color, QColor(Qt::white)));
    defaultMaterial->insert(QStringLiteral("emissivepower"), Property(QStringLiteral("emissivePower"), Q3DS::Float, 0.0f));
    defaultMaterial->insert(QStringLiteral("emissivecolor"), Property(QStringLiteral("emissiveColor"), Q3DS::Color, QColor(Qt::white)));
    defaultMaterial->insert(QStringLiteral("specularmodel"), Property(QStringLiteral("specularModel"), Q3DS::Enum, QStringLiteral("Default")));
    defaultMaterial->insert(QStringLiteral("speculartint"), Property(QStringLiteral("specularTint"), Q3DS::Color, QColor(Qt::white)));
    defaultMaterial->insert(QStringLiteral("ior"), Property(QStringLiteral("indexOfRefraction"), Q3DS::Float, 0.2f));
    defaultMaterial->insert(QStringLiteral("fresnelPower"), Property(QStringLiteral("fresnelPower"), Q3DS::Float, 0.0f));
    defaultMaterial->insert(QStringLiteral("specularamount"), Property(QStringLiteral("specularAmount"), Q3DS::Float, 1.0f));
    defaultMaterial->insert(QStringLiteral("specularroughness"), Property(QStringLiteral("specularRoughness"), Q3DS::Float, 50.0f));
    defaultMaterial->insert(QStringLiteral("opacity"), Property(QStringLiteral("opacity"), Q3DS::Float, 1.0f));
    defaultMaterial->insert(QStringLiteral("bumpamount"), Property(QStringLiteral("bumpAmount"), Q3DS::Float, 0.0f));
    defaultMaterial->insert(QStringLiteral("translucentfalloff"), Property(QStringLiteral("translucentFalloff"), Q3DS::Float, 0.0f));
    defaultMaterial->insert(QStringLiteral("diffuselightwrap"), Property(QStringLiteral("diffuseLightWrap"), Q3DS::Float, 0.0f));
    defaultMaterial->insert(QStringLiteral("vertexColors"), Property(QStringLiteral("vertexColors"), Q3DS::Boolean, false));
    defaultMaterial->insert(QStringLiteral("displacementamount"), Property(QStringLiteral("displacementAmount"), Q3DS::Float, 0.0f));

    m_properties.insert(GraphObject::DefaultMaterial, defaultMaterial);

    // CustomMaterial
    PropertiesMap *customMaterial = new PropertiesMap;
    customMaterial->insert(QStringLiteral("displacementamount"), Property(QStringLiteral("displacementAmount"), Q3DS::Float, 0.0f));

    m_properties.insert(GraphObject::CustomMaterial, customMaterial);

    // ReferenceMaterial
    PropertiesMap *referenceMaterial = new PropertiesMap;
    m_properties.insert(GraphObject::ReferencedMaterial, referenceMaterial);

    // Effect
    PropertiesMap *effect = new PropertiesMap;
    m_properties.insert(GraphObject::Effect, effect);

    // Alias
    PropertiesMap *alias = new PropertiesMap;
    insertNodeProperties(alias);
    m_properties.insert(GraphObject::Alias, alias);
}

PropertyMap::~PropertyMap()
{
    for (auto proprtyMap : m_properties.values())
        delete proprtyMap;
}

QT_END_NAMESPACE

