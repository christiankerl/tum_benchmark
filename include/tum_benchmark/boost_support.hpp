#ifndef BOOST_SUPPORT_HPP_
#define BOOST_SUPPORT_HPP_

#include <tum_benchmark/tum_benchmark.hpp>

#include <boost/fusion/include/in.hpp>
#include <boost/fusion/include/is_sequence.hpp>
#include <boost/fusion/include/transform.hpp>

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_io.hpp>

namespace tum_benchmark
{
namespace internal
{
template<typename EntryFormatT>
struct ApplyStreamManipulators<EntryFormatT, typename boost::enable_if<boost::fusion::traits::is_sequence<EntryFormatT> >::type>
{
  static void apply(std::istream &s)
  {
    using namespace boost::fusion;
    s >> tuple_open(' ') >> tuple_close(' ') >> tuple_delimiter(' ');
  }
};

template <class T0, class T1, class T2, class T3, class T4,
          class T5, class T6, class T7, class T8, class T9>
struct ApplyStreamManipulators<boost::tuples::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>, void>
{
  static void apply(std::istream &s)
  {
    using namespace boost::tuples;
    s >> set_open(' ') >> set_close(' ') >> set_delimiter(' ');
  }
};

struct prefix_file_transform_op
{
  template<typename T>
  struct result;

  template <typename T>
  struct result<prefix_file_transform_op(T)>
  {
    typedef typename boost::remove_reference<T>::type type;
  };

  std::string m_prefix;
  
  prefix_file_transform_op(const std::string &prefix) : 
    m_prefix(prefix)
  {
  }

  template<typename T>
  T operator()(const T &data) const
  {
    T result;
    PrefixFileOp<T> op(m_prefix);
    op(data, result);
    
    return result;
  }
};

template<typename EntryFormatT>
struct PrefixFileOp<EntryFormatT, typename boost::enable_if<boost::fusion::traits::is_sequence<EntryFormatT> >::type>
{
  std::string m_prefix;
  
  PrefixFileOp(const std::string &prefix) : 
    m_prefix(prefix)
  {
  }

  void operator()(const EntryFormatT &in, EntryFormatT &out) const
  {
    out = boost::fusion::transform(in, prefix_file_transform_op(m_prefix));
  }
};

} // namespace internal

} // namespace tum_benchmark
#endif // BOOST_SUPPORT_HPP_
