#include "qdemonqmlutilities_p.h"

#include <QVector3D>
#include <QDebug>

QT_BEGIN_NAMESPACE

namespace QDemonQmlUtilities {

QString insertTabs(int n)
{
    QString tabs;
    for (int i = 0; i < n; ++i)
        tabs += "    ";
    return tabs;
}

QString qmlComponentName(const QString &name) {
    QString nameCopy = name;
    if (nameCopy.isEmpty())
        return QStringLiteral("Presentation");

    if (nameCopy[0].isLower())
        nameCopy[0] = nameCopy[0].toUpper();

    return nameCopy;
}

QString colorToQml(const QColor &color) {
    QString colorString;
    colorString = QStringLiteral("Qt.rgba(") + QString::number(color.redF()) +
            QStringLiteral(", ") + QString::number(color.greenF()) +
            QStringLiteral(", ") + QString::number(color.blueF()) +
            QStringLiteral(", ") + QString::number(color.alphaF()) +
            QStringLiteral(")");
    return colorString;
}

QString sanitizeQmlId(const QString &id)
{

    if (id.isEmpty())
        return QString();
    QString idCopy = id;
    // sometimes first letter is a #
    if (idCopy.startsWith("#"))
        idCopy.remove(0, 1);

    // imported files have < > for certain items
    idCopy.remove('<');
    idCopy.remove('>');

    // replace "."s with _
    idCopy.replace(".", "_");

    // You can even have " " (space) characters in uip files...
    idCopy.replace(" ", "_");

    // Materials sometimes use directory stuff in their name...
    idCopy.replace("/", "_");

    // first letter of id can not be upper case
    if (idCopy[0].isUpper())
        idCopy[0] = idCopy[0].toLower();

    // ### qml keywords as names
    if (idCopy == "default" ||
            idCopy == "alias" ||
            idCopy == "readonly" ||
            idCopy == "property" ||
            idCopy == "signal"||
            idCopy == "int") {
        idCopy += "_";
    }

    return idCopy;
}

QString sanitizeQmlSourcePath(const QString &source)
{
    QString sourceCopy = source;

    sourceCopy.replace("\\", "/");

    // must be surrounded in quotes
    return QString(QStringLiteral("\"") + sourceCopy + QStringLiteral("\""));
}

PropertyMap *PropertyMap::instance()
{
    static PropertyMap p;
    return &p;
}

PropertyMap::PropertiesMap *PropertyMap::propertiesForType(PropertyMap::Type type)
{
    if (m_properties.contains(type))
        return m_properties[type];

    return nullptr;
}

QVariant PropertyMap::getDefaultValue(PropertyMap::Type type, const QString &property)
{
    QVariant value;

    if (m_properties.contains(type)) {
        auto properties = m_properties[type];
        if (properties->contains(property))
            value = properties->value(property);
    }

    return value;
}

bool PropertyMap::isDefaultValue(PropertyMap::Type type, const QString &property, const QVariant &value)
{
    bool isTheSame = value == getDefaultValue(type, property);
    return isTheSame;
}

PropertyMap::PropertyMap()
{
    // Node
    PropertiesMap *node = new PropertiesMap;
    node->insert(QStringLiteral("x"), 0);
    node->insert(QStringLiteral("y"), 0);
    node->insert(QStringLiteral("z"), 0);
    node->insert(QStringLiteral("position"), QVector3D(0, 0, 0));
    node->insert(QStringLiteral("rotation"), QVector3D(0, 0, 0));
    node->insert(QStringLiteral("scale"), QVector3D(1, 1, 1));
    node->insert(QStringLiteral("pivot"), QVector3D(0, 0, 0));
    node->insert(QStringLiteral("opacity"), 1.0);
    node->insert(QStringLiteral("rotationOrder"), QStringLiteral("DemonNode.YXZ"));
    node->insert(QStringLiteral("orientation"), QStringLiteral("DemonNode.LeftHanded"));
    node->insert(QStringLiteral("visible"), true);
    m_properties.insert(Type::Node, node);

    // Model
    PropertiesMap *model = new PropertiesMap;
    model->insert(QStringLiteral("skeletonRoot"), -1);
    model->insert(QStringLiteral("tesselationMode"), QStringLiteral("DemonModel.NoTess"));
    model->insert(QStringLiteral("edgeTess"), 1);
    model->insert(QStringLiteral("innerTess"), 1);
    m_properties.insert(Type::Model, model);

    // Camera
    PropertiesMap *camera = new PropertiesMap;
    camera->insert(QStringLiteral("projectionMode"), QStringLiteral("DemonCamera.Perspective"));
    camera->insert(QStringLiteral("clipNear"), 10.0f);
    camera->insert(QStringLiteral("clipFar"), 10000.0f);
    camera->insert(QStringLiteral("fieldOfView"), 60.0f);
    camera->insert(QStringLiteral("isFieldOFViewHorizontal"), false);
    camera->insert(QStringLiteral("scaleMode"), QStringLiteral("DemonCamera.Fit"));
    camera->insert(QStringLiteral("scaleAnchor"), QStringLiteral("DemonCamera.Center"));
    m_properties.insert(Type::Camera, camera);

    // Light
    PropertiesMap *light = new PropertiesMap;
    light->insert(QStringLiteral("lightType"), QStringLiteral("DemonLight.Directional"));
    light->insert(QStringLiteral("diffuseColor"), QColor(Qt::white));
    light->insert(QStringLiteral("specularColor"), QColor(Qt::white));
    light->insert(QStringLiteral("ambientColor"), QColor(Qt::black));
    light->insert(QStringLiteral("brightness"), 100.0f);
    light->insert(QStringLiteral("linearFade"), 0.0f);
    light->insert(QStringLiteral("exponentialFade"), 0.0f);
    light->insert(QStringLiteral("areaWidth"), 0.0f);
    light->insert(QStringLiteral("areaHeight"), 0.0f);
    light->insert(QStringLiteral("castShadow"), false);
    light->insert(QStringLiteral("shadowBias"), 0.0f);
    light->insert(QStringLiteral("shadowFactor"), 5.0f);
    light->insert(QStringLiteral("shadowMapResolution"), 9);
    light->insert(QStringLiteral("shadowMapFar"), 5000.0f);
    light->insert(QStringLiteral("shadowMapFieldOfView"), 90.0f);
    light->insert(QStringLiteral("shadowFilter"), 35.0f);
    m_properties.insert(Type::Light, light);

    // DefaultMaterial
    PropertiesMap *defaultMaterial = new PropertiesMap;
    defaultMaterial->insert(QStringLiteral("lighting"), QStringLiteral("DemonDefaultMaterial.VertexLighting"));
    defaultMaterial->insert(QStringLiteral("blendMode"), QStringLiteral("DemonDefaultMaterial.Normal"));
    defaultMaterial->insert(QStringLiteral("diffuseColor"), QColor(Qt::white));
    defaultMaterial->insert(QStringLiteral("emissivePower"), 0.0f);
    defaultMaterial->insert(QStringLiteral("emissiveColor"), QColor(Qt::white));
    defaultMaterial->insert(QStringLiteral("specularModel"), QStringLiteral("DemonDefaultMaterial.Default"));
    defaultMaterial->insert(QStringLiteral("specularTint"), QColor(Qt::white));
    defaultMaterial->insert(QStringLiteral("indexOfRefraction"), 0.2f);
    defaultMaterial->insert(QStringLiteral("fresnelPower"), 0.0f);
    defaultMaterial->insert(QStringLiteral("specularAmount"), 1.0f);
    defaultMaterial->insert(QStringLiteral("specularRoughness"), 50.0f);
    defaultMaterial->insert(QStringLiteral("opacity"), 1.0f);
    defaultMaterial->insert(QStringLiteral("bumpAmount"), 0.0f);
    defaultMaterial->insert(QStringLiteral("translucentFalloff"), 0.0f);
    defaultMaterial->insert(QStringLiteral("diffuseLightWrap"), 0.0f);
    defaultMaterial->insert(QStringLiteral("vertexColors"), false);
    defaultMaterial->insert(QStringLiteral("displacementAmount"), 0.0f);

    m_properties.insert(Type::DefaultMaterial, defaultMaterial);

    // Image
    PropertiesMap *image = new PropertiesMap;
    image->insert(QStringLiteral("scaleU"), 1.0f);
    image->insert(QStringLiteral("scaleV"), 1.0f);
    image->insert(QStringLiteral("mappingMode"), QStringLiteral("DemonImage.Normal"));
    image->insert(QStringLiteral("tilingModeHorizontal"), QStringLiteral("DemonImage.ClampToEdge"));
    image->insert(QStringLiteral("tilingModeVertical"), QStringLiteral("DemonImage.ClampToEdge"));
    image->insert(QStringLiteral("rotationUV"), 0.0f);
    image->insert(QStringLiteral("positionU"), 0.0f);
    image->insert(QStringLiteral("positionV"), 0.0f);
    image->insert(QStringLiteral("pivotU"), 0.0f);
    image->insert(QStringLiteral("pivotV"), 0.0f);
    m_properties.insert(Type::Image, image);
}

PropertyMap::~PropertyMap()
{
    for (auto proprtyMap : m_properties.values())
        delete proprtyMap;
}

void writeQmlPropertyHelper(QTextStream &output, int tabLevel, PropertyMap::Type type, const QString &propertyName, const QVariant &value)
{
    if (!PropertyMap::instance()->propertiesForType(type)->contains(propertyName)) {
        qWarning() << "property: " << propertyName << " not found";
        return;
    }

    auto defualtValue = PropertyMap::instance()->propertiesForType(type)->value(propertyName);

    if ((defualtValue != value)) {
        QString valueString = value.toString();
        if (value.type() == QVariant::Color) {
            valueString = QDemonQmlUtilities::colorToQml(value.value<QColor>());
        } else if (value.type() == QVariant::Vector3D) {
            QVector3D v = value.value<QVector3D>();
            valueString = QStringLiteral("Qt.vector3d(") + QString::number(double(v.x())) +
                    QStringLiteral(", ") + QString::number(double(v.y())) +
                    QStringLiteral(", ") + QString::number(double(v.z())) + QStringLiteral(")");
        }


        output << QDemonQmlUtilities::insertTabs(tabLevel) << propertyName << ": " << valueString << endl;
    }

}


}

QT_END_NAMESPACE
