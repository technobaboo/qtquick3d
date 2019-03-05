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

#include <QtGui/QMatrix4x4>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>
#include <QtDemonRender/qdemonrenderprogrampipeline.h>
#include <QtDemon/qdemondataref.h>
#include <QtDemon/qdemonutils.h>

#include <QtCore/QSharedPointer>

QT_BEGIN_NAMESPACE

QDemonRef<QDemonRenderContext> QDemonRenderContext::createGl(const QSurfaceFormat &format)
{
    QDemonRef<QDemonRenderContext> retval;

    Q_ASSERT(format.majorVersion() >= 2);

    // create backend
    QDemonRef<QDemonRenderBackend> theBackend;
    bool isES = format.renderableType() == QSurfaceFormat::OpenGLES;
    if (isES && (format.majorVersion() == 2 || (format.majorVersion() == 3 && format.minorVersion() == 0))) {
        theBackend = new QDemonRenderBackendGLES2Impl(format);
    } else if (format.majorVersion() == 3 && format.minorVersion() >= 1 && !isES) {
        theBackend = new QDemonRenderBackendGL3Impl(format);
    } else if (format.majorVersion() == 4 || (isES && format.majorVersion() == 3 && format.minorVersion() >= 1)) {
#ifdef Q_OS_MACOS
        // TODO: macOS crashes with glTextStorage2DMultisample, so fall back to OpenGL3
        // for now (QDEMON-590)
        theBackend = new QDemonRenderBackendGL3Impl(format);
#else
        theBackend = new QDemonRenderBackendGL4Impl(format);
#endif
    } else {
        Q_ASSERT(false);
        qCCritical(INTERNAL_ERROR) << "Can't find a suitable OpenGL version for" << format;
    }

    QDemonRef<QDemonRenderContextImpl> impl(new QDemonRenderContextImpl(theBackend));
    retval = impl;

    return retval;
}

QT_END_NAMESPACE
