#ifndef QDEMONPERFTIMER_H
#define QDEMONPERFTIMER_H

#include <QtDemon/qtdemonglobal.h>
#include <QtDemon/qdemontime.h>

#include <QtCore/QSharedPointer>

QT_BEGIN_NAMESPACE

class Q_DEMON_EXPORT IPerfTimer
{
protected:
    virtual ~IPerfTimer() {}
public:
    // amount is in counter frequency units
    virtual void Update(const char *inTag, quint64 inAmount) = 0;
    // Dump current summation of timer data.
    virtual void OutputTimerData(quint32 inFrameCount = 0) = 0;
    virtual void ResetTimerData() = 0;

    static QSharedPointer<IPerfTimer> CreatePerfTimer();
};

// Specialize this struct to get the perf timer in different contexts.
template <typename TTimerProvider>
struct STimerProvider
{
    static IPerfTimer &getPerfTimer(TTimerProvider &inProvider)
    {
        return inProvider.getPerfTimer();
    }
};

template <typename TTimerProvider>
IPerfTimer &getPerfTimer(TTimerProvider &inProvider)
{
    return STimerProvider<TTimerProvider>::getPerfTimer(inProvider);
}

struct SStackPerfTimer
{
    IPerfTimer *m_Timer;
    quint64 m_Start;
    const char *m_Id;

    SStackPerfTimer(IPerfTimer &destination, const char *inId)
        : m_Timer(&destination)
        , m_Start(QDemonTime::getCurrentCounterValue())
        , m_Id(inId)
    {
    }

    SStackPerfTimer(IPerfTimer *destination, const char *inId)
        : m_Timer(destination)
        , m_Start(QDemonTime::getCurrentCounterValue())
        , m_Id(inId)
    {
    }

    ~SStackPerfTimer()
    {
        if (m_Timer) {
            quint64 theStop = QDemonTime::getCurrentCounterValue();
            quint64 theAmount = theStop - m_Start;
            m_Timer->Update(m_Id, theAmount);
        }
    }
};

QT_END_NAMESPACE

#endif // QDEMONPERFTIMER_H
