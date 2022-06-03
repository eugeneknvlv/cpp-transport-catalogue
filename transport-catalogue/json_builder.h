#pragma once

#include "json.h"
#include <stack>
#include <string>

namespace json {

	class Builder;
	class DictValueContext;
	class DictKeyContext;
	class ArrayContext;

	class BaseContext {
	public:
		Builder& builder_;
		BaseContext(Builder& builder)
			: builder_(builder)
		{}

		DictValueContext Key(std::string value);
		DictKeyContext StartDict();
		ArrayContext StartArray();
		Builder& EndDict();
		Builder& EndArray();
	};

	class DictKeyContext : private BaseContext {
	public:
		using BaseContext::BaseContext;
		using BaseContext::Key;
		using BaseContext::EndDict;
	};

	class DictValueContext : private BaseContext {
	public:
		using BaseContext::BaseContext;
		using BaseContext::StartDict;
		using BaseContext::StartArray;

		DictKeyContext Value(Node::Value value);
	};

	class ArrayContext : private BaseContext {
	public:
		using BaseContext::BaseContext;
		using BaseContext::StartDict;
		using BaseContext::StartArray;
		using BaseContext::EndArray;

		ArrayContext Value(Node::Value value);
	};

	//class ValueAfterConstructorContext : private BaseContext {
	//	using BaseContext::BaseContext;
	//	using BaseContext::StartDict;
	//	using BaseContext::StartArray;

	//	ValueAfterConstructorContext Value(Node::Value value) {
	//		return ValueAfterConstructorContext(builder_.Value(value));
	//	}
	//};

	class Builder {
	public:
		Builder()
			: root_(nullptr)
			, last_method_called_(POSSIBLE_METHOD_CALLS::CONSTRUCTOR)
		{}

		DictValueContext Key(std::string value);
		Builder& Value(Node::Value value);
		DictKeyContext StartDict();
		ArrayContext StartArray();
		Builder& EndDict();
		Builder& EndArray();
		Node Build();

	private:
		Node root_;
		std::stack<Node*> nodes_stack_;

		enum class POSSIBLE_METHOD_CALLS {
			CONSTRUCTOR,
			KEY,
			VALUE,
			START_DICT,
			START_ARRAY,
			END_DICT,
			END_ARRAY
		} last_method_called_ = POSSIBLE_METHOD_CALLS::CONSTRUCTOR;

		std::string last_called_key_;
	};

} // namespace json