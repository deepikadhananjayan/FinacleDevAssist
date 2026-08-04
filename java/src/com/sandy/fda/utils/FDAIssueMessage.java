package com.sandy.fda.utils;

public class FDAIssueMessage {

    public static final String START_NOT_FOUND = "<--START keyword is missing in script";
    public static final String MULTIPLE_START_FOUND = "<--START keyword appears multiple times in script";

    public static final String END_NOT_FOUND = "END--> keyword is missing in script";
    public static final String MULTIPLE_END_FOUND = "END--> keyword appears multiple times in script";

    public static final String TRACE_ON_NOT_FOUND = "TRACE ON keyword is missing in script";
    public static final String MULTIPLE_TRACE_ON_FOUND = "TRACE ON keyword appears multiple times in script";

    public static final String TRACE_OFF_NOT_FOUND = "TRACE OFF keyword is missing in script";
    public static final String MULTIPLE_TRACE_OFF_FOUND = "TRACE OFF keyword appears multiple times in script";

    public static final String EXITSCRIPT_NOT_FOUND = "EXITSCRIPT keyword is missing in script";
    public static final String MULTIPLE_EXITSCRIPT_FOUND = "EXITSCRIPT keyword appears multiple times in script";

    public static final String START_FOUND_AFTER_END = "<--START keyword appears after END--> in script";
    public static final String TRACE_ON_FOUND_AFTER_TRACE_OFF = "TRACE ON keyword appears after TRACE OFF in script";

    public static final String EXPECTED_ENDIF_FOR_IF = "Expected ENDIF for IF statement in line ";

    public static final String UNEXPECTED_ELSE = "Unexpected ELSE statement found in line ";

    public static final String EXPECTED_ENDIF_FOR_ELSE = "Expected ENDIF for ELSE statement in line ";

    public static final String UNEXPECTED_ENDIF = "Unexpected ENDIF found in line ";

    public static final String EXPECTED_DO_FOR_WHILE = "Expected DO for WHILE statement in line ";

    public static final String UNEXPECTED_DO = "Unexpected DO statement found in line ";

    public static final String EXPECTED_ENDFUNCTION = "Expected ENDFUNCTION for FUNCTION statement in line ";

    public static final String UNEXPECTED_ENDFUNCTION = "Unexpected ENDFUNCTION found in line ";

    public static final String THEN_MISSING_IN_IF = "THEN keyword is missing for IF statement in line ";

    public static final String UNEXPECTED_TOKEN_IN_LINE = "Unexpected token found in line ";

    public static final String JUMP_TO_UNDECLARED_LABEL = "Label is not declared in the script. Referenced in line ";

    public static final String GOTO_LABEL_DECLARED_ABOVE = "Label is declared above in the script, so GOTO cannot be used. Referenced in line ";

    public static final String UNCLOSED_STRING_OR_CHAR_LITERAL_IN_LINE = "Unclosed string/character literal found in line ";

    public static final String INVALID_OPERATOR_SEQUENCE_IN_LINE = "Invalid operator sequence found in line ";

    public static final String INVALID_COMPARISON_OPERATOR_IN_LINE = "Invalid comparison operator found in line ";

    public static final String UNKNOWN_FUNCTION_CALLED = "Unknown function called in line ";

    public static final String EMPTY_PARAMETERS_FOR_FUNCTION = "Function called without parameters in line ";

    public static final String PARAMETER_COUNT_MISMATCH_FOR_FUNCTION = "Parameter count mismatch for function ";

    public static final String CLOSE_BRACKET_NOT_FOUND_IN_CONDITION = "Missing closing bracket in condition for ";

    public static final String INVALID_AND_OR_OPERATOR_IN_CONDITION = "Invalid placement of AND/OR operator in condition for ";

    public static final String INVALID_ARITHMETIC_COMPARISON_OPERATOR_IN_CONDITION = "Invalid placement of arithmetic/comparison operator in condition for ";

    public static final String UNEXPECTED_COMMA_IN_CONDITION = "Invalid comma placement in condition for ";

    public static final String MISSING_CLOSING_BRACKET_IN_FUNCTION = "Missing ')' after function ";

    public static final String MISSING_OPENING_BRACKET_IN_FUNCTION = "Missing '(' after function ";

    public static final String EXTRA_BRACKETS_FOUND_IN_CONDITION = "Extra brackets found in condition for ";

    public static final String EMPTY_EXPRESSION_IN_CONDITION = "Empty expression found in condition for ";

    public static final String UNEXPECTED_ARITHMETIC_OPERATOR_IN_EXPRESSION = "Unexpected arithmetic operator in condition for ";

    public static final String UNARY_MINUS_ALLOWED_FOR_NUMBER_LITERAL = "Unary '-' is allowed only before a numeric literal in ";

    public static final String INVALID_EXPRESSION_IN_CONDITION = "Invalid expression in condition for ";

    public static final String INVALID_IDENTIFIER_FOUND_IN_CONDITION = "Invalid identifier in condition for ";

    public static final String EXPECTED_OPERATOR_IN_EXPRESSION = "Expected an operator in condition for ";

    public static final String EXPECTED_VALUE_IN_EXPRESSION = "Expected a value in condition for ";

    public static final String USERHOOK_NOT_REGISTERED = "Unknown userhook used in line ";

    public static final String LIBNAME_NOT_REGISTERED = "Unknown libname used in line ";

    public static final String LIBNAME_UNDEFINED = "Library name not defined in line ";

    public static final String IMPORT_UNDEFINED = "Import file not defined in line ";
}
