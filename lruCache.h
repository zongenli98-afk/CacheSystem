#pragma once

#include <cstring>
#include <list>
#include <unordered_map>
#include <memory>
#include <mutex>

namespace myCache
{

    template <typename K, typename V> class LruCache;
    template <typename K, typename V>
    class LruNode
    {
    private:
        K key_;
        V value_;
        size_t accessCount_;
        std::weak_ptr<LruNode<K, V>> prev_;
        std::shared_ptr<LruNode<K, V>> next_;
    public:
        LruNode(K key, V value)
          : key_(key)
          , value_(value)
          , accessCount_(1)
        {}
        K getKey() const {return key_;}
        V getValue() const {return value_;}
        void setValue(const V& value) {value_ = value;}
        size_t getAccessCount() const {return accessCount_;}
        void incrementAccessCount() {++accessCount_;}

        friend class LruCache<K, V>;
    };

    template <typename K, typename V>
    class LruCache : public cachePolicy<K, V>
    {
    public:
        using LruNodeType = LruNode<K, V>;
        using NodePtr = std::shared_ptr<LruNodeType>;
        using NodeMap = std::unordered_map<K, NodePtr>;

        LruCache(int capacity)
          : capacity_(capacity)
        {
            initializeList();
        }

        ~LruCache() override = default;
        
        void set(K key, V value) override
        {
            if (capacity_ <= 0)
                return;
            
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end())
            {
                updateExisitingNode(it->second, value);
                return;
            }
            addNewNode(key, value);
        }

        bool get(K key, V& value) override
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end())
            {
                moveToMostRecent(it->second);
                value = it->second->getValue();
                return true;
            }
            return false;
        }

        V get(K key) override
        {
            V value{};
            // memset(&value, 0, sizeof(value));   // memset 是按字节设置内存的，对于复杂类型（如 string）使用 memset 可能会破坏对象的内部结构
            get(key, value);
            return value;
        }

        // 删除指定元素
        void remove(K key) 
        {   
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end())
            {
                removeNode(it->second);
                nodeMap_.erase(it);
            }
        }
    
    private:
        void initializeList()
        {
            dummyHead_ = std::make_shared<LruNodeType>(K(), V());
            dummyTail_ = std::make_shared<LruNodeType>(K(), V());
            dummyHead_->next_ = dummyTail_;
            dummyTail_->prev_ = dummyHead_;
        }

        void updateExisitingNode(NodePtr node, const V& value)
        {
            node->setValue(value);
            // move to most recent
        }

        void addNewNode(const K& key, const V& value)
        {
            if (nodeMap_.size() >= capacity_)
            {
                evictLeastRecent();
            }

            NodePtr newNode = std::make_shared<LruNodeType>(key, value);
            insertNode(newNode);
            nodeMap_[key] = newNode;
        }

        void moveToMostRecent(NodePtr node)
        {
            removeNode(node);
            insertNode(node);
        }

        void removeNode(NodePtr node)
        {
            if(!node->prev_.expired() && node->next_)
            {
                auto prev = node->prev_.lock();
                prev->next_ = node->next_;
                node->next_->prev_ = prev;
                node->next_ = nullptr;
            }
        }

        // insert from tail
        void insertNode(NodePtr node)
        {
            node->next_ = dummyTail_;
            node->prev_ = dummyTail_->prev_;
            dummyTail_->prev_.lock()->next_ = node;
            dummyTail_->prev_ = node;
        }

        void evictLeastRecent()
        {
            NodePtr leastRecent = dummyHead_->next_;
            removeNode(leastRecent);
            nodeMap_.erase(leastRecent->getKey());
        }

    private:
        int capacity_;
        NodeMap nodeMap_;
        std::mutex mutex_;
        NodePtr dummyHead_;
        NodePtr dummyTail_;
    };
};