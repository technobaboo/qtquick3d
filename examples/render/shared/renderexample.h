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
#ifndef RENDER_EXAMPLE_H
#define RENDER_EXAMPLE_H

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrendercontext.h>

#include <QtGui/QWindow>
#include <QtCore/QElapsedTimer>
#include <QtGui/QOpenGLContext>

class QDemonRenderContext;

class QDemonRenderExample : public QWindow
{
    Q_OBJECT
public:
    explicit QDemonRenderExample(QWindow *parent = nullptr);
    ~QDemonRenderExample() override;
    virtual void initialize() = 0;
    virtual void drawFrame(qint64 delta) = 0;

    void setAutoUpdate(bool autoUpdate) { m_autoUpdate = autoUpdate;}

public slots:
    void renderLater();
    void renderNow();

protected:
    bool event(QEvent *event) override;
    void exposeEvent(QExposeEvent *event) override;

private:
    void preInit();
    QElapsedTimer m_frameTimer;
    bool m_autoUpdate = true;
    bool m_isIntialized = false;
    QOpenGLContext *m_context;
};

#define eps 1e-4

int eq(float a, float b);

// Matrix functions
void QDemonGl2DemoMatrixIdentity(float m[16]);
int QDemonGl2DemoMatrixEquals(float a[16], float b[16]);
void QDemonGl2DemoMatrixTranspose(float m[16]);
void QDemonGl2DemoMatrixMultiply(float m0[16], float m1[16]);
void QDemonGl2DemoMatrixMultiply_4x4_3x3(float m0[16], float m1[9]);
void QDemonGl2DemoMatrixMultiply_3x3(float m0[9], float m1[9]);
void QDemonGl2DemoMatrixFrustum(float m[16], float l, float r, float b, float t, float n, float f);
void QDemonGl2DemoMatrixOrtho(float m[16], float l, float r, float b, float t, float n, float f);
void QDemonGl2DemoMatrixTranslate(float m[16], float x, float y, float z);
void QDemonGl2DemoMatrixRotate_create3x3(float m[9], float theta, float x, float y, float z);
void QDemonGl2DemoMatrixRotate(float m[16], float theta, float x, float y, float z);
void QDemonGl2DemoMatrixRotate_3x3(float m[9], float theta, float x, float y, float z);

float QDemonGl2DemoMatrixDeterminant(float m[16]);
void QDemonGl2DemoMatrixInverse(float m[16]);
void QDemonGl2DemoMatrixCopy(float dest[16], float src[16]);

void QDemonGl2DemoMatrixPrint(float a[16]);

void QDemonGl2DemoMatrixVectorMultiply(float m[16], float v[4]);

#endif
