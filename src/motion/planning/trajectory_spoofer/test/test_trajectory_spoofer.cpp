// Copyright 2020 The Autoware Foundation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "gtest/gtest.h"
#include "trajectory_spoofer/trajectory_spoofer.hpp"

#include <autoware_auto_msgs/msg/trajectory.hpp>
#include <autoware_auto_msgs/msg/trajectory_point.hpp>
#include <autoware_auto_msgs/msg/vehicle_kinematic_state.hpp>
#include <common/types.hpp>
#include <time_utils/time_utils.hpp>

#include <cmath>
#include <iostream>

using autoware_auto_msgs::msg::Trajectory;
using autoware_auto_msgs::msg::TrajectoryPoint;
using autoware_auto_msgs::msg::VehicleKinematicState;
using autoware::trajectory_spoofer::NANO_IN_SEC;
using autoware::trajectory_spoofer::TrajectorySpoofer;

using autoware::common::types::float32_t;
using autoware::common::types::float64_t;
using autoware::common::types::TAU;

TEST(test_trajectory_spoofer, straight_trajectory) {
  TrajectorySpoofer ts(10.0);
  VehicleKinematicState starting_point;

  int32_t num_of_points = 20;
  float32_t length = 10.0;
  starting_point.state.longitudinal_velocity_mps = 2.0;

  auto traj = ts.spoof_straight_trajectory(
    starting_point, num_of_points, length, false);

  auto first_point = traj.points.front();
  auto last_point = traj.points.back();
  auto last_time = static_cast<float64_t>(
    time_utils::from_message(last_point.time_from_start).count()) / NANO_IN_SEC;
  // Have to use floating point values due to rounding errors
  // in the calculation of nanosecond times in TrajectorySpoofer
  float64_t end_time = 
    (static_cast<float64_t>(time_utils::from_message(starting_point.state.time_from_start).count()) /
     NANO_IN_SEC) + (length / starting_point.state.longitudinal_velocity_mps);

  // Initial heading of 0 should mean travelling straight along the X axis
  ASSERT_EQ(traj.points.size(), num_of_points);
  ASSERT_FLOAT_EQ(last_time, end_time);
  ASSERT_FLOAT_EQ(last_point.x, length);
  ASSERT_FLOAT_EQ(last_point.y, first_point.y);
  ASSERT_FLOAT_EQ(last_point.heading.real, first_point.heading.real);
  ASSERT_FLOAT_EQ(last_point.heading.imag, first_point.heading.imag);
  ASSERT_FLOAT_EQ(last_point.longitudinal_velocity_mps, first_point.longitudinal_velocity_mps);
  ASSERT_FLOAT_EQ(last_point.lateral_velocity_mps, first_point.lateral_velocity_mps);
  ASSERT_FLOAT_EQ(last_point.acceleration_mps2, first_point.acceleration_mps2);
  ASSERT_FLOAT_EQ(last_point.heading_rate_rps, first_point.heading_rate_rps);
  ASSERT_FLOAT_EQ(last_point.front_wheel_angle_rad, first_point.front_wheel_angle_rad);
  ASSERT_FLOAT_EQ(last_point.rear_wheel_angle_rad, first_point.rear_wheel_angle_rad);

  num_of_points = 100;  // max points
  length = 2500.0;
  constexpr float32_t head_rad = 45.0 * M_PI / 180.0; // Start at heading of 45 degrees
  starting_point.state.heading = TrajectorySpoofer::to_2d_quaternion(head_rad);
  starting_point.state.longitudinal_velocity_mps = 12.5;

  traj = ts.spoof_straight_trajectory(
    starting_point, num_of_points, length, false);

  first_point = traj.points.front();
  last_point = traj.points.back();
  last_time = static_cast<float64_t>(
    time_utils::from_message(last_point.time_from_start).count()) / NANO_IN_SEC;
  end_time = 
    (static_cast<float64_t>(time_utils::from_message(starting_point.state.time_from_start).count()) /
    NANO_IN_SEC) + (length / starting_point.state.longitudinal_velocity_mps);

  // Calc x and y of last point
  const float32_t end_x = std::cos(head_rad) * length;
  const float32_t end_y = std::sin(head_rad) * length;

  ASSERT_EQ(traj.points.size(), num_of_points);
  ASSERT_FLOAT_EQ(last_time, end_time);
  ASSERT_FLOAT_EQ(last_point.x, end_x);
  ASSERT_FLOAT_EQ(last_point.y, end_y);
  ASSERT_FLOAT_EQ(last_point.heading.real, first_point.heading.real);
  ASSERT_FLOAT_EQ(last_point.heading.imag, first_point.heading.imag);
  ASSERT_FLOAT_EQ(last_point.longitudinal_velocity_mps, first_point.longitudinal_velocity_mps);
  ASSERT_FLOAT_EQ(last_point.lateral_velocity_mps, first_point.lateral_velocity_mps);
  ASSERT_FLOAT_EQ(last_point.acceleration_mps2, first_point.acceleration_mps2);
  ASSERT_FLOAT_EQ(last_point.heading_rate_rps, first_point.heading_rate_rps);
  ASSERT_FLOAT_EQ(last_point.front_wheel_angle_rad, first_point.front_wheel_angle_rad);
  ASSERT_FLOAT_EQ(last_point.rear_wheel_angle_rad, first_point.rear_wheel_angle_rad);
}

TEST(test_trajectory_spoofer, circular_trajectory) {
  TrajectorySpoofer ts;
  VehicleKinematicState starting_point;

  // Points = size of circle + 1 to get back to
  // original location and heading
  int32_t num_of_points = 9;
  float32_t radius = 20.0;
  starting_point.state.longitudinal_velocity_mps = 2.0;

  auto traj = ts.spoof_circular_trajectory(starting_point, num_of_points, radius);

  auto first_point = traj.points.front();
  auto last_point = traj.points.back();
  auto last_time = static_cast<float64_t>(
    time_utils::from_message(last_point.time_from_start).count()) / NANO_IN_SEC;
  float64_t seg_angle_rad = TAU / static_cast<float64_t>(num_of_points - 1);
  // Chord distance * number of segments
  float64_t length = (2.0 * radius * std::sin(seg_angle_rad / 2.0)) * (num_of_points - 1);
  float64_t end_time = 
    (static_cast<float64_t>(time_utils::from_message(starting_point.state.time_from_start).count()) /
     NANO_IN_SEC) + (length / starting_point.state.longitudinal_velocity_mps);

  ASSERT_EQ(traj.points.size(), num_of_points);
  ASSERT_FLOAT_EQ(last_time, end_time);
  // last point should have the same x/y as first point
  ASSERT_FLOAT_EQ(last_point.x, first_point.x);
  ASSERT_FLOAT_EQ(last_point.y, first_point.y);
  // Should end up with the same heading as initial heading
  ASSERT_FLOAT_EQ(last_point.heading.real, first_point.heading.real);
  ASSERT_FLOAT_EQ(last_point.heading.imag, first_point.heading.imag);
  ASSERT_FLOAT_EQ(last_point.longitudinal_velocity_mps, first_point.longitudinal_velocity_mps);
  ASSERT_FLOAT_EQ(last_point.acceleration_mps2, first_point.acceleration_mps2);

  num_of_points = 100;  // max points
  radius = 2500.0;
  constexpr float32_t head_rad = 45.0 * M_PI / 180.0; // Start at heading of 45 degrees
  starting_point.state.heading = TrajectorySpoofer::to_2d_quaternion(head_rad);
  starting_point.state.longitudinal_velocity_mps = 12.5;

  traj = ts.spoof_circular_trajectory(starting_point, num_of_points, radius);

  first_point = traj.points.front();
  last_point = traj.points.back();
  last_time = static_cast<float64_t>(
    time_utils::from_message(last_point.time_from_start).count()) / NANO_IN_SEC;
  seg_angle_rad = TAU / static_cast<float64_t>(num_of_points - 1);
  // Chord distance * number of segments
  length = (2.0 * radius * std::sin(seg_angle_rad / 2.0)) * (num_of_points - 1);
  end_time = 
    (static_cast<float64_t>(time_utils::from_message(starting_point.state.time_from_start).count()) /
     NANO_IN_SEC) + (length / starting_point.state.longitudinal_velocity_mps);

  ASSERT_EQ(traj.points.size(), num_of_points);
  ASSERT_FLOAT_EQ(last_time, end_time);
  // last point should have the same x/y as first point
  ASSERT_FLOAT_EQ(last_point.x, first_point.x);
  ASSERT_FLOAT_EQ(last_point.y, first_point.y);
  // Should end up with the same heading as initial heading
  ASSERT_FLOAT_EQ(last_point.heading.real, first_point.heading.real);
  ASSERT_FLOAT_EQ(last_point.heading.imag, first_point.heading.imag);
  ASSERT_FLOAT_EQ(last_point.longitudinal_velocity_mps, first_point.longitudinal_velocity_mps);
  ASSERT_FLOAT_EQ(last_point.acceleration_mps2, first_point.acceleration_mps2);
}
