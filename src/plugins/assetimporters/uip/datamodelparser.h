#ifndef DATAMODELPARSER_H
#define DATAMODELPARSER_H


#include "abstractxmlparser.h"
#include "uippresentation.h"
#include <QDebug>

QT_BEGIN_NAMESPACE

class DataModelParser : public AbstractXmlParser
{
public:
    struct Property
    {
        QString name;
        Q3DS::PropertyType type = Q3DS::Unknown;
        int componentCount = 1;
        QString typeStr;
        QStringList enumValues;
        QString defaultValue;
        bool animatable = true;
    };

    static DataModelParser *instance();

    const QVector<Property> *propertiesForType(const QString &typeName);

private:
    DataModelParser();
    void parseMetaData();
    void parseProperty(QVector<Property> *props);

    bool m_valid = false;

    QHash<QString, QVector<Property> > m_props;
};

QDebug operator<<(QDebug dbg, const DataModelParser::Property &prop);

QT_END_NAMESPACE
#endif // DATAMODELPARSER_H
