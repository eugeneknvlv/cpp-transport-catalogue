#include "json.h"

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;
            bool met_the_end = false;

            for (char c; input >> c;) {
                if (c == ']') {
                    met_the_end = true;
                    break;
                }
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            if (!met_the_end) {
                throw ParsingError("Array parsing error");
            }

            return Node(move(result));
        }

        using Number = std::variant<int, double>;
        Node LoadNumber(istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit was expected");
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }

            if (input.peek() == '0') {
                read_char();
            }
            else {
                read_digits();
            }

            bool is_int = true;
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    try {
                        return Node(std::stoi(parsed_num));
                    }
                    catch (...) {

                    }
                }
                return Node(std::stod(parsed_num));
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }

        }

        Node LoadString(istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();

            std::string s;
            while (true) {
                if (it == end) {
                    throw ParsingError("String parsing error"s);
                }
                const char ch = *it;
                if (ch == '"') {
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    ++it;
                    if (it == end) {
                        throw ParsingError("String parsing error"s);
                    }
                    const char escaped_char = *it;
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    s.push_back(ch);
                }
                ++it;
            }

            return Node(s);
        }

        Node LoadDict(istream& input) {
            Dict result;
            bool met_the_end = false;

            for (char c; input >> c;) {
                if (c == '}') {
                    met_the_end = true;
                    break;
                }
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }
            if (!met_the_end) {
                throw ParsingError("Dict parsing error"s);
            }
            return Node(move(result));
        }

        Node LoadBool(istream& input) {
            string s;
            for (char c; input >> c;) {
                if (c == EOF || c == ',' || c == ']' || c == '}') {
                    input.putback(c);
                    break;
                }
                s += c;
            }

            if (s == "true"s) {
                return Node(true);
            }
            else if (s == "false"s) {
                return Node(false);
            }
            else {
                throw ParsingError("Couldn't parse bool value"s);
            }
        }

        Node LoadNull(istream& input) {
            string s;
            for (char c; input >> c;) {
                if (c == EOF || c == ',' || c == ']' || c == '}') {
                    input.putback(c);
                    break;
                }
                s += c;
            }

            if (s == "null"s) {
                return Node(nullptr);
            }
            else {
                throw ParsingError("Couldn't parse null value");
            }
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            switch (c) {
            case EOF:
                throw ParsingError("End of file");
                break;
            case '[':
                return LoadArray(input);
                break;
            case '{':
                return LoadDict(input);
                break;
            case '"':
                return LoadString(input);
                break;
            case ('t'):
                input.putback(c);
                return LoadBool(input);
                break;
            case ('f'):
                input.putback(c);
                return LoadBool(input);
                break;
            case ('n'):
                input.putback(c);
                return LoadNull(input);
                break;
            default:
                input.putback(c);
                return LoadNumber(input);
            }
        }
    } // anonymous namespace

    bool Node::IsInt() const {
        return holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const {
        return holds_alternative<int>(*this) || holds_alternative<double>(*this);
    }

    bool Node::IsPureDouble() const {
        return holds_alternative<double>(*this);
    }

    bool Node::IsBool() const {
        return holds_alternative<bool>(*this);
    }

    bool Node::IsString() const {
        return holds_alternative<string>(*this);
    }

    bool Node::IsNull() const {
        return holds_alternative<nullptr_t>(*this);
    }

    bool Node::IsArray() const {
        return holds_alternative<Array>(*this);
    }

    bool Node::IsMap() const {
        return holds_alternative<Dict>(*this);
    }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw logic_error("logic error");
        }
        return get<int>(*this);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw logic_error("logic error");
        }
        return get<bool>(*this);
    }

    double Node::AsDouble() const {
        if (!IsDouble()) {
            throw logic_error("logic error");
        }
        try {
            return get<double>(*this);
        }
        catch (...) {
            return get<int>(*this);
        }
    }

    string& Node::AsString() {
        if (!IsString()) {
            throw logic_error("logic error");
        }
        return get<string>(*this);
    }

    Array& Node::AsArray() {
        if (!IsArray()) {
            throw logic_error("logic error");
        }
        return get<Array>(*this);
    }

    Dict& Node::AsMap() {
        if (!IsMap()) {
            throw logic_error("logic error");
        }
        return get<Dict>(*this);
    }

    const std::string& Node::AsString() const {
        if (!IsString()) {
            throw logic_error("logic error");
        }
        return get<string>(*this);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw logic_error("logic error");
        }
        return get<Array>(*this);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw logic_error("logic error");
        }
        return get<Dict>(*this);
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    void PrintNode(const Node& value, const PrintContext& ctx);

    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& ctx) {
        ctx.out << value;
    }

    void PrintString(const std::string& value, std::ostream& out) {
        out.put('"');
        for (const char c : value) {
            switch (c) {
            case '\r':
                out << "\\r"sv;
                break;
            case '\n':
                out << "\\n"sv;
                break;
            case '"':
                // Символы " и \ выводятся как \" или \\, соответственно
                [[fallthrough]];
            case '\\':
                out.put('\\');
                [[fallthrough]];
            default:
                out.put(c);
                break;
            }
        }
        out.put('"');
    }

    template <>
    void PrintValue<std::string>(const std::string& value, const PrintContext& ctx) {
        PrintString(value, ctx.out);
    }

    template <>
    void PrintValue<std::nullptr_t>(const std::nullptr_t&, const PrintContext& ctx) {
        ctx.out << "null"sv;
    }

    // В специализаци шаблона PrintValue для типа bool параметр value передаётся
    // по константной ссылке, как и в основном шаблоне.
    // В качестве альтернативы можно использовать перегрузку:
    // void PrintValue(bool value, const PrintContext& ctx);
    template <>
    void PrintValue<bool>(const bool& value, const PrintContext& ctx) {
        ctx.out << (value ? "true"sv : "false"sv);
    }

    template <>
    void PrintValue<Array>(const Array& nodes, const PrintContext& ctx) {
        std::ostream& out = ctx.out;
        out << "[\n"sv;
        bool first = true;
        auto inner_ctx = ctx.Indented();
        for (const Node& node : nodes) {
            if (first) {
                first = false;
            }
            else {
                out << ",\n"sv;
            }
            inner_ctx.PrintIndent();
            PrintNode(node, inner_ctx);
        }
        out.put('\n');
        ctx.PrintIndent();
        out.put(']');
    }

    template <>
    void PrintValue<Dict>(const Dict& nodes, const PrintContext& ctx) {
        std::ostream& out = ctx.out;
        out << "{\n"sv;
        bool first = true;
        auto inner_ctx = ctx.Indented();
        for (const auto& [key, node] : nodes) {
            if (first) {
                first = false;
            }
            else {
                out << ",\n"sv;
            }
            inner_ctx.PrintIndent();
            PrintString(key, ctx.out);
            out << ": "sv;
            PrintNode(node, inner_ctx);
        }
        out.put('\n');
        ctx.PrintIndent();
        out.put('}');
    }

    void PrintNode(const Node& node, const PrintContext& ctx) {
        std::visit(
            [&ctx](const auto& value) {
                PrintValue(value, ctx);
            },
            node.GetNodeData());
    }

    Document Load(std::istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), PrintContext{ output });
    }

}  // namespace json