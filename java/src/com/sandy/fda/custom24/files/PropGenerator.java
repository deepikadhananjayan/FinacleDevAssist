package com.sandy.fda.custom24.files;

import java.util.List;
import java.util.Map;

import com.sandy.fda.custom24.IFileGenerator;
import com.sandy.fda.custom24.TemplateService;
import com.sandy.fda.models.custom24.Field;
import com.sandy.fda.models.custom24.Menu;
import com.sandy.fda.utils.FDALogger;

public class PropGenerator implements IFileGenerator {

    private static final String ENABLED_TEMPLATE = "{{fieldId}}_ENABLED:\"{{disabled}}\"";

    private static final String MANDATORY_TEMPLATE = "{{fieldId}}_MANDATORY:\"{{mandatoryFlag}}\"";

    private final TemplateService templateService;

    public PropGenerator(TemplateService templateService) {
        this.templateService = templateService;
    }

    @Override
    public String generate(Menu menuDetails) {
        FDALogger.info("Generating PROP File");

        List<Field> fields = menuDetails.fields();
        List<String> buttons = templateService.getButtons(menuDetails.menuType());
        String propsContent = buildPropsContent(fields, buttons);
        
        Map<String, String> values = Map.of(
                "c24", menuDetails.menuName().toLowerCase(),
                "c24PropFields", propsContent);
        
        try {
            propsContent = templateService.render(
                    "props\\props-content.tpl",
                    values,
                    true);
            templateService.beautify(propsContent, "JS");
        } catch (Exception e) {
            e.printStackTrace();
            return "FAILURE";
        }

        System.out.println(propsContent);
        // Write to File
        return "SUCCESS";
    }

    private String buildPropsContent(List<Field> fields, List<String> buttons) {

        StringBuilder propsContent = new StringBuilder();

        for (String button : buttons) {

            String disabledContent = "\t" +
                    ENABLED_TEMPLATE
                            .replace("{{fieldId}}", button)
                            .replace("{{disabled}}", "enabled")
                    +
                    ",";

            String mandatoryContent = "\t" +
                    MANDATORY_TEMPLATE
                            .replace("{{fieldId}}", button)
                            .replace("{{mandatoryFlag}}", "Y")
                    +
                    ",";

            propsContent
                    .append(disabledContent)
                    .append(System.lineSeparator())
                    .append(mandatoryContent)
                    .append(System.lineSeparator());
        }

        for (int i = 0; i < fields.size(); i++) {

            Field field = fields.get(i);

            String disabled = field.disabled() ? "disabled" : "enabled";

            String mandatory = field.mandatory() ? "Y" : "N";

            String disabledContent = "\t" +
                    ENABLED_TEMPLATE
                            .replace("{{fieldId}}", field.id())
                            .replace("{{disabled}}", disabled)
                    +
                    ",";

            String mandatoryContent = "\t" +
                    MANDATORY_TEMPLATE
                            .replace("{{fieldId}}", field.id())
                            .replace("{{mandatoryFlag}}", mandatory)
                    +
                    (i < fields.size() - 1 ? "," : "");

            propsContent
                    .append(disabledContent)
                    .append(System.lineSeparator())
                    .append(mandatoryContent)
                    .append(System.lineSeparator());
        }

        return propsContent.toString();
    }
}