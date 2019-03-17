#include "qdemonpathutilities.h"

#include <QtCore/QVector>
#include <QtCore/QDataStream>

QT_BEGIN_NAMESPACE

using namespace QDemonPathUtilities;

void QDemonPathBuffer::save(QIODevice &outStream) const
{
    QDataStream out(&outStream);

    out << getFileTag();
    out << getFileVersion();
    out << quint32(commands.size());
    out << quint32(data.size());
    int commandSize = commands.size()*sizeof(PathCommand);
    out.writeRawData(reinterpret_cast<const char *>(commands.begin()), commandSize);

    // ensure the floats following the commands are aligned to a 4 byte boundary
    while (commandSize & (sizeof(float) - 1)) {
        out << quint8(0);
        ++commandSize;
    }
    out.writeRawData(reinterpret_cast<const char *>(data.begin()), data.size()*sizeof(float));
}

static quint32 align4(quint32 i)
{
    return (i + 3) & ~quint32(3);
}

QDemonPathBuffer *QDemonPathBuffer::load(QIODevice &inStream)
{
    QDataStream in(&inStream);

    quint64 fileTag;
    quint32 version;
    quint32 numCommands;
    quint32 numData;
    in >> fileTag;
    in >> version;
    in >> numCommands;
    in >> numData;
    if (fileTag != getFileTag()) {
        qCritical("Invalid file, not a path file");
        return nullptr;
    }
    if (version > getFileVersion()) {
        qCritical("Version number out of range.");
        return nullptr;
    }
    quint32 commandSize = numCommands * sizeof(PathCommand);
    quint32 dataSize = numData * sizeof(float);
    quint32 objectSize = sizeof(QDemonPathBuffer);
    quint32 allocSize = objectSize + commandSize + dataSize;
    char *rawData = reinterpret_cast<char *>(::malloc(allocSize));
    QDemonPathBuffer *retval = new (rawData) QDemonPathBuffer();
    char *commandBuffer = rawData + sizeof(QDemonPathBuffer);
    char *dataBuffer = commandBuffer + align4(commandSize);
    in.readRawData(commandBuffer, commandSize);
    in.readRawData(dataBuffer, dataSize);
    retval->commands = toDataRef((PathCommand *)commandBuffer, numCommands);
    retval->data = toDataRef((float *)dataBuffer, numData);
    return retval;
}

QT_END_NAMESPACE
