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

#include <QtDemonRender/qdemonrenderrenderbuffer.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemon/qdemonutils.h>

QT_BEGIN_NAMESPACE


QDemonRenderRenderBuffer::Private::Private(const QDemonRef<QDemonRenderContext> &context, QDemonRenderRenderBufferFormat format)
    : context(context),
      backend(context->getBackend()),
      storageFormat(format),
      handle(nullptr)
{

}

QDemonRenderRenderBuffer::Private::~Private()
{
    if (handle)
        backend->releaseRenderbuffer(handle);
}

QDemonRenderRenderBuffer::QDemonRenderRenderBuffer(const QDemonRef<QDemonRenderContext> &context,
                                                   QDemonRenderRenderBufferFormat format, const QSize &size)
{
    if (size.isNull()) {
        qCCritical(INVALID_PARAMETER, "Invalid renderbuffer width or height");
        return;
    }

    d = new Private(context, format);
    setSize(size);
}

QDemonRenderRenderBuffer::~QDemonRenderRenderBuffer()
{
}

void QDemonRenderRenderBuffer::setSize(const QSize &size)
{
    qint32 maxWidth, maxHeight;
    d->size = size;

    // get max size and clamp to max value
    d->context->getMaxTextureSize(maxWidth, maxHeight);
    if (size.width() > maxWidth || size.height() > maxHeight) {
        qCCritical(INVALID_OPERATION, "Width or height is greater than max texture size (%d, %d)", maxWidth, maxHeight);
        d->size = QSize(qMin(d->size.width(), maxWidth), qMin(d->size.height(), maxHeight));
    }

    bool success = true;

    if (d->handle == nullptr)
        d->handle = d->backend->createRenderbuffer(d->storageFormat, d->size.width(), d->size.height());
    else
        success = d->backend->resizeRenderbuffer(d->handle, d->storageFormat, d->size.width(), d->size.height());

    if (d->handle == nullptr || !success) {
        // We could try smaller sizes
        Q_ASSERT(false);
        qCCritical(INTERNAL_ERROR,
                   "Unable to create render buffer %s, %dx%d",
                   toString(d->storageFormat),
                   d->size.width(),
                   d->size.height());
    }
}

QT_END_NAMESPACE
