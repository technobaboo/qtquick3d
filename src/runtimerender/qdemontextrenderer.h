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
#ifndef QDEMON_TEXT_RENDERER_H
#define QDEMON_TEXT_RENDERER_H

#include <QtDemon/QDemonOption>

#include <QtDemonRender/qdemonrenderbasetypes.h>

#include <QtDemonRuntimeRender/qdemonrendertexttypes.h>

#include <QtDemonRuntimeRender/qtdemonruntimerenderglobal.h>

QT_BEGIN_NAMESPACE

struct QDemonRendererFontEntry
{
    QString m_fontName;
    QString m_fontFile;
    QDemonRendererFontEntry() = default;
    QDemonRendererFontEntry(QString nm, QString file) : m_fontName(nm), m_fontFile(file) {}
};

class QDemonTextRendererInterface;
class QDemonRenderPathFontItem;
class QDemonAbstractThreadPool;
class QDemonPerfTimerInterface;
class QDemonRenderContext;
class QDemonRenderPathFontSpecification;

/**
 *	Opaque text rendering system.  Must be able to render text to an opengl texture object.
 */
class Q_DEMONRUNTIMERENDER_EXPORT QDemonTextRendererInterface
{
public:
    QAtomicInt ref;
    virtual ~QDemonTextRendererInterface() {}
    // You can have several standard font directories and these will be persistent
    virtual void addSystemFontDirectory(const char *inDirectory) = 0;
    // Should be called to clear the current context.
    virtual void addProjectFontDirectory(const char *inProjectDirectory) = 0;
    virtual void clearProjectFontDirectories() = 0;
    // Force font loading *right now*
    virtual void preloadFonts() = 0;
    // Do not access object in between begin/end preload pairs.
    virtual void beginPreloadFonts(QDemonAbstractThreadPool &inThreadPool, QDemonRef<QDemonPerfTimerInterface> inTimer) = 0;
    virtual void endPreloadFonts() = 0;
    // Force a clear and reload of all of the fonts.
    virtual void reloadFonts() = 0;
    // Get the list of project fonts.  These are the only fonts that can be displayed.
    virtual QDemonConstDataRef<QDemonRendererFontEntry> getProjectFontList() = 0;
    // The name stored in the ttf file isn't the actual name we use; we use the file stems.
    // But we used to use the name.  So this provides a sort of first-come-first-serve remapping
    // from ttf-name to file-stem.
    virtual QDemonOption<QString> getFontNameForFont(QString inFontname) = 0;
    virtual QDemonOption<QString> getFontNameForFont(const char *inFontname) = 0;

    virtual QDemonRef<QDemonTextRendererInterface> getTextRenderer(QDemonRef<QDemonRenderContext> inContext) = 0;

    static QDemonRef<QDemonTextRendererInterface> createQtTextRenderer();

    // call this to create onscreen text renderer
    // it needs true type fonts
    static QDemonRef<QDemonTextRendererInterface> createOnscreenTextRenderer();

    // Measure text will inText if it isn't null or the text on the info if inText is null
    virtual QDemonTextDimensions measureText(const QDemonTextRenderInfo &inText,
                                             float inTextScaleFactor,
                                             const char *inTextOverride = nullptr) = 0;
    // The system will use the 'r' channel as an alpha mask in order to render the
    // text.  You can assume GetTextDimensions was called *just* prior to this.
    // It is a good idea to ensure the texture is a power of two as not all rendering systems
    // support nonpot textures.  Our text rendering algorithms will render a sub-rect of the
    // image
    // assuming it is located toward the upper-left of the image and we are also capable of
    // flipping
    // the image.
    virtual QDemonTextTextureDetails renderText(const QDemonTextRenderInfo &inText, QDemonRenderTexture2D &inTexture) = 0;
    // this is for rendering text with NV path rendering
    virtual QDemonTextTextureDetails renderText(const QDemonTextRenderInfo &inText,
                                                QDemonRenderPathFontItem &inPathFontItem,
                                                QDemonRenderPathFontSpecification &inPathFontSpecicification) = 0;
    // this is for rednering text using a texture atlas
    virtual QDemonRenderTextureAtlasDetails renderText(const QDemonTextRenderInfo &inText) = 0;

    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;

    // these two function are for texture atlas usage only
    // returns the atlas entries count
    virtual qint32 createTextureAtlas() = 0;
    virtual QDemonTextTextureAtlasEntryDetails renderAtlasEntry(quint32 index, QDemonRenderTexture2D &inTexture) = 0;

    // Helper function to upload the texture data to the texture
    // Will resize texture as necessary and upload using texSubImage for
    // quickest upload times
    // This function expects that the dataWidth to be divisible by four and
    // that the total data height is larger then inTextHeight *and* divisible by four.
    // and that textWidth and textHeight are less than or equal to dataWidth,dataHeight
    //,can be zero, and don't need to be divisible by four (or 2).
    static QDemonTextTextureDetails uploadData(QDemonDataRef<quint8> inTextureData,
                                               QDemonRenderTexture2D &inTexture,
                                               quint32 inDataWidth,
                                               quint32 inDataHeight,
                                               quint32 inTextWidth,
                                               quint32 inTextHeight,
                                               QDemonRenderTextureFormats::Enum inFormat,
                                               bool inFlipYAxis);

    // Helper function to return the next power of two.
    // Fails for values of 0 or std::numeric_limits<quint32>::max()
    static quint32 nextPowerOf2(quint32 inValue);
    // If inValue is divisible by four, then return inValue
    // else next largest number that is divisible by four.
    static quint32 nextMultipleOf4(quint32 inValue);
};
QT_END_NAMESPACE

#endif
