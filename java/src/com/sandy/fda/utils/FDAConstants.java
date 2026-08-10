package com.sandy.fda.utils;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.util.HashMap;
import java.util.Properties;

public class FDAConstants {

    private static HashMap<String, String> properties;

    private static Properties props;

    private static boolean loaded = false;

    private FDAConstants() {
    }

    static {
        props = new Properties();
        properties = new HashMap<>();
    }

    public static boolean load() {

        if (loaded) {
            return true;
        }

        try {
            String jarPath = FDAConstants.class
                    .getProtectionDomain()
                    .getCodeSource()
                    .getLocation()
                    .toURI()
                    .getPath();

            File appDirectory = new File(jarPath).getParentFile();

            //File appDirectory = new File("D:\\Santhosh\\Personal Learning\\Finacle Validator\\FDA\\java\\");

            File propFile = new File(appDirectory, "fdaplugin.properties");

            if (!propFile.exists()) {
                FDALogger.error(
                        new Exception(
                                "fdaplugin.properties not found : "
                                        + propFile.getAbsolutePath()));

                return false;
            }

            try (FileInputStream fis = new FileInputStream(propFile)) {
                props.load(fis);
            }
            
            properties.put("PROP_FILE_LOCATION", propFile.getAbsolutePath());

            File tokenXml = new File(appDirectory, "tokens.xml");

            if (!tokenXml.exists()) {
                throw new FileNotFoundException(
                        "tokens.xml file is missing");
            }

            String xmlPath = tokenXml.getAbsolutePath();

            properties.put("TOKEN_XML_PATH", xmlPath);

            for (String key : props.stringPropertyNames()) {
                String value = props.getProperty(key);
                properties.put(key, value);
            }

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
        props.setProperty("java.port", String.valueOf(actualPort));
    }

    public static HashMap<String, String> getProperties() {
        return properties;
    }
}