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

#include <QtDemonRuntimeRender/qtdemonruntimerenderglobal.h>
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

class Q_DEMONRUNTIMERENDER_EXPORT QDemonShaderStageGeneratorInterface
{
protected:
    virtual ~QDemonShaderStageGeneratorInterface();
public:
    virtual void addIncoming(const QByteArray &name, const QByteArray &type) = 0;

    virtual void addOutgoing(const QByteArray &name, const QByteArray &type) = 0;

    virtual void addUniform(const QByteArray &name, const QByteArray &type) = 0;

    virtual void addInclude(const QByteArray &name) = 0;

    virtual void addFunction(const QByteArray &functionName) = 0;

    virtual void addConstantBuffer(const QByteArray &name, const QByteArray &layout) = 0;
    virtual void addConstantBufferParam(const QByteArray &cbName, const QByteArray &paramName, const QByteArray &type) = 0;

    virtual QDemonShaderStageGeneratorInterface &operator<<(const QByteArray &data) = 0;
    virtual void append(const QByteArray &data) = 0;
    virtual void appendPartial(const QByteArray &data) = 0;

    virtual ShaderGeneratorStages::Enum stage() const = 0;
};

class QDemonRenderContextInterface;

class Q_DEMONRUNTIMERENDER_EXPORT QDemonShaderProgramGeneratorInterface
{
public:
    QAtomicInt ref;

    virtual ~QDemonShaderProgramGeneratorInterface() {}
    static TShaderGeneratorStageFlags defaultFlags()
    {
        return TShaderGeneratorStageFlags(ShaderGeneratorStages::Vertex
                                          | ShaderGeneratorStages::Fragment);
    }
    virtual void beginProgram(TShaderGeneratorStageFlags inEnabledStages = defaultFlags()) = 0;

    virtual TShaderGeneratorStageFlags getEnabledStages() const = 0;

    // get the stage or nullptr if it has not been created.
    virtual QDemonShaderStageGeneratorInterface *getStage(ShaderGeneratorStages::Enum inStage) = 0;

    // Implicit call to end program.

    virtual QDemonRef<QDemonRenderShaderProgram> compileGeneratedShader(const QString &inShaderName,
                                                                             const QDemonShaderCacheProgramFlags &inFlags,
                                                                             TShaderFeatureSet inFeatureSet,
                                                                             bool separableProgram = false) = 0;

    QDemonRef<QDemonRenderShaderProgram> compileGeneratedShader(const QString &inShaderName, bool separableProgram = false);

    static QDemonRef<QDemonShaderProgramGeneratorInterface> createProgramGenerator(QDemonRenderContextInterface *inContext);

    static void outputParaboloidDepthVertex(QDemonShaderStageGeneratorInterface &inGenerator);
    // By convention, the local space result of the TE is stored in vec4 pos local variable.
    // This function expects such state.
    static void outputParaboloidDepthTessEval(QDemonShaderStageGeneratorInterface &inGenerator);
    // Utilities shared among the various different systems.
    static void outputParaboloidDepthFragment(QDemonShaderStageGeneratorInterface &inGenerator);

    static void outputCubeFaceDepthVertex(QDemonShaderStageGeneratorInterface &inGenerator);
    static void outputCubeFaceDepthGeometry(QDemonShaderStageGeneratorInterface &inGenerator);
    static void outputCubeFaceDepthFragment(QDemonShaderStageGeneratorInterface &inGenerator);
};
QT_END_NAMESPACE
#endif
