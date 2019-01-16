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

struct SShaderCodeGeneratorBase
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
    QSet<quint32> m_Codes; // set of enums we have included.
    QSet<QString> m_Includes;
    TStrTableStrMap m_Uniforms;
    TStrTableStrMap m_ConstantBuffers;
    TConstantBufferParamArray m_ConstantBufferParams;
    TStrTableStrMap m_Attributes;
    QString m_FinalShaderBuilder;
    QString m_CodeBuilder;
    QDemonRenderContextType m_RenderContextType;

    SShaderCodeGeneratorBase(QDemonRenderContextType ctxType);
    virtual ~SShaderCodeGeneratorBase();
    virtual TStrTableStrMap &GetVaryings() = 0;
    void Begin();
    void Append(const QString &data);
    // don't add the newline
    void AppendPartial(const QString &data);
    void AddConstantBuffer(const QString &name, const QString &layout);
    void AddConstantBufferParam(const QString &cbName, const QString &paramName, const QString &type);
    void AddUniform(const QString &name, const QString &type);
    void AddAttribute(const QString &name, const QString &type);
    void AddVarying(const QString &name, const QString &type);
    void AddLocalVariable(const QString &name, const QString &type, int tabCount = 1);
    void AddInclude(const QString &name);
    bool HasCode(Enum value);
    void SetCode(Enum value);
    void SetupWorldPosition();
    void GenerateViewVector();
    void GenerateWorldNormal();
    void GenerateEnvMapReflection(SShaderCodeGeneratorBase &inFragmentShader);
    void GenerateUVCoords();
    void GenerateTextureSwizzle(QDemonRenderTextureSwizzleMode::Enum swizzleMode,
                                QByteArray &texSwizzle,
                                QByteArray &lookupSwizzle);
    void GenerateShadedWireframeBase();
    void AddLighting();
    const QString &BuildShaderSource();
    SShaderCodeGeneratorBase &operator<<(const QString &data);

protected:
    virtual void AddShaderItemMap(const QString &itemType, const TStrTableStrMap &itemMap);
    void AddShaderConstantBufferItemMap(const QString &itemType, const TStrTableStrMap &cbMap,
                                        TConstantBufferParamArray cbParamsArray);
};

struct SShaderVertexCodeGenerator : public SShaderCodeGeneratorBase
{
    TStrTableStrMap m_Varyings;
    SShaderVertexCodeGenerator(QDemonRenderContextType ctxType);
    TStrTableStrMap &GetVaryings() override;
};

struct SShaderTessControlCodeGenerator : public SShaderCodeGeneratorBase
{
    SShaderVertexCodeGenerator &m_VertGenerator;
    TStrTableStrMap m_Varyings;
    SShaderTessControlCodeGenerator(SShaderVertexCodeGenerator &vert,
                                    QDemonRenderContextType ctxType);

    void AddShaderItemMap(const QString &itemType, const TStrTableStrMap &itemMap) override;
    TStrTableStrMap &GetVaryings() override;
};

struct SShaderTessEvalCodeGenerator : public SShaderCodeGeneratorBase
{
    SShaderTessControlCodeGenerator &m_TessControlGenerator;
    bool m_hasGeometryStage;

    SShaderTessEvalCodeGenerator(SShaderTessControlCodeGenerator &tc,
                                 QDemonRenderContextType ctxType);

    void AddShaderItemMap(const QString &itemType, const TStrTableStrMap &itemMap) override;
    TStrTableStrMap &GetVaryings() override;
    virtual void SetGeometryStage(bool hasGeometryStage);
};

struct SShaderGeometryCodeGenerator : public SShaderCodeGeneratorBase
{
    SShaderVertexCodeGenerator &m_VertGenerator;
    bool m_hasTessellationStage;

    SShaderGeometryCodeGenerator(SShaderVertexCodeGenerator &vert,
                                 QDemonRenderContextType ctxType);

    void AddShaderItemMap(const QString &itemType, const TStrTableStrMap &itemMap) override;
    TStrTableStrMap &GetVaryings() override;
    virtual void SetTessellationStage(bool hasTessellationStage);
};

struct SShaderFragmentCodeGenerator : public SShaderCodeGeneratorBase
{
    SShaderVertexCodeGenerator &m_VertGenerator;
    SShaderFragmentCodeGenerator(SShaderVertexCodeGenerator &vert,
                                 QDemonRenderContextType ctxType);
    TStrTableStrMap &GetVaryings() override;
};
QT_END_NAMESPACE

#endif
