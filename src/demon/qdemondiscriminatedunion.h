#ifndef QDEMONDISCRIMINATEDUNION_H
#define QDEMONDISCRIMINATEDUNION_H

#include <QtDemon/qtdemonglobal.h>
#include <QtDemon/qdemonunioncast.h>

QT_BEGIN_NAMESPACE

template <typename TUnionTraits, int TBufferSize>
class DiscriminatedUnion
{
public:
    typedef DiscriminatedUnion<TUnionTraits, TBufferSize> TThisType;
    typedef TUnionTraits TTraits;
    typedef typename TTraits::TIdType TIdType;

protected:
    char m_Data[TBufferSize];
    // Id type must include a no-data type.
    TIdType m_DataType;

public:
    DiscriminatedUnion() { TTraits::defaultConstruct(m_Data, m_DataType); }

    DiscriminatedUnion(const TThisType &inOther)
        : m_DataType(inOther.m_DataType)
    {
        TTraits::copyConstruct(m_Data, inOther.m_Data, m_DataType);
    }

    template <typename TDataType>
    DiscriminatedUnion(const TDataType &inType)
    {
        TTraits::copyConstruct(m_Data, m_DataType, inType);
    }

    ~DiscriminatedUnion() { TTraits::destruct(m_Data, m_DataType); }

    TThisType &operator=(const TThisType &inType)
    {
        if (this != &inType) {
            TTraits::destruct(m_Data, m_DataType);
            m_DataType = inType.m_DataType;
            TTraits::copyConstruct(m_Data, inType.m_Data, inType.m_DataType);
        }
        return *this;
    }

    typename TTraits::TIdType getType() const { return m_DataType; }

    template <typename TDataType>
    const TDataType *getDataPtr() const
    {
        return TTraits::template getDataPtr<TDataType>(m_Data, m_DataType);
    }

    template <typename TDataType>
    TDataType *getDataPtr()
    {
        return TTraits::template getDataPtr<TDataType>(m_Data, m_DataType);
    }

    template <typename TDataType>
    TDataType getData() const
    {
        const TDataType *dataPtr = getDataPtr<TDataType>();
        if (dataPtr)
            return *dataPtr;
        QT3DS_ASSERT(false);
        return TDataType();
    }

    bool operator==(const TThisType &inOther) const
    {
        return m_DataType == inOther.m_DataType
            && TTraits::areEqual(m_Data, inOther.m_Data, m_DataType);
    }

    bool operator!=(const TThisType &inOther) const
    {
        return m_DataType != inOther.m_DataType
            || TTraits::areEqual(m_Data, inOther.m_Data, m_DataType) == false;
    }

    template <typename TRetType, typename TVisitorType>
    TRetType visit(TVisitorType inVisitor)
    {
        return TTraits::template visit<TRetType>(m_Data, m_DataType, inVisitor);
    }

    template <typename TRetType, typename TVisitorType>
    TRetType visit(TVisitorType inVisitor) const
    {
        return TTraits::template visit<TRetType>(m_Data, m_DataType, inVisitor);
    }
};

// Helper system to enable quicker and correct construction of union traits types

struct CopyConstructVisitor
{
    const char *m_Src;
    CopyConstructVisitor(const char *inSrc)
        : m_Src(inSrc)
    {
    }

    template <typename TDataType>
    void operator()(TDataType &inDst)
    {
        new (&inDst) TDataType(*QDemonUnionCast<const TDataType *>(m_Src));
    }
    void operator()() { Q_ASSERT(false); }
};

template <typename TDataType>
struct DestructTraits
{
    void destruct(TDataType &inType) { inType.~TDataType(); }
};

// Until compilers improve a bit, you need this for POD types else you get
// unused parameter warnings.
template <>
struct DestructTraits<quint8>
{
    void destruct(quint8 &) {}
};
template <>
struct DestructTraits<qint8>
{
    void destruct(qint8 &) {}
};
template <>
struct DestructTraits<quint16>
{
    void destruct(quint16 &) {}
};
template <>
struct DestructTraits<qint16>
{
    void destruct(qint16 &) {}
};
template <>
struct DestructTraits<quint32>
{
    void destruct(quint32 &) {}
};
template <>
struct DestructTraits<qint32>
{
    void destruct(qint32 &) {}
};
template <>
struct DestructTraits<quint64>
{
    void destruct(quint64 &) {}
};
template <>
struct DestructTraits<qint64>
{
    void destruct(qint64 &) {}
};
template <>
struct DestructTraits<float>
{
    void destruct(float &) {}
};
template <>
struct DestructTraits<double>
{
    void destruct(double &) {}
};
template <>
struct DestructTraits<bool>
{
    void destruct(bool &) {}
};
template <>
struct DestructTraits<void *>
{
    void destruct(void *&) {}
};
#ifdef __INTEGRITY
template <>
struct DestructTraits<QVector2D>
{
    void destruct(QVector2D &) {}
};
template <>
struct DestructTraits<QVector3D>
{
    void destruct(QVector3D &) {}
};
#endif

struct DestructVisitor
{
    template <typename TDataType>
    void operator()(TDataType &inDst)
    {
        DestructTraits<TDataType>().destruct(inDst);
    }
    void operator()() { Q_ASSERT(false); }
};

template <typename TDataType>
struct EqualVisitorTraits
{
    bool operator()(const TDataType &lhs, const TDataType &rhs) { return lhs == rhs; }
};

struct EqualVisitor
{
    const char *m_Rhs;
    EqualVisitor(const char *rhs)
        : m_Rhs(rhs)
    {
    }
    template <typename TDataType>
    bool operator()(const TDataType &lhs)
    {
        const TDataType &rhs(*QDemonUnionCast<const TDataType *>(m_Rhs));
        return EqualVisitorTraits<TDataType>()(lhs, rhs);
    }
    bool operator()()
    {
        Q_ASSERT(false);
        return true;
    }
};

template <typename TBase, quint32 TBufferSize>
struct DiscriminatedUnionGenericBase : public TBase
{
    typedef typename TBase::TIdType TIdType;

    static void zeroBuf(char *outDst) { memZero(outDst, TBufferSize); }

    static void defaultConstruct(char *outDst, TIdType &outType)
    {
        zeroBuf(outDst);
        outType = TBase::getNoDataId();
    }

    template <typename TDataType>
    static void copyConstruct(char *outDst, TIdType &outType, const TDataType &inSrc)
    {
        zeroBuf(outDst);
        outType = TBase::template getType<TDataType>();
        new (outDst) TDataType(inSrc);
    }

    static void copyConstruct(char *inDst, const char *inSrc, TIdType inType)
    {
        if (inType == TBase::getNoDataId())
            zeroBuf(inDst);
        else
            TBase::template visit<void>(inDst, inType, CopyConstructVisitor(inSrc));
    }

    static void destruct(char *inDst, TIdType inType)
    {
        if (inType != TBase::getNoDataId())
            TBase::template visit<void>(inDst, inType, DestructVisitor());
        zeroBuf(inDst);
    }

    template <typename TDataType>
    static const TDataType *getDataPtr(const char *inData, const TIdType &inType)
    {
        if (TBase::template getType<TDataType>() == inType)
            return QDemonUnionCast<const TDataType *>(inData);
        Q_ASSERT(false);
        return NULL;
    }

    template <typename TDataType>
    static TDataType *getDataPtr(char *inData, const TIdType &inType)
    {
        if (TBase::template getType<TDataType>() == inType)
            return QDemonUnionCast<TDataType *>(inData);
        Q_ASSERT(false);
        return NULL;
    }

    static bool areEqual(const char *inLhs, const char *inRhs, TIdType inType)
    {
        if (inType != TBase::getNoDataId())
            return TBase::template visit<bool>(inLhs, inType, EqualVisitor(inRhs));
        else
            return true;
    }
};

QT_END_NAMESPACE

#endif // QDEMONDISCRIMINATEDUNION_H
