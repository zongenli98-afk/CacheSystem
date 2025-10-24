#pragma once

namespace myCache
{

template <typename K, typename V>
class cachePolicy
{
public:
    virtual ~cachePolicy() = default;
    virtual void set(K key, V value) = 0; 
    virtual bool get(K key, V &value) = 0;
    virtual V get(K key) = 0;
};

} // namespace myCache