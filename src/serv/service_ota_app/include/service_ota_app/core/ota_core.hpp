#ifndef SERVICE_OTA_APP_CORE_OTA_CORE_HPP_
#define SERVICE_OTA_APP_CORE_OTA_CORE_HPP_

#include "service_ota_app/core/ota_types.hpp"

#include <functional>
#include <string>

namespace service_ota_app {
namespace core {

// =============================================================================
// OtaCore：OTA 任务纯逻辑核心
// -----------------------------------------------------------------------------
// 职责（全部不依赖 ROS）：
//   1. 状态机：IDLE -> DOWNLOADING -> VERIFYING -> INSTALLING -> FINISHED
//   2. libcurl 下载镜像 tar（带进度回调，支持断点/超时/限速）
//   3. 完整性校验：sha256 摘要 + Ed25519 分离签名（架构 E8）
//   4. 文件握手：写 request.json 给宿主 cart-ota-agent，由其执行 docker load/
//      切 tag/重启容器；宿主写回 result.json 后由本类读取并转换为 OtaProgress
//
// 使用方式：
//   OtaCore core(dir, pubkey_pem);
//   core.on_progress = [](const OtaProgress& p){ ... };   // node 层转 ROS msg
//   core.run(request);                                     // 阻塞，建议放线程
//   // 宿主 agent 重启容器后，新进程读 result.json：
//   OtaProgress p; std::string err;
//   if (core.read_result(p, err)) { /* 上报最终状态 */ }
// =============================================================================
class OtaCore {
public:
  // 进度回调：状态机每推进一档 / 下载字节变化时触发
  using ProgressCb = std::function<void(const OtaProgress &)>;

  struct Options {
    std::string work_dir;              // 握手目录，如 /opt/roto_ws/data/ota
    std::string download_dir;          // 下载暂存目录，如 /opt/roto_ws/data/ota/cache
    std::string ed25519_pubkey_pem;    // 镜像签名公钥（架构 E8）
    int64_t connect_timeout_ms = 15000;  // curl 连接超时
    int64_t max_speed_bytes_sec = 0;     // 0 = 不限速
    bool allow_http = false;             // 仅调试用；生产必须 https
  };

  explicit OtaCore(Options opts);

  // 执行一个 OTA 任务（阻塞，node 层应放 std::thread 中调用）
  // 返回 false 表示校验或握手失败，进度回调中可拿到最终 result
  bool run(const OtaRequest & req);

  // 宿主 agent 写回 result.json 后调用，转为 OtaProgress
  bool read_result(OtaProgress & out, std::string & err) const;

  // 仅清理握手目录中的 request/result，不删除已下载文件（供重试）
  void reset_handoff();

  ProgressCb on_progress;

private:
  // ---- 内部阶段函数 ----
  bool do_download(const OtaRequest & req, std::string & local_path, std::string & err);
  bool do_verify(const std::string & local_path, const OtaRequest & req, std::string & err);
  bool do_handoff(const OtaRequest & req, const std::string & local_path, std::string & err);

  // ---- 辅助 ----
  void emit(OtaProgress & p, Stage stage, uint8_t progress, Result result,
            const std::string & msg) const;
  std::string url_to_filename(const std::string & url) const;
  bool ensure_dir(const std::string & dir) const;

  Options opts_;
  std::string request_json_path_;   // work_dir/request.json
  std::string result_json_path_;    // work_dir/result.json
};

}  // namespace core
}  // namespace service_ota_app

#endif  // SERVICE_OTA_APP_CORE_OTA_CORE_HPP_
