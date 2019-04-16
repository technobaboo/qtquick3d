#include "uippresentation.h"
#include "enummaps.h"
#include "datamodelparser.h"
#include "propertymap.h"
#include "utils.h"

QT_BEGIN_NAMESPACE

namespace Q3DS {

bool convertToPropertyType(const QStringRef &value, Q3DS::PropertyType *type, int *componentCount, const char *desc, QXmlStreamReader *reader)
{
    if (componentCount)
        *componentCount = 1;
    bool ok = false;
    if (value == QStringLiteral("StringList")) {
        ok = true;
        *type = Q3DS::StringList;
    } else if (value == QStringLiteral("FloatRange")) {
        ok = true;
        *type = Q3DS::FloatRange;
    } else if (value == QStringLiteral("LongRange")) {
        ok = true;
        *type = Q3DS::LongRange;
    } else if (value == QStringLiteral("Float") || value == QStringLiteral("float")) {
        ok = true;
        *type = Q3DS::Float;
    } else if (value == QStringLiteral("Float2")) {
        ok = true;
        *type = Q3DS::Float2;
        if (componentCount)
            *componentCount = 2;
    } else if (value == QStringLiteral("Long")) {
        ok = true;
        *type = Q3DS::Long;
    } else if (value == QStringLiteral("Matrix4x4")) {
        ok = true;
        *type = Q3DS::Matrix4x4;
        if (componentCount)
            *componentCount = 16;
    } else if (value == QStringLiteral("Vector") || value == QStringLiteral("Float3")) {
        ok = true;
        *type = Q3DS::Vector;
        if (componentCount)
            *componentCount = 3;
    } else if (value == QStringLiteral("Scale")) {
        ok = true;
        *type = Q3DS::Scale;
        if (componentCount)
            *componentCount = 3;
    } else if (value == QStringLiteral("Rotation")) {
        ok = true;
        *type = Q3DS::Rotation;
        if (componentCount)
            *componentCount = 3;
    } else if (value == QStringLiteral("Color")) {
        ok = true;
        *type = Q3DS::Color;
        if (componentCount)
            *componentCount = 3;
    } else if (value == QStringLiteral("Boolean") || value == QStringLiteral("Bool")) {
        ok = true;
        *type = Q3DS::Boolean;
    } else if (value == QStringLiteral("Slide")) {
        ok = true;
        *type = Q3DS::Slide;
    } else if (value == QStringLiteral("Font")) {
        ok = true;
        *type = Q3DS::Font;
    } else if (value == QStringLiteral("FontSize")) {
        ok = true;
        *type = Q3DS::FontSize;
    } else if (value == QStringLiteral("String")) {
        ok = true;
        *type = Q3DS::String;
    } else if (value == QStringLiteral("MultiLineString")) {
        ok = true;
        *type = Q3DS::MultiLineString;
    } else if (value == QStringLiteral("ObjectRef")) {
        ok = true;
        *type = Q3DS::ObjectRef;
    } else if (value == QStringLiteral("Image")) {
        ok = true;
        *type = Q3DS::Image;
    } else if (value == QStringLiteral("Mesh")) {
        ok = true;
        *type = Q3DS::Mesh;
    } else if (value == QStringLiteral("Import")) {
        ok = true;
        *type = Q3DS::Import;
    } else if (value == QStringLiteral("Texture")) {
        ok = true;
        *type = Q3DS::Texture;
    } else if (value == QStringLiteral("Image2D")) {
        ok = true;
        *type = Q3DS::Image2D;
    } else if (value == QStringLiteral("Buffer")) {
        ok = true;
        *type = Q3DS::Buffer;
    } else if (value == QStringLiteral("Guid")) {
        ok = true;
        *type = Q3DS::Guid;
    } else if (value == QStringLiteral("StringListOrInt")) {
        ok = true;
        *type = Q3DS::StringListOrInt;
    } else if (value == QStringLiteral("Renderable")) {
        ok = true;
        *type = Q3DS::String;
    } else if (value == QStringLiteral("PathBuffer")) {
        ok = true;
        *type = Q3DS::String;
    } else if (value == QStringLiteral("ShadowMapResolution")) {
        ok = true;
        *type = Q3DS::Long;
    } else {
        *type = Q3DS::Unknown;
        if (reader)
            reader->raiseError(QObject::tr("Invalid %1 \"%2\"").arg(QString::fromUtf8(desc)).arg(value.toString()));
    }
    return ok;
}

bool convertToInt(const QStringRef &value, int *v, const char *desc, QXmlStreamReader *reader)
{
    if (value.isEmpty()) {
        *v = 0;
        return true;
    }
    bool ok = false;
    *v = value.toInt(&ok);
    if (!ok && reader)
        reader->raiseError(QObject::tr("Invalid %1 \"%2\"").arg(QString::fromUtf8(desc)).arg(value.toString()));
    return ok;
}

bool convertToInt32(const QStringRef &value, qint32 *v, const char *desc, QXmlStreamReader *reader)
{
    if (value.isEmpty()) {
        *v = 0;
        return true;
    }
    int vv;
    bool r = convertToInt(value, &vv, desc, reader);
    if (r)
        *v = qint32(vv);
    return r;
}

bool convertToBool(const QStringRef &value, bool *v, const char *desc, QXmlStreamReader *reader)
{
    Q_UNUSED(desc);
    Q_UNUSED(reader);
    *v = (value == QStringLiteral("True") || value == QStringLiteral("true")
          || value == QStringLiteral("Yes") || value == QStringLiteral("yes")
          || value == QStringLiteral("1"));
    return true;
}

bool convertToFloat(const QStringRef &value, float *v, const char *desc, QXmlStreamReader *reader)
{
    if (value.isEmpty()) {
        *v = 0;
        return true;
    }
    bool ok = false;
    *v = value.toFloat(&ok);
    // Adjust values that are "almost" zero values to 0.0, as we don't really expect values with
    // more then 3-4 decimal points (note that qFuzzyIsNull does allow slightly higher resolution though).
    if (ok && qFuzzyIsNull(*v))
        *v = 0.0f;
    if (!ok && reader)
        reader->raiseError(QObject::tr("Invalid %1 \"%2\"").arg(QString::fromUtf8(desc)).arg(value.toString()));
    return ok;
}

bool convertToVector2D(const QStringRef &value, QVector2D *v, const char *desc, QXmlStreamReader *reader)
{
    QVector<QStringRef> floatStrings = value.split(' ', QString::SkipEmptyParts);
    if (floatStrings.count() != 2) {
        if (reader)
            reader->raiseError(QObject::tr("Invalid %1 \"%2\"").arg(QString::fromUtf8(desc)).arg(value.toString()));
        return false;
    }
    float x;
    float y;
    if (!convertToFloat(floatStrings[0], &x, "Vector2D[x]", reader))
        return false;
    if (!convertToFloat(floatStrings[1], &y, "Vector2D[y]", reader))
        return false;
    v->setX(x);
    v->setY(y);
    return true;
}

bool convertToVector3D(const QStringRef &value, QVector3D *v, const char *desc, QXmlStreamReader *reader)
{
    QVector<QStringRef> floatStrings = value.split(' ', QString::SkipEmptyParts);
    if (floatStrings.count() != 3) {
        if (reader)
            reader->raiseError(QObject::tr("Invalid %1 \"%2\"").arg(QString::fromUtf8(desc)).arg(value.toString()));
        return false;
    }
    float x;
    float y;
    float z;
    if (!convertToFloat(floatStrings[0], &x, "Vector3D[x]", reader))
        return false;
    if (!convertToFloat(floatStrings[1], &y, "Vector3D[y]", reader))
        return false;
    if (!convertToFloat(floatStrings[2], &z, "Vector3D[z]", reader))
        return false;
    v->setX(x);
    v->setY(y);
    v->setZ(z);
    return true;
}

bool convertToMatrix4x4(const QStringRef &value, QMatrix4x4 *v, const char *desc, QXmlStreamReader *reader)
{
    QVector<QStringRef> floatStrings = value.split(' ', QString::SkipEmptyParts);
    if (floatStrings.count() != 16) {
        if (reader)
            reader->raiseError(QObject::tr("Invalid %1 \"%2\"").arg(QString::fromUtf8(desc)).arg(value.toString()));
        return false;
    }

    float m[16];

    for (int i = 0; i < 16; ++i) {
        if (!convertToFloat(floatStrings[i], &m[i], ("Matrix4x4[" + QString::number(i) + "]")
                            .toUtf8().constData(), reader)) {
            return false;
        }
    }

    float *data = v->data();
    for (int i = 0; i < 16; ++i)
        data[i] = m[i];

    return true;
}

int animatablePropertyTypeToMetaType(Q3DS::PropertyType type)
{
    switch (type) {
    case Float:
        return QMetaType::Float;
    case Long:
        return QVariant::Int;
    case Float2:
        return QVariant::Vector2D;
    case Matrix4x4:
        return QVariant::Matrix4x4;
    case Vector:
    case Scale:
    case Rotation:
        return QVariant::Vector3D;
    case Color:
        return QVariant::Color;
    default:
        return QVariant::Invalid;
    }
}

QVariant convertToVariant(const QString &value, Q3DS::PropertyType type)
{
    switch (type) {
    case StringList:
    case Slide:
    case Font:
    case String:
    case MultiLineString:
    case ObjectRef:
    case Image:
    case Mesh:
    case Import:
    case Texture:
    case Image2D:
    case Buffer:
    case Guid:
    case StringListOrInt:
    case Renderable:
    case PathBuffer:
        return value;
    case LongRange:
    case Long:
        return value.toInt();
    case FloatRange:
    case Float:
    case FontSize:
        return value.toFloat();
    case Float2:
    {
        QVector2D v;
        if (convertToVector2D(&value, &v))
            return v;
    }
        break;
    case Matrix4x4:
    {
        QMatrix4x4 v;
        if (convertToMatrix4x4(&value, &v))
            return v;
    }
        break;
    case Vector:
    case Scale:
    case Rotation:
    case Color:
    {
        QVector3D v;
        if (convertToVector3D(&value, &v))
            return v;
    }
        break;
    case Boolean:
    {
        bool v;
        if (convertToBool(&value, &v))
            return v;
    }
        break;
    default:
        break;
    }

    return QVariant();
}

//QVariant convertToVariant(const QString &value, const Q3DSMaterial::PropertyElement &propMeta)
//{
//    switch (propMeta.type) {
//    case Enum:
//    {
//        int idx = propMeta.enumValues.indexOf(value);
//        return idx >= 0 ? idx : 0;
//    }
//    default:
//        return convertToVariant(value, propMeta.type);
//    }
//}

QString convertFromVariant(const QVariant &value)
{
    switch (value.type()) {
    case QVariant::Vector2D:
    {
        const QVector2D v = value.value<QVector2D>();
        return QString(QLatin1String("%1 %2"))
                .arg(QString::number(v.x())).arg(QString::number(v.y()));
    }
    case QVariant::Vector3D:
    {
        const QVector3D v = value.value<QVector3D>();
        return QString(QLatin1String("%1 %2 %3"))
                .arg(QString::number(v.x())).arg(QString::number(v.y())).arg(QString::number(v.z()));
    }
    case QVariant::Color:
    {
        const QColor c = value.value<QColor>();
        const QVector3D v = QVector3D(c.redF(), c.greenF(), c.blueF());
        return QString(QLatin1String("%1 %2 %3"))
                .arg(QString::number(v.x())).arg(QString::number(v.y())).arg(QString::number(v.z()));
    }
    case QVariant::Bool:
        return value.toBool() ? QLatin1String("true") : QLatin1String("false");
    case QVariant::Matrix4x4:
    {
        const QMatrix4x4 v = value.value<QMatrix4x4>();
        const float *data = v.constData();
        return QString(QLatin1String("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12 %13 %14 %15 %16"))
                .arg(QString::number(data[0])).arg(QString::number(data[1])).arg(QString::number(data[2])).arg(QString::number(data[3]))
                .arg(QString::number(data[4])).arg(QString::number(data[5])).arg(QString::number(data[6])).arg(QString::number(data[7]))
                .arg(QString::number(data[8])).arg(QString::number(data[9])).arg(QString::number(data[10])).arg(QString::number(data[11]))
                .arg(QString::number(data[12])).arg(QString::number(data[13])).arg(QString::number(data[14])).arg(QString::number(data[15]));
    }
    default:
        return value.toString();
    }
}

} // namespace Q3DS

namespace {

QString qmlPresentationComponentName(const QString &name) {
    QString nameCopy = name;


    if (nameCopy.isEmpty())
        return QStringLiteral("Default");

    if (nameCopy.startsWith("#"))
        nameCopy.remove(0, 1);

    if (nameCopy.startsWith("materials/"))
        nameCopy.remove("materials/");

    if (nameCopy.startsWith("/"))
        nameCopy.remove(0, 1);

    if (nameCopy[0].isLower())
        nameCopy[0] = nameCopy[0].toUpper();

    return nameCopy;
}

void writeQmlPropertyHelper(QTextStream &output, int tabLevel, GraphObject::Type type, const QString &propertyName, const QVariant &value)
{
    if (!PropertyMap::instance()->propertiesForType(type)->contains(propertyName)) {
        qWarning() << "property: " << propertyName << " not found";
        return;
    }

    auto property = PropertyMap::instance()->propertiesForType(type)->value(propertyName);

    if ((property.defaultValue != value)) {
        QString valueString = value.toString();
        if (value.type() == QVariant::Color) {
            valueString = colorToQml(value.value<QColor>());
        } else if (value.type() == QVariant::Vector3D) {
            QVector3D v = value.value<QVector3D>();
            valueString = QStringLiteral("Qt.vector3d(") + QString::number(double(v.x())) +
                          QStringLiteral(", ") + QString::number(double(v.y())) +
                          QStringLiteral(", ") + QString::number(double(v.z())) + QStringLiteral(")");
        }


        output << insertTabs(tabLevel) << property.name << ": " << valueString << endl;
    }

}

}

PropertyChange PropertyChange::fromVariant(const QString &name, const QVariant &value)
{
    return PropertyChange(name, Q3DS::convertFromVariant(value));
}

void PropertyChangeList::append(const PropertyChange &change)
{
    if (!change.isValid())
        return;

    m_changes.append(change);
    m_keys.insert(change.nameStr());
}

GraphObject::GraphObject(GraphObject::Type type)
    : m_type(type)
{

}

GraphObject::~GraphObject()
{
    destroyGraph();
}

QString GraphObject::typeName() const
{
    switch (m_type) {
    case Type::Asset:
        return QStringLiteral("Asset");
    case Type::Scene:
        return QStringLiteral("Scene");
    case Type::Slide:
        return QStringLiteral("Slide");
    case Type::Image:
        return QStringLiteral("Image");
    case Type::DefaultMaterial:
        return QStringLiteral("DefaultMaterial");
    case Type::ReferencedMaterial:
        return QStringLiteral("ReferencedMaterial");
    case Type::CustomMaterial:
        return QStringLiteral("CustomMaterial");
    case Type::Effect:
        return QStringLiteral("Effect");
    case Type::Behavior:
        return QStringLiteral("Behavior");
    case Type::Layer:
        return QStringLiteral("Layer");
    case Type::Camera:
        return QStringLiteral("Camera");
    case Type::Light:
        return QStringLiteral("Light");
    case Type::Model:
        return QStringLiteral("Model");
    case Type::Group:
        return QStringLiteral("Group");
    case Type::Text:
        return QStringLiteral("Text");
    case Type::Component:
        return QStringLiteral("Component");
    case Type::Alias:
        return QStringLiteral("Alias");
    }

    Q_UNREACHABLE();
    return QString();
}

int GraphObject::childCount() const
{
    int count = 0;
    GraphObject *n = m_firstChild;
    while (n) {
        ++count;
        n = n->m_nextSibling;
    }
    return count;
}

GraphObject *GraphObject::childAtIndex(int idx) const
{
    GraphObject *n = m_firstChild;
    while (idx && n) {
        --idx;
        n = n->m_nextSibling;
    }
    return n;
}

void GraphObject::removeChildNode(GraphObject *node)
{
    Q_ASSERT(node->parent() == this);

    GraphObject *previous = node->m_previousSibling;
    GraphObject *next = node->m_nextSibling;

    if (previous)
        previous->m_nextSibling = next;
    else
        m_firstChild = next;

    if (next)
        next->m_previousSibling = previous;
    else
        m_lastChild = previous;

    node->m_previousSibling = nullptr;
    node->m_nextSibling = nullptr;
    // now it can be nulled out
    node->m_parent = nullptr;
}

void GraphObject::removeAllChildNodes()
{
    while (m_firstChild) {
        GraphObject *node = m_firstChild;
        m_firstChild = node->m_nextSibling;
        node->m_nextSibling = nullptr;
        if (m_firstChild)
            m_firstChild->m_previousSibling = nullptr;
        else
            m_lastChild = nullptr;
        node->m_parent = nullptr;
    }
}

void GraphObject::prependChildNode(GraphObject *node)
{
    Q_ASSERT_X(!node->m_parent, "GraphObject::prependChildNode", "GraphObject already has a parent");

    if (m_firstChild)
        m_firstChild->m_previousSibling = node;
    else
        m_lastChild = node;

    node->m_nextSibling = m_firstChild;
    m_firstChild = node;
    node->m_parent = this;
}

void GraphObject::appendChildNode(GraphObject *node)
{
    Q_ASSERT_X(!node->m_parent, "GraphObject::appendChildNode", "GraphObject already has a parent");

    if (m_lastChild)
        m_lastChild->m_nextSibling = node;
    else
        m_firstChild = node;

    node->m_previousSibling = m_lastChild;
    m_lastChild = node;
    node->m_parent = this;
}

void GraphObject::insertChildNodeBefore(GraphObject *node, GraphObject *before)
{
    Q_ASSERT_X(!node->m_parent, "GraphObject::insertChildNodeBefore", "GraphObject already has a parent");
    Q_ASSERT_X(before && before->m_parent == this, "GraphObject::insertChildNodeBefore", "The parent of \'before\' is wrong");

    GraphObject *previous = before->m_previousSibling;
    if (previous)
        previous->m_nextSibling = node;
    else
        m_firstChild = node;

    node->m_previousSibling = previous;
    node->m_nextSibling = before;
    before->m_previousSibling = node;
    node->m_parent = this;
}

void GraphObject::insertChildNodeAfter(GraphObject *node, GraphObject *after)
{
    Q_ASSERT_X(!node->m_parent, "GraphObject::insertChildNodeAfter", "GraphObject already has a parent");
    Q_ASSERT_X(after && after->m_parent == this, "GraphObject::insertChildNodeAfter", "The parent of \'after\' is wrong");

    GraphObject *next = after->m_nextSibling;
    if (next)
        next->m_previousSibling = node;
    else
        m_lastChild = node;

    node->m_nextSibling = next;
    node->m_previousSibling = after;
    after->m_nextSibling = node;
    node->m_parent = this;
}

void GraphObject::reparentChildNodesTo(GraphObject *newParent)
{
    for (GraphObject *c = firstChild(); c; c = firstChild()) {
        removeChildNode(c);
        newParent->appendChildNode(c);
    }
}

// The property conversion functions all follow the same pattern:
// 1. Check if a value is provided explicitly in the attribute list.
// 2. Then, when PropSetDefaults is set, see if the metadata provided a default value.
// 3. If all else fails, just return false. This is not fatal (and perfectly normal when PropSetDefaults is not set).

// V is const iterable with name() and value() on iter
template<typename T, typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags,
                   const QString &dataModelTypeName, const QString &propName, Q3DS::PropertyType propType,
                   T *dst, std::function<bool(const QStringRef &, T *v)> convertFunc)
{
    auto it = std::find_if(attrs.cbegin(), attrs.cend(), [propName](const typename V::value_type &v) { return v.name() == propName; });
    if (it != attrs.cend()) {
        const QStringRef v = it->value();
        return convertFunc(it->value(), dst);
    } else if (flags.testFlag(GraphObject::PropSetDefaults)) {
        DataModelParser *dataModelParser = DataModelParser::instance();
        if (dataModelParser) {
            const QVector<DataModelParser::Property> *props = dataModelParser->propertiesForType(dataModelTypeName);
            if (props) {
                auto it = std::find_if(props->cbegin(), props->cend(),
                                       [propName](const DataModelParser::Property &v) { return v.name == propName; });
                if (it != props->cend()) {
                    Q_UNUSED(propType);
                    Q_ASSERT(it->type == propType);
                    return convertFunc(QStringRef(&it->defaultValue), dst);
                }
            }
        }
    }
    return false;
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, bool *dst)
{
    return ::parseProperty<bool>(attrs, flags, typeName, propName, Q3DS::Boolean, dst, [](const QStringRef &s, bool *v) { return Q3DS::convertToBool(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, qint32 *dst)
{
    return ::parseProperty<qint32>(attrs, flags, typeName, propName, Q3DS::Long, dst, [](const QStringRef &s, qint32 *v) { return Q3DS::convertToInt32(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, float *dst)
{
    return ::parseProperty<float>(attrs, flags, typeName, propName, Q3DS::Float, dst, [](const QStringRef &s, float *v) { return Q3DS::convertToFloat(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, QMatrix4x4 *dst)
{
    return ::parseProperty<float>(attrs, flags, typeName, propName, Q3DS::Matrix4x4, dst, [](const QStringRef &s, QMatrix4x4 *v) { return Q3DS::convertToMatrix4x4(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, QVector3D *dst)
{
    return ::parseProperty<QVector3D>(attrs, flags, typeName, propName, Q3DS::Vector, dst, [](const QStringRef &s, QVector3D *v) { return Q3DS::convertToVector3D(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, QColor *dst)
{
    QVector3D rgb;
    bool r = ::parseProperty<QVector3D>(attrs, flags, typeName, propName, Q3DS::Color, &rgb, [](const QStringRef &s, QVector3D *v) { return Q3DS::convertToVector3D(s, v); });
    if (r)
        *dst = QColor::fromRgbF(rgb.x(), rgb.y(), rgb.z());
    return r;
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, QString *dst)
{
    return ::parseProperty<QString>(attrs, flags, typeName, propName, Q3DS::String, dst, [](const QStringRef &s, QString *v) { *v = s.toString(); return true; });
}

template<typename V>
bool parseRotationProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, QVector3D *dst)
{
    return ::parseProperty<QVector3D>(attrs, flags, typeName, propName, Q3DS::Rotation, dst, [](const QStringRef &s, QVector3D *v) { return Q3DS::convertToVector3D(s, v); });
}

template<typename V>
bool parseImageProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, QString *dst)
{
    return ::parseProperty<QString>(attrs, flags, typeName, propName, Q3DS::Image, dst, [](const QStringRef &s, QString *v) { *v = s.toString(); return true; });
}

template<typename V>
bool parseMeshProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, QString *dst)
{
    return ::parseProperty<QString>(attrs, flags, typeName, propName, Q3DS::Mesh, dst, [](const QStringRef &s, QString *v) { *v = s.toString(); return true; });
}

template<typename V>
bool parseObjectRefProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, QString *dst)
{
    return ::parseProperty<QString>(attrs, flags, typeName, propName, Q3DS::ObjectRef, dst, [](const QStringRef &s, QString *v) { *v = s.toString(); return true; });
}

template<typename V>
bool parseMultiLineStringProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, QString *dst)
{
    return ::parseProperty<QString>(attrs, flags, typeName, propName, Q3DS::MultiLineString, dst, [](const QStringRef &s, QString *v) { *v = s.toString(); return true; });
}

template<typename V>
bool parseFontProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, QString *dst)
{
    return ::parseProperty<QString>(attrs, flags, typeName, propName, Q3DS::Font, dst, [](const QStringRef &s, QString *v) { *v = s.toString(); return true; });
}

template<typename V>
bool parseFontSizeProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, float *dst)
{
    return ::parseProperty<float>(attrs, flags, typeName, propName, Q3DS::FontSize, dst, [](const QStringRef &s, float *v) { return Q3DS::convertToFloat(s, v); });
}

template<typename V>
bool parseSizeProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, QVector2D *dst)
{
    return ::parseProperty<QVector2D>(attrs, flags, typeName, propName, Q3DS::Float2, dst, [](const QStringRef &s, QVector2D *v) { return Q3DS::convertToVector2D(s, v); });
}

struct StringOrInt {
    QString s;
    int n;
    bool isInt;
};

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, StringOrInt *dst)
{
    // StringListOrInt -> either an enum value or an int
    QString tmp;
    if (parseProperty<QString>(attrs, flags, typeName, propName, Q3DS::StringListOrInt, &tmp,
                                 [](const QStringRef &s, QString *v) { *v = s.toString(); return true; }))
    {
        bool ok = false;
        int v = tmp.toInt(&ok);
        if (ok) {
            dst->isInt = true;
            dst->n = v;
        } else {
            dst->isInt = false;
            dst->s = tmp;
        }
        return true;
    }
    return false;
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, Node::RotationOrder *dst)
{
    return ::parseProperty<Node::RotationOrder>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, Node::RotationOrder *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, Node::Orientation *dst)
{
    return ::parseProperty<Node::Orientation>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, Node::Orientation *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs,GraphObject:: PropSetFlags flags, const QString &typeName, const QString &propName, Slide::PlayMode *dst)
{
    return ::parseProperty<Slide::PlayMode>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, Slide::PlayMode *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, Slide::InitialPlayState *dst)
{
    return ::parseProperty<Slide::InitialPlayState>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, Slide::InitialPlayState *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, LayerNode::ProgressiveAA *dst)
{
    return ::parseProperty<LayerNode::ProgressiveAA>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, LayerNode::ProgressiveAA *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, LayerNode::MultisampleAA *dst)
{
    return ::parseProperty<LayerNode::MultisampleAA>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, LayerNode::MultisampleAA *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, LayerNode::LayerBackground *dst)
{
    return ::parseProperty<LayerNode::LayerBackground>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, LayerNode::LayerBackground *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, LayerNode::BlendType *dst)
{
    return ::parseProperty<LayerNode::BlendType>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, LayerNode::BlendType *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, LayerNode::HorizontalFields *dst)
{
    return ::parseProperty<LayerNode::HorizontalFields>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, LayerNode::HorizontalFields *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, LayerNode::Units *dst)
{
    return ::parseProperty<LayerNode::Units>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, LayerNode::Units *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, LayerNode::VerticalFields *dst)
{
    return ::parseProperty<LayerNode::VerticalFields>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, LayerNode::VerticalFields *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, Image::MappingMode *dst)
{
    return ::parseProperty<Image::MappingMode>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, Image::MappingMode *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, Image::TilingMode *dst)
{
    return ::parseProperty<Image::TilingMode>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, Image::TilingMode *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, ModelNode::Tessellation *dst)
{
    return ::parseProperty<ModelNode::Tessellation>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, ModelNode::Tessellation *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, CameraNode::ScaleMode *dst)
{
    return ::parseProperty<CameraNode::ScaleMode>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, CameraNode::ScaleMode *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, CameraNode::ScaleAnchor *dst)
{
    return ::parseProperty<CameraNode::ScaleAnchor>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, CameraNode::ScaleAnchor *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, LightNode::LightType *dst)
{
    return ::parseProperty<LightNode::LightType>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, LightNode::LightType *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, DefaultMaterial::ShaderLighting *dst)
{
    return ::parseProperty<DefaultMaterial::ShaderLighting>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, DefaultMaterial::ShaderLighting *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, DefaultMaterial::BlendMode *dst)
{
    return ::parseProperty<DefaultMaterial::BlendMode>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, DefaultMaterial::BlendMode *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, DefaultMaterial::SpecularModel *dst)
{
    return ::parseProperty<DefaultMaterial::SpecularModel>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, DefaultMaterial::SpecularModel *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, TextNode::HorizontalAlignment *dst)
{
    return ::parseProperty<TextNode::HorizontalAlignment>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, TextNode::HorizontalAlignment *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, TextNode::VerticalAlignment *dst)
{
    return ::parseProperty<TextNode::VerticalAlignment>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, TextNode::VerticalAlignment *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, TextNode::WordWrap *dst)
{
    return ::parseProperty<TextNode::WordWrap>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, TextNode::WordWrap *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
bool parseProperty(const V &attrs, GraphObject::PropSetFlags flags, const QString &typeName, const QString &propName, TextNode::Elide *dst)
{
    return ::parseProperty<TextNode::Elide>(attrs, flags, typeName, propName, Q3DS::Enum, dst, [](const QStringRef &s, TextNode::Elide *v) { return EnumMap::enumFromStr(s, v); });
}

template<typename V>
void GraphObject::setProps(const V &attrs, PropSetFlags flags)
{
    const QString typeName = QStringLiteral("Asset");
    parseProperty(attrs, flags, typeName, QStringLiteral("starttime"), &m_startTime);
    parseProperty(attrs, flags, typeName, QStringLiteral("endtime"), &m_endTime);

    // name is not parsed here since the data model metadata defines a default
    // value per type, so leave it to the subclasses
}

void GraphObject::setProperties(const QXmlStreamAttributes &attrs, PropSetFlags flags)
{
    setProps(attrs, flags);
}

void GraphObject::applyPropertyChanges(const PropertyChangeList &changeList)
{
    setProps(changeList, 0);
}

void GraphObject::writeQmlFooter(QTextStream &output, int tabLevel)
{
    output << insertTabs(tabLevel) << "}" << endl;
}

namespace {
    QString sanitizeQmlId(const QString &id)
    {

        if (id.isEmpty())
            return QString();
        QString idCopy = id;
        // sometimes first letter is a #
        if (idCopy.startsWith("#"))
            idCopy.remove(0, 1);

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
            idCopy == "signal") {
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
}


QString GraphObject::qmlId()
{
    // does not matter
    return sanitizeQmlId(m_id);
}

void GraphObject::destroyGraph()
{
    if (m_parent) {
        m_parent->removeChildNode(this);
        Q_ASSERT(!m_parent);
    }
    while (m_firstChild) {
        GraphObject *child = m_firstChild;
        removeChildNode(child);
        Q_ASSERT(!child->m_parent);
        delete child;
    }
    Q_ASSERT(!m_firstChild && !m_lastChild);
}

Scene::Scene()
    : GraphObject(GraphObject::Scene)
{

}

Scene::~Scene()
{
    destroyGraph();
}

void Scene::setProperties(const QXmlStreamAttributes &attrs, PropSetFlags flags)
{
    // Asset properties (starttime, endtime) are not in use, hence no base call.

    const QString typeName = QStringLiteral("Scene");
    parseProperty(attrs, flags, typeName, QStringLiteral("bgcolorenable"), &m_useClearColor);
    parseProperty(attrs, flags, typeName, QStringLiteral("backgroundcolor"), &m_clearColor);

    // Different default value.
    parseProperty(attrs, flags, typeName, QStringLiteral("name"), &m_name);
}

void Scene::writeQmlHeader(QTextStream &output, int tabLevel)
{
}

void Scene::writeQmlProperties(QTextStream &output, int tabLevel)
{
}

void Scene::writeQmlFooter(QTextStream &output, int tabLevel)
{
}

Slide::Slide()
    : GraphObject(GraphObject::Slide)
{

}

Slide::~Slide()
{
    if (!parent())
        destroyGraph();

    qDeleteAll(m_propChanges);
}

template<typename V>
void Slide::setProps(const V &attrs, PropSetFlags flags)
{
    const QString typeName = QStringLiteral("Slide");
    parseProperty(attrs, flags, typeName, QStringLiteral("playmode"), &m_playMode);
    parseProperty(attrs, flags, typeName, QStringLiteral("initialplaystate"), &m_initialPlayState);

    StringOrInt pt;
    if (parseProperty(attrs, flags, typeName, QStringLiteral("playthroughto"), &pt)) {
        const bool isRef = (!pt.isInt && pt.s.startsWith(QLatin1Char('#')));
        const bool hasExplicitValue = (pt.isInt || isRef);
        if (hasExplicitValue) {
            m_playThrough = Value;
            m_playThroughValue = isRef ? QVariant::fromValue(pt.s) : QVariant::fromValue(pt.n);
        } else {
            EnumMap::enumFromStr(QStringRef(&pt.s), &m_playThrough);
        }
    }

    // Different default value.
    parseProperty(attrs, flags, typeName, QStringLiteral("name"), &m_name);
}

void Slide::setProperties(const QXmlStreamAttributes &attrs, PropSetFlags flags)
{
    GraphObject::setProperties(attrs, flags);
    setProps(attrs, flags);
}

void Slide::applyPropertyChanges(const PropertyChangeList &changeList)
{
    GraphObject::applyPropertyChanges(changeList);
    setProps(changeList, 0);
}

void Slide::addObject(GraphObject *obj)
{
    m_objects.insert(obj);
}

void Slide::removeObject(GraphObject *obj)
{
    auto it = m_objects.find(obj);
    if (it != m_objects.end()) {
        m_objects.erase(it);
    }
}

void Slide::addPropertyChanges(GraphObject *target, PropertyChangeList *changeList)
{
    m_propChanges.insert(target, changeList);
}

void Slide::removePropertyChanges(GraphObject *target)
{
    delete takePropertyChanges(target);
}

PropertyChangeList *Slide::takePropertyChanges(GraphObject *target)
{
    auto it = m_propChanges.find(target);
    if (it != m_propChanges.end()) {
        PropertyChangeList *propChanges = *it;
        m_propChanges.erase(it);
        return propChanges;
    }
    return nullptr;
}

void Slide::addAnimation(const AnimationTrack &track)
{
    m_anims.append(track);
}

void Slide::removeAnimation(const AnimationTrack &track)
{
    const int idx = m_anims.indexOf(track);
    if (idx >= 0) {
        m_anims.removeAt(idx);
    }
}

void Slide::writeQmlHeader(QTextStream &output, int tabLevel)
{
}

void Slide::writeQmlProperties(QTextStream &output, int tabLevel)
{
}

void Slide::writeQmlFooter(QTextStream &output, int tabLevel)
{
}

Image::Image()
    : GraphObject(GraphObject::Image)
{

}

void Image::setProperties(const QXmlStreamAttributes &attrs, PropSetFlags flags)
{
    GraphObject::setProperties(attrs, flags);
    setProps(attrs, flags);
}

void Image::applyPropertyChanges(const PropertyChangeList &changeList)
{
    GraphObject::applyPropertyChanges(changeList);
    setProps(changeList, 0);
}

void Image::writeQmlHeader(QTextStream &output, int tabLevel)
{
    output << insertTabs(tabLevel) << QStringLiteral("DemonImage {\n");
}

namespace  {
QString mappingModeToString(Image::MappingMode mode)
{
    switch (mode) {
    case Image::EnvironmentalMapping:
        return QStringLiteral("DemonImage.Environment");
    case Image::LightProbe:
    case Image::IBLOverride:
        return QStringLiteral("DemonImage.LightProbe");
    default:
        return QStringLiteral("DemonImage.Normal");
    }
}

QString tilingModeToString(Image::TilingMode mode)
{
    switch (mode) {
    case Image::Tiled:
        return QStringLiteral("DemonImage.Repeat");
    case Image::Mirrored:
        return QStringLiteral("DemonImage.MirroredRepeat");
    case Image::NoTiling:
        return QStringLiteral("DemonImage.ClampToEdge");
    }
}
}

void Image::writeQmlProperties(QTextStream &output, int tabLevel)
{
    output << insertTabs(tabLevel) << QStringLiteral("id: ") << qmlId() << endl;
    output << insertTabs(tabLevel) << QStringLiteral("source: ") <<  sanitizeQmlSourcePath(m_sourcePath) << endl;
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("scaleu"), m_scaleU);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("scalev"), m_scaleV);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("mappingmode"), mappingModeToString(m_mappingMode));
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("tilingmodehorz"), tilingModeToString(m_tilingHoriz));
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("tilingmodevert"), tilingModeToString(m_tilingVert));
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("rotationuv"), m_rotationUV);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("positionu"), m_positionU);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("positionv"), m_positionV);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("pivotu"), m_pivotU);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("pivotv"), m_pivotV);
}

template<typename V>
void Image::setProps(const V &attrs, PropSetFlags flags)
{
    const QString typeName = QStringLiteral("Image");
    parseProperty(attrs, flags, typeName, QStringLiteral("sourcepath"), &m_sourcePath);
    parseProperty(attrs, flags, typeName, QStringLiteral("scaleu"), &m_scaleU);
    parseProperty(attrs, flags, typeName, QStringLiteral("scalev"), &m_scaleV);
    parseProperty(attrs, flags, typeName, QStringLiteral("mappingmode"), &m_mappingMode);
    // Legacy behavior for light probes - default is tiled as opposed to ordinary image.
    // Therefore if a light probe does not have explicit horizontal tiling, set it to tiled.
    if (m_mappingMode == LightProbe || m_mappingMode == IBLOverride) {
        bool res = parseProperty(attrs, 0, typeName,
                                 QStringLiteral("tilingmodehorz"), &m_tilingHoriz);
        if (!res)
            m_tilingHoriz = Tiled;
    } else {
        parseProperty(attrs, flags, typeName, QStringLiteral("tilingmodehorz"), &m_tilingHoriz);
    }
    parseProperty(attrs, flags, typeName, QStringLiteral("tilingmodevert"), &m_tilingVert);
    parseProperty(attrs, flags, typeName, QStringLiteral("rotationuv"), &m_rotationUV);
    parseProperty(attrs, flags, typeName, QStringLiteral("positionu"), &m_positionU);
    parseProperty(attrs, flags, typeName, QStringLiteral("positionv"), &m_positionV);
    parseProperty(attrs, flags, typeName, QStringLiteral("pivotu"), &m_pivotU);
    parseProperty(attrs, flags, typeName, QStringLiteral("pivotv"), &m_pivotV);
    parseProperty(attrs, flags, typeName, QStringLiteral("subpresentation"), &m_subPresentation);

    // Different default value.
    parseProperty(attrs, flags, typeName, QStringLiteral("name"), &m_name);
    parseProperty(attrs, flags, typeName, QStringLiteral("endtime"), &m_endTime);
}

Node::Node(GraphObject::Type type)
    : GraphObject(type)
{

}

void Node::setProperties(const QXmlStreamAttributes &attrs, PropSetFlags flags)
{
    GraphObject::setProperties(attrs, flags);
    setProps(attrs, flags);
}

void Node::applyPropertyChanges(const PropertyChangeList &changeList)
{
    GraphObject::applyPropertyChanges(changeList);
    setProps(changeList, 0);
}

void Node::writeQmlHeader(QTextStream &output, int tabLevel)
{
    output << insertTabs(tabLevel) << QStringLiteral("DemonNode {") << endl;
}

namespace {
QString rotationOrderToString(Node::RotationOrder ro) {
    switch (ro) {
    case Node::XYZ:
        return QStringLiteral("DemonNode.XYZ");
    case Node::YZX:
        return QStringLiteral("DemonNode.YZX");
    case Node::ZXY:
        return QStringLiteral("DemonNode.ZXY");
    case Node::XZY:
        return QStringLiteral("DemonNode.XZY");
    case Node::YXZ:
        return QStringLiteral("DemonNode.YZX");
    case Node::ZYX:
        return QStringLiteral("DemonNode.ZYX");
    case Node::XYZr:
        return QStringLiteral("DemonNode.XYZr");
    case Node::YZXr:
        return QStringLiteral("DemonNode.YZXr");
    case Node::ZXYr:
        return QStringLiteral("DemonNode.ZXYr");
    case Node::XZYr:
        return QStringLiteral("DemonNode.XZYr");
    case Node::YXZr:
        return QStringLiteral("DemonNode.YXZr");
    case Node::ZYXr:
        return QStringLiteral("DemonNode.ZYXr");
    }
}
QString orientationToString(Node::Orientation orientation)
{
    if (orientation == Node::LeftHanded)
        return QStringLiteral("DemonNode.LeftHanded");

    return QStringLiteral("DemonNode.RightHanded");
}
}

void Node::writeQmlProperties(QTextStream &output, int tabLevel)
{
    output << insertTabs(tabLevel) << QStringLiteral("id: ") << qmlId() << endl;
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("position"), m_position);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("rotation"), m_rotation);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("scale"), m_scale);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("pivot"), m_pivot);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("opacity"), m_localOpacity * 0.01f);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("rotationorder"), rotationOrderToString(m_rotationOrder));
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("orientation"), orientationToString(m_orientation));
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("visible"), m_flags.testFlag(Node::Active));
}

template<typename V>
void Node::setProps(const V &attrs, PropSetFlags flags)
{
    const QString typeName = QStringLiteral("Node");

    bool b;
    if (parseProperty(attrs, flags, typeName, QStringLiteral("eyeball"), &b))
        m_flags.setFlag(Active, b);
    if (parseProperty(attrs, flags, typeName, QStringLiteral("ignoresparent"), &b))
        m_flags.setFlag(IgnoresParentTransform, b);

    parseRotationProperty(attrs, flags, typeName, QStringLiteral("rotation"), &m_rotation);
    parseProperty(attrs, flags, typeName, QStringLiteral("position"), &m_position);
    parseProperty(attrs, flags, typeName, QStringLiteral("scale"), &m_scale);
    parseProperty(attrs, flags, typeName, QStringLiteral("pivot"), &m_pivot);
    parseProperty(attrs, flags, typeName, QStringLiteral("opacity"), &m_localOpacity);
    parseProperty(attrs, flags, typeName, QStringLiteral("boneid"), &m_skeletonId);
    parseProperty(attrs, flags, typeName, QStringLiteral("rotationorder"), &m_rotationOrder);
    parseProperty(attrs, flags, typeName, QStringLiteral("orientation"), &m_orientation);
}

LayerNode::LayerNode()
    : Node(GraphObject::Layer)
{

}

void LayerNode::setProperties(const QXmlStreamAttributes &attrs, PropSetFlags flags)
{
    Node::setProperties(attrs, flags);
    setProps(attrs, flags);
}

void LayerNode::applyPropertyChanges(const PropertyChangeList &changeList)
{
    Node::applyPropertyChanges(changeList);
    setProps(changeList, 0);
}

void LayerNode::writeQmlHeader(QTextStream &output, int tabLevel)
{
    output << insertTabs(tabLevel) << "DemonView3D {" << endl;
}

namespace {
QString progressiveAAToString(LayerNode::ProgressiveAA mode)
{
    switch (mode) {
    case LayerNode::NoPAA:
        return QStringLiteral("DemonSceneEnvironment.NoAA");
    case LayerNode::PAA2x:
        return QStringLiteral("DemonSceneEnvironment.X2");
    case LayerNode::PAA4x:
        return QStringLiteral("DemonSceneEnvironment.X4");
    case LayerNode::PAA8x:
        return QStringLiteral("DemonSceneEnvironment.X8");
    }
}

QString multisampleAAToString(LayerNode::MultisampleAA mode)
{
    switch (mode) {
    case LayerNode::NoMSAA:
        return QStringLiteral("DemonSceneEnvironment.NoAA");
    case LayerNode::MSAA2x:
        return QStringLiteral("DemonSceneEnvironment.X2");
    case LayerNode::MSAA4x:
        return QStringLiteral("DemonSceneEnvironment.X4");
    case LayerNode::SSAA:
        return QStringLiteral("DemonSceneEnvironment.SSAA");
    }
}

QString layerBackgroundToString(LayerNode::LayerBackground mode)
{
    switch (mode) {
    case LayerNode::Transparent:
        return QStringLiteral("DemonSceneEnvironment.Transparent");
    case LayerNode::SolidColor:
        return QStringLiteral("DemonSceneEnvironment.Color");
    case LayerNode::Unspecified:
        return QStringLiteral("DemonSceneEnvironment.Unspecified");
    }
}

QString blendTypeToString(LayerNode::BlendType type)
{
    switch (type) {
    case LayerNode::Normal:
        return QStringLiteral("DemonSceneEnvironment.Normal");
    case LayerNode::Screen:
        return QStringLiteral("DemonSceneEnvironment.Screen");
    case LayerNode::Multiply:
        return QStringLiteral("DemonSceneEnvironment.Multiply");
    case LayerNode::Add:
        return QStringLiteral("DemonSceneEnvironment.Add");
    case LayerNode::Subtract:
        return QStringLiteral("DemonSceneEnvironment.Subtract");
    case LayerNode::Overlay:
        return QStringLiteral("DemonSceneEnvironment.Overlay");
    case LayerNode::ColorBurn:
        return QStringLiteral("DemonSceneEnvironment.ColorBurn");
    case LayerNode::ColorDodge:
        return QStringLiteral("DemonSceneEnvironment.ColorDodge");
    }
}
}

void LayerNode::writeQmlProperties(QTextStream &output, int tabLevel)
{
    output << insertTabs(tabLevel) << QStringLiteral("id: ") << qmlId() << endl;

    // QQuickItem position/anchors
    if (m_horizontalFields == LeftWidth) {
        // left anchor
        output << insertTabs(tabLevel) << QStringLiteral("anchors.left: parent.left") << endl;
        if (m_leftUnits == Pixels)
            output << insertTabs(tabLevel) << QStringLiteral("anchors.leftMargin: ") << m_left << endl;
        else
            output << insertTabs(tabLevel) << QStringLiteral("anchors.leftMargin: parent.width * ") << m_left * 0.01f << endl;

        // width
        if (m_widthUnits == Pixels)
            output << insertTabs(tabLevel) << QStringLiteral("width: ") << m_width << endl;
        else
            output << insertTabs(tabLevel) << QStringLiteral("width: parent.width * ") << m_width * 0.01f << endl;

    } else if (m_horizontalFields == LeftRight) {
        // left anchor
        output << insertTabs(tabLevel) << QStringLiteral("anchors.left: parent.left") << endl;
        if (m_leftUnits == Pixels)
            output << insertTabs(tabLevel) << QStringLiteral("anchors.leftMargin: ") << m_left << endl;
        else
            output << insertTabs(tabLevel) << QStringLiteral("anchors.leftMargin: parent.width * ") << m_left * 0.01f << endl;

        // right anchor
        output << insertTabs(tabLevel) << QStringLiteral("anchors.right: parent.right") << endl;
        if (m_rightUnits == Pixels)
            output << insertTabs(tabLevel) << QStringLiteral("anchors.rightMargin: ") << m_right << endl;
        else
            output << insertTabs(tabLevel) << QStringLiteral("anchors.rightMargin: parent.width * ") << m_right * 0.01f << endl;

    } else if (m_horizontalFields == WidthRight) {
        // width
        if (m_widthUnits == Pixels)
            output << insertTabs(tabLevel) << QStringLiteral("width: ") << m_width << endl;
        else
            output << insertTabs(tabLevel) << QStringLiteral("width: parent.width * ") << m_width * 0.01f << endl;

        // right anchor
        output << insertTabs(tabLevel) << QStringLiteral("anchors.right: parent.right") << endl;
        if (m_rightUnits == Pixels)
            output << insertTabs(tabLevel) << QStringLiteral("anchors.rightMargin: ") << m_right << endl;
        else
            output << insertTabs(tabLevel) << QStringLiteral("anchors.rightMargin: parent.width * ") << m_right * 0.01f << endl;
    }

    if (m_verticalFields == TopHeight) {
        // top anchor
        output << insertTabs(tabLevel) << QStringLiteral("anchors.top: parent.top") << endl;
        if (m_topUnits == Pixels)
            output << insertTabs(tabLevel) << QStringLiteral("anchors.topMargin: ") << m_top << endl;
        else
            output << insertTabs(tabLevel) << QStringLiteral("anchors.topMargin: parent.height * ") << m_top * 0.01f << endl;

        // height
        if (m_heightUnits == Pixels)
            output << insertTabs(tabLevel) << QStringLiteral("height: ") << m_height << endl;
        else
            output << insertTabs(tabLevel) << QStringLiteral("height: parent.height * ") << m_height * 0.01f << endl;

    } else if (m_verticalFields == TopBottom) {
        // top anchor
        output << insertTabs(tabLevel) << QStringLiteral("anchors.top: parent.top") << endl;
        if (m_topUnits == Pixels)
            output << insertTabs(tabLevel) << QStringLiteral("anchors.topMargin: ") << m_top << endl;
        else
            output << insertTabs(tabLevel) << QStringLiteral("anchors.topMargin: parent.height * ") << m_top * 0.01f << endl;

        // bottom anchor
        output << insertTabs(tabLevel) << QStringLiteral("anchors.bottom: parent.bottom") << endl;
        if (m_bottomUnits == Pixels)
            output << insertTabs(tabLevel) << QStringLiteral("anchors.bottomMargin: ") << m_bottom << endl;
        else
            output << insertTabs(tabLevel) << QStringLiteral("anchors.bottomMargin: parent.height * ") << m_bottom * 0.01f << endl;


    } else if (m_verticalFields == HeightBottom) {
        // height
        if (m_heightUnits == Pixels)
            output << insertTabs(tabLevel) << QStringLiteral("height: ") << m_height << endl;
        else
            output << insertTabs(tabLevel) << QStringLiteral("height: parent.height * ") << m_height * 0.01f << endl;

        // bottom anchor
        output << insertTabs(tabLevel) << QStringLiteral("anchors.bottom: parent.bottom") << endl;
        if (m_bottomUnits == Pixels)
            output << insertTabs(tabLevel) << QStringLiteral("anchors.bottomMargin: ") << m_bottom << endl;
        else
            output << insertTabs(tabLevel) << QStringLiteral("anchors.bottomMargin: parent.height * ") << m_bottom * 0.01f << endl;
    }


    // SceneEnvironment Properties (seperate component)
    output << insertTabs(tabLevel) << QStringLiteral("environment: DemonSceneEnvironment {") << endl;
    writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("progressiveaa"), progressiveAAToString(m_progressiveAA));
    writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("multisampleaa"), multisampleAAToString(m_multisampleAA));
    writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("background"), layerBackgroundToString(m_layerBackground));
    writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("backgroundcolor"), m_backgroundColor);
    writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("blendtype"), blendTypeToString(m_blendType));

    writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("aostrength"), m_aoStrength);
    writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("aodistance"), m_aoDistance);
    writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("aosoftness"), m_aoSoftness);
    writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("aodither"), m_aoDither);
    writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("aosamplerate"), m_aoSampleRate);
    writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("aobias"), m_aoBias);

    writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("shadowstrength"), m_shadowStrength);
    writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("shadowdistance"), m_shadowDist);
    writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("shadowsoftness"), m_shadowSoftness);
    writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("shadowbias"), m_shadowBias);

    writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("disabledepthtest"),  m_layerFlags.testFlag(DisableDepthTest));
    writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("disabledepthprepass"),  m_layerFlags.testFlag(DisableDepthPrePass));

    if (!m_lightProbe_unresolved.isEmpty()) {
        output << insertTabs(tabLevel + 1) << "lightProbe: " << sanitizeQmlId(m_lightProbe_unresolved) << endl;
        writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("probebright"), m_probeBright);
        writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("fastibl"), m_layerFlags.testFlag(LayerNode::FastIBL));
        writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("probehorizon"), m_probeHorizon);
        writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("probefov"), m_probeFov);
    }

    if (!m_lightProbe2_unresolved.isEmpty()) {
        output << insertTabs(tabLevel + 1) << "lightProbe2: " << sanitizeQmlId(m_lightProbe2_unresolved) << endl;
        writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("probe2fade"), m_probe2Fade);
        writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("probe2window"), m_probe2Window);
        writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("probe2pos"), m_probe2Window);
    }
    writeQmlPropertyHelper(output, tabLevel + 1, type(), QStringLiteral("temporalaa"), (m_layerFlags.testFlag(LayerNode::TemporalAA)));
    output << insertTabs(tabLevel) << QStringLiteral("}") << endl;
}

template<typename V>
void LayerNode::setProps(const V &attrs, PropSetFlags flags)
{
    const QString typeName = QStringLiteral("Layer");

    bool b;
    if (parseProperty(attrs, flags, typeName, QStringLiteral("disabledepthtest"), &b))
        m_layerFlags.setFlag(DisableDepthTest, b);
    if (parseProperty(attrs, flags, typeName, QStringLiteral("disabledepthprepass"), &b))
        m_layerFlags.setFlag(DisableDepthPrePass, b);

    parseProperty(attrs, flags, typeName, QStringLiteral("progressiveaa"), &m_progressiveAA);
    parseProperty(attrs, flags, typeName, QStringLiteral("multisampleaa"), &m_multisampleAA);

    if (parseProperty(attrs, flags, typeName, QStringLiteral("temporalaa"), &b))
        m_layerFlags.setFlag(TemporalAA, b);

    parseProperty(attrs, flags, typeName, QStringLiteral("background"), &m_layerBackground);
    parseProperty(attrs, flags, typeName, QStringLiteral("backgroundcolor"), &m_backgroundColor);
    parseProperty(attrs, flags, typeName, QStringLiteral("blendtype"), &m_blendType);

    parseProperty(attrs, flags, typeName, QStringLiteral("horzfields"), &m_horizontalFields);
    parseProperty(attrs, flags, typeName, QStringLiteral("left"), &m_left);
    parseProperty(attrs, flags, typeName, QStringLiteral("leftunits"), &m_leftUnits);
    parseProperty(attrs, flags, typeName, QStringLiteral("width"), &m_width);
    parseProperty(attrs, flags, typeName, QStringLiteral("widthunits"), &m_widthUnits);
    parseProperty(attrs, flags, typeName, QStringLiteral("right"), &m_right);
    parseProperty(attrs, flags, typeName, QStringLiteral("rightunits"), &m_rightUnits);
    parseProperty(attrs, flags, typeName, QStringLiteral("vertfields"), &m_verticalFields);
    parseProperty(attrs, flags, typeName, QStringLiteral("top"), &m_top);
    parseProperty(attrs, flags, typeName, QStringLiteral("topunits"), &m_topUnits);
    parseProperty(attrs, flags, typeName, QStringLiteral("height"), &m_height);
    parseProperty(attrs, flags, typeName, QStringLiteral("heightunits"), &m_heightUnits);
    parseProperty(attrs, flags, typeName, QStringLiteral("bottom"), &m_bottom);
    parseProperty(attrs, flags, typeName, QStringLiteral("bottomunits"), &m_bottomUnits);

    parseProperty(attrs, flags, typeName, QStringLiteral("sourcepath"), &m_sourcePath);

    // SSAO
    parseProperty(attrs, flags, typeName, QStringLiteral("aostrength"), &m_aoStrength);
    parseProperty(attrs, flags, typeName, QStringLiteral("aodistance"), &m_aoDistance);
    parseProperty(attrs, flags, typeName, QStringLiteral("aosoftness"), &m_aoSoftness);
    parseProperty(attrs, flags, typeName, QStringLiteral("aobias"), &m_aoBias);
    parseProperty(attrs, flags, typeName, QStringLiteral("aosamplerate"), &m_aoSampleRate);
    parseProperty(attrs, flags, typeName, QStringLiteral("aodither"), &m_aoDither);

    // SSDO (these are always hidden in the application, it seems, and so SSDO cannot be enabled in practice)
    parseProperty(attrs, flags, typeName, QStringLiteral("shadowstrength"), &m_shadowStrength);
    parseProperty(attrs, flags, typeName, QStringLiteral("shadowdist"), &m_shadowDist);
    parseProperty(attrs, flags, typeName, QStringLiteral("shadowsoftness"), &m_shadowSoftness);
    parseProperty(attrs, flags, typeName, QStringLiteral("shadowbias"), &m_shadowBias);

    // IBL
    parseImageProperty(attrs, flags, typeName, QStringLiteral("lightprobe"), &m_lightProbe_unresolved);
    parseProperty(attrs, flags, typeName, QStringLiteral("probebright"), &m_probeBright);
    if (parseProperty(attrs, flags, typeName, QStringLiteral("fastibl"), &b))
        m_layerFlags.setFlag(FastIBL, b);
    parseProperty(attrs, flags, typeName, QStringLiteral("probehorizon"), &m_probeHorizon);
    parseProperty(attrs, flags, typeName, QStringLiteral("probefov"), &m_probeFov);
    parseImageProperty(attrs, flags, typeName, QStringLiteral("lightprobe2"), &m_lightProbe2_unresolved);
    parseProperty(attrs, flags, typeName, QStringLiteral("probe2fade"), &m_probe2Fade);
    parseProperty(attrs, flags, typeName, QStringLiteral("probe2window"), &m_probe2Window);
    parseProperty(attrs, flags, typeName, QStringLiteral("probe2pos"), &m_probe2Pos);

    // Different default value.
    parseProperty(attrs, flags, typeName, QStringLiteral("name"), &m_name);
}

CameraNode::CameraNode()
    : Node(GraphObject::Camera)
{

}

void CameraNode::setProperties(const QXmlStreamAttributes &attrs, PropSetFlags flags)
{
    Node::setProperties(attrs, flags);
    setProps(attrs, flags);
}

void CameraNode::applyPropertyChanges(const PropertyChangeList &changeList)
{
    Node::applyPropertyChanges(changeList);
    setProps(changeList, 0);
}

void CameraNode::writeQmlHeader(QTextStream &output, int tabLevel)
{
    output << insertTabs(tabLevel) << QStringLiteral("DemonCamera {") << endl;
}

namespace {
QString cameraScaleModeToString(CameraNode::ScaleMode mode)
{
    switch (mode) {
    case CameraNode::SameSize:
        return QStringLiteral("DemonCamera.SameSize");
    case CameraNode::Fit:
        return QStringLiteral("DemonCamera.Fit");
    case CameraNode::FitHorizontal:
        return QStringLiteral("DemonCamera.FitHorizontal");
    case CameraNode::FitVertical:
        return QStringLiteral("DemonCamera.FitVertical");
    }
}
QString cameraScaleAnchorToString(CameraNode::ScaleAnchor anchor)
{
    switch (anchor) {
    case CameraNode::Center:
        return QStringLiteral("DemonCamera.Center");
    case CameraNode::N:
        return QStringLiteral("DemonCamera.North");
    case CameraNode::NE:
        return QStringLiteral("DemonCamera.NorthEast");
    case CameraNode::E:
        return QStringLiteral("DemonCamera.East");
    case CameraNode::SE:
        return QStringLiteral("DemonCamera.SouthEast");
    case CameraNode::S:
        return QStringLiteral("DemonCamera.South");
    case CameraNode::SW:
        return QStringLiteral("DemonCamera.SouthWest");
    case CameraNode::W:
        return QStringLiteral("DemonCamera.West");
    case CameraNode::NW:
        return QStringLiteral("DemonCamera.NorthWest");
    }
}

}

void CameraNode::writeQmlProperties(QTextStream &output, int tabLevel)
{
    Node::writeQmlProperties(output, tabLevel);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("orthographic"), m_orthographic ? QStringLiteral("DemonCamera.Orthographic") : QStringLiteral("DemonCamera.Perspective"));
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("clipnear"), m_clipNear);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("clipfar"), m_clipFar);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("fov"), m_fov);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("fovhorizontal"), m_fovHorizontal);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("scalemode"), cameraScaleModeToString(m_scaleMode));
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("scaleanchor"), cameraScaleAnchorToString(m_scaleAnchor));
}

template<typename V>
void CameraNode::setProps(const V &attrs, PropSetFlags flags)
{
    const QString typeName = QStringLiteral("Camera");

    parseProperty(attrs, flags, typeName, QStringLiteral("orthographic"), &m_orthographic);
    parseProperty(attrs, flags, typeName, QStringLiteral("fov"), &m_fov);
    parseProperty(attrs, flags, typeName, QStringLiteral("fovhorizontal"), &m_fovHorizontal);
    parseProperty(attrs, flags, typeName, QStringLiteral("clipnear"), &m_clipNear);
    parseProperty(attrs, flags, typeName, QStringLiteral("clipfar"), &m_clipFar);
    parseProperty(attrs, flags, typeName, QStringLiteral("scalemode"), &m_scaleMode);
    parseProperty(attrs, flags, typeName, QStringLiteral("scaleanchor"), &m_scaleAnchor);

    // Different default value.
    parseProperty(attrs, flags, typeName, QStringLiteral("name"), &m_name);
    parseProperty(attrs, flags, typeName, QStringLiteral("position"), &m_position);
}

LightNode::LightNode()
    : Node(GraphObject::Light)
{

}

void LightNode::setProperties(const QXmlStreamAttributes &attrs, PropSetFlags flags)
{
    Node::setProperties(attrs, flags);
    setProps(attrs, flags);
}

void LightNode::applyPropertyChanges(const PropertyChangeList &changeList)
{
    Node::applyPropertyChanges(changeList);
    setProps(changeList, 0);
}

void LightNode::writeQmlHeader(QTextStream &output, int tabLevel)
{
    output << insertTabs(tabLevel) << QStringLiteral("DemonLight {") << endl;
}

namespace  {
QString lightTypeToString(LightNode::LightType type)
{
    switch (type) {
    case LightNode::Directional:
        return QStringLiteral("DemonLight.Directional");
    case LightNode::Point:
        return QStringLiteral("DemonLight.Point");
    case LightNode::Area:
        return QStringLiteral("DemonLight.Area");
    }
}
}

void LightNode::writeQmlProperties(QTextStream &output, int tabLevel)
{
    Node::writeQmlProperties(output, tabLevel);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("lighttype"), lightTypeToString(m_lightType));
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("lightdiffuse"), m_lightDiffuse);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("lightspecular"), m_lightSpecular);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("lightambient"), m_lightAmbient);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("brightness"), m_brightness);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("linearfade"), m_linearFade);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("expfade"), m_expFade);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("areawidth"), m_areaWidth);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("areaheight"), m_areaHeight);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("castshadow"), m_castShadow);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("shdwbias"), m_shadowBias);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("shdwfactor"), m_shadowFactor);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("shdwmapres"), m_shadowMapRes);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("shdwmapfar"), m_shadowMapFar);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("shdwmapfov"), m_shadowMapFov);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("shdwfilter"), m_shadowFilter);
}

template<typename V>
void LightNode::setProps(const V &attrs, PropSetFlags flags)
{
    const QString typeName = QStringLiteral("Light");

    parseObjectRefProperty(attrs, flags, typeName, QStringLiteral("scope"), &m_scope_unresolved);

    parseProperty(attrs, flags, typeName, QStringLiteral("lighttype"), &m_lightType);
    parseProperty(attrs, flags, typeName, QStringLiteral("lightdiffuse"), &m_lightDiffuse);
    parseProperty(attrs, flags, typeName, QStringLiteral("lightspecular"), &m_lightSpecular);
    parseProperty(attrs, flags, typeName, QStringLiteral("lightambient"), &m_lightAmbient);
    parseProperty(attrs, flags, typeName, QStringLiteral("brightness"), &m_brightness);
    parseProperty(attrs, flags, typeName, QStringLiteral("linearfade"), &m_linearFade);
    parseProperty(attrs, flags, typeName, QStringLiteral("expfade"), &m_expFade);
    parseProperty(attrs, flags, typeName, QStringLiteral("areawidth"), &m_areaWidth);
    parseProperty(attrs, flags, typeName, QStringLiteral("areaheight"), &m_areaHeight);
    parseProperty(attrs, flags, typeName, QStringLiteral("castshadow"), &m_castShadow);
    parseProperty(attrs, flags, typeName, QStringLiteral("shdwfactor"), &m_shadowFactor);
    parseProperty(attrs, flags, typeName, QStringLiteral("shdwfilter"), &m_shadowFilter);
    parseProperty(attrs, flags, typeName, QStringLiteral("shdwmapres"), &m_shadowMapRes);
    parseProperty(attrs, flags, typeName, QStringLiteral("shdwbias"), &m_shadowBias);
    parseProperty(attrs, flags, typeName, QStringLiteral("shdwmapfar"), &m_shadowMapFar);
    parseProperty(attrs, flags, typeName, QStringLiteral("shdwmapfov"), &m_shadowMapFov);

    // Different default value.
    parseProperty(attrs, flags, typeName, QStringLiteral("name"), &m_name);
}

ModelNode::ModelNode()
    : Node(GraphObject::Model)
{

}

ModelNode::~ModelNode()
{

}

void ModelNode::setProperties(const QXmlStreamAttributes &attrs, PropSetFlags flags)
{
    Node::setProperties(attrs, flags);
    setProps(attrs, flags);
}

void ModelNode::applyPropertyChanges(const PropertyChangeList &changeList)
{
    Node::applyPropertyChanges(changeList);
    setProps(changeList, 0);
}

void ModelNode::writeQmlHeader(QTextStream &output, int tabLevel)
{
    output << insertTabs(tabLevel) << QStringLiteral("DemonModel {") << endl;
}

namespace {
QString tesselationModeToString(ModelNode::Tessellation mode)
{
    switch (mode) {
    case ModelNode::None:
        return QStringLiteral("DemonModel.NoTess");
    case ModelNode::Linear:
        return QStringLiteral("DemonModel.TessLinear");
    case ModelNode::Phong:
        return QStringLiteral("DemonModel.TessPhong");
    case ModelNode::NPatch:
        return QStringLiteral("DemonModel.TessNPatch");
    }
}
}

void ModelNode::writeQmlProperties(QTextStream &output, int tabLevel)
{
    Node::writeQmlProperties(output, tabLevel);
    output << insertTabs(tabLevel) << QStringLiteral("source: ") << sanitizeQmlSourcePath(m_mesh_unresolved) << endl;
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("poseroot"), m_skeletonRoot);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("tessellation"), tesselationModeToString(m_tessellation));
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("edgetess"), m_edgeTess);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("innertess"), m_innerTess);
    // ### Materials
}

template<typename V>
void ModelNode::setProps(const V &attrs, PropSetFlags flags)
{
    const QString typeName = QStringLiteral("Model");
    parseMeshProperty(attrs, flags, typeName, QStringLiteral("sourcepath"), &m_mesh_unresolved);
    parseProperty(attrs, flags, typeName, QStringLiteral("poseroot"), &m_skeletonRoot);
    parseProperty(attrs, flags, typeName, QStringLiteral("tessellation"), &m_tessellation);
    parseProperty(attrs, flags, typeName, QStringLiteral("edgetess"), &m_edgeTess);
    parseProperty(attrs, flags, typeName, QStringLiteral("innertess"), &m_innerTess);

    // Different default value.
    parseProperty(attrs, flags, typeName, QStringLiteral("name"), &m_name);
}

GroupNode::GroupNode()
    : Node(GraphObject::Group)
{

}

void GroupNode::setProperties(const QXmlStreamAttributes &attrs, PropSetFlags flags)
{
    Node::setProperties(attrs, flags);
    setProps(attrs, flags);
}

void GroupNode::applyPropertyChanges(const PropertyChangeList &changeList)
{
    Node::applyPropertyChanges(changeList);
    setProps(changeList, 0);
}

void GroupNode::writeQmlHeader(QTextStream &output, int tabLevel)
{
    Node::writeQmlHeader(output, tabLevel);
}

void GroupNode::writeQmlProperties(QTextStream &output, int tabLevel)
{
    Node::writeQmlProperties(output, tabLevel);
}

template<typename V>
void GroupNode::setProps(const V &attrs, PropSetFlags flags)
{
    const QString typeName = QStringLiteral("Group");

    // Different default value.
    parseProperty(attrs, flags, typeName, QStringLiteral("name"), &m_name);
}

ComponentNode::ComponentNode()
    : Node(GraphObject::Component)
{

}

ComponentNode::~ComponentNode()
{

}

void ComponentNode::setProperties(const QXmlStreamAttributes &attrs, PropSetFlags flags)
{
    Node::setProperties(attrs, flags);
    setProps(attrs, flags);
}

void ComponentNode::applyPropertyChanges(const PropertyChangeList &changeList)
{
    Node::applyPropertyChanges(changeList);
    setProps(changeList, 0);
}

void ComponentNode::writeQmlHeader(QTextStream &output, int tabLevel)
{
    Node::writeQmlHeader(output, tabLevel);
}

void ComponentNode::writeQmlProperties(QTextStream &output, int tabLevel)
{
    Node::writeQmlProperties(output, tabLevel);
}

template<typename V>
void ComponentNode::setProps(const V &attrs, PropSetFlags flags)
{
    const QString typeName = QStringLiteral("Component");

    // Different default value.
    parseProperty(attrs, flags, typeName, QStringLiteral("name"), &m_name);
}

TextNode::TextNode()
    : Node(GraphObject::Text)
{

}

void TextNode::setProperties(const QXmlStreamAttributes &attrs, PropSetFlags flags)
{
    Node::setProperties(attrs, flags);
    setProps(attrs, flags);
}

void TextNode::applyPropertyChanges(const PropertyChangeList &changeList)
{
    Node::applyPropertyChanges(changeList);
    setProps(changeList, 0);
}

void TextNode::writeQmlHeader(QTextStream &output, int tabLevel)
{
}

void TextNode::writeQmlProperties(QTextStream &output, int tabLevel)
{
}

void TextNode::writeQmlFooter(QTextStream &output, int tabLevel)
{
}

template<typename V>
void TextNode::setProps(const V &attrs, PropSetFlags flags)
{
    const QString typeName = QStringLiteral("Text");
    parseMultiLineStringProperty(attrs, flags, typeName, QStringLiteral("textstring"), &m_text);
    parseProperty(attrs, flags, typeName, QStringLiteral("textcolor"), &m_color);
    parseFontProperty(attrs, flags, typeName, QStringLiteral("font"), &m_font);
    parseFontSizeProperty(attrs, flags, typeName, QStringLiteral("size"), &m_size);
    parseProperty(attrs, flags, typeName, QStringLiteral("horzalign"), &m_horizAlign);
    parseProperty(attrs, flags, typeName, QStringLiteral("vertalign"), &m_vertAlign);
    parseProperty(attrs, flags, typeName, QStringLiteral("leading"), &m_leading);
    parseProperty(attrs, flags, typeName, QStringLiteral("tracking"), &m_tracking);
    parseProperty(attrs, flags, typeName, QStringLiteral("dropshadow"), &m_shadow);
    parseProperty(attrs, flags, typeName, QStringLiteral("dropshadowstrength"), &m_shadowStrength);
    parseProperty(attrs, flags, typeName, QStringLiteral("dropshadowoffsetx"), &m_shadowOffsetX);
    parseProperty(attrs, flags, typeName, QStringLiteral("dropshadowoffsety"), &m_shadowOffsetY);
    parseProperty(attrs, flags, typeName, QStringLiteral("dropshadowoffset"), &m_shadowOffset); // To be removed in 2.x (when UIP version is next updated)
    parseProperty(attrs, flags, typeName, QStringLiteral("dropshadowhorzalign"), &m_shadowHorzAlign); // To be removed in 2.x (when UIP version is next updated)
    parseProperty(attrs, flags, typeName, QStringLiteral("dropshadowvertalign"), &m_shadowVertAlign); // To be removed in 2.x (when UIP version is next updated)
    parseSizeProperty(attrs, flags, typeName, QStringLiteral("boundingbox"), &m_boundingBox);
    parseProperty(attrs, flags, typeName, QStringLiteral("wordwrap"), &m_wordWrap);
    parseProperty(attrs, flags, typeName, QStringLiteral("elide"), &m_elide);

    // Different default value.
    parseProperty(attrs, flags, typeName, QStringLiteral("name"), &m_name);
}

DefaultMaterial::DefaultMaterial()
    : GraphObject(GraphObject::DefaultMaterial)
{

}

void DefaultMaterial::setProperties(const QXmlStreamAttributes &attrs, PropSetFlags flags)
{
    GraphObject::setProperties(attrs, flags);
    setProps(attrs, flags);
}

void DefaultMaterial::applyPropertyChanges(const PropertyChangeList &changeList)
{
    GraphObject::applyPropertyChanges(changeList);
    setProps(changeList, 0);
}

void DefaultMaterial::writeQmlHeader(QTextStream &output, int tabLevel)
{
    output << insertTabs(tabLevel) << QStringLiteral("DemonDefaultMaterial {") << endl;
}

namespace {
QString shaderLightingToString(DefaultMaterial::ShaderLighting mode)
{
    if (mode == DefaultMaterial::PixelShaderLighting) {
        return QStringLiteral("DemonDefaultMaterial.FragmentLighting");
    } else {
        return QStringLiteral("DemonDefaultMaterial.NoLighting");
    }
}
QString shaderBlendModeToString(DefaultMaterial::BlendMode mode)
{
    switch (mode) {
    case DefaultMaterial::Normal:
        return QStringLiteral("DemonDefaultMaterial.Normal");
    case DefaultMaterial::Screen:
        return QStringLiteral("DemonDefaultMaterial.Screen");
    case DefaultMaterial::Multiply:
        return QStringLiteral("DemonDefaultMaterial.Multiply");
    case DefaultMaterial::Overlay:
        return QStringLiteral("DemonDefaultMaterial.Overlay");
    case DefaultMaterial::ColorBurn:
        return QStringLiteral("DemonDefaultMaterial.ColorBurn");
    case DefaultMaterial::ColorDodge:
        return QStringLiteral("DemonDefaultMaterial.ColorDodge");
    }
}
QString shaderSpecularModelToString(DefaultMaterial::SpecularModel model)
{
    switch (model) {
        case DefaultMaterial::DefaultSpecularModel:
            return QStringLiteral("DemonDefaultMaterial.Default");
    case DefaultMaterial::KGGX:
        return QStringLiteral("DemonDefaultMaterial.KGGX");
    case DefaultMaterial::KWard:
        return QStringLiteral("DemonDefaultMaterial.KWard");
    }
}
}

void DefaultMaterial::writeQmlProperties(QTextStream &output, int tabLevel)
{
    output << insertTabs(tabLevel) << QStringLiteral("id: ") << qmlId() << endl;
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("shaderlighting"), shaderLightingToString(m_shaderLighting));
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("blendmode"), shaderBlendModeToString(m_blendMode));
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("diffuse"), m_diffuse);
    if (!m_diffuseMap_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("diffuseMap: ") << sanitizeQmlId(m_diffuseMap_unresolved) << endl;
    if (!m_diffuseMap2_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("diffuseMap2: ") << sanitizeQmlId(m_diffuseMap2_unresolved) << endl;
    if (!m_diffuseMap3_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("diffuseMap3: ") << sanitizeQmlId(m_diffuseMap3_unresolved) << endl;

    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("emissivepower"), m_emissivePower);
    if (!m_emissiveMap_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("emissiveMap: ") << sanitizeQmlId(m_emissiveMap_unresolved) << endl;
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("emissivecolor"), m_emissiveColor);

    if (!m_specularReflection_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("specularReflectionMap: ") << sanitizeQmlId(m_specularReflection_unresolved) << endl;
    if (!m_specularMap_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("specularMap: ") << sanitizeQmlId(m_specularMap_unresolved) << endl;

    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("specularmodel"), shaderSpecularModelToString(m_specularModel));
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("speculartint"), m_specularTint);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("ior"), m_ior);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("fresnelPower"), m_fresnelPower);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("specularamount"), m_specularAmount);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("specularroughness"), m_specularRoughness);

    if (!m_roughnessMap_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("roughnessMap: ") << sanitizeQmlId(m_roughnessMap_unresolved) << endl;

    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("opacity"), m_opacity * 0.01);
    if (!m_opacityMap_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("opacityMap: ") << sanitizeQmlId(m_opacityMap_unresolved) << endl;

    if (!m_bumpMap_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("bumpMap: ") << sanitizeQmlId(m_bumpMap_unresolved) << endl;
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("bumpamount"), m_bumpAmount);

    if (!m_normalMap_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("normalMap: ") << sanitizeQmlId(m_normalMap_unresolved) << endl;

    if (!m_translucencyMap_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("translucencyMap: ") << sanitizeQmlId(m_translucencyMap_unresolved) << endl;
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("translucentfalloff"), m_translucentFalloff);

    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("diffuselightwrap"), m_diffuseLightWrap);
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("vertexColors"), m_vertexColors);

    // Common Material values
    if (!m_lightmapIndirectMap_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("lightmapIndirect: ") << sanitizeQmlId(m_lightmapIndirectMap_unresolved) << endl;
    if (!m_lightmapRadiosityMap_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("lightmapRadiosity: ") << sanitizeQmlId(m_lightmapRadiosityMap_unresolved) << endl;
    if (!m_lightmapShadowMap_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("lightmapShadow: ") << sanitizeQmlId(m_lightmapShadowMap_unresolved) << endl;
    if (!m_lightProbe_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("iblProbe: ") << sanitizeQmlId(m_lightProbe_unresolved) << endl;
    if (!m_emissiveMap2_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("emissiveMap2: ") << sanitizeQmlId(m_emissiveMap2_unresolved) << endl;
    if (!m_displacementMap_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("displacementMap: ") << sanitizeQmlId(m_displacementMap_unresolved) << endl;
    writeQmlPropertyHelper(output, tabLevel, type(), QStringLiteral("displacementamount"), m_displaceAmount);
}

template<typename V>
void DefaultMaterial::setProps(const V &attrs, PropSetFlags flags)
{
    const QString typeName = QStringLiteral("Material");

    parseProperty(attrs, flags, typeName, QStringLiteral("shaderlighting"), &m_shaderLighting);
    parseProperty(attrs, flags, typeName, QStringLiteral("blendmode"), &m_blendMode);

    parseProperty(attrs, flags, typeName, QStringLiteral("vertexcolors"), &m_vertexColors);
    parseProperty(attrs, flags, typeName, QStringLiteral("diffuse"), &m_diffuse);

    parseImageProperty(attrs, flags, typeName, QStringLiteral("diffusemap"), &m_diffuseMap_unresolved);
    parseImageProperty(attrs, flags, typeName, QStringLiteral("diffusemap2"), &m_diffuseMap2_unresolved);
    parseImageProperty(attrs, flags, typeName, QStringLiteral("diffusemap3"), &m_diffuseMap3_unresolved);
    parseImageProperty(attrs, flags, typeName, QStringLiteral("specularreflection"), &m_specularReflection_unresolved);

    parseProperty(attrs, flags, typeName, QStringLiteral("speculartint"), &m_specularTint);
    parseProperty(attrs, flags, typeName, QStringLiteral("specularamount"), &m_specularAmount);

    parseImageProperty(attrs, flags, typeName, QStringLiteral("specularmap"), &m_specularMap_unresolved);

    parseProperty(attrs, flags, typeName, QStringLiteral("specularmodel"), &m_specularModel);
    parseProperty(attrs, flags, typeName, QStringLiteral("specularroughness"), &m_specularRoughness);

    parseImageProperty(attrs, flags, typeName, QStringLiteral("roughnessmap"), &m_roughnessMap_unresolved);

    parseProperty(attrs, flags, typeName, QStringLiteral("fresnelPower"), &m_fresnelPower);
    parseProperty(attrs, flags, typeName, QStringLiteral("ior"), &m_ior);

    parseImageProperty(attrs, flags, typeName, QStringLiteral("bumpmap"), &m_bumpMap_unresolved);
    parseImageProperty(attrs, flags, typeName, QStringLiteral("normalmap"), &m_normalMap_unresolved);

    parseProperty(attrs, flags, typeName, QStringLiteral("bumpamount"), &m_bumpAmount);

    parseImageProperty(attrs, flags, typeName, QStringLiteral("displacementmap"), &m_displacementMap_unresolved);

    parseProperty(attrs, flags, typeName, QStringLiteral("displaceamount"), &m_displaceAmount);
    parseProperty(attrs, flags, typeName, QStringLiteral("opacity"), &m_opacity);

    parseImageProperty(attrs, flags, typeName, QStringLiteral("opacitymap"), &m_opacityMap_unresolved);

    parseProperty(attrs, flags, typeName, QStringLiteral("emissivecolor"), &m_emissiveColor);
    parseProperty(attrs, flags, typeName, QStringLiteral("emissivepower"), &m_emissivePower);

    parseImageProperty(attrs, flags, typeName, QStringLiteral("emissivemap"), &m_emissiveMap_unresolved);
    parseImageProperty(attrs, flags, typeName, QStringLiteral("emissivemap2"), &m_emissiveMap2_unresolved);
    parseImageProperty(attrs, flags, typeName, QStringLiteral("translucencymap"), &m_translucencyMap_unresolved);

    parseProperty(attrs, flags, typeName, QStringLiteral("translucentfalloff"), &m_translucentFalloff);
    parseProperty(attrs, flags, typeName, QStringLiteral("diffuselightwrap"), &m_diffuseLightWrap);

    parseImageProperty(attrs, flags, typeName, QStringLiteral("lightmapindirect"), &m_lightmapIndirectMap_unresolved);
    parseImageProperty(attrs, flags, typeName, QStringLiteral("lightmapradiosity"), &m_lightmapRadiosityMap_unresolved);
    parseImageProperty(attrs, flags, typeName, QStringLiteral("lightmapshadow"), &m_lightmapShadowMap_unresolved);
    parseImageProperty(attrs, flags, typeName, QStringLiteral("iblprobe"), &m_lightProbe_unresolved);

    // Different default value.
    parseProperty(attrs, flags, typeName, QStringLiteral("name"), &m_name);
}

ReferencedMaterial::ReferencedMaterial()
    : GraphObject(GraphObject::ReferencedMaterial)
{

}

void ReferencedMaterial::setProperties(const QXmlStreamAttributes &attrs, PropSetFlags flags)
{
    GraphObject::setProperties(attrs, flags);
    setProps(attrs, flags);
}

void ReferencedMaterial::applyPropertyChanges(const PropertyChangeList &changeList)
{
    GraphObject::applyPropertyChanges(changeList);
    setProps(changeList, 0);
}

void ReferencedMaterial::writeQmlHeader(QTextStream &output, int tabLevel)
{
    // This is a bit special because it references a component
    // so the Comonent type is Material.(ReferencedMaterial)
    QString componentName = QStringLiteral("Materials.") + qmlPresentationComponentName(m_referencedMaterial_unresolved);
    output << insertTabs(tabLevel) << componentName << QStringLiteral(" {") << endl;
}

void ReferencedMaterial::writeQmlProperties(QTextStream &output, int tabLevel)
{
    output << insertTabs(tabLevel) << QStringLiteral("id: ") << qmlId() << endl;
    if (!m_lightmapIndirectMap_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("lightmapIndirect: ") << sanitizeQmlId(m_lightmapIndirectMap_unresolved) << endl;
    if (!m_lightmapRadiosityMap_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("lightmapRadiosity: ") << sanitizeQmlId(m_lightmapRadiosityMap_unresolved) << endl;
    if (!m_lightmapShadowMap_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("lightmapShadow: ") << sanitizeQmlId(m_lightmapShadowMap_unresolved) << endl;
    if (!m_lightProbe_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("iblProbe: ") << sanitizeQmlId(m_lightProbe_unresolved) << endl;
}

template<typename V>
void ReferencedMaterial::setProps(const V &attrs, PropSetFlags flags)
{
    const QString typeName = QStringLiteral("ReferencedMaterial");
    parseObjectRefProperty(attrs, flags, typeName, QStringLiteral("referencedmaterial"), &m_referencedMaterial_unresolved);

    parseImageProperty(attrs, flags, typeName, QStringLiteral("lightmapindirect"), &m_lightmapIndirectMap_unresolved);
    parseImageProperty(attrs, flags, typeName, QStringLiteral("lightmapradiosity"), &m_lightmapRadiosityMap_unresolved);
    parseImageProperty(attrs, flags, typeName, QStringLiteral("lightmapshadow"), &m_lightmapShadowMap_unresolved);
    parseImageProperty(attrs, flags, typeName, QStringLiteral("iblprobe"), &m_lightProbe_unresolved);

    // Different default value.
    parseProperty(attrs, flags, typeName, QStringLiteral("name"), &m_name);
}

CustomMaterialInstance::CustomMaterialInstance()
    : GraphObject(GraphObject::CustomMaterial)
{

}

void CustomMaterialInstance::setProperties(const QXmlStreamAttributes &attrs, PropSetFlags flags)
{
    GraphObject::setProperties(attrs, flags);
    setProps(attrs, flags);

    // Save attributes for the 2nd pass (resolveReferences) since they may
    // refer to custom properties defined in the custom material.
    for (const QXmlStreamAttribute &attr : attrs)
        m_pendingCustomProperties.append(PropertyChange(attr.name().toString(), attr.value().toString()));
}

void CustomMaterialInstance::applyPropertyChanges(const PropertyChangeList &changeList)
{
    GraphObject::applyPropertyChanges(changeList);
    setProps(changeList, 0);

//    QVariantMap propChanges;
//    for (const PropertyChange &change : changeList) {
//        const Q3DS::PropertyType type = m_material.properties().value(change.nameStr()).type;
//        propChanges[change.nameStr()] = Q3DS::convertToVariant(change.valueStr(), type);
//    }

//    if (!propChanges.isEmpty())
    //        applyDynamicProperties(propChanges);
}

void CustomMaterialInstance::writeQmlHeader(QTextStream &output, int tabLevel)
{
    output << insertTabs(tabLevel) << QStringLiteral("DemonCustomMaterial {") << endl;
}

void CustomMaterialInstance::writeQmlProperties(QTextStream &output, int tabLevel)
{
    output << insertTabs(tabLevel) << QStringLiteral("id: ") << qmlId() << endl;
    output << insertTabs(tabLevel) << QStringLiteral("source: ") << QStringLiteral("\"") << sanitizeQmlId(m_material_unresolved) << QStringLiteral("\"") << endl;

    // Common Material values
    if (!m_lightmapIndirectMap_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("lightmapIndirect: ") << sanitizeQmlId(m_lightmapIndirectMap_unresolved) << endl;
    if (!m_lightmapRadiosityMap_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("lightmapRadiosity: ") << sanitizeQmlId(m_lightmapRadiosityMap_unresolved) << endl;
    if (!m_lightmapShadowMap_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("lightmapShadow: ") << sanitizeQmlId(m_lightmapShadowMap_unresolved) << endl;
    if (!m_lightProbe_unresolved.isEmpty())
        output << insertTabs(tabLevel) << QStringLiteral("iblProbe: ") << sanitizeQmlId(m_lightProbe_unresolved) << endl;
}

template<typename V>
void CustomMaterialInstance::setProps(const V &attrs, PropSetFlags flags)
{
    const QString typeName = QStringLiteral("CustomMaterial");
    if (parseProperty(attrs, flags, typeName, QStringLiteral("class"), &m_material_unresolved))
        m_materialIsResolved = false;

    parseImageProperty(attrs, flags, typeName, QStringLiteral("lightmapindirect"), &m_lightmapIndirectMap_unresolved);
    parseImageProperty(attrs, flags, typeName, QStringLiteral("lightmapradiosity"), &m_lightmapRadiosityMap_unresolved);
    parseImageProperty(attrs, flags, typeName, QStringLiteral("lightmapshadow"), &m_lightmapShadowMap_unresolved);
    parseImageProperty(attrs, flags, typeName, QStringLiteral("iblprobe"), &m_lightProbe_unresolved);

    // Different default value.
    parseProperty(attrs, flags, typeName, QStringLiteral("name"), &m_name);
}

EffectInstance::EffectInstance()
    : GraphObject(GraphObject::Effect)
{

}

void EffectInstance::setProperties(const QXmlStreamAttributes &attrs, PropSetFlags flags)
{
    GraphObject::setProperties(attrs, flags);
    setProps(attrs, flags);

    // Save attributes for the 2nd pass (resolveReferences) since they may
    // refer to custom properties defined in the effect.
    for (const QXmlStreamAttribute &attr : attrs)
        m_pendingCustomProperties.append(PropertyChange(attr.name().toString(), attr.value().toString()));
}

void EffectInstance::applyPropertyChanges(const PropertyChangeList &changeList)
{
    GraphObject::applyPropertyChanges(changeList);
    setProps(changeList, 0);

//    // could be a custom effect property
//    QVariantMap propChanges;
//    for (const PropertyChange &change : changeList) {
//        const Q3DS::PropertyType type = m_effect.properties().value(change.nameStr()).type;
//        propChanges[change.nameStr()] = Q3DS::convertToVariant(change.valueStr(), type);
//    }

//    if (!propChanges.isEmpty())
    //        applyDynamicProperties(propChanges);
}

void EffectInstance::writeQmlHeader(QTextStream &output, int tabLevel)
{
    output << insertTabs(tabLevel) << QStringLiteral("DemonEffect {") << endl;
}

void EffectInstance::writeQmlProperties(QTextStream &output, int tabLevel)
{
    output << insertTabs(tabLevel) << QStringLiteral("id: ") << qmlId() << endl;
    output << insertTabs(tabLevel) << QStringLiteral("source: ") << sanitizeQmlId(m_effect_unresolved) << endl;
}

template<typename V>
void EffectInstance::setProps(const V &attrs, PropSetFlags flags)
{
    const QString typeName = QStringLiteral("Effect");
    if (parseProperty(attrs, flags, typeName, QStringLiteral("class"), &m_effect_unresolved))
        m_effectIsResolved = false;

    parseProperty(attrs, flags, typeName, QStringLiteral("eyeball"), &m_eyeballEnabled);

    // Different default value.
    parseProperty(attrs, flags, typeName, QStringLiteral("name"), &m_name);
}

BehaviorInstance::BehaviorInstance()
    : GraphObject(GraphObject::Behavior)
{

}

void BehaviorInstance::setProperties(const QXmlStreamAttributes &attrs, PropSetFlags flags)
{
    GraphObject::setProperties(attrs, flags);
    setProps(attrs, flags);

    // Save attributes for the 2nd pass (resolveReferences) since they may
    // refer to custom properties defined in the behavior.
    for (const QXmlStreamAttribute &attr : attrs)
        m_pendingCustomProperties.append(PropertyChange(attr.name().toString(), attr.value().toString()));
}

void BehaviorInstance::applyPropertyChanges(const PropertyChangeList &changeList)
{
    GraphObject::applyPropertyChanges(changeList);
    setProps(changeList, 0);

//    // could be a custom behavior property
//    QVariantMap propChanges;
//    for (const PropertyChange &change : changeList) {
//        const Q3DS::PropertyType type = m_behavior.properties().value(change.nameStr()).type;
//        propChanges[change.nameStr()] = Q3DS::convertToVariant(change.valueStr(), type);
//    }

    //    if (!propChanges.isEmpty())
}

void BehaviorInstance::writeQmlHeader(QTextStream &output, int tabLevel)
{

}

void BehaviorInstance::writeQmlProperties(QTextStream &output, int tabLevel)
{

}

void BehaviorInstance::writeQmlFooter(QTextStream &output, int tabLevel)
{

}

template<typename V>
void BehaviorInstance::setProps(const V &attrs, PropSetFlags flags)
{
    const QString typeName = QStringLiteral("Behavior");
    parseProperty(attrs, flags, typeName, QStringLiteral("class"), &m_behavior_unresolved);

    parseProperty(attrs, flags, typeName, QStringLiteral("eyeball"), &m_eyeballEnabled);

    // Different default value.
    parseProperty(attrs, flags, typeName, QStringLiteral("name"), &m_name);
}

AliasNode::AliasNode()
    : Node(Node::Alias)
{

}

void AliasNode::setProperties(const QXmlStreamAttributes &attrs, PropSetFlags flags)
{
    Node::setProperties(attrs, flags);
    setProps(attrs, flags);
}

void AliasNode::applyPropertyChanges(const PropertyChangeList &changeList)
{
    Node::applyPropertyChanges(changeList);
    setProps(changeList, 0);
}

void AliasNode::writeQmlHeader(QTextStream &output, int tabLevel)
{
    // This is a bit special because it references a component
    // so the Comonent type is Material.(ReferencedMaterial)
    QString componentName = QStringLiteral("Aliases.") + qmlPresentationComponentName(m_referencedNode_unresolved);
    output << insertTabs(tabLevel) << componentName << QStringLiteral(" {") << endl;
}

void AliasNode::writeQmlProperties(QTextStream &output, int tabLevel)
{
    Node::writeQmlProperties(output, tabLevel);
}

template<typename V>
void AliasNode::setProps(const V &attrs, PropSetFlags flags)
{
    const QString typeName = QStringLiteral("Alias");
    parseObjectRefProperty(attrs, flags, typeName, QStringLiteral("referencednode"), &m_referencedNode_unresolved);

    // Different default value.
    parseProperty(attrs, flags, typeName, QStringLiteral("name"), &m_name);
}

UipPresentation::UipPresentation()
    : d(new UipPresentationData)
{

}

UipPresentation::~UipPresentation()
{
    delete d->scene;
    delete d->masterSlide;
}

void UipPresentation::reset()
{
    delete d->scene;
    delete d->masterSlide;
    d.reset(new UipPresentationData);
}

QString UipPresentation::sourceFile() const
{
    return d->sourceFile;
}

void UipPresentation::setSourceFile(const QString &s)
{
    d->sourceFile = s;
}

QString UipPresentation::assetFileName(const QString &xmlFileNameRef, int *part) const
{
    QString rawName = xmlFileNameRef;
    if (rawName.startsWith('#')) {
        // Can be a built-in primitive ref, like #Cube.
        if (part)
            *part = 1;
        return rawName;
    }

    if (rawName.contains('#')) {
        int pos = rawName.lastIndexOf('#');
        bool ok = false;
        int idx = rawName.mid(pos + 1).toInt(&ok);
        if (!ok) {
            qWarning() << QObject::tr("Invalid part index '%1'").arg(rawName);
            return QString();
        }
        if (part)
            *part = idx;
        rawName = rawName.left(pos);
    } else {
        // If no part is specified return -1 so the mesh parser can decide which
        // part is the best.  This will usually be 1 but for older versions
        // of the editor multi-meshes were used for revisions, and we would
        // need to return the last part in the list, not the first.
        if (part)
            *part = -1;
    }

    rawName.replace('\\', '/');
    if (rawName.startsWith(QStringLiteral("./")))
        rawName = rawName.mid(2);

    // workaround for confused users of the QML API trying to pass an URL
    // starting with qrc:/ instead of a normal string with a filename where :/
    // is the only valid option
    if (rawName.startsWith(QStringLiteral("qrc:/")))
        rawName = rawName.mid(3);

    if (QFileInfo(rawName).isAbsolute())
        return rawName;

    QString path;
    if (d->sourceFile.isEmpty()) {
        path = QFileInfo(rawName).canonicalFilePath();
    } else {
        QString fn = QFileInfo(d->sourceFile).canonicalPath();
        fn += QLatin1Char('/');
        fn += rawName;
        path = QFileInfo(fn).absoluteFilePath();

        // We need the path to uia files for some assets, for example the textures of effects
        // As that is not readily available here, go up the directory hoping to find the
        // file wanted. We're using 3 levels for now, as that is what Editor is using as well.
        if (!QFileInfo(path).exists()) {
            int loops = 0;
            QString searchPath = QFileInfo(d->sourceFile).canonicalPath();
            searchPath.append(QLatin1String("/../"));
            while (!QFileInfo(searchPath + rawName).exists() && ++loops < 3)
                searchPath.append(QLatin1String("../"));
            path = searchPath + rawName;
            if (!QFileInfo(path).exists())
                path = QFileInfo(fn).absoluteFilePath();
        }
    }
    return path;
}

QString UipPresentation::name() const
{
    return d->name;
}

void UipPresentation::setName(const QString &s)
{
    d->name = s;
}

QString UipPresentation::author() const
{
    return d->author;
}

QString UipPresentation::company() const
{
    return d->company;
}

int UipPresentation::presentationWidth() const
{
    return d->presentationWidth;
}

int UipPresentation::presentationHeight() const
{
    return d->presentationHeight;
}

UipPresentation::Rotation UipPresentation::presentationRotation() const
{
    return d->presentationRotation;
}

bool UipPresentation::maintainAspectRatio() const
{
    return d->maintainAspectRatio;
}

void UipPresentation::setAuthor(const QString &author)
{
    d->author = author;
}

void UipPresentation::setCompany(const QString &company)
{
    d->company = company;
}

void UipPresentation::setPresentationWidth(int w)
{
    d->presentationWidth = w;
}

void UipPresentation::setPresentationHeight(int h)
{
    d->presentationHeight = h;
}

void UipPresentation::setPresentationRotation(UipPresentation::Rotation r)
{
    d->presentationRotation = r;
}

void UipPresentation::setMaintainAspectRatio(bool maintain)
{
    d->maintainAspectRatio = maintain;
}

Scene *UipPresentation::scene() const
{
    return d->scene;
}

Slide *UipPresentation::masterSlide() const
{
    return d->masterSlide;
}

void UipPresentation::setScene(Scene *p)
{
    d->scene = p;
}

void UipPresentation::setMasterSlide(Slide *p)
{
    d->masterSlide = p;
}

bool UipPresentation::registerObject(const QByteArray &id, GraphObject *p)
{
    if (d->objects.contains(id)) {
        qWarning("UipPresentation: Multiple registrations for object id '%s'", id.constData());
        return false;
    }
    p->m_id = id;
    d->objects[id] = p;
    return true;
}

void UipPresentation::unregisterObject(const QByteArray &id)
{
    d->objects.remove(id);
}

void UipPresentation::registerImageBuffer(const QString &sourcePath, bool hasTransparency)
{
    d->imageBuffers[sourcePath] = hasTransparency;
}

const UipPresentation::ImageBufferMap &UipPresentation::imageBuffer() const
{
    return d->imageBuffers;
}

void UipPresentation::applyPropertyChanges(const Slide::PropertyChanges &changeList) const
{
    for (auto it = changeList.cbegin(), ite = changeList.cend(); it != ite; ++it) {
        for (auto change = it.value()->begin(); change != it.value()->end(); change++)
            qDebug() << "\t" << it.key() << "applying property change:" << change->name() << change->value();

        it.key()->applyPropertyChanges(*it.value());
    }
}

void UipPresentation::applySlidePropertyChanges(Slide *slide) const
{
    const auto &changeList = slide->propertyChanges();
    qDebug("Applying %d property changes from slide %s", changeList.count(), slide->m_id.constData());
    applyPropertyChanges(changeList);
}

GraphObject *UipPresentation::newObject(const char *type, const QByteArray &id)
{
    GraphObject *obj = nullptr;

    if (type == QByteArrayLiteral("Scene"))
        obj = newObject<Scene>(id);
    else if (type == QByteArrayLiteral("Slide"))
        obj = newObject<Slide>(id);
    else if (type == QByteArrayLiteral("Image"))
        obj = newObject<Image>(id);
    else if (type == QByteArrayLiteral("DefaultMaterial"))
        obj = newObject<DefaultMaterial>(id);
    else if (type == QByteArrayLiteral("ReferencedMaterial"))
        obj = newObject<ReferencedMaterial>(id);
    else if (type == QByteArrayLiteral("CustomMaterial"))
        obj = newObject<CustomMaterialInstance>(id);
    else if (type == QByteArrayLiteral("Effect"))
        obj = newObject<EffectInstance>(id);
    else if (type == QByteArrayLiteral("Behavior"))
        obj = newObject<BehaviorInstance>(id);
    else if (type == QByteArrayLiteral("Layer"))
        obj = newObject<LayerNode>(id);
    else if (type == QByteArrayLiteral("Camera"))
        obj = newObject<CameraNode>(id);
    else if (type == QByteArrayLiteral("Light"))
        obj = newObject<LightNode>(id);
    else if (type == QByteArrayLiteral("Model"))
        obj = newObject<ModelNode>(id);
    else if (type == QByteArrayLiteral("Group"))
        obj = newObject<GroupNode>(id);
    else if (type == QByteArrayLiteral("Text"))
        obj = newObject<TextNode>(id);
    else if (type == QByteArrayLiteral("Component"))
        obj = newObject<ComponentNode>(id);
    else if (type == QByteArrayLiteral("Alias"))
        obj = newObject<AliasNode>(id);

    return obj;
}

GraphObject *UipPresentation::getObject(const QByteArray &id) const
{
    return d->objects.value(id);
}

GraphObject *UipPresentation::getObjectByName(const QString &name) const
{
    for (auto it = d->objects.cbegin(), itEnd = d->objects.cend(); it != itEnd; ++it) {
        if ((*it)->m_name == name)
            return *it;
    }
    return nullptr;
}

QT_END_NAMESPACE


