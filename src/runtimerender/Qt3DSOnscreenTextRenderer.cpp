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

#include <Qt3DSTextRenderer.h>
#include <Qt3DSRenderTextureAtlas.h>
#include <Qt3DSContainers.h>
#include <Qt3DSFoundation.h>
#include <StrConvertUTF.h>
#include <qdemonrendercontext.h>
#include <QtGui/qpainter.h>
#include <QtGui/qimage.h>
#include <QtGui/qrawfont.h>
#include <QtCore/qfile.h>

using namespace render;

namespace {

struct STextureAtlasFontEntry
{
    STextureAtlasFontEntry()
        : m_x(0)
        , m_y(0)
        , m_width(0)
        , m_height(0)
        , m_xOffset(0)
        , m_yOffset(0)
        , m_advance(0)
        , m_s(0)
        , m_t(0)
        , m_s1(0)
        , m_t1(0)
    {
    }

    STextureAtlasFontEntry(float x, float y, float width, float height, float xoffset,
                           float yoffset, float advance, float s, float t, float s1, float t1)
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

    float m_x;
    float m_y;
    float m_width;
    float m_height;
    float m_xOffset;
    float m_yOffset;
    float m_advance;

    float m_s;
    float m_t;
    float m_s1;
    float m_t1;
};

typedef eastl::basic_string<char8_t, ForwardingAllocator> TStrType;
typedef nvhash_map<wchar_t, STextureAtlasFontEntry> TTextureAtlasMap;

struct STextAtlasFont
{
    NVFoundationBase &m_Foundation;
    volatile qint32 mRefCount;
    quint32 m_FontSize;
    TTextureAtlasMap m_AtlasEntries; ///< our entries in the atlas

    STextAtlasFont(NVFoundationBase &inFoundation, quint32 fontSize)
        : m_Foundation(inFoundation)
        , mRefCount(0)
        , m_FontSize(fontSize)
        , m_AtlasEntries(inFoundation.getAllocator(),
                         "Qt3DSOnscreenRenderer::STextAtlasFont::m_AtlasEntrys")
    {
    }

    ~STextAtlasFont() { m_AtlasEntries.clear(); }

    QDEMON_IMPLEMENT_REF_COUNT_ADDREF_RELEASE(m_Foundation.getAllocator())

    static STextAtlasFont &CreateTextureAtlasFont(NVFoundationBase &inFnd, quint32 fontSize)
    {
        return *QDEMON_NEW(inFnd.getAllocator(), STextAtlasFont)(inFnd, fontSize);
    }
};

// This class is only for rendering 2D screen aligned text
// it uses a predefined true type font and character set with various sizes
struct Qt3DSOnscreenTextRenderer : public ITextRenderer
{

    static const qint32 TEXTURE_ATLAS_DIM =
        256; // if you change this you need to adjust STextTextureAtlas size as well

private:
    NVFoundationBase &m_Foundation;
    QDemonScopedRefCounted<QDemonRenderContext> m_RenderContext;
    volatile qint32 mRefCount;
    bool m_TextureAtlasInitialized; ///< true if atlas is setup
    QDemonScopedRefCounted<ITextureAtlas> m_TextTextureAtlas;
    QDemonScopedRefCounted<STextAtlasFont> m_TextFont;
    QRawFont *m_font;
public:
    Qt3DSOnscreenTextRenderer(NVFoundationBase &inFoundation)
        : m_Foundation(inFoundation)
        , mRefCount(0)
        , m_TextureAtlasInitialized(false)
        , m_font(nullptr)
    {
    }

    virtual ~Qt3DSOnscreenTextRenderer()
    {
    }

    QDEMON_IMPLEMENT_REF_COUNT_ADDREF_RELEASE_OVERRIDE(m_Foundation.getAllocator())

    void AddSystemFontDirectory(const char8_t *) override {}

    virtual void AddProjectFontDirectory(CRegisteredString)
    {
        // We always render using the default font with on-screen renderer,
        // so no need to care about font directories
    }

    void AddProjectFontDirectory(const char8_t *inProjectDirectory) override
    {
        if (m_RenderContext)
            AddProjectFontDirectory(
                m_RenderContext->GetStringTable().RegisterStr(inProjectDirectory));
    }

    void loadFont()
    {
        // Ensure font. We can only render text of single size at the moment.
        // Add a size map of fonts if it ever becomes necessary to render multiple font sizes.
        if (!m_font)
            m_font = new QRawFont(QStringLiteral(":res/Font/TitilliumWeb-Regular.ttf"), 20.0);

        // setup texture atlas
        m_TextTextureAtlas =
            ITextureAtlas::CreateTextureAtlas(m_RenderContext->GetFoundation(), *m_RenderContext,
                                              TEXTURE_ATLAS_DIM, TEXTURE_ATLAS_DIM);

        // our list of predefined cached characters
        QString cache = QStringLiteral(" !\"#$%&'()*+,-./0123456789:;<=>?"
                                       "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
                                       "`abcdefghijklmnopqrstuvwxyz{|}~");

        m_TextFont = STextAtlasFont::CreateTextureAtlasFont(m_Foundation, 20);
        CreateTextureAtlasEntries(m_font, *m_TextFont, cache);
        m_TextureAtlasInitialized = true;
    }

    void CreateTextureAtlasEntries(QRawFont *rawFont, STextAtlasFont &font, const QString &cache)
    {
        if (m_TextureAtlasInitialized || !m_TextTextureAtlas || !rawFont)
            return;

        STextureAtlasRect theAtlasRect;

        QVector<quint32> glyphIndices = rawFont->glyphIndexesForString(cache);
        QVector<QPointF> glyphAdvances = rawFont->advancesForGlyphIndexes(glyphIndices);

        for (int i = 0; i < glyphIndices.size(); i++) {
            quint32 index = glyphIndices[i];
            QImage glyphImage;

            // blank char is not contained in a true type font
            if (cache.at(i) != QChar(' '))
                glyphImage = rawFont->alphaMapForGlyph(index, QRawFont::PixelAntialiasing);

            QRectF rect = rawFont->boundingRect(index);
            QDemonConstDataRef<quint8> bufferData(static_cast<const quint8 *>(glyphImage.bits()),
                                               glyphImage.byteCount());

            theAtlasRect = m_TextTextureAtlas->AddAtlasEntry(
                glyphImage.width(), glyphImage.height(),
                glyphImage.bytesPerLine(), glyphImage.width(), bufferData);

            if (theAtlasRect.m_Width != 0) {
                font.m_AtlasEntries.insert(
                            eastl::make_pair(
                                cache.at(i).unicode(), STextureAtlasFontEntry(
                                    (float)theAtlasRect.m_X, (float)theAtlasRect.m_Y,
                                    (float)theAtlasRect.m_Width, (float)theAtlasRect.m_Height,
                                    (float)rect.x(), (float)(0.0 - rect.height() - rect.y()),
                                    glyphAdvances[i].x(),
                                    theAtlasRect.m_NormX, theAtlasRect.m_NormY,
                                    theAtlasRect.m_NormX + theAtlasRect.m_NormWidth,
                                    theAtlasRect.m_NormY + theAtlasRect.m_NormHeight)));
            }
        }
    }

    void ClearProjectFontDirectories() override {}

    ITextRenderer &GetTextRenderer(QDemonRenderContext &inRenderContext) override
    {
        m_RenderContext = inRenderContext;
        return *this;
    }

    void PreloadFonts() override {}
    void ReloadFonts() override {}

    // unused
    QDemonConstDataRef<SRendererFontEntry> GetProjectFontList() override
    {
        Q_ASSERT(false);
        return QDemonConstDataRef<SRendererFontEntry>();
    }

    // unused
    Option<CRegisteredString> GetFontNameForFont(CRegisteredString) override
    {
        Q_ASSERT(false);
        return Empty();
    }

    // unused
    Option<CRegisteredString> GetFontNameForFont(const char8_t *) override
    {
        Q_ASSERT(false);
        return Empty();
    }

    // unused
    STextDimensions MeasureText(const STextRenderInfo &, float, const char8_t *) override
    {
        Q_ASSERT(false);
        return STextDimensions(0, 0);
    }

    // unused
    STextTextureDetails RenderText(const STextRenderInfo &, QDemonRenderTexture2D &) override
    {
        Q_ASSERT(false);
        return STextTextureDetails();
    }

    // unused
    STextTextureDetails RenderText(const STextRenderInfo &, QDemonRenderPathFontItem &,
                                           QDemonRenderPathFontSpecification &) override
    {
        Q_ASSERT(false);
        return STextTextureDetails();
    }

    SRenderTextureAtlasDetails RenderText(const STextRenderInfo &inText) override
    {
        IStringTable &theStringTable(m_RenderContext->GetStringTable());

        const wchar_t *wText = theStringTable.GetWideStr(inText.m_Text);
        quint32 length = (quint32)wcslen(wText);

        if (length) {
            STextAtlasFont *pFont = m_TextFont;
            const STextureAtlasFontEntry *pEntry;
            float x1, y1, x2, y2;
            float s, t, s1, t1;
            float advance = 0.0;
            // allocate buffer for all the vertex data we need
            // we construct triangles here
            // which means character count x 6 vertices x 5 floats
            float *vertexData =
                (float *)QDEMON_ALLOC(m_Foundation.getAllocator(),
                                        length * 6 * 5 * sizeof(float),
                                        "Qt3DSOnscreenTextRenderer");
            float *bufPtr = vertexData;
            if (vertexData) {
                for (size_t i = 0; i < length; ++i) {
                    pEntry = &pFont->m_AtlasEntries.find(wText[i])->second;

                    if (pEntry) {
                        x1 = advance + pEntry->m_xOffset;
                        x2 = x1 + pEntry->m_width * inText.m_ScaleX;
                        y1 = pEntry->m_yOffset;
                        y2 = y1 + pEntry->m_height * inText.m_ScaleY;

                        s = pEntry->m_s;
                        s1 = pEntry->m_s1;
                        t = pEntry->m_t;
                        t1 = pEntry->m_t1;
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

                        advance += pEntry->m_advance * inText.m_ScaleX;

                        bufPtr += 30;
                    }
                }

                m_TextTextureAtlas->RelaseEntries();

                return SRenderTextureAtlasDetails(length * 6,
                                                  toU8DataRef(vertexData, length * 6 * 5));
            }
        }

        return SRenderTextureAtlasDetails();
    }

    STextTextureAtlasEntryDetails RenderAtlasEntry(quint32 index,
                                                           QDemonRenderTexture2D &inTexture) override
    {
        if (m_TextTextureAtlas) {
            TTextureAtlasEntryAndBuffer theEntry = m_TextTextureAtlas->GetAtlasEntryByIndex(index);
            if (theEntry.first.m_Width) {
                inTexture.SetTextureData(theEntry.second, 0, theEntry.first.m_Width,
                                         theEntry.first.m_Height, QDemonRenderTextureFormats::Alpha8);
                inTexture.SetMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
                inTexture.SetMinFilter(QDemonRenderTextureMinifyingOp::Linear);
                inTexture.SetTextureWrapS(QDemonRenderTextureCoordOp::ClampToEdge);
                inTexture.SetTextureWrapT(QDemonRenderTextureCoordOp::ClampToEdge);
                STextureDetails theTextureDetails = inTexture.GetTextureDetails();
                return STextTextureAtlasEntryDetails(theTextureDetails.m_Width,
                                                     theTextureDetails.m_Height, theEntry.first.m_X,
                                                     theEntry.first.m_Y);
            }
        }

        return STextTextureAtlasEntryDetails();
    }
    qint32 CreateTextureAtlas() override
    {
        loadFont();

        qint32 count = 0;
        if (m_TextTextureAtlas)
            count = m_TextTextureAtlas->GetAtlasEntryCount();

        return count;
    }

    void BeginFrame() override {}
    void EndFrame() override {}
    void BeginPreloadFonts(IThreadPool &, IPerfTimer &) override {}
    void EndPreloadFonts() override {}
};
}

ITextRendererCore &ITextRendererCore::CreateOnscreenTextRenderer(NVFoundationBase &inFnd)
{
    return *QDEMON_NEW(inFnd.getAllocator(), Qt3DSOnscreenTextRenderer)(inFnd);
}

#endif // QDEMON_RENDER_ONSCREEN_TEXT
