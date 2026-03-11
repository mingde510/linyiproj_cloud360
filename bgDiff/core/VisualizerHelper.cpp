#include "VisualizerHelper.h"
#include <vtkRenderWindow.h>
#include <vtkCommand.h>
#include <vtkSmartPointer.h>
#include <iostream>

struct CallbackData {
    bool* stop{nullptr};
    pcl::visualization::PCLVisualizer* vis{nullptr};
};

// 键盘事件：置 stop，并终止渲染/关闭窗口
static void OnKeyboard(const pcl::visualization::KeyboardEvent& e, void* cookie) {
    if (!cookie || !e.keyDown()) return;
    auto* data = static_cast<CallbackData*>(cookie);
    if (!data->stop) return;
    auto key = e.getKeySym();
    if (key=="q" || key=="Q" || key=="x" || key=="X" || key=="Escape") {
        *data->stop = true;
        if (data->vis) {
            data->vis->getRenderWindow()->Finalize();
            data->vis->close();
        }
        std::cout << "[viz] keyboard exit: " << key << std::endl;
    }
}

// 窗口关闭事件：置 stop，并终止渲染/关闭窗口
struct CloseObserver : vtkCommand {
    CallbackData* data{};
    static CloseObserver* New() { return new CloseObserver; }
    void Execute(vtkObject*, unsigned long, void*) override {
        if (data && data->stop) {
            *data->stop = true;
            if (data->vis) {
                data->vis->getRenderWindow()->Finalize();
                data->vis->close();
            }
            std::cout << "[viz] window close event\n";
        }
    }
};

// 统一的 spin 循环，支持键盘/关闭退出
static void SpinWithExit(pcl::visualization::PCLVisualizer::Ptr vis) {
    bool stop = false;
    CallbackData cb_data{&stop, vis.get()};
    vis->registerKeyboardCallback(OnKeyboard, &cb_data);
    auto cb = vtkSmartPointer<CloseObserver>::New();
    cb->data = &cb_data;
    vis->getRenderWindow()->AddObserver(vtkCommand::ExitEvent, cb);
    vis->getRenderWindow()->AddObserver(vtkCommand::DeleteEvent, cb);

    while (!stop && !vis->wasStopped()) {
        vis->spinOnce(30);
    }
    vis->getRenderWindow()->Finalize();
    vis->close();
}

void VisualizerHelper::showCloudWithPlane(
    pcl::PointCloud<PointType>::ConstPtr cloud_roi,
    const Eigen::Vector4f& plane_coeff,
    const std::vector<int>& /*plane_inliers*/)
{
    auto vis = boost::make_shared<pcl::visualization::PCLVisualizer>("ROI Plane View");
    vis->setBackgroundColor(0,0,0);

    pcl::visualization::PointCloudColorHandlerGenericField<PointType>
        intensity_handler(cloud_roi, "intensity");
    vis->addPointCloud<PointType>(cloud_roi, intensity_handler, "roi_cloud");

    pcl::ModelCoefficients coeff;
    coeff.values = {plane_coeff[0], plane_coeff[1], plane_coeff[2], plane_coeff[3]};
    vis->addPlane(coeff, "fitted_plane");

    vis->addCoordinateSystem(0.5);
    SpinWithExit(vis);
}

void VisualizerHelper::showCloudWithPlaneHull(
    pcl::PointCloud<PointType>::ConstPtr cloud_roi,
    pcl::PointCloud<pcl::PointXYZ>::ConstPtr hull_world,
    const Eigen::Vector4f& plane_coeff)
{
    auto vis = boost::make_shared<pcl::visualization::PCLVisualizer>("ROI Plane Hull");
    vis->setBackgroundColor(0,0,0);

    pcl::visualization::PointCloudColorHandlerGenericField<PointType>
        intensity_handler(cloud_roi, "intensity");
    vis->addPointCloud<PointType>(cloud_roi, intensity_handler, "roi_cloud");
    vis->setPointCloudRenderingProperties(
        pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 2, "roi_cloud");

    if (hull_world && hull_world->size() >= 3) {
        vis->addPolygon<pcl::PointXYZ>(hull_world, 0.8, 0.8, 0.8, "plane_hull");
        vis->setShapeRenderingProperties(
            pcl::visualization::PCL_VISUALIZER_OPACITY, 0.4, "plane_hull");
    } else {
        pcl::ModelCoefficients coeff;
        coeff.values = {plane_coeff[0], plane_coeff[1], plane_coeff[2], plane_coeff[3]};
        vis->addPlane(coeff, "fitted_plane");
    }

    vis->addCoordinateSystem(0.5);
    SpinWithExit(vis);
}