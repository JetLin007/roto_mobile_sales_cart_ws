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
    libcurl4-openssl-dev \
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
    libcurl4-openssl \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# 从 builder 拷编译好的 paho-mqtt 运行库
COPY --from=builder /usr/local/lib/libpaho-mqtt3as* /usr/local/lib/
COPY --from=builder /usr/local/lib/libpaho-mqtt3cs* /usr/local/lib/
COPY --from=builder /usr/local/lib/libpaho-mqttpp3* /usr/local/lib/
RUN ldconfig

WORKDIR /opt/roto_ws
COPY --from=builder /opt/roto_ws/install ./install
COPY launch ./launch
COPY scripts/cart_ota_agent.py ./scripts/

# OTA 握手目录（宿主通过 docker -v 挂载对齐）
RUN mkdir -p /opt/roto_ws/data/ota /opt/roto_ws/data/ota/cache

# 显式 source 底层 ROS 环境 + 工作空间，启动板端 launch（不含 sim 工具）
CMD [ "bash", "-c", "source /opt/ros/humble/setup.bash && source install/setup.bash && ros2 launch /opt/roto_ws/launch/launch_board.launch.py" ]
