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
    void drawIndirect(QDemonRenderDrawMode::Enum drawMode, const void *indirect) override;
    void drawIndexedIndirect(QDemonRenderDrawMode::Enum drawMode, QDemonRenderComponentTypes::Enum type, const void *indirect) override;

    void createTextureStorage2D(QDemonRenderBackendTextureObject to,
                                QDemonRenderTextureTargetType::Enum target,
                                qint32 levels,
                                QDemonRenderTextureFormats::Enum internalFormat,
                                qint32 width,
                                qint32 height) override;

    void setMultisampledTextureData2D(QDemonRenderBackendTextureObject to,
                                      QDemonRenderTextureTargetType::Enum target,
                                      qint32 samples,
                                      QDemonRenderTextureFormats::Enum internalFormat,
                                      qint32 width,
                                      qint32 height,
                                      bool fixedsamplelocations) override;

    void setConstantValue(QDemonRenderBackendShaderProgramObject po,
                          quint32 id,
                          QDemonRenderShaderDataTypes::Enum type,
                          qint32 count,
                          const void *value,
                          bool transpose) override;

    void setPatchVertexCount(QDemonRenderBackendInputAssemblerObject iao, quint32 count) override;
    virtual QDemonRenderBackendTessControlShaderObject createTessControlShader(QDemonConstDataRef<qint8> source,
                                                                               QByteArray &errorMessage,
                                                                               bool binary) override;
    virtual QDemonRenderBackendTessEvaluationShaderObject createTessEvaluationShader(QDemonConstDataRef<qint8> source,
                                                                                     QByteArray &errorMessage,
                                                                                     bool binary) override;
    virtual QDemonRenderBackendGeometryShaderObject createGeometryShader(QDemonConstDataRef<qint8> source,
                                                                         QByteArray &errorMessage,
                                                                         bool binary) override;

    qint32 getStorageBufferCount(QDemonRenderBackendShaderProgramObject po) override;
    qint32 getStorageBufferInfoByID(QDemonRenderBackendShaderProgramObject po,
                                    quint32 id,
                                    quint32 nameBufSize,
                                    qint32 *paramCount,
                                    qint32 *bufferSize,
                                    qint32 *length,
                                    char *nameBuf) override;
    void programSetStorageBuffer(quint32 index, QDemonRenderBackendBufferObject bo) override;

    qint32 getAtomicCounterBufferCount(QDemonRenderBackendShaderProgramObject po) override;
    qint32 getAtomicCounterBufferInfoByID(QDemonRenderBackendShaderProgramObject po,
                                          quint32 id,
                                          quint32 nameBufSize,
                                          qint32 *paramCount,
                                          qint32 *bufferSize,
                                          qint32 *length,
                                          char *nameBuf) override;
    void programSetAtomicCounterBuffer(quint32 index, QDemonRenderBackendBufferObject bo) override;

    void setMemoryBarrier(QDemonRenderBufferBarrierFlags barriers) override;
    void bindImageTexture(QDemonRenderBackendTextureObject to,
                          quint32 unit,
                          qint32 level,
                          bool layered,
                          qint32 layer,
                          QDemonRenderImageAccessType::Enum access,
                          QDemonRenderTextureFormats::Enum format) override;

    virtual QDemonRenderBackendComputeShaderObject createComputeShader(QDemonConstDataRef<qint8> source,
                                                                       QByteArray &errorMessage,
                                                                       bool binary) override;
    void dispatchCompute(QDemonRenderBackendShaderProgramObject po, quint32 numGroupsX, quint32 numGroupsY, quint32 numGroupsZ) override;

    QDemonRenderBackendProgramPipeline createProgramPipeline() override;
    void releaseProgramPipeline(QDemonRenderBackendProgramPipeline ppo) override;
    void setActiveProgramPipeline(QDemonRenderBackendProgramPipeline ppo) override;
    void setProgramStages(QDemonRenderBackendProgramPipeline ppo,
                          QDemonRenderShaderTypeFlags flags,
                          QDemonRenderBackendShaderProgramObject po) override;

    void setBlendEquation(const QDemonRenderBlendEquationArgument &pBlendEquArg) override;
    void setBlendBarrier(void) override;

    QDemonRenderBackendPathObject createPathNVObject(size_t range) override;
    void setPathSpecification(QDemonRenderBackendPathObject inPathObject,
                              QDemonConstDataRef<quint8> inPathCommands,
                              QDemonConstDataRef<float> inPathCoords) override;
    QDemonBounds3 getPathObjectBoundingBox(QDemonRenderBackendPathObject inPathObject) override;
    QDemonBounds3 getPathObjectFillBox(QDemonRenderBackendPathObject inPathObject) override;
    QDemonBounds3 getPathObjectStrokeBox(QDemonRenderBackendPathObject inPathObject) override;
    void setStrokeWidth(QDemonRenderBackendPathObject inPathObject, float inStrokeWidth) override;

    void setPathProjectionMatrix(const QMatrix4x4 inPathProjection) override;
    void setPathModelViewMatrix(const QMatrix4x4 inPathModelview) override;
    void setPathStencilDepthOffset(float inSlope, float inBias) override;
    void setPathCoverDepthFunc(QDemonRenderBoolOp::Enum inDepthFunction) override;
    void stencilStrokePath(QDemonRenderBackendPathObject inPathObject) override;
    void stencilFillPath(QDemonRenderBackendPathObject inPathObject) override;
    void releasePathNVObject(QDemonRenderBackendPathObject po, size_t range) override;

    void stencilFillPathInstanced(QDemonRenderBackendPathObject po,
                                  size_t numPaths,
                                  QDemonRenderPathFormatType::Enum type,
                                  const void *charCodes,
                                  QDemonRenderPathFillMode::Enum fillMode,
                                  quint32 stencilMask,
                                  QDemonRenderPathTransformType::Enum transformType,
                                  const float *transformValues) override;
    void stencilStrokePathInstancedN(QDemonRenderBackendPathObject po,
                                     size_t numPaths,
                                     QDemonRenderPathFormatType::Enum type,
                                     const void *charCodes,
                                     qint32 stencilRef,
                                     quint32 stencilMask,
                                     QDemonRenderPathTransformType::Enum transformType,
                                     const float *transformValues) override;
    void coverFillPathInstanced(QDemonRenderBackendPathObject po,
                                size_t numPaths,
                                QDemonRenderPathFormatType::Enum type,
                                const void *charCodes,
                                QDemonRenderPathCoverMode::Enum coverMode,
                                QDemonRenderPathTransformType::Enum transformType,
                                const float *transformValues) override;
    void coverStrokePathInstanced(QDemonRenderBackendPathObject po,
                                  size_t numPaths,
                                  QDemonRenderPathFormatType::Enum type,
                                  const void *charCodes,
                                  QDemonRenderPathCoverMode::Enum coverMode,
                                  QDemonRenderPathTransformType::Enum transformType,
                                  const float *transformValues) override;
    void loadPathGlyphs(QDemonRenderBackendPathObject po,
                        QDemonRenderPathFontTarget::Enum fontTarget,
                        const void *fontName,
                        QDemonRenderPathFontStyleFlags fontStyle,
                        size_t numGlyphs,
                        QDemonRenderPathFormatType::Enum type,
                        const void *charCodes,
                        QDemonRenderPathMissingGlyphs::Enum handleMissingGlyphs,
                        QDemonRenderBackendPathObject pathParameterTemplate,
                        float emScale) override;
    virtual QDemonRenderPathReturnValues::Enum loadPathGlyphsIndexed(QDemonRenderBackendPathObject po,
                                                                     QDemonRenderPathFontTarget::Enum fontTarget,
                                                                     const void *fontName,
                                                                     QDemonRenderPathFontStyleFlags fontStyle,
                                                                     quint32 firstGlyphIndex,
                                                                     size_t numGlyphs,
                                                                     QDemonRenderBackendPathObject pathParameterTemplate,
                                                                     float emScale) override;
    virtual QDemonRenderBackendPathObject loadPathGlyphsIndexedRange(QDemonRenderPathFontTarget::Enum fontTarget,
                                                                     const void *fontName,
                                                                     QDemonRenderPathFontStyleFlags fontStyle,
                                                                     QDemonRenderBackendPathObject pathParameterTemplate,
                                                                     float emScale,
                                                                     quint32 *count) override;
    void loadPathGlyphRange(QDemonRenderBackendPathObject po,
                            QDemonRenderPathFontTarget::Enum fontTarget,
                            const void *fontName,
                            QDemonRenderPathFontStyleFlags fontStyle,
                            quint32 firstGlyph,
                            size_t numGlyphs,
                            QDemonRenderPathMissingGlyphs::Enum handleMissingGlyphs,
                            QDemonRenderBackendPathObject pathParameterTemplate,
                            float emScale) override;
    void getPathMetrics(QDemonRenderBackendPathObject po,
                        size_t numPaths,
                        QDemonRenderPathGlyphFontMetricFlags metricQueryMask,
                        QDemonRenderPathFormatType::Enum type,
                        const void *charCodes,
                        size_t stride,
                        float *metrics) override;
    void getPathMetricsRange(QDemonRenderBackendPathObject po,
                             size_t numPaths,
                             QDemonRenderPathGlyphFontMetricFlags metricQueryMask,
                             size_t stride,
                             float *metrics) override;
    void getPathSpacing(QDemonRenderBackendPathObject po,
                        size_t numPaths,
                        QDemonRenderPathListMode::Enum pathListMode,
                        QDemonRenderPathFormatType::Enum type,
                        const void *charCodes,
                        float advanceScale,
                        float kerningScale,
                        QDemonRenderPathTransformType::Enum transformType,
                        float *spacing) override;

private:
#if !defined(QT_OPENGL_ES)
    QOpenGLExtension_NV_path_rendering *m_nvPathRendering;
    QOpenGLExtension_EXT_direct_state_access *m_directStateAccess;
#endif
};

QT_END_NAMESPACE

#endif
