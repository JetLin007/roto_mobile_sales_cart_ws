# roto_mobile_sales_cart_ws

移动售卖车 ROS2 Humble 工作空间（RK3576S / NK-6A21，Ubuntu 22.04 arm64 + Docker + CycloneDDS）。
基线文档：《架构详细设计与逻辑流程文档 v1.2》（2026-11-16，已同步协议 v1.3）+《云端与端侧通信协议 v1.3》。

## 当前进度

| 步骤 | 内容 | 状态 |
|---|---|---|
| 1 | 工程骨架 21 包（L2×10 / L3×6 / L4×4 + 契约包）+ `robot_msgs` 44 msg / 9 srv | ✅ 已完成 |
| 2 | `driver_chassis_rs485`（RS485 帧 + 状态机 + 20ms + 60ms 超时） | ⏳ 待确认 |
| 3 | `driver_rs485_dev`（Modbus RTU 多端口） | 未开始 |
| 4 | 其余 HAL：lidar / ultrasonic / gps / camera / audio / alarm_light / remote_24g | 未开始 |
| 5 | L3 服务：state_mgr / logger / config / cloud_comm / local_comm / ota_app | 未开始 |
| 6 | L4 业务：order_task / alarm_warn / power_mgr / remote_video | 未开始 |
| 7 | 部署层：launch / arm64 镜像 / udev / systemd / cart-ota-agent | 未开始 |
| 8 | 无硬件 mock + 协议第 5 节验收场景脚本 | 未开始 |

## 工程结构（21 个包，对应架构 v1.2 §7）

| 层 | 包 | 职责 | 主接口 |
|---|---|---|---|
| 契约 | `robot_msgs` | 44 msg / 9 srv | 全部 Topic/Service 类型 |
| L2 HAL | `driver_chassis_rs485` | 底盘控制帧 + YMODEM OTA | `/cmd_vel`、`/chassis/status`、`/chassis/odom`、`/chassis/ota/*` |
| L2 HAL | `driver_rs485_dev` | Modbus RTU 多路外设 | `/peripheral/cmd`、`/peripheral/status` |
| L2 HAL | `driver_lidar` | 单线/多线雷达 | `/scan` |
| L2 HAL | `driver_ultrasonic` | 超声波避障 | `/ultrasonic/range` |
| L2 HAL | `driver_gps` | GNSS（v1.2 P10 后不再供 PPS） | `/gps/fix` |
| L2 HAL | `driver_camera` | 图像采集 | `/camera/image_raw` |
| L2 HAL | `driver_audio` | 语音播报/拾音 | `/voice/play` |
| L2 HAL | `driver_alarm_light` | 声光报警 | `/alarm/light` |
| L2 HAL | `driver_remote_24g` | 2.4G 遥控 + 急停 | `/remote/cmd` |
| L2 HAL | `driver_npu_perception` | 预留（RK3588 迁移启用） | — |
| L3 | `service_state_mgr` | 整车状态机 | `/state/get`、`/state/set` |
| L3 | `service_logger` | 本地日志 + 云端上报 | `/log` |
| L3 | `service_config` | 配置热更新 | `/config/reload` |
| L3 | `service_cloud_comm` | 云端 Broker（MQTT5 / 4G） | `/cloud/*` |
| L3 | `service_local_comm` | 内网 Broker（Kiosk / RNDIS） | `/local/*` |
| L3 | `service_ota_app` | 应用与底盘 OTA | `/ota/*`、`/chassis/ota/enter` |
| L4 | `app_order_task` | 订单调度与 action_id 履约 | `/order/*`、`/task/event` |
| L4 | `app_remote_video` | WebRTC 推流与信令 | `/remote/video/*` |
| L4 | `app_alarm_warn` | 告警与语音编排 | `/alarm/*` |
| L4 | `app_power_mgr` | 电源与充电管理 | `/battery/*` |

### L1 协议层落在哪里（不单独成包）

协议层不是包，是各包 `core/` 里的实现或运行时配置：

| 协议 | 落点 |
|---|---|
| RS485 自定义帧 + YMODEM | `driver_chassis_rs485/src/core/`（帧编解码、状态机） |
| Modbus RTU | `driver_rs485_dev/src/core/`（libmodbus） |
| MQTT 5.0 | `service_cloud_comm` / `service_local_comm` 的 `core/`（paho） |
| WebRTC | `app_remote_video/src/core/`（信令与推流） |
| RNDIS | 宿主网络 + 容器 `--net=host`，不入代码 |
| DDS | 容器内 `rmw_cyclonedds_cpp`，由环境变量与 QoS 配置控制 |

## 目录

```
roto_mobile_sales_cart_ws/
├── src/robot_msgs/        # 契约包：44 msg + 9 srv（唯一跨层接口来源）
│   ├── msg/
│   └── srv/
├── config/cart_config.json  # 统一配置入口（/opt/roto_ws/config）
├── udev/99-cart.rules       # 设备固定别名（架构 §8.4）
├── docker/                  # 镜像分层与 compose（步骤 7）
├── scripts/build.sh         # colcon 构建入口
└── docs/
    ├── 00-接口裁决.md        # 两文档 6 处冲突的定稿结论
    └── 01-契约清单.md        # topic/service/MQTT 映射与字段口径
```

## 路径约定（源码仓 ≠ 运行时根）

两个名字不是同一件事，别混用：

| 概念 | WSL2 | RK3576S | 说明 |
|---|---|---|---|
| **源码仓**（git 仓库） | `~/ws/roto_mobile_sales_cart_ws` | `/data/workspace/roto_mobile_sales_cart_ws` | 仓名取自架构文档标题；`build/ install/ log/` 在仓内生成，由 `.gitignore` 排除 |
| **运行时根**（宿主可变数据） | 无 | `/opt/roto_ws` | 架构 §4.5 / §8.2（M6 统一）规定的挂载点：`config/`，另有 `log/`、`data/cart.db` |
| **容器内二进制** | — | 镜像里的 `/opt/roto_ws/install`（随镜像 OTA） | 宿主不存二进制，只有可变数据 |

为什么运行时根是 `/opt`：FHS 里 `/opt` 放第三方/附加软件（`ros:humble` 镜像的 `/opt/ros` 就是同一惯例）；用 `/home` 会随用户变化，systemd 单元与 `docker -v` 挂载都要求稳定绝对路径。为什么不共用仓名：容器只挂 `config/log/data` 三个可变目录，镜像本身不可变，OTA 换镜像不动数据；若把源码仓直接当运行时根，`git pull` 会与运行中的 `data/cart.db`、`log/` 混在一棵树里，回滚时容易误伤数据。

## 构建拓扑（双轨：WSL2 交叉构建 + 板端 git 原生验证）

| 环境 | 角色 | 具体职责 |
|---|---|---|
| WSL2 (amd64) | 开发 + 交叉构建 | `colcon` 编 amd64 跑单测；`docker buildx --platform linux/arm64` 交叉构建镜像并 push |
| RK3576S (aarch64) | 发布验证 + 运行 | `git pull` 到固定 commit；容器内 `colcon` 原生编译；起容器跑联调 |

两条路的产物必须能追到**同一个 commit**：镜像写 OCI label（`org.opencontainers.image.revision`），板端编译写 `install/BUILD_INFO.txt`。

```bash
# A. 日常开发：WSL2 编 amd64 跑纯逻辑单测（architecture 保持可移植）
./scripts/build.sh --packages-select robot_msgs

# B. 出 arm64 镜像（WSL2，buildx 交叉构建）
./scripts/build_image.sh -t registry.example.com/roto-cart:1.3.0
./scripts/build_image.sh --tar out/roto-cart-oci.tar          # 离线包（板端 skopeo 转 docker-archive）
./scripts/build_image.sh --multiarch                          # amd64+arm64 manifest

# C. 发布验证：板端 git 更新代码 → 原生编译 → 运行
ssh roto@192.168.1.50 'cd /data/workspace/roto_mobile_sales_cart_ws && git fetch --all --tags && ./scripts/build_on_board.sh --pull --require-clean -j 4'
ssh roto@192.168.1.50 'source /data/workspace/roto_mobile_sales_cart_ws/install/setup.bash && ros2 interface list | grep -c robot_msgs'
```

- 板端编译并行度 `-j4` 起步（RK3576 是 4×A72 + 4×A53，4GB 板子 `-j8` 易 OOM）。
- `build_on_board.sh` 用宿主 UID 跑容器（`--user $(id -u):$(id -g)`），ccache 存 `roto-ccache` 卷。
- 发布判定：`git describe --dirty` 无 `-dirty`，且镜像 label 与 `BUILD_INFO.txt` 的 `git_rev` 相同。
- `sync_to_board.sh` 仅调试用，发布验证不走 rsync。
- 仓库必须保留 `.gitignore`（已提供）：板端在仓内编译会生成 `build/ install/ log/`，不忽略会被 `--require-clean` 判为脏工作区。

首次建仓（在你的 GitLab/Gitea 上）：

```bash
git init && git add . && git commit -m "chore: 基线（架构v1.2 + 协议v1.3 契约包）"
git remote add origin git@your-gitlab:roto/mobile_sales_cart_ws.git && git push -u origin main
git tag v1.3.0-rc1 && git push --tags
```

## 相对 v1.1 的 7 处修订（已落到契约层）

1. `OrderEvent` → `TaskEvent + TaskAction`（`task_id`/`event_version`/数量账本），WAN 与 LAN 共用。
2. 云端状态改 `state/report` 单主题，LWT 用 `CartState.STATE_OFFLINE` 发小 JSON。
3. 本地主题收敛为 v1.2 §5.2 六条，并为 `local/cart/status`、`local/order/start`、`local/order/cancel`、`local/ui/heartbeat` 各建专用类型。
4. `CmdVel` / `PeripheralCmd` 增加 `expire_at`、`control_session_id`、`seq`、`transaction_id`。
5. 状态机补第 8 状态 `OFFLINE`（全禁止，由云端与本地 watchdog 判定）。
6. 时间基准改为“云端四时间戳为主、GNSS 可选”，`driver_gps` 不再供 PPS。
7. 新增 `ErrorCode`（14 个统一错误码）与 `RemoteSession`（远控鉴权/保活事件）。

## 分层与依赖（硬约束）

```
L4 app_*  ──► L3 service_*  ──► L2 driver_*  ──► L1 通道  ──► L0 运行时
                 └────────────► robot_msgs ◄────────────┘
```

- `driver_*` 只依赖 `robot_msgs` 与第三方库，**禁止**依赖 `service_*` / `app_*`。
- `service_*` 只依赖 `robot_msgs`，**禁止**依赖 `app_*`。
- L2 驱动之间禁止点对点调用，必须经 DDS Topic 或 L3 中转。
- 驱动节点不承载业务逻辑（订单、计费、库存判定都不在驱动层）。

## 运行时基线

- 每节点独立进程，`launch` 一律 `respawn=True`；容器 `restart=always`，宿主 `cart-watchdog` 二次兜底。
- 单机 DDS 用 CycloneDDS + `--net=host`；4G、RNDIS 直通。
- 时间基准统一 UTC 毫秒，`GNSS + chrony` 为唯一时源。
- 双 Broker：内网 Broker 跑在 RK3576S（mosquitto，服务 Kiosk），云端 Broker 服务车云。

> （注：部分内容由豆包 AI 生成）