package com.sandy.fda.parser;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import com.sandy.fda.models.Token;
import com.sandy.fda.models.Token.Builder;
import com.sandy.fda.models.enums.TokenType;
import com.sandy.fda.utils.FDAConstants;

public class TokenParser {
    private List<Token> allTokens;

    private Map<String, Token> allKeywords;
    private Map<String, Token> allUserhooks;
    private Map<String, Token> allFunctions;

    final Set<String> COMPARISON_OPERATORS = Collections.unmodifiableSet(
            new HashSet<>(Arrays.asList(
                    "==",
                    "!=",
                    ">",
                    "<",
                    ">=",
                    "<=")));

    public TokenParser() {
        this.allKeywords = new HashMap<>();
        this.allUserhooks = new HashMap<>();
        this.allFunctions = new HashMap<>();
    }

    public JsonObject getKeywordsAndUserhooks() throws Exception {
        if (allTokens == null) {
            allTokens = parse(FDAConstants.getXmlPath());
        }
        return convertToJsonResponse();
    }

    public Map<String, Token> getKeywordMap() throws Exception {
        if (allTokens == null) {
            allTokens = parse(FDAConstants.getXmlPath());
        }
        return allKeywords;
    }

    public Map<String, Token> getFunctionMap() throws Exception {
        if (allTokens == null) {
            allTokens = parse(FDAConstants.getXmlPath());
        }
        return allFunctions;
    }

    public Map<String, Token> getUserhookMap() throws Exception {
        if (allTokens == null) {
            allTokens = parse(FDAConstants.getXmlPath());
        }
        return allUserhooks;
    }

    private List<Token> parse(String xmlPath) throws Exception {

        List<Token> tokens = new ArrayList<>();

        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        DocumentBuilder builder = factory.newDocumentBuilder();
        Document doc = builder.parse(new File(xmlPath));
        doc.getDocumentElement().normalize();

        NodeList keywordNodes = doc.getElementsByTagName("Keyword");

        for (int i = 0; i < keywordNodes.getLength(); i++) {

            String keyword = keywordNodes.item(i).getTextContent().trim();

            if (!keyword.isEmpty()) {

                if (keyword.equals("START") || keyword.equals("END")) {
                    keyword = keyword.equals("START") ? "<--START" : "END-->";
                }

                if (keyword.equals("IF")) {
                    keyword = "IF() THEN";
                }

                if (keyword.equals("WHILE")) {
                    keyword = "WHILE()";
                }

                Builder tokenBuilder = new Token.Builder();
                tokenBuilder.setToken(keyword).setType(TokenType.KEYWORD);

                tokens.add(tokenBuilder.build());
            }
        }

        NodeList functionNodes = doc.getElementsByTagName("Function");

        for (int i = 0; i < functionNodes.getLength(); i++) {

            Node functionNode = functionNodes.item(i);

            Element functionElement = (Element) functionNode;
            String name = functionElement.getAttribute("name");

            Builder tokenBuilder = new Token.Builder();
            tokenBuilder.setToken(name).setType(TokenType.FUNCTION);

            Element paramInputs = (Element) functionElement.getElementsByTagName("ParamInputs").item(0);
            tokenBuilder.setParamInputs(getListFromNodes(paramInputs));

            tokens.add(tokenBuilder.build());
        }

        NodeList userHookNodes = doc.getElementsByTagName("Hook");

        for (int i = 0; i < userHookNodes.getLength(); i++) {

            Node userHookNode = userHookNodes.item(i);

            Element hookElement = (Element) userHookNode;
            String name = hookElement.getAttribute("name");

            Builder tokenBuilder = new Token.Builder();
            tokenBuilder.setToken(name).setType(TokenType.FUNCTION);

            Element paramInputs = (Element) hookElement.getElementsByTagName("ParamInputs").item(0);
            Element inputs = (Element) hookElement.getElementsByTagName("Inputs").item(0);
            Element outputs = (Element) hookElement.getElementsByTagName("Outputs").item(0);

            tokenBuilder.setParamInputs(getListFromNodes(paramInputs));
            tokenBuilder.setInputs(getListFromNodes(inputs));
            tokenBuilder.setOutputs(getListFromNodes(outputs));
            tokenBuilder.setType(TokenType.USERHOOK);

            tokens.add(tokenBuilder.build());
        }

        categorizeTokens(tokens);

        return tokens;
    }

    private void categorizeTokens(List<Token> allTokens) {

        for (Token token : allTokens) {
            if (token.getType() == TokenType.KEYWORD)
                allKeywords.put(token.getToken(), token);
            else if (token.getType() == TokenType.FUNCTION)
                allFunctions.put(token.getToken(), token);
            else
                allUserhooks.put(token.getToken(), token);
        }
    }

    private List<String> getListFromNodes(Node node) {

        List<String> nodesValue = new ArrayList<>();
        NodeList children = node.getChildNodes();

        for (int i = 0; i < children.getLength(); i++) {

            Node valueNode = children.item(i);

            if (valueNode.getNodeType() == Node.ELEMENT_NODE) {
                nodesValue.add(valueNode.getTextContent().trim());
            }
        }

        return nodesValue;
    }

    private JsonObject convertToJsonResponse() {

        JsonObject response = new JsonObject();

        JsonArray keywords = new JsonArray();
        for (Token token : allKeywords.values()) {
            keywords.add(token.getToken());
        }

        JsonArray functions = new JsonArray();
        for (Token token : allFunctions.values()) {
            functions.add(token.getToken());
        }

        JsonArray userhooks = new JsonArray();
        for (Token token : allUserhooks.values()) {
            userhooks.add(token.getToken());
        }

        response.addProperty("STATUS", "SUCCESS");

        response.add("keywords", keywords);
        response.add("functions", functions);
        response.add("userhooks", userhooks);

        return response;
    }
}
