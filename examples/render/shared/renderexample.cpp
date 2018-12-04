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
#include "renderexample.h"
#include <QtDemonRender/qdemonrenderbasetypes.h>

#include <QtCore/QElapsedTimer>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#include <QtGui/QSurfaceFormat>

static TExampleCreateFunc gCreateFuncs[128];

TExampleCreateFunc *QDemonRenderExampleFactory::mExampleCreators = gCreateFuncs;
quint32 QDemonRenderExampleFactory::mMaxCreators = 128;
quint32 QDemonRenderExampleFactory::mNumCreators = 0;
namespace {
QDemonRenderContext *g_renderContext(nullptr);
QDemonRenderExample *g_example(nullptr);
qint32 g_exampleIdx(0);
quint64 g_runTime;
QElapsedTimer g_timer;
}

bool QDemonRenderExampleFactory::nextExample()
{
    if (g_example)
        g_example->release();
    g_example = 0;
    if (g_exampleIdx < (int)mNumCreators) {
        g_example = mExampleCreators[g_exampleIdx](*g_renderContext);
        ++g_exampleIdx;
        g_runTime = g_example->getRuntimeInSeconds();
        g_timer.restart();
        return true;
    }
    return false;
}

void QDemonRenderExampleFactory::beginExamples()
{
    g_renderContext = &QDemonRenderContext::CreateGL(format());
    nextExample();
}
bool QDemonRenderExampleFactory::update()
{
    qint64 currentTime = g_timer.elapsed();
    if (currentTime > g_runTime)
        nextExample();
    if (g_example)
        g_example->drawFrame(currentTime);

    return g_example != NULL;
}

void QDemonRenderExampleFactory::endExamples()
{
    g_exampleIdx = mNumCreators;
    nextExample();
    delete g_renderContext;
    g_renderContext = nullptr;
}

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

void QDemonGl2DemoMatrixIdentity(float m[16])
{
    memset(m, 0, sizeof(float) * 16);
    m[4 * 0 + 0] = m[4 * 1 + 1] = m[4 * 2 + 2] = m[4 * 3 + 3] = 1.0;
}

int QDemonGl2DemoMatrixEquals(float a[16], float b[16])
{
    int i;
    for (i = 0; i < 16; ++i)
        if (!eq(a[i], b[i]))
            return 0;

    return 1;
}

void QDemonGl2DemoMatrixTranspose(float m[16])
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

void QDemonGl2DemoMatrixMultiply(float m0[16], float m1[16])
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

void QDemonGl2DemoMatrixMultiply_4x4_3x3(float m0[16], float m1[9])
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

void QDemonGl2DemoMatrixMultiply_3x3(float m0[9], float m1[9])
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

void QDemonGl2DemoMatrixFrustum(float m[16], float l, float r, float b, float t, float n, float f)
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

    QDemonGl2DemoMatrixMultiply(m, m1);
}

void QDemonGl2DemoMatrixOrtho(float m[16], float l, float r, float b, float t, float n, float f)
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

    QDemonGl2DemoMatrixMultiply(m, m1);
}

void QDemonGl2DemoMatrixTranslate(float m[16], float x, float y, float z)
{
    float m1[16];
    QDemonGl2DemoMatrixIdentity(m1);

    m1[4 * 3 + 0] = x;
    m1[4 * 3 + 1] = y;
    m1[4 * 3 + 2] = z;

    QDemonGl2DemoMatrixMultiply(m, m1);
}

void QDemonGl2DemoMatrixRotate_create3x3(float m[9], float theta, float x, float y, float z)
{
    float len = (float)sqrtf(x * x + y * y + z * z);
    float u0 = x / len;
    float u1 = y / len;
    float u2 = z / len;
    float rad = (float)(theta / 180 * M_PI);
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

void QDemonGl2DemoMatrixRotate(float m[16], float theta, float x, float y, float z)
{
    float r[9];
    QDemonGl2DemoMatrixRotate_create3x3(r, theta, x, y, z);
    QDemonGl2DemoMatrixMultiply_4x4_3x3(m, r);
}

void QDemonGl2DemoMatrixRotate_3x3(float m[9], float theta, float x, float y, float z)
{
    float r[9];
    QDemonGl2DemoMatrixRotate_create3x3(r, theta, x, y, z);
    QDemonGl2DemoMatrixMultiply_3x3(m, r);
}

float QDemonGl2DemoMatrixDeterminant(float m[16])
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

void QDemonGl2DemoMatrixInverse(float m[16])
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

    det = QDemonGl2DemoMatrixDeterminant(m);

    for (i = 0; i < 16; ++i)
        a[i] /= det;

    QDemonGl2DemoMatrixIdentity(e);

    QDemonGl2DemoMatrixCopy(b, m);
    QDemonGl2DemoMatrixMultiply(b, a);

    QDemonGl2DemoMatrixCopy(m, a);
}

void QDemonGl2DemoMatrixCopy(float dest[16], float src[16])
{
    memcpy(dest, src, 16 * sizeof(float));
}

void QDemonGl2DemoMatrixPrint(float a[16])
{
    int i, j;

    for (i = 0; i < 4; ++i)
        for (j = 0; j < 4; ++j)
            printf("%f%c", a[4 * i + j], j == 3 ? '\n' : ' ');
}

void QDemonGl2DemoMatrixVectorMultiply(float m[16], float v[4])
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
