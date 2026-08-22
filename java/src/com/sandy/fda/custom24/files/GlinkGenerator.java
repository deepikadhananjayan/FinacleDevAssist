package com.sandy.fda.custom24.files;

import com.sandy.fda.custom24.IFileGenerator;
import com.sandy.fda.custom24.TemplateService;
import com.sandy.fda.models.custom24.Menu;
import com.sandy.fda.utils.FDALogger;

public class GlinkGenerator implements IFileGenerator {

    private final TemplateService templateService;

    public GlinkGenerator(TemplateService templateService) {
        this.templateService = templateService;
    }

    @Override
    public String generate(Menu menuDetails) {
        FDALogger.info("Generating GLINK File");
        // TODO Auto-generated method stub
        return null;
    }
}
