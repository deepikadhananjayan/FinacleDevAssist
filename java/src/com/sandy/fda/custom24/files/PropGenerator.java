package com.sandy.fda.custom24.files;

import java.util.List;
import java.util.Map;

import com.sandy.fda.custom24.IFileGenerator;
import com.sandy.fda.custom24.TemplateService;
import com.sandy.fda.models.custom24.Field;
import com.sandy.fda.models.custom24.Menu;
import com.sandy.fda.utils.FDALogger;

public class PropGenerator implements IFileGenerator {

    private final String ENABLED_TEMPLATE = "{{fieldId}}_ENABLED:\"{{disabled}}\"";
    private final String MANDATORY_TEMPLATE = "{{fieldId}}_MANDATORY:\"{{mandatoryFlag}}\"";

    private final String PROPS_TEMPLATE = "props\\props-content.tpl";
    private final TemplateService templateService;

    public PropGenerator(TemplateService templateService) {
        this.templateService = templateService;
    }

    @Override
    public String generate(Menu menuDetails) {
        FDALogger.info("Generating PROP File");

        String propsContent = new String();
        List<Field> fields = menuDetails.fields();
        List<String> buttons = templateService.getButtons(menuDetails.menuType());
        String propFields = buildPropsContent(fields, buttons);

        Map<String, String> values = Map.of(
                "c24", menuDetails.menuName().toLowerCase(),
                "c24PropFields", propFields);

        try {
            propsContent = templateService.render(
                    PROPS_TEMPLATE,
                    values,
                    true);
            propsContent = templateService.beautify(propsContent, "JS");
        } catch (Exception e) {
            e.printStackTrace();
            return "FAILURE";
        }

        System.out.println(propsContent);
        return "SUCCESS";
    }

    private String buildPropsContent(List<Field> fields, List<String> buttons) {

        StringBuilder propsContent = new StringBuilder();

        for (String button : buttons) {
            propsContent
                    .append(ENABLED_TEMPLATE
                            .replace("{{fieldId}}", button)
                            .replace("{{disabled}}", "enabled"))
                    .append(",")
                    .append(System.lineSeparator())

                    .append(MANDATORY_TEMPLATE
                            .replace("{{fieldId}}", button)
                            .replace("{{mandatoryFlag}}", "Y"))
                    .append(",")
                    .append(System.lineSeparator());
        }

        for (Field field : fields) {
            String disabled = field.disabled() ? "disabled" : "enabled";
            String mandatory = field.mandatory() ? "Y" : "N";

            propsContent
                    .append(ENABLED_TEMPLATE
                            .replace("{{fieldId}}", field.id())
                            .replace("{{disabled}}", disabled))
                    .append(",")
                    .append(System.lineSeparator())

                    .append(MANDATORY_TEMPLATE
                            .replace("{{fieldId}}", field.id())
                            .replace("{{mandatoryFlag}}", mandatory))
                    .append(System.lineSeparator());
        }

        return propsContent.toString();
    }
}