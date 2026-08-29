package com.sandy.fda.custom24.files;

import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import com.sandy.fda.custom24.IFileGenerator;
import com.sandy.fda.custom24.TemplateService;
import com.sandy.fda.models.custom24.Field;
import com.sandy.fda.models.custom24.Menu;
import com.sandy.fda.models.custom24.Option;
import com.sandy.fda.models.custom24.enums.PageType;
import com.sandy.fda.utils.FDALogger;

public class InfengGenerator implements IFileGenerator {

    private final String CRITERIA_PAGE_TEMPLATE = "infeng\\infeng-criteria-content.tpl";
    private final String DETAIL_PAGE_TEMPLATE = "infeng\\infeng-detail-content.tpl";
    private final String RESULT_PAGE_TEMPLATE = "infeng\\infeng-result-content.tpl";
    private final TemplateService templateService;

    public InfengGenerator(TemplateService templateService) {
        this.templateService = templateService;
    }

    @Override
    public String generate(Menu menuDetails) {
        FDALogger.info("Generating INFENG File");

        List<Field> fields = menuDetails.fields();
        String menuDesc = menuDetails.menuDescription();

        Map<String, String> fltCodes = templateService.getFltCodes(fields);

        String critInfengContent = critInfengContent(fields, menuDesc, fltCodes);

        String detInfengContent = detInfengContent(fields, menuDesc, fltCodes);

        String resInfengContent = resInfengContent(menuDesc);

        System.out.println("CRITERIA");
        System.out.println(critInfengContent);

        System.out.println("DETAIL");
        System.out.println(detInfengContent);

        System.out.println("RESULT");
        System.out.println(resInfengContent);

        return "SUCCESS";
    }

    private String buildFltContent(List<Field> fields, PageType pageType, Map<String, String> fltCodes) {
        StringBuilder content = new StringBuilder();

        for (Field field : fields) {
            if (pageType != null && field.pageType() != pageType) {
                continue;
            }

            content.append(fltCodes.get(field.id()))
                    .append(" : \"")
                    .append(field.label())
                    .append("\",")
                    .append(System.lineSeparator());

            for (Option option : field.options()) {
                content.append(
                        fltCodes.get(field.id() + "-" + option.label()))
                        .append(" : \"")
                        .append(option.label())
                        .append("\",")
                        .append(System.lineSeparator());
            }
        }

        return content.toString();
    }

    public String critInfengContent(List<Field> fields, String menuDesc, Map<String, String> fltCodes) {
        Map<String, String> values = Map.of(
                "c24InfengTitle", menuDesc,
                "c24FLT", buildFltContent(
                        fields,
                        PageType.CRITERIA,
                        fltCodes));

        return renderInfeng(
                CRITERIA_PAGE_TEMPLATE,
                values);
    }

    public String detInfengContent(List<Field> fields, String menuDesc, Map<String, String> fltCodes) {
        Map<String, String> values = Map.of(
                "c24InfengTitle", menuDesc,
                "c24FLT", buildFltContent(
                        fields,
                        null,
                        fltCodes));

        return renderInfeng(
                DETAIL_PAGE_TEMPLATE,
                values);
    }

    public String resInfengContent(String menuDesc) {
        Map<String, String> values = Map.of(
                "c24InfengTitle", menuDesc);

        return renderInfeng(
                RESULT_PAGE_TEMPLATE,
                values);
    }

    private String renderInfeng(String template, Map<String, String> values) {
        try {
            String content = templateService.render(
                    template,
                    values,
                    true);

            return templateService.beautify(content, "JS");

        } catch (Exception e) {
            e.printStackTrace();
            return "FAILURE";
        }
    }
}