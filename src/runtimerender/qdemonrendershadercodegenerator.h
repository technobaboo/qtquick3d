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
#ifndef QDEMON_RENDER_SHADER_CODE_GENERATOR_H
#define QDEMON_RENDER_SHADER_CODE_GENERATOR_H

#include <QtDemonRender/qdemonrenderbasetypes.h>

QT_BEGIN_NAMESPACE

typedef QPair<QByteArray, QByteArray> TParamPair;
typedef QPair<QByteArray, TParamPair> TConstantBufferParamPair;
typedef QVector<TConstantBufferParamPair> TConstantBufferParamArray;
typedef QHash<QByteArray, QByteArray> TStrTableStrMap;

struct QDemonShaderCodeGeneratorBase
{
    enum Enum {
        Unknown = 0,
        Lighting,
        ViewVector,
        WorldNormal,
        WorldPosition,
        EnvMapReflection,
        UVCoords,
    };
    QSet<quint32> m_codes; // set of enums we have included.
    QSet<QByteArray> m_includes;
    TStrTableStrMap m_uniforms;
    TStrTableStrMap m_constantBuffers;
    TConstantBufferParamArray m_constantBufferParams;
    TStrTableStrMap m_attributes;
    QByteArray m_finalShaderBuilder;
    QByteArray m_codeBuilder;
    QDemonRenderContextType m_renderContextType;

    QDemonShaderCodeGeneratorBase(const QDemonRenderContextType &ctxType);
    virtual ~QDemonShaderCodeGeneratorBase();
    virtual TStrTableStrMap &getVaryings() = 0;
    void begin();
    void append(const QByteArray &data);
    // don't add the newline
    void appendPartial(const QByteArray &data);
    void addConstantBuffer(const QByteArray &name, const QByteArray &layout);
    void addConstantBufferParam(const QByteArray &cbName, const QByteArray &paramName, const QByteArray &type);
    void addUniform(const QByteArray &name, const QByteArray &type);
    void addAttribute(const QByteArray &name, const QByteArray &type);
    void addVarying(const QByteArray &name, const QByteArray &type);
    void addLocalVariable(const QByteArray &name, const QByteArray &type, int tabCount = 1);
    void addInclude(const QByteArray &name);
    bool hasCode(Enum value);
    void setCode(Enum value);
    void setupWorldPosition();
    void generateViewVector();
    void generateWorldNormal();
    void generateEnvMapReflection(QDemonShaderCodeGeneratorBase &inFragmentShader);
    void generateUVCoords();
    void generateTextureSwizzle(QDemonRenderTextureSwizzleMode::Enum swizzleMode, QByteArray &texSwizzle, QByteArray &lookupSwizzle);
    void generateShadedWireframeBase();
    void addLighting();
    QByteArray buildShaderSource();
    QDemonShaderCodeGeneratorBase &operator<<(const QByteArray &data);

protected:
    virtual void addShaderItemMap(const QByteArray &itemType, const TStrTableStrMap &itemMap);
    void addShaderConstantBufferItemMap(const QByteArray &itemType, const TStrTableStrMap &cbMap, TConstantBufferParamArray cbParamsArray);
};

struct QDemonShaderVertexCodeGenerator : public QDemonShaderCodeGeneratorBase
{
    TStrTableStrMap m_varyings;
    QDemonShaderVertexCodeGenerator(const QDemonRenderContextType &ctxType);
    TStrTableStrMap &getVaryings() override;
};

struct QDemonShaderTessControlCodeGenerator : public QDemonShaderCodeGeneratorBase
{
    QDemonShaderVertexCodeGenerator &m_vertGenerator;
    TStrTableStrMap m_varyings;
    QDemonShaderTessControlCodeGenerator(QDemonShaderVertexCodeGenerator &vert, const QDemonRenderContextType &ctxType);

    void addShaderItemMap(const QByteArray &itemType, const TStrTableStrMap &itemMap) override;
    TStrTableStrMap &getVaryings() override;
};

struct QDemonShaderTessEvalCodeGenerator : public QDemonShaderCodeGeneratorBase
{
    QDemonShaderTessControlCodeGenerator &m_tessControlGenerator;
    bool m_hasGeometryStage = true;

    QDemonShaderTessEvalCodeGenerator(QDemonShaderTessControlCodeGenerator &tc, const QDemonRenderContextType &ctxType);

    void addShaderItemMap(const QByteArray &itemType, const TStrTableStrMap &itemMap) override;
    TStrTableStrMap &getVaryings() override;
    virtual void setGeometryStage(bool hasGeometryStage);
};

struct QDemonShaderGeometryCodeGenerator : public QDemonShaderCodeGeneratorBase
{
    QDemonShaderVertexCodeGenerator &m_vertGenerator;
    bool m_hasTessellationStage;

    QDemonShaderGeometryCodeGenerator(QDemonShaderVertexCodeGenerator &vert, const QDemonRenderContextType &ctxType);

    void addShaderItemMap(const QByteArray &itemType, const TStrTableStrMap &itemMap) override;
    TStrTableStrMap &getVaryings() override;
    virtual void setTessellationStage(bool hasTessellationStage);
};

struct QDemonShaderFragmentCodeGenerator : public QDemonShaderCodeGeneratorBase
{
    QDemonShaderVertexCodeGenerator &m_vertGenerator;
    QDemonShaderFragmentCodeGenerator(QDemonShaderVertexCodeGenerator &vert, const QDemonRenderContextType &ctxType);
    TStrTableStrMap &getVaryings() override;
};
QT_END_NAMESPACE

#endif
