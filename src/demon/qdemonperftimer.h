#ifndef QDEMONPERFTIMER_H
#define QDEMONPERFTIMER_H

#include <QtDemon/qtdemonglobal.h>
#include <QtDemon/qdemontime.h>

#include <QtCore/QSharedPointer>
#include <QtCore/QVector>
#include <QtCore/qmutex.h>

QT_BEGIN_NAMESPACE

struct QDemonTimerEntry;

class Q_DEMON_EXPORT QDemonPerfTimer
{
    struct Private {
        ~Private();

        QAtomicInt ref;
        typedef QHash<QString, QDemonTimerEntry> Map;
        // This object needs its own string table because it is used during the binary load process with
        // the application string table gets booted up.
        Map entries;
        QVector<QDemonTimerEntry> printEntries;
        QMutex mutex;
    };
    QExplicitlySharedDataPointer<Private> d;

public:
    QDemonPerfTimer() = default;
    ~QDemonPerfTimer();
    // amount is in counter frequency units
    void update(const char *inTag, quint64 inAmount);
    // Dump current summation of timer data.
    void outputTimerData(quint32 inFrameCount = 0);
    void resetTimerData();
    bool isValid() const { return d != nullptr; }

    static QDemonPerfTimer create();
};

#if 0
// Specialize this struct to get the perf timer in different contexts.
template<typename TTimerProvider>
struct QDemonTimerProvider
{
    static QDemonPerfTimer &getPerfTimer(TTimerProvider &inProvider) { return inProvider.getPerfTimer(); }
};

template<typename TTimerProvider>
QDemonPerfTimer &getPerfTimer(TTimerProvider &inProvider)
{
    return QDemonTimerProvider<TTimerProvider>::getPerfTimer(inProvider);
}
#endif

struct QDemonStackPerfTimer
{
    QDemonPerfTimer m_timer;
    quint64 m_start;
    const char *m_id;

    QDemonStackPerfTimer(QDemonPerfTimer destination, const char *inId)
        : m_timer(destination), m_start(QDemonTime::getCurrentCounterValue()), m_id(inId)
    {
    }

    ~QDemonStackPerfTimer()
    {
        if (m_timer.isValid()) {
            quint64 theStop = QDemonTime::getCurrentCounterValue();
            quint64 theAmount = theStop - m_start;
            m_timer.update(m_id, theAmount);
        }
    }
};

QT_END_NAMESPACE

#endif // QDEMONPERFTIMER_H
