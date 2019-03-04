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
#include "qdemonrendertextureatlas.h"

#include <QtDemonRender/qdemonrendertexture2d.h>
#include <QtDemonRender/qdemonrendercontext.h>

QT_BEGIN_NAMESPACE

namespace {

// a algorithm based on http://clb.demon.fi/files/RectangleBinPack/
struct QDemonTextureAtlasBinPackSL
{
public:
    QDemonTextureAtlasBinPackSL(QDemonRef<QDemonRenderContext> inContext, qint32 width, qint32 height)
        : m_binWidth(width)
        , m_binHeight(height)
    {
        // TODO:
        Q_UNUSED(inContext);
        // setup first entry
        SkylineNode theNode = { 0, 0, width };
        m_skyLine.push_back(theNode);
    }

    /*	insert new rect
     *
     */
    QDemonTextureAtlasRect insert(qint32 width, qint32 height)
    {
        qint32 binHeight;
        qint32 binWidth;
        qint32 binIndex;

        QDemonTextureAtlasRect newNode = findPosition(width, height, &binWidth, &binHeight, &binIndex);

        if (binIndex != -1) {
            // adjust skyline nodes
            addSkylineLevelNode(binIndex, newNode);
        }

        return newNode;
    }

private:
    /// Represents a single level (a horizontal line) of the skyline/horizon/envelope.
    struct SkylineNode
    {
        int x; ///< The starting x-coordinate (leftmost).
        int y; ///< The y-coordinate of the skyline level line.
        int width; /// The line width. The ending coordinate (inclusive) will be x+width-1.
    };

    /*	find position
     *
     */
    QDemonTextureAtlasRect findPosition(qint32 width, qint32 height, qint32 *binWidth, qint32 *binHeight,
                                   qint32 *binIndex)
    {
        *binWidth = m_binWidth;
        *binHeight = m_binHeight;
        *binIndex = -1;
        QDemonTextureAtlasRect newRect;

        for (quint32 i = 0; i < m_skyLine.size(); ++i) {
            qint32 y = getSkylineLevel(i, width, height);

            if (y >= 0) {
                if ((y + height < *binHeight) || ((y + height == *binHeight) && m_skyLine[i].width < *binWidth)) {
                    *binHeight = y + height;
                    *binIndex = i;
                    *binWidth = m_skyLine[i].width;
                    newRect.x = m_skyLine[i].x;
                    newRect.y = y;
                    newRect.width = width;
                    newRect.height = height;
                }
            }
        }

        return newRect;
    }

    /* @brief check if rectangle can be placed into the the bin
     *
     * return skyline hight
     */
    int getSkylineLevel(quint32 binIndex, qint32 width, qint32 height)
    {
        // first check width exceed
        qint32 x = m_skyLine[binIndex].x;
        if (x + width > m_binWidth)
            return -1;

        qint32 leftAlign = width;
        quint32 index = binIndex;
        qint32 y = m_skyLine[index].y;

        while (leftAlign > 0) {
            y = (y > m_skyLine[index].y) ? y : m_skyLine[index].y;
            // check hight
            if (y + height > m_binHeight)
                return -1;

            leftAlign -= m_skyLine[index].width;
            ++index;

            if (index > m_skyLine.size())
                return -1;
        }

        return y;
    }

    /* @brief add an new skyline entry
     *
     * return no return
     */
    void addSkylineLevelNode(qint32 binIndex, const QDemonTextureAtlasRect &newRect)
    {
        SkylineNode newNode;

        newNode.x = newRect.x;
        newNode.y = newRect.y + newRect.height;
        newNode.width = newRect.width;
        m_skyLine.insert(m_skyLine.begin() + binIndex, newNode);

        // iterate over follow up nodes and adjust
        for (quint32 i = binIndex + 1; i < m_skyLine.size(); ++i) {
            if (m_skyLine[i].x < m_skyLine[i - 1].x + m_skyLine[i - 1].width) {
                int shrink = m_skyLine[i - 1].x + m_skyLine[i - 1].width - m_skyLine[i].x;

                m_skyLine[i].x += shrink;
                m_skyLine[i].width -= shrink;

                if (m_skyLine[i].width <= 0) {
                    m_skyLine.erase(m_skyLine.begin() + i);
                    --i;
                } else {
                    break;
                }
            } else {
                break;
            }
        }

        mergeSkylineLevelNodes();
    }

    /* @brief merge skyline node
     *
     * return no return
     */
    void mergeSkylineLevelNodes()
    {
        // check if we can merge nodes
        for (quint32 i = 0; i < m_skyLine.size() - 1; ++i) {
            if (m_skyLine[i].y == m_skyLine[i + 1].y) {
                m_skyLine[i].width += m_skyLine[i + 1].width;
                m_skyLine.erase(m_skyLine.begin() + (i + 1));
                --i;
            }
        }
    }

    qint32 m_binWidth;
    qint32 m_binHeight;

    QVector<SkylineNode> m_skyLine;
};

struct QDemonTextureAtlasEntry
{
    QDemonTextureAtlasEntry() = default;
    QDemonTextureAtlasEntry(float x, float y, float w, float h, QDemonDataRef<quint8> buffer)
        : m_x(x)
        , m_y(y)
        , m_width(w)
        , m_height(h)
        , m_buffer(buffer)
    {
    }
    QDemonTextureAtlasEntry(const QDemonTextureAtlasEntry &entry)
    {
        m_x = entry.m_x;
        m_y = entry.m_y;
        m_width = entry.m_width;
        m_height = entry.m_height;
        m_buffer = entry.m_buffer;
    }

    float m_x = 0.0f;
    float m_y = 0.0f;
    float m_width = 0.0f;
    float m_height = 0.0f;
    QDemonDataRef<quint8> m_buffer;
};

struct QDemonTextureAtlas : public QDemonTextureAtlasInterface
{
    QDemonRef<QDemonRenderContext> m_renderContext;

    QDemonTextureAtlas(const QDemonRef<QDemonRenderContext>& inRenderContext, qint32 width, qint32 height)
        : m_renderContext(inRenderContext)
        , m_width(width)
        , m_height(height)
        , m_spacing(1)
    {
        m_binPack = new QDemonTextureAtlasBinPackSL(inRenderContext, width, height);
    }

    virtual ~QDemonTextureAtlas()
    {
        relaseEntries();

        if (m_binPack)
            delete m_binPack;
    }

    void relaseEntries() override
    {
        QVector<QDemonTextureAtlasEntry>::iterator it;

        for (it = m_atlasEntrys.begin(); it != m_atlasEntrys.end(); it++) {
            ::free(it->m_buffer.begin());
        }

        m_atlasEntrys.clear();
    }
    qint32 getWidth() const override { return m_width; }
    qint32 getHeight() const override { return m_height; }

    qint32 getAtlasEntryCount() const override { return m_atlasEntrys.size(); }

    TTextureAtlasEntryAndBuffer getAtlasEntryByIndex(quint32 index) override
    {
        if (index >= m_atlasEntrys.size())
            return TTextureAtlasEntryAndBuffer(QDemonTextureAtlasRect(), QDemonDataRef<quint8>());

        return TTextureAtlasEntryAndBuffer(QDemonTextureAtlasRect((qint32)m_atlasEntrys[index].m_x,
                                                             (qint32)m_atlasEntrys[index].m_y,
                                                             (qint32)m_atlasEntrys[index].m_width,
                                                             (qint32)m_atlasEntrys[index].m_height),
                                           m_atlasEntrys[index].m_buffer);
    }

    QDemonTextureAtlasRect addAtlasEntry(qint32 width, qint32 height, qint32 pitch,
                                    qint32 dataWidth, QDemonConstDataRef<quint8> bufferData) override
    {
        QDemonTextureAtlasRect rect;

        // pitch is the number of bytes per line in bufferData
        // dataWidth is the relevant data width in bufferData. Rest is padding that can be ignored.
        if (m_binPack) {
            qint32 paddedWith, paddedPitch, paddedHeight;
            // add spacing around the character
            paddedWith = width + 2 * m_spacing;
            paddedPitch = dataWidth + 2 * m_spacing;
            paddedHeight = height + 2 * m_spacing;
            // first get entry in the texture atlas
            rect = m_binPack->insert(paddedWith, paddedHeight);
            if (rect.width == 0)
                return rect;

            // we align the data be to 4 byte
            int alignment = (4 - (paddedPitch % 4)) % 4;
            paddedPitch += alignment;

            // since we do spacing around the character we need to copy line by line
            quint8 *glyphBuffer =
                    static_cast<quint8 *>(::malloc(paddedHeight * paddedPitch * sizeof(quint8)));
            if (glyphBuffer) {
                memset(glyphBuffer, 0, paddedHeight * paddedPitch);

                quint8 *pDst = glyphBuffer + paddedPitch + m_spacing;
                quint8 *pSrc = const_cast<quint8 *>(bufferData.begin());
                for (qint32 i = 0; i < height; ++i) {
                    memcpy(pDst, pSrc, dataWidth);

                    pDst += paddedPitch;
                    pSrc += pitch;
                }

                // add new entry
                m_atlasEntrys.push_back(QDemonTextureAtlasEntry(
                                            (float)rect.x, (float)rect.y, (float)paddedWith, (float)paddedHeight,
                                            QDemonDataRef<quint8>(glyphBuffer, paddedHeight * paddedPitch * sizeof(quint8))));

                // normalize texture coordinates
                rect.normX = (float)rect.x / (float)m_width;
                rect.normY = (float)rect.y / (float)m_height;
                rect.normWidth = (float)paddedWith / (float)m_width;
                rect.normHeight = (float)paddedHeight / (float)m_height;
            }
        }

        return rect;
    }

private:
    qint32 m_width; ///< texture atlas width
    qint32 m_height; ///< texture atlas height
    qint32 m_spacing; ///< spacing around the entry
    QVector<QDemonTextureAtlasEntry> m_atlasEntrys; ///< our entries in the atlas
    QDemonTextureAtlasBinPackSL *m_binPack; ///< our bin packer which actually does most of the work
};

} // namespace

QDemonRef<QDemonTextureAtlasInterface> QDemonTextureAtlasInterface::createTextureAtlas(const QDemonRef<QDemonRenderContext>& inRenderContext, qint32 width, qint32 height)
{
    return QDemonRef<QDemonTextureAtlasInterface>(new QDemonTextureAtlas(inRenderContext, width, height));
}

QT_END_NAMESPACE
