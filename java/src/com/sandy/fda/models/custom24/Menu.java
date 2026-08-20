package com.sandy.fda.models.custom24;

import java.util.List;

import com.sandy.fda.models.custom24.enums.FileType;
import com.sandy.fda.models.custom24.enums.MenuType;

public record Menu(
        String menuName,
        String menuDescription,
        MenuType menuType,
        List<Field> fields,
        List<FileType> generateDesign
) {
}