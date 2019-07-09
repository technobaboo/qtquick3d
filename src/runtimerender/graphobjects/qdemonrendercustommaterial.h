/****************************************************************************
**
** Copyright (C) 2008-2012 NVIDIA Corporation.
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Quick 3D.
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

#ifndef QDEMON_RENDER_CUSTOM_MATERIAL_H
#define QDEMON_RENDER_CUSTOM_MATERIAL_H

#include <QtDemonRuntimeRender/qdemonrenderimage.h>
#include <QtDemonRuntimeRender/qdemonrenderlightmaps.h>

#include <QtCore/qurl.h>
#include <QtCore/qvector.h>
#include <QtDemonRuntimeRender/qdemonrenderdynamicobjectsystemcommands.h>

QT_BEGIN_NAMESPACE

struct Q_DEMONRUNTIMERENDER_EXPORT QDemonRenderCustomMaterial : public QDemonRenderGraphObject
{
    QDemonRenderCustomMaterial() : QDemonRenderGraphObject(Type::CustomMaterial) {}

    struct TextureProperty
    {
        QDemonRenderImage *texImage = nullptr;
        QByteArray name;
        QDemonRenderShaderDataType shaderDataType;
        // TODO: Note needed?
        QDemonRenderTextureMagnifyingOp magFilterType = QDemonRenderTextureMagnifyingOp::Linear;
        QDemonRenderTextureMinifyingOp minFilterType = QDemonRenderTextureMinifyingOp::Linear;
        QDemonRenderTextureCoordOp clampType = QDemonRenderTextureCoordOp::ClampToEdge;
        QDemonRenderTextureTypeValue usageType;
    };

    QVector<TextureProperty> textureProperties;

    struct Property
    {
        QByteArray name;
        mutable QVariant value;
        QDemonRenderShaderDataType shaderDataType;
        int pid = -1;
    };

    QVector<Property> properties;

    struct ShaderInfo
    {
        QByteArray version;
        QByteArray type; // I.e., GLSL
        QByteArray shaderPrefix;
    };

    ShaderInfo shaderInfo;

    struct Shader
    {
        enum class Stage : quint8
        {
            Shared,
            Vertex,
            Fragment
        };

        QString code;
        Stage stage;
    };

    QVector<Shader> shaders;

    struct Pass
    {
        struct BufferInput
        {
            QString bufferName;
            QString shaderParam;
            // dynamic::QDemonApplyBufferValue(bufferName, shaderParam)
        };

        struct BufferBlit
        {
            QString source;
            QString dest;
            // dynamic::QDemonApplyBlitFramebuffer(source, dest)
        };

        struct Blending
        {
            QDemonRenderSrcBlendFunc source = QDemonRenderSrcBlendFunc::One;
            QDemonRenderDstBlendFunc dest = QDemonRenderDstBlendFunc::One;
            // hasBlending = true; when used
            // dynamic::QDemonApplyBlending(source, dest)
        };

        struct RenderState
        {
            QDemonRenderState renderState = QDemonRenderState::Unknown;
            bool enabled;
            // dynamic::QDemonApplyRenderState(renderState, enabled)
        };

        QString shaderName;
        QString input;
        QString output;
        QDemonRenderTextureFormat::Format outputFormat;
        bool needsClear;

        Pass()
            : input(QLatin1String("[source]"))
            , output(QLatin1String("[dest]"))
            , outputFormat(QDemonRenderTextureFormat::RGBA8)
            , needsClear(false)
        {}
    };

    QVector<Pass> passes;
    QVector<dynamic::QDemonCommand *> commands;

    // IMPORTANT: These flags matches the key produced by a MDL export file
    enum class MaterialShaderKeyValues
    {
        diffuse = 1 << 0,
        specular = 1 << 1,
        glossy = 1 << 2,
        cutout = 1 << 3,
        refraction = 1 << 4,
        transparent = 1 << 5,
        displace = 1 << 6,
        volumetric = 1 << 7,
        transmissive = 1 << 8,
    };
    Q_DECLARE_FLAGS(MaterialShaderKeyFlags, MaterialShaderKeyValues)

    using Flag = QDemonRenderNode::Flag;
    Q_DECLARE_FLAGS(Flags, Flag)

    const char *className = nullptr;

    // lightmap section
    QDemonRenderLightmaps m_lightmaps;
    // material section
    bool m_hasTransparency = false;
    bool m_hasRefraction = false;
    bool m_hasVolumetricDF = false;
    QDemonRenderImage *m_iblProbe = nullptr;
    QDemonRenderImage *m_emissiveMap2 = nullptr;
    QDemonRenderImage *m_displacementMap = nullptr;
    float m_displaceAmount = 0.0f; ///< depends on the object size

    QDemonRenderGraphObject *m_nextSibling = nullptr;

    MaterialShaderKeyFlags m_shaderKeyValues; ///< input from MDL files
    qint32 m_layerCount = 0; ///< input from MDL files

    Flags flags;
    bool m_alwaysDirty = false;

    bool isDielectric() const { return m_shaderKeyValues & MaterialShaderKeyValues::diffuse; }
    bool isSpecularEnabled() const { return m_shaderKeyValues & MaterialShaderKeyValues::specular; }
    bool isCutOutEnabled() const { return m_shaderKeyValues & MaterialShaderKeyValues::cutout; }
    bool isVolumetric() const { return m_shaderKeyValues & MaterialShaderKeyValues::volumetric; }
    bool isTransmissive() const { return m_shaderKeyValues & MaterialShaderKeyValues::transmissive; }
    bool hasLighting() const { return true; }

    // Dirty
    bool m_dirtyFlagWithInFrame;
    bool isDirty() const { return flags.testFlag(Flag::Dirty) || m_dirtyFlagWithInFrame || m_alwaysDirty; }
    void updateDirtyForFrame()
    {
        m_dirtyFlagWithInFrame = flags.testFlag(Flag::Dirty);
        flags.setFlag(Flag::Dirty, false);
    }
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDemonRenderCustomMaterial::MaterialShaderKeyFlags)

QT_END_NAMESPACE

#endif
