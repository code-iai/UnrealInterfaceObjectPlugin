# UnrealInterfaceObjectPlugin

**UE4 Version: 4.22.1** 

## Features

- A new Actor by the name of ObjectPosePublisher which publishes a PoseStamped message, under the given topic (default: "/unreal_interface/object_poses") for each Object with predefined Type (default: "UnrealInterface") and Key (default: "spawned"), onto the RosBridge.

## Installation

**Required Plugins:**
- [UROSBridge](https://github.com/robcog-iai/UROSBridge)
- [UUtils](https://github.com/robcog-iai/UUtils)
- [USemLog](https://github.com/robcog-iai/USemLog)

Download this Plugin and simply put it into your Projects Plugins folder, while having the before mentioned Plugins added to your project.
