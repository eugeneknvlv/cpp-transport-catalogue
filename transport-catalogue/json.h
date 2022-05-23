#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    class Node;
    // —охраните объ€влени€ Dict и Array без изменени€
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using NodeData = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;

    // Ёта ошибка должна выбрасыватьс€ при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node : public NodeData {
    public:
        using variant::variant;

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        NodeData GetValue() const {
            return *this;
        }

        bool operator==(const Node& other) const {
            return *this == other;
        }

        bool operator!=(const Node& other) const {
            return *this != other;
        }
    };

    class Document {
    public:
        Document() = default;
        explicit Document(Node root);

        const Node& GetRoot() const;
        bool operator==(const Document& other) const {
            return root_ == other.root_;
        }
        bool operator!=(const Document& other) const {
            return !(root_ == other.root_);
        }

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };

    void Print(const Document& doc, std::ostream& output);

}  // namespace json