package com.sandy.fda.utils;

import javax.crypto.Cipher;
import javax.crypto.Mac;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Machine-bound AES-256-GCM encryption/decryption, matching FDACrypto.cpp
 * exactly. Both sides must produce byte-identical keys given the same
 * MachineGuid — see the STATIC_SECRET comment below.
 *
 * Key = HMAC-SHA256(key = STATIC_SECRET, message = MachineGuid)
 */
public final class FDACrypto {

    private static final byte[] STATIC_SECRET = {
        0x4A, 0x2F, (byte)0x8C, 0x1D, 0x6E, (byte)0x93, 0x5B, 0x7A,
        (byte)0xC4, 0x0E, 0x2B, (byte)0xF1, (byte)0x88, 0x3D, (byte)0x9A, 0x56,
        0x71, (byte)0xE0, 0x4C, (byte)0xA8, 0x2D, 0x67, (byte)0xB3, (byte)0xF9,
        0x1A, 0x5E, (byte)0xC2, (byte)0x84, 0x39, (byte)0xD6, 0x0F, 0x7B
    };

    private static final int GCM_IV_LENGTH = 12;
    private static final int GCM_TAG_LENGTH_BITS = 128; // 16 bytes

    private FDACrypto() {}

    public static String encrypt(String plaintext) {
        try {
            SecretKeySpec key = deriveKey();
            if (key == null) return "";

            byte[] iv = new byte[GCM_IV_LENGTH];
            new SecureRandom().nextBytes(iv);

            Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
            GCMParameterSpec spec = new GCMParameterSpec(GCM_TAG_LENGTH_BITS, iv);
            cipher.init(Cipher.ENCRYPT_MODE, key, spec);

            byte[] ciphertextAndTag = cipher.doFinal(plaintext.getBytes("UTF-8"));

            byte[] combined = new byte[iv.length + ciphertextAndTag.length];
            System.arraycopy(iv, 0, combined, 0, iv.length);
            System.arraycopy(ciphertextAndTag, 0, combined, iv.length, ciphertextAndTag.length);

            return Base64.getEncoder().encodeToString(combined);
        } catch (Exception e) {
            throw new IllegalStateException("[CRYPTO] encrypt failed." , e);
        }
    }

    public static String decrypt(String base64Ciphertext) {
        try {
            SecretKeySpec key = deriveKey();
            if (key == null) return "";

            byte[] combined = Base64.getDecoder().decode(base64Ciphertext);
            if (combined.length < GCM_IV_LENGTH + 16) return ""; 

            byte[] iv = new byte[GCM_IV_LENGTH];
            System.arraycopy(combined, 0, iv, 0, GCM_IV_LENGTH);

            byte[] ciphertextAndTag = new byte[combined.length - GCM_IV_LENGTH];
            System.arraycopy(combined, GCM_IV_LENGTH, ciphertextAndTag, 0, ciphertextAndTag.length);

            Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
            GCMParameterSpec spec = new GCMParameterSpec(GCM_TAG_LENGTH_BITS, iv);
            cipher.init(Cipher.DECRYPT_MODE, key, spec);

            byte[] plaintext = cipher.doFinal(ciphertextAndTag);
            return new String(plaintext, "UTF-8");
        } catch (Exception e) {
            // Includes AEADBadTagException — tampered or corrupted value
            throw new IllegalStateException("[CRYPTO] decrypt failed — value may be corrupted or tampered with. " + e);
        }
    }

    private static SecretKeySpec deriveKey() throws Exception{
        try {
            String machineGuid = getMachineGuid();
            if (machineGuid == null || machineGuid.isEmpty()) return null;

            Mac hmac = Mac.getInstance("HmacSHA256");
            hmac.init(new SecretKeySpec(STATIC_SECRET, "HmacSHA256"));
            byte[] keyBytes = hmac.doFinal(machineGuid.getBytes("UTF-8"));

            return new SecretKeySpec(keyBytes, "AES");
        } catch (Exception e) {
            throw new IllegalStateException("[CRYPTO] deriveKey failed.",e);
        }
    }

    private static String getMachineGuid() {
        try {
            ProcessBuilder pb = new ProcessBuilder(
                "reg", "query",
                "HKLM\\SOFTWARE\\Microsoft\\Cryptography",
                "/v", "MachineGuid"
            );
            pb.redirectErrorStream(true);
            Process proc = pb.start();

            StringBuilder output = new StringBuilder();
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(proc.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    output.append(line).append('\n');
                }
            }
            proc.waitFor();

            // Expected line format: "    MachineGuid    REG_SZ    <guid>"
            Pattern pattern = Pattern.compile("MachineGuid\\s+REG_SZ\\s+(\\S+)");
            Matcher matcher = pattern.matcher(output.toString());
            if (matcher.find()) {
                return matcher.group(1);
            }
            throw new IllegalStateException("[CRYPTO] MachineGuid not found in reg query output");
        } catch (Exception e) {
            throw new IllegalStateException("[CRYPTO] getMachineGuid failed.",e);
        }
    }
}