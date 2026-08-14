package com.sandy.fda.custom24;

import com.google.gson.Gson;
import com.google.gson.JsonObject;
import com.sandy.fda.models.custom24.Menu;
import com.sandy.fda.models.custom24.enums.FileType;

public class Custom24Handler {

    private FileFactory fileFactory;

    public Custom24Handler() {
        this.fileFactory = new FileFactory();
    }

    public JsonObject generateCustomMenu(JsonObject req) {
        Menu menuDetails = new Gson().fromJson(req, Menu.class);

        JsonObject response = new JsonObject();
        response.addProperty("STATUS", "SUCCESS");
        
        for (FileType type : menuDetails.getGenerateDesign()) {
            IFileGenerator fileGenerator = fileFactory.getFileGenerator(type);
            String typeStatus = fileGenerator.generate(menuDetails);
            response.addProperty(type.toString(), typeStatus);
        }
        return response;
    }
}
