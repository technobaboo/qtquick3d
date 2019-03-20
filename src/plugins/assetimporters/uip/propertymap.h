#ifndef PROPERTYMAP_H
#define PROPERTYMAP_H

#include "uippresentation.h"

QT_BEGIN_NAMESPACE

class PropertyMap
{    
public:

    struct Property
    {
        QString name;
        Q3DS::PropertyType type;
        QVariant defaultValue;
        bool animatable = true;
        Property() = default;
        Property(const QString &n, Q3DS::PropertyType t, const QVariant &v)
            : name(n)
            , type(t)
            , defaultValue(v)
        {}
    };

    typedef QHash<QString, Property> PropertiesMap;

    static PropertyMap *instance();

    PropertiesMap *propertiesForType(GraphObject::Type type);


private:
    PropertyMap();
    ~PropertyMap();

    QHash<GraphObject::Type, PropertiesMap *> m_properties;

};

QT_END_NAMESPACE

#endif // PROPERTYMAP_H
