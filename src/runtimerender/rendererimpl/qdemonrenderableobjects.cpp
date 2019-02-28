/****************************************************************************
**
** Copyright (C) 2008-2012 NVIDIA Corporation.
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt 3D Studio.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "qdemonrenderableobjects.h"

#include <QtDemonRuntimeRender/qdemonrendererimpl.h>
#include <QtDemonRuntimeRender/qdemonrendercustommaterialsystem.h>
#include <QtDemonRuntimeRender/qdemonrendercustommaterialrendercontext.h>
#include <QtDemonRuntimeRender/qdemonrenderlight.h>
#include <QtDemonRuntimeRender/qdemonrenderpathmanager.h>
#include <QtDemonRuntimeRender/qdemonrenderpathrendercontext.h>
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterialshadergenerator.h>

QT_BEGIN_NAMESPACE
struct QDemonRenderableImage;
struct QDemonShaderGeneratorGeneratedShader;
struct QDemonSubsetRenderable;

QDemonTextScaleAndOffset::QDemonTextScaleAndOffset(QDemonRenderTexture2D &inTexture,
                                         const QDemonTextTextureDetails &inTextDetails,
                                         const QDemonTextRenderInfo &inInfo)
    : textOffset(0, 0)
    , textScale(1, 1)

{
    QDemonRenderTexture2D &theTexture = inTexture;
    QDemonTextureDetails theDetails(theTexture.getTextureDetails());
    QVector2D textDimensions(inTextDetails.textWidth / 2.0f, inTextDetails.textHeight / 2.0f);
    textDimensions.setX(textDimensions.x() / inTextDetails.scaleFactor.x());
    textDimensions.setY(textDimensions.y() / inTextDetails.scaleFactor.y());
    QVector2D theTextScale(textDimensions.x(), textDimensions.y());
    QVector2D theTextOffset(0, 0);

    // Set the offsets to use after scaling the rect coordinates.
    switch (inInfo.horizontalAlignment) {
    case TextHorizontalAlignment::Left:
        theTextOffset[0] = theTextScale[0];
        break;
    case TextHorizontalAlignment::Center:
        break;
    case TextHorizontalAlignment::Right:
        theTextOffset[0] = -theTextScale[0];
        break;
    default:
        break;
    }

    switch (inInfo.verticalAlignment) {
    case TextVerticalAlignment::Top:
        theTextOffset[1] = -theTextScale[1];
        break;
    case TextVerticalAlignment::Middle:
        break;
    case TextVerticalAlignment::Bottom:
        theTextOffset[1] = theTextScale[1];
        break;
    default:
        break;
    }
    textScale = theTextScale;
    textOffset = theTextOffset;
}

void QDemonSubsetRenderableBase::renderShadowMapPass(const QVector2D &inCameraVec,
                                                     const QDemonRenderLight *inLight,
                                                     const QDemonRenderCamera &inCamera,
                                                     QDemonShadowMapEntry *inShadowMapEntry)
{
    auto context = generator->getContext();
    QSharedPointer<QDemonRenderableDepthPrepassShader> shader = nullptr;
    QSharedPointer<QDemonRenderInputAssembler> pIA = nullptr;

    /*
        if ( inLight->m_LightType == RenderLightTypes::Area )
                shader = m_Generator.GetParaboloidDepthShader( m_TessellationMode );
        else if ( inLight->m_LightType == RenderLightTypes::Directional )
                shader = m_Generator.GetOrthographicDepthShader( m_TessellationMode );
        else if ( inLight->m_LightType == RenderLightTypes::Point )
                shader = m_Generator.GetCubeShadowDepthShader( m_TessellationMode );	// This
        will change to include a geometry shader pass.
        */

    if (inLight->m_lightType == RenderLightTypes::Directional)
        shader = generator->getOrthographicDepthShader(tessellationMode);
    else
        shader = generator->getCubeShadowDepthShader(tessellationMode);

    if (shader == nullptr || inShadowMapEntry == nullptr)
        return;

    // for phong and npatch tesselleation we need the normals too
    if (tessellationMode == TessModeValues::NoTess
            || tessellationMode == TessModeValues::TessLinear)
        pIA = subset.inputAssemblerDepth;
    else
        pIA = subset.inputAssembler;

    QMatrix4x4 theModelViewProjection = inShadowMapEntry->m_lightVP * globalTransform;
    // QMatrix4x4 theModelView = inLight->m_GlobalTransform.getInverse() * m_GlobalTransform;

    context->setActiveShader(shader->shader);
    shader->mvp.set(theModelViewProjection);
    shader->cameraPosition.set(inCamera.position);
    shader->globalTransform.set(globalTransform);
    shader->cameraProperties.set(inCameraVec);
    /*
        shader->m_CameraDirection.Set( inCamera.GetDirection() );

        shader->m_shadowMV[0].set( inShadowMapEntry->m_lightCubeView[0] * m_globalTransform );
        shader->m_shadowMV[1].set( inShadowMapEntry->m_lightCubeView[1] * m_globalTransform );
        shader->m_shadowMV[2].set( inShadowMapEntry->m_lightCubeView[2] * m_globalTransform );
        shader->m_shadowMV[3].set( inShadowMapEntry->m_lightCubeView[3] * m_globalTransform );
        shader->m_shadowMV[4].set( inShadowMapEntry->m_lightCubeView[4] * m_globalTransform );
        shader->m_shadowMV[5].set( inShadowMapEntry->m_lightCubeView[5] * m_globalTransform );
        shader->m_projection.set( inCamera.m_projection );
        */

    // tesselation
    if (tessellationMode != TessModeValues::NoTess) {
        // set uniforms we need
        shader->tessellation.edgeTessLevel.set(subset.edgeTessFactor);
        shader->tessellation.insideTessLevel.set(subset.innerTessFactor);
        // the blend value is hardcoded
        shader->tessellation.phongBlend.set(0.75);
        // set distance range value
        shader->tessellation.distanceRange.set(inCameraVec);
        // disable culling
        shader->tessellation.disableCulling.set(1.0);
    }

    context->setInputAssembler(pIA);
    context->draw(subset.primitiveType, subset.count, subset.offset);
}

void QDemonSubsetRenderableBase::renderDepthPass(const QVector2D &inCameraVec,
                                                 QDemonRenderableImage *inDisplacementImage,
                                                 float inDisplacementAmount)
{
    auto context = generator->getContext();
    QSharedPointer<QDemonRenderableDepthPrepassShader> shader = nullptr;
    QSharedPointer<QDemonRenderInputAssembler> pIA = nullptr;
    QDemonRenderableImage *displacementImage = inDisplacementImage;

    if (subset.primitiveType != QDemonRenderDrawMode::Patches)
        shader = generator->getDepthPrepassShader(displacementImage != nullptr);
    else
        shader = generator->getDepthTessPrepassShader(tessellationMode,
                                                       displacementImage != nullptr);

    if (shader == nullptr)
        return;

    // for phong and npatch tesselleation or displacement mapping we need the normals (and uv's)
    // too
    if ((tessellationMode == TessModeValues::NoTess
         || tessellationMode == TessModeValues::TessLinear)
            && !displacementImage)
        pIA = subset.inputAssemblerDepth;
    else
        pIA = subset.inputAssembler;

    context->setActiveShader(shader->shader);
    context->setCullingEnabled(true);

    shader->mvp.set(modelContext.modelViewProjection);

    if (displacementImage) {
        // setup image transform
        const QMatrix4x4 &textureTransform = displacementImage->m_image.m_textureTransform;
        const float *dataPtr(textureTransform.data());
        QVector3D offsets(dataPtr[12], dataPtr[13],
                displacementImage->m_image.m_textureData.m_textureFlags.isPreMultiplied()
                ? 1.0f
                : 0.0f);
        QVector4D rotations(dataPtr[0], dataPtr[4], dataPtr[1], dataPtr[5]);
        displacementImage->m_image.m_textureData.m_texture->setTextureWrapS(
                    displacementImage->m_image.m_horizontalTilingMode);
        displacementImage->m_image.m_textureData.m_texture->setTextureWrapT(
                    displacementImage->m_image.m_verticalTilingMode);

        shader->displaceAmount.set(inDisplacementAmount);
        shader->displacementProps.offsets.set(offsets);
        shader->displacementProps.rotations.set(rotations);
        shader->displacementProps.sampler.set(displacementImage->m_image.m_textureData.m_texture.data());
    }

    // tesselation
    if (tessellationMode != TessModeValues::NoTess) {
        // set uniforms we need
        shader->globalTransform.set(globalTransform);

        if (generator->getLayerRenderData() && generator->getLayerRenderData()->camera)
            shader->cameraPosition.set(
                        generator->getLayerRenderData()->camera->getGlobalPos());
        else if (generator->getLayerRenderData()->camera)
            shader->cameraPosition.set(QVector3D(0.0, 0.0, 1.0));

        shader->tessellation.edgeTessLevel.set(subset.edgeTessFactor);
        shader->tessellation.insideTessLevel.set(subset.innerTessFactor);
        // the blend value is hardcoded
        shader->tessellation.phongBlend.set(0.75);
        // set distance range value
        shader->tessellation.distanceRange.set(inCameraVec);
        // enable culling
        shader->tessellation.disableCulling.set(0.0);
    }

    context->setInputAssembler(pIA);
    context->draw(subset.primitiveType, subset.count, subset.offset);
}

// An interface to the shader generator that is available to the renderables

void QDemonSubsetRenderable::render(const QVector2D &inCameraVec, TShaderFeatureSet inFeatureSet)
{
    auto context = generator->getContext();

    QSharedPointer<QDemonShaderGeneratorGeneratedShader> shader = generator->getShader(*this, inFeatureSet);
    if (shader == nullptr)
        return;

    context->setActiveShader(shader->shader);

    generator->getDemonContext()->getDefaultMaterialShaderGenerator()->setMaterialProperties(
                shader->shader, material, inCameraVec, modelContext.modelViewProjection,
                modelContext.normalMatrix, modelContext.model.globalTransform, firstImage,
                opacity, generator->getLayerGlobalRenderProperties());

    // tesselation
    if (subset.primitiveType == QDemonRenderDrawMode::Patches) {
        shader->tessellation.edgeTessLevel.set(subset.edgeTessFactor);
        shader->tessellation.insideTessLevel.set(subset.innerTessFactor);
        // the blend value is hardcoded
        shader->tessellation.phongBlend.set(0.75);
        // this should finally be based on some user input
        shader->tessellation.distanceRange.set(inCameraVec);
        // enable culling
        shader->tessellation.disableCulling.set(0.0);

        if (subset.wireframeMode) {
            // we need the viewport matrix
            QRect theViewport(context->getViewport());
            float matrixData[16] = {
                float(theViewport.width()) / 2.0f, 0.0f, 0.0f, 0.0f,
                0.0f, float(theViewport.width()) / 2.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                float(theViewport.width()) / 2.0f + float(theViewport.x()),
                float(theViewport.height()) / 2.0f + float(theViewport.y()),
                0.0f, 1.0f
            };
            QMatrix4x4 vpMatrix(matrixData);
            shader->viewportMatrix.set(vpMatrix);
        }
    }

    context->setCullingEnabled(true);
    context->setInputAssembler(subset.inputAssembler);
    context->draw(subset.primitiveType, subset.count, subset.offset);
}

void QDemonSubsetRenderable::renderDepthPass(const QVector2D &inCameraVec)
{
    QDemonRenderableImage *displacementImage = nullptr;
    for (QDemonRenderableImage *theImage = firstImage;
         theImage != nullptr && displacementImage == nullptr; theImage = theImage->m_nextImage) {
        if (theImage->m_mapType == QDemonImageMapTypes::Displacement)
            displacementImage = theImage;
    }
    QDemonSubsetRenderableBase::renderDepthPass(inCameraVec, displacementImage,
                                           material.displaceAmount);
}

void QDemonTextRenderable::render(const QVector2D &inCameraVec)
{
    QSharedPointer<QDemonRenderContext> context(generator.getContext());

    if (!text.m_pathFontDetails) {

        QDemonTextRenderHelper theInfo = generator.getShader(*this, false);
        if (theInfo.shader == nullptr)
            return;
        // All of our shaders produce premultiplied values.
        QDemonRenderBlendFunctionArgument blendFunc(
                    QDemonRenderSrcBlendFunc::One, QDemonRenderDstBlendFunc::OneMinusSrcAlpha,
                    QDemonRenderSrcBlendFunc::One, QDemonRenderDstBlendFunc::OneMinusSrcAlpha);

        QDemonRenderBlendEquationArgument blendEqu(QDemonRenderBlendEquation::Add,
                                                   QDemonRenderBlendEquation::Add);

        context->setBlendFunction(blendFunc);
        context->setBlendEquation(blendEqu);
        QVector4D theColor(text.m_textColor, text.globalOpacity);

        QDemonTextShader &shader(*theInfo.shader);
        shader.render(text.m_textTexture, *this, theColor, modelViewProjection,
                      inCameraVec, context, theInfo.quadInputAssembler,
                      theInfo.quadInputAssembler->getIndexCount(), text.m_textTextureDetails,
                      QVector3D(0, 0, 0));
    } else {
        Q_ASSERT(context->isPathRenderingSupported() && context->isProgramPipelineSupported());

        QDemonTextRenderHelper theInfo = generator.getShader(*this, true);
        if (theInfo.shader == nullptr)
            return;

        // All of our shaders produce premultiplied values.
        QDemonRenderBlendFunctionArgument blendFunc(
                    QDemonRenderSrcBlendFunc::One, QDemonRenderDstBlendFunc::OneMinusSrcAlpha,
                    QDemonRenderSrcBlendFunc::One, QDemonRenderDstBlendFunc::OneMinusSrcAlpha);

        QDemonRenderBlendEquationArgument blendEqu(QDemonRenderBlendEquation::Add,
                                                   QDemonRenderBlendEquation::Add);

        context->setBlendFunction(blendFunc);
        context->setBlendEquation(blendEqu);
        QVector4D theColor(text.m_textColor, text.globalOpacity);

        QDemonTextShader &shader(*theInfo.shader);

        shader.renderPath(text.m_pathFontItem, text.m_pathFontDetails, *this, theColor,
                          viewProjection, globalTransform, inCameraVec, context,
                          text.m_textTextureDetails, QVector3D(0, 0, 0));
    }
}

void QDemonTextRenderable::renderDepthPass(const QVector2D &inCameraVec)
{
    auto context = generator.getContext();
    QSharedPointer<QDemonTextDepthShader> theDepthShader = generator.getTextDepthShader();
    if (theDepthShader == nullptr)
        return;

    if (!text.m_pathFontDetails) {
        // we may change stencil test state
        QDemonRenderContextScopedProperty<bool> __stencilTest(*context, &QDemonRenderContext::isStencilTestEnabled, &QDemonRenderContext::setStencilTestEnabled, true);

        QSharedPointer<QDemonRenderShaderProgram> theShader(theDepthShader->shader);
        context->setCullingEnabled(false);
        context->setActiveShader(theShader);
        theDepthShader->mvp.set(modelViewProjection);
        theDepthShader->sampler.set(text.m_textTexture.data());
        const QDemonTextScaleAndOffset &theScaleAndOffset(*this);
        theDepthShader->dimensions.set(
                    QVector4D(theScaleAndOffset.textScale.x(), theScaleAndOffset.textScale.y(),
                              theScaleAndOffset.textOffset.x(), theScaleAndOffset.textOffset.y()));
        theDepthShader->cameraProperties.set(inCameraVec);

        QDemonTextureDetails theTextureDetails = text.m_textTexture->getTextureDetails();
        const QDemonTextTextureDetails &theTextTextureDetails(text.m_textTextureDetails);
        float theWidthScale =
                (float)theTextTextureDetails.textWidth / (float)theTextureDetails.width;
        float theHeightScale =
                (float)theTextTextureDetails.textHeight / (float)theTextureDetails.height;
        theDepthShader->textDimensions.set(
                    QVector3D(theWidthScale, theHeightScale, theTextTextureDetails.flipY ? 1.0f : 0.0f));
        context->setInputAssembler(theDepthShader->quadInputAssembler);
        context->draw(QDemonRenderDrawMode::Triangles,
                     theDepthShader->quadInputAssembler->getIndexCount(), 0);
    } else {
        QDemonRenderBoolOp::Enum theDepthFunction = context->getDepthFunction();
        bool isDepthEnabled = context->isDepthTestEnabled();
        bool isStencilEnabled = context->isStencilTestEnabled();
        bool isDepthWriteEnabled = context->isDepthWriteEnabled();
        QDemonRenderStencilFunctionArgument theArg(QDemonRenderBoolOp::NotEqual, 0, 0xFF);
        QDemonRenderStencilOperationArgument theOpArg(QDemonRenderStencilOp::Keep,
                                                      QDemonRenderStencilOp::Keep,
                                                      QDemonRenderStencilOp::Zero);
        QSharedPointer<QDemonRenderDepthStencilState> depthStencilState =
                context->createDepthStencilState(isDepthEnabled, isDepthWriteEnabled,
                                                theDepthFunction, false, theArg, theArg, theOpArg,
                                                theOpArg);

        context->setActiveShader(nullptr);
        context->setCullingEnabled(false);

        context->setDepthStencilState(depthStencilState);

        // setup transform
        QMatrix4x4 offsetMatrix;
        offsetMatrix(3, 0) = textOffset.x() - float(text.m_textTextureDetails.textWidth) / 2.0f;
        offsetMatrix(3, 1) = textOffset.y() - float(text.m_textTextureDetails.textHeight) / 2.0f;

        QMatrix4x4 pathMatrix = text.m_pathFontItem->getTransform();

        context->setPathProjectionMatrix(viewProjection);
        context->setPathModelViewMatrix(globalTransform * offsetMatrix * pathMatrix);

        // first pass
        text.m_pathFontDetails->stencilFillPathInstanced(text.m_pathFontItem);

        // second pass
        context->setStencilTestEnabled(true);
        text.m_pathFontDetails->coverFillPathInstanced(text.m_pathFontItem);

        context->setStencilTestEnabled(isStencilEnabled);
        context->setDepthFunction(theDepthFunction);
    }
}

void QDemonCustomMaterialRenderable::render(const QVector2D & /*inCameraVec*/,
                                       const QDemonLayerRenderData &inLayerData,
                                       const QDemonRenderLayer &inLayer,
                                       const QVector<QDemonRenderLight *> &inLights,
                                       const QDemonRenderCamera &inCamera,
                                       const QSharedPointer<QDemonRenderTexture2D> inDepthTexture,
                                       const QSharedPointer<QDemonRenderTexture2D> inSsaoTexture,
                                       TShaderFeatureSet inFeatureSet)
{
    QDemonRenderContextInterface *demonContext(generator->getDemonContext());
    QDemonCustomMaterialRenderContext theRenderContext(
                inLayer, inLayerData, inLights, inCamera, modelContext.model, subset,
                modelContext.modelViewProjection, globalTransform, modelContext.normalMatrix,
                material, inDepthTexture, inSsaoTexture, shaderDescription, firstImage,
                opacity);

    demonContext->getCustomMaterialSystem()->renderSubset(theRenderContext, inFeatureSet);
}

void QDemonCustomMaterialRenderable::renderDepthPass(const QVector2D &inCameraVec,
                                                const QDemonRenderLayer & /*inLayer*/,
                                                const QVector<QDemonRenderLight *> /*inLights*/
                                                ,
                                                const QDemonRenderCamera & /*inCamera*/,
                                                const QDemonRenderTexture2D * /*inDepthTexture*/)
{

    QDemonRenderContextInterface *demonContext(generator->getDemonContext());
    if (!demonContext->getCustomMaterialSystem()->renderDepthPrepass(
                modelContext.modelViewProjection, material, subset)) {
        QDemonRenderableImage *displacementImage = nullptr;
        for (QDemonRenderableImage *theImage = firstImage;
             theImage != nullptr && displacementImage == nullptr; theImage = theImage->m_nextImage) {
            if (theImage->m_mapType == QDemonImageMapTypes::Displacement)
                displacementImage = theImage;
        }

        QDemonSubsetRenderableBase::renderDepthPass(inCameraVec, displacementImage,
                                               material.m_displaceAmount);
    }
}

void QDemonPathRenderable::renderDepthPass(const QVector2D &inCameraVec, const QDemonRenderLayer & /*inLayer*/,
                                      const QVector<QDemonRenderLight *> &inLights,
                                      const QDemonRenderCamera &inCamera,
                                      const QDemonRenderTexture2D * /*inDepthTexture*/)
{
    QDemonRenderContextInterface *demonContext(m_generator->getDemonContext());
    QDemonPathRenderContext theRenderContext(
                inLights, inCamera, m_path, m_mvp, globalTransform, m_normalMatrix,
                m_opacity, m_material, m_shaderDescription, m_firstImage, demonContext->getWireframeMode(),
                inCameraVec, false, m_isStroke);

    demonContext->getPathManager()->renderDepthPrepass(
                theRenderContext, m_generator->getLayerGlobalRenderProperties(), TShaderFeatureSet());
}

void QDemonPathRenderable::render(const QVector2D &inCameraVec, const QDemonRenderLayer & /*inLayer*/,
                             const QVector<QDemonRenderLight *> &inLights, const QDemonRenderCamera &inCamera,
                             const QSharedPointer<QDemonRenderTexture2D> /*inDepthTexture*/
                             ,
                             const QSharedPointer<QDemonRenderTexture2D> /*inSsaoTexture*/
                             ,
                             TShaderFeatureSet inFeatureSet)
{
    QDemonRenderContextInterface *demonContext(m_generator->getDemonContext());
    QDemonPathRenderContext theRenderContext(
                inLights, inCamera, m_path, m_mvp, globalTransform, m_normalMatrix,
                m_opacity, m_material, m_shaderDescription, m_firstImage, demonContext->getWireframeMode(),
                inCameraVec, renderableFlags.hasTransparency(), m_isStroke);

    demonContext->getPathManager()->renderPath(theRenderContext, m_generator->getLayerGlobalRenderProperties(), inFeatureSet);
}

void QDemonPathRenderable::renderShadowMapPass(const QVector2D &inCameraVec, const QDemonRenderLight *inLight,
                                          const QDemonRenderCamera &inCamera,
                                          QDemonShadowMapEntry *inShadowMapEntry)
{
    QVector<QDemonRenderLight *> theLights;
    QDemonRenderContextInterface *demonContext(m_generator->getDemonContext());

    QMatrix4x4 theModelViewProjection = inShadowMapEntry->m_lightVP * globalTransform;
    QDemonPathRenderContext theRenderContext(
                theLights, inCamera, m_path, theModelViewProjection, globalTransform, m_normalMatrix,
                m_opacity, m_material, m_shaderDescription, m_firstImage, demonContext->getWireframeMode(),
                inCameraVec, false, m_isStroke);

    if (inLight->m_lightType != RenderLightTypes::Directional) {
        demonContext->getPathManager()->renderCubeFaceShadowPass(
                    theRenderContext, m_generator->getLayerGlobalRenderProperties(),
                    TShaderFeatureSet());
    } else
        demonContext->getPathManager()->renderShadowMapPass(
                    theRenderContext, m_generator->getLayerGlobalRenderProperties(),
                    TShaderFeatureSet());
}
QT_END_NAMESPACE
