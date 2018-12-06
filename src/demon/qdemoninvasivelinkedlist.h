#ifndef QDEMONINVASIVELINKEDLIST_H
#define QDEMONINVASIVELINKEDLIST_H

#include <QtDemon/qtdemonglobal.h>

QT_BEGIN_NAMESPACE

// Base linked list without an included head or tail member.
template <typename TObjType, typename TObjHeadOp, typename TObjTailOp>
struct SInvasiveLinkListBase
{
    TObjType *tail(TObjType *inObj)
    {
        if (inObj)
            return TObjTailOp().get(inObj);
        return nullptr;
    }

    TObjType *head(TObjType *inObj)
    {
        if (inObj)
            return TObjHeadOp().get(inObj);
        return nullptr;
    }

    const TObjType *tail(const TObjType *inObj)
    {
        if (inObj)
            return TObjTailOp().get(inObj);
        return nullptr;
    }

    const TObjType *head(const TObjType *inObj)
    {
        if (inObj)
            return TObjHeadOp().get(inObj);
        return nullptr;
    }

    void remove(TObjType &inObj)
    {
        TObjHeadOp theHeadOp;
        TObjTailOp theTailOp;
        TObjType *theHead = theHeadOp.get(inObj);
        TObjType *theTail = theTailOp.get(inObj);
        if (theHead)
            theTailOp.set(*theHead, theTail);
        if (theTail)
            theHeadOp.set(*theTail, theHead);
        theHeadOp.set(inObj, nullptr);
        theTailOp.set(inObj, nullptr);
    }

    void insert_after(TObjType &inPosition, TObjType &inObj)
    {
        TObjTailOp theTailOp;
        TObjType *theHead = &inPosition;
        TObjType *theTail = theTailOp.get(inPosition);
        insert(theHead, theTail, inObj);
    }

    void insert_before(TObjType &inPosition, TObjType &inObj)
    {
        TObjHeadOp theHeadOp;
        TObjType *theHead = theHeadOp.get(inPosition);
        TObjType *theTail = &inPosition;
        insert(theHead, theTail, inObj);
    }

    void insert(TObjType *inHead, TObjType *inTail, TObjType &inObj)
    {
        TObjHeadOp theHeadOp;
        TObjTailOp theTailOp;
        if (inHead)
            theTailOp.set(*inHead, &inObj);
        if (inTail)
            theHeadOp.set(*inTail, &inObj);
        theHeadOp.set(inObj, inHead);
        theTailOp.set(inObj, inTail);
    }
};

template <typename TObjType, typename TObjTailOp>
struct SLinkedListIterator
{
    typedef SLinkedListIterator<TObjType, TObjTailOp> TMyType;
    TObjType *m_Obj;
    SLinkedListIterator(TObjType *inObj = nullptr)
        : m_Obj(inObj)
    {
    }

    bool operator!=(const TMyType &inIter) const { return m_Obj != inIter.m_Obj; }
    bool operator==(const TMyType &inIter) const { return m_Obj == inIter.m_Obj; }

    TMyType &operator++()
    {
        if (m_Obj)
            m_Obj = TObjTailOp().get(*m_Obj);
        return *this;
    }

    TMyType &operator++(int)
    {
        TMyType retval(*this);
        ++(*this);
        return retval;
    }

    TObjType &operator*() { return *m_Obj; }
    TObjType *operator->() { return m_Obj; }
};

// Used for singly linked list where
// items have either no head or tail ptr.
template <typename TObjType>
struct SNullOp
{
    void set(TObjType &, TObjType *) {}
    TObjType *get(const TObjType &) { return nullptr; }
};

template <typename TObjType, typename TObjTailOp>
struct InvasiveSingleLinkedList
        : public SInvasiveLinkListBase<TObjType, SNullOp<TObjType>, TObjTailOp>
{
    typedef InvasiveSingleLinkedList<TObjType, TObjTailOp> TMyType;
    typedef SInvasiveLinkListBase<TObjType, SNullOp<TObjType>, TObjTailOp> TBaseType;
    typedef SLinkedListIterator<TObjType, TObjTailOp> iterator;
    typedef iterator const_iterator;
    TObjType *m_Head;
    InvasiveSingleLinkedList()
        : m_Head(nullptr)
    {
    }
    InvasiveSingleLinkedList(const TMyType &inOther)
        : m_Head(inOther.m_Head)
    {
    }
    TMyType &operator=(const TMyType &inOther)
    {
        m_Head = inOther.m_Head;
        return *this;
    }

    TObjType &front() const { return *m_Head; }

    void push_front(TObjType &inObj)
    {
        if (m_Head != nullptr)
            TBaseType::insert_before(*m_Head, inObj);
        m_Head = &inObj;
    }

    void push_back(TObjType &inObj)
    {
        if (m_Head == nullptr)
            m_Head = &inObj;
        else {
            TObjType *lastObj = nullptr;
            for (iterator iter = begin(), endIter = end(); iter != endIter; ++iter)
                lastObj = &(*iter);

            QT3DS_ASSERT(lastObj);
            if (lastObj)
                TObjTailOp().set(*lastObj, &inObj);
        }
    }

    void remove(TObjType &inObj)
    {
        if (m_Head == &inObj)
            m_Head = TObjTailOp().get(inObj);
        TBaseType::remove(inObj);
    }

    bool empty() const { return m_Head == nullptr; }

    iterator begin() { return iterator(m_Head); }
    iterator end() { return iterator(nullptr); }

    const_iterator begin() const { return iterator(m_Head); }
    const_iterator end() const { return iterator(nullptr); }
};

template <typename TObjType, typename TObjHeadOp, typename TObjTailOp>
struct InvasiveLinkedList : public SInvasiveLinkListBase<TObjType, TObjHeadOp, TObjTailOp>
{
    typedef InvasiveLinkedList<TObjType, TObjHeadOp, TObjTailOp> TMyType;
    typedef SInvasiveLinkListBase<TObjType, TObjHeadOp, TObjTailOp> TBaseType;
    typedef SLinkedListIterator<TObjType, TObjTailOp> iterator;
    typedef iterator const_iterator;
    typedef SLinkedListIterator<TObjType, TObjHeadOp> reverse_iterator;
    typedef reverse_iterator const_reverse_iterator;

    TObjType *m_Head;
    TObjType *m_Tail;

    InvasiveLinkedList()
        : m_Head(nullptr)
        , m_Tail(nullptr)
    {
    }
    InvasiveLinkedList(const TMyType &inOther)
        : m_Head(inOther.m_Head)
        , m_Tail(inOther.m_Tail)
    {
    }
    TMyType &operator=(const TMyType &inOther)
    {
        m_Head = inOther.m_Head;
        m_Tail = inOther.m_Tail;
        return *this;
    }

    TObjType &front() const
    {
        QT3DS_ASSERT(m_Head);
        return *m_Head;
    }
    TObjType &back() const
    {
        QT3DS_ASSERT(m_Tail);
        return *m_Tail;
    }

    TObjType *front_ptr() const { return m_Head; }
    TObjType *back_ptr() const { return m_Tail; }

    void push_front(TObjType &inObj)
    {
        if (m_Head != nullptr)
            TBaseType::insert_before(*m_Head, inObj);
        m_Head = &inObj;

        if (m_Tail == nullptr)
            m_Tail = &inObj;
    }

    void push_back(TObjType &inObj)
    {
        if (m_Tail != nullptr)
            TBaseType::insert_after(*m_Tail, inObj);
        m_Tail = &inObj;

        if (m_Head == nullptr)
            m_Head = &inObj;
    }

    void remove(TObjType &inObj)
    {
        if (m_Head == &inObj)
            m_Head = TObjTailOp().get(inObj);
        if (m_Tail == &inObj)
            m_Tail = TObjHeadOp().get(inObj);

        TBaseType::remove(inObj);
    }

    bool empty() const { return m_Head == nullptr; }

    iterator begin() { return iterator(m_Head); }
    iterator end() { return iterator(nullptr); }

    const_iterator begin() const { return iterator(m_Head); }
    const_iterator end() const { return iterator(nullptr); }

    reverse_iterator rbegin() { return reverse_iterator(m_Tail); }
    reverse_iterator rend() { return reverse_iterator(nullptr); }

    const_reverse_iterator rbegin() const { return reverse_iterator(m_Tail); }
    const_reverse_iterator rend() const { return reverse_iterator(nullptr); }
};

// Macros to speed up the definitely of invasive linked lists.
#define DEFINE_INVASIVE_SINGLE_LIST(type)                                                          \
    struct S##type;                                                                                \
    struct S##type##NextOp                                                                         \
{                                                                                              \
    S##type *get(S##type &s);                                                                  \
    const S##type *get(const S##type &s) const;                                                \
    void set(S##type &inItem, S##type *inNext);                                                \
    };                                                                                             \
    typedef InvasiveSingleLinkedList<S##type, S##type##NextOp> T##type##List;

#define DEFINE_INVASIVE_LIST(type)                                                                 \
    struct S##type;                                                                                \
    struct S##type##NextOp                                                                         \
{                                                                                              \
    S##type *get(S##type &s);                                                                  \
    const S##type *get(const S##type &s) const;                                                \
    void set(S##type &inItem, S##type *inNext);                                                \
    };                                                                                             \
    struct S##type##PreviousOp                                                                     \
{                                                                                              \
    S##type *get(S##type &s);                                                                  \
    const S##type *get(const S##type &s) const;                                                \
    void set(S##type &inItem, S##type *inNext);                                                \
    };                                                                                             \
    typedef InvasiveLinkedList<S##type, S##type##PreviousOp, S##type##NextOp> T##type##List;

#define IMPLEMENT_INVASIVE_LIST(type, prevMember, nextMember)                                      \
    inline S##type *S##type##NextOp::get(S##type &s) { return s.nextMember; }                      \
    inline const S##type *S##type##NextOp::get(const S##type &s) const { return s.nextMember; }    \
    inline void S##type##NextOp::set(S##type &inItem, S##type *inNext)                             \
{                                                                                              \
    inItem.nextMember = inNext;                                                                \
    }                                                                                              \
    inline S##type *S##type##PreviousOp::get(S##type &s) { return s.prevMember; }                  \
    inline const S##type *S##type##PreviousOp::get(const S##type &s) const                         \
{                                                                                              \
    return s.prevMember;                                                                       \
    }                                                                                              \
    inline void S##type##PreviousOp::set(S##type &inItem, S##type *inNext)                         \
{                                                                                              \
    inItem.prevMember = inNext;                                                                \
    }

#define IMPLEMENT_INVASIVE_SINGLE_LIST(type, nextMember)                                           \
    inline S##type *S##type##NextOp::get(S##type &s) { return s.nextMember; }                      \
    inline const S##type *S##type##NextOp::get(const S##type &s) const { return s.nextMember; }    \
    inline void S##type##NextOp::set(S##type &inItem, S##type *inNext)                             \
{                                                                                              \
    inItem.nextMember = inNext;                                                                \
    }

QT_END_NAMESPACE

#endif // QDEMONINVASIVELINKEDLIST_H
