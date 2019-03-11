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

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRuntimeRender/qdemonrenderwidgets.h>
#include <QtDemonRuntimeRender/qdemonrendernode.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRuntimeRender/qdemonrendershadercodegeneratorv2.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>
#include <QtDemonRuntimeRender/qdemonrendererimpl.h>
#include <QtDemon/qdemonutils.h>
#include <QtDemon/qdemondataref.h>

QT_BEGIN_NAMESPACE

namespace {

struct QDemonWidgetBBox : public QDemonRenderWidgetInterface
{
    QDemonBounds3 m_bounds;
    QVector3D m_color;
    QDemonRef<QDemonRenderVertexBuffer> m_boxVertexBuffer;
    QDemonRef<QDemonRenderIndexBuffer> m_boxIndexBuffer;
    QDemonRef<QDemonRenderInputAssembler> m_boxInputAssembler;
    QDemonRef<QDemonRenderShaderProgram> m_boxShader;
    QByteArray m_itemName;
    QDemonWidgetBBox(QDemonGraphNode &inNode, const QDemonBounds3 &inBounds, const QVector3D &inColor)
        : QDemonRenderWidgetInterface(inNode), m_bounds(inBounds), m_color(inColor)
    {
    }

    void setupBoxShader(QDemonRendererImpl &inContext)
    {
        m_boxShader = inContext.getShader(m_itemName);
        if (!m_boxShader) {
            QDemonRef<QDemonShaderProgramGeneratorInterface> theGenerator(inContext.getProgramGenerator());
            theGenerator->beginProgram();
            QDemonShaderStageGeneratorInterface &theVertexGenerator(*theGenerator->getStage(QDemonShaderGeneratorStage::Vertex));
            QDemonShaderStageGeneratorInterface &theFragmentGenerator(*theGenerator->getStage(QDemonShaderGeneratorStage::Fragment));

            theVertexGenerator.addIncoming("attr_pos", "vec3");
            theVertexGenerator.addUniform("model_view_projection", "mat4");
            theVertexGenerator.append("void main() {");
            theVertexGenerator.append("\tgl_Position = model_view_projection * vec4(attr_pos, 1.0);");
            theVertexGenerator.append("}");
            theFragmentGenerator.addUniform("output_color", "vec3");
            theFragmentGenerator.append("void main() {");
            theFragmentGenerator.append("\tgl_FragColor.rgb = output_color;");
            theFragmentGenerator.append("\tgl_FragColor.a = 1.0;");
            theFragmentGenerator.append("}");
            m_boxShader = inContext.compileAndStoreShader(m_itemName);
        }
    }

    void setupBoundingBoxGraphicsObjects(QDemonRendererImpl &inContext, QDemonDataRef<QVector3D> thePoints)
    {
        QDemonRenderVertexBufferEntry theEntry("attr_pos", QDemonRenderComponentType::Float16, 3);
        m_boxVertexBuffer = inContext.getOrCreateVertexBuffer(m_itemName,
                                                              3 * sizeof(float),
                                                              toU8DataRef(thePoints.begin(), thePoints.size()));
        m_boxIndexBuffer = inContext.getIndexBuffer(m_itemName);
        if (!m_boxIndexBuffer) {
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
                xyz,
                Xyz,
                xyz,
                xYz,
                xyz,
                xyZ,

                // Max corner
                XYZ,
                xYZ,
                XYZ,
                XyZ,
                XYZ,
                XYz,

                // Now connect the rest of the dots.
                // the rules are that only one letter can change
                // else you are connecting *across* the box somehow.

                Xyz,
                XYz,
                Xyz,
                XyZ,

                xYz,
                XYz,
                xYz,
                xYZ,

                xyZ,
                XyZ,
                xyZ,
                xYZ,
            };
            m_boxIndexBuffer = inContext.getOrCreateIndexBuffer(m_itemName,
                                                                QDemonRenderComponentType::UnsignedInteger8,
                                                                sizeof(indexes),
                                                                toU8DataRef(indexes, sizeof(indexes)));
        }

        m_boxInputAssembler = inContext.getInputAssembler(m_itemName);
        if (!m_boxInputAssembler && m_boxIndexBuffer && m_boxVertexBuffer) {
            // create our attribute layout
            QDemonRef<QDemonRenderAttribLayout> theAttribLayout = inContext.createAttributeLayout(toConstDataRef(&theEntry, 1));

            quint32 strides = m_boxVertexBuffer->stride();
            quint32 offsets = 0;
            m_boxInputAssembler = (inContext.getOrCreateInputAssembler(m_itemName,
                                                                       theAttribLayout,
                                                                       toConstDataRef(&m_boxVertexBuffer, 1),
                                                                       m_boxIndexBuffer,
                                                                       toConstDataRef(&strides, 1),
                                                                       toConstDataRef(&offsets, 1)));
        }
        setupBoxShader(inContext);
    }

    void render(QDemonRendererImpl &inWidgetContext, QDemonRenderContext &inRenderContext) override
    {
        m_itemName = "SWidgetBBox";
        QDemonWidgetRenderInformation theInfo(
                inWidgetContext.getWidgetRenderInformation(*m_node, m_node->position, RenderWidgetModes::Local));
        QDemonBounds2BoxPoints thePoints;
        m_bounds.expand(thePoints);
        QMatrix4x4 theNodeRotation;
        QMatrix4x4 theNodeToCamera = theInfo.m_nodeParentToCamera * m_node->localTransform;
        for (quint32 idx = 0; idx < 8; ++idx)
            thePoints[idx] = mat44::transform(theNodeToCamera, thePoints[idx]);
        setupBoundingBoxGraphicsObjects(inWidgetContext, toDataRef(thePoints, 8));
        if (m_boxShader && m_boxInputAssembler) {
            inRenderContext.setBlendingEnabled(false);
            inRenderContext.setDepthWriteEnabled(true);
            inRenderContext.setDepthTestEnabled(true);
            inRenderContext.setCullingEnabled(false);
            inRenderContext.setActiveShader(m_boxShader);
            m_boxShader->setPropertyValue("model_view_projection", theInfo.m_layerProjection);
            m_boxShader->setPropertyValue("output_color", m_color);
            inRenderContext.setInputAssembler(m_boxInputAssembler);
            inRenderContext.draw(QDemonRenderDrawMode::Lines, m_boxInputAssembler->indexCount(), 0);
        }
    }
};

struct QDemonWidgetAxis : public QDemonRenderWidgetInterface
{
    QDemonRef<QDemonRenderVertexBuffer> m_axisVertexBuffer;
    QDemonRef<QDemonRenderInputAssembler> m_axisInputAssembler;
    QDemonRef<QDemonRenderShaderProgram> m_axisShader;
    QByteArray m_itemName;

    QDemonWidgetAxis(QDemonGraphNode &inNode)
        : QDemonRenderWidgetInterface(inNode), m_axisVertexBuffer(nullptr), m_axisInputAssembler(nullptr), m_axisShader(nullptr)
    {
    }

    void setupAxisShader(QDemonRendererImpl &inContext)
    {
        m_axisShader = inContext.getShader(m_itemName);
        if (!m_axisShader) {
            QDemonRef<QDemonShaderProgramGeneratorInterface> theGenerator(inContext.getProgramGenerator());
            theGenerator->beginProgram();
            QDemonShaderStageGeneratorInterface &theVertexGenerator(*theGenerator->getStage(QDemonShaderGeneratorStage::Vertex));
            QDemonShaderStageGeneratorInterface &theFragmentGenerator(*theGenerator->getStage(QDemonShaderGeneratorStage::Fragment));
            theVertexGenerator.addIncoming("attr_pos", "vec3");
            theVertexGenerator.addIncoming("attr_color", "vec3");
            theVertexGenerator.addOutgoing("output_color", "vec3");
            theVertexGenerator.addUniform("model_view_projection", "mat4");
            theVertexGenerator.append("void main() {");
            theVertexGenerator.append("\tgl_Position = model_view_projection * vec4(attr_pos, 1.0);");
            theVertexGenerator.append("\toutput_color = attr_color;");
            theVertexGenerator.append("}");
            theFragmentGenerator.append("void main() {");
            theFragmentGenerator.append("\tgl_FragColor.rgb = output_color;");
            theFragmentGenerator.append("\tgl_FragColor.a = 1.0;");
            theFragmentGenerator.append("}");
            m_axisShader = inContext.compileAndStoreShader(m_itemName);
        }
    }

    void setupAxesGraphicsObjects(QDemonRendererImpl &inContext, QDemonDataRef<QVector3D> theAxes)
    {
        QDemonRenderVertexBufferEntry theEntries[] = {
            QDemonRenderVertexBufferEntry("attr_pos", QDemonRenderComponentType::Float16, 3),
            QDemonRenderVertexBufferEntry("attr_color", QDemonRenderComponentType::Float16, 3, 12),
        };

        m_axisVertexBuffer = inContext.getOrCreateVertexBuffer(m_itemName,
                                                               6 * sizeof(float),
                                                               toU8DataRef(theAxes.begin(), theAxes.size()));

        if (!m_axisInputAssembler && m_axisVertexBuffer) {
            // create our attribute layout
            QDemonRef<QDemonRenderAttribLayout> theAttribLAyout = inContext.createAttributeLayout(toConstDataRef(theEntries, 2));

            quint32 strides = m_axisVertexBuffer->stride();
            quint32 offsets = 0;
            m_axisInputAssembler = (inContext.getOrCreateInputAssembler(m_itemName,
                                                                        theAttribLAyout,
                                                                        toConstDataRef(&m_axisVertexBuffer, 1),
                                                                        nullptr,
                                                                        toConstDataRef(&strides, 1),
                                                                        toConstDataRef(&offsets, 1)));
        }
    }

    inline QVector3D transformDirection(const QMatrix3x3 &inMatrix, const QVector3D &inDir)
    {
        QVector3D retval = mat33::transform(inMatrix, inDir);
        retval.normalize();
        return retval;
    }
    void render(QDemonRendererImpl &inWidgetContext, QDemonRenderContext &inRenderContext) override
    {
        m_itemName = "SWidgetAxis";

        setupAxisShader(inWidgetContext);

        if (m_axisShader) {
            QVector3D Red = QVector3D(1, 0, 0);
            QVector3D Green = QVector3D(0, 1, 0);
            QVector3D Blue = QVector3D(0, 0, 1);
            if (m_node->parent && m_node->parent->type != QDemonGraphObject::Type::Layer) {
                m_node->parent->calculateGlobalVariables();
            }
            QVector3D thePivot(m_node->pivot);
            if (m_node->flags.testFlag(QDemonGraphNode::Flag::LeftHanded))
                thePivot[2] /* .z */ *= -1;
            QDemonWidgetRenderInformation theInfo(
                    inWidgetContext.getWidgetRenderInformation(*m_node, thePivot, RenderWidgetModes::Local));

            QMatrix4x4 theNodeRotation;
            m_node->calculateRotationMatrix(theNodeRotation);
            if (m_node->flags.testFlag(QDemonGraphNode::Flag::LeftHanded))
                QDemonGraphNode::flipCoordinateSystem(theNodeRotation);
            QMatrix3x3 theRotationMatrix(theNodeRotation.constData());
            // Move the camera position into camera space.  This is so that when we render we don't
            // have to account
            // for scaling done in the camera's MVP.
            QVector3D theItemPosition = theInfo.m_position;
            QMatrix3x3 theAxisTransform = theInfo.m_normalMatrix * theRotationMatrix;
            QVector3D xAxis = transformDirection(theAxisTransform, QVector3D(1, 0, 0));
            QVector3D yAxis = transformDirection(theAxisTransform, QVector3D(0, 1, 0));
            QVector3D zAxis = transformDirection(theAxisTransform, QVector3D(0, 0, -1));

            // This world to pixel scale factor function
            float theScaleFactor = theInfo.m_scale;
            theItemPosition = theInfo.m_position;

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

            setupAxesGraphicsObjects(inWidgetContext, toDataRef(theAxis, 3));

            if (m_axisInputAssembler) {
                inRenderContext.setBlendingEnabled(false);
                inRenderContext.setDepthWriteEnabled(false);
                inRenderContext.setDepthTestEnabled(false);
                inRenderContext.setCullingEnabled(false);
                inRenderContext.setActiveShader(m_axisShader);
                m_axisShader->setPropertyValue("model_view_projection", theInfo.m_layerProjection);
                inRenderContext.setInputAssembler(m_axisInputAssembler);
                // Draw six points.
                inRenderContext.draw(QDemonRenderDrawMode::Lines, 6, 0);
            }
        }
    }
};
}

QDemonRenderWidgetInterface::~QDemonRenderWidgetInterface() = default;

QDemonRef<QDemonRenderWidgetInterface> QDemonRenderWidgetInterface::createBoundingBoxWidget(QDemonGraphNode &inNode,
                                                                                            const QDemonBounds3 &inBounds,
                                                                                            const QVector3D &inColor)
{
    return QDemonRef<QDemonRenderWidgetInterface>(new QDemonWidgetBBox(inNode, inBounds, inColor));
}

QDemonRef<QDemonRenderWidgetInterface> QDemonRenderWidgetInterface::createAxisWidget(QDemonGraphNode &inNode)
{
    return QDemonRef<QDemonRenderWidgetInterface>(new QDemonWidgetAxis(inNode));
}

QT_END_NAMESPACE
