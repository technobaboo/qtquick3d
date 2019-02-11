#include "qdemonscene.h"

QT_BEGIN_NAMESPACE

QDemonScene::QDemonScene()
{

}

QDemonScene::~QDemonScene()
{

}

QDemonObject::Type QDemonScene::type() const
{
    return QDemonObject::Scene;
}

bool QDemonScene::useClearColor() const
{
    return m_useClearColor;
}

QColor QDemonScene::clearColor() const
{
    return m_clearColor;
}

void QDemonScene::setUseClearColor(bool useClearColor)
{
    if (m_useClearColor == useClearColor)
        return;

    m_useClearColor = useClearColor;
    emit useClearColorChanged(m_useClearColor);
    update();
}

void QDemonScene::setClearColor(QColor clearColor)
{
    if (m_clearColor == clearColor)
        return;

    m_clearColor = clearColor;
    emit clearColorChanged(m_clearColor);
    update();
}

SGraphObject *QDemonScene::updateSpatialNode(SGraphObject *node)
{
    if (!node) {
        m_sceneNode = new SScene();
        node = m_sceneNode;
    }

    m_sceneNode = static_cast<SScene*>(node);
    m_sceneNode->m_ClearColor.setX(m_clearColor.redF());
    m_sceneNode->m_ClearColor.setY(m_clearColor.greenF());
    m_sceneNode->m_ClearColor.setZ(m_clearColor.blueF());
    m_sceneNode->m_UseClearColor = m_useClearColor;

    return m_sceneNode;
}

QT_END_NAMESPACE
