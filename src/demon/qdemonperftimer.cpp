#include "qdemonperftimer.h"

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QHash>
#include <QtCore/QVector>

#include <QtDemon/qdemonutils.h>

QT_BEGIN_NAMESPACE

static uint qHash(const QDemonPerfTimer::Key &key)
{
    const uchar *s = reinterpret_cast<const uchar *>(key.id);
    uint h = 0;
    while (*s) {
        h = 31 * h + *s;
        ++s;
    }
    return h;
}
static bool operator==(const QDemonPerfTimer::Key &a, const QDemonPerfTimer::Key &b) { return !strcmp(a.id, b.id); }

static bool operator<(const QDemonPerfTimer::Entry &a, const QDemonPerfTimer::Entry &b) { return a.tag < b.tag; }

void QDemonPerfTimer::Entry::update(qint64 elapsed)
{
    totalTime += elapsed;
    maxTime = qMax(maxTime, elapsed);
    ++count;
}

void QDemonPerfTimer::Entry::reset()
{
    totalTime = 0;
    maxTime = 0;
    count = 0;
}

QString QDemonPerfTimer::Entry::toString(quint32 inFramesPassed) const
{
    if (!count)
        return QString();


    const double milliseconds = totalTime / 1000000.0;
    const double maxMilliseconds = maxTime / 1000000.0;
    if (inFramesPassed == 0)
        return QString::fromLatin1("%1 - %2ms").arg(tag).arg(milliseconds);

    return QString::fromLatin1("%1 - %2ms/frame; %3ms max; %4 hits").arg(tag).arg(milliseconds/inFramesPassed).arg(maxMilliseconds).arg(count);
}


QDemonPerfTimer::QDemonPerfTimer() = default;

QDemonPerfTimer::~QDemonPerfTimer() = default;

void QDemonPerfTimer::update(const char *inId, qint64 elapsed)
{
    QMutexLocker locker(&mutex);
    auto it = entries.find(Key{inId});
    if (it == entries.end())
        it = entries.insert(Key{inId}, Entry(QString::fromUtf8(inId)));
    it.value().update(elapsed);
}

void QDemonPerfTimer::dump()
{
    QMutexLocker locker(&mutex);
    QVector<QDemonPerfTimer::Entry> allEntries;
    for (auto iter = entries.begin(), end = entries.end(); iter != end; ++iter) {
        allEntries.push_back(iter.value());
        iter.value().reset();
    }

    std::sort(allEntries.begin(), allEntries.end());

    qDebug() << "performance data:";
    for (const auto &e: qAsConst(allEntries))
        qDebug() << "    " << e.toString(frameCount).toUtf8().constData();
    qDebug() << "";

    frameCount = 0;
}

void QDemonPerfTimer::reset()
{
    QMutexLocker locker(&mutex);
    auto iter = entries.begin();
    const auto end = entries.end();
    while (iter != end) {
        iter.value().reset();
        ++iter;
    }

    frameCount = 0;
}

QT_END_NAMESPACE
