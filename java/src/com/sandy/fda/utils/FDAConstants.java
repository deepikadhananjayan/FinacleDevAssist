package com.sandy.fda.utils;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.util.Properties;

public class FDAConstants {

    private static String xmlPath;
    private static int port;
    private static File propFile;

    private static boolean loaded = false;

    private FDAConstants() {
    }

    public static boolean load() {

        if (loaded) {
            return true;
        }

        try {
            // TODO Change it Compiling to JAR
            // String jarPath = FDAConstants.class
            //         .getProtectionDomain()
            //         .getCodeSource()
            //         .getLocation()
            //         .toURI()
            //         .getPath();

            //File appDirectory = new File(jarPath).getParentFile();

             File appDirectory = new File("D:\\Santhosh\\Personal Learning\\Finacle Validator\\FDA\\java\\");

            propFile = new File(appDirectory, "fdaplugin.properties");

            if (!propFile.exists()) {
                FDALogger.error(
                        new Exception(
                                "fdaplugin.properties not found : "
                                        + propFile.getAbsolutePath()));

                return false;
            }

            Properties props = new Properties();

            try (FileInputStream fis = new FileInputStream(propFile)) {
                props.load(fis);
            }

            String portValue = props.getProperty("java.port");

            if (portValue == null) {
                throw new Exception(
                        "[java.port] missing in properties");
            }

            port = Integer.parseInt(portValue);

            File tokenXml = new File(appDirectory, "tokens.xml");

            if (!tokenXml.exists()) {
                throw new FileNotFoundException(
                        "tokens.xml file is missing");
            }

            xmlPath = tokenXml.getAbsolutePath();

            loaded = true;

            FDALogger.info(
                    "FDA Constants loaded successfully");

            return true;
        } catch (Exception e) {
            FDALogger.error(e);
            return false;
        }
    }

    public static void updatePortInProperties(int actualPort) throws Exception {
        Properties props = new Properties();

        try (FileInputStream fis = new FileInputStream(propFile)) {
            props.load(fis);
        }

        props.setProperty("java.port", String.valueOf(actualPort));

        try (FileOutputStream fos = new FileOutputStream(propFile)) {
            props.store(fos, "Updated by FDA Java Server");
        }
    }

    public static String getXmlPath() {
        return xmlPath;
    }

    public static int getPort() {
        return port;
    }
}