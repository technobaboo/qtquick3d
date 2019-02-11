#include "qdemonobject.h"

QT_BEGIN_NAMESPACE

QDemonObject::QDemonObject(QObject *parent) : QObject(parent)
{

}

QDemonObject::~QDemonObject()
{

}

void QDemonObject::update()
{

}

QByteArray QDemonObject::id() const
{
    return m_id;
}

QString QDemonObject::name() const
{
    return m_name;
}

void QDemonObject::setName(QString name)
{
    m_name = name;
}

void QDemonObject::markDirty()
{

}

void QDemonObject::addToDirtyList()
{

}

void QDemonObject::removeFromDirtyList()
{

}

QT_END_NAMESPACE
