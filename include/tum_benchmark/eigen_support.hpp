#ifndef EIGEN_SUPPORT_HPP_
#define EIGEN_SUPPORT_HPP_

#include <tum_benchmark/tum_benchmark.hpp>
#include <Eigen/Geometry>

namespace tum_benchmark
{

template<typename ScalarT, int TypeT>
void toEigen(Trajectory &trajectory, Eigen::Transform<ScalarT, 3, TypeT> &transform)
{
  Eigen::Quaternion<ScalarT> q(ScalarT(trajectory.qw), ScalarT(trajectory.qx), ScalarT(trajectory.qy), ScalarT(trajectory.qz));
  Eigen::Transform<ScalarT, 3, TypeT> result(q);

  result.translation()(0) = ScalarT(trajectory.tx);
  result.translation()(1) = ScalarT(trajectory.ty);
  result.translation()(2) = ScalarT(trajectory.tz);
  
  transform = result;
}

} // namespace tum_benchmark

#endif // EIGEN_SUPPORT_HPP_
