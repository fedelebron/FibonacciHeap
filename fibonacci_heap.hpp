#include <algorithm>
#include <list>
#include <memory>
#include <vector>
namespace fibonacci_heap {

template<typename T> struct heap_node {
  int key;
  T value;
  bool marked;
  std::vector<std::unique_ptr<heap_node>> children;
  heap_node* parent;
};

template<typename T> struct heap {
  heap_node<T>* insert(const T& value, int key) {
    auto ptr = std::make_unique<heap_node<T>>(
                 heap_node<T>{key, value, false, {}, nullptr});
    auto it = roots_.insert(roots_.end(), std::move(ptr));
    update_minimum(it);
    return it->get();
  }

  void decreaseKey(heap_node<T>* x, int newkey) {
    x->key = newkey;
    if (x->parent && x->parent->key > newkey) {
      while (x->parent && x->parent->marked) {
        x = x->parent;
        x->marked = false;
      }
      if (x->parent) {
        x->parent->marked = true;
      }
      update_minimum(cut(x));
    }
  }

  T extractMin() {
    auto& children = (*smallest_)->children;
    for (int i = children.size() - 1; i >= 0; --i) {
      cut(std::move(children[i]), i);
    }

    // We preserve the smallest element in an ad-hoc list,
    // since otherwise it would be touched by consolidate,
    // and we need it intact to return it.
    roots_list l;
    l.splice(begin(l), roots_, smallest_);

    consolidate();
    return (*begin(l))->value;
  }

  bool empty() const {
    return roots_.empty();
  }

private:
  typedef std::list<std::unique_ptr<heap_node<T>>> roots_list;

  typename roots_list::iterator cut(heap_node<T>* x) {
    if (!x->parent) return roots_.end();
    auto idx = 0;
    while (x->parent->children[idx].get() != x) ++idx;
    return cut(std::move(x->parent->children[idx]), idx);
  }

  typename roots_list::iterator cut(std::unique_ptr<heap_node<T>> x, int idx) {
    auto& children = x->parent->children;
    auto it = roots_.insert(roots_.end(), std::move(x));
    children.erase(std::begin(children) + idx);
    (*it)->parent = nullptr;
    return it;
  }

  void update_minimum(typename roots_list::iterator it) {
    if (roots_.size() == 1
        || it == roots_.end()
        || (*smallest_)->key > (*it)->key) {
      smallest_ = it;
    }
  }

  typename roots_list::iterator mergeHeaps(typename roots_list::iterator it,
                                           typename roots_list::iterator jt) {
    if ((*it)->key < (*jt)->key) {
      swap(it, jt);
    }
    (*it)->parent = jt->get();
    (*jt)->children.push_back(std::move(*it));
    roots_.erase(it);
    return jt;
  }

  void consolidate() {
    std::vector<std::vector<typename roots_list::iterator>> roots_by_rank;
    for (auto it = begin(roots_); it != end(roots_); ++it) {
      int rank = (*it)->children.size();
      if (roots_by_rank.size() <= rank) {
        roots_by_rank.resize(rank + 1);
      }
      roots_by_rank[rank].push_back(it);
    }

    int n = roots_by_rank.size();
    for (int i = 0; i < n; ++i) {
      int m = roots_by_rank[i].size(),
          j = 0;
      while  (m - j >= 2) {
        auto it = roots_by_rank[i][j++];
        auto jt = roots_by_rank[i][j++];
        auto kt = mergeHeaps(it, jt);
        if (roots_by_rank.size() <= i + 1) {
          roots_by_rank.resize(i + 2);
          n = i + 2;
        }
        roots_by_rank[i + 1].push_back(kt);
      }
    }

    auto compare = [](const std::unique_ptr<heap_node<T>>& a,
                      const std::unique_ptr<heap_node<T>>& b) {
      return a->key < b->key;
    };
    smallest_ = std::min_element(begin(roots_), end(roots_), compare);
  }

  roots_list roots_;
  typename roots_list::iterator smallest_;
};

}
