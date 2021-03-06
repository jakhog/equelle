/*
  Copyright 2013 SINTEF ICT, Applied Mathematics.
*/

#include "Common.hpp"
#include "EquelleType.hpp"
#include "SymbolTable.hpp"
#include "ASTNodes.hpp"
#include "ParseActions.hpp"
#include <sstream>
#include <iostream>



// ------ Parsing event handlers ------


SequenceNode* handleProgram(SequenceNode* lineblocknode)
{
    SymbolTable::setProgram(lineblocknode);
    return lineblocknode;
}



Node* handleNumber(const double num)
{
    return new NumberNode(num);
}



Node* handleIdentifier(const std::string& name)
{
    if (SymbolTable::isVariableDeclared(name)) {
        return new VarNode(name);
    } else {
        if (SymbolTable::isFunctionDeclared(name)) {
            return new FuncRefNode(name);
        } else {
            // This is a small problem: we want the error below, to catch
            // usage of undeclared identifiers, but the function start section
            // would then generate errors, because we are not yet in the function
            // scope.

            // std::string e("unknown identifier ");
            // e += name;
            // yyerror(e.c_str());
            return new JustAnIdentifierNode(name);
        }
    }
}



VarDeclNode* handleDeclaration(const std::string& name, TypeNode* type)
{
    EquelleType t = type->type();
    SymbolTable::declareVariable(name, t);
    return new VarDeclNode(name, type);
}



VarAssignNode* handleAssignment(const std::string& name, Node* expr)
{
    // If already declared...
    if (SymbolTable::isVariableDeclared(name)) {
        // Check if already assigned.
        if (SymbolTable::isVariableAssigned(name) && !SymbolTable::variableType(name).isMutable()) {
            std::string err_msg = "variable already assigned, cannot re-assign ";
            err_msg += name;
            yyerror(err_msg.c_str());
            return nullptr;
        }
        // Check that declared type matches right hand side.
        EquelleType lhs_type = SymbolTable::variableType(name);
        EquelleType rhs_type = expr->type();
        if (lhs_type != rhs_type) {
            // Check for special case: variable declared to have type
            // 'Collection Of <Entity> Subset Of <Some entityset>',
            // actual setting its grid mapping is postponed until the entityset
            // has been created by a function call.
            // That means we have to set the actual grid mapping here.
            if (lhs_type.gridMapping() == PostponedDefinition
                && lhs_type.basicType() == rhs_type.basicType()
                && lhs_type.isCollection() && rhs_type.isCollection()
                && rhs_type.isDomain()
                && SymbolTable::isSubset(rhs_type.gridMapping(), lhs_type.subsetOf())) {
                // OK, should make postponed definition of the variable.
                SymbolTable::setVariableType(name, rhs_type);
                SymbolTable::setEntitySetName(rhs_type.gridMapping(), name);
            } else {
                std::string err_msg = "mismatch between type in assignment and declaration for ";
                err_msg += name;
                yyerror(err_msg.c_str());
                return nullptr;
            }
        }
    } else {
        // Setting the gridmapping, as in the block above. Only this is
        // the direct (no previous declaration) assignment scenario.
        const int gm = expr->type().gridMapping();
        if (gm != NotApplicable && SymbolTable::entitySetName(gm) == "AnonymousEntitySet") {
            SymbolTable::setEntitySetName(gm, name);
        }
        SymbolTable::declareVariable(name, expr->type());
    }

    // Set variable to assigned (unless mutable) and return.
    if (!SymbolTable::variableType(name).isMutable()) {
        SymbolTable::setVariableAssigned(name, true);
    }
    return new VarAssignNode(name, expr);
}



Node* handleFuncDeclaration(const std::string& name, FuncTypeNode* ftype)
{
    SymbolTable::renameCurrentFunction(name);
    SymbolTable::retypeCurrentFunction(ftype->funcType());
    SymbolTable::setCurrentFunction(SymbolTable::getCurrentFunction().parentScope());
    return new FuncDeclNode(name, ftype);
}



Node* handleFuncStart(const std::string& name, Node* funcargs)
{
    SymbolTable::setCurrentFunction(name);
    return new FuncStartNode(name, funcargs);
}



void handleFuncStartType()
{
    SymbolTable::declareFunction("TemporaryFunction");
    SymbolTable::setCurrentFunction("TemporaryFunction");
}



SequenceNode* handleBlock(SequenceNode* block)
{
    // This is called after the block AST has been constructed,
    // so we should switch back to Main scope.
    SymbolTable::setCurrentFunction(SymbolTable::getCurrentFunction().parentScope());
    return block;
}



FuncAssignNode* handleFuncAssignment(Node* funcstart, SequenceNode* fbody)
{
    return new FuncAssignNode(funcstart, fbody);
}



ReturnStatementNode* handleReturnStatement(Node* expr)
{
    return new ReturnStatementNode(expr);
}



Node* handleDeclarationAssign(const std::string& name, TypeNode* type, Node* expr)
{
    SequenceNode* seq = new SequenceNode;
    seq->pushNode(handleDeclaration(name, type));
    seq->pushNode(handleAssignment(name, expr));
    return seq;
}



TypeNode* handleCollection(TypeNode* btype, Node* gridmapping, Node* subsetof)
{
    assert(gridmapping == nullptr || subsetof == nullptr);
    EquelleType bt = btype->type();
    if (!bt.isBasic()) {
        std::string errmsg = "attempting to declare a Collection Of <nonsense>";
        yyerror(errmsg.c_str());
    }
    int gm = NotApplicable;
    if (gridmapping) {
        if (!gridmapping->type().isEntityCollection() || gridmapping->type().gridMapping() == NotApplicable) {
            yyerror("a Collection must be On a Collection of Cell, Face etc.");
        } else {
            gm = gridmapping->type().gridMapping();
        }
    }
    int subset = NotApplicable;
    if (subsetof) {
        // We are creating a new entity collection.
        if (!subsetof->type().isEntityCollection() || subsetof->type().gridMapping() == NotApplicable) {
            yyerror("a Collection must be Subset Of a Collection of Cell, Face etc.");
        } else {
            gm = PostponedDefinition;
            subset = subsetof->type().gridMapping();
        }
    }
    delete btype;
    delete gridmapping;
    delete subsetof;
    return new TypeNode(EquelleType(bt.basicType(), Collection, gm, subset));
}



FuncTypeNode* handleFuncType(FuncArgsDeclNode* argtypes, TypeNode* rtype)
{
    return new FuncTypeNode(FunctionType(argtypes->arguments(), rtype->type()));
}



FuncCallNode* handleFuncCall(const std::string& name, FuncArgsNode* args)
{
    const Function& f = SymbolTable::getFunction(name);
    // Check function call arguments.
    const auto argtypes = args->argumentTypes();
    if (argtypes.size() != f.functionType().arguments().size()) {
        std::string err_msg = "wrong number of arguments when calling function ";
        err_msg += name;
        yyerror(err_msg.c_str());
    }
    // At the moment, we do not check function argument types.
    // If the function returns a new dynamically created domain,
    // we must declare it (anonymously for now).
    const EquelleType rtype = f.returnType(argtypes);
    if (rtype.isDomain()) {
        const int dynsubret = f.functionType().dynamicSubsetReturn(argtypes);
        if (dynsubret != NotApplicable) {
            // Create a new domain.
            const int gm = SymbolTable::declareNewEntitySet("AnonymousEntitySet", dynsubret);
            return new FuncCallNode(name, args, gm);
        }
    }
    return new FuncCallNode(name, args);
}



FuncCallStatementNode* handleFuncCallStatement(FuncCallNode* fcall)
{
    return new FuncCallStatementNode(fcall);
}



BinaryOpNode* handleBinaryOp(BinaryOp op, Node* left, Node* right)
{
    EquelleType lt = left->type();
    EquelleType rt = right->type();
    if (!isNumericType(lt.basicType()) || !(isNumericType(rt.basicType()))) {
        yyerror("arithmetic binary operators only apply to numeric types");
    }
    if (lt.isArray() || rt.isArray()) {
        yyerror("arithmetic binary operators cannot be applied to Array types");
    }
    if (lt.isCollection() && rt.isCollection()) {
        if (lt.gridMapping() != rt.gridMapping()) {
            yyerror("arithmetic binary operators on Collections only acceptable "
                    "if both sides are On the same set.");
        }
    }
    switch (op) {
    case Add:
        // Intentional fall-through.
    case Subtract:
        if (lt != rt) {
        	if ((lt.basicType() == StencilI || lt.basicType() == StencilJ || lt.basicType() == StencilK)
        			&& rt.basicType() == Scalar) {
        		//i,j,k OP n is OK
        	}
        	else if (lt.basicType() == Scalar &&
        			(rt.basicType() == StencilI || rt.basicType() == StencilJ || rt.basicType() == StencilK)) {
        		//n OP i,j,k is OK
        	}
        	else {
        		yyerror("addition and subtraction only allowed between identical types.");
        	}
        }
        break;
    case Multiply:
        if (lt.basicType() == Vector && rt.basicType() == Vector) {
            yyerror("cannot multiply two 'Vector' types.");
        }
        break;
    case Divide:
        if (rt.basicType() != Scalar) {
            yyerror("can only divide by 'Scalar' types");
        }
        break;
    default:
        yyerror("internal compiler error in handleBinaryOp().");
    }
    return new BinaryOpNode(op, left, right);
}



ComparisonOpNode* handleComparison(ComparisonOp op, Node* left, Node* right)
{
    EquelleType lt = left->type();
    EquelleType rt = right->type();
    if ((lt.basicType() != Scalar) || (rt.basicType() != Scalar)) {
        yyerror("comparison operators can only be applied to scalars");
    }
    if (lt.isArray() || rt.isArray()) {
        yyerror("comparison operators cannot be applied to Array types");
    }
    if (lt.isCollection() && rt.isCollection()) {
        if (lt.gridMapping() != rt.gridMapping()) {
            yyerror("comparison operators on Collections only acceptable "
                    "if both sides are On the same set.");
        }
    }
    return new ComparisonOpNode(op, left, right);
}



NormNode* handleNorm(Node* expr_to_norm)
{
    if (expr_to_norm->type().isArray()) {
        yyerror("cannot take norm of an Array.");
    }
    const BasicType bt = expr_to_norm->type().basicType();
    if (isEntityType(bt) || bt == Scalar || bt == Vector) {
        return new NormNode(expr_to_norm);
    } else {
        yyerror("can only take norm of Scalar, Vector, Cell, Face, Edge and Vertex types.");
        return new NormNode(expr_to_norm);
    }
}



UnaryNegationNode* handleUnaryNegation(Node* expr_to_negate)
{
    if (!isNumericType(expr_to_negate->type().basicType())) {
        yyerror("unary minus can only be applied to numeric types.");
    }
    if (expr_to_negate->type().isArray()) {
        yyerror("unary minus cannot be applied to an Array.");
    }
    return new UnaryNegationNode(expr_to_negate);
}



TrinaryIfNode* handleTrinaryIf(Node* predicate, Node* iftrue, Node* iffalse)
{
    const EquelleType pt = predicate->type();
    const EquelleType tt = iftrue->type();
    const EquelleType ft = iffalse->type();
    if (pt.isArray() || tt.isArray() || ft.isArray()) {
        yyerror("in trinary if operator, no operands can be of Array type.");
    }
    if (pt.basicType() != Bool) {
        yyerror("in trinary if '<predicate> ? <iftrue> : <iffalse>' "
                "<predicate> must be a Bool type.");
    }
    if (tt != ft) {
        yyerror("in trinary if '<predicate> ? <iftrue> : <iffalse>' "
                "<iftrue> and <iffalse> must have the same type.");
    }
    if ((pt.isCollection() != tt.isCollection()) ||
        (pt.gridMapping() != tt.gridMapping())) {
        yyerror("in trinary if '<predicate> ? <iftrue> : <iffalse>' "
                "all three expressions must be 'On' the same set.");
    }
    return new TrinaryIfNode(predicate, iftrue, iffalse);
}



OnNode* handleOn(Node* left, Node* right)
{
    const EquelleType lt = left->type();
    const EquelleType rt = right->type();
    if (lt.isArray() || rt.isArray()) {
        yyerror("cannot use On operator with an Array.");
    }
    // Left side can be anything but a sequence.
    if (lt.isSequence()) {
        yyerror("cannot use On operator with a Sequence.");
    }
    // Right side must be an entity collection.
    if (!rt.isEntityCollection()) {
        yyerror("in a '<left> On <right>' expression "
                "the expression <right> must be a Collection Of Cell, Face, Edge or Vertex.");
    }
    // Left side must be some collection.
    if (!lt.isCollection()) {
        yyerror("in a '<left> On <right>' expression "
                "the expression <left> must be a Collection.");
    } else {
        // The domain (grid mapping) of the left side must contain
        // the right hand side. Explanation by example:
        //   x = AllCells()
        //   y : Collection Of Scalar On x = |x|
        // The above defines x and y, and the On part declares that there is a 1-1 mapping x -> y.
        // We can even denote this mapping y(x).
        //   z = InteriorFaces()
        //   w : Collection Of Cell On z = FirstCell(z)
        // Now w is On z, meaning that there is a 1-1 mapping z -> w, we denote it w(z)
        // Finally, what does then the following mean?
        //   yow = y On w
        // Its meaning is intended to be a composition of mappings, that is
        // there is now a 1-1 mapping z->yow defined by yow(z) = y(w(z)).
        // This is only ok if the range of w(z) is contained in the domain of y(x), that is x.
        // In our case that means that the entity collection on the right hand side must be contained
        // in the domain of the left.
        const int left_domain = lt.gridMapping();
        const int right_collection = rt.subsetOf();
        if (!SymbolTable::isSubset(right_collection, left_domain)) {
            std::string err_msg;
            err_msg += "in a '<left> On <right>' expression the expression <right> must "
                "be a collection that is contained in the domain that <left> is On. ";
            err_msg += "Collection on the left is On ";
            err_msg += SymbolTable::entitySetName(left_domain);
            yyerror(err_msg.c_str());
        }
    }
    return new OnNode(left, right, false);
}



OnNode* handleExtend(Node* left, Node* right)
{
    const EquelleType lt = left->type();
    const EquelleType rt = right->type();
    if (lt.isArray() || rt.isArray()) {
        yyerror("cannot use Extend operator with an Array.");
    }
    // Left side can be anything but a sequence.
    if (lt.isSequence()) {
        yyerror("cannot use Extend operator with a Sequence.");
    }
    // Right side must be a domain.
    if (!rt.isDomain()) {
        yyerror("in a '<left> Extend <right>' expression "
                "the expression <right> must be a Collection Of Cell, Face, Edge or Vertex, "
                "that also is a domain (all unique, non-Empty elements).");
    }
    // If left side is a collection, its domain (grid mapping) must be
    // a subset of the right hand side.
    if (lt.isCollection()) {
        const int left_domain = lt.gridMapping();
        const int right_domain = lt.gridMapping();
        if (!SymbolTable::isSubset(left_domain, right_domain)) {
            std::string err_msg;
            err_msg += "in a '<left> Extend <right>' expression the expression <right> must "
                "be a domain that contains the domain that <left> is On. ";
            err_msg += "Collection on the left is On ";
            err_msg += SymbolTable::entitySetName(left_domain);
            err_msg += " and Domain on the right is On ";
            err_msg += SymbolTable::entitySetName(right_domain);
            yyerror(err_msg.c_str());
        }
    }
    return new OnNode(left, right, true);
}



StringNode* handleString(const std::string& content)
{
    return new StringNode(content);
}



TypeNode* handleMutableType(TypeNode* type_expr)
{
    EquelleType et = type_expr->type();
    et.setMutable(true);
    TypeNode* tn = new TypeNode(et);
    delete type_expr;
    return tn;
}



TypeNode* handleSequence(TypeNode* basic_type)
{
    const EquelleType et = basic_type->type();
    if (!et.isBasic()) {
        yyerror("cannot create a Sequence of non-basic types.");
    }
    return new TypeNode(EquelleType(et.basicType(), Sequence, et.gridMapping(), et.subsetOf()));
}



TypeNode* handleArrayType(const int array_size, TypeNode* type_expr)
{
    EquelleType et = type_expr->type();
    if (et.isArray()) {
        yyerror("cannot create an Array of an Array.");
        return type_expr;
    } else {
        et.setArraySize(array_size);
        TypeNode* tn = new TypeNode(et);
        delete type_expr;
        return tn;
    }
}



ArrayNode* handleArray(FuncArgsNode* expr_list)
{
    const auto& elems = expr_list->arguments();
    if (elems.empty()) {
        yyerror("cannot create an empty array.");
    } else {
        const EquelleType et = elems[0]->type();
        if (et.isArray()) {
            yyerror("an Array cannot contain another Array.");
        }
        for (const auto& elem : elems) {
            if (elem->type() != et) {
                yyerror("elements of an Array must all have the same type");
            }
        }
    }
    return new ArrayNode(expr_list);
}



LoopNode* handleLoopStart(const std::string& loop_variable, const std::string& loop_set)
{
    // Check that loop_set is a sequence, extract its type.
    EquelleType loop_set_type;
    if (SymbolTable::isVariableDeclared(loop_set)) {
        loop_set_type = SymbolTable::variableType(loop_set);
        if (!loop_set_type.isSequence()) {
            std::string err_msg = "loop set must be a Sequence: ";
            err_msg += loop_set;
            yyerror(err_msg.c_str());
        }
        if (loop_set_type.isArray()) {
            std::string err_msg = "loop set cannot be an Array: ";
            err_msg += loop_set;
            yyerror(err_msg.c_str());
        }
    } else {
        std::string err_msg = "unknown variable used: ";
        err_msg += loop_set;
        yyerror(err_msg.c_str());
    }
    // Create LoopNode
    LoopNode* ln = new LoopNode(loop_variable, loop_set);
    // Create a name for the loop scope.
    static int next_loop_index = 0;
    std::ostringstream os;
    os << "ForLoopWithIndex" << next_loop_index++;
    // Set name in loop node, declare scope and
    // set to current.
    ln->setName(os.str());
    SymbolTable::declareFunction(os.str());
    SymbolTable::setCurrentFunction(os.str());
    // Declare loop variable
    SymbolTable::declareVariable(loop_variable, loop_set_type.basicType());
    return ln;
}



LoopNode* handleLoopStatement(LoopNode* loop_start, SequenceNode* loop_block)
{
    loop_start->setBlock(loop_block);
    return loop_start;
}



RandomAccessNode* handleRandomAccess(Node* expr, const int index)
{
    if (expr->type().isArray()) {
        if (index < 0 || index >= expr->type().arraySize()) {
            yyerror("index out of array bounds in '[<index>]' random access operator.");
        }
    } else if (expr->type().basicType() == Vector) {
        if (index < 0 || index > 2) {
            yyerror("cannot use '[<index>]' random access operator on a Vector with index < 0 or > 2");
        }
    } else {
        yyerror("cannot use '[<index>]' random access operator with anything other than a Vector or Array");
    }
    return new RandomAccessNode(expr, index);
}



StencilAccessNode *handleStencilAccess(const std::string grid_variable,
                                       FuncArgsNode* expr_list)
{
    return new StencilAccessNode( grid_variable, expr_list );
}


SequenceNode *handleStencilStatement( StencilAccessNode *lhsStencilAccess,
                                              Node *expr)
{
    SequenceNode* seq = new SequenceNode;
    TypeNode* type = new TypeNode(EquelleType(Scalar));
    /**FIXME: Need stencilDeclnode here.........
    Right now it ends up with
    const Scalar u = auto cell_stencil = [&] (int i, int j) { ...
    instead of the more sensible
    equelle::CartesianGrid::CartesianCollectionOfScalar u = grid.inputCellScalarWithDefault( "u", 1.0 );
    auto cell_stencil = [&] (int i, int j) { ...
    */
    StencilStatementNode* stencil = new StencilStatementNode( lhsStencilAccess, expr );
    seq->pushNode(handleDeclaration(lhsStencilAccess->name(), type));
    seq->pushNode(handleAssignment(lhsStencilAccess->name(), stencil));
    return seq;
}
