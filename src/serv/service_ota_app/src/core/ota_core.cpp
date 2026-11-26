#include "service_ota_app/core/ota_core.hpp"
#include "service_ota_app/core/crypto.hpp"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <sys/stat.h>

namespace service_ota_app {
namespace core {

using json = nlohmann::json;
namespace fs = std::filesystem;

// =============================================================================
// 构造：初始化握手路径与下载目录
// =============================================================================
OtaCore::OtaCore(Options opts)
: opts_(std::move(opts)) {
  request_json_path_ = opts_.work_dir + "/request.json";
  result_json_path_  = opts_.work_dir + "/result.json";
  // 握手目录不存在则创建（运行时通常由 docker -v 挂载进来）
  ensure_dir(opts_.work_dir);
  ensure_dir(opts_.download_dir);
}

// =============================================================================
// run：OTA 状态机主流程（阻塞式，建议放 std::thread）
//   IDLE -> DOWNLOADING -> VERIFYING -> INSTALLING -> FINISHED
// 每个阶段通过 on_progress 回调把状态推给 node 层（再转成 OtaStatus msg）
// =============================================================================
bool OtaCore::run(const OtaRequest & req) {
  // --- 1. 初始进度 ---
  OtaProgress p;
  p.ota_task_id    = req.ota_task_id;
  p.target_module  = req.target_module;
  p.target_version = req.target_version;
  emit(p, Stage::kDownloading, 0, Result::kNone, "ota start");

  // --- 2. 下载 ---
  std::string local_path, err;
  if (!do_download(req, local_path, err)) {
    emit(p, Stage::kDownloading, 0, Result::kFailed, "download failed: " + err);
    return false;
  }
  // 下载完成，进度推到 50%（剩余 50% 留给校验+安装）
  emit(p, Stage::kVerifying, 50, Result::kNone, "download done, verifying");

  // --- 3. 校验：sha256 完整性 + Ed25519 来源（架构 E8 摘要与签名分离）---
  if (!do_verify(local_path, req, err)) {
    emit(p, Stage::kVerifying, 50, Result::kFailed, "verify failed: " + err);
    // 校验失败的文件直接删除，避免被误用
    std::error_code ec;
    fs::remove(local_path, ec);
    return false;
  }
  emit(p, Stage::kInstalling, 75, Result::kNone, "verify done, handing off");

  // --- 4. 握手：写 request.json 交给宿主 agent 做 docker load/切 tag/重启 ---
  if (!do_handoff(req, local_path, err)) {
    emit(p, Stage::kInstalling, 75, Result::kFailed, "handoff failed: " + err);
    return false;
  }

  // 宿主 agent 会在重启容器前写 result.json；当前进程可能马上被杀。
  // 这里上报一个"等待宿主执行"的中间状态，新进程起来后再 read_result。
  emit(p, Stage::kInstalling, 90, Result::kNone, "waiting for host agent");
  return true;
}

// =============================================================================
// do_download：libcurl 下载到 *.part，成功后 rename 为正式文件
// =============================================================================
bool OtaCore::do_download(const OtaRequest & req, std::string & local_path,
                          std::string & err) {
  // 从 url 取文件名，避免重复
  local_path = opts_.download_dir + "/" + url_to_filename(req.url);
  const std::string part_path = local_path + ".part";

  // 打开临时文件写入
  FILE * fp = std::fopen(part_path.c_str(), "wb");
  if (!fp) {
    err = "cannot open " + part_path + " for write";
    return false;
  }

  CURL * curl = curl_easy_init();
  if (!curl) { std::fclose(fp); err = "curl_easy_init failed"; return false; }

  // 把 OtaCore this 指针传给进度回调
  curl_easy_setopt(curl, CURLOPT_URL, req.url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
    +[](char * ptr, size_t size, size_t nmemb, void * user) -> size_t {
      // libcurl 写数据回调：直接写文件
      return std::fwrite(ptr, size, nmemb, static_cast<FILE *>(user));
    });
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
  // 进度回调：必须 NOPROGRESS=0 才会触发
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
  curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION,
    +[](void * clientp, curl_off_t dltotal, curl_off_t dlnow,
        curl_off_t, curl_off_t) -> int {
      // clientp 是 this，更新进度（0..50 留给下载阶段）
      auto * self = static_cast<OtaCore *>(clientp);
      if (dltotal > 0) {
        uint8_t pct = static_cast<uint8_t>(dlnow * 50 / dltotal);
        OtaProgress prog;
        prog.stage = Stage::kDownloading;
        prog.progress = pct;
        prog.message = "downloading";
        if (self->on_progress) self->on_progress(prog);
      }
      return 0;
    });
  curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
  // 超时与限速
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, opts_.connect_timeout_ms);
  curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 30L);
  curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1024L);
  if (opts_.max_speed_bytes_sec > 0) {
    curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE,
                     static_cast<curl_off_t>(opts_.max_speed_bytes_sec));
  }
  // HTTPS 校验（生产环境必须为 true）
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

  CURLcode rc = curl_easy_perform(curl);
  curl_easy_cleanup(curl);
  std::fclose(fp);

  if (rc != CURLE_OK) {
    err = std::string("curl error: ") + curl_easy_strerror(rc);
    std::error_code ec;
    fs::remove(part_path, ec);
    return false;
  }

  // 下载完成：part -> 正式文件
  std::error_code ec;
  fs::rename(part_path, local_path, ec);
  if (ec) { err = "rename failed: " + ec.message(); return false; }
  return true;
}

// =============================================================================
// do_verify：先 sha256 摘要校验完整性，再 Ed25519 签名校验来源
// =============================================================================
bool OtaCore::do_verify(const std::string & local_path, const OtaRequest & req,
                        std::string & err) {
  // 1) sha256：防传输损坏 / 镜像被篡改
  std::string actual_sha;
  if (!Crypto::sha256_file(local_path, actual_sha, err)) { return false; }
  if (!Crypto::constant_time_eq(actual_sha, req.sha256)) {
    err = "sha256 mismatch: expect=" + req.sha256 + " actual=" + actual_sha;
    return false;
  }

  // 2) Ed25519 签名：防镜像被冒充（架构 E8：签名与摘要分离，sha256 只管完整性）
  if (!opts_.ed25519_pubkey_pem.empty()) {
    if (!Crypto::verify_ed25519_file(local_path, req.signature,
                                     opts_.ed25519_pubkey_pem, err)) {
      return false;
    }
  }
  return true;
}

// =============================================================================
// do_handoff：把安装请求序列化为 request.json，交给宿主 agent
//   agent 读到后会执行 docker load/pull -> 校验 OCI label -> 切 tag -> 重启容器
// =============================================================================
bool OtaCore::do_handoff(const OtaRequest & req, const std::string & local_path,
                         std::string & err) {
  HandoffRequest hr;
  hr.ota_task_id    = req.ota_task_id;
  hr.target_version = req.target_version;
  hr.action         = "install";
  hr.delivery       = (req.delivery == Delivery::kTar) ? "tar" : "pull";
  hr.image_ref      = (req.delivery == Delivery::kPull) ? req.url : "";
  hr.tar_path       = (req.delivery == Delivery::kTar) ? local_path : "";
  hr.sha256         = req.sha256;
  hr.min_battery_soc = req.min_battery_soc;
  // UTC 毫秒时间戳
  auto now = std::chrono::system_clock::now();
  hr.requested_at_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
      now.time_since_epoch()).count();

  json j;
  j["ota_task_id"]    = hr.ota_task_id;
  j["target_version"] = hr.target_version;
  j["action"]         = hr.action;
  j["delivery"]       = hr.delivery;
  j["image_ref"]      = hr.image_ref;
  j["tar_path"]       = hr.tar_path;
  j["sha256"]         = hr.sha256;
  j["requested_at_ms"] = hr.requested_at_ms;
  j["min_battery_soc"] = hr.min_battery_soc;

  // 原子写：先写 .tmp 再 rename，防止 agent 读到半截 JSON
  const std::string tmp = request_json_path_ + ".tmp";
  std::ofstream f(tmp, std::ios::trunc);
  if (!f) { err = "cannot write " + tmp; return false; }
  f << j.dump(2);
  f.close();
  std::error_code ec;
  fs::rename(tmp, request_json_path_, ec);
  if (ec) { err = "rename failed: " + ec.message(); return false; }
  return true;
}

// =============================================================================
// read_result：宿主 agent 写回 result.json 后，转为 OtaProgress
//   容器重启后由新进程调用，上报最终状态给云端
// =============================================================================
bool OtaCore::read_result(OtaProgress & out, std::string & err) const {
  std::ifstream f(result_json_path_);
  if (!f) { err = "no result.json"; return false; }
  json j;
  try { f >> j; } catch (...) { err = "result.json parse error"; return false; }

  out.ota_task_id    = j.value("ota_task_id", "");
  out.target_version = j.value("target_version", "");
  const std::string status = j.value("status", "");

  // 把 agent 的状态映射到 OtaProgress 的 stage + result
  if (status == "success") {
    out.stage    = Stage::kFinished;
    out.result   = Result::kSuccess;
    out.progress = 100;
  } else if (status == "rollback_success") {
    out.stage    = Stage::kFinished;
    out.result   = Result::kRollbackSuccess;
  } else if (status == "rollback_failed") {
    out.stage    = Stage::kFinished;
    out.result   = Result::kRollbackFailed;
  } else if (status == "installing") {
    out.stage    = Stage::kInstalling;
    out.result   = Result::kNone;
    out.progress = 95;
  } else {
    out.stage    = Stage::kFinished;
    out.result   = Result::kFailed;
  }
  out.message = j.value("message", "");
  return true;
}

// =============================================================================
// reset_handoff：清理 request/result，供重试（不删下载文件）
// =============================================================================
void OtaCore::reset_handoff() {
  std::error_code ec;
  fs::remove(request_json_path_, ec);
  fs::remove(result_json_path_, ec);
}

// =============================================================================
// emit：填充进度字段并触发回调
// =============================================================================
void OtaCore::emit(OtaProgress & p, Stage stage, uint8_t progress,
                   Result result, const std::string & msg) const {
  p.stage    = stage;
  p.progress = progress;
  p.result   = result;
  p.message  = msg;
  if (on_progress) { on_progress(p); }
}

// =============================================================================
// url_to_filename：从 URL 末尾取文件名，fallback 到时间戳
// =============================================================================
std::string OtaCore::url_to_filename(const std::string & url) const {
  auto pos = url.find_last_of('/');
  std::string name = (pos != std::string::npos) ? url.substr(pos + 1) : url;
  if (name.empty()) { name = "ota-image.tar"; }
  return name;
}

// =============================================================================
// ensure_dir：递归创建目录
// =============================================================================
bool OtaCore::ensure_dir(const std::string & dir) const {
  std::error_code ec;
  fs::create_directories(dir, ec);
  return !ec;
}

}  // namespace core
}  // namespace service_ota_app
