#include "uniqueidmapper.h"

#include <QtQuick3DAssetImport/private/qssgqmlutilities_p.h>

QT_BEGIN_NAMESPACE

UniqueIdMapper *UniqueIdMapper::instance()
{
    static UniqueIdMapper mapper;
    return &mapper;
}

void UniqueIdMapper::reset()
{
    m_uniqueIdMap.clear();
    m_uniqueIds.clear();
}

QByteArray UniqueIdMapper::queryId(const QByteArray &id)
{
    return m_uniqueIdMap[id];
}

QByteArray UniqueIdMapper::generateUniqueId(const QByteArray &id)
{
    int index = 0;
    QByteArray uniqueID = QSSGQmlUtilities::sanitizeQmlId(id).toLocal8Bit();
    while (m_uniqueIds.contains(uniqueID))
        uniqueID = id + QByteArrayLiteral("_") + QString::number(++index).toLocal8Bit();
    m_uniqueIds.insert(uniqueID);
    m_uniqueIdMap.insert(id, uniqueID);
    return uniqueID;
}

UniqueIdMapper::UniqueIdMapper()
{

}

UniqueIdMapper::~UniqueIdMapper()
{

}

QT_END_NAMESPACE
