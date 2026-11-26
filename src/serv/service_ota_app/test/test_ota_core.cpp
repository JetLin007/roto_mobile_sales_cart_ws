// =============================================================================
// test_ota_core.cpp — service_ota_app core 层单元测试
// -----------------------------------------------------------------------------
// 覆盖：sha256 摘要、Ed25519 签名校验、常量时间比较、OtaCore 下载+握手、
//       read_result 解析。全部不依赖 ROS，可在 WSL2 直接编译运行。
// 编译：见同目录 run_tests.sh 或底部注释。
// =============================================================================
#include "service_ota_app/core/crypto.hpp"
#include "service_ota_app/core/ota_core.hpp"

#include <gtest/gtest.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include <filesystem>

namespace fs = std::filesystem;
using service_ota_app::core::Crypto;
using service_ota_app::core::Delivery;
using service_ota_app::core::OtaCore;
using service_ota_app::core::OtaProgress;
using service_ota_app::core::OtaRequest;
using service_ota_app::core::Result;
using service_ota_app::core::Stage;
using service_ota_app::core::TargetModule;

// 测试用临时目录
static const std::string kTestDir = "/tmp/ota_core_test";

// =============================================================================
// 1. sha256_bytes 已知向量
// =============================================================================
TEST(CryptoSha256, KnownVectorEmpty) {
  std::string out;
  ASSERT_TRUE(Crypto::sha256_bytes("", out));
  // echo -n "" | sha256sum
  EXPECT_EQ(out, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
}

TEST(CryptoSha256, KnownVectorHello) {
  std::string out;
  ASSERT_TRUE(Crypto::sha256_bytes("hello", out));
  // echo -n "hello" | sha256sum
  EXPECT_EQ(out, "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");
}

// =============================================================================
// 2. sha256_file 与 sha256sum 命令比对
// =============================================================================
TEST(CryptoSha256, FileMatchesSha256sum) {
  const std::string path = kTestDir + "/data.bin";
  // 写 1MB 随机数据
  std::ofstream f(path, std::ios::binary);
  for (int i = 0; i < 1024 * 1024; ++i) f.put(static_cast<char>(i & 0xFF));
  f.close();

  std::string out, err;
  ASSERT_TRUE(Crypto::sha256_file(path, out, err));

  // 用系统 sha256sum 交叉验证
  std::string cmd = "sha256sum " + path + " | awk '{print $1}'";
  FILE * pipe = popen(cmd.c_str(), "r");
  ASSERT_TRUE(pipe != nullptr);
  char buf[128] = {0};
  fgets(buf, sizeof(buf), pipe);
  pclose(pipe);
  std::string expected(buf);
  expected.erase(expected.find_last_not_of(" \n\r\t") + 1);

  EXPECT_EQ(out, expected);
}

// =============================================================================
// 3. 常量时间比较
// =============================================================================
TEST(CryptoConstantTimeEq, SameString) {
  EXPECT_TRUE(Crypto::constant_time_eq("abc123", "abc123"));
}

TEST(CryptoConstantTimeEq, DifferentLength) {
  EXPECT_FALSE(Crypto::constant_time_eq("abc", "abcd"));
}

TEST(CryptoConstantTimeEq, DifferentContent) {
  EXPECT_FALSE(Crypto::constant_time_eq("abc123", "abc124"));
}

TEST(CryptoConstantTimeEq, BothEmpty) {
  EXPECT_TRUE(Crypto::constant_time_eq("", ""));
}

// =============================================================================
// 4. Ed25519 签名校验（用 openssl CLI 生成密钥+签名，本类校验）
// =============================================================================
class Ed25519Test : public ::testing::Test {
protected:
  void SetUp() override {
    fs::create_directories(kTestDir);
    key_path_   = kTestDir + "/sign.key";
    pub_path_   = kTestDir + "/sign.pub";
    data_path_  = kTestDir + "/signed.bin";
    sig_path_   = kTestDir + "/signed.sig";

    // 生成 Ed25519 密钥对
    std::system(("openssl genpkey -algorithm Ed25519 -out " + key_path_ + " 2>/dev/null").c_str());
    std::system(("openssl pkey -in " + key_path_ + " -pubout -out " + pub_path_ + " 2>/dev/null").c_str());

    // 写待签名数据
    std::ofstream d(data_path_, std::ios::binary);
    d << "OTA image payload for Ed25519 signature test";
    d.close();

    // 用 openssl 签名（Ed25519 直接对原始数据签名，不需要 digest）
    std::system(("openssl pkeyutl -sign -inkey " + key_path_ +
                 " -rawin -in " + data_path_ + " -out " + sig_path_ + " 2>/dev/null").c_str());

    // 读公钥 PEM
    std::ifstream pf(pub_path_);
    pubkey_pem_.assign((std::istreambuf_iterator<char>(pf)),
                        std::istreambuf_iterator<char>());

    // 读签名并 base64 编码
    std::ifstream sf(sig_path_, std::ios::binary);
    std::string sig_raw((std::istreambuf_iterator<char>(sf)),
                         std::istreambuf_iterator<char>());
    signature_b64_ = base64_encode(sig_raw);
  }

  static std::string base64_encode(const std::string & in) {
    static const char * tbl =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
      val = (val << 8) + c;
      valb += 8;
      while (valb >= 0) {
        out.push_back(tbl[(val >> valb) & 0x3F]);
        valb -= 6;
      }
    }
    if (valb > -6) out.push_back(tbl[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
  }

  std::string key_path_, pub_path_, data_path_, sig_path_;
  std::string pubkey_pem_, signature_b64_;
};

TEST_F(Ed25519Test, ValidSignaturePasses) {
  std::string err;
  EXPECT_TRUE(Crypto::verify_ed25519_file(data_path_, signature_b64_, pubkey_pem_, err))
      << "err=" << err;
}

TEST_F(Ed25519Test, TamperedFileFails) {
  // 篡改文件内容
  std::ofstream d(data_path_, std::ios::binary | std::ios::app);
  d << "TAMPERED";
  d.close();

  std::string err;
  EXPECT_FALSE(Crypto::verify_ed25519_file(data_path_, signature_b64_, pubkey_pem_, err));
}

TEST_F(Ed25519Test, WrongSignatureFails) {
  // 用错误的签名（全零 base64）
  std::string err;
  std::string bad_sig(88, 'A');  // 伪造的 base64
  EXPECT_FALSE(Crypto::verify_ed25519_file(data_path_, bad_sig, pubkey_pem_, err));
}

TEST_F(Ed25519Test, WrongPubkeyFails) {
  // 生成另一对密钥，用其公钥校验
  std::string other_key = kTestDir + "/other.key";
  std::string other_pub = kTestDir + "/other.pub";
  std::system(("openssl genpkey -algorithm Ed25519 -out " + other_key + " 2>/dev/null").c_str());
  std::system(("openssl pkey -in " + other_key + " -pubout -out " + other_pub + " 2>/dev/null").c_str());
  std::ifstream pf(other_pub);
  std::string other_pem((std::istreambuf_iterator<char>(pf)),
                         std::istreambuf_iterator<char>());

  std::string err;
  EXPECT_FALSE(Crypto::verify_ed25519_file(data_path_, signature_b64_, other_pem, err));
}

// =============================================================================
// 5. OtaCore 下载 + 校验 + 握手（file:// 协议，无需 HTTP 服务器）
// =============================================================================
TEST(OtaCoreRun, DownloadVerifyAndHandoff) {
  const std::string work_dir = kTestDir + "/work";
  const std::string dl_dir   = kTestDir + "/cache";
  fs::remove_all(work_dir);
  fs::remove_all(dl_dir);
  fs::create_directories(dl_dir);

  // 构造一个待下载的文件
  const std::string src_file = dl_dir + "/test-image.tar";
  const std::string content = "fake docker image tar content for ota test";
  {
    std::ofstream f(src_file, std::ios::binary);
    f << content;
  }
  // 计算其 sha256
  std::string expected_sha, err;
  ASSERT_TRUE(Crypto::sha256_file(src_file, expected_sha, err));

  // 构造 OtaRequest，url 用 file:// 协议
  OtaRequest req;
  req.ota_task_id    = "unit-test-001";
  req.target_module  = TargetModule::kApp;
  req.target_version = "9.9.9-test";
  req.url            = "file://" + src_file;
  req.sha256         = expected_sha;
  req.signature      = "";   // 跳过签名校验
  req.delivery       = Delivery::kTar;

  OtaCore::Options opts;
  opts.work_dir       = work_dir;
  opts.download_dir   = dl_dir;
  opts.ed25519_pubkey_pem = "";  // 不校验签名

  OtaCore core(opts);

  // 收集进度回调
  std::vector<Stage> stages;
  std::vector<Result> results;
  core.on_progress = [&](const auto & p) {
    stages.push_back(p.stage);
    results.push_back(p.result);
  };

  bool ok = core.run(req);
  EXPECT_TRUE(ok);

  // 验证进度推进顺序：DOWNLOADING -> VERIFYING -> INSTALLING
  ASSERT_FALSE(stages.empty());
  EXPECT_EQ(stages.front(), Stage::kDownloading);
  bool has_verifying = std::find(stages.begin(), stages.end(), Stage::kVerifying) != stages.end();
  bool has_installing = std::find(stages.begin(), stages.end(), Stage::kInstalling) != stages.end();
  EXPECT_TRUE(has_verifying);
  EXPECT_TRUE(has_installing);

  // 验证 request.json 已写入
  const std::string req_json = work_dir + "/request.json";
  EXPECT_TRUE(fs::exists(req_json));

  // 验证下载的文件存在且摘要匹配
  const std::string dl_file = dl_dir + "/test-image.tar";
  EXPECT_TRUE(fs::exists(dl_file));
  std::string actual_sha;
  ASSERT_TRUE(Crypto::sha256_file(dl_file, actual_sha, err));
  EXPECT_EQ(actual_sha, expected_sha);
}

TEST(OtaCoreRun, Sha256MismatchFails) {
  const std::string work_dir = kTestDir + "/work_bad";
  const std::string dl_dir   = kTestDir + "/cache_bad";
  fs::remove_all(work_dir);
  fs::remove_all(dl_dir);
  fs::create_directories(dl_dir);

  const std::string src_file = dl_dir + "/test-image.tar";
  { std::ofstream f(src_file, std::ios::binary); f << "some content"; }

  OtaRequest req;
  req.ota_task_id    = "unit-test-bad";
  req.target_version = "0.0.0";
  req.url            = "file://" + src_file;
  req.sha256         = "0000000000000000000000000000000000000000000000000000000000000000";
  req.delivery       = Delivery::kTar;

  OtaCore::Options opts{work_dir, dl_dir, ""};
  OtaCore core(opts);

  Result final_result = Result::kNone;
  core.on_progress = [&](const auto & p) { final_result = p.result; };

  bool ok = core.run(req);
  EXPECT_FALSE(ok);
  EXPECT_EQ(final_result, Result::kFailed);

  // 校验失败的文件应被删除
  EXPECT_FALSE(fs::exists(dl_dir + "/test-image.tar"));
  // request.json 不应被写入（未到握手阶段）
  EXPECT_FALSE(fs::exists(work_dir + "/request.json"));
}

// =============================================================================
// 6. read_result 解析宿主 agent 写回的 result.json
// =============================================================================
TEST(OtaCoreReadResult, Success) {
  const std::string work_dir = kTestDir + "/result_success";
  fs::remove_all(work_dir);
  fs::create_directories(work_dir);

  std::ofstream f(work_dir + "/result.json");
  f << R"({
    "ota_task_id": "task-001",
    "target_version": "1.2.3",
    "status": "success",
    "message": "installed image",
    "image_ref": "roto-cart:1.2.3",
    "timestamp_ms": 1700000000000
  })";
  f.close();

  OtaCore::Options opts{work_dir, work_dir, ""};
  OtaCore core(opts);

  OtaProgress p;
  std::string err;
  ASSERT_TRUE(core.read_result(p, err));
  EXPECT_EQ(p.ota_task_id, "task-001");
  EXPECT_EQ(p.target_version, "1.2.3");
  EXPECT_EQ(p.stage, Stage::kFinished);
  EXPECT_EQ(p.result, Result::kSuccess);
  EXPECT_EQ(p.progress, 100);
}

TEST(OtaCoreReadResult, RollbackSuccess) {
  const std::string work_dir = kTestDir + "/result_rb";
  fs::remove_all(work_dir);
  fs::create_directories(work_dir);

  std::ofstream f(work_dir + "/result.json");
  f << R"({"ota_task_id":"t2","status":"rollback_success","message":"rolled back"})";
  f.close();

  OtaCore::Options opts{work_dir, work_dir, ""};
  OtaCore core(opts);
  OtaProgress p; std::string err;
  ASSERT_TRUE(core.read_result(p, err));
  EXPECT_EQ(p.result, Result::kRollbackSuccess);
}

TEST(OtaCoreReadResult, NoFile) {
  const std::string work_dir = kTestDir + "/result_none";
  fs::remove_all(work_dir);
  fs::create_directories(work_dir);

  OtaCore::Options opts{work_dir, work_dir, ""};
  OtaCore core(opts);
  OtaProgress p; std::string err;
  EXPECT_FALSE(core.read_result(p, err));
}

// =============================================================================
int main(int argc, char ** argv) {
  fs::create_directories(kTestDir);
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  // 清理
  std::error_code ec;
  fs::remove_all(kTestDir, ec);
  return ret;
}
