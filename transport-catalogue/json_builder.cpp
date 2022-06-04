#include "json_builder.h"
#include <stdexcept>
#include <variant>

namespace json {
	using namespace std::literals;

	DictValueContext BaseContext::Key(std::string value) {
		return { builder_.Key(value) };
	}
	DictKeyContext BaseContext::StartDict() {
		return DictKeyContext(builder_.StartDict());
	}
	ArrayContext BaseContext::StartArray() {
		return ArrayContext(builder_.StartArray());
	}
	Builder& BaseContext::EndDict() {
		return builder_.EndDict();
	}
	Builder& BaseContext::EndArray() {
		return builder_.EndArray();
	}

	DictKeyContext DictValueContext::Value(Node::Value value) {
		return DictKeyContext(builder_.Value(value));
	}

	ArrayContext ArrayContext::Value(Node::Value value) {
		return ArrayContext(builder_.Value(value));
	}

	DictValueContext Builder::Key(std::string key) {
		if (last_method_called_ == POSSIBLE_METHOD_CALLS::KEY || !nodes_stack_.top()->IsMap()) {
			throw std::logic_error("Can't call key outside the Dict"s);
		}
		last_method_called_ = POSSIBLE_METHOD_CALLS::KEY;
		last_called_key_ = std::move(key);

		((nodes_stack_.top())->AsMap()).insert({ last_called_key_, nullptr });

		return DictValueContext(*this);
	}

	Builder& Builder::Value(Node::Value value) {
		if (!(last_method_called_ == POSSIBLE_METHOD_CALLS::CONSTRUCTOR ||
			nodes_stack_.top()->IsArray() ||
			(nodes_stack_.top()->IsMap() && last_method_called_ == POSSIBLE_METHOD_CALLS::KEY)))
		{
			throw std::logic_error("Value inserting error");
		}

		if (last_method_called_ == POSSIBLE_METHOD_CALLS::CONSTRUCTOR) {
			root_.GetValue() = value;
			nodes_stack_.push(&root_);
		}
		else if (nodes_stack_.top()->IsArray()) {
			Node& inserted_into_array = ((nodes_stack_.top())->AsArray()).emplace_back(Node());
			inserted_into_array.GetValue() = value;
		}
		else if (nodes_stack_.top()->IsMap()) {
			((nodes_stack_.top())->AsMap())[last_called_key_].GetValue() = value;
		}
		last_method_called_ = POSSIBLE_METHOD_CALLS::VALUE;
		return *this;
	}

	DictKeyContext Builder::StartDict() {
		if (!(last_method_called_ == POSSIBLE_METHOD_CALLS::CONSTRUCTOR
			|| last_method_called_ == POSSIBLE_METHOD_CALLS::KEY
			|| nodes_stack_.top()->IsArray()))
		{
			throw std::logic_error("Dict starting error");
		}

		if (last_method_called_ == POSSIBLE_METHOD_CALLS::CONSTRUCTOR) {
			root_ = Dict();
			nodes_stack_.push(&root_);
		}
		else if (last_method_called_ == POSSIBLE_METHOD_CALLS::KEY) {
			Node& last_dict_value = ((nodes_stack_.top())->AsMap())[last_called_key_];
			last_dict_value = Dict();
			nodes_stack_.push(&last_dict_value);
		}
		else if (nodes_stack_.top()->IsArray()) {
			Node& emplaced_into_array = ((nodes_stack_.top())->AsArray()).emplace_back(Dict());
			nodes_stack_.push(&emplaced_into_array);
		}

		last_method_called_ = POSSIBLE_METHOD_CALLS::START_DICT;
		return DictKeyContext(*this);
	}

	ArrayContext Builder::StartArray() {
		if (!(last_method_called_ == POSSIBLE_METHOD_CALLS::CONSTRUCTOR
			|| last_method_called_ == POSSIBLE_METHOD_CALLS::KEY
			|| nodes_stack_.top()->IsArray()))
		{
			throw std::logic_error("Array starting error");
		}

		if (last_method_called_ == POSSIBLE_METHOD_CALLS::CONSTRUCTOR) {
			root_ = Array();
			nodes_stack_.push(&root_);
		}
		else if (last_method_called_ == POSSIBLE_METHOD_CALLS::KEY) {
			Node& last_dict_value = ((nodes_stack_.top())->AsMap())[last_called_key_];
			last_dict_value = Array();
			nodes_stack_.push(&last_dict_value);
		}
		else if (nodes_stack_.top()->IsArray()) {
			Node& emplaced_into_array = ((nodes_stack_.top())->AsArray()).emplace_back(Array());
			nodes_stack_.push(&emplaced_into_array);
		}

		last_method_called_ = POSSIBLE_METHOD_CALLS::START_ARRAY;
		return ArrayContext(*this);
	}

	Builder& Builder::EndDict() {
		if (!nodes_stack_.top()->IsMap()) {
			throw std::logic_error("Dict ending error");
		}
		nodes_stack_.pop();
		return *this;
	}

	Builder& Builder::EndArray() {
		if (!nodes_stack_.top()->IsArray()) {
			throw std::logic_error("Array ending error");
		}
		nodes_stack_.pop();
		return *this;
	}

	Node Builder::Build() {
		if (nodes_stack_.size() > 1 || last_method_called_ == POSSIBLE_METHOD_CALLS::CONSTRUCTOR) {
			throw std::logic_error("Building error");
		}
		return root_;
	}
}