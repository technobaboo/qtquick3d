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
#pragma once
#ifndef QDEMON_RENDER_BACKEND_GL4_H
#define QDEMON_RENDER_BACKEND_GL4_H

/// @file qdemonrenderbackendgl4.h
///       NVRender OpenGL 4 backend definition.

#include <QtDemonRender/qdemonrenderbackendgl3.h>

QT_BEGIN_NAMESPACE

class QDemonRenderBackendGL4Impl : public QDemonRenderBackendGL3Impl
{
public:
    /// constructor
    QDemonRenderBackendGL4Impl(const QSurfaceFormat &format);
    /// destructor
    virtual ~QDemonRenderBackendGL4Impl();

    public:
        void DrawIndirect(QDemonRenderDrawMode::Enum drawMode, const void *indirect) override;
    void DrawIndexedIndirect(QDemonRenderDrawMode::Enum drawMode,
                             QDemonRenderComponentTypes::Enum type, const void *indirect) override;

    void CreateTextureStorage2D(QDemonRenderBackendTextureObject to,
                                QDemonRenderTextureTargetType::Enum target, quint32 levels,
                                QDemonRenderTextureFormats::Enum internalFormat,
                                size_t width, size_t height) override;

    void SetMultisampledTextureData2D(QDemonRenderBackendTextureObject to,
                                      QDemonRenderTextureTargetType::Enum target,
                                      size_t samples,
                                      QDemonRenderTextureFormats::Enum internalFormat,
                                      size_t width, size_t height,
                                      bool fixedsamplelocations) override;

    void SetConstantValue(QDemonRenderBackendShaderProgramObject po, quint32 id,
                          QDemonRenderShaderDataTypes::Enum type, qint32 count, const void *value,
                          bool transpose) override;

    void SetPatchVertexCount(QDemonRenderBackendInputAssemblerObject iao, quint32 count) override;
    virtual QDemonRenderBackendTessControlShaderObject
    CreateTessControlShader(QDemonConstDataRef<qint8> source, QByteArray &errorMessage,
                            bool binary) override;
    virtual QDemonRenderBackendTessEvaluationShaderObject
    CreateTessEvaluationShader(QDemonConstDataRef<qint8> source, QByteArray &errorMessage,
                               bool binary) override;
    virtual QDemonRenderBackendGeometryShaderObject
    CreateGeometryShader(QDemonConstDataRef<qint8> source, QByteArray &errorMessage, bool binary) override;

    qint32 GetStorageBufferCount(QDemonRenderBackendShaderProgramObject po) override;
    qint32 GetStorageBufferInfoByID(QDemonRenderBackendShaderProgramObject po, quint32 id,
                                    quint32 nameBufSize, qint32 *paramCount,
                                    qint32 *bufferSize, qint32 *length, char *nameBuf) override;
    void ProgramSetStorageBuffer(quint32 index, QDemonRenderBackendBufferObject bo) override;

    qint32 GetAtomicCounterBufferCount(QDemonRenderBackendShaderProgramObject po) override;
    qint32 GetAtomicCounterBufferInfoByID(QDemonRenderBackendShaderProgramObject po,
                                          quint32 id, quint32 nameBufSize, qint32 *paramCount,
                                          qint32 *bufferSize, qint32 *length,
                                          char *nameBuf) override;
    void ProgramSetAtomicCounterBuffer(quint32 index, QDemonRenderBackendBufferObject bo) override;

    void SetMemoryBarrier(QDemonRenderBufferBarrierFlags barriers) override;
    void BindImageTexture(QDemonRenderBackendTextureObject to, quint32 unit, qint32 level,
                          bool layered, qint32 layer,
                          QDemonRenderImageAccessType::Enum access,
                          QDemonRenderTextureFormats::Enum format) override;

    virtual QDemonRenderBackendComputeShaderObject
    CreateComputeShader(QDemonConstDataRef<qint8> source, QByteArray &errorMessage, bool binary) override;
    void DispatchCompute(QDemonRenderBackendShaderProgramObject po, quint32 numGroupsX,
                         quint32 numGroupsY, quint32 numGroupsZ) override;

    QDemonRenderBackendProgramPipeline CreateProgramPipeline() override;
    void ReleaseProgramPipeline(QDemonRenderBackendProgramPipeline ppo) override;
    void SetActiveProgramPipeline(QDemonRenderBackendProgramPipeline ppo) override;
    void SetProgramStages(QDemonRenderBackendProgramPipeline ppo,
                          QDemonRenderShaderTypeFlags flags,
                          QDemonRenderBackendShaderProgramObject po) override;

    void SetBlendEquation(const QDemonRenderBlendEquationArgument &pBlendEquArg) override;
    void SetBlendBarrier(void) override;

    QDemonRenderBackendPathObject CreatePathNVObject(size_t range) override;
    void SetPathSpecification(QDemonRenderBackendPathObject inPathObject,
                              QDemonConstDataRef<quint8> inPathCommands,
                              QDemonConstDataRef<float> inPathCoords) override;
    QDemonBounds3 GetPathObjectBoundingBox(QDemonRenderBackendPathObject inPathObject) override;
    QDemonBounds3 GetPathObjectFillBox(QDemonRenderBackendPathObject inPathObject) override;
    QDemonBounds3 GetPathObjectStrokeBox(QDemonRenderBackendPathObject inPathObject) override;
    void SetStrokeWidth(QDemonRenderBackendPathObject inPathObject, float inStrokeWidth) override;

    void SetPathProjectionMatrix(const QMatrix4x4 inPathProjection) override;
    void SetPathModelViewMatrix(const QMatrix4x4 inPathModelview) override;
    void SetPathStencilDepthOffset(float inSlope, float inBias) override;
    void SetPathCoverDepthFunc(QDemonRenderBoolOp::Enum inDepthFunction) override;
    void StencilStrokePath(QDemonRenderBackendPathObject inPathObject) override;
    void StencilFillPath(QDemonRenderBackendPathObject inPathObject) override;
    void ReleasePathNVObject(QDemonRenderBackendPathObject po, size_t range) override;

    void StencilFillPathInstanced(
            QDemonRenderBackendPathObject po, size_t numPaths, QDemonRenderPathFormatType::Enum type,
            const void *charCodes, QDemonRenderPathFillMode::Enum fillMode, quint32 stencilMask,
            QDemonRenderPathTransformType::Enum transformType, const float *transformValues) override;
    void StencilStrokePathInstancedN(QDemonRenderBackendPathObject po, size_t numPaths,
                                     QDemonRenderPathFormatType::Enum type,
                                     const void *charCodes, qint32 stencilRef,
                                     quint32 stencilMask,
                                     QDemonRenderPathTransformType::Enum transformType,
                                     const float *transformValues) override;
    void CoverFillPathInstanced(QDemonRenderBackendPathObject po, size_t numPaths,
                                QDemonRenderPathFormatType::Enum type,
                                const void *charCodes,
                                QDemonRenderPathCoverMode::Enum coverMode,
                                QDemonRenderPathTransformType::Enum transformType,
                                const float *transformValues) override;
    void CoverStrokePathInstanced(QDemonRenderBackendPathObject po, size_t numPaths,
                                  QDemonRenderPathFormatType::Enum type,
                                  const void *charCodes,
                                  QDemonRenderPathCoverMode::Enum coverMode,
                                  QDemonRenderPathTransformType::Enum transformType,
                                  const float *transformValues) override;
    void LoadPathGlyphs(QDemonRenderBackendPathObject po,
                        QDemonRenderPathFontTarget::Enum fontTarget, const void *fontName,
                        QDemonRenderPathFontStyleFlags fontStyle, size_t numGlyphs,
                        QDemonRenderPathFormatType::Enum type, const void *charCodes,
                        QDemonRenderPathMissingGlyphs::Enum handleMissingGlyphs,
                        QDemonRenderBackendPathObject pathParameterTemplate, float emScale) override;
    virtual QDemonRenderPathReturnValues::Enum
    LoadPathGlyphsIndexed(QDemonRenderBackendPathObject po, QDemonRenderPathFontTarget::Enum fontTarget,
                          const void *fontName, QDemonRenderPathFontStyleFlags fontStyle,
                          quint32 firstGlyphIndex, size_t numGlyphs,
                          QDemonRenderBackendPathObject pathParameterTemplate, float emScale) override;
    virtual QDemonRenderBackendPathObject
    LoadPathGlyphsIndexedRange(QDemonRenderPathFontTarget::Enum fontTarget, const void *fontName,
                               QDemonRenderPathFontStyleFlags fontStyle,
                               QDemonRenderBackendPathObject pathParameterTemplate, float emScale,
                               quint32 *count) override;
    void LoadPathGlyphRange(QDemonRenderBackendPathObject po,
                            QDemonRenderPathFontTarget::Enum fontTarget,
                            const void *fontName, QDemonRenderPathFontStyleFlags fontStyle,
                            quint32 firstGlyph, size_t numGlyphs,
                            QDemonRenderPathMissingGlyphs::Enum handleMissingGlyphs,
                            QDemonRenderBackendPathObject pathParameterTemplate,
                            float emScale) override;
    void GetPathMetrics(QDemonRenderBackendPathObject po, size_t numPaths,
                        QDemonRenderPathGlyphFontMetricFlags metricQueryMask,
                        QDemonRenderPathFormatType::Enum type, const void *charCodes,
                        size_t stride, float *metrics) override;
    void GetPathMetricsRange(QDemonRenderBackendPathObject po, size_t numPaths,
                             QDemonRenderPathGlyphFontMetricFlags metricQueryMask,
                             size_t stride, float *metrics) override;
    void GetPathSpacing(QDemonRenderBackendPathObject po, size_t numPaths,
                        QDemonRenderPathListMode::Enum pathListMode,
                        QDemonRenderPathFormatType::Enum type, const void *charCodes,
                        float advanceScale, float kerningScale,
                        QDemonRenderPathTransformType::Enum transformType, float *spacing) override;
private:
#if !defined(QT_OPENGL_ES)
    QOpenGLExtension_NV_path_rendering *m_nvPathRendering;
    QOpenGLExtension_EXT_direct_state_access *m_directStateAccess;
#endif
};

QT_END_NAMESPACE

#endif
