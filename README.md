![resultRaytraceShadowMedieval](docs/Images/resultRaytraceShadowMedieval.png)

# vk_raytracing_tutorial_KHR

This branch is the [Circle C++ shaders](https://github.com/seanbaxter/shaders/blob/master/README.md) port of [Martin-K Lefrançois's](https://twitter.com/doragonhanta) [**Ray tracing tutorial**](https://nvpro-samples.github.io/vk_raytracing_tutorial_KHR/).

Compile with [Circle build 111](https://www.circle-lang.org).

You'll need the dependencies listed [here](https://nvpro-samples.github.io/vk_mini_path_tracer/index.html#hello,vulkan!/settingupyourdevelopmentenvironment/installdependencies).

To build, try this:

```
# Download the nvpro framework
$ mkdir vk_tutorial
$ cd vk_tutorial
vk_tutorial$ git clone git@github.com:nvpro-samples/shared_sources
vk_tutorial$ git clone git@github.com:nvpro-samples/shared_external

# Download the circle branch of the tutorial
vk_tutorial$ git clone git@github.com:seanbaxter/vk_raytracing_tutorial_KHR -b circle

# Make a build directory
vk_tutorial$ mkdir build
vk_tutorial$ cd build

# Point cmake to circle. If it's in the path, it's real easy.
vk_tutorial/build$ cmake -DCMAKE_CXX_COMPILER=circle ..

# Compile on some number of cores.
vk_tutorial/build$ make -j4
[100%] Linking CXX executable /vk_tutorial/bin_x64/vk_ray_tracing__simple_KHR.exe
[100%] Built target vk_ray_tracing__simple_KHR

# Run the thing.
vk_tutorial/build$ /vk_tutorial/bin_x64/vk_ray_tracing__simple_KHR.exe
```

