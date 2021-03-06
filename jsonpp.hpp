﻿#pragma once
#include <string>
#include <stdexcept>
#include <map>
#include <memory>
#include <vector>

namespace json {

    struct parse_exception : public std::runtime_error {
        parse_exception(const std::string& msg);
    };

    struct ValueVisitor;

    class Value {
    public:
        virtual ~Value() {}
        virtual void accept(ValueVisitor* visitor) const = 0;

        bool isObject() const;
        bool isArray() const;
        bool isString() const;
        bool isNumber() const;
        bool isNull() const;
        bool isBool() const;

    protected:
        enum class ValueType {
            OBJECT,
            ARRAY,
            STRING,
            BOOL,
            JNULL,
            NUMBER
        } _type;

        bool isType(ValueType type) const;
        Value(ValueType type);
    };

    class Object;
    class Array;

    class Array : public Value {
    public:
        Array();
        void addValue(std::unique_ptr<Value> value);
        size_t size() const;

        std::vector<Value*> getValues() const;
        Value* getValue(size_t index) const;
        Object* getObjectValue(size_t index) const;
        Array* getArrayValue(size_t index) const;

        std::string getStringValue(size_t index, const std::string& defaulValue = "") const;
        bool getBoolValue(size_t index, bool defaultValue = false) const;
        double getNumberValue(size_t index, double defaultValue = 0.0f) const;
        void* getNullValue(size_t index) const;

        virtual void accept(ValueVisitor* visitor) const override;
    private:
        std::vector<std::unique_ptr<Value>> _values;
    };

    class Object : public Value {
    public:
        Object();

        std::map<std::string, Value*> getValues() const;

        size_t size() const;

        Value* getValue(const std::string& name) const;
        Object* getObjectValue(const std::string& name) const;
        Array* getArrayValue(const std::string& name) const;

        std::string getStringValue(const std::string& name, const std::string& defaulValue = "") const;
        bool getBoolValue(const std::string& name, bool defaultValue = false) const;
        double getNumberValue(const std::string& name, double defaultValue = 0.0f) const;
        void* getNullValue(const std::string& name) const;

        void addValue(const std::string& name, std::unique_ptr<Value> value);

        virtual void accept(ValueVisitor* visitor) const override;
    private:
        std::map<std::string, std::unique_ptr<Value>> _values;
    };

    class String : public Value {
    public:
        String(const std::string& value);
        std::string getValue() const;

        virtual void accept(ValueVisitor* visitor) const override;
    private:
        std::string _value;
    };

    class Bool : public Value {
    public:
        Bool(bool value);
        bool getValue() const;

        virtual void accept(ValueVisitor* visitor) const override;
    private:
        bool _value;
    };

    class Null : public Value {
    public:
        Null();
        void* getValue() const;

        virtual void accept(ValueVisitor* visitor) const override;
    };

    class Number : public Value {
    public:
        Number(double value);
        double getValue() const;

        virtual void accept(ValueVisitor* visitor) const override;
    private:
        double _value;
    };

    namespace detail {

        enum class TokenType {
            LBRACE,
            RBRACE,
            COLON,
            STRING,
            COMMA,
            LBRACKET,
            RBRACKET,
            JBOOL, // this name prevents macro collision with BOOL macro from WinAPI
            JNULL, // this name prevents macro collision with NULL macro
            NUMBER,
            NONE
        };

        struct Token {
            TokenType type;
            std::string value;
            int line;
            int pos;

            Token(TokenType type = TokenType::NONE, const std::string& value = "", int line = 1, int pos = 1);
        };

        class Lexer {
        public:
            Lexer(const std::string& text);
            Token getToken();
        private:
            size_t _cursor;
            std::string _text;
            int _line;
            int _pos;

            enum NumberState { SIGN, DIGIT, DECIMAL, EXPONENT, EXPONENT_DIGIT, END };

            Lexer::NumberState processState(NumberState state, std::string& value);

            bool isDoneReading() const;

            bool isHexChar(char c);
            std::string getHexDigits();
            bool isWhitespaceControlChar(char n) const;
            bool isControlChar(char c) const;
            char getControlChar();
            Token lexString();

            std::string lexValueSequence(const std::string& expected);
            Token lexBool(const std::string& expected);
            Token lexNull();

            Token lexNumber();

            void raiseError(const std::string& expected);

            void skipWhitespace();
            char next();
            char curr();
            char peek();

            Token reportToken(TokenType type, const std::string& str);
        };

        class Parser {
        public:
            std::unique_ptr<Object> parse();
            Parser(Lexer lexer);
        private:
            Lexer lexer;
            Token currentToken;

            std::unique_ptr<Object> parseObject();
            std::unique_ptr<Value> parseValue();
            std::unique_ptr<Array> parseArray();
            std::unique_ptr<Object> parseValueList();
            void raiseError(const std::string& expected);
        };
    }

    std::unique_ptr<Object> load(const std::string& filePath);
    
    void write(const Object* obj, const std::string& filePath);

    std::unique_ptr<Object> parse(const std::string& text);

    struct ValueVisitor {
        virtual ~ValueVisitor() {}

        virtual void visit(const Object* obj) = 0;
        virtual void visit(const Array* obj) = 0;
        virtual void visit(const String* obj) = 0;
        virtual void visit(const Bool* obj) = 0;
        virtual void visit(const Null* obj) = 0;
        virtual void visit(const Number* obj) = 0;
    };

    class ValueWriter : public ValueVisitor {
    public:
        virtual void visit(const Object* obj) override;
        virtual void visit(const Array* obj) override;
        virtual void visit(const String* obj) override;
        virtual void visit(const Bool* obj) override;
        virtual void visit(const Null* obj) override;
        virtual void visit(const Number* obj) override;

        std::string getString() const;
    private:
        std::string _str;
    };
}