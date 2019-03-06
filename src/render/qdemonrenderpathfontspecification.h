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
#ifndef QDEMON_RENDER_PATH_FONT_SPECIFICATION_H
#define QDEMON_RENDER_PATH_FONT_SPECIFICATION_H

#include <QtDemonRender/qtdemonrenderglobal.h>
#include <QVector2D>
#include <QtDemonRender/qdemonrenderbackend.h>

#include <QtCore/QString>

QT_BEGIN_NAMESPACE

class QDemonRenderContextImpl;
class QDemonRenderPathRender;
class QDemonRenderPathFontItem;

class Q_DEMONRENDER_EXPORT QDemonRenderPathFontSpecification
{
    Q_DISABLE_COPY(QDemonRenderPathFontSpecification)
public:
    QAtomicInt ref;

private:
    QDemonRef<QDemonRenderContextImpl> m_context; ///< pointer to context
    QDemonRef<QDemonRenderBackend> m_backend; ///< pointer to backend

public:
    /**
     * @brief constructor
     *
     * @param[in] context		Pointer to render context
     * @param[in] fnd			Pointer to foundation
     * @param[in]  fontName					Name of font ( may include path
     * )
     *
     * @return No return.
     */
    QDemonRenderPathFontSpecification(const QDemonRef<QDemonRenderContextImpl> &context, const QString &fontName);

    /// @QDemonRenderPathSpecification destructor
    virtual ~QDemonRenderPathFontSpecification();

    /**
     * @brief Load numGlyphs glyphs from specified font file
     *
     * @param[in]  pathBase					Base of path objects
     * @param[in]  fontName					Name of font ( may include path
     * )
     * @param[in]  numGlyphs				Glyph count
     * @param[in]  type						type ( byte, int,... )
     * @param[in]  charCodes				character string
     *
     * @return No return
     */
    virtual void loadPathGlyphs(const char *fontName, QDemonRenderPathFormatType type);

    /**
     * @brief Render a stencil fill pass for fonts
     *
     * @param[in] inPathFontSpec		Pointer to QDemonRenderPathFontSpecification
     *
     * @return no return
     */
    void stencilFillPathInstanced(const QDemonRef<QDemonRenderPathFontItem> &inPathFontItem);

    /**
     * @brief Render a cover fill pass for fonts
     *
     * @param[in] inPathFontSpec		Pointer to QDemonRenderPathFontSpecification
     *
     * @return no return
     */
    void coverFillPathInstanced(const QDemonRef<QDemonRenderPathFontItem> &inPathFontItem);

    /**
     * @brief get type for font path set
     *
     * @return path font type
     */
    QDemonRenderPathFormatType getPathFontType() { return m_type; }

    /**
     * @brief get font glyph count
     *
     * @return get glyph count
     */
    quint32 getFontGlyphsCount() { return m_numFontGlyphs; }

    /**
     * @brief get spacing for char set
     *
     * @return spacing array
     */
    float getEmScale() const { return m_emScale; }

    /**
     * @brief Get font name
     *
     * @return name set
     */
    QString getFontName() const { return m_fontName; }

private:
    quint32 m_numFontGlyphs; ///< glyph count of the entire font set
    float m_emScale; ///< true type scale
    QDemonRenderPathFormatType m_type; ///< type ( byte, int,... )
    QDemonRenderPathTransformType::Enum m_transformType; ///< transform type default 2D
    QString m_fontName; ///< Name of Font
    QDemonRenderBackend::QDemonRenderBackendPathObject m_pathRenderHandle; ///< opaque backend handle

private:
    /**
     * @brief Get size of type
     *
     * @param[in]  type						type ( byte, int,... )
     *
     * @return true if successful
     */
    quint32 getSizeOfType(QDemonRenderPathFormatType type);

public:
    static QDemonRef<QDemonRenderPathFontSpecification> createPathFontSpecification(const QDemonRef<QDemonRenderContextImpl> &context,
                                                                                    const QString &fontName);
};

QT_END_NAMESPACE

#endif
