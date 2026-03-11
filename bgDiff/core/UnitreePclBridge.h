#pragma once
#include "unitree_lidar_sdk.h"
#include "UnitreePointType.h"
#include <pcl/point_cloud.h>

using namespace unilidar_sdk2;

// 只声明
void UnitreeToPcl(const PointCloudUnitree& cloud,
                  pcl::PointCloud<PointType>::Ptr& pclCloud);
