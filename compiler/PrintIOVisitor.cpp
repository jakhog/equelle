/*
  Copyright 2013 SINTEF ICT, Applied Mathematics.
*/

#include "PrintIOVisitor.hpp"
#include "ASTNodes.hpp"
#include "SymbolTable.hpp"
#include <iostream>
#include <stdexcept>


PrintIOVisitor::PrintIOVisitor() {}

PrintIOVisitor::~PrintIOVisitor() {}


void PrintIOVisitor::visit(FuncCallNode& node)
{
    if (node.name().find("Input") == 0) {
        // This is a call to an input function
        std::cout << "Input\n";
        // Loop through declared and specified arguments to this function to figure out the tag and default value
        auto argumentDeclarations = SymbolTable::getFunction(node.name()).functionType().arguments();
        auto argumentExpressions = node.argumentsNode().arguments();
        size_t arguments = argumentExpressions.size();
        for ( size_t i = 0; i < arguments; ++i) {
            auto name = argumentDeclarations[i].name();
            auto arg = argumentExpressions[i];
            if (name == "name" && arg->type() == EquelleType(String)) {
                // This is the tag argument, and it is a simple string
                auto tag = static_cast<StringNode*>(arg);
                std::cout << "Tag: " << tag->content() << '\n';
            } else if (name == "default" && arg->type() == EquelleType(Scalar)) {
                // This is the default argument, and it is a simple scalar
                auto val = static_cast<NumberNode*>(arg);
                std::cout << "Default: " << val->number() << '\n';
            }
        }
        // Print the expected type of the input (ie. what should be in the provided file)
        std::cout << "Type: " << SymbolTable::equelleString(node.type()) << "\n\n";

    } else if (node.name().find("Output") == 0) {
        // This is a call to an output function
        std::cout << "Output\n";
        // Loop through declared and specified arguments to this function to figure out the tag and what will be written to file
        auto argumentDeclarations = SymbolTable::getFunction(node.name()).functionType().arguments();
        auto argumentExpressions = node.argumentsNode().arguments();
        size_t arguments = argumentExpressions.size();
        for ( size_t i = 0; i < arguments; ++i) {
            auto name = argumentDeclarations[i].name();
            auto arg = argumentExpressions[i];
            if (name == "tag" && arg->type() == EquelleType(String)) {
                // This is the tag argument, and it is a simple string
                auto tag = static_cast<StringNode*>(arg);
                std::cout << "Tag: " << tag->content() << '\n';
            } else if (name == "data") {
                std::cout << "Type: " << SymbolTable::equelleString(arg->type()) << '\n';
            }
        }
        // This does not return anything
        std::cout << '\n';
    }
}
void PrintIOVisitor::postVisit(FuncCallNode& node)
{
}

void PrintIOVisitor::visit(FuncArgsNode& node) {}
void PrintIOVisitor::visit(SequenceNode&) {}
void PrintIOVisitor::visit(NumberNode& node)  {}
void PrintIOVisitor::visit(StringNode& node) {}
void PrintIOVisitor::visit(TypeNode& node) {}
void PrintIOVisitor::visit(FuncTypeNode& node) {}
void PrintIOVisitor::visit(BinaryOpNode& node) {}
void PrintIOVisitor::visit(ComparisonOpNode& node) {}
void PrintIOVisitor::visit(NormNode&) {}
void PrintIOVisitor::visit(UnaryNegationNode&) {}
void PrintIOVisitor::visit(OnNode& node) {}
void PrintIOVisitor::visit(TrinaryIfNode&) {}
void PrintIOVisitor::visit(VarDeclNode& node) {}
void PrintIOVisitor::visit(VarAssignNode& node) {}
void PrintIOVisitor::visit(VarNode& node) {}
void PrintIOVisitor::visit(FuncRefNode& node) {}
void PrintIOVisitor::visit(JustAnIdentifierNode& node) {}
void PrintIOVisitor::visit(FuncArgsDeclNode&) {}
void PrintIOVisitor::visit(FuncDeclNode& node) {}
void PrintIOVisitor::visit(FuncStartNode& node) {}
void PrintIOVisitor::visit(FuncAssignNode&) {}
void PrintIOVisitor::visit(ReturnStatementNode&) {}
void PrintIOVisitor::visit(FuncCallStatementNode&) {}
void PrintIOVisitor::visit(LoopNode& node) {}
void PrintIOVisitor::visit(ArrayNode& node) {}
void PrintIOVisitor::visit(RandomAccessNode& node) {}
void PrintIOVisitor::midVisit(SequenceNode&) {}
void PrintIOVisitor::postVisit(SequenceNode&) {}
void PrintIOVisitor::midVisit(BinaryOpNode&) {}
void PrintIOVisitor::postVisit(BinaryOpNode&) {}
void PrintIOVisitor::midVisit(ComparisonOpNode&) {}
void PrintIOVisitor::postVisit(ComparisonOpNode&) {}
void PrintIOVisitor::postVisit(NormNode&) {}
void PrintIOVisitor::postVisit(UnaryNegationNode&) {}
void PrintIOVisitor::midVisit(OnNode&) {}
void PrintIOVisitor::postVisit(OnNode&) {}
void PrintIOVisitor::questionMarkVisit(TrinaryIfNode&) {}
void PrintIOVisitor::colonVisit(TrinaryIfNode&) {}
void PrintIOVisitor::postVisit(TrinaryIfNode&) {}
void PrintIOVisitor::postVisit(VarDeclNode&) {}
void PrintIOVisitor::postVisit(VarAssignNode&) {}
void PrintIOVisitor::midVisit(FuncArgsDeclNode&) {}
void PrintIOVisitor::postVisit(FuncArgsDeclNode&) {}
void PrintIOVisitor::postVisit(FuncDeclNode&) {}
void PrintIOVisitor::postVisit(FuncStartNode&) {}
void PrintIOVisitor::postVisit(FuncAssignNode&) {}
void PrintIOVisitor::midVisit(FuncArgsNode&) {}
void PrintIOVisitor::postVisit(FuncArgsNode&) {}
void PrintIOVisitor::postVisit(ReturnStatementNode&) {}
void PrintIOVisitor::postVisit(FuncCallStatementNode&) {}
void PrintIOVisitor::postVisit(LoopNode&) {}
void PrintIOVisitor::postVisit(ArrayNode&) {}
void PrintIOVisitor::postVisit(RandomAccessNode&) {}
void PrintIOVisitor::visit(StencilAccessNode &node) {}
void PrintIOVisitor::midVisit(StencilAccessNode &node) {}
void PrintIOVisitor::postVisit(StencilAccessNode &node) {}
void PrintIOVisitor::visit(StencilStatementNode &node) {}
void PrintIOVisitor::midVisit(StencilStatementNode &node) {}
void PrintIOVisitor::postVisit(StencilStatementNode &node) {}
