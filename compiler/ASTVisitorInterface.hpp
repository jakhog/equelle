/*
  Copyright 2013 SINTEF ICT, Applied Mathematics.
*/

#ifndef ASTVISITORINTERFACE_HEADER_INCLUDED
#define ASTVISITORINTERFACE_HEADER_INCLUDED


class SequenceNode;
class NumberNode;
class StringNode;
class TypeNode;
class FuncTypeNode;
class BinaryOpNode;
class ComparisonOpNode;
class NormNode;
class UnaryNegationNode;
class OnNode;
class TrinaryIfNode;
class VarDeclNode;
class VarAssignNode;
class VarNode;
class FuncRefNode;
class JustAnIdentifierNode;
class FuncArgsDeclNode;
class FuncDeclNode;
class FuncStartNode;
class FuncAssignNode;
class FuncArgsNode;
class ReturnStatementNode;
class FuncCallNode;
class FuncCallStatementNode;
class LoopNode;
class ArrayNode;
class RandomAccessNode;
class StencilAccessNode;
class StencilStatementNode;


class ASTVisitorInterface
{
public:
    virtual void visit(SequenceNode& node) = 0;
    virtual void midVisit(SequenceNode& node) = 0;
    virtual void postVisit(SequenceNode& node) = 0;
    virtual void visit(NumberNode& node) = 0;
    virtual void visit(StringNode& node) = 0;
    virtual void visit(TypeNode& node) = 0;
    virtual void visit(FuncTypeNode& node) = 0;
    virtual void visit(BinaryOpNode& node) = 0;
    virtual void midVisit(BinaryOpNode& node) = 0;
    virtual void postVisit(BinaryOpNode& node) = 0;
    virtual void visit(ComparisonOpNode& node) = 0;
    virtual void midVisit(ComparisonOpNode& node) = 0;
    virtual void postVisit(ComparisonOpNode& node) = 0;
    virtual void visit(NormNode& node) = 0;
    virtual void postVisit(NormNode& node) = 0;
    virtual void visit(UnaryNegationNode& node) = 0;
    virtual void postVisit(UnaryNegationNode& node) = 0;
    virtual void visit(OnNode& node) = 0;
    virtual void midVisit(OnNode& node) = 0;
    virtual void postVisit(OnNode& node) = 0;
    virtual void visit(TrinaryIfNode& node) = 0;
    virtual void questionMarkVisit(TrinaryIfNode& node) = 0;
    virtual void colonVisit(TrinaryIfNode& node) = 0;
    virtual void postVisit(TrinaryIfNode& node) = 0;
    virtual void visit(VarDeclNode& node) = 0;
    virtual void postVisit(VarDeclNode& node) = 0;
    virtual void visit(VarAssignNode& node) = 0;
    virtual void postVisit(VarAssignNode& node) = 0;
    virtual void visit(VarNode& node) = 0;
    virtual void visit(FuncRefNode& node) = 0;
    virtual void visit(JustAnIdentifierNode& node) = 0;
    virtual void visit(FuncArgsDeclNode& node) = 0;
    virtual void midVisit(FuncArgsDeclNode& node) = 0;
    virtual void postVisit(FuncArgsDeclNode& node) = 0;
    virtual void visit(FuncDeclNode& node) = 0;
    virtual void postVisit(FuncDeclNode& node) = 0;
    virtual void visit(FuncStartNode& node) = 0;
    virtual void postVisit(FuncStartNode& node) = 0;
    virtual void visit(FuncAssignNode& node) = 0;
    virtual void postVisit(FuncAssignNode& node) = 0;
    virtual void visit(FuncArgsNode& node) = 0;
    virtual void midVisit(FuncArgsNode& node) = 0;
    virtual void postVisit(FuncArgsNode& node) = 0;
    virtual void visit(ReturnStatementNode& node) = 0;
    virtual void postVisit(ReturnStatementNode& node) = 0;
    virtual void visit(FuncCallNode& node) = 0;
    virtual void postVisit(FuncCallNode& node) = 0;
    virtual void visit(FuncCallStatementNode& node) = 0;
    virtual void postVisit(FuncCallStatementNode& node) = 0;
    virtual void visit(LoopNode& node) = 0;
    virtual void postVisit(LoopNode& node) = 0;
    virtual void visit(ArrayNode& node) = 0;
    virtual void postVisit(ArrayNode& node) = 0;
    virtual void visit(RandomAccessNode& node) = 0;
    virtual void postVisit(RandomAccessNode& node) = 0;

    virtual void visit( StencilAccessNode& node ) = 0;
    virtual void midVisit( StencilAccessNode& node ) = 0;
    virtual void postVisit( StencilAccessNode& node ) = 0;
    virtual void visit( StencilStatementNode& node ) = 0;
    virtual void midVisit( StencilStatementNode& node ) = 0;
    virtual void postVisit( StencilStatementNode& node ) = 0;


    virtual ~ASTVisitorInterface()
    {
    };
};

#endif // ASTVISITORINTERFACE_HEADER_INCLUDED
