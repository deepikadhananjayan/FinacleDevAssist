#define WIN32_LEAN_AND_MEAN
#include "FDACrypto.h"
#include "Logger.h"
#include <Windows.h>
#include <bcrypt.h>
#include <vector>
#include <string>

#pragma comment(lib, "bcrypt.lib")

// -------------------------------------------------------
// Static secret — MUST match FDAConstants.java's STATIC_SECRET exactly,
// byte for byte. This alone is not the encryption key; it's combined
// with the machine's MachineGuid via HMAC-SHA256 (see deriveKey()).
// -------------------------------------------------------
static const unsigned char STATIC_SECRET[32] = {
    0x4A, 0x2F, 0x8C, 0x1D, 0x6E, 0x93, 0x5B, 0x7A,
    0xC4, 0x0E, 0x2B, 0xF1, 0x88, 0x3D, 0x9A, 0x56,
    0x71, 0xE0, 0x4C, 0xA8, 0x2D, 0x67, 0xB3, 0xF9,
    0x1A, 0x5E, 0xC2, 0x84, 0x39, 0xD6, 0x0F, 0x7B
};
// NOTE: replace these with your own randomly generated 32 bytes before
// shipping — the values above are placeholders for illustration only.
// Generate real ones once (e.g. via `openssl rand -hex 32` split into
// byte literals) and hardcode identically on both sides.

// -------------------------------------------------------
// Base64 helpers (Windows CryptStringToBinary/CryptBinaryToString —
// already part of the OS, no new dependency)
// -------------------------------------------------------
#include <wincrypt.h>
#pragma comment(lib, "crypt32.lib")

static std::string base64Encode(const std::vector<unsigned char>& data)
{
    DWORD outLen = 0;
    if (!CryptBinaryToStringA(data.data(), static_cast<DWORD>(data.size()),
        CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, nullptr, &outLen))
        return "";

    std::string result(outLen, '\0');
    if (!CryptBinaryToStringA(data.data(), static_cast<DWORD>(data.size()),
        CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, &result[0], &outLen))
        return "";

    // CryptBinaryToStringA includes a null terminator in outLen — trim it.
    if (!result.empty() && result.back() == '\0')
        result.pop_back();

    return result;
}

static std::vector<unsigned char> base64Decode(const std::string& text)
{
    DWORD outLen = 0;
    if (!CryptStringToBinaryA(text.c_str(), static_cast<DWORD>(text.size()),
        CRYPT_STRING_BASE64, nullptr, &outLen, nullptr, nullptr))
        return {};

    std::vector<unsigned char> result(outLen);
    if (!CryptStringToBinaryA(text.c_str(), static_cast<DWORD>(text.size()),
        CRYPT_STRING_BASE64, result.data(), &outLen, nullptr, nullptr))
        return {};

    return result;
}

// -------------------------------------------------------
// MachineGuid — HKLM\SOFTWARE\Microsoft\Cryptography\MachineGuid
// -------------------------------------------------------
std::string FDACrypto::getMachineGuid()
{
    char buf[64] = {};
    DWORD bufSize = sizeof(buf);
    DWORD type = REG_SZ;

    LONG res = RegGetValueA(
        HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Cryptography",
        "MachineGuid",
        RRF_RT_REG_SZ,
        &type,
        buf,
        &bufSize
    );

    if (res != ERROR_SUCCESS)
    {
        Logger::error("[CRYPTO] Failed to read MachineGuid, error : " + std::to_string(res));
        return "";
    }

    return std::string(buf);
}

// -------------------------------------------------------
// Key derivation — HMAC-SHA256(key=STATIC_SECRET, message=MachineGuid)
// -------------------------------------------------------
std::vector<unsigned char> FDACrypto::deriveKey()
{
    std::string machineGuid = getMachineGuid();
    if (machineGuid.empty())
        return {};

    std::vector<unsigned char> keyOut(32);

    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCRYPT_HASH_HANDLE hHash = nullptr;
    NTSTATUS status;

    status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, BCRYPT_ALG_HANDLE_HMAC_FLAG);
    if (status < 0) { Logger::error("[CRYPTO] BCryptOpenAlgorithmProvider failed"); return {}; }

    status = BCryptCreateHash(hAlg, &hHash, nullptr, 0,
        const_cast<unsigned char*>(STATIC_SECRET), sizeof(STATIC_SECRET), 0);
    if (status < 0) { BCryptCloseAlgorithmProvider(hAlg, 0); Logger::error("[CRYPTO] BCryptCreateHash failed"); return {}; }

    status = BCryptHashData(hHash,
        reinterpret_cast<PUCHAR>(const_cast<char*>(machineGuid.data())),
        static_cast<ULONG>(machineGuid.size()), 0);
    if (status < 0) { BCryptDestroyHash(hHash); BCryptCloseAlgorithmProvider(hAlg, 0); Logger::error("[CRYPTO] BCryptHashData failed"); return {}; }

    status = BCryptFinishHash(hHash, keyOut.data(), static_cast<ULONG>(keyOut.size()), 0);

    BCryptDestroyHash(hHash);
    BCryptCloseAlgorithmProvider(hAlg, 0);

    if (status < 0) { Logger::error("[CRYPTO] BCryptFinishHash failed"); return {}; }

    return keyOut;
}

// -------------------------------------------------------
// encrypt() — AES-256-GCM, random 12-byte IV, output = iv || ciphertext || tag
// -------------------------------------------------------
std::string FDACrypto::encrypt(const std::string& plaintext)
{
    std::vector<unsigned char> key = deriveKey();
    if (key.empty()) return "";

    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCRYPT_KEY_HANDLE hKey = nullptr;
    NTSTATUS status;

    status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, nullptr, 0);
    if (status < 0) { Logger::error("[CRYPTO] encrypt: OpenAlgorithmProvider failed"); return ""; }

    BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE,
        reinterpret_cast<PUCHAR>(const_cast<wchar_t*>(BCRYPT_CHAIN_MODE_GCM)),
        sizeof(BCRYPT_CHAIN_MODE_GCM), 0);

    status = BCryptGenerateSymmetricKey(hAlg, &hKey, nullptr, 0, key.data(), static_cast<ULONG>(key.size()), 0);
    if (status < 0) { BCryptCloseAlgorithmProvider(hAlg, 0); Logger::error("[CRYPTO] encrypt: GenerateSymmetricKey failed"); return ""; }

    unsigned char iv[12];
    if (BCryptGenRandom(nullptr, iv, sizeof(iv), BCRYPT_USE_SYSTEM_PREFERRED_RNG) < 0)
    {
        BCryptDestroyKey(hKey); BCryptCloseAlgorithmProvider(hAlg, 0);
        Logger::error("[CRYPTO] encrypt: GenRandom (IV) failed");
        return "";
    }

    unsigned char tag[16];
    std::vector<unsigned char> ciphertext(plaintext.size());
    ULONG resultLen = 0;

    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO authInfo;
    BCRYPT_INIT_AUTH_MODE_INFO(authInfo);
    authInfo.pbNonce = iv;
    authInfo.cbNonce = sizeof(iv);
    authInfo.pbTag = tag;
    authInfo.cbTag = sizeof(tag);

    status = BCryptEncrypt(hKey,
        reinterpret_cast<PUCHAR>(const_cast<char*>(plaintext.data())), static_cast<ULONG>(plaintext.size()),
        &authInfo,
        nullptr, 0,
        ciphertext.empty() ? nullptr : ciphertext.data(), static_cast<ULONG>(ciphertext.size()),
        &resultLen, 0);

    BCryptDestroyKey(hKey);
    BCryptCloseAlgorithmProvider(hAlg, 0);

    if (status < 0) { Logger::error("[CRYPTO] encrypt: BCryptEncrypt failed"); return ""; }

    std::vector<unsigned char> combined;
    combined.reserve(sizeof(iv) + resultLen + sizeof(tag));
    combined.insert(combined.end(), iv, iv + sizeof(iv));
    combined.insert(combined.end(), ciphertext.begin(), ciphertext.begin() + resultLen);
    combined.insert(combined.end(), tag, tag + sizeof(tag));

    return base64Encode(combined);
}

// -------------------------------------------------------
// decrypt() — reverses encrypt(); empty string on any failure (including
// auth tag mismatch, which GCM uses to detect tampering/corruption)
// -------------------------------------------------------
std::string FDACrypto::decrypt(const std::string& base64Ciphertext)
{
    std::vector<unsigned char> key = deriveKey();
    if (key.empty()) return "";

    std::vector<unsigned char> combined = base64Decode(base64Ciphertext);
    if (combined.size() < 12 + 16) return ""; // too short to be iv+tag at minimum

    const unsigned char* iv = combined.data();
    const unsigned char* tag = combined.data() + combined.size() - 16;
    const unsigned char* ct = combined.data() + 12;
    size_t ctLen = combined.size() - 12 - 16;

    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCRYPT_KEY_HANDLE hKey = nullptr;
    NTSTATUS status;

    status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, nullptr, 0);
    if (status < 0) { Logger::error("[CRYPTO] decrypt: OpenAlgorithmProvider failed"); return ""; }

    BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE,
        reinterpret_cast<PUCHAR>(const_cast<wchar_t*>(BCRYPT_CHAIN_MODE_GCM)),
        sizeof(BCRYPT_CHAIN_MODE_GCM), 0);

    status = BCryptGenerateSymmetricKey(hAlg, &hKey, nullptr, 0, key.data(), static_cast<ULONG>(key.size()), 0);
    if (status < 0) { BCryptCloseAlgorithmProvider(hAlg, 0); Logger::error("[CRYPTO] decrypt: GenerateSymmetricKey failed"); return ""; }

    std::vector<unsigned char> plaintext(ctLen);
    ULONG resultLen = 0;

    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO authInfo;
    BCRYPT_INIT_AUTH_MODE_INFO(authInfo);
    authInfo.pbNonce = const_cast<PUCHAR>(iv);
    authInfo.cbNonce = 12;
    authInfo.pbTag = const_cast<PUCHAR>(tag);
    authInfo.cbTag = 16;

    status = BCryptDecrypt(hKey,
        const_cast<PUCHAR>(ct), static_cast<ULONG>(ctLen),
        &authInfo,
        nullptr, 0,
        plaintext.empty() ? nullptr : plaintext.data(), static_cast<ULONG>(plaintext.size()),
        &resultLen, 0);

    BCryptDestroyKey(hKey);
    BCryptCloseAlgorithmProvider(hAlg, 0);

    if (status < 0)
    {
        // Includes auth tag verification failure — tampered or corrupted value
        Logger::error("[CRYPTO] decrypt failed — value may be corrupted or tampered with");
        return "";
    }

    return std::string(reinterpret_cast<char*>(plaintext.data()), resultLen);
}