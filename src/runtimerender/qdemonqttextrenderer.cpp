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

#include "qdemontextrenderer.h"
#include "qdemonrenderthreadpool.h"

#include <QtDemonRuntimeRender/qdemonrendertext.h>

#include <QtDemonRender/qdemonrendercontext.h>

#include <QtDemon/qdemonutils.h>

#include <QtGui/QPainter>
#include <QtGui/QImage>
#include <QtGui/QFontDatabase>
#include <QtGui/QRawFont>
#include <QtGui/QGuiApplication>

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QHash>
#include <QtCore/QWaitCondition>
#include <QtCore/QMutex>

#include <QtMath>

QT_BEGIN_NAMESPACE

namespace {

struct QDemonQtTextRenderer : public ITextRenderer
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

    typedef QSet<QString> TStringSet;
    typedef QHash<QString, FontInfo> TFontInfoHash;

    QSharedPointer<QDemonRenderContext> m_renderContext;
    QSharedPointer<IPerfTimer> m_perfTimer;
    QVector<SRendererFontEntry> m_installedFonts;

    QWaitCondition m_PreloadSync;
    QMutex m_mutex;

    TStringSet m_systemFontDirs;
    TStringSet m_projectFontDirs;
    TFontInfoHash m_projectFontInfos;
    TFontInfoHash m_systemFontInfos;
    QString m_workspace;

    bool m_systemFontsInitialized;
    bool m_projectFontsInitialized;
    bool m_PreloadingFonts;

    QStringList m_nameFilters;
    qreal m_pixelRatio;

    QDemonQtTextRenderer()
        : m_systemFontsInitialized(false)
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
    virtual ~QDemonQtTextRenderer()
    {
        QFontDatabase::removeAllApplicationFonts();
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
            QString localDir = CFileTools::NormalizePathForQtUsage(*theIter);
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

    bool AddFontDirectory(const QString &inDirectory, TStringSet &inDirSet)
    {
        if (inDirectory.isEmpty()) {
            m_workspace = QStringLiteral("./");
        } else {
            m_workspace.clear();
            for (const char *item = inDirectory.toLocal8Bit().constData(); item && *item; ++item) {
                if (*item == '\\')
                    m_workspace.append(QStringLiteral("/"));
                else
                    m_workspace.append(QString::fromLocal8Bit(item));
            }
            if (m_workspace.back() != '/')
                m_workspace.append(QStringLiteral("/"));
        }
        auto iterator = inDirSet.insert(m_workspace);
        return iterator->isEmpty();
    }

    // You can have several standard font directories and these will be persistent
    void AddSystemFontDirectory(const char *inDirectory) override
    {
        AddFontDirectory(QString::fromLocal8Bit(inDirectory), m_systemFontDirs);
    }

    void AddProjectFontDirectory(const char *inProjectDirectory) override
    {
        bool theAddResult = AddFontDirectory(QString::fromLocal8Bit(inProjectDirectory), m_projectFontDirs);
        if (theAddResult && m_projectFontsInitialized)
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
        QDemonQtTextRenderer *theRenderer(reinterpret_cast<QDemonQtTextRenderer *>(inData));
        theRenderer->PreloadFonts();
        theRenderer->m_PreloadSync.wakeAll();
    }

    void BeginPreloadFonts(IThreadPool &inThreadPool, QSharedPointer<IPerfTimer> inTimer) override
    {
        m_PreloadingFonts = true;

        m_PreloadSync.wakeAll();
        m_perfTimer = inTimer;

        inThreadPool.AddTask(this, PreloadThreadCallback, nullptr);
    }

    void EndPreloadFonts() override
    {
        m_mutex.lock();
        if (m_PreloadingFonts) {
            {
                //SStackPerfTimer __perfTimer(*m_perfTimer, "QtText: Wait till font preloading completed");
                m_PreloadSync.wait(&m_mutex);
            }
        }
        m_PreloadingFonts = false;
        m_mutex.unlock();
    }

    // Get the list of project fonts. These are the only fonts that can be displayed.
    QDemonConstDataRef<SRendererFontEntry> GetProjectFontList() override
    {
        PreloadFonts();
        if (m_installedFonts.empty()) {
            m_installedFonts.reserve(m_projectFontInfos.size());
            for (FontInfo &fi : m_projectFontInfos.values()) {
                m_installedFonts.push_back(SRendererFontEntry(fi.fontName, fi.fontFileName));
            }
        }
        return toConstDataRef(m_installedFonts.constData(), m_installedFonts.count());
    }

    QDemonOption<QString> GetFontNameForFont(QString inFontname) override
    {
        // This function is there to support legacy font names.

        QString inStr = inFontname;
        if (m_projectFontInfos.keys().contains(inStr))
            return inFontname;

        // Fall back for family name detection if not found by font name
        for (FontInfo &fi : m_projectFontInfos.values()) {
            if (inStr == fi.fontFamily)
                return fi.fontName;
        }

        return QDemonEmpty();
    }

    QDemonOption<QString> GetFontNameForFont(const char *inFontname) override
    {
        return GetFontNameForFont(QString::fromLocal8Bit(inFontname));
    }

    ITextRenderer &GetTextRenderer(QSharedPointer<QDemonRenderContext> inRenderContext) override
    {
        m_renderContext = inRenderContext;
        return *this;
    }

    FontInfo &fontInfoForName(const QString &fontName)
    {
        PreloadFonts();
        QString qtFontName = fontName;
        if (m_projectFontInfos.contains(qtFontName))
            return m_projectFontInfos[qtFontName];

        if (m_systemFontInfos.contains(qtFontName))
            return m_systemFontInfos[qtFontName];

        // Unknown font, create a system font for it
        FontInfo fi(QString(), qtFontName, qtFontName, -1);
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

    QStringList splitText(const char *theText)
    {
        // Split the text into lines
        int lines = 1;
        int lineLen = 0;
        QStringList lineList;
        const char *lineStartItem = nullptr;
        for (const char *item = theText; item && *item; ++item) {
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
                           QVector<qreal> &lineWidths, const char *inTextOverride = nullptr)
    {
        const char *theText = inTextOverride ? inTextOverride : inText.m_Text.toLocal8Bit().constData();
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
                                const char *inTextOverride) override
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

ITextRendererCore &ITextRendererCore::CreateQtTextRenderer()
{
    return *new QDemonQtTextRenderer();
}

QT_END_NAMESPACE