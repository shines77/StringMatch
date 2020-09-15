
#ifndef STRING_MATCH_AHO_CORASICK_H
#define STRING_MATCH_AHO_CORASICK_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "basic/stddef.h"
#include "basic/stdint.h"
#include <assert.h>

#include <cstdint>
#include <cstddef>
#include <vector>
#include <list>
#include <memory>

#include "StringMatch.h"
#include "algorithm/AlgorithmWrapper.h"
#include "jstd/scoped_ptr.h"
#include "jstd/vector.h"

#define USE_PLACEMENT_NEW   1

//
// Article:
//
// See: https://blog.csdn.net/q547550831/article/details/51860017
// See: https://blog.csdn.net/v_JULY_v/article/details/7041827
//
// Code:
//
// See: https://blog.csdn.net/creatorx/article/details/71100840
// See: https://blog.csdn.net/silence401/article/details/52662605
//

namespace StringMatch {

struct ACNode {
    typedef std::size_t size_type;
    static const size_type kMaxAscii = 256;

    ACNode * fail;
    Long cnt;
    ACNode * next[kMaxAscii];

    ACNode(ACNode * _fail = nullptr)  { init(_fail); }
    ~ACNode() { destroy(); }

    void init(ACNode * _fail = nullptr) {
        this->fail = _fail;
        this->cnt = 0;
#if 1
        memset((void *)&this->next[0], 0, kMaxAscii * sizeof(ACNode *));
#else
        for (size_type i = 0; i < kMaxAscii; ++i) {
            this->next[i] = nullptr;
        }
#endif
    }

    void destroy() {
#if USE_PLACEMENT_NEW
  #if 0
        for (size_type i = 0; i < kMaxAscii; ++i) {
            if (this->next[i] != nullptr) {
                this->next[i]->~ACNode();
                this->next[i] = nullptr;
            }
        }
  #endif
#else
        for (size_type i = 0; i < kMaxAscii; ++i) {
            if (this->next[i] != nullptr) {
                delete next[i];
                this->next[i] = nullptr;
            }
        }
#endif
    }
};

struct MemoryPool {
public:
    typedef std::size_t size_type;
    typedef ACNode      node_type;

    static const size_type kMaxSize = 16384;

private:
    bool inited_;
    std::vector<node_type *> nodes_;

public:
    MemoryPool() : inited_(false) { init(); }
    ~MemoryPool() { destroy(); }

    void init() {
        if (!this->inited_) {
            this->nodes_.reserve(kMaxSize);
            for (size_type i = 0; i < kMaxSize; ++i) {
                this->nodes_.emplace_back(new node_type());
            }
            this->inited_ = true;
        }
    }

    void destroy() {
        if (this->inited_) {
            for (size_type i = 0; i < kMaxSize; ++i) {
                if (this->nodes_[i] != nullptr) {
                    delete this->nodes_[i];
                    this->nodes_[i] = nullptr;
                }
            }
            this->inited_ = false;
        }
    }

    void * get(size_type i) {
        assert(i < kMaxSize);
        return (void *)this->nodes_[i];
    }
};

template <typename CharTy>
class AhoCorasickImpl {
public:
    typedef AhoCorasickImpl<CharTy> this_type;
    typedef CharTy                  char_type;
    typedef std::size_t             size_type;
    typedef typename jstd::uchar_traits<CharTy>::type
                                    uchar_type;
    typedef ACNode                  node_type;

    static const size_type kMaxAscii = 256;

    static MemoryPool memory_pool;
    static int pool_idx;

private:
    jstd::scoped_ptr<node_type> root_;
    jstd::vector<node_type *> queue_;

public:
    AhoCorasickImpl() : root_(nullptr) {
        this->init();
        AhoCorasickImpl::memory_pool.init();
    }
    ~AhoCorasickImpl() {
        this->destroy();
    }

    static void reset_counter() {
        AhoCorasickImpl::pool_idx = 0;
    }

    static int get_counter() {
        return AhoCorasickImpl::pool_idx;
    }

    static const char * name() { return "AhoCorasick"; }
    static bool need_preprocessing() { return true; }

    bool is_alive() const {
        return (this->root_.get() != nullptr);
    }

    void init() {
        this->root_.reset(new node_type());
        this->queue_.reserve(63);
    }

    void destroy() {
        this->root_.reset();
    }

#if 1
    void build_trie(const char_type * pattern, size_type length) {
        node_type * root = this->root_.get();
        node_type * node = root;
        assert(node != nullptr);

        for (size_type i = 0; i < length; ++i) {
            uchar_type ch = (uchar_type)*pattern++;
            assert(node != nullptr);
            if (likely(node->next[ch] == nullptr)) {
#if USE_PLACEMENT_NEW
                void * next_ptr = AhoCorasickImpl::memory_pool.get(AhoCorasickImpl::pool_idx);
                node_type * next = new (next_ptr) node_type();
                assert((void *)next == next_ptr);
                AhoCorasickImpl::pool_idx++;
#else
                node_type * next = new node_type();
#endif
                node->next[ch] = next;
                assert(next != nullptr);
                node = next;
            }
            else {
                node = node->next[ch];
            }
        }

        // Record the leaf node.
        node->cnt++;
    }

    void build_automation(const char_type * pattern, size_type length) {
        // First step: build the trie tree.
        build_trie(pattern, length);

        // Second step: build the automation.
        node_type * root = this->root_.get();
        this->queue_.emplace_back(root);

        size_type head = 0;
        while (likely(head < this->queue_.size())) {
            node_type * cur = this->queue_[head++];
            for (size_type i = 0; i < kMaxAscii; ++i) {
                node_type * next = cur->next[i];
                if (likely(next == nullptr)) {
                    continue;
                }
                else {
                    if (likely(cur == root)) {
                        next->fail = root;
                        this->queue_.emplace_back(next);
                    }
                    else {
                        node_type * node = cur->fail;
                        do {
                            if (likely(node != nullptr)) {
                                if (likely(node->next[i] == nullptr)) {
                                    node = node->fail;
                                }
                                else {
                                    next->fail = node->next[i];
                                    break;
                                }
                            }
                            else {
                                next->fail = root;
                                break;
                            }
                        } while (1);
                        this->queue_.emplace_back(next);
                    }
                }
            }
        }
    }
#else
    void build_trie(const char_type * pattern, size_type length) {
        node_type * root = this->root_.get();
        node_type * node = root;
        assert(node != nullptr);

        for (size_type i = 0; i < length; ++i) {
            uchar_type ch = (uchar_type)*pattern++;
            assert(node != nullptr);
            if (likely(node->next[ch] == nullptr)) {
#if USE_PLACEMENT_NEW
                void * next_ptr = AhoCorasickImpl::memory_pool.get(AhoCorasickImpl::pool_idx);
                node_type * next = new (next_ptr) node_type();
                assert((void *)next == next_ptr);
                AhoCorasickImpl::pool_idx++;
#else
                node_type * next = new node_type();
#endif
                node->next[ch] = next;
                assert(next != nullptr);
            }
            node = node->next[ch];
        }

        // Record the leaf node.
        node->cnt++;
    }

    void build_automation(const char_type * pattern, size_type length) {
        // First step: build the trie tree.
        build_trie(pattern, length);

        // Second step: build the automation.
        node_type * root = this->root_.get();
        this->queue_.push_back(root);

        size_type head = 0;
        while (likely(head < this->queue_.size())) {
            node_type * node = nullptr;
            node_type * cur = this->queue_[head++];
            for (size_type i = 0; i < kMaxAscii; ++i) {
                if (likely(cur->next[i] == nullptr)) {
                    continue;
                }
                else {
                    if (likely(cur == root)) {
                        cur->next[i]->fail = root;
                    }
                    else {
                        node = cur->fail;
                        while (likely(node != nullptr)) {
                            if (likely(node->next[i] == nullptr)) {
                                node = node->fail;
                            }
                            else {
                                cur->next[i]->fail = node->next[i];
                                break;
                            }
                        }
                        if (likely(node == nullptr)) {
                            cur->next[i]->fail = root;
                        }
                    }
                    this->queue_.push_back(cur->next[i]);
                }
            }
        }
    }
#endif

    /* Preprocessing */
    bool preprocessing(const char_type * pattern, size_type length) {
        bool success = false;
        assert(pattern != nullptr);

        this->build_automation(pattern, length);
        return true;
    }

    /* Searching */
    SM_NOINLINE_DECLARE(Long)
    search(const char_type * text, size_type text_len,
           const char_type * pattern, size_type pattern_len) const {
        assert(text != nullptr);
        assert(pattern != nullptr);

        if (likely(pattern_len <= text_len)) {
            node_type * root = this->root_.get();
            node_type * node = root;
            assert(root != nullptr);

            const char_type * text_start = text;
            const char_type * text_end = text + text_len;
            while (likely(text < text_end)) {
                uchar_type ch = (uchar_type)*text;
                do {
                    node_type * next = node->next[ch];
                    if (likely(next == nullptr)) {
                        // Dismatch
                        if (likely(node == root)) {
                            text++;
                            break;
                        }
                        else {
                            if (likely(node->fail != nullptr)) {
                                node = node->fail;
                            }
                            else {
                                node = root;
                                text++;
                                break;
                            }
                        }
                    }
                    else {
                        // Matched one char
                        assert(next != nullptr);
                        node = next;
                        if (likely(node->cnt <= 0)) {
                            // Isn't a leaf node.
                            text++;
                            break;
                        }
                        else {
                            // Has found
                            text++;
                            return (Long)(text - text_start - pattern_len);
                        }
                    }
                } while (1);
            }
        }

        return Status::NotFound;
    }

    /* Searching */
    SM_NOINLINE_DECLARE(Long)
    search2(const char_type * text, size_type text_len,
            const char_type * pattern, size_type pattern_len) const {
        assert(text != nullptr);
        assert(pattern != nullptr);

        if (likely(pattern_len <= text_len)) {
            ACNode * root = this->root_.get();
            ACNode * node = root;
            assert(root != nullptr);

            const char_type * text_start = text;
            const char_type * text_end = text + text_len;
            while (likely(text < text_end)) {
                uchar_type ch = (uchar_type)*text;
                while (likely(node->next[ch] == nullptr && node != root)) {
                    if (likely(node->fail != nullptr)) {
                        node = node->fail;
                    }
                    else {
                        node = root;
                        break;
                    }
                }
                node = node->next[ch];
                if (likely(node == nullptr)) {
                    node = root;
                    text++;
                }
                else {
                    if (likely(node->cnt <= 0)) {
                        text++;
                    }
                    else {
                        // Has found
                        text++;
                        return (Long)(text - text_start - pattern_len);
                    }
                }
            }
        }

        return Status::NotFound;
    }
};

template <typename T>
MemoryPool AhoCorasickImpl<T>::memory_pool;

template <typename T>
int AhoCorasickImpl<T>::pool_idx = 0;

} // namespace StringMatch

#undef USE_PLACEMENT_NEW

#endif // STRING_MATCH_AHO_CORASICK_H
