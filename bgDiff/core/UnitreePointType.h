#pragma once
#include <pcl/point_types.h>
#include <pcl/register_point_struct.h>
#include <Eigen/Core>
#include <cstdint>

// Bridge.cpp 会定义 UNITREE_SDK_PCL_INCLUDED 并 include SDK 的 PointType
// 所以在那个编译单元里不要复刻
#ifndef UNITREE_SDK_PCL_INCLUDED

struct PointType
{
    PCL_ADD_POINT4D;
    PCL_ADD_INTENSITY;
    std::uint16_t ring;
    float time;
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
} EIGEN_ALIGN16;

POINT_CLOUD_REGISTER_POINT_STRUCT(PointType,
    (float, x, x)
    (float, y, y)
    (float, z, z)
    (float, intensity, intensity)
    (std::uint16_t, ring, ring)
    (float, time, time)
)

#endif
