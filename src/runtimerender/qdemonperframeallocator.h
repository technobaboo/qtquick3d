#ifndef QDEMONPERFRAMEALLOCATOR_H
#define QDEMONPERFRAMEALLOCATOR_H

#include <QtDemonRuntimeRender/qtdemonruntimerenderglobal.h>

QT_BEGIN_NAMESPACE

class QDemonPerFrameAllocator
{
    struct FastAllocator
    {
        struct Slab;

        enum : size_t {
            ChunkSize = 8192*2,
            Alignment = 4,
            SlabSize = ChunkSize - sizeof(Slab *),
            MaxAlloc = ChunkSize/2 // don't go all the way up to SlabSize, or we'd almost always get a big hole
        };
        struct Slab {
            Slab() = default;
            Slab(Slab *previous)
            {
                previous->next = this;
            }
            Slab *next = nullptr;
            quint8 data[SlabSize];
        };
        Q_STATIC_ASSERT(sizeof(Slab) == ChunkSize);

        Slab *first = nullptr;
        Slab *current = nullptr;
        size_t offset = 0;

        FastAllocator()
        {
            first = current = new Slab;
        }

        ~FastAllocator()
        {
            Slab *s = first;
            while (s) {
                Slab *n = s->next;
                delete s;
                s = n;
            }
        }
        void *allocate(size_t size)
        {
            size = (size + Alignment - 1) & ~(Alignment - 1);
            Q_ASSERT(size <= SlabSize);
            Q_ASSERT(!(offset % Alignment));

            size_t amountLeftInSlab = SlabSize - offset;
            if (size > amountLeftInSlab) {
                if (current->next)
                    current = current->next;
                else
                    current = new Slab(current);
                offset = 0;
            }

            quint8 *data = current->data + offset;
            offset += size;
            return data;
        }

        // only reset, so we can re-use the memory
        void reset() { current = first; offset = 0; }
    };

    struct LargeAllocator
    {
        struct Slab {
            Slab *next = nullptr;
        };
        Slab *current = nullptr;

        LargeAllocator() {}

        // Automatically deallocates everything that hasn't already been deallocated.
        ~LargeAllocator() { deallocateAll(); }

        void deallocateAll()
        {
            while (current) {
                Slab *n = current->next;
                ::free(current);
                current = n;
            }
            current = nullptr;
        }

        void *allocate(size_t size)
        {
            quint8 *mem = reinterpret_cast<quint8 *>(::malloc(sizeof(Slab) + size));
            Slab *s = reinterpret_cast<Slab *>(mem);
            s->next = current;
            return mem + sizeof(Slab);
        }
    };

    FastAllocator m_fastAllocator;
    LargeAllocator m_largeAllocator;

public:
    QDemonPerFrameAllocator() {}

    inline void *allocate(size_t size)
    {
        if (size < FastAllocator::MaxAlloc)
            return m_fastAllocator.allocate(size);
        else
            return m_largeAllocator.allocate(size);
    }

    void reset()
    {
        m_fastAllocator.reset();
        m_largeAllocator.deallocateAll();
    }
};

QT_END_NAMESPACE

#endif // QDEMONPERFRAMEALLOCATOR_H
