#ifndef QDEMONPERFRAMEALLOCATOR_H
#define QDEMONPERFRAMEALLOCATOR_H

#include <QtDemonRuntimeRender/qtdemonruntimerenderglobal.h>

#include <QtCore/QVector>
#include <QtCore/QHash>

QT_BEGIN_NAMESPACE

template<quint32 alignmentInBytes = 4, quint32 slabSize = 8192>
struct QDemonFastAllocator
{
    QVector<quint8 *> m_slabs;
    quint32 m_Offset = 0;

    enum {
        SlabSize = slabSize,
    };

    static size_t getSlabSize() { return slabSize; }

    QDemonFastAllocator() {}

    ~QDemonFastAllocator()
    {
        for (quint32 idx = 0, end = m_slabs.size(); idx < end; ++idx)
            ::free(m_slabs[idx]);
        m_slabs.clear();
        m_Offset = 0;
    }
    void *allocate(size_t inSize)
    {
        if (inSize > slabSize) {
            Q_ASSERT(false);
            return nullptr;
        }
        quint32 misalign = m_Offset % alignmentInBytes;
        if (misalign)
            m_Offset = m_Offset + (alignmentInBytes - misalign);

        quint32 currentSlab = m_Offset / slabSize;
        quint32 slabOffset = m_Offset % slabSize;
        quint32 amountLeftInSlab = slabSize - slabOffset;
        if (inSize > amountLeftInSlab) {
            ++currentSlab;
            slabOffset = 0;
            m_Offset = currentSlab * slabSize;
        }
        while (currentSlab >= quint32(m_slabs.size())) {
            m_slabs.push_back(reinterpret_cast<quint8 *>(::malloc(slabSize)));
        }
        quint8 *data = m_slabs[currentSlab] + slabOffset;
        // This would indicate the underlying allocator isn't handing back aligned memory.
        Q_ASSERT(reinterpret_cast<size_t>(data) % alignmentInBytes == 0);
        m_Offset += quint32(inSize);
        return data;
    }

    void *allocate(size_t size, size_t alignment, size_t /*alignmentOffset*/)
    {
        Q_ASSERT(alignment == alignmentInBytes);
        if (alignment == alignmentInBytes)
            return allocate(size);
        return nullptr;
    }
    // only reset works with deallocation
    void deallocate(void *) {}
    void reset() { m_Offset = 0; }
};

struct QDemonAutoDeallocatorAllocator
{
    QHash<void *, size_t> m_allocations;
    QDemonAutoDeallocatorAllocator() {}

    // Automatically deallocates everything that hasn't already been deallocated.
    ~QDemonAutoDeallocatorAllocator() { deallocateAllAllocations(); }

    void deallocateAllAllocations()
    {
        for (const auto key : m_allocations.keys()) {
            ::free(key);
        }
        m_allocations.clear();
    }

    void *allocate(size_t size)
    {
        void *value = ::malloc(size);
        m_allocations.insert(value, size);
        return value;
    }

    void *allocate(size_t size, size_t alignment, size_t alignmentOffset)
    {
        // TODO: Ignores alignment
        Q_UNUSED(alignment)
        Q_UNUSED(alignmentOffset)

        void *value = ::malloc(size);
        m_allocations.insert(value, size);
        return value;
    }
    void deallocate(void *ptr)
    {
        m_allocations.remove(ptr);
        ::free(ptr);
    }
};

struct QDemonPerFrameAllocator
{
    QDemonFastAllocator<> m_fastAllocator;
    QDemonAutoDeallocatorAllocator m_largeAllocator;

public:
    QDemonPerFrameAllocator() {}

    inline void *allocate(size_t inSize)
    {
        if (inSize < 8192)
            return m_fastAllocator.allocate(inSize);
        else
            return m_largeAllocator.allocate(inSize);
    }

    inline void deallocate(void *, size_t) {}

    void reset()
    {
        m_fastAllocator.reset();
        m_largeAllocator.deallocateAllAllocations();
    }

    void *allocate(size_t inSize, size_t alignment, size_t alignmentOffset)
    {
        if (inSize < QDemonFastAllocator<>::SlabSize)
            return m_fastAllocator.allocate(inSize, alignment, alignmentOffset);
        else
            return m_largeAllocator.allocate(inSize, alignment, alignmentOffset);
    }

    void deallocate(void *) {}
};

QT_END_NAMESPACE

#endif // QDEMONPERFRAMEALLOCATOR_H
