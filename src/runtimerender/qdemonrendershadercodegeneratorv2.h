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
#ifndef QDEMON_RENDER_SHADER_CODE_GENERATOR_V2_H
#define QDEMON_RENDER_SHADER_CODE_GENERATOR_V2_H
#include <QtDemonRuntimeRender/qdemonrendershadercodegenerator.h>
#include <QtDemonRuntimeRender/qdemonrendershadercache.h>
#include <QtDemon/qdemonflags.h>

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE
// So far the generator is only useful for graphics stages,
// it doesn't seem useful for compute stages.
struct ShaderGeneratorStages
{
    enum Enum {
        Vertex = 1,
        TessControl = 1 << 1,
        TessEval = 1 << 2,
        Geometry = 1 << 3,
        Fragment = 1 << 4,
        StageCount = 5,
    };
};

typedef QDemonFlags<ShaderGeneratorStages::Enum, quint32> TShaderGeneratorStageFlags;

class IShaderStageGenerator
{
protected:
    virtual ~IShaderStageGenerator() {}
public:
    virtual void AddIncoming(const char *name, const char *type) = 0;
    virtual void AddIncoming(const QString &name, const char *type) = 0;

    virtual void AddOutgoing(const char *name, const char *type) = 0;
    virtual void AddOutgoing(const QString &name, const char *type) = 0;

    virtual void AddUniform(const char *name, const char *type) = 0;
    virtual void AddUniform(const QString &name, const char *type) = 0;

    virtual void AddInclude(const char *name) = 0;
    virtual void AddInclude(const QString &name) = 0;

    virtual void AddFunction(const QString &functionName) = 0;

    virtual void AddConstantBuffer(const char *name, const char *layout) = 0;
    virtual void AddConstantBufferParam(const char *cbName, const char *paramName, const char *type) = 0;

    virtual IShaderStageGenerator &operator<<(const char *data) = 0;
    virtual IShaderStageGenerator &operator<<(const QString &data) = 0;
    virtual void Append(const char *data) = 0;
    virtual void AppendPartial(const char *data) = 0;

    virtual ShaderGeneratorStages::Enum Stage() const = 0;
};

class IQDemonRenderContext;

class IShaderProgramGenerator
{
public:
    virtual ~IShaderProgramGenerator() {}
    static TShaderGeneratorStageFlags DefaultFlags()
    {
        return TShaderGeneratorStageFlags(ShaderGeneratorStages::Vertex
                                          | ShaderGeneratorStages::Fragment);
    }
    virtual void BeginProgram(TShaderGeneratorStageFlags inEnabledStages = DefaultFlags()) = 0;

    virtual TShaderGeneratorStageFlags GetEnabledStages() const = 0;

    // get the stage or nullptr if it has not been created.
    virtual IShaderStageGenerator *GetStage(ShaderGeneratorStages::Enum inStage) = 0;

    // Implicit call to end program.
    virtual QSharedPointer<QDemonRenderShaderProgram>
    CompileGeneratedShader(const char *inShaderName, const SShaderCacheProgramFlags &inFlags,
                           TShaderFeatureSet inFeatureSet, bool separableProgram = false) = 0;

    QSharedPointer<QDemonRenderShaderProgram> CompileGeneratedShader(const char *inShaderName, bool separableProgram = false)
    {
        return CompileGeneratedShader(inShaderName, SShaderCacheProgramFlags(), TShaderFeatureSet(), separableProgram);
    }

    static QSharedPointer<IShaderProgramGenerator> CreateProgramGenerator(QSharedPointer<IQDemonRenderContext> inContext);

    static void OutputParaboloidDepthVertex(QSharedPointer<IShaderStageGenerator> inGenerator);
    // By convention, the local space result of the TE is stored in vec4 pos local variable.
    // This function expects such state.
    static void OutputParaboloidDepthTessEval(QSharedPointer<IShaderStageGenerator> inGenerator);
    // Utilities shared among the various different systems.
    static void OutputParaboloidDepthFragment(QSharedPointer<IShaderStageGenerator> inGenerator);

    static void OutputCubeFaceDepthVertex(QSharedPointer<IShaderStageGenerator> inGenerator);
    static void OutputCubeFaceDepthGeometry(QSharedPointer<IShaderStageGenerator> inGenerator);
    static void OutputCubeFaceDepthFragment(QSharedPointer<IShaderStageGenerator> inGenerator);
};
QT_END_NAMESPACE
#endif
