
#ifndef CARBON_ALLOCATOR_HPP
#define CARBON_ALLOCATOR_HPP

#include <cstddef>
#include <cstdlib>

class Allocator{
public:
    inline explicit Allocator(size_t bytes);
    inline ~Allocator();
    template<typename T>
    inline T* alloc();
private:
    size_t m_size;
    std::byte* m_buffer;
    std::byte* m_head;
};



Allocator::Allocator(size_t bytes):m_size(bytes) {
    m_buffer = static_cast<std::byte *>(malloc(m_size));
    m_head = m_buffer;
}

Allocator::~Allocator() {
    free(m_buffer);
}

template<typename T>
inline T *Allocator::alloc() {
    std::byte* head = m_head;
    m_head += sizeof(T);
    return reinterpret_cast<T*>(head);
}

#endif //CARBON_ALLOCATOR_HPP
