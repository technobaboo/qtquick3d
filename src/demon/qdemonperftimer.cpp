#include "qdemonperftimer.h"

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QHash>
#include <QtCore/QVector>

#include <QtDemon/qdemonutils.h>

QT_BEGIN_NAMESPACE

namespace {
struct STimerEntry
{
    quint64 m_Total;
    quint64 m_Max;
    quint32 m_UpdateCount;
    QString m_Tag;
    size_t m_Order;

    STimerEntry(const QString &tag, size_t order)
        : m_Total(0)
        , m_Max(0)
        , m_UpdateCount(0)
        , m_Tag(tag)
        , m_Order(order)
    {
    }

    STimerEntry(const STimerEntry &e)
        : m_Total(e.m_Total)
        , m_Max(e.m_Max)
        , m_UpdateCount(e.m_UpdateCount)
        , m_Tag(e.m_Tag)
        , m_Order(e.m_Order)
    {
    }

    STimerEntry()
        : m_Total(0)
        , m_Max(0)
        , m_UpdateCount(0)
        , m_Order(0)
    {
    }

    void Update(quint64 increment)
    {
        m_Total += increment;
        m_Max = increment > m_Max ? increment : m_Max;
        ++m_UpdateCount;
    }

    void Output(quint32 inFramesPassed)
    {
        if (m_Total) {
            quint64 tensNanos = QDemonTime::sCounterFreq.toTensOfNanos(m_Total);
            quint64 maxNanos = QDemonTime::sCounterFreq.toTensOfNanos(m_Max);

            double milliseconds = tensNanos / 100000.0;
            double maxMilliseconds = maxNanos / 100000.0;
            if (inFramesPassed == 0)
                qWarning("%s - %fms", qPrintable(m_Tag), milliseconds);
            else {
                milliseconds /= inFramesPassed;
                qWarning("%s - %fms/frame-total %fms-max %u hits",
                         qPrintable(m_Tag), milliseconds, maxMilliseconds, m_UpdateCount);
            }
        }
    }

    void Reset()
    {
        m_Total = 0;
        m_Max = 0;
        m_UpdateCount = 0;
    }

    bool operator<(const STimerEntry &other) const { return m_Order < other.m_Order; }
};
struct SPerfTimer : public IPerfTimer
{
    typedef QHash<QString, STimerEntry> THashMapType;
    // This object needs its own string table because it is used during the binary load process with
    // the application string table gets booted up.
    THashMapType m_Entries;
    QVector<STimerEntry> m_PrintEntries;
    QMutex m_Mutex;

    SPerfTimer()
    {
    }

    void Update(const char *inId, quint64 inAmount) override
    {
        QMutexLocker locker(&m_Mutex);
        QString theStr = QString::fromLocal8Bit(inId);
        THashMapType::iterator theFind = m_Entries.insert(theStr, STimerEntry(theStr, m_Entries.size()));
        theFind.value().Update(inAmount);
    }

    // Dump current summation of timer data.
    void OutputTimerData(quint32 inFramesPassed = 0) override
    {
        QMutexLocker locker(&m_Mutex);
        m_PrintEntries.clear();
        for (THashMapType::iterator iter = m_Entries.begin(), end = m_Entries.end(); iter != end; ++iter) {
            m_PrintEntries.push_back(iter.value());
            iter.value().Reset();
        }

        std::sort(m_PrintEntries.begin(), m_PrintEntries.end());

        for (quint32 idx = 0, end = (quint32)m_PrintEntries.size(); idx < end; ++idx) {
            m_PrintEntries[idx].Output(inFramesPassed);
        }
    }

    void ResetTimerData() override
    {
        QMutexLocker locker(&m_Mutex);
        for (THashMapType::iterator iter = m_Entries.begin(), end = m_Entries.end(); iter != end;
             ++iter) {
            iter.value().Reset();
        }
    }

    virtual void ClearPerfKeys()
    {
        QMutexLocker locker(&m_Mutex);
        m_Entries.clear();
    }
};
}

QSharedPointer<IPerfTimer> IPerfTimer::CreatePerfTimer()
{
    return QSharedPointer<IPerfTimer>(new SPerfTimer());
}

QT_END_NAMESPACE
