#include "qdemonrendermaterialshadergenerator.h"
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>

QDemonMaterialShaderGeneratorInterface::QDemonMaterialShaderGeneratorInterface(QDemonRenderContextInterface *renderContext)
    : m_renderContext(renderContext),
      m_programGenerator(m_renderContext->shaderProgramGenerator())

{}

QDemonMaterialShaderGeneratorInterface::~QDemonMaterialShaderGeneratorInterface() {}
