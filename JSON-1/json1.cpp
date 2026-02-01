#include <cctype>
#include <fstream>
#include <iostream>
#include <string>

enum class Token {
    EndOfStream,
    Invalid,
    OpenCurlyBrace,
    CloseCurlBrace,
    OpenSquareBrace,
    CloseSquareBrace,
    Colon,
    Comma,
    String,
    Number,
    True,
    False,
    Null
};

class JsonLexer
{
    std::istream&   file;
    std::string     token;

    Token extractTrue();
    Token extractFalse();
    Token extractNull();
    Token extractString();
    Token extractNumber(char n);

    public:
        JsonLexer(std::istream& file)
            : file(file)
        {}
        Token nextToken();
};

class JsonParser
{
    JsonLexer&      lexer;

    bool parseValue(Token next);
    bool parseObject(Token next);
    bool parseArray(Token next);

    public:
        JsonParser(JsonLexer& lexer)
            : lexer(lexer)
        {}

    bool parse();
};

bool checkJson(std::string const& fileName, std::istream& file)
{
    JsonLexer       lexer(file);
    JsonParser      parser(lexer);

    bool valid      = parser.parse();
    std::cout << fileName << ":\t\t" << ((valid ? "Valid" : "In Valid")) << "\n";
    return valid;
}

int main(int argc, char* argv[])
{
    bool result = true;
    if (argc == 1) {
        result = checkJson("", std::cin);
    }
    else {
        for (int loop = 1; loop < argc; ++loop) {
            std::ifstream   file(argv[loop]);
            if (!file) {
                std::cerr << "Invalid File: " << argv[loop] << "\n";
            }
            if (!checkJson(argv[loop], file)) {
                result = false;
            }
        }
    }
    return result ? 0 : 1;
}

bool JsonParser::parse()
{
    Token   next = lexer.nextToken();
    bool result = parseValue(next);
    if (result) {
        next = lexer.nextToken();
        // There should be no more tokens on the input stream.
        // If there are then this is an error.
        result = (next == Token::EndOfStream);
    }
    return result;
};

bool JsonParser::parseValue(Token next)
{
    switch (next)
    {
        case Token::OpenCurlyBrace:         return parseObject(next);
        case Token::OpenSquareBrace:        return parseArray(next);
        case Token::String:                 return true;
        case Token::Number:                 return true;
        case Token::True:                   return true;
        case Token::False:                  return true;
        case Token::Null:                   return true;
        default:
            // Anything else is an error for a value.
            return false;
    }
}

bool JsonParser::parseArray(Token next)
{
    if (next != Token::OpenSquareBrace) {
        return false;
    }
    next = lexer.nextToken();
    if (next == Token::CloseSquareBrace) {
        return true;
    }
    while (true)
    {
        if (!parseValue(next)) {
            return false;
        }
        next = lexer.nextToken();
        if (next == Token::CloseSquareBrace) {
            return true;
        }
        if (next != Token::Comma) {
            return false;
        }
        next = lexer.nextToken();
    }
}

bool JsonParser::parseObject(Token next)
{
    if (next != Token::OpenCurlyBrace) {
        return false;
    }
    next = lexer.nextToken();
    if (next == Token::CloseCurlBrace) {
        return true;
    }
    while (true)
    {
        if (next != Token::String) {
            return false;
        }
        next = lexer.nextToken();
        if (next != Token::Colon) {
            return false;
        }
        next = lexer.nextToken();
        if (!parseValue(next)) {
            return false;
        }
        next = lexer.nextToken();
        if (next == Token::CloseCurlBrace) {
            return true;
        }
        if (next != Token::Comma) {
            return false;
        }
        next = lexer.nextToken();
    }
}

Token JsonLexer::nextToken()
{
    char    n;
    if (file >> n) {
        // White space automatically skipped.
        switch (n) {
            case '{':       return Token::OpenCurlyBrace;
            case '}':       return Token::CloseCurlBrace;
            case '[':       return Token::OpenSquareBrace;
            case ']':       return Token::CloseSquareBrace;
            case ':':       return Token::Colon;
            case ',':       return Token::Comma;
            case 't':       return extractTrue();
            case 'f':       return extractFalse();
            case 'n':       return extractNull();
            case '"':       return extractString();
            default:        return extractNumber(n);
        }
    }
    else {
        // Failing to read from the stream is an invalid token.
        return Token::EndOfStream;
    }
}

Token JsonLexer::extractTrue()
{
    static char buffer[3];
    if (!file.read(buffer, 3) || file.gcount() != 3 || strncmp(buffer, "rue", 3) != 0) {
        return Token::Invalid;
    }
    return Token::True;
}

Token JsonLexer::extractFalse()
{
    static char buffer[4];
    if (!file.read(buffer, 4) || file.gcount() != 4 || strncmp(buffer, "alse", 4) != 0) {
        return Token::Invalid;
    }
    return Token::False;
}

Token JsonLexer::extractNull()
{
    static char buffer[3];
    if (!file.read(buffer, 3) || file.gcount() != 3 || strncmp(buffer, "ull", 3) != 0) {
        return Token::Invalid;
    }
    return Token::Null;
}

Token JsonLexer::extractString()
{
    token.clear();
    while (true) {
        int n = file.get();
        if (n == std::char_traits<char>::eof()) {
            return Token::Invalid;
        }
        if (n == '"') {
            return Token::String;
        }
        if (n != '\\') {
            token += n;
            continue;
        }
        n = file.get();
        if (n == std::char_traits<char>::eof()) {
            return Token::Invalid;
        }
        switch (n) {
            case '"':
            case '\\':
            case '/':
                token += n;
                break;
            case 'b':       token += '\b';break;
            case 'f':       token += '\f';break;
            case 'n':       token += '\n';break;
            case 'r':       token += '\r';break;
            case 't':       token += '\t';break;
            case 'u':
            {
                int UTF8 = 0;
                for (int loop = 0; loop < 4; ++loop) {
                    n = file.get();
                    if (n == std::char_traits<char>::eof()) {
                        return Token::Invalid;
                    }
                    if (n >= '0' && n <= '9') {
                        UTF8 = UTF8 * 16 + (n - '0');
                    }
                    else if (n >= 'a' && n <= 'f') {
                        UTF8 = UTF8 * 16 + (n - 'a' + 10);
                    }
                    else if (n >= 'A' && n <= 'F') {
                        UTF8 = UTF8 * 16 + (n - 'A' + 10);
                    }
                    else {
                        return Token::Invalid;
                    }
                }
                if (UTF8 <= 0x7f) {
                    // U+0000	U+007F	0yyyzzzz	
                    token += static_cast<char>(UTF8);
                }
                else if (UTF8 <= 0x7FF) {
                    // U+0080	U+07FF	110xxxyy	10yyzzzz	
                    int z = (UTF8 >> 0) & 0xF;
                    int y = (UTF8 >> 4) & 0xF;
                    int x = (UTF8 >> 8) & 0x7;
                    token += static_cast<char>(0xC0 | (x << 2) | (y >> 2));
                    token += static_cast<char>(0x80 | ((y & 0x3) << 4) | z);
                }
                else if (UTF8 <= 0xFFFF) {
                    // U+0800	U+FFFF	1110wwww	10xxxxyy	10yyzzzz	
                    int z = (UTF8 >>  0) & 0xF;
                    int y = (UTF8 >>  4) & 0xF;
                    int x = (UTF8 >>  8) & 0xF;
                    int w = (UTF8 >> 12) & 0xF;
                    token += static_cast<char>(0xE0 | w);
                    token += static_cast<char>(0x80 | (x << 2) | (y >> 2));
                    token += static_cast<char>(0x80 | ((y & 0x3) << 4) | z);
                }
                else if (UTF8 <= 0x10FFFF) {
                    // U+010000	U+10FFFF	11110uvv	10vvwwww	10xxxxyy	10yyzzzz
                    int z = (UTF8 >>  0) & 0xF;
                    int y = (UTF8 >>  4) & 0xF;
                    int x = (UTF8 >>  8) & 0xF;
                    int w = (UTF8 >> 12) & 0xF;
                    int v = (UTF8 >> 16) & 0xF;
                    int u = (UTF8 >> 20) & 0x1;
                    token += static_cast<char>(0xF0 | (u << 2) | (v >> 2));
                    token += static_cast<char>(0x80 | ((v & 0x3) << 4) | w);
                    token += static_cast<char>(0x80 | (x << 2) | (y >> 2));
                    token += static_cast<char>(0x80 | ((y & 0x3) << 4) | z);
                }
                else {
                    return Token::Invalid;
                }
                break;
            }
            default:
                return Token::Invalid;
        }
    }
}

Token JsonLexer::extractNumber(char c)
{
    token.clear();

    // Optional neg sign.
    if (c == '-') {
        token += c;
        int n = file.get();
        if (n == std::char_traits<char>::eof()) {
            return Token::Invalid;
        }
        c = n;
    }

    if (!std::isdigit(c)) {
        return Token::Invalid;
    }
    else if (c == '0') {
        // Leading zero must not be followed by numbers;
        token += c;
        int n = file.get();
        if (n == std::char_traits<char>::eof()) {
            return Token::Number;
        }
        c = n;
    }
    else {
        // Any other digit then suck up all the digits.
        while (std::isdigit(c)) {
            token += c;
            int n = file.get();
            if (n == std::char_traits<char>::eof()) {
                return Token::Number;
            }
            c = n;
        }
    }

    // Fraction
    if (c == '.') {
        token += c;
        int n = file.get();
        if (n == std::char_traits<char>::eof()) {
            return Token::Invalid;
        }
        c = n;

        if (!std::isdigit(c)) {
            return Token::Invalid;
        }
        while (std::isdigit(c)) {
            token += c;
            int n = file.get();
            if (n == std::char_traits<char>::eof()) {
                return Token::Number;
            }
            c = n;
        }
    }

    // Exponent
    if (c == 'e' || c == 'E') {
        token += c;
        int n = file.get();
        if (n == std::char_traits<char>::eof()) {
            return Token::Invalid;
        }
        c = n;

        // Optional sign
        if (c == '-' || c == '+') {
            token += c;
            int n = file.get();
            if (n == std::char_traits<char>::eof()) {
                return Token::Invalid;
            }
            c = n;
        }

        if (!std::isdigit(c)) {
            return Token::Invalid;
        }
        while (std::isdigit(c)) {
            token += c;
            int n = file.get();
            if (n == std::char_traits<char>::eof()) {
                return Token::Number;
            }
            c = n;
        }
    }
    // unused character.
    // put it back on the stream.
    file.unget();
    return Token::Number;
}


