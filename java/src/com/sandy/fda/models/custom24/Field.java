package com.sandy.fda.models.custom24;

import java.util.List;

import com.sandy.fda.models.custom24.enums.FieldType;
import com.sandy.fda.models.custom24.enums.PageType;
import com.sandy.fda.models.custom24.enums.SearcherType;

public class Field {
    private String id;
    private String label;
    private String description;
    private FieldType type;
    private PageType pageType;
    private SearcherType searcher;
    private boolean disabled;
    private boolean mandatory;
    private List<Option> options;

    public String getId() {
        return id;
    }

    public String getLabel() {
        return label;
    }

    public String getDescription() {
        return description;
    }

    public FieldType getType() {
        return type;
    }

    public PageType getPageType() {
        return pageType;
    }

    public SearcherType getSearcher() {
        return searcher;
    }

    public boolean isDisabled() {
        return disabled;
    }

    public boolean isMandatory() {
        return mandatory;
    }

    public List<Option> getOptions() {
        return options;
    }
}
