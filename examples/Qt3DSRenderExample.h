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
#pragma once
#ifndef QT3DS_RENDER_EXAMPLE_H
#define QT3DS_RENDER_EXAMPLE_H
#include "foundation/Qt3DS.h"
#include "foundation/Qt3DSAssert.h"
#include "foundation/Qt3DSFoundation.h"
#include "foundation/Qt3DSBroadcastingAllocator.h"
#include "render/NvRenderBaseTypes.h"
#include "render/Qt3DSRenderContext.h"

namespace qt3ds {
namespace render {

    class NVRenderContext;

    class NVRenderExample
    {
    protected:
        virtual ~NVRenderExample() {}
    public:
        virtual void drawFrame(double currentSeconds) = 0;
        virtual QT3DSU32 getRuntimeInSeconds() { return 5; }
        virtual void handleKeypress(int /*keypress*/) {}
        virtual void release() = 0;
    };

    typedef NVRenderExample *(*TExampleCreateFunc)(NVRenderContext &context);

    struct NVRenderExampleFactory
    {
        static TExampleCreateFunc *mExampleCreators;
        static QT3DSU32 mMaxCreators;
        static QT3DSU32 mNumCreators;

        static void addCreator(TExampleCreateFunc creator)
        {
            if (mNumCreators < mMaxCreators) {
                mExampleCreators[mNumCreators] = creator;
                ++mNumCreators;
            } else
                QT3DS_ASSERT(false);
        }

        // Assuming that the render context is egl
        // Relies on the global structures defined in demo common
        // to figure out window state.
        static bool nextExample();
        static void beginExamples();
        static bool update();
        static void endExamples();
    };

    template <typename TExample>
    struct NVRenderExampleCreator
    {

        static NVRenderExample *createExample(NVRenderContext &context)
        {
            return QT3DS_NEW(context.GetFoundation().getAllocator(), TExample)(context);
        }
        NVRenderExampleCreator() { NVRenderExampleFactory::addCreator(createExample); }
    };

#define QT3DS_RENDER_REGISTER_EXAMPLE(dtype) static NVRenderExampleCreator<dtype> dtype##Creator

#define eps 1e-4

    int eq(float a, float b);

    // Matrix functions
    void NvGl2DemoMatrixIdentity(float m[16]);
    int NvGl2DemoMatrixEquals(float a[16], float b[16]);
    void NvGl2DemoMatrixTranspose(float m[16]);
    void NvGl2DemoMatrixMultiply(float m0[16], float m1[16]);
    void NvGl2DemoMatrixMultiply_4x4_3x3(float m0[16], float m1[9]);
    void NvGl2DemoMatrixMultiply_3x3(float m0[9], float m1[9]);
    void NvGl2DemoMatrixFrustum(float m[16], float l, float r, float b, float t, float n, float f);
    void NvGl2DemoMatrixOrtho(float m[16], float l, float r, float b, float t, float n, float f);
    void NvGl2DemoMatrixTranslate(float m[16], float x, float y, float z);
    void NvGl2DemoMatrixRotate_create3x3(float m[9], float theta, float x, float y, float z);
    void NvGl2DemoMatrixRotate(float m[16], float theta, float x, float y, float z);
    void NvGl2DemoMatrixRotate_3x3(float m[9], float theta, float x, float y, float z);

    float NvGl2DemoMatrixDeterminant(float m[16]);
    void NvGl2DemoMatrixInverse(float m[16]);
    void NvGl2DemoMatrixCopy(float dest[16], float src[16]);

    void NvGl2DemoMatrixPrint(float a[16]);

    void NvGl2DemoMatrixVectorMultiply(float m[16], float v[4]);
}
}

#endif