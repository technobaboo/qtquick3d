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
#define QDEMON_RENDER_ONSCREEN_TEXT
#ifdef QDEMON_RENDER_ONSCREEN_TEXT

#include "qdemontextrenderer.h"
#include "qdemonrendertextureatlas.h"

#include <QtDemonRender/qdemonrendercontext.h>

#include <QtGui/QPainter>
#include <QtGui/QImage>
#include <QtGui/QRawFont>
#include <QtCore/QFile>

QT_BEGIN_NAMESPACE

namespace {

struct QDemonTextureAtlasFontEntry
{
    float m_x = 0.0f;
    float m_y = 0.0f;
    float m_width = 0.0f;
    float m_height = 0.0f;
    float m_xOffset = 0.0f;
    float m_yOffset = 0.0f;
    float m_advance = 0.0f;

    float m_s = 0.0f;
    float m_t = 0.0f;
    float m_s1 = 0.0f;
    float m_t1 = 0.0f;

    QDemonTextureAtlasFontEntry() = default;
    QDemonTextureAtlasFontEntry(float x, float y, float width, float height, float xoffset, float yoffset, float advance, float s, float t, float s1, float t1)
        : m_x(x)
        , m_y(y)
        , m_width(width)
        , m_height(height)
        , m_xOffset(xoffset)
        , m_yOffset(yoffset)
        , m_advance(advance)
        , m_s(s)
        , m_t(t)
        , m_s1(s1)
        , m_t1(t1)
    {
    }
};

typedef QString TStrType;
typedef QHash<wchar_t, QDemonTextureAtlasFontEntry> TTextureAtlasMap;

struct QDemonTextAtlasFont
{
    QAtomicInt ref;
    quint32 m_fontSize;
    TTextureAtlasMap m_atlasEntries; ///< our entries in the atlas

    QDemonTextAtlasFont(quint32 fontSize) : m_fontSize(fontSize) {}

    static QDemonRef<QDemonTextAtlasFont> createTextureAtlasFont(quint32 fontSize)
    {
        return QDemonRef<QDemonTextAtlasFont>(new QDemonTextAtlasFont(fontSize));
    }
};

// This class is only for rendering 2D screen aligned text
// it uses a predefined true type font and character set with various sizes
struct QDemonOnscreenTextRenderer : public QDemonTextRendererInterface
{
    // if you change this you need to adjust STextTextureAtlas size as well
    static constexpr qint32 TEXTURE_ATLAS_DIM = 256;

private:
    QDemonRef<QDemonRenderContext> m_renderContext;
    bool m_textureAtlasInitialized = false; ///< true if atlas is setup
    QDemonRef<QDemonTextureAtlasInterface> m_textTextureAtlas;
    QDemonRef<QDemonTextAtlasFont> m_textFont;
    QRawFont *m_font = nullptr;

public:
    virtual ~QDemonOnscreenTextRenderer() override = default;

    void addSystemFontDirectory(const char *) override {}

    virtual void addProjectFontDirectory(const QString &)
    {
        // We always render using the default font with on-screen renderer,
        // so no need to care about font directories
    }

    void addProjectFontDirectory(const char *inProjectDirectory) override
    {
        if (m_renderContext)
            addProjectFontDirectory(QString::fromLocal8Bit(inProjectDirectory));
    }

    void loadFont()
    {
        // Ensure font. We can only render text of single size at the moment.
        // Add a size map of fonts if it ever becomes necessary to render multiple font sizes.
        if (!m_font)
            m_font = new QRawFont(QStringLiteral(":res/Font/TitilliumWeb-Regular.ttf"), 20.0);

        // setup texture atlas
        m_textTextureAtlas = QDemonTextureAtlasInterface::createTextureAtlas(m_renderContext, TEXTURE_ATLAS_DIM, TEXTURE_ATLAS_DIM);

        // our list of predefined cached characters
        QString cache = QStringLiteral(" !\"#$%&'()*+,-./0123456789:;<=>?"
                                       "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
                                       "`abcdefghijklmnopqrstuvwxyz{|}~");

        m_textFont = QDemonTextAtlasFont::createTextureAtlasFont(20);
        createTextureAtlasEntries(m_font, *m_textFont, cache);
        m_textureAtlasInitialized = true;
    }

    void createTextureAtlasEntries(QRawFont *rawFont, QDemonTextAtlasFont &font, const QString &cache)
    {
        if (m_textureAtlasInitialized || !m_textTextureAtlas || !rawFont)
            return;

        QDemonTextureAtlasRect theAtlasRect;

        QVector<quint32> glyphIndices = rawFont->glyphIndexesForString(cache);
        QVector<QPointF> glyphAdvances = rawFont->advancesForGlyphIndexes(glyphIndices);

        for (int i = 0; i < glyphIndices.size(); i++) {
            quint32 index = glyphIndices[i];
            QImage glyphImage;

            // blank char is not contained in a true type font
            if (cache.at(i) != QLatin1Char(' '))
                glyphImage = rawFont->alphaMapForGlyph(index, QRawFont::PixelAntialiasing);

            QRectF rect = rawFont->boundingRect(index);
            QDemonConstDataRef<quint8> bufferData(static_cast<const quint8 *>(glyphImage.bits()), glyphImage.sizeInBytes());

            theAtlasRect = m_textTextureAtlas->addAtlasEntry(glyphImage.width(),
                                                             glyphImage.height(),
                                                             glyphImage.bytesPerLine(),
                                                             glyphImage.width(),
                                                             bufferData);

            if (theAtlasRect.width != 0) {
                font.m_atlasEntries.insert(cache.at(i).unicode(),
                                           QDemonTextureAtlasFontEntry((float)theAtlasRect.x,
                                                                       (float)theAtlasRect.y,
                                                                       (float)theAtlasRect.width,
                                                                       (float)theAtlasRect.height,
                                                                       (float)rect.x(),
                                                                       (float)(0.0 - rect.height() - rect.y()),
                                                                       glyphAdvances[i].x(),
                                                                       theAtlasRect.normX,
                                                                       theAtlasRect.normY,
                                                                       theAtlasRect.normX + theAtlasRect.normWidth,
                                                                       theAtlasRect.normY + theAtlasRect.normHeight));
            }
        }
    }

    void clearProjectFontDirectories() override {}

    QDemonRef<QDemonTextRendererInterface> getTextRenderer(QDemonRef<QDemonRenderContext> inRenderContext) override
    {
        m_renderContext = inRenderContext;
        return this;
    }

    void preloadFonts() override {}
    void reloadFonts() override {}

    // unused
    QDemonConstDataRef<QDemonRendererFontEntry> getProjectFontList() override
    {
        Q_ASSERT(false);
        return QDemonConstDataRef<QDemonRendererFontEntry>();
    }

    // unused
    QDemonOption<QString> getFontNameForFont(QString) override
    {
        Q_ASSERT(false);
        return QDemonEmpty();
    }

    // unused
    QDemonOption<QString> getFontNameForFont(const char *) override
    {
        Q_ASSERT(false);
        return QDemonEmpty();
    }

    // unused
    QDemonTextDimensions measureText(const QDemonTextRenderInfo &, float, const char *) override
    {
        Q_ASSERT(false);
        return QDemonTextDimensions(0, 0);
    }

    // unused
    QDemonTextTextureDetails renderText(const QDemonTextRenderInfo &, QDemonRenderTexture2D &) override
    {
        Q_ASSERT(false);
        return QDemonTextTextureDetails();
    }

    // unused
    QDemonTextTextureDetails renderText(const QDemonTextRenderInfo &, QDemonRenderPathFontItem &, QDemonRenderPathFontSpecification &) override
    {
        Q_ASSERT(false);
        return QDemonTextTextureDetails();
    }

    QDemonRenderTextureAtlasDetails renderText(const QDemonTextRenderInfo &inText) override
    {
        //        const wchar_t *wText = theStringTable.GetWideStr(inText.m_Text);
        //        quint32 length = (quint32)wcslen(wText);
        // ### Fix this to not use w_char's and instead use 8bit values
        QByteArray wText = inText.text.toLocal8Bit();
        const int length = wText.size();

        if (length) {
            QDemonRef<QDemonTextAtlasFont> pFont = m_textFont;

            float x1, y1, x2, y2;
            float s, t, s1, t1;
            float advance = 0.0;
            // allocate buffer for all the vertex data we need
            // we construct triangles here
            // which means character count x 6 vertices x 5 floats
            float *vertexData = static_cast<float *>(::malloc(length * 6 * 5 * sizeof(float)));
            float *bufPtr = vertexData;
            if (vertexData) {
                for (int i = 0; i < length; ++i) {
                    if (!pFont->m_atlasEntries.contains(wText[i]))
                        continue;
                    const QDemonTextureAtlasFontEntry pEntry = pFont->m_atlasEntries.value(wText[i]);

                    x1 = advance + pEntry.m_xOffset;
                    x2 = x1 + pEntry.m_width * inText.scaleX;
                    y1 = pEntry.m_yOffset;
                    y2 = y1 + pEntry.m_height * inText.scaleY;

                    s = pEntry.m_s;
                    s1 = pEntry.m_s1;
                    t = pEntry.m_t;
                    t1 = pEntry.m_t1;
                    // store vertex data
                    bufPtr[0] = x1;
                    bufPtr[1] = y1;
                    bufPtr[2] = 0.0;
                    bufPtr[3] = s;
                    bufPtr[4] = t;
                    bufPtr[5] = x2;
                    bufPtr[6] = y1;
                    bufPtr[7] = 0.0;
                    bufPtr[8] = s1;
                    bufPtr[9] = t;
                    bufPtr[10] = x2;
                    bufPtr[11] = y2;
                    bufPtr[12] = 0.0;
                    bufPtr[13] = s1;
                    bufPtr[14] = t1;

                    bufPtr[15] = x1;
                    bufPtr[16] = y1;
                    bufPtr[17] = 0.0;
                    bufPtr[18] = s;
                    bufPtr[19] = t;
                    bufPtr[20] = x2;
                    bufPtr[21] = y2;
                    bufPtr[22] = 0.0;
                    bufPtr[23] = s1;
                    bufPtr[24] = t1;
                    bufPtr[25] = x1;
                    bufPtr[26] = y2;
                    bufPtr[27] = 0.0;
                    bufPtr[28] = s;
                    bufPtr[29] = t1;

                    advance += pEntry.m_advance * inText.scaleX;

                    bufPtr += 30;
                }

                m_textTextureAtlas->relaseEntries();

                return QDemonRenderTextureAtlasDetails(length * 6, toU8DataRef(vertexData, length * 6 * 5));
            }
        }

        return QDemonRenderTextureAtlasDetails();
    }

    QDemonTextTextureAtlasEntryDetails renderAtlasEntry(quint32 index, QDemonRenderTexture2D &inTexture) override
    {
        if (m_textTextureAtlas) {
            TTextureAtlasEntryAndBuffer theEntry = m_textTextureAtlas->getAtlasEntryByIndex(index);
            if (theEntry.first.width) {
                inTexture.setTextureData(theEntry.second, 0, theEntry.first.width, theEntry.first.height, QDemonRenderTextureFormats::Alpha8);
                inTexture.setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
                inTexture.setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
                inTexture.setTextureWrapS(QDemonRenderTextureCoordOp::ClampToEdge);
                inTexture.setTextureWrapT(QDemonRenderTextureCoordOp::ClampToEdge);
                QDemonTextureDetails theTextureDetails = inTexture.getTextureDetails();
                return QDemonTextTextureAtlasEntryDetails(theTextureDetails.width,
                                                          theTextureDetails.height,
                                                          theEntry.first.x,
                                                          theEntry.first.y);
            }
        }

        return QDemonTextTextureAtlasEntryDetails();
    }
    qint32 createTextureAtlas() override
    {
        loadFont();

        qint32 count = 0;
        if (m_textTextureAtlas)
            count = m_textTextureAtlas->getAtlasEntryCount();

        return count;
    }

    void beginFrame() override {}
    void endFrame() override {}
    void beginPreloadFonts(QDemonAbstractThreadPool &, QDemonRef<QDemonPerfTimerInterface>) override {}
    void endPreloadFonts() override {}
};
}

QDemonRef<QDemonTextRendererInterface> QDemonTextRendererInterface::createOnscreenTextRenderer()
{
    return QDemonRef<QDemonOnscreenTextRenderer>(new QDemonOnscreenTextRenderer());
}

QT_END_NAMESPACE

#endif // QDEMON_RENDER_ONSCREEN_TEXT
