#include "qdemonrenderer.h"

QT_BEGIN_NAMESPACE

bool IQDemonRenderer::IsGlEsContext(QDemonRenderContextType inContextType)
{
    QDemonRenderContextType esContextTypes(QDemonRenderContextValues::GLES2
                                           | QDemonRenderContextValues::GLES3
                                           | QDemonRenderContextValues::GLES3PLUS);

    if ((inContextType & esContextTypes))
        return true;

    return false;
}

bool IQDemonRenderer::IsGlEs3Context(QDemonRenderContextType inContextType)
{
    if (inContextType == QDemonRenderContextValues::GLES3
            || inContextType == QDemonRenderContextValues::GLES3PLUS)
        return true;

    return false;
}

bool IQDemonRenderer::IsGl2Context(QDemonRenderContextType inContextType)
{
    if (inContextType == QDemonRenderContextValues::GL2)
        return true;

    return false;
}

QT_END_NAMESPACE
