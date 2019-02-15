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

typedef QPair<QString, QString> TParamPair;
typedef QPair<QString, TParamPair> TConstantBufferParamPair;
typedef QVector<TConstantBufferParamPair> TConstantBufferParamArray;
typedef QHash<QString, QString> TStrTableStrMap;

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
    QSet<QString> m_includes;
    TStrTableStrMap m_uniforms;
    TStrTableStrMap m_constantBuffers;
    TConstantBufferParamArray m_constantBufferParams;
    TStrTableStrMap m_attributes;
    QString m_finalShaderBuilder;
    QString m_codeBuilder;
    QDemonRenderContextType m_renderContextType;

    QDemonShaderCodeGeneratorBase(QDemonRenderContextType ctxType);
    virtual ~QDemonShaderCodeGeneratorBase();
    virtual TStrTableStrMap &getVaryings() = 0;
    void begin();
    void append(const QString &data);
    // don't add the newline
    void appendPartial(const QString &data);
    void addConstantBuffer(const QString &name, const QString &layout);
    void addConstantBufferParam(const QString &cbName, const QString &paramName, const QString &type);
    void addUniform(const QString &name, const QString &type);
    void addAttribute(const QString &name, const QString &type);
    void addVarying(const QString &name, const QString &type);
    void addLocalVariable(const QString &name, const QString &type, int tabCount = 1);
    void addInclude(const QString &name);
    bool hasCode(Enum value);
    void setCode(Enum value);
    void setupWorldPosition();
    void generateViewVector();
    void generateWorldNormal();
    void generateEnvMapReflection(QDemonShaderCodeGeneratorBase &inFragmentShader);
    void generateUVCoords();
    void generateTextureSwizzle(QDemonRenderTextureSwizzleMode::Enum swizzleMode,
                                QByteArray &texSwizzle,
                                QByteArray &lookupSwizzle);
    void generateShadedWireframeBase();
    void addLighting();
    const QString &buildShaderSource();
    QDemonShaderCodeGeneratorBase &operator<<(const QString &data);

protected:
    virtual void addShaderItemMap(const QString &itemType, const TStrTableStrMap &itemMap);
    void addShaderConstantBufferItemMap(const QString &itemType,
                                        const TStrTableStrMap &cbMap,
                                        TConstantBufferParamArray cbParamsArray);
};

struct QDemonShaderVertexCodeGenerator : public QDemonShaderCodeGeneratorBase
{
    TStrTableStrMap m_varyings;
    QDemonShaderVertexCodeGenerator(QDemonRenderContextType ctxType);
    TStrTableStrMap &getVaryings() override;
};

struct QDemonShaderTessControlCodeGenerator : public QDemonShaderCodeGeneratorBase
{
    QDemonShaderVertexCodeGenerator &m_vertGenerator;
    TStrTableStrMap m_varyings;
    QDemonShaderTessControlCodeGenerator(QDemonShaderVertexCodeGenerator &vert,
                                         QDemonRenderContextType ctxType);

    void addShaderItemMap(const QString &itemType, const TStrTableStrMap &itemMap) override;
    TStrTableStrMap &getVaryings() override;
};

struct QDemonShaderTessEvalCodeGenerator : public QDemonShaderCodeGeneratorBase
{
    QDemonShaderTessControlCodeGenerator &m_tessControlGenerator;
    bool m_hasGeometryStage;

    QDemonShaderTessEvalCodeGenerator(QDemonShaderTessControlCodeGenerator &tc,
                                      QDemonRenderContextType ctxType);

    void addShaderItemMap(const QString &itemType, const TStrTableStrMap &itemMap) override;
    TStrTableStrMap &getVaryings() override;
    virtual void setGeometryStage(bool hasGeometryStage);
};

struct QDemonShaderGeometryCodeGenerator : public QDemonShaderCodeGeneratorBase
{
    QDemonShaderVertexCodeGenerator &m_vertGenerator;
    bool m_hasTessellationStage;

    QDemonShaderGeometryCodeGenerator(QDemonShaderVertexCodeGenerator &vert,
                                      QDemonRenderContextType ctxType);

    void addShaderItemMap(const QString &itemType, const TStrTableStrMap &itemMap) override;
    TStrTableStrMap &getVaryings() override;
    virtual void setTessellationStage(bool hasTessellationStage);
};

struct QDemonShaderFragmentCodeGenerator : public QDemonShaderCodeGeneratorBase
{
    QDemonShaderVertexCodeGenerator &m_vertGenerator;
    QDemonShaderFragmentCodeGenerator(QDemonShaderVertexCodeGenerator &vert,
                                      QDemonRenderContextType ctxType);
    TStrTableStrMap &getVaryings() override;
};
QT_END_NAMESPACE

#endif
