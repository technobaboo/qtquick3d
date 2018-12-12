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

QDemonRenderPathFontSpecification::QDemonRenderPathFontSpecification(QSharedPointer<QDemonRenderContextImpl> context, const QString &fontName)
    : m_Context(context)
    , m_Backend(context->GetBackend())
    , m_NumFontGlyphs(0)
    , m_EmScale(2048) // 2048 is default true type scale
    , m_Type(QDemonRenderPathFormatType::UByte)
    , m_TransformType(QDemonRenderPathTransformType::Translate2D)
    , m_FontName(fontName)
{
}

QDemonRenderPathFontSpecification::~QDemonRenderPathFontSpecification()
{
    m_Context->ReleasePathFontSpecification(this);
}

void QDemonRenderPathFontSpecification::LoadPathGlyphs(const char *fontName,
                                                       QDemonRenderPathFormatType::Enum type)
{
    // check if we already created it
    if (m_NumFontGlyphs)
        return;

    m_Type = type;

    // create fonts based on the input
    m_PathRenderHandle = m_Backend->LoadPathGlyphsIndexedRange(
                QDemonRenderPathFontTarget::FileFont, fontName, QDemonRenderPathFontStyleFlags(), 0, m_EmScale,
                &m_NumFontGlyphs);

    // Fallback in case the previuos call fails
    // This is a no-op if the previous call succeeds
    // Note that sans is an inbuild driver font
    if (!m_PathRenderHandle) {
        m_PathRenderHandle = m_Backend->LoadPathGlyphsIndexedRange(
                    QDemonRenderPathFontTarget::SystemFont, "Arial", QDemonRenderPathFontStyleFlags(), 0,
                    m_EmScale, &m_NumFontGlyphs);
    }

    // we should have some glyphs
    Q_ASSERT(m_NumFontGlyphs);
}

void
QDemonRenderPathFontSpecification::StencilFillPathInstanced(QSharedPointer<QDemonRenderPathFontItem> inPathFontItem)
{
    const void *glyphIDs = inPathFontItem->GetGlyphIDs();
    const float *spacing = inPathFontItem->GetSpacing();
    if (!glyphIDs || !spacing || !inPathFontItem->GetGlyphsCount()) {
        Q_ASSERT(false || !inPathFontItem->GetGlyphsCount());
        return;
    }

    m_Backend->StencilFillPathInstanced(m_PathRenderHandle, inPathFontItem->GetGlyphsCount(),
                                        m_Type, glyphIDs, QDemonRenderPathFillMode::Fill, 0xFF,
                                        m_TransformType, spacing);
}

void QDemonRenderPathFontSpecification::CoverFillPathInstanced(QSharedPointer<QDemonRenderPathFontItem> inPathFontItem)
{
    const void *glyphIDs = inPathFontItem->GetGlyphIDs();
    const float *spacing = inPathFontItem->GetSpacing();
    if (!glyphIDs || !spacing || !inPathFontItem->GetGlyphsCount()) {
        Q_ASSERT(false || !inPathFontItem->GetGlyphsCount());
        return;
    }

    m_Backend->CoverFillPathInstanced(
                m_PathRenderHandle, inPathFontItem->GetGlyphsCount(), m_Type, glyphIDs,
                QDemonRenderPathCoverMode::BoundingBoxOfBoundingBox, m_TransformType, spacing);
}

quint32
QDemonRenderPathFontSpecification::getSizeofType(QDemonRenderPathFormatType::Enum type)
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

QSharedPointer<QDemonRenderPathFontSpecification>
QDemonRenderPathFontSpecification::CreatePathFontSpecification(QSharedPointer<QDemonRenderContextImpl> context,
                                                               const QString &fontName)
{
    Q_ASSERT(context->IsPathRenderingSupported());

    return QSharedPointer<QDemonRenderPathFontSpecification>(new QDemonRenderPathFontSpecification(context, fontName));
}
QT_END_NAMESPACE
