#define UNITREE_SDK_PCL_INCLUDED
#include "unitree_lidar_sdk_pcl.h"   // SDK 原生 PointType + transformUnitreeCloudToPCL

#include "UnitreePclBridge.h"

void UnitreeToPcl(const PointCloudUnitree& cloud,
                  pcl::PointCloud<PointType>::Ptr& pclCloud)
{
    transformUnitreeCloudToPCL(cloud, pclCloud);
}
