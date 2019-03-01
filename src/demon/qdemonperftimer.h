#ifndef QDEMONPERFTIMER_H
#define QDEMONPERFTIMER_H

#include <QtDemon/qtdemonglobal.h>
#include <QtDemon/qdemontime.h>

#include <QtCore/QSharedPointer>

QT_BEGIN_NAMESPACE

class Q_DEMON_EXPORT QDemonPerfTimerInterface
{
public:
    QAtomicInt ref;
    virtual ~QDemonPerfTimerInterface() {}
    // amount is in counter frequency units
    virtual void update(const char *inTag, quint64 inAmount) = 0;
    // Dump current summation of timer data.
    virtual void outputTimerData(quint32 inFrameCount = 0) = 0;
    virtual void resetTimerData() = 0;

    static QDemonRef<QDemonPerfTimerInterface> createPerfTimer();
};

// Specialize this struct to get the perf timer in different contexts.
template <typename TTimerProvider>
struct QDemonTimerProvider
{
    static QDemonPerfTimerInterface &getPerfTimer(TTimerProvider &inProvider)
    {
        return inProvider.getPerfTimer();
    }
};

template <typename TTimerProvider>
QDemonPerfTimerInterface &getPerfTimer(TTimerProvider &inProvider)
{
    return QDemonTimerProvider<TTimerProvider>::getPerfTimer(inProvider);
}

struct QDemonStackPerfTimer
{
    QDemonPerfTimerInterface *m_timer;
    quint64 m_start;
    const char *m_id;

    QDemonStackPerfTimer(QDemonPerfTimerInterface &destination, const char *inId)
        : m_timer(&destination)
        , m_start(QDemonTime::getCurrentCounterValue())
        , m_id(inId)
    {
    }

    QDemonStackPerfTimer(QDemonPerfTimerInterface *destination, const char *inId)
        : m_timer(destination)
        , m_start(QDemonTime::getCurrentCounterValue())
        , m_id(inId)
    {
    }

    ~QDemonStackPerfTimer()
    {
        if (m_timer) {
            quint64 theStop = QDemonTime::getCurrentCounterValue();
            quint64 theAmount = theStop - m_start;
            m_timer->update(m_id, theAmount);
        }
    }
};

QT_END_NAMESPACE

#endif // QDEMONPERFTIMER_H
