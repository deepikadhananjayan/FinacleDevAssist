package com.sandy.fda.utils;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Properties;

public class FDAConstants {

    private static HashMap<String, String> properties;
    private static File appDirectory;
    private static File propFile;
    private static Properties props;
    private static boolean loaded = false;
    private static Long oldPropFileModified;

    private FDAConstants() {
    }

    static {
        props = new Properties();
        properties = new HashMap<>();
    }

    public static boolean load() {
        try {
            if (!loaded) {
                String userHome = System.getProperty("user.home");
                appDirectory = new File(userHome, "FDA");
                propFile = new File(appDirectory, "fdaplugin.properties");

                appDirectory = new File("D:\\Santhosh\\Personal Learning\\Finacle Validator\\FDA\\java\\");
                propFile = new File(appDirectory, "dev.fdaplugin.properties");

                properties.put("PROP_FILE_LOCATION", propFile.getAbsolutePath());

                // Verify token.xml exists in resources
                try (InputStream is = FDAConstants.class.getResourceAsStream("/resources/tokens.xml")) {
                    if (is == null) {
                        throw new FileNotFoundException("tokens.xml resource is missing inside JAR under /resources/");
                    }
                }
                loaded = true;
            }

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
            
            for (String key : props.stringPropertyNames()) {
                String value = props.getProperty(key);
                properties.put(key, value);
            }

            oldPropFileModified = propFile.lastModified();

            FDALogger.info(
                    "FDA Constants loaded successfully");

            return true;
        } catch (Exception e) {
            FDALogger.error(e);
            return false;
        }
    }

    public static Long getPropertiesFileModified() {
    return propFile.lastModified();
}

public static Long getOldPropFileModified() {
    return oldPropFileModified;
}

    public static void updatePortInProperties(int actualPort) throws Exception {
    props.setProperty("java.port", String.valueOf(actualPort));

    try (FileOutputStream fos = new FileOutputStream(propFile)) {
        props.store(fos, null);
    }
}

    public static HashMap<String, String> getProperties() {
        return properties;
    }
}