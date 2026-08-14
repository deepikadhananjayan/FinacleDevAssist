package com.sandy.fda.models.custom24;

import java.util.List;

import com.sandy.fda.models.custom24.enums.FileType;
import com.sandy.fda.models.custom24.enums.MenuType;

public record Menu(
        MenuType menuType,
        String menuName,
        String menuDescription,
        List<FileType> generateDesign
) {
}