#ifndef TUM_BENCHMARK_HPP_
#define TUM_BENCHMARK_HPP_

#include <string>
#include <fstream>
#include <iterator>
#include <iomanip>

namespace tum_benchmark
{

// types used in standard TUM RGB-D benchmark files
struct File
{
  double timestamp;
  std::string name;
};

struct Trajectory
{
  double timestamp;
  double tx, ty, tz, qx, qy, qz, qw;
};

namespace internal
{

template<typename EntryFormatT, typename Enable = void>
struct ApplyStreamManipulators
{
  static void apply(std::istream &s)
  {
  }
};

template<typename EntryFormatT, typename Enable = void>
struct PrefixFileOp
{
  std::string m_prefix;
  
  PrefixFileOp(const std::string &prefix) : 
    m_prefix(prefix)
  {
  }

  void operator()(const EntryFormatT &in, EntryFormatT &out) const
  {
    out = in;
  }
};

template<>
struct PrefixFileOp<File, void>
{
  std::string m_prefix;
  
  PrefixFileOp(const std::string &prefix) : 
    m_prefix(prefix)
  {
  }

  void operator()(const File &in, File &out) const
  {
    out.timestamp = in.timestamp;
    out.name = m_prefix + in.name;
  }
};

} // namespace internal

template<typename EntryFormatT>
struct FileReaderIterator;

template<typename EntryFormatT>
struct FileReader
{
private:
  std::string m_filename;
  std::ifstream m_stream;
  
  void skipLine()
  {
    std::string tmp;
    getline(m_stream, tmp);
  }
  
  void openFileAndSkipComments()
  {
    m_stream.open(m_filename.c_str());
    
    while(m_stream.good() && m_stream.peek() == '#')
    {
      skipLine();
    }
  }
public:
  typedef FileReaderIterator<EntryFormatT> iterator;
  
  FileReader(const std::string &filename) :
    m_filename(filename)
  {
    openFileAndSkipComments();
    internal::ApplyStreamManipulators<EntryFormatT>::apply(m_stream);
  }
  
  ~FileReader()
  {
    m_stream.close();
  }
  
  FileReaderIterator<EntryFormatT> begin()
  {
    FileReaderIterator<EntryFormatT> iterator = m_stream.good() ? FileReaderIterator<EntryFormatT>(this) : end();
    // make iterator valid, by reading one entry
    return ++iterator;
  }
  
  FileReaderIterator<EntryFormatT> end()
  {
    return FileReaderIterator<EntryFormatT>();
  }
  
  bool tryReadNext(EntryFormatT &entry)
  {
    m_stream >> entry;
    
    return m_stream.good();
  }
};

template<typename EntryFormatT>
struct FileReaderIterator : std::iterator<std::input_iterator_tag, EntryFormatT>
{
private:
  FileReader<EntryFormatT> *m_reader;
  EntryFormatT m_entry;
public:
  FileReaderIterator() : 
    m_reader(0)
  {
  }
  
  FileReaderIterator(FileReader<EntryFormatT> *reader) : 
    m_reader(reader)
  {
  }
  
  bool operator==(const FileReaderIterator<EntryFormatT> &other)
  {
    return m_reader == other.m_reader;
  }
  
  bool operator!=(const FileReaderIterator<EntryFormatT> &other)
  {
    return m_reader != other.m_reader;
  }

  FileReaderIterator<EntryFormatT>& operator++() 
  {
    if(m_reader != 0 && !m_reader->tryReadNext(m_entry))
    {
      m_reader = 0;
    }
    return *this;
  }
  
  FileReaderIterator<EntryFormatT> operator++(int) 
  {
    FileReaderIterator<EntryFormatT> tmp(*this);
    operator++();
    return tmp;
  }
  
  EntryFormatT const& operator*() const 
  {
    return m_entry;
  }
  
  EntryFormatT const* operator->() const 
  {
    return &m_entry;
  }
};


template<typename InnerIteratorT>
struct PrefixFileIterator : std::iterator<std::input_iterator_tag, typename std::iterator_traits<InnerIteratorT>::value_type>
{
private:
  typedef typename std::iterator_traits<InnerIteratorT>::value_type EntryFormatT;
  internal::PrefixFileOp<EntryFormatT> m_prefix_op;
  InnerIteratorT m_inner;
  mutable EntryFormatT m_entry;
public:
  PrefixFileIterator()
  {
  }
  
  PrefixFileIterator(const std::string &prefix, const InnerIteratorT &inner) : 
    m_prefix_op(prefix),
    m_inner(inner)
  {
  }
  
  bool operator==(const InnerIteratorT &other)
  {
    return m_inner == other;
  }
  
  bool operator==(const PrefixFileIterator<InnerIteratorT> &other)
  {
    return m_inner == other.m_inner;
  }
  
  bool operator!=(const InnerIteratorT &other)
  {
    return m_inner != other;
  }
  
  bool operator!=(const PrefixFileIterator<InnerIteratorT> &other)
  {
    return m_inner != other.m_inner;
  }

  PrefixFileIterator<InnerIteratorT>& operator++() 
  {
    ++m_inner;
    return *this;
  }
  
  PrefixFileIterator<InnerIteratorT> operator++(int) 
  {
    PrefixFileIterator<InnerIteratorT> tmp(*this);
    operator++();
    return tmp;
  }
  
  EntryFormatT const& operator*() const 
  {
    m_prefix_op(*m_inner, m_entry);
    return m_entry;
  }
  
  EntryFormatT const* operator->() const 
  {    
    m_prefix_op(*m_inner, m_entry);
    return &m_entry;
  }
};

template<typename InnerIteratorT>
PrefixFileIterator<InnerIteratorT> make_prefix_file_iterator(const std::string &prefix, const InnerIteratorT &inner)
{
  return PrefixFileIterator<InnerIteratorT>(prefix, inner);
}

std::istream& operator>>(std::istream& is, tum_benchmark::File& file)
{
  is >> file.timestamp >> file.name;
  
  return is;
}

std::istream& operator>>(std::istream& is, tum_benchmark::Trajectory& t)
{
  is >> t.timestamp >> t.tx >> t.ty >> t.tz >> t.qx >> t.qy >> t.qz >> t.qw;
  
  return is;
}

struct FormatTimestamp
{
  double ts;

  FormatTimestamp(const double &ts) : ts(ts) {}
};

std::ostream& operator<<(std::ostream& os, const tum_benchmark::FormatTimestamp& ts)
{
  os << std::fixed << std::setprecision(6) << ts.ts << std::resetiosflags(std::ios_base::fixed);
  return os;
}

std::ostream& operator<<(std::ostream& os, const tum_benchmark::Trajectory& t)
{
  os << FormatTimestamp(t.timestamp) << " " << t.tx << " " << t.ty << " " << t.tz << " " << t.qx << " " << t.qy << " " << t.qz << " " << t.qw;

  return os;
}

} // namespace tum_benchmark


#endif // TUM_BENCHMARK_HPP_
