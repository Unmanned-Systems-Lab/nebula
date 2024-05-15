#pragma once

#include "nebula_ros/common/parameter_descriptors.hpp"
#include "nebula_ros/common/watchdog_timer.hpp"

#include <nebula_common/nebula_common.hpp>
#include <nebula_common/hesai/hesai_common.hpp>
#include <nebula_decoders/nebula_decoders_hesai/hesai_driver.hpp>
#include <nebula_hw_interfaces/nebula_hw_interfaces_hesai/hesai_hw_interface.hpp>
#include <rclcpp/rclcpp.hpp>

#include <nebula_msgs/msg/nebula_packet.hpp>
#include <nebula_msgs/msg/nebula_packets.hpp>

#include <filesystem>
#include <memory>
#include <mutex>

namespace nebula
{
namespace ros
{
using TutorialDriver = nebula::drivers::HesaiDriver;
using TutorialHwInterface = nebula::drivers::HesaiHwInterface;
using TutorialSensorConfiguration = nebula::drivers::HesaiSensorConfiguration;
using TutorialCalibrationConfiguration = nebula::drivers::HesaiCalibrationConfiguration;
using get_calibration_result_t = nebula::util::expected<
  std::shared_ptr<TutorialCalibrationConfiguration>, nebula::Status>;

class TutorialDecoderWrapper
{

public:
  TutorialDecoderWrapper(
    rclcpp::Node * const parent_node,
    const std::shared_ptr<TutorialHwInterface> & hw_interface,
    std::shared_ptr<const TutorialSensorConfiguration> & config);

  void ProcessCloudPacket(std::unique_ptr<nebula_msgs::msg::NebulaPacket> packet_msg);

  void OnConfigChange(
    const std::shared_ptr<const TutorialSensorConfiguration> & new_config);

  void OnCalibrationChange(
    const std::shared_ptr<const TutorialCalibrationConfiguration> &
      new_calibration);

  rcl_interfaces::msg::SetParametersResult OnParameterChange(
    const std::vector<rclcpp::Parameter> & p);

  nebula::Status Status();

private:
  /// @brief Load calibration data from the best available source:
  /// 1. If sensor connected, download and save from sensor
  /// 2. If downloaded file available, load that file
  /// 3. Load the file given by `calibration_file_path`
  /// @param calibration_file_path The file to use if no better option is available
  /// @param ignore_others If true, skip straight so step 3 above, ignoring better calibration
  /// options
  /// @return The calibration data if successful, or an error code if not
  get_calibration_result_t GetCalibrationData(
    const std::string & calibration_file_path, bool ignore_others = false);

  void PublishCloud(std::unique_ptr<sensor_msgs::msg::PointCloud2> pointcloud,
                    const rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr& publisher);

  nebula::Status status_;
  rclcpp::Logger logger_;

  const std::shared_ptr<TutorialHwInterface> hw_interface_;
  std::shared_ptr<const TutorialSensorConfiguration> sensor_cfg_;

  std::string calibration_file_path_{};
  std::shared_ptr<const TutorialCalibrationConfiguration> calibration_cfg_ptr_{};

  std::shared_ptr<TutorialDriver> driver_ptr_{};
  std::mutex mtx_driver_ptr_;

  rclcpp::Publisher<nebula_msgs::msg::NebulaPackets>::SharedPtr packets_pub_{};
  nebula_msgs::msg::NebulaPackets::UniquePtr current_scan_msg_{};

  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr nebula_points_pub_{};

  std::shared_ptr<WatchdogTimer> cloud_watchdog_;
};
}  // namespace ros
}  // namespace nebula