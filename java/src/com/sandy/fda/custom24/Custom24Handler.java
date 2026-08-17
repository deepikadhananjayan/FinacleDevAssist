package com.sandy.fda.custom24;

import java.io.File;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.google.gson.Gson;
import com.google.gson.JsonObject;
import com.sandy.fda.models.custom24.C24Environment;
import com.sandy.fda.models.custom24.Menu;
import com.sandy.fda.models.custom24.enums.FileType;
import com.sandy.fda.utils.FDAConstants;

public class Custom24Handler {

    private FileFactory fileFactory;
    private SFTPHandler sftpHandler;

    public Custom24Handler() {
        this.fileFactory = new FileFactory();
        this.sftpHandler = new SFTPHandler();
    }

    public JsonObject generateCustomMenu(JsonObject req) {
        Menu menuDetails = new Gson().fromJson(req, Menu.class);

        JsonObject response = new JsonObject();
        response.addProperty("STATUS", "SUCCESS");

        for (FileType type : menuDetails.generateDesign()) {
            IFileGenerator fileGenerator = fileFactory.getFileGenerator(type);
            String typeStatus = fileGenerator.generate(menuDetails);
            response.addProperty(type.toString(), typeStatus);
        }
        return response;
    }

    public JsonObject deployCustomMenu(JsonObject req) throws Exception {
        JsonObject response = new JsonObject();
        File parentPath = new File(req.get("filesPath").getAsString());
        String environment = req.get("environment").getAsString();

        C24Environment c24Env = getEnvDetails(environment, parentPath);
        Map<String, List<File>> filesToBeDeployed = new HashMap<>();

        for (File file : parentPath.listFiles()) {
            if (file.isDirectory()) {
                switch (file.getName()) {
                    case "XML" -> filesToBeDeployed.put(c24Env.xmlPath(), Arrays.asList(file.listFiles()));
                    case "PROPS" -> filesToBeDeployed.put(c24Env.propsPath(), Arrays.asList(file.listFiles()));
                    case "INFENG" -> filesToBeDeployed.put(c24Env.infengPath(), Arrays.asList(file.listFiles()));
                    case "INC_GINC" -> filesToBeDeployed.put(c24Env.incGincPath(), Arrays.asList(file.listFiles()));
                    case "LINK_GLINK" -> filesToBeDeployed.put(c24Env.linkGlinkPath(), Arrays.asList(file.listFiles()));
                    case "HELP" -> filesToBeDeployed.put(c24Env.helpPath(), Arrays.asList(file.listFiles()));
                    case "SQL" -> filesToBeDeployed.put(c24Env.sqlPath(), Arrays.asList(file.listFiles()));
                }
            }
        }

        sftpHandler.transferFiles(c24Env, filesToBeDeployed);
        response.addProperty("STATUS", "SUCCESS");
        return response;
    }

    private C24Environment getEnvDetails(String environment, File parentPath) {
        Map<String, String> props = FDAConstants.getProperties();
        String envPrefix = environment + ".";

        String host = props.get(envPrefix + "host");
        int port = Integer.parseInt(props.get(envPrefix + "port"));
        String username = props.get(envPrefix + "username");
        String password = props.get(envPrefix + "password");
        String bankId = props.get(envPrefix + "bankId");

        char sep = '/';
        String fePath = ensureTrailingSeparator(props.get(envPrefix + "fePath"), sep);
        String bePath = ensureTrailingSeparator(props.get(envPrefix + "bePath"), sep);

        String normalizedBePath = bePath + "cust" + sep;
        String helpPath = fePath + "helpfiles" + sep;
        String customFePath = fePath + "custom" + sep;

        String xmlPath = customFePath + "GroupXML" + sep;
        String propsPath = customFePath + "props" + sep;
        String incGincPath = customFePath + parentPath.getName() + sep;

        String jsPath = customFePath + "javascripts" + sep;
        String linkGlinkPath = jsPath + parentPath.getName() + sep;
        String infengPath = jsPath + "jspjs" + sep + "INFENG" + sep;

        String sqlPath = normalizedBePath + bankId + sep + "INFENG" + sep + "sql" + sep;
        String scriptPath = normalizedBePath + bankId + sep + "INFENG" + sep + "scripts" + sep;

        return new C24Environment(
                environment,
                host,
                port,
                username,
                password,
                bankId,
                bePath,
                fePath,
                xmlPath,
                propsPath,
                incGincPath,
                jsPath,
                linkGlinkPath,
                infengPath,
                helpPath,
                sqlPath,
                scriptPath);
    }

    private String ensureTrailingSeparator(String path, char separator) {
        if (path == null || path.isEmpty()) {
            return path;
        }
        return path.endsWith(String.valueOf(separator)) ? path : path + separator;
    }
}
