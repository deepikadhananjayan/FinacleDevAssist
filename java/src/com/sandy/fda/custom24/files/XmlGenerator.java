package com.sandy.fda.custom24.files;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.sandy.fda.custom24.IFileGenerator;
import com.sandy.fda.custom24.TemplateService;
import com.sandy.fda.models.custom24.Field;
import com.sandy.fda.models.custom24.Menu;
import com.sandy.fda.models.custom24.Option;
import com.sandy.fda.models.custom24.enums.MenuType;
import com.sandy.fda.models.custom24.enums.PageType;
import com.sandy.fda.utils.FDALogger;

public class XmlGenerator implements IFileGenerator {

    private final String XML_TEMPLATE = "xml\\xml-content.tpl";
    private final String XML_FIELD_TEMPLATE = "xml\\xml-fieldlist-content.tpl";
    private final String XML_MULTIREC_TEMPLATE = "xml\\xml-multirec-content.tpl";
    private final String XML_INVOCATION_TEMPLATE = "xml\\xml-invocationlist-content.tpl";
    private final TemplateService templateService;

    public XmlGenerator(TemplateService templateService) {
        this.templateService = templateService;
    }

    @Override
    public String generate(Menu menuDetails) {
        FDALogger.info("Generating XML File");

        String xmlContent = new String();
        List<String> invocationButtons = templateService.getInvocationButtons(menuDetails.menuType());
        String critJspName = (menuDetails.menuType() == MenuType.UPLOAD || menuDetails.menuType() == MenuType.TWO_PAGE)
                ? "cust_dummy_InitialPage.jsp"
                : menuDetails.menuName().toLowerCase() + "_crit.jsp";
        List<Field> fields = menuDetails.fields();
        MenuType menuType = menuDetails.menuType();

        String xmlFieldContent = buildXmlFields(fields, menuType);
        String xmlInvocationContent = buildXmlInvocationDetails(fields, invocationButtons, menuDetails.menuName());
        String xmlMultiRecContent = buildXmlMultiRec(fields, menuDetails.menuName(), menuType);
        String xmlMultiTabContent = "";

        Map<String, String> values = Map.of(
                "c24", menuDetails.menuName().toLowerCase(),
                "c24CritJspName", critJspName,
                "c24FieldList", xmlFieldContent,
                "c24InvocationList", xmlInvocationContent,
                "c24MultiRecList", xmlMultiRecContent,
                "c24MultiTabList", xmlMultiTabContent);
        try {
            xmlContent = templateService.render(
                    XML_TEMPLATE,
                    values,
                    true);
            xmlContent = templateService.beautify(xmlContent, "XML");
        } catch (Exception e) {
            e.printStackTrace();
            return "FAILURE";
        }

        System.out.println(xmlContent);
        return "SUCCESS";
    }

    public String buildXmlFields(List<Field> fields, MenuType menuType) {
        StringBuilder xmlFieldsContent = new StringBuilder();
        Map<String, String> values = new HashMap<>();
        String multiRecField = (menuType == MenuType.UPLOAD || menuType == MenuType.TWO_PAGE
                || menuType == MenuType.THREE_PAGE) ? "N" : "Y";

        try {
            String template = templateService.load(XML_FIELD_TEMPLATE);
            for (Field field : fields) {
                values = Map.of(
                        "c24FieldId", field.id(),
                        "c24MRField", multiRecField);
                xmlFieldsContent
                        .append(templateService.render(
                                template,
                                values,
                                false));
            }
        } catch (Exception e) {
            e.printStackTrace();
            return "FAILURE";
        }
        return xmlFieldsContent.toString();
    }

    private String buildXmlMultiRec(List<Field> fields, String menuName, MenuType menuType) {
        StringBuilder xmlMultiRecContent = new StringBuilder();
        if (menuType == MenuType.MRH_TYPE1
                || menuType == MenuType.MRH_TYPE2
                || menuType == MenuType.MRH_TYPE3) {

            String mrhType = menuType.name().replace("_TYPE", "");

            long mrhFieldCnt = fields.stream()
                    .filter(field -> field.pageType() == PageType.DETAIL)
                    .count();

            Map<String, String> values = Map.of(
                    "c24", menuName,
                    "c24MRHType", mrhType,
                    "c24MRHColumns", String.valueOf(mrhFieldCnt));
            try {
                xmlMultiRecContent
                        .append(templateService.render(
                                XML_MULTIREC_TEMPLATE,
                                values,
                                true));
            } catch (Exception e) {
                e.printStackTrace();
                return "FAILURE";
            }
        } else {
            xmlMultiRecContent.append("");
        }
        return xmlMultiRecContent.toString();
    }

    private String buildXmlInvocationDetails(List<Field> fields, List<String> buttons, String menuName) {

        StringBuilder xmlInvocationContent = new StringBuilder();

        try {
            String template = templateService.load(XML_INVOCATION_TEMPLATE);

            Field funcCodeField = fields.stream()
                    .filter(field -> "funcCode".equals(field.id()))
                    .findFirst()
                    .orElse(null);

            for (String button : buttons) {
                switch (button) {
                    case "GETDATA" -> {
                        if (funcCodeField == null) {
                            continue;
                        }

                        for (Option option : funcCodeField.options()) {
                            Map<String, String> values = Map.of(
                                    "c24", menuName,
                                    "c24InvocationButton", button,
                                    "c24FuncCode", option.value(),
                                    "c24InvocationButtonLower", "GO");

                            xmlInvocationContent.append(
                                    templateService.render(
                                            template,
                                            values,
                                            false));
                        }
                    }

                    case "UPLOAD", "SUBMIT", "VALIDATE" -> {

                        Map<String, String> values = Map.of(
                                "c24", menuName,
                                "c24InvocationButton", button,
                                "c24FuncCode", "NA",
                                "c24InvocationButtonLower", button.toLowerCase());

                        xmlInvocationContent.append(
                                templateService.render(
                                        template,
                                        values,
                                        false));
                    }
                }
            }

        } catch (Exception e) {
            e.printStackTrace();
            return "FAILURE";
        }

        return xmlInvocationContent.toString();
    }
}