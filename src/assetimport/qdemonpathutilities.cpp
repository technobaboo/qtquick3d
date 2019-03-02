#include "qdemonpathutilities.h"

#include <QtCore/QVector>
#include <QtCore/QDataStream>

QT_BEGIN_NAMESPACE

using namespace QDemonPathUtilities;

QDemonPathBuffer::QDemonPathBuffer()
{

}

void QDemonPathBuffer::save(QIODevice &outStream) const
{
    QDataStream out(&outStream);

    out << getFileTag();
    out << getFileVersion();
    out << quint32(commands.size());
    out << quint32(data.size());
    out.writeRawData(reinterpret_cast<const char *>(commands.begin()), commands.size());
    out.writeRawData(reinterpret_cast<const char *>(data.begin()), data.size());
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
    quint32 commandSize = numCommands * sizeof(quint32);
    quint32 dataSize = numData * sizeof(float);
    quint32 objectSize = sizeof(QDemonPathBuffer);
    quint32 allocSize = objectSize + commandSize + dataSize;
    char *rawData = reinterpret_cast<char*>(::malloc(allocSize));
    QDemonPathBuffer *retval = new (rawData) QDemonPathBuffer();
    char *commandBuffer = rawData + sizeof(QDemonPathBuffer);
    char *dataBuffer = commandBuffer + commandSize;
    in.readRawData(commandBuffer, commandSize);
    in.readRawData(dataBuffer, dataSize);
    retval->commands = toDataRef((PathCommand::Enum *)commandBuffer, numCommands);
    retval->data = toDataRef((float *)dataBuffer, numData);
    return retval;
}

namespace {
struct QDemonPathBufferBuilder : public QDemonPathBufferBuilderInterface
{
    QVector<PathCommand::Enum> m_commands;
    QVector<float> m_data;

    void clear() override
    {
        m_commands.clear();
        m_data.clear();
    }

    void push(const QVector2D &inPos)
    {
        m_data.push_back(inPos.x());
        m_data.push_back(inPos.y());
    }

    void moveTo(const QVector2D &inPos) override
    {
        m_commands.push_back(PathCommand::MoveTo);
        push(inPos);
    }

    void cubicCurveTo(const QVector2D &inC1, const QVector2D &inC2, const QVector2D &inP2) override
    {
        m_commands.push_back(PathCommand::CubicCurveTo);
        push(inC1);
        push(inC2);
        push(inP2);
    }

    void close() override { m_commands.push_back(PathCommand::Close); }

    // Points back to internal data structures, must use or copy.
    QDemonPathBuffer getPathBuffer() override
    {
        QDemonPathBuffer retval;
        retval.data = toConstDataRef(static_cast<const float *>(m_data.constData()), m_data.size());
        retval.commands = toConstDataRef(static_cast<const PathCommand::Enum *>(m_commands.constData()), m_commands.size());;
        return retval;
    }
};
}

QDemonPathBufferBuilderInterface::~QDemonPathBufferBuilderInterface()
{

}

QDemonRef<QDemonPathBufferBuilderInterface> QDemonPathBufferBuilderInterface::createBuilder()
{
    return QDemonRef<QDemonPathBufferBuilderInterface>(new QDemonPathBufferBuilder());
}

QT_END_NAMESPACE
