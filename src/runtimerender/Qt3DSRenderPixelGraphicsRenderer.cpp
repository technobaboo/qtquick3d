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
#include <Qt3DSRenderPixelGraphicsRenderer.h>
#include <Qt3DSRenderPixelGraphicsTypes.h>
#include <Qt3DSAtomic.h>
#include <qdemonrendercontext.h>
#include <Qt3DSRenderCamera.h>
#include <Qt3DSRenderContextCore.h>
#include <Qt3DSRenderShaderCodeGenerator.h>
#include <qdemonrendershaderprogram.h>
#include <Qt3DSRenderShaderCache.h>

using namespace qt3ds;
using namespace render;

namespace {

struct SPGRectShader
{
    QDemonScopedRefCounted<QDemonRenderShaderProgram> m_RectShader;
    QDemonRenderShaderConstantBase *mvp;
    QDemonRenderShaderConstantBase *rectColor;
    QDemonRenderShaderConstantBase *leftright;
    QDemonRenderShaderConstantBase *bottomtop;

    SPGRectShader()
        : mvp(nullptr)
        , rectColor(nullptr)
        , leftright(nullptr)
        , bottomtop(nullptr)
    {
    }
    void SetShader(QDemonRenderShaderProgram *program)
    {
        m_RectShader = program;
        if (program) {
            mvp = program->GetShaderConstant("model_view_projection");
            rectColor = program->GetShaderConstant("rect_color");
            leftright = program->GetShaderConstant("leftright[0]");
            bottomtop = program->GetShaderConstant("bottomtop[0]");
        }
    }

    void Apply(QMatrix4x4 &inVP, const SPGRect &inObject)
    {
        if (mvp)
            m_RectShader->SetConstantValue(mvp, toConstDataRef(inVP), 1);
        if (rectColor)
            m_RectShader->SetConstantValue(rectColor, inObject.m_FillColor, 1);
        if (leftright) {
            float theData[] = { inObject.m_Left, inObject.m_Right };
            m_RectShader->SetConstantValue(leftright, *theData, 2);
        }
        if (bottomtop) {
            float theData[] = { inObject.m_Bottom, inObject.m_Top };
            m_RectShader->SetConstantValue(bottomtop, *theData, 2);
        }
    }

    operator bool() { return m_RectShader.mPtr != nullptr; }
};

struct SPGRenderer : public IPixelGraphicsRenderer
{
    IQt3DSRenderContext &m_RenderContext;
    IStringTable &m_StringTable;
    QDemonScopedRefCounted<QDemonRenderVertexBuffer> m_QuadVertexBuffer;
    QDemonScopedRefCounted<QDemonRenderIndexBuffer> m_QuadIndexBuffer;
    QDemonScopedRefCounted<QDemonRenderInputAssembler> m_QuadInputAssembler;
    QDemonScopedRefCounted<QDemonRenderAttribLayout> m_QuadAttribLayout;
    SShaderVertexCodeGenerator m_VertexGenerator;
    SShaderFragmentCodeGenerator m_FragmentGenerator;
    SPGRectShader m_RectShader;
    qint32 mRefCount;

    SPGRenderer(IQt3DSRenderContext &ctx, IStringTable &strt)
        : m_RenderContext(ctx)
        , m_StringTable(strt)
        , m_VertexGenerator(m_StringTable, ctx.GetAllocator(),
                            m_RenderContext.GetRenderContext().GetRenderContextType())
        , m_FragmentGenerator(m_VertexGenerator, ctx.GetAllocator(),
                              m_RenderContext.GetRenderContext().GetRenderContextType())
        , mRefCount(0)
    {
    }

    QDEMON_IMPLEMENT_REF_COUNT_ADDREF_RELEASE_OVERRIDE(m_RenderContext.GetAllocator())
    void GetRectShaderProgram()
    {
        if (!m_RectShader) {
            m_VertexGenerator.Begin();
            m_FragmentGenerator.Begin();
            m_VertexGenerator.AddAttribute("attr_pos", "vec2");
            m_VertexGenerator.AddUniform("model_view_projection", "mat4");
            m_VertexGenerator.AddUniform("leftright[2]", "float");
            m_VertexGenerator.AddUniform("bottomtop[2]", "float");
            m_FragmentGenerator.AddVarying("rect_uvs", "vec2");
            m_FragmentGenerator.AddUniform("rect_color", "vec4");
            m_VertexGenerator << "void main() {" << Endl
                              << "\tgl_Position = model_view_projection * vec4( "
                                 "leftright[int(attr_pos.x)], bottomtop[int(attr_pos.y)], 0.0, 1.0 "
                                 ");"
                              << Endl << "\trect_uvs = attr_pos;" << Endl << "}" << Endl;

            m_FragmentGenerator << "void main() {" << Endl << "\tfragOutput = rect_color;" << Endl
                                << "}" << Endl;

            m_VertexGenerator.BuildShaderSource();
            m_FragmentGenerator.BuildShaderSource();

            m_RectShader.SetShader(m_RenderContext.GetShaderCache().CompileProgram(
                m_StringTable.RegisterStr("PixelRectShader"),
                m_VertexGenerator.m_FinalShaderBuilder.c_str(),
                m_FragmentGenerator.m_FinalShaderBuilder.c_str(), nullptr // no tess control shader
                ,
                nullptr // no tess eval shader
                ,
                nullptr // no geometry shader
                ,
                SShaderCacheProgramFlags(), ShaderCacheNoFeatures()));
        }
    }
    void GenerateXYQuad()
    {
        QDemonRenderContext &theRenderContext(m_RenderContext.GetRenderContext());

        QDemonRenderVertexBufferEntry theEntries[] = {
            QDemonRenderVertexBufferEntry("attr_pos",
                                                  QDemonRenderComponentTypes::float, 2),
        };

        QVector2D pos[] = { QVector2D(0, 0), QVector2D(0, 1), QVector2D(1, 1), QVector2D(1, 0) };

        if (m_QuadVertexBuffer == nullptr) {
            size_t bufSize = sizeof(pos);
            m_QuadVertexBuffer = theRenderContext.CreateVertexBuffer(
                QDemonRenderBufferUsageType::Static, bufSize, 2 * sizeof(float),
                toU8DataRef(pos, 4));
        }

        if (m_QuadIndexBuffer == nullptr) {
            quint8 indexData[] = {
                0, 1, 2, 0, 2, 3,
            };
            m_QuadIndexBuffer = theRenderContext.CreateIndexBuffer(
                QDemonRenderBufferUsageType::Static,
                QDemonRenderComponentTypes::quint8, sizeof(indexData),
                toU8DataRef(indexData, sizeof(indexData)));
        }

        if (m_QuadAttribLayout == nullptr) {
            // create our attribute layout
            m_QuadAttribLayout =
                theRenderContext.CreateAttributeLayout(toConstDataRef(theEntries, 1));
        }

        if (m_QuadInputAssembler == nullptr) {

            // create input assembler object
            quint32 strides = m_QuadVertexBuffer->GetStride();
            quint32 offsets = 0;
            m_QuadInputAssembler = theRenderContext.CreateInputAssembler(
                m_QuadAttribLayout, toConstDataRef(&m_QuadVertexBuffer.mPtr, 1), m_QuadIndexBuffer,
                toConstDataRef(&strides, 1), toConstDataRef(&offsets, 1));
        }
    }

    void RenderPixelObject(QMatrix4x4 &inProjection, const SPGRect &inObject)
    {
        GenerateXYQuad();
        GetRectShaderProgram();
        if (m_RectShader) {
            m_RenderContext.GetRenderContext().SetActiveShader(m_RectShader.m_RectShader.mPtr);
            m_RectShader.Apply(inProjection, inObject);

            m_RenderContext.GetRenderContext().SetInputAssembler(m_QuadInputAssembler.mPtr);
            m_RenderContext.GetRenderContext().Draw(QDemonRenderDrawMode::Triangles,
                                                    m_QuadInputAssembler->GetIndexCount(), 0);
        }
    }

    void RenderPixelObject(QMatrix4x4 &inProjection, const SPGVertLine &inObject)
    {
        // lines are really just rects, but they grow in width in a sort of odd way.
        // specifically, they grow the increasing coordinate on even boundaries and centered on odd
        // boundaries.
        SPGRect theRect;
        theRect.m_Top = inObject.m_Top;
        theRect.m_Bottom = inObject.m_Bottom;
        theRect.m_FillColor = inObject.m_LineColor;
        theRect.m_Left = inObject.m_X;
        theRect.m_Right = theRect.m_Left + 1.0f;
        RenderPixelObject(inProjection, theRect);
    }

    void RenderPixelObject(QMatrix4x4 &inProjection, const SPGHorzLine &inObject)
    {
        SPGRect theRect;
        theRect.m_Right = inObject.m_Right;
        theRect.m_Left = inObject.m_Left;
        theRect.m_FillColor = inObject.m_LineColor;
        theRect.m_Bottom = inObject.m_Y;
        theRect.m_Top = theRect.m_Bottom + 1.0f;
        RenderPixelObject(inProjection, theRect);
    }

    void Render(QDemonConstDataRef<SPGGraphObject *> inObjects) override
    {
        QDemonRenderContext &theRenderContext(m_RenderContext.GetRenderContext());
        theRenderContext.PushPropertySet();
        // Setup an orthographic camera that places the center at the
        // lower left of the viewport.
        QDemonRenderRectF theViewport = theRenderContext.GetViewport();
        // With no projection at all, we are going to get a square view box
        // with boundaries from -1,1 in all dimensions.  This is close to what we want.
        theRenderContext.SetDepthTestEnabled(false);
        theRenderContext.SetDepthWriteEnabled(false);
        theRenderContext.SetScissorTestEnabled(false);
        theRenderContext.SetBlendingEnabled(true);
        theRenderContext.SetCullingEnabled(false);
        // Colors are expected to be non-premultiplied, so we premultiply alpha into them at this
        // point.
        theRenderContext.SetBlendFunction(QDemonRenderBlendFunctionArgument(
            QDemonRenderSrcBlendFunc::SrcAlpha, QDemonRenderDstBlendFunc::OneMinusSrcAlpha,
            QDemonRenderSrcBlendFunc::One, QDemonRenderDstBlendFunc::OneMinusSrcAlpha));
        theRenderContext.SetBlendEquation(QDemonRenderBlendEquationArgument(
            QDemonRenderBlendEquation::Add, QDemonRenderBlendEquation::Add));

        SCamera theCamera;
        theCamera.m_Position.z = -5;
        theCamera.m_ClipNear = 1.0f;
        theCamera.m_ClipFar = 10.0f;
        theCamera.m_Flags.SetOrthographic(true);
        // Setup camera projection
        theCamera.ComputeFrustumOrtho(theViewport,
                                      QVector2D(theViewport.m_Width, theViewport.m_Height));
        // Translate such that 0, 0 is lower left of screen.
        QDemonRenderRectF theIdealViewport = theViewport;
        theIdealViewport.m_X -= theViewport.m_Width / 2.0f;
        theIdealViewport.m_Y -= theViewport.m_Height / 2.0f;
        QMatrix4x4 theProjectionMatrix = QDemonRenderContext::ApplyVirtualViewportToProjectionMatrix(
            theCamera.m_Projection, theViewport, theIdealViewport);
        theCamera.m_Projection = theProjectionMatrix;
        // Explicitly call the node's calculate global variables so that the camera doesn't attempt
        // to change the projection we setup.
        static_cast<SNode &>(theCamera).CalculateGlobalVariables();
        QMatrix4x4 theVPMatrix(QMatrix4x4::createIdentity());
        theCamera.CalculateViewProjectionMatrix(theVPMatrix);

        QVector4D theTest(60, 200, 0, 1);
        QVector4D theResult = theVPMatrix.transform(theTest);

        (void)theTest;
        (void)theResult;

        for (quint32 idx = 0, end = inObjects.size(); idx < end; ++idx) {
            const SPGGraphObject &theObject(*inObjects[idx]);

            switch (theObject.m_Type) {
            case SGTypes::VertLine:
                RenderPixelObject(theVPMatrix, static_cast<const SPGVertLine &>(theObject));
                break;
            case SGTypes::HorzLine:
                RenderPixelObject(theVPMatrix, static_cast<const SPGHorzLine &>(theObject));
                break;
            case SGTypes::Rect:
                RenderPixelObject(theVPMatrix, static_cast<const SPGRect &>(theObject));
                break;
            default:
                Q_ASSERT(false);
                break;
            }
        }

        theRenderContext.PopPropertySet(false);
    }
};
}

IPixelGraphicsRenderer &IPixelGraphicsRenderer::CreateRenderer(IQt3DSRenderContext &ctx,
                                                               IStringTable &strt)
{
    return *QDEMON_NEW(ctx.GetAllocator(), SPGRenderer)(ctx, strt);
}
