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
#include "Qt3DSRenderExample.h"
#include "render/NvRenderBaseTypes.h"
#include "render_util/NVRenderAllocator.h"
#include "render_util/NVRenderErrorStream.h"
#include "render_util/NVRenderUtils.h"
#include "foundation/Qt3DSFoundation.h"
#include "foundation/Qt3DSBroadcastingAllocator.h"
#include "foundation/Qt3DSTime.h"
#include "render/Qt3DSRenderContext.h"
#include "foundation/Qt3DSMath.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

using namespace qt3ds;
using namespace qt3ds::render;

static TExampleCreateFunc gCreateFuncs[128];

TExampleCreateFunc *NVRenderExampleFactory::mExampleCreators = gCreateFuncs;
QT3DSU32 NVRenderExampleFactory::mMaxCreators = 128;
QT3DSU32 NVRenderExampleFactory::mNumCreators = 0;
namespace {
NVRenderErrorStream g_errorStream;
NVRenderAllocator g_allocator(g_errorStream);
NVFoundation *g_foundation(NULL);
NVRenderContext *g_renderContext(NULL);
NVRenderExample *g_example(NULL);
QT3DSI32 g_exampleIdx(0);
Time::Second g_runTime;
Time g_timer;
}

bool NVRenderExampleFactory::nextExample()
{
    if (g_example)
        g_example->release();
    g_example = 0;
    if (g_exampleIdx < (int)mNumCreators) {
        g_example = mExampleCreators[g_exampleIdx](*g_renderContext);
        ++g_exampleIdx;
        g_runTime = g_example->getRuntimeInSeconds();
        g_timer = Time();
        return true;
    }
    return false;
}

void NVRenderExampleFactory::beginExamples()
{
    g_foundation = NVCreateFoundation(QT3DS_FOUNDATION_VERSION, g_allocator, g_errorStream);
    g_renderContext =
        &NVRenderContext::CreateGL(*g_foundation, NVRenderContextType(NVRenderContextValues::GL2));
    nextExample();
}
bool NVRenderExampleFactory::update()
{
    Time::Second currentTime = g_timer.peekElapsedSeconds();
    if (currentTime > g_runTime)
        nextExample();
    if (g_example)
        g_example->drawFrame(currentTime);

    return g_example != NULL;
}

void NVRenderExampleFactory::endExamples()
{
    g_exampleIdx = mNumCreators;
    nextExample();
    if (g_renderContext)
        g_renderContext->release();
    if (g_foundation)
        g_foundation->release();

    g_renderContext = NULL;
    g_foundation = NULL;
}

namespace qt3ds {
namespace render {
    ///////////////////////////////////////////////////////////////////////////////

    // Math stuff

    int eq(float a, float b)
    {
        float diff = a - b;
        if (diff < 0) {
            diff = -diff;
        }
        return diff <= eps;
    }

    //
    // Matrix functions, since GLES 2.x doesn't provide them
    //

    void NvGl2DemoMatrixIdentity(float m[16])
    {
        memset(m, 0, sizeof(float) * 16);
        m[4 * 0 + 0] = m[4 * 1 + 1] = m[4 * 2 + 2] = m[4 * 3 + 3] = 1.0;
    }

    int NvGl2DemoMatrixEquals(float a[16], float b[16])
    {
        int i;
        for (i = 0; i < 16; ++i)
            if (!eq(a[i], b[i]))
                return 0;

        return 1;
    }

    void NvGl2DemoMatrixTranspose(float m[16])
    {
        int i, j;
        float t;
        for (i = 1; i < 4; ++i)
            for (j = 0; j < i; ++j) {
                t = m[4 * i + j];
                m[4 * i + j] = m[4 * j + i];
                m[4 * j + i] = t;
            }
    }

    void NvGl2DemoMatrixMultiply(float m0[16], float m1[16])
    {
        int r, c, i;
        for (r = 0; r < 4; r++) {
            float m[4] = { 0.0, 0.0, 0.0, 0.0 };
            for (c = 0; c < 4; c++) {
                for (i = 0; i < 4; i++) {
                    m[c] += m0[4 * i + r] * m1[4 * c + i];
                }
            }
            for (c = 0; c < 4; c++) {
                m0[4 * c + r] = m[c];
            }
        }
    }

    void NvGl2DemoMatrixMultiply_4x4_3x3(float m0[16], float m1[9])
    {
        int r, c, i;
        for (r = 0; r < 4; r++) {
            float m[3] = { 0.0, 0.0, 0.0 };
            for (c = 0; c < 3; c++) {
                for (i = 0; i < 3; i++) {
                    m[c] += m0[4 * i + r] * m1[3 * c + i];
                }
            }
            for (c = 0; c < 3; c++) {
                m0[4 * c + r] = m[c];
            }
        }
    }

    void NvGl2DemoMatrixMultiply_3x3(float m0[9], float m1[9])
    {
        int r, c, i;
        for (r = 0; r < 3; r++) {
            float m[3] = { 0.0, 0.0, 0.0 };
            for (c = 0; c < 3; c++) {
                for (i = 0; i < 3; i++) {
                    m[c] += m0[3 * i + r] * m1[3 * c + i];
                }
            }
            for (c = 0; c < 3; c++) {
                m0[3 * c + r] = m[c];
            }
        }
    }

    void NvGl2DemoMatrixFrustum(float m[16], float l, float r, float b, float t, float n, float f)
    {
        float m1[16];
        float rightMinusLeftInv, topMinusBottomInv, farMinusNearInv, twoNear;

        rightMinusLeftInv = 1.0f / (r - l);
        topMinusBottomInv = 1.0f / (t - b);
        farMinusNearInv = 1.0f / (f - n);
        twoNear = 2.0f * n;

        m1[0] = twoNear * rightMinusLeftInv;
        m1[1] = 0.0f;
        m1[2] = 0.0f;
        m1[3] = 0.0f;

        m1[4] = 0.0f;
        m1[5] = twoNear * topMinusBottomInv;
        m1[6] = 0.0f;
        m1[7] = 0.0f;

        m1[8] = (r + l) * rightMinusLeftInv;
        m1[9] = (t + b) * topMinusBottomInv;
        m1[10] = -(f + n) * farMinusNearInv;
        m1[11] = -1.0f;

        m1[12] = 0.0f;
        m1[13] = 0.0f;
        m1[14] = -(twoNear * f) * farMinusNearInv;
        m1[15] = 0.0f;

        NvGl2DemoMatrixMultiply(m, m1);
    }

    void NvGl2DemoMatrixOrtho(float m[16], float l, float r, float b, float t, float n, float f)
    {
        float m1[16];
        float rightMinusLeftInv, topMinusBottomInv, farMinusNearInv;

        rightMinusLeftInv = 1.0f / (r - l);
        topMinusBottomInv = 1.0f / (t - b);
        farMinusNearInv = 1.0f / (f - n);

        m1[0] = 2.0f * rightMinusLeftInv;
        m1[1] = 0.0f;
        m1[2] = 0.0f;
        m1[3] = 0.0f;

        m1[4] = 0.0f;
        m1[5] = 2.0f * topMinusBottomInv;
        m1[6] = 0.0f;
        m1[7] = 0.0f;

        m1[8] = 0.0f;
        m1[9] = 0.0f;
        m1[10] = -2.0f * farMinusNearInv;
        m1[11] = 0.0f;

        m1[12] = -(r + l) * rightMinusLeftInv;
        m1[13] = -(t + b) * topMinusBottomInv;
        m1[14] = -(f + n) * farMinusNearInv;
        m1[15] = 1.0f;

        NvGl2DemoMatrixMultiply(m, m1);
    }

    void NvGl2DemoMatrixTranslate(float m[16], float x, float y, float z)
    {
        float m1[16];
        NvGl2DemoMatrixIdentity(m1);

        m1[4 * 3 + 0] = x;
        m1[4 * 3 + 1] = y;
        m1[4 * 3 + 2] = z;

        NvGl2DemoMatrixMultiply(m, m1);
    }

    void NvGl2DemoMatrixRotate_create3x3(float m[9], float theta, float x, float y, float z)
    {
        float len = (float)sqrtf(x * x + y * y + z * z);
        float u0 = x / len;
        float u1 = y / len;
        float u2 = z / len;
        float rad = (float)(theta / 180 * NVPi);
        float c = (float)cosf(rad);
        float s = (float)sinf(rad);
        m[3 * 0 + 0] = u0 * u0 + c * (1 - u0 * u0) + s * 0;
        m[3 * 0 + 1] = u0 * u1 + c * (0 - u0 * u1) + s * u2;
        m[3 * 0 + 2] = u0 * u2 + c * (0 - u0 * u2) - s * u1;

        m[3 * 1 + 0] = u1 * u0 + c * (0 - u1 * u0) - s * u2;
        m[3 * 1 + 1] = u1 * u1 + c * (1 - u1 * u1) + s * 0;
        m[3 * 1 + 2] = u1 * u2 + c * (0 - u1 * u2) + s * u0;

        m[3 * 2 + 0] = u2 * u0 + c * (0 - u2 * u0) + s * u1;
        m[3 * 2 + 1] = u2 * u1 + c * (0 - u2 * u1) - s * u0;
        m[3 * 2 + 2] = u2 * u2 + c * (1 - u2 * u2) + s * 0;
    }

    void NvGl2DemoMatrixRotate(float m[16], float theta, float x, float y, float z)
    {
        float r[9];
        NvGl2DemoMatrixRotate_create3x3(r, theta, x, y, z);
        NvGl2DemoMatrixMultiply_4x4_3x3(m, r);
    }

    void NvGl2DemoMatrixRotate_3x3(float m[9], float theta, float x, float y, float z)
    {
        float r[9];
        NvGl2DemoMatrixRotate_create3x3(r, theta, x, y, z);
        NvGl2DemoMatrixMultiply_3x3(m, r);
    }

    float NvGl2DemoMatrixDeterminant(float m[16])
    {
        return m[4 * 0 + 3] * m[4 * 1 + 2] * m[4 * 2 + 1] * m[4 * 3 + 0]
            - m[4 * 0 + 2] * m[4 * 1 + 3] * m[4 * 2 + 1] * m[4 * 3 + 0]
            - m[4 * 0 + 3] * m[4 * 1 + 1] * m[4 * 2 + 2] * m[4 * 3 + 0]
            + m[4 * 0 + 1] * m[4 * 1 + 3] * m[4 * 2 + 2] * m[4 * 3 + 0]
            + m[4 * 0 + 2] * m[4 * 1 + 1] * m[4 * 2 + 3] * m[4 * 3 + 0]
            - m[4 * 0 + 1] * m[4 * 1 + 2] * m[4 * 2 + 3] * m[4 * 3 + 0]
            - m[4 * 0 + 3] * m[4 * 1 + 2] * m[4 * 2 + 0] * m[4 * 3 + 1]
            + m[4 * 0 + 2] * m[4 * 1 + 3] * m[4 * 2 + 0] * m[4 * 3 + 1]
            + m[4 * 0 + 3] * m[4 * 1 + 0] * m[4 * 2 + 2] * m[4 * 3 + 1]
            - m[4 * 0 + 0] * m[4 * 1 + 3] * m[4 * 2 + 2] * m[4 * 3 + 1]
            - m[4 * 0 + 2] * m[4 * 1 + 0] * m[4 * 2 + 3] * m[4 * 3 + 1]
            + m[4 * 0 + 0] * m[4 * 1 + 2] * m[4 * 2 + 3] * m[4 * 3 + 1]
            + m[4 * 0 + 3] * m[4 * 1 + 1] * m[4 * 2 + 0] * m[4 * 3 + 2]
            - m[4 * 0 + 1] * m[4 * 1 + 3] * m[4 * 2 + 0] * m[4 * 3 + 2]
            - m[4 * 0 + 3] * m[4 * 1 + 0] * m[4 * 2 + 1] * m[4 * 3 + 2]
            + m[4 * 0 + 0] * m[4 * 1 + 3] * m[4 * 2 + 1] * m[4 * 3 + 2]
            + m[4 * 0 + 1] * m[4 * 1 + 0] * m[4 * 2 + 3] * m[4 * 3 + 2]
            - m[4 * 0 + 0] * m[4 * 1 + 1] * m[4 * 2 + 3] * m[4 * 3 + 2]
            - m[4 * 0 + 2] * m[4 * 1 + 1] * m[4 * 2 + 0] * m[4 * 3 + 3]
            + m[4 * 0 + 1] * m[4 * 1 + 2] * m[4 * 2 + 0] * m[4 * 3 + 3]
            + m[4 * 0 + 2] * m[4 * 1 + 0] * m[4 * 2 + 1] * m[4 * 3 + 3]
            - m[4 * 0 + 0] * m[4 * 1 + 2] * m[4 * 2 + 1] * m[4 * 3 + 3]
            - m[4 * 0 + 1] * m[4 * 1 + 0] * m[4 * 2 + 2] * m[4 * 3 + 3]
            + m[4 * 0 + 0] * m[4 * 1 + 1] * m[4 * 2 + 2] * m[4 * 3 + 3];
    }

    void NvGl2DemoMatrixInverse(float m[16])
    {
        float a[16];
        float det;
        int i;
        float b[16], e[16];

        a[4 * 0 + 0] = m[4 * 1 + 2] * m[4 * 2 + 3] * m[4 * 3 + 1]
            - m[4 * 1 + 3] * m[4 * 2 + 2] * m[4 * 3 + 1]
            + m[4 * 1 + 3] * m[4 * 2 + 1] * m[4 * 3 + 2]
            - m[4 * 1 + 1] * m[4 * 2 + 3] * m[4 * 3 + 2]
            - m[4 * 1 + 2] * m[4 * 2 + 1] * m[4 * 3 + 3]
            + m[4 * 1 + 1] * m[4 * 2 + 2] * m[4 * 3 + 3];
        a[4 * 0 + 1] = m[4 * 0 + 3] * m[4 * 2 + 2] * m[4 * 3 + 1]
            - m[4 * 0 + 2] * m[4 * 2 + 3] * m[4 * 3 + 1]
            - m[4 * 0 + 3] * m[4 * 2 + 1] * m[4 * 3 + 2]
            + m[4 * 0 + 1] * m[4 * 2 + 3] * m[4 * 3 + 2]
            + m[4 * 0 + 2] * m[4 * 2 + 1] * m[4 * 3 + 3]
            - m[4 * 0 + 1] * m[4 * 2 + 2] * m[4 * 3 + 3];
        a[4 * 0 + 2] = m[4 * 0 + 2] * m[4 * 1 + 3] * m[4 * 3 + 1]
            - m[4 * 0 + 3] * m[4 * 1 + 2] * m[4 * 3 + 1]
            + m[4 * 0 + 3] * m[4 * 1 + 1] * m[4 * 3 + 2]
            - m[4 * 0 + 1] * m[4 * 1 + 3] * m[4 * 3 + 2]
            - m[4 * 0 + 2] * m[4 * 1 + 1] * m[4 * 3 + 3]
            + m[4 * 0 + 1] * m[4 * 1 + 2] * m[4 * 3 + 3];
        a[4 * 0 + 3] = m[4 * 0 + 3] * m[4 * 1 + 2] * m[4 * 2 + 1]
            - m[4 * 0 + 2] * m[4 * 1 + 3] * m[4 * 2 + 1]
            - m[4 * 0 + 3] * m[4 * 1 + 1] * m[4 * 2 + 2]
            + m[4 * 0 + 1] * m[4 * 1 + 3] * m[4 * 2 + 2]
            + m[4 * 0 + 2] * m[4 * 1 + 1] * m[4 * 2 + 3]
            - m[4 * 0 + 1] * m[4 * 1 + 2] * m[4 * 2 + 3];
        a[4 * 1 + 0] = m[4 * 1 + 3] * m[4 * 2 + 2] * m[4 * 3 + 0]
            - m[4 * 1 + 2] * m[4 * 2 + 3] * m[4 * 3 + 0]
            - m[4 * 1 + 3] * m[4 * 2 + 0] * m[4 * 3 + 2]
            + m[4 * 1 + 0] * m[4 * 2 + 3] * m[4 * 3 + 2]
            + m[4 * 1 + 2] * m[4 * 2 + 0] * m[4 * 3 + 3]
            - m[4 * 1 + 0] * m[4 * 2 + 2] * m[4 * 3 + 3];
        a[4 * 1 + 1] = m[4 * 0 + 2] * m[4 * 2 + 3] * m[4 * 3 + 0]
            - m[4 * 0 + 3] * m[4 * 2 + 2] * m[4 * 3 + 0]
            + m[4 * 0 + 3] * m[4 * 2 + 0] * m[4 * 3 + 2]
            - m[4 * 0 + 0] * m[4 * 2 + 3] * m[4 * 3 + 2]
            - m[4 * 0 + 2] * m[4 * 2 + 0] * m[4 * 3 + 3]
            + m[4 * 0 + 0] * m[4 * 2 + 2] * m[4 * 3 + 3];
        a[4 * 1 + 2] = m[4 * 0 + 3] * m[4 * 1 + 2] * m[4 * 3 + 0]
            - m[4 * 0 + 2] * m[4 * 1 + 3] * m[4 * 3 + 0]
            - m[4 * 0 + 3] * m[4 * 1 + 0] * m[4 * 3 + 2]
            + m[4 * 0 + 0] * m[4 * 1 + 3] * m[4 * 3 + 2]
            + m[4 * 0 + 2] * m[4 * 1 + 0] * m[4 * 3 + 3]
            - m[4 * 0 + 0] * m[4 * 1 + 2] * m[4 * 3 + 3];
        a[4 * 1 + 3] = m[4 * 0 + 2] * m[4 * 1 + 3] * m[4 * 2 + 0]
            - m[4 * 0 + 3] * m[4 * 1 + 2] * m[4 * 2 + 0]
            + m[4 * 0 + 3] * m[4 * 1 + 0] * m[4 * 2 + 2]
            - m[4 * 0 + 0] * m[4 * 1 + 3] * m[4 * 2 + 2]
            - m[4 * 0 + 2] * m[4 * 1 + 0] * m[4 * 2 + 3]
            + m[4 * 0 + 0] * m[4 * 1 + 2] * m[4 * 2 + 3];
        a[4 * 2 + 0] = m[4 * 1 + 1] * m[4 * 2 + 3] * m[4 * 3 + 0]
            - m[4 * 1 + 3] * m[4 * 2 + 1] * m[4 * 3 + 0]
            + m[4 * 1 + 3] * m[4 * 2 + 0] * m[4 * 3 + 1]
            - m[4 * 1 + 0] * m[4 * 2 + 3] * m[4 * 3 + 1]
            - m[4 * 1 + 1] * m[4 * 2 + 0] * m[4 * 3 + 3]
            + m[4 * 1 + 0] * m[4 * 2 + 1] * m[4 * 3 + 3];
        a[4 * 2 + 1] = m[4 * 0 + 3] * m[4 * 2 + 1] * m[4 * 3 + 0]
            - m[4 * 0 + 1] * m[4 * 2 + 3] * m[4 * 3 + 0]
            - m[4 * 0 + 3] * m[4 * 2 + 0] * m[4 * 3 + 1]
            + m[4 * 0 + 0] * m[4 * 2 + 3] * m[4 * 3 + 1]
            + m[4 * 0 + 1] * m[4 * 2 + 0] * m[4 * 3 + 3]
            - m[4 * 0 + 0] * m[4 * 2 + 1] * m[4 * 3 + 3];
        a[4 * 2 + 2] = m[4 * 0 + 1] * m[4 * 1 + 3] * m[4 * 3 + 0]
            - m[4 * 0 + 3] * m[4 * 1 + 1] * m[4 * 3 + 0]
            + m[4 * 0 + 3] * m[4 * 1 + 0] * m[4 * 3 + 1]
            - m[4 * 0 + 0] * m[4 * 1 + 3] * m[4 * 3 + 1]
            - m[4 * 0 + 1] * m[4 * 1 + 0] * m[4 * 3 + 3]
            + m[4 * 0 + 0] * m[4 * 1 + 1] * m[4 * 3 + 3];
        a[4 * 2 + 3] = m[4 * 0 + 3] * m[4 * 1 + 1] * m[4 * 2 + 0]
            - m[4 * 0 + 1] * m[4 * 1 + 3] * m[4 * 2 + 0]
            - m[4 * 0 + 3] * m[4 * 1 + 0] * m[4 * 2 + 1]
            + m[4 * 0 + 0] * m[4 * 1 + 3] * m[4 * 2 + 1]
            + m[4 * 0 + 1] * m[4 * 1 + 0] * m[4 * 2 + 3]
            - m[4 * 0 + 0] * m[4 * 1 + 1] * m[4 * 2 + 3];
        a[4 * 3 + 0] = m[4 * 1 + 2] * m[4 * 2 + 1] * m[4 * 3 + 0]
            - m[4 * 1 + 1] * m[4 * 2 + 2] * m[4 * 3 + 0]
            - m[4 * 1 + 2] * m[4 * 2 + 0] * m[4 * 3 + 1]
            + m[4 * 1 + 0] * m[4 * 2 + 2] * m[4 * 3 + 1]
            + m[4 * 1 + 1] * m[4 * 2 + 0] * m[4 * 3 + 2]
            - m[4 * 1 + 0] * m[4 * 2 + 1] * m[4 * 3 + 2];
        a[4 * 3 + 1] = m[4 * 0 + 1] * m[4 * 2 + 2] * m[4 * 3 + 0]
            - m[4 * 0 + 2] * m[4 * 2 + 1] * m[4 * 3 + 0]
            + m[4 * 0 + 2] * m[4 * 2 + 0] * m[4 * 3 + 1]
            - m[4 * 0 + 0] * m[4 * 2 + 2] * m[4 * 3 + 1]
            - m[4 * 0 + 1] * m[4 * 2 + 0] * m[4 * 3 + 2]
            + m[4 * 0 + 0] * m[4 * 2 + 1] * m[4 * 3 + 2];
        a[4 * 3 + 2] = m[4 * 0 + 2] * m[4 * 1 + 1] * m[4 * 3 + 0]
            - m[4 * 0 + 1] * m[4 * 1 + 2] * m[4 * 3 + 0]
            - m[4 * 0 + 2] * m[4 * 1 + 0] * m[4 * 3 + 1]
            + m[4 * 0 + 0] * m[4 * 1 + 2] * m[4 * 3 + 1]
            + m[4 * 0 + 1] * m[4 * 1 + 0] * m[4 * 3 + 2]
            - m[4 * 0 + 0] * m[4 * 1 + 1] * m[4 * 3 + 2];
        a[4 * 3 + 3] = m[4 * 0 + 1] * m[4 * 1 + 2] * m[4 * 2 + 0]
            - m[4 * 0 + 2] * m[4 * 1 + 1] * m[4 * 2 + 0]
            + m[4 * 0 + 2] * m[4 * 1 + 0] * m[4 * 2 + 1]
            - m[4 * 0 + 0] * m[4 * 1 + 2] * m[4 * 2 + 1]
            - m[4 * 0 + 1] * m[4 * 1 + 0] * m[4 * 2 + 2]
            + m[4 * 0 + 0] * m[4 * 1 + 1] * m[4 * 2 + 2];

        det = NvGl2DemoMatrixDeterminant(m);

        for (i = 0; i < 16; ++i)
            a[i] /= det;

        NvGl2DemoMatrixIdentity(e);

        NvGl2DemoMatrixCopy(b, m);
        NvGl2DemoMatrixMultiply(b, a);

        NvGl2DemoMatrixCopy(m, a);
    }

    void NvGl2DemoMatrixCopy(float dest[16], float src[16])
    {
        memcpy(dest, src, 16 * sizeof(float));
    }

    void NvGl2DemoMatrixPrint(float a[16])
    {
        int i, j;

        for (i = 0; i < 4; ++i)
            for (j = 0; j < 4; ++j)
                printf("%f%c", a[4 * i + j], j == 3 ? '\n' : ' ');
    }

    void NvGl2DemoMatrixVectorMultiply(float m[16], float v[4])
    {
        float res[4];
        int i, j;

        for (i = 0; i < 4; ++i) {
            res[i] = 0;
            for (j = 0; j < 4; ++j)
                res[i] += m[i * 4 + j] * v[j];
        }

        memcpy(v, res, sizeof(res));
    }
}
}