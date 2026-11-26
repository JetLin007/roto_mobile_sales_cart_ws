#ifndef SERVICE_OTA_APP_CORE_CRYPTO_HPP_
#define SERVICE_OTA_APP_CORE_CRYPTO_HPP_

#include <string>

namespace service_ota_app {
namespace core {

// =============================================================================
// Crypto：OTA 完整性与来源校验工具
// 架构 E8 裁决：摘要与签名分离——
//   - sha256：校验镜像完整性（传输损坏/篡改检测）
//   - Ed25519：校验镜像来源（防止冒充发布者）
// 两者独立校验，缺一不可。
// =============================================================================
class Crypto {
public:
  // 计算文件的 sha256 摘要，输出小写 hex 字符串
  // 返回 false 时 err 填原因
  static bool sha256_file(const std::string & path, std::string & out_hex, std::string & err);

  // 计算字节串的 sha256 摘要（用于单测）
  static bool sha256_bytes(const std::string & data, std::string & out_hex);

  // 校验文件的 Ed25519 分离签名
  //   file_path        : 待校验文件
  //   signature_base64 : 对文件原始字节的 Ed25519 签名，base64 编码
  //   pubkey_pem       : Ed25519 公钥的 PEM 文本
  static bool verify_ed25519_file(const std::string & file_path,
                                  const std::string & signature_base64,
                                  const std::string & pubkey_pem,
                                  std::string & err);

  // 常量时间字符串比较，防止时序侧信道攻击
  static bool constant_time_eq(const std::string & a, const std::string & b);
};

}  // namespace core
}  // namespace service_ota_app

#endif  // SERVICE_OTA_APP_CORE_CRYPTO_HPP_
