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

#include <QtDemonRuntimeRender/qdemonrenderwidgets.h>
#include <QtDemonRuntimeRender/qdemonrendernode.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRuntimeRender/qdemonrendershadercodegeneratorv2.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>

QT_BEGIN_NAMESPACE

namespace {

struct SWidgetBBox : public IRenderWidget
{
    QDemonBounds3 m_Bounds;
    QVector3D m_Color;
    QDemonRenderVertexBuffer *m_BoxVertexBuffer;
    QDemonRenderIndexBuffer *m_BoxIndexBuffer;
    QDemonRenderInputAssembler *m_BoxInputAssembler;
    QDemonRenderShaderProgram *m_BoxShader;
    QString m_ItemName;
    SWidgetBBox(SNode &inNode, const QDemonBounds3 &inBounds, const QVector3D &inColor)
        : IRenderWidget(inNode)
        , m_Bounds(inBounds)
        , m_Color(inColor)
        , m_BoxVertexBuffer(nullptr)
        , m_BoxIndexBuffer(nullptr)
        , m_BoxInputAssembler(nullptr)
        , m_BoxShader(nullptr)
    {
    }

    void SetupBoxShader(IRenderWidgetContext &inContext)
    {
        m_BoxShader = inContext.GetShader(m_ItemName);
        if (!m_BoxShader) {
            IShaderProgramGenerator &theGenerator(inContext.GetProgramGenerator());
            theGenerator.BeginProgram();
            IShaderStageGenerator &theVertexGenerator(
                        *theGenerator.GetStage(ShaderGeneratorStages::Vertex));
            IShaderStageGenerator &theFragmentGenerator(
                        *theGenerator.GetStage(ShaderGeneratorStages::Fragment));

            theVertexGenerator.AddIncoming("attr_pos", "vec3");
            theVertexGenerator.AddUniform("model_view_projection", "mat4");
            theVertexGenerator.Append("void main() {");
            theVertexGenerator.Append(
                        "\tgl_Position = model_view_projection * vec4(attr_pos, 1.0);");
            theVertexGenerator.Append("}");
            theFragmentGenerator.AddUniform("output_color", "vec3");
            theFragmentGenerator.Append("void main() {");
            theFragmentGenerator.Append("\tgl_FragColor.rgb = output_color;");
            theFragmentGenerator.Append("\tgl_FragColor.a = 1.0;");
            theFragmentGenerator.Append("}");
            m_BoxShader = inContext.CompileAndStoreShader(m_ItemName);
        }
    }

    void SetupBoundingBoxGraphicsObjects(IRenderWidgetContext &inContext,
                                         QDemonDataRef<QVector3D> thePoints)
    {
        QDemonRenderVertexBufferEntry theEntry(
                    "attr_pos", QDemonRenderComponentTypes::float, 3);
        m_BoxVertexBuffer = &inContext.GetOrCreateVertexBuffer(
                    m_ItemName, 3 * sizeof(float), toU8DataRef(thePoints.begin(), thePoints.size()));
        m_BoxIndexBuffer = inContext.GetIndexBuffer(m_ItemName);
        if (!m_BoxIndexBuffer) {
            // The way the bounds lays out the bounds for the box
            // capitalization indicates whether this was a max or min value.
            enum _Indexes {
                xyz = 0,
                Xyz,
                xYz,
                xyZ,
                XYZ,
                xYZ,
                XyZ,
                XYz,
            };
            quint8 indexes[] = {
                // The toBoxBounds function lays out points such that
                // xyz, Xyz, xYz, xyZ, XYZ, xYZ, XyZ, XYz
                // Min corner
                xyz, Xyz, xyz, xYz, xyz, xyZ,

                // Max corner
                XYZ, xYZ, XYZ, XyZ, XYZ, XYz,

                // Now connect the rest of the dots.
                // the rules are that only one letter can change
                // else you are connecting *across* the box somehow.

                Xyz, XYz, Xyz, XyZ,

                xYz, XYz, xYz, xYZ,

                xyZ, XyZ, xyZ, xYZ,
            };
            m_BoxIndexBuffer = &inContext.GetOrCreateIndexBuffer(
                        m_ItemName, QDemonRenderComponentTypes::quint8, sizeof(indexes),
                        toU8DataRef(indexes, sizeof(indexes)));
        }

        m_BoxInputAssembler = inContext.GetInputAssembler(m_ItemName);
        if (!m_BoxInputAssembler && m_BoxIndexBuffer && m_BoxVertexBuffer) {
            // create our attribute layout
            QDemonRenderAttribLayout *theAttribLAyout =
                    &inContext.CreateAttributeLayout(toConstDataRef(&theEntry, 1));

            quint32 strides = m_BoxVertexBuffer->GetStride();
            quint32 offsets = 0;
            m_BoxInputAssembler = &inContext.GetOrCreateInputAssembler(
                        m_ItemName, theAttribLAyout, toConstDataRef(&m_BoxVertexBuffer, 1),
                        m_BoxIndexBuffer, toConstDataRef(&strides, 1), toConstDataRef(&offsets, 1));
        }
        SetupBoxShader(inContext);
    }

    void Render(IRenderWidgetContext &inWidgetContext, QDemonRenderContext &inRenderContext) override
    {
        m_ItemName = QString::fromLocal8Bit("SWidgetBBox");
        SWidgetRenderInformation theInfo(inWidgetContext.GetWidgetRenderInformation(
                                             *m_Node, m_Node->m_Position, RenderWidgetModes::Local));
        TNVBounds2BoxPoints thePoints;
        m_Bounds.expand(thePoints);
        QMatrix4x4 theNodeRotation;
        QMatrix4x4 theNodeToCamera = theInfo.m_NodeParentToCamera * m_Node->m_LocalTransform;
        for (quint32 idx = 0; idx < 8; ++idx)
            thePoints[idx] = theNodeToCamera.transform(thePoints[idx]);
        SetupBoundingBoxGraphicsObjects(inWidgetContext, toDataRef(thePoints, 8));
        if (m_BoxShader && m_BoxInputAssembler) {
            inRenderContext.SetBlendingEnabled(false);
            inRenderContext.SetDepthWriteEnabled(true);
            inRenderContext.SetDepthTestEnabled(true);
            inRenderContext.SetCullingEnabled(false);
            inRenderContext.SetActiveShader(m_BoxShader);
            m_BoxShader->SetPropertyValue("model_view_projection", theInfo.m_LayerProjection);
            m_BoxShader->SetPropertyValue("output_color", m_Color);
            inRenderContext.SetInputAssembler(m_BoxInputAssembler);
            inRenderContext.Draw(QDemonRenderDrawMode::Lines,
                                 m_BoxInputAssembler->GetIndexCount(), 0);
        }
    }
};

struct SWidgetAxis : public IRenderWidget
{
    QDemonRenderVertexBuffer *m_AxisVertexBuffer;
    QDemonRenderInputAssembler *m_AxisInputAssembler;
    QDemonRenderShaderProgram *m_AxisShader;
    QString m_ItemName;

    SWidgetAxis(SNode &inNode)
        : IRenderWidget(inNode)
        , m_AxisVertexBuffer(nullptr)
        , m_AxisInputAssembler(nullptr)
        , m_AxisShader(nullptr)
    {
    }

    void SetupAxisShader(IRenderWidgetContext &inContext)
    {
        m_AxisShader = inContext.GetShader(m_ItemName);
        if (!m_AxisShader) {
            IShaderProgramGenerator &theGenerator(inContext.GetProgramGenerator());
            theGenerator.BeginProgram();
            IShaderStageGenerator &theVertexGenerator(
                        *theGenerator.GetStage(ShaderGeneratorStages::Vertex));
            IShaderStageGenerator &theFragmentGenerator(
                        *theGenerator.GetStage(ShaderGeneratorStages::Fragment));
            theVertexGenerator.AddIncoming("attr_pos", "vec3");
            theVertexGenerator.AddIncoming("attr_color", "vec3");
            theVertexGenerator.AddOutgoing("output_color", "vec3");
            theVertexGenerator.AddUniform("model_view_projection", "mat4");
            theVertexGenerator.Append("void main() {");
            theVertexGenerator.Append(
                        "\tgl_Position = model_view_projection * vec4(attr_pos, 1.0);");
            theVertexGenerator.Append("\toutput_color = attr_color;");
            theVertexGenerator.Append("}");
            theFragmentGenerator.Append("void main() {");
            theFragmentGenerator.Append("\tgl_FragColor.rgb = output_color;");
            theFragmentGenerator.Append("\tgl_FragColor.a = 1.0;");
            theFragmentGenerator.Append("}");
            m_AxisShader = inContext.CompileAndStoreShader(m_ItemName);
        }
    }

    void SetupAxesGraphicsObjects(IRenderWidgetContext &inContext, QDemonDataRef<QVector3D> theAxes)
    {
        QDemonRenderVertexBufferEntry theEntries[] = {
            QDemonRenderVertexBufferEntry("attr_pos",
            QDemonRenderComponentTypes::float, 3),
            QDemonRenderVertexBufferEntry("attr_color",
            QDemonRenderComponentTypes::float, 3, 12),
        };

        m_AxisVertexBuffer = &inContext.GetOrCreateVertexBuffer(
                    m_ItemName, 6 * sizeof(float), toU8DataRef(theAxes.begin(), theAxes.size()));

        if (!m_AxisInputAssembler && m_AxisVertexBuffer) {
            // create our attribute layout
            QDemonRenderAttribLayout *theAttribLAyout =
                    &inContext.CreateAttributeLayout(toConstDataRef(theEntries, 2));

            quint32 strides = m_AxisVertexBuffer->GetStride();
            quint32 offsets = 0;
            m_AxisInputAssembler = &inContext.GetOrCreateInputAssembler(
                        m_ItemName, theAttribLAyout, toConstDataRef(&m_AxisVertexBuffer, 1), nullptr,
                        toConstDataRef(&strides, 1), toConstDataRef(&offsets, 1));
        }
    }

    inline QVector3D TransformDirection(const QMatrix3x3 &inMatrix, const QVector3D &inDir)
    {
        QVector3D retval = inMatrix.transform(inDir);
        retval.normalize();
        return retval;
    }
    void Render(IRenderWidgetContext &inWidgetContext, QDemonRenderContext &inRenderContext) override
    {
        m_ItemName = QString::fromLocal8Bit("SWidgetAxis");

        SetupAxisShader(inWidgetContext);

        if (m_AxisShader) {
            QVector3D Red = QVector3D(1, 0, 0);
            QVector3D Green = QVector3D(0, 1, 0);
            QVector3D Blue = QVector3D(0, 0, 1);
            if (m_Node->m_Parent && m_Node->m_Parent->m_Type != GraphObjectTypes::Layer) {
                m_Node->m_Parent->CalculateGlobalVariables();
            }
            QVector3D thePivot(m_Node->m_Pivot);
            if (m_Node->m_Flags.IsLeftHanded())
                thePivot.z *= -1;
            SWidgetRenderInformation theInfo(inWidgetContext.GetWidgetRenderInformation(
                                                 *m_Node, thePivot, RenderWidgetModes::Local));

            QMatrix4x4 theNodeRotation;
            m_Node->CalculateRotationMatrix(theNodeRotation);
            if (m_Node->m_Flags.IsLeftHanded())
                SNode::FlipCoordinateSystem(theNodeRotation);
            QMatrix3x3 theRotationMatrix(theNodeRotation.column0.getXYZ(),
                                         theNodeRotation.column1.getXYZ(),
                                         theNodeRotation.column2.getXYZ());
            // Move the camera position into camera space.  This is so that when we render we don't
            // have to account
            // for scaling done in the camera's MVP.
            QVector3D theItemPosition = theInfo.m_Position;
            QMatrix3x3 theAxisTransform = theInfo.m_NormalMatrix * theRotationMatrix;
            QVector3D xAxis = TransformDirection(theAxisTransform, QVector3D(1, 0, 0));
            QVector3D yAxis = TransformDirection(theAxisTransform, QVector3D(0, 1, 0));
            QVector3D zAxis = TransformDirection(theAxisTransform, QVector3D(0, 0, -1));

            // This world to pixel scale factor function
            float theScaleFactor = theInfo.m_Scale;
            theItemPosition = theInfo.m_Position;

            float overshootFactor = 10.0f * theScaleFactor; // amount to scale past the origin in
            // the opposite direction as the axis
            float scaleFactor = 50.0f * theScaleFactor;
            QVector3D theAxis[] = {
                theItemPosition - (xAxis * overshootFactor), Red,
                theItemPosition + (xAxis * scaleFactor),     Red, // X axis
                theItemPosition - (yAxis * overshootFactor), Green,
                theItemPosition + (yAxis * scaleFactor),     Green, // Y axis
                theItemPosition - (zAxis * overshootFactor), Blue,
                theItemPosition + (zAxis * scaleFactor),     Blue, // Z axis
            };

            SetupAxesGraphicsObjects(inWidgetContext, toDataRef(theAxis, 3));

            if (m_AxisInputAssembler) {
                inRenderContext.SetBlendingEnabled(false);
                inRenderContext.SetDepthWriteEnabled(false);
                inRenderContext.SetDepthTestEnabled(false);
                inRenderContext.SetCullingEnabled(false);
                inRenderContext.SetActiveShader(m_AxisShader);
                m_AxisShader->SetPropertyValue("model_view_projection", theInfo.m_LayerProjection);
                inRenderContext.SetInputAssembler(m_AxisInputAssembler);
                // Draw six points.
                inRenderContext.Draw(QDemonRenderDrawMode::Lines, 6, 0);
            }
        }
    }
};
}

IRenderWidget &IRenderWidget::CreateBoundingBoxWidget(SNode &inNode, const QDemonBounds3 &inBounds,
                                                      const QVector3D &inColor)
{
    return *new SWidgetBBox(inNode, inBounds, inColor);
}

IRenderWidget &IRenderWidget::CreateAxisWidget(SNode &inNode)
{
    return *new SWidgetAxis(inNode);
}

QT_END_NAMESPACE
