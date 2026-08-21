#pragma once

#include <string>
#include <vector>

// Machine-bound AES-256-GCM encryption for sensitive property values
// (currently: c24.env.<name>.username / .password). Uses Windows BCrypt —
// no third-party crypto dependency.
//
// Key = HMAC-SHA256(key = STATIC_SECRET, message = MachineGuid)
// This binds decryption to the local machine: even with the static secret
// extracted from the binary, a copied properties file is useless on a
// different PC. Java must derive the IDENTICAL key the same way — see
// FDACrypto.java. The static secret bytes below MUST match exactly on
// both sides.

class FDACrypto
{
public:
    // Returns base64(iv[12] || ciphertext || tag[16]), or empty string on failure.
    static std::string encrypt(const std::string& plaintext);

    // Reverses encrypt(). Returns empty string on failure (including if
    // the auth tag doesn't verify — GCM detects tampering/corruption).
    static std::string decrypt(const std::string& base64Ciphertext);

private:
    static std::vector<unsigned char> deriveKey();
    static std::string getMachineGuid();
};