// Scanner.c
// Scanner implementation

#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"
#include "console.h"


typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;

Scanner scanner;

void initScanner(const char* source) {
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

static bool isAtEnd() {
    return *scanner.current == '\0';
}

static bool match(char expected) {
    if (isAtEnd()) return false;
    if (*scanner.current != expected) return false;

    scanner.current++;
    return true;
}

static bool isDigit(char c) {
    // debugPrint("checking if char is numeric");
    return c >= '0' && c <= '9';
}

static bool isAlpha(char c) {
    // debugPrint("Checking if char is alphabetical");
    return (c <= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

static char advance() {
    // debugPrint("Advancing character");
    scanner.current++;
    return scanner.current[-1];
}

static char peek() {
    return *scanner.current;
}

static char peekNext() {
    if (isAtEnd()) return '\0';
    return scanner.current[1]; // Returns the character current from next
    // equivalent to ++(scanner.current)
}

static void skipWhiteSpace() {
    debugPrint("Skipping whitepsace...");
    for (;;) {
        char c = peek();
        switch (c){
            case ' ': 
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                scanner.line++;
                advance();
                break;
            //Comments!
            case '/':
                if (peekNext() == '/') {
                    // A comment goes to the end of the line
                    while (peek() != '\n' && !isAtEnd()) advance();
                } else {
                    return;
                }
                break;
            
            default:
                return;
        }
    }
}

static TokenType checkKeyword(int start, int length, 
    const char* rest, TokenType type) {
        if (scanner.current - scanner.start == start + length &&
        memcmp(scanner.start + start, rest, length) == 0) {
            return type;
        }

        return TOKEN_IDENTIFIER;
}

static Token errorToken(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int) strlen(message);
    token.line = scanner.line;
    
    return token;
}

static Token makeToken(TokenType type) {
    // debugPrint("Making Token")
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int) (scanner.current - scanner.start);
    token.line = scanner.line;

    return token;
}

static Token string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() != '\n') scanner.line++;
    }

    if (isAtEnd()) return errorToken("Unterminated string.");

    //The closing quote
    advance();
    return makeToken(TOKEN_STRING);
}

static Token number() {
    while (isDigit(peek())) advance();

    // Fractional parts
    if (peek() == '.' && isDigit(peekNext())) {
        advance(); // consume '.'

        while (isDigit(peek())) advance();
    }
    
    return makeToken(TOKEN_NUMBER);
}

static TokenType identiferType() {


    switch (scanner.start[0]) {
        case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);
        case 'c': return checkKeyword(1, 4, "lass", TOKEN_CLASS);
        case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
        case 'f':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
                    case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
                    case 'u': return checkKeyword(2, 1, "n", TOKEN_FUN);
                }

                
            }

        case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
        case 'n': return checkKeyword(1, 2, "il", TOKEN_NIL);
        case 'o': return checkKeyword(1,1, "r", TOKEN_OR);
        case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
        case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
        case 's': return checkKeyword(1, 4, "uper", TOKEN_SUPER);
        case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
        case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    }

    debugPrint("Unable to detect reserved word, outputing Identifier Token");
    return TOKEN_IDENTIFIER;
}


static Token identifier() {
    // printDebug("");
    while (isAlpha(peek()) || isDigit(peek())) advance();

    return makeToken(identiferType());
}



Token scanToken() {
    debugPrint("Scanning Token...");
    
    skipWhiteSpace();

    scanner.start = scanner.current;

    if (isAtEnd()) { debugPrint("Making EOF Token"); return makeToken(TOKEN_EOF);}
    // Basic characters

    char c = advance(); 
    
    //printf("Scanning char %c", c);

    if (isAlpha(c)) {debugPrint("Attempting to generate identifier"); return identifier();}
    if (isDigit(c)) {debugPrint("Attempting to generate number"); return number();}

    switch(c) {
        case '(': {debugPrint("Outputing '(' Token"); return makeToken(TOKEN_LEFT_PAREN);}
        case ')': {debugPrint("Outputing ')' Token"); return makeToken(TOKEN_RIGHT_PAREN);}
        case '{': {debugPrint("Outputing '{' Token"); return makeToken(TOKEN_LEFT_BRACE);}
        case '}': {debugPrint("Outputing '}' Token"); return makeToken(TOKEN_RIGHT_BRACE);}
        case ';': {debugPrint("Outputing ';' Token"); return makeToken(TOKEN_SEMICOLON);}
        case ',': {debugPrint("Outputing ',' Token"); return makeToken(TOKEN_COMMA);}
        case '.': {debugPrint("Outputing '.' Token"); return makeToken(TOKEN_DOT);}
        case '+': {debugPrint("Outputing '+' Token"); return makeToken(TOKEN_PLUS);}
        case '-': {debugPrint("Outputing '-' Token"); return makeToken(TOKEN_MINUS);}
        case '/': {debugPrint("Outputing '/' Token");return makeToken(TOKEN_SLASH);} // We consume comments in skipWhiteSpace()
        case '*': {debugPrint("Outputing '*' Token"); return makeToken(TOKEN_STAR);}

        case '!':
            return makeToken(match('=') ? (debugPrint("Outputing '!=' Token"), TOKEN_BANG_EQUAL) 
            : ( debugPrint("Outputing '!' Token"),TOKEN_BANG) );
        case '=':
            return makeToken(match('=') ? (debugPrint("Outputing '==' Token"), TOKEN_EQUAL_EQUAL) 
            : (debugPrint ("Outputing '=' Token"), TOKEN_EQUAL));
        case '<':
            return makeToken(match('=') ? (debugPrint("Outputing '<=' Token"), TOKEN_LESS_EQUAL)
            : (debugPrint ("Outputing '<' Token"), TOKEN_EQUAL));
        case '>':
            return makeToken(match('=') ? (debugPrint("Outputing '>=' Token"), TOKEN_GREATER_EQUAL) 
            : (debugPrint("Outputing '>' Token"), TOKEN_GREATER) );

        // Literals
        case '"': {debugPrint("Outputing String literal"); return string();}
    }



    debugPrint("Couldn't detect character, outputing error token");
    return errorToken("Unexpected character.");
}

