package com.sandy.fda.custom24;

import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import com.sandy.fda.models.custom24.enums.MenuType;

public class TemplateService {

    public String render(String template, Map<String, String> values) throws Exception {
        String result = load(template);
        for (Map.Entry<String, String> entry : values.entrySet()) {
            result = result.replace(
                    "{{" + entry.getKey() + "}}",
                    entry.getValue());
        }
        return result;
    }

    private String load(String template) throws IOException {

        // try (InputStream inputStream = getClass().getClassLoader()
        // .getResourceAsStream("templates/" + template)) {

        // if (inputStream == null) {
        // throw new IOException(
        // "Template not found: " + template);
        // }

        // return new String(
        // inputStream.readAllBytes(),
        // StandardCharsets.UTF_8);
        // }

        return Files.readString(
                Path.of("D:\\Santhosh\\Personal Learning\\Finacle Validator\\FDA\\java\\src\\resources\\templates\\"
                        + template));
    }

    public List<String> getButtons(MenuType menuType) {
        return switch (menuType) {
            case TWO_PAGE ->
                Arrays.asList("Submit", "Validate", "Ok", "Cancel");

            case THREE_PAGE ->
                Arrays.asList(
                        "Go",
                        "Clear",
                        "Submit",
                        "Validate",
                        "Ok",
                        "Cancel");

            default -> Collections.emptyList();
        };
    }
}
