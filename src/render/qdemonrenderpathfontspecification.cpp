/****************************************************************************
**
** Copyright (C) 2015 NVIDIA Corporation.
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

#include <QtDemonRender/qdemonrenderpathfontspecification.h>
#include <QtDemonRender/qdemonrenderpathfonttext.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrendercontext.h>

QT_BEGIN_NAMESPACE

QDemonRenderPathFontSpecification::QDemonRenderPathFontSpecification(const QSharedPointer<QDemonRenderContextImpl> &context,
                                                                     const QString &fontName)
    : m_context(context)
    , m_backend(context->getBackend())
    , m_numFontGlyphs(0)
    , m_emScale(2048) // 2048 is default true type scale
    , m_type(QDemonRenderPathFormatType::UByte)
    , m_transformType(QDemonRenderPathTransformType::Translate2D)
    , m_fontName(fontName)
{
}

QDemonRenderPathFontSpecification::~QDemonRenderPathFontSpecification()
{
    m_context->releasePathFontSpecification(this);
}

void QDemonRenderPathFontSpecification::loadPathGlyphs(const char *fontName,
                                                       QDemonRenderPathFormatType::Enum type)
{
    // check if we already created it
    if (m_numFontGlyphs)
        return;

    m_type = type;

    // create fonts based on the input
    m_pathRenderHandle = m_backend->loadPathGlyphsIndexedRange(
                QDemonRenderPathFontTarget::FileFont, fontName, QDemonRenderPathFontStyleFlags(), 0, m_emScale,
                &m_numFontGlyphs);

    // Fallback in case the previuos call fails
    // This is a no-op if the previous call succeeds
    // Note that sans is an inbuild driver font
    if (!m_pathRenderHandle) {
        m_pathRenderHandle = m_backend->loadPathGlyphsIndexedRange(
                    QDemonRenderPathFontTarget::SystemFont, "Arial", QDemonRenderPathFontStyleFlags(), 0,
                    m_emScale, &m_numFontGlyphs);
    }

    // we should have some glyphs
    Q_ASSERT(m_numFontGlyphs);
}

void
QDemonRenderPathFontSpecification::stencilFillPathInstanced(const QSharedPointer<QDemonRenderPathFontItem> &inPathFontItem)
{
    const void *glyphIDs = inPathFontItem->getGlyphIDs();
    const float *spacing = inPathFontItem->getSpacing();
    if (!glyphIDs || !spacing || !inPathFontItem->getGlyphsCount()) {
        Q_ASSERT(false || !inPathFontItem->getGlyphsCount());
        return;
    }

    m_backend->stencilFillPathInstanced(m_pathRenderHandle,
                                        inPathFontItem->getGlyphsCount(),
                                        m_type, glyphIDs,
                                        QDemonRenderPathFillMode::Fill,
                                        0xFF,
                                        m_transformType,
                                        spacing);
}

void QDemonRenderPathFontSpecification::coverFillPathInstanced(const QSharedPointer<QDemonRenderPathFontItem> &inPathFontItem)
{
    const void *glyphIDs = inPathFontItem->getGlyphIDs();
    const float *spacing = inPathFontItem->getSpacing();
    if (!glyphIDs || !spacing || !inPathFontItem->getGlyphsCount()) {
        Q_ASSERT(false || !inPathFontItem->getGlyphsCount());
        return;
    }

    m_backend->coverFillPathInstanced(m_pathRenderHandle,
                                      inPathFontItem->getGlyphsCount(),
                                      m_type,
                                      glyphIDs,
                                      QDemonRenderPathCoverMode::BoundingBoxOfBoundingBox,
                                      m_transformType,
                                      spacing);
}

quint32
QDemonRenderPathFontSpecification::getSizeOfType(QDemonRenderPathFormatType::Enum type)
{
    switch (type) {
    case QDemonRenderPathFormatType::Byte:
        return sizeof(qint8);
    case QDemonRenderPathFormatType::UByte:
        return sizeof(quint8);
    case QDemonRenderPathFormatType::Bytes2:
        return sizeof(quint16);
    case QDemonRenderPathFormatType::Uint:
        return sizeof(quint32);
    case QDemonRenderPathFormatType::Utf8:
        return sizeof(quint32);
    default:
        Q_ASSERT(false);
        return 1;
    }
}

QSharedPointer<QDemonRenderPathFontSpecification> QDemonRenderPathFontSpecification::createPathFontSpecification(const QSharedPointer<QDemonRenderContextImpl> &context,
                                                                                                                 const QString &fontName)
{
    Q_ASSERT(context->isPathRenderingSupported());

    return QSharedPointer<QDemonRenderPathFontSpecification>(new QDemonRenderPathFontSpecification(context, fontName));
}
QT_END_NAMESPACE
