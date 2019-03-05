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

QDemonPerfTimer::~QDemonPerfTimer()
{
}

void QDemonPerfTimer::update(const char *inId, quint64 inAmount)
{
    QMutexLocker locker(&d->mutex);
    QString theStr = QString::fromLocal8Bit(inId);
    auto it = d->entries.find(inId);
    if (it == d->entries.end())
        it = d->entries.insert(inId, QDemonTimerEntry(inId, d->entries.size()));
    it.value().update(inAmount);
}

void QDemonPerfTimer::outputTimerData(quint32 inFramesPassed)
{
    QMutexLocker locker(&d->mutex);
    d->printEntries.clear();
    for (Private::Map::iterator iter = d->entries.begin(), end = d->entries.end(); iter != end; ++iter) {
        d->printEntries.push_back(iter.value());
        iter.value().reset();
    }

    std::sort(d->printEntries.begin(), d->printEntries.end());

    for (const auto &printEntry : qAsConst(d->printEntries))
        printEntry.output(inFramesPassed);
}

void QDemonPerfTimer::resetTimerData()
{
    QMutexLocker locker(&d->mutex);
    auto iter = d->entries.begin();
    const auto end = d->entries.end();
    while (iter != end) {
        iter.value().reset();
        ++iter;
    }
}

QDemonPerfTimer QDemonPerfTimer::create()
{
    QDemonPerfTimer timer;
    timer.d = new Private;
    return timer;
}


QDemonPerfTimer::Private::~Private()
{

}

QT_END_NAMESPACE
