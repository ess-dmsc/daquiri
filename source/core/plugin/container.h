#pragma once

#include <string>
#include <list>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#define TT template<typename T>

TT class Container
{
    using container_t = typename std::list<T>;
    container_t data_;
  public:
    using iterator = typename container_t::iterator;
    using const_iterator = typename container_t::const_iterator;

    Container() {}

    void join(const Container<T> other);

    inline bool empty() const { return data_.empty(); }
    inline size_t size() const { return data_.size(); }
    inline void clear() { data_.clear(); }
    inline std::list<T>& data() { return data_; }
    inline const std::list<T>& data() const { return data_; }

    inline iterator begin() { return data_.begin(); }
    inline iterator end() { return data_.end(); }
    inline const_iterator begin() const { return data_.begin(); }
    inline const_iterator end() const { return data_.end(); }
    inline const_iterator cbegin() const { return data_.cbegin(); }
    inline const_iterator cend() const { return data_.cend(); }

    bool operator!= (const Container& other) const;
    bool operator== (const Container& other) const;

    bool has(const T& t) const;   //using deep compare
    bool has_a(const T& t) const; //using shallow compare

    void add(T t);   //using shallow compare
    void add_a(T t); //force add

    void replace(T t); //using shallow compare
    void replace(size_t i, T t);

    void remove(const T &t);   //using deep compare
    void remove_a(const T &t); //using shallow compare
    void remove(size_t i);
    
    T get(T t) const;
    T get(size_t i) const;
    void up(size_t i);
    void down(size_t i);

    std::vector<T> to_vector() const;
    void from_vector(std::vector<T> vec);

    inline friend void to_json(json& j, const Container<T>& t)
    {
      j = t.data_;
      //    for (auto k : t.data_)
      //      j.push_back(json(k));
    }

    inline friend void from_json(const json& j, Container<T>& t)
    {
      for (auto it : j)
        t.data_.push_back(it);
    }
};

TT void Container<T>::join(const Container<T> other)
{
  data_.insert(data_.end(), other.data_.begin(), other.data_.end());
}

TT bool Container<T>::operator!= (const Container& other) const
{
  return !operator==(other);
}

TT bool Container<T>::operator== (const Container& other) const
{
  if (data_.size() != other.data_.size())
    return false;
  //ordering does not matter, deep compare on elements
  for (auto &q : data_)
    if (!other.has(q))
      return false;
  return true;
}

TT bool Container<T>::has(const T& t) const
{
  for (auto &q : data_)
    if (q == t)
      return true;
  return false;
}

TT bool Container<T>::has_a(const T& t) const
{
  for (auto &q : data_)
    if (t.shallow_equals(q))
      return true;
  return false;
}

TT void Container<T>::add(T t)
{
  if (t == T())
    return;
  if (!has_a(t))
    data_.push_back(t);
}

TT void Container<T>::add_a(T t)
{
  if (t == T())
    return;
  data_.push_back(t);
}

TT void Container<T>::replace(T t)
{
  if (t == T())
    return;
  bool replaced = false;
  for (auto &q : data_)
    if (q.shallow_equals(t))
    {
      replaced = true;
      q = t;
    }
  if (!replaced)
    data_.push_back(t);
}

TT void Container<T>::replace(size_t i, T t)
{
  if ((i >= 0) && (i < size())) {
    typename std::list<T>::iterator it = std::next(data_.begin(), i);
    (*it) = t;
  }
}

TT void Container<T>::remove(const T &t)  //using deep compare
{
  typename std::list<T>::iterator it = data_.begin();
  while (it != data_.end()) {
    if (*it == t)
      it = data_.erase(it);
    ++it;
  }
}

TT void Container<T>::remove_a(const T &t)   //using shallow compare
{
  typename std::list<T>::iterator it = data_.begin();
  while (it != data_.end()) {
    if (it->shallow_equals(t))
      it = data_.erase(it);
    ++it;
  }
}

TT void Container<T>::remove(size_t i)
{
  if ((i >= 0) && (i < size())) {
    typename std::list<T>::iterator it = std::next(data_.begin(), i);
    data_.erase(it);
  }
}

TT T Container<T>::get(T t) const
{
  for (auto &q: data_)
    if (q.shallow_equals(t))
      return q;
  return T();
}

TT T Container<T>::get(size_t i) const
{
  if ((i >= 0) && (i < size())) {
    typename std::list<T>::const_iterator it = std::next(data_.begin(), i);
    return *it;
  }
  return T();
}

TT void Container<T>::up(size_t i)
{
  if ((i > 0) && (i < size())) {
    typename std::list<T>::iterator it = std::next(data_.begin(), i-1);
    std::swap( *it, *std::next( it ) );
  }
}

TT void Container<T>::down(size_t i)
{
  if ((i >= 0) && ((i+1) < size())) {
    typename std::list<T>::iterator it = std::next(data_.begin(), i);
    std::swap( *it, *std::next( it ) );
  }
}


TT std::vector<T> Container<T>::to_vector() const
{
  if (!data_.empty())
    return std::vector<T>(data_.begin(), data_.end());
  else
    return std::vector<T>();
}

TT void Container<T>::from_vector(std::vector<T> vec)
{
  data_.clear();
  if (!vec.empty())
    data_ = std::list<T>(vec.begin(), vec.end());
}

#undef TT
