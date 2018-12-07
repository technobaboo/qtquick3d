#ifndef QDEMONTIME_H
#define QDEMONTIME_H

#include <QtDemon/qtdemonglobal.h>

QT_BEGIN_NAMESPACE

struct QDemonTimeCounterFrequencyToTensOfNanos
{
    quint64 mNumerator;
    quint64 mDenominator;
    QDemonTimeCounterFrequencyToTensOfNanos(quint64 inNum, quint64 inDenom)
        : mNumerator(inNum)
        , mDenominator(inDenom)
    {
    }

    // quite slow.
    quint64 toTensOfNanos(quint64 inCounter) const
    {
        return (inCounter * mNumerator) / mDenominator;
    }
};

class QDemonTime
{
public:
    typedef double Second;
    static const quint64 sNumTensOfNanoSecondsInASecond = 100000000;
    // This is supposedly guaranteed to not change after system boot
    // regardless of processors, speedstep, etc.
    static const QDemonTimeCounterFrequencyToTensOfNanos sCounterFreq;

    static QDemonTimeCounterFrequencyToTensOfNanos getCounterFrequency();

    static quint64 getCurrentCounterValue();

    // SLOW!!
    // Thar be a 64 bit divide in thar!
    static quint64 getCurrentTimeInTensOfNanoSeconds()
    {
        quint64 ticks = getCurrentCounterValue();
        return sCounterFreq.toTensOfNanos(ticks);
    }

    QDemonTime();
    Second getElapsedSeconds();
    Second peekElapsedSeconds();
    Second getLastTime() const;

private:
    qint64 mTickCount;
};

QT_END_NAMESPACE

#endif // QDEMONTIME_H
