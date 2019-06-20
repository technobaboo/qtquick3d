#ifndef QDEMONQMLUTILITIES_P_H
#define QDEMONQMLUTILITIES_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qtdemonassetimportglobal.h>

#include <QString>
#include <QColor>
#include <QTextStream>

QT_BEGIN_NAMESPACE

namespace QDemonQmlUtilities {

class PropertyMap
{
public:
    enum Type {
        Node,
        Model,
        Camera,
        Light,
        DefaultMaterial,
        Image
    };

    typedef QHash<QString, QVariant> PropertiesMap;

    static PropertyMap *instance();

    PropertiesMap *propertiesForType(Type type);
    QVariant getDefaultValue(Type type, const QString &property);
    bool isDefaultValue(Type type, const QString &property, const QVariant &value);


private:
    PropertyMap();
    ~PropertyMap();

    QHash<Type, PropertiesMap *> m_properties;

};

QString Q_DEMONASSETIMPORT_EXPORT insertTabs(int n);
QString Q_DEMONASSETIMPORT_EXPORT qmlComponentName(const QString &name);
QString Q_DEMONASSETIMPORT_EXPORT colorToQml(const QColor &color);
QString Q_DEMONASSETIMPORT_EXPORT sanitizeQmlId(const QString &id);
QString Q_DEMONASSETIMPORT_EXPORT sanitizeQmlSourcePath(const QString &source, bool removeParentDirectory = false);

void Q_DEMONASSETIMPORT_EXPORT writeQmlPropertyHelper(QTextStream &output, int tabLevel, PropertyMap::Type type, const QString &propertyName, const QVariant &value);

}


QT_BEGIN_NAMESPACE

#endif // QDEMONQMLUTILITIES_P_H
