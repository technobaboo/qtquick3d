#include "qdemonsgrendernode_p.h"
#include <QtGui/QOpenGLContext>

QDemonSGRenderNode::QDemonSGRenderNode(QDemonSceneManager *sceneRenderer)
    : m_sceneRenderer(sceneRenderer)
{
}

QSGRenderNode::StateFlags QDemonSGRenderNode::changedStates() const
{
    return  QSGRenderNode::DepthState |
            QSGRenderNode::StencilState |
            QSGRenderNode::ScissorState |
            QSGRenderNode::ColorState |
            QSGRenderNode::BlendState |
            QSGRenderNode::CullState |
            QSGRenderNode::ViewportState;
}

void QDemonSGRenderNode::render(const QSGRenderNode::RenderState *state)
{
    QOpenGLContext *context = QOpenGLContext::currentContext();
    if (m_renderContext.isNull())
        m_renderContext = QDemonRenderContext::createGl(context->format());
    if (m_sgContext.isNull())
        m_sgContext = new QDemonRenderContextInterface(m_renderContext, QString::fromLatin1("./"));

    if (!m_layer)
        return;

    QOpenGLFunctions *f = context->functions();
    GLint viewport[4];
    f->glGetIntegerv(GL_VIEWPORT, viewport);

    m_sgContext->setSceneColor(QVector4D(0.0f, 0.0f, 0.0f, 0.0f));
    m_sgContext->renderList()->setViewport(QRect(viewport[0], viewport[1], viewport[2], viewport[3]));
    m_sgContext->setWindowDimensions(QSize(viewport[2], viewport[3]));
    m_sgContext->setViewport(QRect(viewport[0], viewport[1], viewport[2], viewport[3]));
    // Render
    m_sgContext->setPresentationDimensions(QSize(viewport[2], viewport[3]));
    m_sgContext->beginFrame();
    m_sgContext->renderContext()->resetBlendState();

    m_sgContext->renderer()->prepareLayerForRender(*m_layer, QSize(viewport[2], viewport[3]), true, nullptr);
    m_sgContext->runRenderTasks();
    m_sgContext->renderer()->renderLayer(*m_layer, QSize(viewport[2], viewport[3]), false, QVector3D(), true, nullptr);

    m_sgContext->endFrame();

}

QSGRenderNode::RenderingFlags QDemonSGRenderNode::flags() const
{
    return QSGRenderNode::RenderingFlags();
}

void QDemonSGRenderNode::setRenderLayer(QDemonRenderLayer *layer)
{
    m_layer = layer;
}
