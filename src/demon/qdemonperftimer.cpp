#include "qdemonperftimer.h"

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QHash>
#include <QtCore/QVector>

#include <QtDemon/qdemonutils.h>

QT_BEGIN_NAMESPACE

struct QDemonTimerEntry
{
    quint64 m_total = 0;
    quint64 m_max = 0;
    quint32 m_updateCount = 0;
    QString m_tag;
    size_t m_order = 0;

    QDemonTimerEntry(const QString &tag, size_t order) : m_tag(tag), m_order(order) {}

    QDemonTimerEntry() = default;

    void update(quint64 increment)
    {
        m_total += increment;
        m_max = increment > m_max ? increment : m_max;
        ++m_updateCount;
    }

    void output(quint32 inFramesPassed) const
    {
        if (m_total) {
            const quint64 tensNanos = QDemonTime::sCounterFreq.toTensOfNanos(m_total);
            const quint64 maxNanos = QDemonTime::sCounterFreq.toTensOfNanos(m_max);

            double milliseconds = tensNanos / 100000.0;
            const double maxMilliseconds = maxNanos / 100000.0;
            if (inFramesPassed == 0) {
                qWarning("%s - %fms", qPrintable(m_tag), milliseconds);
            } else {
                milliseconds /= inFramesPassed;
                qWarning("%s - %fms/frame-total %fms-max %u hits", qPrintable(m_tag), milliseconds, maxMilliseconds, m_updateCount);
            }
        }
    }

    void reset()
    {
        m_total = 0;
        m_max = 0;
        m_updateCount = 0;
    }

    bool operator<(const QDemonTimerEntry &other) const { return m_order < other.m_order; }
};

QDemonPerfTimer::QDemonPerfTimer() = default;

QDemonPerfTimer::~QDemonPerfTimer() = default;

void QDemonPerfTimer::update(const char *inId, quint64 inAmount)
{
    QMutexLocker locker(&mutex);
    QString theStr = QString::fromLocal8Bit(inId);
    auto it = entries.find(inId);
    if (it == entries.end())
        it = entries.insert(inId, QDemonTimerEntry(inId, entries.size()));
    it.value().update(inAmount);
}

void QDemonPerfTimer::outputTimerData(quint32 inFramesPassed)
{
    QMutexLocker locker(&mutex);
    printEntries.clear();
    for (Map::iterator iter = entries.begin(), end = entries.end(); iter != end; ++iter) {
        printEntries.push_back(iter.value());
        iter.value().reset();
    }

    std::sort(printEntries.begin(), printEntries.end());

    for (const auto &printEntry : qAsConst(printEntries))
        printEntry.output(inFramesPassed);
}

void QDemonPerfTimer::resetTimerData()
{
    QMutexLocker locker(&mutex);
    auto iter = entries.begin();
    const auto end = entries.end();
    while (iter != end) {
        iter.value().reset();
        ++iter;
    }
}

QT_END_NAMESPACE
