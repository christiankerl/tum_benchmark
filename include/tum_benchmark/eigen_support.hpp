#ifndef EIGEN_SUPPORT_HPP_
#define EIGEN_SUPPORT_HPP_

#include <tum_benchmark/tum_benchmark.hpp>
#include <Eigen/Geometry>

namespace tum_benchmark
{

template<typename ScalarT, int TypeT>
void toEigen(const Trajectory &trajectory, Eigen::Transform<ScalarT, 3, TypeT> &transform)
{
  Eigen::Quaternion<ScalarT> q(ScalarT(trajectory.qw), ScalarT(trajectory.qx), ScalarT(trajectory.qy), ScalarT(trajectory.qz));
  Eigen::Transform<ScalarT, 3, TypeT> result(q);

  result.translation()(0) = ScalarT(trajectory.tx);
  result.translation()(1) = ScalarT(trajectory.ty);
  result.translation()(2) = ScalarT(trajectory.tz);
  
  transform = result;
}

template<typename ScalarT, int TypeT>
void fromEigen(const Eigen::Transform<ScalarT, 3, TypeT> &transform, Trajectory &trajectory)
{
  Eigen::Quaternion<ScalarT> q(transform.rotation());

  trajectory.tx = transform.translation()(0);
  trajectory.ty = transform.translation()(1);
  trajectory.tz = transform.translation()(2);
  trajectory.qx = q.x();
  trajectory.qy = q.y();
  trajectory.qz = q.z();
  trajectory.qw = q.w();
}

} // namespace tum_benchmark

#endif // EIGEN_SUPPORT_HPP_
