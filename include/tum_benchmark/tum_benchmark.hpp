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

struct Dataset
{
public:
  struct Intrinsics
  {
    int width, height;
    double fx, fy, ox, oy, d0, d1, d2, d3, d4;
    double depth_scale;
  };

private:
  std::string m_path;
  Intrinsics m_intrinsics;
public:
  Dataset(const std::string& path) : m_path(path.at(path.size() - 1) == '/' ? path : path + "/")
  {
    m_intrinsics = { 640, 480, 525.0, 525.0, 319.5, 239.5, 0, 0, 0, 0, 0, 1.0 / 5000.0 };
  }

  std::string prefix(const std::string &str) const
  {
    return m_path + str;
  }

  std::string prefix(const char *str) const
  {
    return prefix(std::string(str));
  }

  template<typename InnerIteratorT>
  PrefixFileIterator<InnerIteratorT> prefix(const InnerIteratorT &inner) const
  {
    return make_prefix_file_iterator(m_path, inner);
  }

  template<typename EntryFormatT>
  FileReader<EntryFormatT> *open(const std::string &filename) const
  {
    return new FileReader<EntryFormatT>(prefix(filename));
  }

  bool tryLoadIntrinsics()
  {
    static const std::string id_prefix = "rgbd_dataset_freiburg";
    std::string::size_type pos = m_path.find(id_prefix);

    bool found = false;

    if(pos != std::string::npos)
    {
      char c = m_path.at(pos + id_prefix.size());

      switch(c)
      {
      case '1':
        m_intrinsics = { 640, 480, 517.3, 516.5, 318.6, 255.3, 0.2624, -0.9531, -0.0054, 0.0026, 1.1633, 1.035 / 5000.0 };
        break;
      case '2':
        m_intrinsics = { 640, 480, 520.9, 521.0, 325.1, 249.7, 0.2312, -0.7849, -0.0033, -0.0001, 0.9172, 1.031 / 5000.0 };
        break;
      case '3':
        m_intrinsics = { 640, 480, 535.4, 539.2, 320.1, 247.6, 0, 0, 0, 0, 0, 1.0 / 5000.0 };
        break;
      default:
        break;
      }
    }

    return found;
  }

  const Intrinsics &intrinsics() const
  {
    return m_intrinsics;
  }
};

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

std::ostream& operator<<(std::ostream& os, const tum_benchmark::Dataset::Intrinsics& intrinsics)
{
  os
      << intrinsics.width << "x" << intrinsics.height
      << " fx: " << intrinsics.fx
      << " fy: " << intrinsics.fy
      << " ox: " << intrinsics.ox
      << " oy: " << intrinsics.oy
      << " ds: " << intrinsics.depth_scale;

  return os;
}

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
