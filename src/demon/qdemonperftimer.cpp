#include "qdemonperftimer.h"

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QHash>
#include <QtCore/QVector>

#include <QtDemon/qdemonutils.h>

QT_BEGIN_NAMESPACE

namespace {
struct QDemonTimerEntry
{
    quint64 m_total = 0;
    quint64 m_max = 0;
    quint32 m_updateCount = 0;
    QString m_tag;
    size_t m_order = 0;

    QDemonTimerEntry(const QString &tag, size_t order)
        : m_total(0)
        , m_max(0)
        , m_updateCount(0)
        , m_tag(tag)
        , m_order(order)
    {
    }

    QDemonTimerEntry() = default;

    void update(quint64 increment)
    {
        m_total += increment;
        m_max = increment > m_max ? increment : m_max;
        ++m_updateCount;
    }

    void output(quint32 inFramesPassed)
    {
        if (m_total) {
            quint64 tensNanos = QDemonTime::sCounterFreq.toTensOfNanos(m_total);
            quint64 maxNanos = QDemonTime::sCounterFreq.toTensOfNanos(m_max);

            double milliseconds = tensNanos / 100000.0;
            double maxMilliseconds = maxNanos / 100000.0;
            if (inFramesPassed == 0) {
                qWarning("%s - %fms", qPrintable(m_tag), milliseconds);
            } else {
                milliseconds /= inFramesPassed;
                qWarning("%s - %fms/frame-total %fms-max %u hits",
                         qPrintable(m_tag), milliseconds, maxMilliseconds, m_updateCount);
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
struct QDemonPerfTimer : public QDemonPerfTimerInterface
{
    typedef QHash<QString, QDemonTimerEntry> THashMapType;
    // This object needs its own string table because it is used during the binary load process with
    // the application string table gets booted up.
    THashMapType m_entries;
    QVector<QDemonTimerEntry> m_printEntries;
    QMutex m_mutex;

    void update(const char *inId, quint64 inAmount) override
    {
        QMutexLocker locker(&m_mutex);
        QString theStr = QString::fromLocal8Bit(inId);
        THashMapType::iterator theFind = m_entries.insert(theStr, QDemonTimerEntry(theStr, m_entries.size()));
        theFind.value().update(inAmount);
    }

    // Dump current summation of timer data.
    void outputTimerData(quint32 inFramesPassed = 0) override
    {
        QMutexLocker locker(&m_mutex);
        m_printEntries.clear();
        for (THashMapType::iterator iter = m_entries.begin(), end = m_entries.end(); iter != end; ++iter) {
            m_printEntries.push_back(iter.value());
            iter.value().reset();
        }

        std::sort(m_printEntries.begin(), m_printEntries.end());

        for (quint32 idx = 0, end = (quint32)m_printEntries.size(); idx < end; ++idx) {
            m_printEntries[idx].output(inFramesPassed);
        }
    }

    void resetTimerData() override
    {
        QMutexLocker locker(&m_mutex);
        for (THashMapType::iterator iter = m_entries.begin(), end = m_entries.end(); iter != end;
             ++iter) {
            iter.value().reset();
        }
    }

    virtual void clearPerfKeys()
    {
        QMutexLocker locker(&m_mutex);
        m_entries.clear();
    }
};
}

QSharedPointer<QDemonPerfTimerInterface> QDemonPerfTimerInterface::createPerfTimer()
{
    return QSharedPointer<QDemonPerfTimerInterface>(new QDemonPerfTimer());
}

QT_END_NAMESPACE
