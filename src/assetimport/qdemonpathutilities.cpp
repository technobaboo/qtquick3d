#include "qdemonpathutilities.h"

#include <QtCore/QVector>
#include <QtCore/QDataStream>

QT_BEGIN_NAMESPACE

using namespace QDemonPathUtilities;

SPathBuffer::SPathBuffer()
{

}

void SPathBuffer::Save(QIODevice &outStream) const
{
    QDataStream out(&outStream);

    out << GetFileTag();
    out << GetFileVersion();
    out << quint32(m_Commands.size());
    out << quint32(m_Data.size());
    out.writeRawData(reinterpret_cast<const char *>(m_Commands.begin()), m_Commands.size());
    out.writeRawData(reinterpret_cast<const char *>(m_Data.begin()), m_Data.size());
}

SPathBuffer *SPathBuffer::Load(QIODevice &inStream)
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
    if (fileTag != GetFileTag()) {
        qCritical("Invalid file, not a path file");
        return nullptr;
    }
    if (version > GetFileVersion()) {
        qCritical("Version number out of range.");
        return nullptr;
    }
    quint32 commandSize = numCommands * sizeof(quint32);
    quint32 dataSize = numData * sizeof(float);
    quint32 objectSize = sizeof(SPathBuffer);
    quint32 allocSize = objectSize + commandSize + dataSize;
    char *rawData = reinterpret_cast<char*>(::malloc(allocSize));
    SPathBuffer *retval = new (rawData) SPathBuffer();
    char *commandBuffer = rawData + sizeof(SPathBuffer);
    char *dataBuffer = commandBuffer + commandSize;
    in.readRawData(commandBuffer, commandSize);
    in.readRawData(dataBuffer, dataSize);
    retval->m_Commands = toDataRef((PathCommand::Enum *)commandBuffer, numCommands);
    retval->m_Data = toDataRef((float *)dataBuffer, numData);
    return retval;
}

namespace {
struct SBuilder : public IPathBufferBuilder
{
    QVector<PathCommand::Enum> m_Commands;
    QVector<float> m_Data;

    SBuilder()
    {
    }

    void Clear() override
    {
        m_Commands.clear();
        m_Data.clear();
    }

    void Push(const QVector2D &inPos)
    {
        m_Data.push_back(inPos.x());
        m_Data.push_back(inPos.y());
    }

    void MoveTo(const QVector2D &inPos) override
    {
        m_Commands.push_back(PathCommand::MoveTo);
        Push(inPos);
    }

    void CubicCurveTo(const QVector2D &inC1, const QVector2D &inC2, const QVector2D &inP2) override
    {
        m_Commands.push_back(PathCommand::CubicCurveTo);
        Push(inC1);
        Push(inC2);
        Push(inP2);
    }

    void Close() override { m_Commands.push_back(PathCommand::Close); }

    // Points back to internal data structures, must use or copy.
    SPathBuffer GetPathBuffer() override
    {
        SPathBuffer retval;
        retval.m_Data = toConstDataRef(static_cast<const float *>(m_Data.constData()), m_Data.size());
        retval.m_Commands = toConstDataRef(static_cast<const PathCommand::Enum *>(m_Commands.constData()), m_Commands.size());;
        return retval;
    }
};
}

QSharedPointer<IPathBufferBuilder> IPathBufferBuilder::CreateBuilder()
{
    return QSharedPointer<IPathBufferBuilder>(new SBuilder());
}

QT_END_NAMESPACE
