#pragma once
#include "UnitreePointType.h"
#include <pcl/visualization/pcl_visualizer.h>
#include <Eigen/Dense>
#include <vector>

struct VisualizerHelper {
    static void showCloudWithPlane(
        pcl::PointCloud<PointType>::ConstPtr cloud_roi,
        const Eigen::Vector4f& plane_coeff,
        const std::vector<int>& plane_inliers
    );

    static void showCloudWithPlaneHull(
        pcl::PointCloud<PointType>::ConstPtr cloud_roi,
        pcl::PointCloud<pcl::PointXYZ>::ConstPtr hull_world,
        const Eigen::Vector4f& plane_coeff
    );
};

