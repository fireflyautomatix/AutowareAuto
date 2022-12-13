F1Tenth Demonstration {#f1tenth-demo-lgsvl}
=================================================

# Setup Simulation {#f1tenth-simulation}

## Setup Joystick Controller
Plug in your Logitech wireless gamepad F710 controller receiver, and verify that device is connected by

```{bash}
$ ls /dev/input/js0
```

## Launching the simulator
Launch the simulator by

```{bash}
$ cd ~/adehome/AutowareAuto
$ ade --rc .aderc-amd64-foxy-lgsvl start --update --enter

ade$ /opt/lgsvl/simulator
```

If you have never setup LGSVL before, please follow the instructions in [lgsvl.md](https://gitlab.com/autowarefoundation/autoware.auto/AutowareAuto/-/blob/master/docs/installation/lgsvl.md) until the section called “Configure the cluster”.

## Creating a simulation
### Choosing a map
The goal is to add `Red Bull Ring Racetrack` map to your library. If that map is already in your library then nothing needs to be done.

Adding a map to your library:
- Go to `Store` -> `Maps`.
- Click `+` button next to `Red Bull Ring Racetrack` map (you can use search bar to filter by name).

### Configuring a vehicle {#f1tenth-lgsvl-configuring-vehicle}

The goal is to add `F1TenthCar` vehicle to your library. If this vehicle is already in your library then nothing needs to be done.

Adding a vehicle to your library:
- Go to `Store` -> `Vehicles`.
- Click `+` button next to `F1TenthCar` vehicle (you can use search bar to filter by name).

### Configure vehicle sensors

Once you added vehicle to your library:
- Go to `Library` -> `Vehicles`.
- Click on the `F1TenthCar` portrait. You will be forwarded to a vehicle edit page.
- Click a button near `Sensor Configurations` section to modify sensor configurations.

Create a new one and use the most recent version of sensor configuration file:
- Click `+ Create New Configuration` button at the bottom of the page.
- Set a configuration name and pick `ROS2` as a bridge. - Confirm.

In the configuration edit view:
- Click `{...}` symbol near Visual Editor (preview) window.
- Paste contents of [f1tenth_sensors.json](https://gitlab.com/autowarefoundation/autoware.auto/AutowareAuto/-/blob/master/src/launch/f1tenth_launch/config/svl/f1tenth-sensors.json) file inside edit window. `Configurator` window should populate with 
bunch of sensors now.
- Click `Save` to save configuration.

### Choosing/creating a simulation

The SVL simulator lets you store and reuse multiple simulation configurations. To use an existing simulation, navigate to `Simulations` tab and press the "Run Simulation" button for desired instance. The simulator should now start in the SVL window.

To create a new simulation, follow the below steps:

- Switch to the `Simulations` tab and click the `Add new` button.
- Enter a name, description and select a cluster. Click `Next`.
- Select the `Random Traffic` runtime template from the drop-down menu.
- Select `Red Bull Ring Racetrack` map and `F1TenthCar` vehicle with your sensor configuration. Click `Next`.
- Select `Autoware.Auto` autopilot and leave `Bridge Connection` with default value.
- Click `Next` and then `Publish`.

You can visit [SVL documentation](https://www.svlsimulator.com/docs/user-interface/web/simulations/) for more in-depth description.

# F1Tenth RecordReplay Trajectory Demo

## Starting ade and setting up Autoware for F1Tenth

```{bash}
$ cd adehome/AutowareAuto

# start ade
# If you are using a joystick, add '-- --device /dev/input/js0'
$ ade --rc .aderc-amd64-foxy-lgsvl start --update --enter

# build autoware
ade$ cd AutowareAuto
ade$ git checkout master
ade$ vcs import . < autoware.auto.foxy.repos
ade$ sudo apt update; rosdep update; rosdep install --from-paths src --ignore-src --rosdistro ${ROS_DISTRO} -yr
ade$ colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Release
```

## Launching simulator
```{bash}
$ ade enter
$ /opt/lgsvl/simulator
```

In your browser, start the Simulation environment you setup in the previous section.

## Creating Map
Skip this step if you want to use redbull_ring_racetrack. The map is already provided.

```{bash}
# (Terminal 1)
$ ade enter
ade$ source AutowareAuto/install/setup.bash

# add 'with_joy:=True' to drive vehicle using joystick
# press the right shoulder button to start driving.
ade$ ros2 launch f1tenth_launch f1tenth_mapping_demo.launch.py
```

In LGSVL, drive around the vehicle. Make sure to drive more than one complete lap for the mapping node to detect loop closure.

Save the map for AMCL by running:

```{bash}
# (Terminal 2)
$ ade enter
ade$ mkdir ${HOME}/map
ade$ ros2 run nav2_map_server map_saver_cli -f $HOME/map/redbull_ring_racetrack
```

This will save an image-style map for AMCL localization.

Alternatively, save the map for SLAM by running:

```{bash}
# (Terminal 2)
ade$ mkdir ${HOME}/map
ade$ ros2 service call /slam_toolbox/serialize_map slam_toolbox/srv/SerializePoseGraph "{filename: "$HOME/map/redbull_ring_racetrack"}"
```

This will save a pose graph for `slam_toolbox`'s localization.

After saving maps, stop mapping nodes by pressing `Ctrl+C`

## Recording and replaying trajectory
### Record a trajectory

```{bash}
# (Terminal 1)
$ ade enter
ade$ source AutowareAuto/install/setup.bash

# add 'with_joy:=True' to drive vehicle using joystick
# press the right shoulder button to start driving.
# add 'localization:='slam'' to use slam_toolbox for localization (default is amcl)
# add 'map:=/path/to/map.yaml' (AMCL) or 'map:=/path/to/map.posegraph' (SLAM) to select your original map
ade$ ros2 launch f1tenth_launch f1tenth_recordreplay_demo.launch.py
```

Set a intial pose with correct orientation in Rviz using `2D pose estimate`

```{bash}
# (Terminal 2)
ade$ source /opt/AutowareAuto/setup.bash

ade$ mkdir ${HOME}/path
ade$ ros2 action send_goal /planning/recordtrajectory autoware_auto_planning_msgs/action/RecordTrajectory "{record_path: "path/saved_path"}" --feedback
```
In LGSVL, drive around the vehicle and stop recording with Ctrl + C in terminal where `f1tenth_recordreplay_demo.launch.py` was launched.

### Replay a trajectory

```{bash}
(Terminal 1)
ade$ source /opt/AutowareAuto/setup.bash

# add 'with_joy:=True' to drive vehicle using joystick
# press the right shoulder button to switch between manual and auto mode
# add 'map:=/path/to/map.yaml' to select your original map
ade$ ros2 launch f1tenth_launch f1tenth_recordreplay_demo.launch.py
```

Set a intial pose with correct orientation in Rviz using `2D pose estimate`

```{bash}
(Terminal 2)
ade$ source /opt/AutowareAuto/setup.bash
ade$ ros2 action send_goal /planning/replaytrajectory autoware_auto_planning_msgs/action/ReplayTrajectory "{replay_path: "path/saved_path"}" --feedback
```

## Known issues

1. The replayed velocity might not be exactly the same with the recorded one. This is due to limitation of `Pure Pursuit` algorithm. The implementation does not take delay and external force into consideration, which means that it assumes constant speed when acceleration is released even if break is not pressed. This causes velocity of the vehicle to be wrong. Improvements will be made in the future.

2. The connection between the SVL Simulator local machine and remote server may be unstable, causing the ROS2 message to be lost and 2D Pose Estimate to fail. If that occurs, restart the SVL Simulator and try again.
