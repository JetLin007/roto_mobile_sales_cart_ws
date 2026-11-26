#include "service_ota_app/core/crypto.hpp"

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/bio.h>

#include <array>
#include <cstdint>
#include <fstream>
#include <vector>

namespace service_ota_app {
namespace core {

// 把字节数组转成小写 hex 字符串（OpenSSL 不直接提供 hex 输出）
static std::string to_hex(const unsigned char * buf, size_t len) {
  static const char * k = "0123456789abcdef";
  std::string out;
  out.reserve(len * 2);
  for (size_t i = 0; i < len; ++i) {
    out.push_back(k[buf[i] >> 4]);
    out.push_back(k[buf[i] & 0x0F]);
  }
  return out;
}

// 计算字节串的 sha256（主要供单测使用）
bool Crypto::sha256_bytes(const std::string & data, std::string & out_hex) {
  std::array<unsigned char, EVP_MAX_MD_SIZE> md{};
  unsigned int md_len = 0;
  EVP_MD_CTX * ctx = EVP_MD_CTX_new();
  if (ctx == nullptr) { return false; }
  // OpenSSL 3.x 推荐用 EVP_Digest* 系列，不直接用 SHA256()
  bool ok = EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) == 1 &&
            EVP_DigestUpdate(ctx, data.data(), data.size()) == 1 &&
            EVP_DigestFinal_ex(ctx, md.data(), &md_len) == 1;
  EVP_MD_CTX_free(ctx);
  if (!ok) { return false; }
  out_hex = to_hex(md.data(), md_len);
  return true;
}

// 分块读取大文件并计算 sha256（镜像可达数百 MB，不能一次读入内存）
bool Crypto::sha256_file(const std::string & path, std::string & out_hex, std::string & err) {
  std::ifstream f(path, std::ios::binary);
  if (!f) { err = "cannot open file: " + path; return false; }
  std::array<unsigned char, EVP_MAX_MD_SIZE> md{};
  unsigned int md_len = 0;
  EVP_MD_CTX * ctx = EVP_MD_CTX_new();
  if (ctx == nullptr) { err = "EVP_MD_CTX_new failed"; return false; }
  bool ok = EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) == 1;
  // 64KB 分块
  std::vector<char> buf(1 << 16);
  while (ok && f.good()) {
    f.read(buf.data(), static_cast<std::streamsize>(buf.size()));
    std::streamsize n = f.gcount();
    if (n > 0) {
      ok = EVP_DigestUpdate(ctx, buf.data(), static_cast<size_t>(n)) == 1;
    }
  }
  if (ok) { ok = EVP_DigestFinal_ex(ctx, md.data(), &md_len) == 1; }
  EVP_MD_CTX_free(ctx);
  if (!ok) { err = "digest failed: " + path; return false; }
  out_hex = to_hex(md.data(), md_len);
  return true;
}

// 常量时间比较：避免攻击者通过响应时间推断摘要差异（侧信道攻击）
bool Crypto::constant_time_eq(const std::string & a, const std::string & b) {
  if (a.size() != b.size()) { return false; }
  unsigned char diff = 0;
  for (size_t i = 0; i < a.size(); ++i) {
    diff |= static_cast<unsigned char>(a[i] ^ b[i]);
  }
  return diff == 0;
}

// 校验文件的 Ed25519 分离签名
// 流程：解析公钥 PEM -> base64 解码签名 -> 读文件内容 -> EVP_DigestVerify 校验
bool Crypto::verify_ed25519_file(const std::string & file_path,
                                 const std::string & signature_base64,
                                 const std::string & pubkey_pem,
                                 std::string & err) {
  // 1) 从 PEM 文本加载 Ed25519 公钥
  BIO * kbio = BIO_new_mem_buf(pubkey_pem.data(), static_cast<int>(pubkey_pem.size()));
  if (kbio == nullptr) { err = "BIO_new_mem_buf failed"; return false; }
  EVP_PKEY * pkey = PEM_read_bio_PUBKEY(kbio, nullptr, nullptr, nullptr);
  BIO_free(kbio);
  if (pkey == nullptr) { err = "invalid Ed25519 public key PEM"; return false; }

  // 2) base64 解码签名（OpenSSL BIO base64 filter）
  std::string sig;
  {
    BIO * b64 = BIO_new(BIO_f_base64());
    BIO * mem = BIO_new_mem_buf(signature_base64.data(),
                                static_cast<int>(signature_base64.size()));
    if (b64 == nullptr || mem == nullptr) {
      EVP_PKEY_free(pkey); if (b64) BIO_free(b64); if (mem) BIO_free(mem);
      err = "BIO alloc failed"; return false;
    }
    // 不忽略换行（签名是单行 base64）
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO * chain = BIO_push(b64, mem);
    char out[1024];
    int n = BIO_read(chain, out, sizeof(out));
    if (n > 0) { sig.assign(out, static_cast<size_t>(n)); }
    BIO_free_all(chain);
  }
  if (sig.empty()) { EVP_PKEY_free(pkey); err = "empty signature"; return false; }

  // 3) 读取待校验文件的全部字节
  std::ifstream f(file_path, std::ios::binary);
  if (!f) { EVP_PKEY_free(pkey); err = "cannot open file: " + file_path; return false; }
  std::vector<char> data((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());

  // 4) Ed25519 单次签名校验（Ed25519 不需要指定 digest，直接对原始消息签名）
  EVP_MD_CTX * ctx = EVP_MD_CTX_new();
  bool ok = ctx != nullptr &&
            EVP_DigestVerifyInit(ctx, nullptr, nullptr, nullptr, pkey) == 1 &&
            EVP_DigestVerify(ctx,
                             reinterpret_cast<const unsigned char *>(sig.data()), sig.size(),
                             reinterpret_cast<const unsigned char *>(data.data()), data.size()) == 1;
  if (ctx != nullptr) { EVP_MD_CTX_free(ctx); }
  EVP_PKEY_free(pkey);
  if (!ok) { err = "signature verification failed"; }
  return ok;
}

}  // namespace core
}  // namespace service_ota_app
