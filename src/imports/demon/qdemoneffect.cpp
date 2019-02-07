#include "qdemoneffect.h"
#include <QtDemonRuntimeRender/qdemonrendereffect.h>

QDemonEffect::QDemonEffect()
{

}

QDemonEffect::~QDemonEffect()
{

}

QDemonObject::Type QDemonEffect::type() const
{
    return QDemonObject::Effect;
}

QString QDemonEffect::source() const
{
    return m_source;
}

void QDemonEffect::setSource(QString source)
{
    if (m_source == source)
        return;

    m_source = source;
    emit sourceChanged(m_source);
}

SGraphObject *QDemonEffect::updateSpatialNode(SGraphObject *node)
{
    // TODO: Add Effect Node and update properties

    return node;

}
