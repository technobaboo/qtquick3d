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
#ifndef QDEMON_RENDER_PATH_FONT_TEXT_H
#define QDEMON_RENDER_PATH_FONT_TEXT_H

#include <QtGui/QVector2D>

#include <QtDemonRender/qtdemonrenderglobal.h>
#include <QtDemonRender/qdemonrenderbackend.h>

QT_BEGIN_NAMESPACE

class QDemonRenderContextImpl;
class QDemonRenderPathFontSpecification;

class Q_DEMONRENDER_EXPORT QDemonRenderPathFontItem
{
public:
    QAtomicInt ref;
    /**
     * @brief constructor
     *
     * @return No return.
     */
    QDemonRenderPathFontItem();

    /// @QDemonRenderPathFontItem destructor
    ~QDemonRenderPathFontItem();

    /**
     * @brief Setup text
     *
     * @param[in] glyphCount		number of glyphs
     * @param[in] glyphIDs			array of glyhp ID's
     * @param[in] type				type ( byte, int,... )
     * @param[in] posArray			array of glyhp positions
     * @param[in] pixelBound		pixel boundary
     * @param[in] logicalBound		logical boundary
     * @param[in] emScale			true type scale
     *
     * @return No return.
     */
    void initTextItem(size_t glyphCount,
                      const quint32 *glyphIDs,
                      QDemonRenderPathFormatType type,
                      float *posArray,
                      QVector2D pixelBound,
                      QVector2D logicalBound,
                      float emScale);

    /**
     * @brief get glyph count
     *
     * @return get glyph count
     */
    size_t getGlyphsCount() { return m_numGlyphs; }

    /**
     * @brief get spacing for char set
     *
     * @return spacing array
     */
    const float *getSpacing() { return m_translateXY; }

    /**
     * @brief get name set
     *
     * @return name set
     */
    const void *getGlyphIDs() { return reinterpret_cast<void *>(m_glyphIDs); }

    /**
     * @brief Get Y bound of font metric
     *
     * @return transform matrix
     */
    const QMatrix4x4 getTransform();

private:
    /**
     * @brief Get size of type
     *
     * @param[in]  type						type ( byte, int,... )
     *
     * @return true if successful
     */
    quint32 getSizeOfType(QDemonRenderPathFormatType type);

private:
    size_t m_numGlyphs; ///< glyph count
    quint32 *m_glyphIDs; ///< array glyph ID's
    float *m_translateXY; ///< pointer to arrray for character advance information like kerning
    QMatrix4x4 m_modelMatrix; ///< Matrix which converts from font space to box space

public:
    static QDemonRef<QDemonRenderPathFontItem> createPathFontItem(const QDemonRef<QDemonRenderContextImpl> &context);
};

QT_END_NAMESPACE

#endif
