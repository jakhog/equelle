/*
  Copyright 2013 SINTEF ICT, Applied Mathematics.
*/

#include "PrintCPUBackendASTVisitor.hpp"
#include "ASTNodes.hpp"
#include "SymbolTable.hpp"
#include <iostream>
#include <cctype>


namespace
{
    const char* cppStartString();
    const char* cppEndString();
}



PrintCPUBackendASTVisitor::PrintCPUBackendASTVisitor()
    : suppressed_(false),
      indent_(1),
      sequence_depth_(0),
      emit_ad_(false),
      in_extra_funcassign_accept_(false),
      in_function_call_requiring_ad_(false)
{
}

PrintCPUBackendASTVisitor::~PrintCPUBackendASTVisitor()
{
}





void PrintCPUBackendASTVisitor::visit(SequenceNode&)
{
    if (sequence_depth_ == 0) {
        // This is the root node of the program.
        std::cout << cppStartString();
        endl();
    }
    ++sequence_depth_;
}

void PrintCPUBackendASTVisitor::midVisit(SequenceNode&)
{
}

void PrintCPUBackendASTVisitor::postVisit(SequenceNode&)
{
    --sequence_depth_;
    if (sequence_depth_ == 0) {
        // We are back at the root node.
        std::cout << cppEndString();
    }
}

void PrintCPUBackendASTVisitor::visit(NumberNode& node)
{
    std::cout.precision(16);
    std::cout << "double(" << node.number() << ")";
}

void PrintCPUBackendASTVisitor::visit(StringNode& node)
{
    std::cout << node.content();
}

void PrintCPUBackendASTVisitor::visit(TypeNode&)
{
    // std::cout << SymbolTable::equelleString(node.type());
}

void PrintCPUBackendASTVisitor::visit(FuncTypeNode&)
{
    // std::cout << node.funcType().equelleString();
}

void PrintCPUBackendASTVisitor::visit(BinaryOpNode&)
{
    std::cout << '(';
}

void PrintCPUBackendASTVisitor::midVisit(BinaryOpNode& node)
{
    char op = ' ';
    switch (node.op()) {
    case Add:
        op = '+';
        break;
    case Subtract:
        op = '-';
        break;
    case Multiply:
        op = '*';
        break;
    case Divide:
        op = '/';
        break;
    default:
        break;
    }
    std::cout << ' ' << op << ' ';
}

void PrintCPUBackendASTVisitor::postVisit(BinaryOpNode&)
{
    std::cout << ')';
}

void PrintCPUBackendASTVisitor::visit(NormNode&)
{
    std::cout << "er.norm(";
}

void PrintCPUBackendASTVisitor::postVisit(NormNode&)
{
    std::cout << ')';
}

void PrintCPUBackendASTVisitor::visit(UnaryNegationNode&)
{
    std::cout << '-';
}

void PrintCPUBackendASTVisitor::postVisit(UnaryNegationNode&)
{
}

void PrintCPUBackendASTVisitor::visit(OnNode&)
{
    std::cout << "er.operatorOn(";
}

void PrintCPUBackendASTVisitor::midVisit(OnNode& node)
{
    // Backend's operatorOn has three arguments when the left argument
    // is a collection, not two. The middle argument (that we will
    // write in this method) should be the set that the first argument
    // is On. Example:
    // a : Collection Of Scalar On InteriorFaces()
    // a On AllFaces() ===> er.operatorOn(a, InteriorFaces(), AllFaces()).
    std::cout << ", ";
    if (node.lefttype().isCollection()) {
        const std::string esname = SymbolTable::entitySetName(node.lefttype().gridMapping());
        // Now esname can be either a user-created named set or an Equelle built-in
        // function call such as AllCells(). If the second, we must transform to
        // proper call syntax for the C++ backend.
        const char first = esname[0];
        const std::string cppterm = std::isupper(first) ?
            std::string("er.") + char(std::tolower(first)) + esname.substr(1)
            : esname;
        std::cout << cppterm;
        std::cout << ", ";
    }
}

void PrintCPUBackendASTVisitor::postVisit(OnNode&)
{
    std::cout << ')';
}

void PrintCPUBackendASTVisitor::visit(TrinaryIfNode&)
{
    std::cout << "er.trinaryIf(";
}

void PrintCPUBackendASTVisitor::questionMarkVisit(TrinaryIfNode&)
{
    std::cout << ", ";
}

void PrintCPUBackendASTVisitor::colonVisit(TrinaryIfNode&)
{
    std::cout << ", ";
}

void PrintCPUBackendASTVisitor::postVisit(TrinaryIfNode&)
{
    std::cout << ')';
}

void PrintCPUBackendASTVisitor::visit(VarDeclNode&)
{
    suppress();
}

void PrintCPUBackendASTVisitor::postVisit(VarDeclNode&)
{
    unsuppress();
}

void PrintCPUBackendASTVisitor::visit(VarAssignNode& node)
{
    std::cout << indent() << "const " << cppTypeString(node.type()) << " " << node.name() << " = ";
}

void PrintCPUBackendASTVisitor::postVisit(VarAssignNode&)
{
    std::cout << ';';
    endl();
}

void PrintCPUBackendASTVisitor::visit(VarNode& node)
{
    std::cout << node.name();
}

void PrintCPUBackendASTVisitor::visit(FuncRefNode& node)
{
    if (in_function_call_requiring_ad_) {
        std::cout << "AD";
    }
    std::cout << node.name();
}

void PrintCPUBackendASTVisitor::visit(JustAnIdentifierNode&)
{
}

void PrintCPUBackendASTVisitor::visit(FuncArgsDeclNode&)
{
    std::cout << "{FuncArgsDeclNode::visit()}";
}

void PrintCPUBackendASTVisitor::midVisit(FuncArgsDeclNode&)
{
    std::cout << "{FuncArgsDeclNode::midVisit()}";
}

void PrintCPUBackendASTVisitor::postVisit(FuncArgsDeclNode&)
{
    std::cout << "{FuncArgsDeclNode::postVisit()}";
}

void PrintCPUBackendASTVisitor::visit(FuncDeclNode&)
{
    // std::cout << node.name() << " : ";
}

void PrintCPUBackendASTVisitor::postVisit(FuncDeclNode&)
{
    // endl();
}

void PrintCPUBackendASTVisitor::visit(FuncStartNode& node)
{
    std::cout << indent() << "auto ";
    if (emit_ad_) {
        std::cout << "AD";
    }
    std::cout << node.name() << " = [&](";
    const FunctionType& ft = SymbolTable::getFunction(node.name()).functionType();
    const size_t n = ft.arguments().size();
    for (int i = 0; i < n; ++i) {
        std::cout << "const "
                  << cppTypeString(ft.arguments()[i].type())
                  << "& " << ft.arguments()[i].name();
        if (i < n - 1) {
            std::cout << ", ";
        }
    }
    suppress();
    ++indent_;
}

void PrintCPUBackendASTVisitor::postVisit(FuncStartNode& node)
{
    unsuppress();
    const FunctionType& ft = SymbolTable::getFunction(node.name()).functionType();
    std::cout << ") -> " << cppTypeString(ft.returnType()) << " {";
    endl();
}

void PrintCPUBackendASTVisitor::visit(FuncAssignNode& node)
{
    if (!in_extra_funcassign_accept_) {
        in_extra_funcassign_accept_ = true;
        node.accept(*this);
        in_extra_funcassign_accept_ = false;
    }
}

void PrintCPUBackendASTVisitor::postVisit(FuncAssignNode&)
{
    emit_ad_ = !emit_ad_;
    --indent_;
    std::cout << indent() << "};";
    endl();
}

void PrintCPUBackendASTVisitor::visit(FuncArgsNode&)
{
}

void PrintCPUBackendASTVisitor::midVisit(FuncArgsNode&)
{
    if (!suppressed_) {
        std::cout << ", ";
    }
}

void PrintCPUBackendASTVisitor::postVisit(FuncArgsNode&)
{
}

void PrintCPUBackendASTVisitor::visit(ReturnStatementNode&)
{
    std::cout << indent() << "return ";
}

void PrintCPUBackendASTVisitor::postVisit(ReturnStatementNode&)
{
    std::cout << ';';
    endl();
}

void PrintCPUBackendASTVisitor::visit(FuncCallNode& node)
{
    const std::string fname = node.name();
    // The below is a hack, but it could be replaced by a more general
    // mechanism if necessary.
    if (fname == "NewtonSolve") {
        in_function_call_requiring_ad_ = true;
    }
    const char first = fname[0];
    std::string cppname;
    if (std::isupper(first)) {
        cppname += std::string("er.") + char(std::tolower(first)) + fname.substr(1);
    } else {
        if (emit_ad_) {
            cppname += "AD";
        }
        cppname += fname;
    }
    std::cout << cppname << '(';
}

void PrintCPUBackendASTVisitor::postVisit(FuncCallNode&)
{
    in_function_call_requiring_ad_ = false;
    std::cout << ')';
}

void PrintCPUBackendASTVisitor::visit(FuncCallStatementNode&)
{
    std::cout << indent();
}

void PrintCPUBackendASTVisitor::postVisit(FuncCallStatementNode&)
{
    std::cout << ';';
    endl();
}

void PrintCPUBackendASTVisitor::visit(LoopNode& node)
{
    BasicType loopvartype = SymbolTable::variableType(node.loopSet()).basicType();
    std::cout << indent() << "for (const " << cppTypeString(loopvartype) << "& "
              << node.loopVariable() << " : " << node.loopSet() << ") {";
    ++indent_;
    endl();
}

void PrintCPUBackendASTVisitor::postVisit(LoopNode&)
{
    --indent_;
    std::cout << indent() << "}";
    endl();
}



void PrintCPUBackendASTVisitor::endl() const
{
    std::cout << '\n';
}

std::string PrintCPUBackendASTVisitor::indent() const
{
    return std::string(indent_*4, ' ');
}

void PrintCPUBackendASTVisitor::suppress()
{
    suppressed_ = true;
}

void PrintCPUBackendASTVisitor::unsuppress()
{
    suppressed_ = false;
}

std::string PrintCPUBackendASTVisitor::cppTypeString(const EquelleType& et) const
{
    std::string cppstring = et.isCollection() ? "CollOf" : "";
    cppstring += et.isSequence() ? "SeqOf" : "";
    cppstring += basicTypeString(et.basicType());
    if (emit_ad_ && et.isCollection() && et.basicType() == Scalar) {
        cppstring += "AD";
    }
    return cppstring;
}



namespace
{
    const char* cppStartString()
    {
        return
"\n"
"// This program was created by the Equelle compiler from SINTEF.\n"
"\n"
"#include <opm/core/utility/parameters/ParameterGroup.hpp>\n"
"#include <opm/core/linalg/LinearSolverFactory.hpp>\n"
"#include <opm/core/utility/ErrorMacros.hpp>\n"
"#include <opm/autodiff/AutoDiffBlock.hpp>\n"
"#include <opm/autodiff/AutoDiffHelpers.hpp>\n"
"#include <opm/core/grid.h>\n"
"#include <opm/core/grid/GridManager.hpp>\n"
"#include <algorithm>\n"
"#include <iterator>\n"
"#include <iostream>\n"
"#include <cmath>\n"
"\n"
"#include \"EquelleRuntimeCPU.hpp\"\n"
"\n"
"int main(int argc, char** argv)\n"
"{\n"
"    // Get user parameters.\n"
"    Opm::parameter::ParameterGroup param(argc, argv, false);\n"
"\n"
"    // Create the Equelle runtime.\n"
"    EquelleRuntimeCPU er(param);\n"
"\n"
"    // ============= Generated code starts here ================\n";
    }

    const char* cppEndString()
    {
        return "\n"
"    // ============= Generated code ends here ================\n"
"\n"
"    return 0;\n"
"}\n";
    }
}