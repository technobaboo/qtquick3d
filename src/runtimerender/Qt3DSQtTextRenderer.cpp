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

#include <Qt3DSTextRenderer.h>
#include <Qt3DSRenderText.h>
#include <Qt3DSFoundation.h>
#include <StringTable.h>
#include <Qt3DSBroadcastingAllocator.h>
#include <QVector2D.h>
#include <FileTools.h>
#include <qdemonrendercontext.h>
#include <Qt3DSContainers.h>
#include <Qt3DSRenderThreadPool.h>
#include <Qt3DSSync.h>
#include <Qt3DSPerfTimer.h>

#include <QPainter>
#include <QImage>
#include <QFontDatabase>
#include <QDir>
#include <QDebug>
#include <QHash>
#include <QGuiApplication>
#include <QtMath>
#include <QRawFont>

using namespace render;

namespace {

struct Qt3DSQtTextRenderer : public ITextRenderer
{
    struct FontInfo
    {
        QString fontFileName;
        QString fontName;
        QString fontFamily;
        int fontId;
        QFont font;

        FontInfo() :
            fontId(-1)
        {}

        FontInfo(const QString &fileName, const QString &name, const QString &family, int id) :
            fontFileName(fileName)
            , fontName(name)
            , fontFamily(family)
            , fontId(id)
        {
            font.setFamily(fontFamily);
        }

        FontInfo(const FontInfo &other) :
            fontFileName(other.fontFileName)
            , fontName(other.fontName)
            , fontFamily(other.fontFamily)
            , fontId(other.fontId)
            , font(other.font)
        {}

        FontInfo &operator=(const FontInfo &other)
        {
            fontFileName = other.fontFileName;
            fontName = other.fontName;
            fontFamily = other.fontFamily;
            fontId = other.fontId;
            font = other.font;

            return *this;
        }
    };

    typedef QString TStrType;
    typedef eastl::set<TStrType> TStringSet;
    typedef QHash<QString, FontInfo> TFontInfoHash;

    NVFoundationBase &m_foundation;
    QDemonScopedRefCounted<IStringTable> m_stringTable;
    QDemonScopedRefCounted<QDemonRenderContext> m_renderContext;
    QDemonScopedRefCounted<IPerfTimer> m_perfTimer;
    volatile qint32 mRefCount;
    nvvector<SRendererFontEntry> m_installedFonts;

    Sync m_PreloadSync;

    TStringSet m_systemFontDirs;
    TStringSet m_projectFontDirs;
    TFontInfoHash m_projectFontInfos;
    TFontInfoHash m_systemFontInfos;
    TStrType m_workspace;

    bool m_systemFontsInitialized;
    bool m_projectFontsInitialized;
    bool m_PreloadingFonts;

    QStringList m_nameFilters;
    qreal m_pixelRatio;

    Qt3DSQtTextRenderer(NVFoundationBase &inFoundation, IStringTable &inStrTable)
        : m_foundation(inFoundation)
        , m_stringTable(inStrTable)
        , mRefCount(0)
        , m_installedFonts(inFoundation.getAllocator(), "Qt3DSQtTextRenderer::m_installedFonts")
        , m_PreloadSync(inFoundation.getAllocator())
        , m_systemFontsInitialized(false)
        , m_projectFontsInitialized(false)
        , m_PreloadingFonts(false)
        , m_pixelRatio(1.0)
    {
        const QWindowList list = QGuiApplication::topLevelWindows();
        if (list.size() > 0)
            m_pixelRatio = list[0]->devicePixelRatio();

        m_nameFilters << QStringLiteral("*.ttf");
        m_nameFilters << QStringLiteral("*.otf");
    }
    virtual ~Qt3DSQtTextRenderer()
    {
        QFontDatabase::removeAllApplicationFonts();
    }

    QString stringToQString(const CRegisteredString &str)
    {
        return QString::fromUtf8(str.c_str());
    }

    QString stringToQString(const QString &str)
    {
        return QString::fromUtf8(str.c_str());
    }

    QString stringToQString(const char8_t *str)
    {
        return QString::fromUtf8(str);
    }

    CRegisteredString QStringToRegisteredString(const QString &str)
    {
        return m_stringTable->RegisterStr(str.toUtf8().constData());
    }

    void unregisterProjectFonts()
    {
        for (FontInfo &fi : m_projectFontInfos.values())
            QFontDatabase::removeApplicationFont(fi.fontId);
        m_projectFontsInitialized = false;
        m_installedFonts.clear();
        m_projectFontInfos.clear();
    }

    QString getFileStem(const QString &fileName)
    {
        QString retVal;
        int dotPos = fileName.lastIndexOf(QChar('.'));
        if (dotPos < 0)
            return retVal;
        int slashPos = fileName.lastIndexOf(QChar('/'));
        retVal = fileName.mid(slashPos + 1);
        retVal.chop(fileName.length() - dotPos);
        return retVal;
    }

    void registerFonts(TStringSet dirSet, TFontInfoHash *fontInfos = nullptr)
    {
        for (TStringSet::const_iterator theIter = dirSet.begin(),
            theEnd = dirSet.end();
            theIter != theEnd; ++theIter) {
            QString localDir = CFileTools::NormalizePathForQtUsage(stringToQString(*theIter));
            QDir dir(localDir);
            if (!dir.exists()) {
                qCCritical(INTERNAL_ERROR) << "Adding font directory:" << localDir;
                continue;
            }
            if (fontInfos)
                dir.cd(QStringLiteral("fonts"));
            QStringList entryList = dir.entryList(m_nameFilters);
            for (QString entry : entryList) {
                entry = dir.absoluteFilePath(entry);
                QFile file(entry);
                if (file.open(QIODevice::ReadOnly)) {
                    QByteArray rawData = file.readAll();
                    int fontId = QFontDatabase::addApplicationFontFromData(rawData);
                    if (fontId < 0) {
                        qCWarning(WARNING, "Failed to register font: %s",
                            entry.toStdString().c_str());
                    } else if (fontInfos) {
                        QString fontName = getFileStem(entry);
                        QString fontFamily;
                        QStringList families = QFontDatabase::applicationFontFamilies(fontId);
                        if (families.size() > 0)
                            fontFamily = families.at(0);
                        FontInfo fi(entry, fontName, fontFamily, fontId);
                        // Detect font style and weight using a dummy QRawFont
                        QRawFont rawFont(rawData, 1.0);
                        if (rawFont.isValid()) {
                            if (rawFont.style() != QFont::StyleOblique) {
                                fi.font.setStyle(rawFont.style());
                                fi.font.setWeight(rawFont.weight());
                            }
                        } else {
                            qCWarning(WARNING, "Failed to determine font style: %s",
                                entry.toStdString().c_str());
                        }
                        fontInfos->insert(fontName, fi);
                    }
                } else {
                    qCWarning(WARNING, "Failed to load font: %s",
                        entry.toStdString().c_str());
                }
            }
        }
    }

    void projectCleanup()
    {
        m_projectFontsInitialized = false;
        unregisterProjectFonts();
        m_projectFontDirs.clear();
    }

    QDEMON_IMPLEMENT_REF_COUNT_ADDREF_RELEASE_OVERRIDE(m_foundation.getAllocator())

    eastl::pair<TStrType, bool> AddFontDirectory(const TStrType &inDirectory, TStringSet &inDirSet)
    {
        if (inDirectory.empty()) {
            m_workspace.assign("./");
        } else {
            m_workspace.clear();
            for (const char8_t *item = inDirectory.c_str(); item && *item; ++item) {
                if (*item == '\\')
                    m_workspace.append(1, '/');
                else
                    m_workspace.append(1, static_cast<char8_t>(*item));
            }
            if (m_workspace.back() != '/')
                m_workspace.append(1, '/');
        }

        return eastl::make_pair(m_workspace, inDirSet.insert(m_workspace).second);
    }

    // You can have several standard font directories and these will be persistent
    void AddSystemFontDirectory(const char8_t *inDirectory) override
    {
        AddFontDirectory(inDirectory, m_systemFontDirs);
    }

    void AddProjectFontDirectory(const char8_t *inProjectDirectory) override
    {
        eastl::pair<TStrType, bool> theAddResult =
            AddFontDirectory(inProjectDirectory, m_projectFontDirs);
        if (theAddResult.second && m_projectFontsInitialized)
            ReloadFonts();
    }

    void ReloadFonts() override
    {
        unregisterProjectFonts();
        PreloadFonts();
    }

    void PreloadFonts() override
    {
        if (!m_systemFontsInitialized) {
            m_systemFontsInitialized = true;
            registerFonts(m_systemFontDirs, &m_systemFontInfos);
        }

        if (!m_projectFontsInitialized) {
            m_projectFontsInitialized = true;
            registerFonts(m_projectFontDirs, &m_projectFontInfos);
        }
    }

    void ClearProjectFontDirectories() override
    {
        projectCleanup();
    }

    static void PreloadThreadCallback(void *inData)
    {
        Qt3DSQtTextRenderer *theRenderer(reinterpret_cast<Qt3DSQtTextRenderer *>(inData));
        theRenderer->PreloadFonts();
        theRenderer->m_PreloadSync.set();
    }

    void BeginPreloadFonts(IThreadPool &inThreadPool, IPerfTimer &inTimer) override
    {
        m_PreloadingFonts = true;

        m_PreloadSync.reset();
        m_perfTimer = inTimer;

        inThreadPool.AddTask(this, PreloadThreadCallback, nullptr);
    }

    void EndPreloadFonts() override
    {
        if (m_PreloadingFonts) {
            {
                SStackPerfTimer __perfTimer(*m_perfTimer, "QtText: Wait till font preloading completed");
                m_PreloadSync.wait();
            }
        }
        m_PreloadingFonts = false;
    }

    // Get the list of project fonts. These are the only fonts that can be displayed.
    QDemonConstDataRef<SRendererFontEntry> GetProjectFontList() override
    {
        PreloadFonts();
        if (m_installedFonts.empty()) {
            m_installedFonts.reserve(m_projectFontInfos.size());
            for (FontInfo &fi : m_projectFontInfos.values()) {
                m_installedFonts.push_back(SRendererFontEntry(
                    QStringToRegisteredString(fi.fontName),
                    QStringToRegisteredString(fi.fontFileName)));
            }
        }
        return m_installedFonts;
    }

    Option<CRegisteredString> GetFontNameForFont(CRegisteredString inFontname) override
    {
        // This function is there to support legacy font names.

        QString inStr = stringToQString(inFontname);
        if (m_projectFontInfos.keys().contains(inStr))
             return inFontname;

        // Fall back for family name detection if not found by font name
        for (FontInfo &fi : m_projectFontInfos.values()) {
            if (inStr == fi.fontFamily)
                return QStringToRegisteredString(fi.fontName);
        }

        return Empty();
    }

    Option<CRegisteredString> GetFontNameForFont(const char8_t *inFontname) override
    {
        return GetFontNameForFont(m_stringTable->RegisterStr(inFontname));
    }

    ITextRenderer &GetTextRenderer(QDemonRenderContext &inRenderContext) override
    {
        m_renderContext = inRenderContext;
        return *this;
    }

    FontInfo &fontInfoForName(const CRegisteredString &fontName)
    {
        PreloadFonts();
        QString qtFontName = stringToQString(fontName);
        if (m_projectFontInfos.contains(qtFontName))
            return m_projectFontInfos[qtFontName];

        if (m_systemFontInfos.contains(qtFontName))
            return m_systemFontInfos[qtFontName];

        // Unknown font, create a system font for it
        FontInfo fi("", qtFontName, qtFontName, -1);
        m_systemFontInfos.insert(qtFontName, fi);

        return m_systemFontInfos[qtFontName];
    }

    void updateFontInfo(FontInfo &fi, const STextRenderInfo &inText,
                        float inTextScaleFactor = 1.0f)
    {
        qreal pixelSize = inText.m_FontSize;
        fi.font.setPixelSize(pixelSize * inTextScaleFactor);
        fi.font.setLetterSpacing(QFont::AbsoluteSpacing, qreal(inText.m_Tracking));
    }

    QStringList splitText(const char8_t *theText)
    {
        // Split the text into lines
        int lines = 1;
        int lineLen = 0;
        QStringList lineList;
        const char8_t *lineStartItem = nullptr;
        for (const char8_t *item = theText; item && *item; ++item) {
            if (!lineLen)
                lineStartItem = item;
            ++lineLen;
            if (*item == '\n') {
                int chopAmount = 1;
                if (lineLen > 1 && *(item - 1) == '\r')
                    ++chopAmount;

                ++lines;
                lineList.append(QString::fromUtf8(lineStartItem, lineLen - chopAmount));
                lineLen = 0;
            }
        }
        if (lineStartItem)
            lineList.append(QString::fromUtf8(lineStartItem, lineLen));

        return lineList;
    }

    QRectF textBoundingBox(const STextRenderInfo &inText,
        const QFontMetricsF &fm, QStringList &lineList,
        QVector<qreal> &lineWidths, const char8_t *inTextOverride = nullptr)
    {
        const char8_t *theText = inTextOverride ? inTextOverride : inText.m_Text.c_str();
        lineList = splitText(theText);

        QRectF boundingBox;
        boundingBox.setHeight(lineList.size() * fm.height() + qCeil(qreal(lineList.size() - 1) * qreal(inText.m_Leading)));

        lineWidths.resize(lineList.size());

        for (int i = 0; i < lineList.size(); ++i) {
            // For italicized fonts the bounding box right is the correct method
            // to measure since the left offset may extend, but for
            // non-italicized fonts we need the width method to meausure
            // otherwise the resultant text will be clipped.
            QString line = lineList.at(i);
            qreal width = fm.width(line);
            qreal right = fm.boundingRect(line).right();
            // For hdpi displays, fontmetrics doesn't always calculate enough space for fonts, so
            // we add the pixel ratio to all widths to avoid clipping
            qreal lineWidth = qMax(width, right) + m_pixelRatio;
            lineWidths[i] = lineWidth;
            if (boundingBox.width() < lineWidth)
                boundingBox.setWidth(lineWidth);
        }

        // We don't want extra letter spacing on the last glyph, so let's remove it
        boundingBox.setRight(qMax(boundingBox.left(), boundingBox.right() - qFloor(inText.m_Tracking)));

        return boundingBox;
    }

    STextDimensions MeasureText(const STextRenderInfo &inText, float inTextScaleFactor,
                                const char8_t *inTextOverride) override
    {
        FontInfo &fi = fontInfoForName(inText.m_Font);
        updateFontInfo(fi, inText, inTextScaleFactor);
        QFontMetricsF fm(fi.font);
        QStringList dummyList;
        QVector<qreal> dummyWidth;
        QRectF boundingBox = textBoundingBox(inText, fm, dummyList, dummyWidth, inTextOverride);
        return STextDimensions(boundingBox.width(), boundingBox.height());
    }

    int alignToQtAlign(TextVerticalAlignment::Enum va)
    {
        int qtAlign(0);
        switch (va) {
        case TextVerticalAlignment::Top:
            qtAlign = Qt::AlignTop;
            break;
        case TextVerticalAlignment::Bottom:
            qtAlign = Qt::AlignBottom;
            break;
        default:
            qtAlign = Qt::AlignVCenter;
        }

        return qtAlign;
    }

    STextTextureDetails RenderText(const STextRenderInfo &inSrcText,
                                   QDemonRenderTexture2D &inTexture) override
    {
        FontInfo &fi = fontInfoForName(inSrcText.m_Font);
        updateFontInfo(fi, inSrcText);
        QFontMetricsF fm(fi.font);

        int shadowRgb = int(2.55f * (100 - int(inSrcText.m_DropShadowStrength)));
        QStringList lineList;
        QVector<qreal> lineWidths;
        QRectF boundingBox = textBoundingBox(inSrcText, fm, lineList, lineWidths);

        if (boundingBox.width() <= 0 || boundingBox.height() <= 0) {
            return ITextRenderer::UploadData(toU8DataRef((char *)nullptr, 0), inTexture, 4, 4,
                                             0, 0,
                                             QDemonRenderTextureFormats::RGBA8, true);
        }

        int finalWidth = NextMultipleOf4(boundingBox.width());
        int finalHeight = NextMultipleOf4(boundingBox.height());

        QImage image(finalWidth, finalHeight, QImage::Format_ARGB32);
        image.fill(0);
        QPainter painter(&image);
        painter.setPen(Qt::white);
        painter.setFont(fi.font);

        // Translate painter to remove the extra spacing of the last letter
        qreal tracking = 0.0;
        switch (inSrcText.m_HorizontalAlignment) {
        case TextHorizontalAlignment::Center:
            tracking += qreal(inSrcText.m_Tracking / 2.0f);
            break;
        case TextHorizontalAlignment::Right:
            tracking += qreal(inSrcText.m_Tracking);
            break;
        default:
            break; // Do nothing
        }

        qreal shadowOffsetX = 0.;
        qreal shadowOffsetY = 0.;
        if (inSrcText.m_DropShadow) {
            const qreal offset = qreal(inSrcText.m_DropShadowOffset) / 10.;
            switch (inSrcText.m_DropShadowHorizontalAlignment) {
            case TextHorizontalAlignment::Left:
                shadowOffsetX = -offset;
                break;
            case TextHorizontalAlignment::Right:
                shadowOffsetX = offset;
                break;
            default:
                break;
            }
            switch (inSrcText.m_DropShadowVerticalAlignment) {
            case TextVerticalAlignment::Top:
                shadowOffsetY = -offset;
                break;
            case TextVerticalAlignment::Bottom:
                shadowOffsetY = offset;
                break;
            default:
                break;
            }
        }

        int lineHeight = fm.height();
        float nextHeight = 0;
        for (int i = 0; i < lineList.size(); ++i) {
            const QString &line = lineList.at(i);
            qreal xTranslation = tracking;
            switch (inSrcText.m_HorizontalAlignment) {
            case TextHorizontalAlignment::Center:
                xTranslation += qreal(boundingBox.width() - lineWidths.at(i)) / 2.0;
                break;
            case TextHorizontalAlignment::Right:
                xTranslation += qreal(boundingBox.width() - lineWidths.at(i));
                break;
            default:
                break; // Do nothing
            }
            QRectF bound(xTranslation, qreal(nextHeight), lineWidths.at(i), lineHeight);
            QRectF actualBound;
            if (inSrcText.m_DropShadow) {
                QRectF boundShadow(xTranslation + shadowOffsetX, nextHeight + shadowOffsetY,
                                   qreal(lineWidths.at(i)), lineHeight);
                // shadow is a darker shade of the given font color
                painter.setPen(QColor(shadowRgb, shadowRgb, shadowRgb));
                painter.drawText(boundShadow,
                                 alignToQtAlign(inSrcText.m_VerticalAlignment) |
                                 Qt::TextDontClip | Qt::AlignLeft, line, &actualBound);
                painter.setPen(Qt::white); // coloring is done in the shader
            }
            painter.drawText(bound,
                             alignToQtAlign(inSrcText.m_VerticalAlignment) |
                             Qt::TextDontClip | Qt::AlignLeft, line, &actualBound);

            nextHeight += float(lineHeight) + inSrcText.m_Leading;
        }

        return ITextRenderer::UploadData(toU8DataRef(image.bits(), image.byteCount()), inTexture,
                                         image.width(), image.height(),
                                         image.width(), image.height(),
                                         QDemonRenderTextureFormats::RGBA8, true);
    }

    STextTextureDetails RenderText(const STextRenderInfo &inText,
                                   QDemonRenderPathFontItem &inPathFontItem,
                                   QDemonRenderPathFontSpecification &inFontPathSpec) override
    {
        Q_UNUSED(inText);
        Q_UNUSED(inPathFontItem);
        Q_UNUSED(inFontPathSpec);
        Q_ASSERT(m_renderContext->IsPathRenderingSupported());

        // We do not support HW accelerated fonts (yet?)
        Q_ASSERT(false);

        return STextTextureDetails();
    }

    void BeginFrame() override
    {
        // Nothing to do
    }

    void EndFrame() override
    {
        // Nothing to do
    }

    // unused for text rendering via texture atlas
    STextTextureAtlasEntryDetails RenderAtlasEntry(quint32, QDemonRenderTexture2D &) override
    {
        return STextTextureAtlasEntryDetails();
    }
    qint32 CreateTextureAtlas() override
    {
        return 0;
    }
    SRenderTextureAtlasDetails RenderText(const STextRenderInfo &) override
    {
        return SRenderTextureAtlasDetails();
    }
};
}

ITextRendererCore &ITextRendererCore::CreateQtTextRenderer(NVFoundationBase &inFnd,
                                                           IStringTable &inStrTable)
{
    return *QDEMON_NEW(inFnd.getAllocator(), Qt3DSQtTextRenderer)(inFnd, inStrTable);
}
