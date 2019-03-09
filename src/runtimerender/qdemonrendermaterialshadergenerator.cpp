#include "qdemonrendermaterialshadergenerator.h"
#include <QtDemonRuntimeRender/QDemonRenderContextCoreInterface>

QDemonMaterialShaderGeneratorInterface::QDemonMaterialShaderGeneratorInterface(QDemonRenderContextInterface *renderContext)
    : m_renderContext(renderContext),
      m_programGenerator(m_renderContext->getShaderProgramGenerator())

{}

QDemonMaterialShaderGeneratorInterface::~QDemonMaterialShaderGeneratorInterface() {}
