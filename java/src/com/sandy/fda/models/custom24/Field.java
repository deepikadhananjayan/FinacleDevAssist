package com.sandy.fda.models.custom24;

import java.util.List;

import com.sandy.fda.models.custom24.enums.FieldType;
import com.sandy.fda.models.custom24.enums.PageType;
import com.sandy.fda.models.custom24.enums.SearcherType;

public record Field(
        String id,
        String label,
        String description,
        FieldType type,
        PageType pageType,
        SearcherType searcher,
        boolean disabled,
        boolean mandatory,
        List<Option> options
) {
}