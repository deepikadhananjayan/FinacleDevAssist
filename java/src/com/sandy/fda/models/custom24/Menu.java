package com.sandy.fda.models.custom24;

import java.util.List;

import com.sandy.fda.models.custom24.enums.FileType;
import com.sandy.fda.models.custom24.enums.MenuType;

public class Menu {
    private MenuType menuType;
    private String menuName;
    private String menuDescription;
    List<FileType> generateDesign;

    public MenuType getMenuType() {
        return menuType;
    }

    public String getMenuName() {
        return menuName;
    }

    public String getMenuDescription() {
        return menuDescription;
    }

    public List<FileType> getGenerateDesign() {
        return generateDesign;
    }
}
