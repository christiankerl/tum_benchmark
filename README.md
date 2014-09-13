# tum_benchmark

C++ header only library to easily access files used in [TUM RGB-D SLAM datasets](http://vision.in.tum.de/data/datasets/rgbd-dataset).

## Optional Requirements

 *  Boost
 *  Eigen

## Basic Types

The benchmark datasets come with two types of files. The first contains lists of timestamped images. Every entry of such a file is represented by the `tum_benchmark::File` struct:

```c++
struct File
{
  double timestamp;
  std::string name;
};
```

The second file type represents groundtruth trajectories. Their entries are represented by the `tum_benchmark::Trajectory` struct:

```c++
struct Trajectory
{
  double timestamp;
  double tx, ty, tz, qx, qy, qz, qw;
};
```

For more details about these files have a look at the [format documentation](http://vision.in.tum.de/data/datasets/rgbd-dataset/file_formats).

## Examples

Reading a dataset file containing the timestamped RGB images into a vector:

```c++
#include <tum_benchmark/tum_benchmark.hpp>
#include <vector>

int main(int argc, char **argv)
{
  using namespace tum_benchmark;
  
  // define the format of each line in the file
  typedef File EntryFormat;
  
  // open the file reader
  FileReader<EntryFormat> reader("/path/to/rgbd_dataset_freiburg1_desk/rgb.txt");

  // read all lines into a vector
  std::vector<EntryFormat> all_entries;
  std::copy(reader.begin(), reader.end(), std::back_inserter(all_entries));

  return 0;
}
```

Reading a dataset file containing the timestamped RGB images into a vector and prefixing every filename with the absolute dataset path:

```c++
#include <tum_benchmark/tum_benchmark.hpp>
#include <vector>

int main(int argc, char **argv)
{
  using namespace tum_benchmark;
  
  std::string dataset = "/path/to/rgbd_dataset_freiburg1_desk/";
  
  typedef File EntryFormat;
  FileReader<EntryFormat> reader(dataset + "rgb.txt");

  std::vector<EntryFormat> all_entries;
  // make_prefix_file_iterator creates an iterator, which prefixes the name field in every File object
  std::copy(make_prefix_file_iterator(dataset, reader.begin()), make_prefix_file_iterator(dataset, reader.end()), std::back_inserter(all_entries));

  return 0;
}
```

Reading a groundtruth trajectory and converting it to `Eigen::Isometry3f` objects:

```c++
#include <tum_benchmark/tum_benchmark.hpp>
#include <tum_benchmark/eigen_support.hpp>

#include <Eigen/Geometry>

#include <vector>
#include <iostream>

int main(int argc, char **argv)
{
  using namespace tum_benchmark;
  
  typedef Trajectory EntryFormat;
  FileReader<EntryFormat> reader("/path/to/rgbd_dataset_freiburg1_desk/groundtruth.txt");

  Eigen::Isometry3f pose;

  for(FileReader<EntryFormat>::iterator it = reader.begin(); it != reader.end(); ++it)
  {
    // convert the Trajectory object to a Eigen object
    toEigen(*it, pose);
    
    std::cout << pose.matrix() << std::endl;
  }

  return 0;
}
```

Use Boost.Fusion to read dataset files created with the [`associate.py` script](http://vision.in.tum.de/data/datasets/rgbd-dataset/tools#associating_color_and_depth_images):


```c++
#include <tum_benchmark/tum_benchmark.hpp>
#include <tum_benchmark/boost_support.hpp>

#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/at_c.hpp>

#include <vector>
#include <iostream>

int main(int argc, char **argv)
{
  using namespace tum_benchmark;
  namespace bf = boost::fusion;
  
  std::string dataset = "/path/to/rgbd_dataset_freiburg1_desk/";
  
  // use boost::fusion::vector to define the format of associated files
  typedef bf::vector<File, File, Trajectory> EntryFormat;
  
  FileReader<EntryFormat> reader(dataset + "rgb_depth_groundtruth.txt");

  // prefixing every filename also works for complex formats
  for(PrefixFileIterator<FileReader<EntryFormat>::iterator> it = make_prefix_file_iterator(dataset, reader.begin()); it != reader.end(); ++it)
  {
    // get the prefixed name of the depth file
    std::cout << bf::at_c<1>(*it).name << std::endl;
  }

  return 0;
}
```
