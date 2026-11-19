# ========== 阶段1：编译 builder ==========
FROM ros:humble-ros-base AS builder
ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /opt/roto_ws

# 换源（中科大）
RUN sed -i 's/archive.ubuntu.com/mirrors.ustc.edu.cn/g' /etc/apt/sources.list && \
    sed -i 's/security.ubuntu.com/mirrors.ustc.edu.cn/g' /etc/apt/sources.list

COPY ./src ./src
COPY ./third_party ./third_party

RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-humble-cyclonedds \
    ros-humble-rmw-cyclonedds-cpp \
    ros-humble-ros2launch \
    python3-colcon-common-extensions \
    build-essential \
    cmake \
    pkg-config \
    python3-pip \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

# 在 builder 阶段的 apt-get install 之后，COPY src 之前加入：

# 安装 Paho MQTT C 库的编译依赖
RUN apt-get update && apt-get install -y --no-install-recommends \
    git \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

# 编译安装 Paho MQTT C 库（C++ 库依赖它）
RUN cd /opt/roto_ws/third_party/paho.mqtt.c && \
    mkdir build && cd build && \
    cmake -DPAHO_BUILD_STATIC=ON -DPAHO_BUILD_SHARED=ON \
          -DPAHO_BUILD_DOCUMENTATION=OFF -DPAHO_BUILD_SAMPLES=OFF \
          -DPAHO_WITH_SSL=TRUE .. && \
    make -j2 && make install && \
    ldconfig

# 编译安装 Paho MQTT C++ 库
RUN cd /opt/roto_ws/third_party/paho.mqtt.cpp && \
    mkdir build && cd build && \
    cmake -DPAHO_BUILD_DOCUMENTATION=OFF -DPAHO_BUILD_SAMPLES=OFF \
          -DPAHO_WITH_SSL=TRUE .. && \
    make -j2 && make install && \
    ldconfig
    
RUN mkdir -p /usr/local/lib/pkgconfig && \
    printf '%s\n' \
    'prefix=/usr/local' \
    'exec_prefix=${prefix}' \
    'libdir=${exec_prefix}/lib' \
    'includedir=${prefix}/include' \
    '' \
    'Name: paho-mqttpp3' \
    'Description: Eclipse Paho MQTT C++ Client Library' \
    'Version: 1.5.0' \
    'Libs: -L${libdir} -lpaho-mqttpp3' \
    'Cflags: -I${includedir}' \
    > /usr/local/lib/pkgconfig/libpaho-mqttpp3.pc

# 在验证 pkg-config 之前，手动为 paho-mqtt3as 创建 .pc 文件
RUN printf '%s\n' \
    'prefix=/usr/local' \
    'exec_prefix=${prefix}' \
    'libdir=${exec_prefix}/lib' \
    'includedir=${prefix}/include' \
    '' \
    'Name: paho-mqtt3as' \
    'Description: Eclipse Paho MQTT C Async Client Library (with SSL)' \
    'Version: 1.3.13' \
    'Libs: -L${libdir} -lpaho-mqtt3as' \
    'Cflags: -I${includedir}' \
    > /usr/local/lib/pkgconfig/libpaho-mqtt3as.pc

RUN pkg-config --modversion libpaho-mqtt3as && \
    pkg-config --modversion libpaho-mqttpp3

# 清理可能残留的旧构建产物，确保干净构建
RUN rm -rf /opt/roto_ws/build /opt/roto_ws/install

RUN . /opt/ros/humble/setup.sh && \
    colcon build --cmake-args -DCMAKE_BUILD_TYPE=Release --event-handlers console_direct+

# ========== 阶段2：运行 runtime ==========
FROM ros:humble-ros-base
ENV DEBIAN_FRONTEND=noninteractive
ENV ROS_DOMAIN_ID=0
ENV RMW_IMPLEMENTATION=rmw_cyclonedds_cpp

RUN sed -i 's/archive.ubuntu.com/mirrors.ustc.edu.cn/g' /etc/apt/sources.list && \
    sed -i 's/security.ubuntu.com/mirrors.ustc.edu.cn/g' /etc/apt/sources.list && \
    apt-get update && apt-get install -y --no-install-recommends \
    ros-humble-cyclonedds \
    ros-humble-rmw-cyclonedds-cpp \
    ros-humble-ros2launch \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/roto_ws
COPY --from=builder /opt/roto_ws/install ./install
COPY launch ./launch

# 显式 source 底层 ROS 环境 + 工作空间，启动板端 launch（不含 sim 工具）
CMD [ "bash", "-c", "source /opt/ros/humble/setup.bash && source install/setup.bash && ros2 launch /opt/roto_ws/launch/launch_board.launch.py" ]
