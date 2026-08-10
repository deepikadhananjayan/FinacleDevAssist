package com.sandy.fda.models.beautifier;

import com.sandy.fda.models.beautifier.enums.ContentType;

public class BeautifyData {
    private ContentType contentType;
    private String content;

    public BeautifyData(Builder builder) {
        this.contentType = builder.contentType;
        this.content = builder.content;
    }

    public ContentType getContentType() {
        return contentType;
    }

    public String getContent() {
        return content;
    }

    public static class Builder {
        private ContentType contentType;
        private String content;

        public Builder setContentType(ContentType contentType) {
            this.contentType = contentType;
            return this;
        }

        public Builder setContent(String content) throws Exception {
            this.content = content;
            return this;
        }

        public BeautifyData build() {
            if (this.contentType == null) {
                throw new IllegalStateException("Content Type missing");
            }

            return new BeautifyData(this);
        }
    }
}
