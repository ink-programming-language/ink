#pragma once

#include "text_io.h"

#include <utility>

namespace ink::ir::text
{
  inline constexpr EnumEntry<SymbolVisibility> Visibilities[] = {
      {SymbolVisibility::Private, "private"},
      {SymbolVisibility::Public, "public"},
  };

  inline constexpr EnumEntry<NativeLinkage> Linkages[] = {
      {NativeLinkage::None, "none"},
      {NativeLinkage::Import, "import"},
      {NativeLinkage::Export, "export"},
  };

  inline constexpr EnumEntry<MemberKind> MemberKinds[] = {
      {MemberKind::Instance, "instance"},
      {MemberKind::Static, "static"},
  };

  inline constexpr EnumEntry<Parameter::Kind> ParameterKinds[] = {
      {Parameter::Kind::Positional, "positional"},
      {Parameter::Kind::Named, "named"},
  };

  inline constexpr EnumEntry<ABIKind> ABIs[] = {
      {ABIKind::Ink, "ink"},
      {ABIKind::C, "c"},
  };

  inline constexpr EnumEntry<CallingConvention> Conventions[] = {
      {CallingConvention::Default, "default"},
      {CallingConvention::CDecl, "cdecl"},
      {CallingConvention::StdCall, "stdcall"},
      {CallingConvention::FastCall, "fastcall"},
      {CallingConvention::VectorCall, "vectorcall"},
  };

  inline constexpr EnumEntry<bool> VariadicKinds[] = {
      {false, "fixed"},
      {true, "variadic"},
  };

  inline constexpr EnumEntry<bool> RecordKinds[] = {
      {false, "struct"},
      {true, "class"},
  };

  inline constexpr EnumEntry<bool> FunctionKinds[] = {
      {false, "declare"},
      {true, "define"},
  };

  inline constexpr EnumEntry<InstructionKind> InstructionKinds[] = {
#define INSTRUCTION(Kind, Number, Class, Spelling) {InstructionKind::Kind, Spelling},
#include "ink/ir/instruction/instruction.def"
#undef INSTRUCTION
  };

  // The iteration mechanics differ by direction; every element still uses the same format function.
  template <typename IO, typename Node, typename Formatter, typename Stopper>
  bool sequence(IO &Io, std::vector<Node> &Nodes, Formatter Format, Stopper Stop)
  {
    if constexpr (IO::Reading)
    {
      while (!Stop())
      {
        const auto Start = Io.position();
        Node Element;
        if (!Format(Io, Element))
        {
          return false;
        }
        if (Io.position() == Start)
        {
          return Io.fail("list element did not consume input");
        }
        Nodes.push_back(std::move(Element));
      }
      return true;
    }
    else
    {
      for (auto &Element : Nodes)
      {
        if (!Format(Io, Element))
        {
          return false;
        }
      }
      return true;
    }
  }

  template <typename IO, typename Node, typename Formatter>
  bool commaList(IO &Io, std::vector<Node> &Nodes, std::string_view End, Formatter Format)
  {
    if constexpr (IO::Reading)
    {
      if (Io.peek(End))
      {
        return true;
      }
      for (;;)
      {
        const auto Start = Io.position();
        Node Element;
        if (!Format(Io, Element))
        {
          return false;
        }
        if (Io.position() == Start)
        {
          return Io.fail("list element did not consume input");
        }
        Nodes.push_back(std::move(Element));
        if (Io.peek(End))
        {
          return true;
        }
        if (!Io.token(","))
        {
          return false;
        }
      }
    }
    else
    {
      bool First = true;
      for (auto &Element : Nodes)
      {
        if ((!First && !Io.token(",")) || !Format(Io, Element))
        {
          return false;
        }
        First = false;
      }
      return true;
    }
  }

  template <typename IO>
  bool formatExpression(IO &Io, Expression &Node);

  template <typename IO>
  bool expressionChild(IO &Io, Expression &Node, std::size_t Index)
  {
    if constexpr (IO::Reading)
    {
      if (Node.Children.size() <= Index)
      {
        Node.Children.resize(Index + 1);
      }
    }
    if (Index >= Node.Children.size())
    {
      return Io.fail("missing type/value expression child");
    }
    return formatExpression(Io, Node.Children[Index]);
  }

  struct ParameterSyntax
  {
      Parameter::Kind Kind = Parameter::Kind::Positional;
      Expression Type;
      ArgumentSyntax Argument;
  };

  template <typename IO>
  bool formatParameter(IO &Io, ParameterSyntax &Node)
  {
    return Io.enumValue(Node.Kind, ParameterKinds) && formatExpression(Io, Node.Type);
  }

  template <typename IO>
  bool formatBoundParameter(IO &Io, ParameterSyntax &Node)
  {
    Io.offset(Node.Argument.Offset);
    return formatParameter(Io, Node) && Io.reference(Node.Argument.Id) && Io.token("name") && Io.quoted(Node.Argument.Name);
  }

  template <typename IO>
  bool formatParameters(IO &Io, Expression &Signature, std::vector<ArgumentSyntax> *Arguments)
  {
    std::vector<ParameterSyntax> Parameters;
    if constexpr (!IO::Reading)
    {
      if (Signature.Children.size() != Signature.ParameterKinds.size() + 1 || (Arguments && Arguments->size() != Signature.ParameterKinds.size()))
      {
        return Io.fail("signature parameter metadata and argument bindings differ");
      }
      for (std::size_t Index = 0; Index < Signature.ParameterKinds.size(); ++Index)
      {
        ParameterSyntax ParameterValue;
        ParameterValue.Kind = Signature.ParameterKinds[Index];
        ParameterValue.Type = Signature.Children[Index + 1];
        if (Arguments)
        {
          ParameterValue.Argument = (*Arguments)[Index];
        }
        Parameters.push_back(std::move(ParameterValue));
      }
    }
    const bool Result = Arguments ? commaList(Io, Parameters, ")", formatBoundParameter<IO>) : commaList(Io, Parameters, ")", formatParameter<IO>);
    if (!Result)
    {
      return false;
    }
    if constexpr (IO::Reading)
    {
      if (Signature.Children.empty())
      {
        Signature.Children.resize(1);
      }
      for (auto &ParameterValue : Parameters)
      {
        Signature.ParameterKinds.push_back(ParameterValue.Kind);
        Signature.Children.push_back(std::move(ParameterValue.Type));
        if (Arguments)
        {
          Arguments->push_back(std::move(ParameterValue.Argument));
        }
      }
    }
    return true;
  }

  template <typename IO>
  bool formatExpression(IO &Io, Expression &Node)
  {
    if (!Io.enterExpression())
    {
      return false;
    }
    struct DepthGuard
    {
        IO &Io;

        ~DepthGuard()
        {
          Io.leaveExpression();
        }
    } Guard{Io};
    if (!Io.expressionKind(Node))
    {
      return false;
    }
    switch (Node.Kind)
    {
    case ExpressionKind::Reference:
    case ExpressionKind::MetaType:
    case ExpressionKind::BoolType:
    case ExpressionKind::VoidType:
    case ExpressionKind::IntegerType:
    case ExpressionKind::FloatType:
      return true;
    case ExpressionKind::PointerType:
      return Io.token("<") && expressionChild(Io, Node, 0) && Io.token(">");
    case ExpressionKind::ArrayType:
      return Io.number(Node.Count) && Io.token("x") && expressionChild(Io, Node, 0) && Io.token("]");
    case ExpressionKind::FunctionType:
      return Io.token("<") && Io.enumValue(Node.ABI.ABI, ABIs) && Io.enumValue(Node.ABI.Convention, Conventions) && Io.enumValue(Node.Variadic, VariadicKinds) && Io.token(">") && Io.token("(") && formatParameters(Io, Node, nullptr) && Io.token(")") && Io.token("->") && expressionChild(Io, Node, 0);
    case ExpressionKind::IntegerConstant:
    case ExpressionKind::FloatConstant:
      return Io.token("<") && expressionChild(Io, Node, 0) && Io.token(">") && Io.token("(") && Io.number(Node.Bits) && Io.token(")");
    case ExpressionKind::StringConstant:
      return Io.token("(") && Io.quoted(Node.Text) && Io.token(")");
    }
    return Io.fail("unknown type/value expression");
  }

  template <typename IO>
  bool expressionOperands(IO &Io, InstructionSyntax &Node, std::size_t Count)
  {
    if constexpr (IO::Reading)
    {
      Node.Operands.resize(Count);
    }
    if (Node.Operands.size() != Count)
    {
      return Io.fail("incorrect instruction operand count");
    }
    for (std::size_t Index = 0; Index < Count; ++Index)
    {
      if ((Index && !Io.token(",")) || !formatExpression(Io, Node.Operands[Index]))
      {
        return false;
      }
    }
    return true;
  }

  template <typename IO>
  bool target(IO &Io, InstructionSyntax &Node, std::size_t Index)
  {
    if constexpr (IO::Reading)
    {
      if (Node.Targets.size() <= Index)
      {
        Node.Targets.resize(Index + 1);
      }
    }
    return Index < Node.Targets.size() ? Io.reference(Node.Targets[Index]) : Io.fail("missing branch target");
  }

  template <typename IO>
  bool formatIncoming(IO &Io, IncomingSyntax &Node)
  {
    Io.offset(Node.Offset);
    return Io.token("[") && formatExpression(Io, Node.Value) && Io.token(",") && Io.reference(Node.Block) && Io.token("]");
  }

  template <typename IO>
  bool formatInstruction(IO &Io, InstructionSyntax &Node)
  {
    Io.offset(Node.Offset);
    if (!Io.resultId(Node.Id) || !Io.enumValue(Node.Kind, InstructionKinds))
    {
      return false;
    }
    const bool HasResult = Node.Kind == InstructionKind::Add || Node.Kind == InstructionKind::Alloca || Node.Kind == InstructionKind::Load || Node.Kind == InstructionKind::Phi;
    if (HasResult != !Node.Id.empty())
    {
      return Io.fail("only value-producing instructions require a result identifier");
    }
    bool Result = false;
    switch (Node.Kind)
    {
    case InstructionKind::Add:
    case InstructionKind::Store:
      Result = expressionOperands(Io, Node, 2);
      break;
    case InstructionKind::Alloca:
    case InstructionKind::Load:
      Result = expressionOperands(Io, Node, 1);
      break;
    case InstructionKind::Return:
    {
      bool HasOperand = !Node.Operands.empty();
      if constexpr (IO::Reading)
      {
        HasOperand = !Io.peek(";");
      }
      Result = expressionOperands(Io, Node, HasOperand ? 1 : 0);
      break;
    }
    case InstructionKind::Branch:
      Result = target(Io, Node, 0);
      break;
    case InstructionKind::CondBranch:
      Result = expressionOperands(Io, Node, 1) && Io.token(",") && target(Io, Node, 0) && Io.token(",") && target(Io, Node, 1);
      break;
    case InstructionKind::Phi:
      Result = commaList(Io, Node.Incoming, ":", formatIncoming<IO>);
      break;
    }
    if (!Result)
    {
      return Io.fail("instruction format or operands are invalid");
    }
    if (HasResult)
    {
      if (!Io.token(":") || !formatExpression(Io, Node.ResultType))
      {
        return false;
      }
    }
    else if constexpr (IO::Reading)
    {
      Node.ResultType.Kind = ExpressionKind::VoidType;
      Node.ResultType.Offset = Node.Offset;
    }
    return Io.token(";") && Io.newline();
  }

  template <typename IO>
  bool formatBlock(IO &Io, BlockSyntax &Node)
  {
    Io.offset(Node.Offset);
    if (!Io.reference(Node.Id) || !Io.token("name") || !Io.quoted(Node.Name) || !Io.token(":") || !Io.newline() || !Io.indent())
    {
      return false;
    }
    return sequence(Io, Node.Instructions, formatInstruction<IO>, [&]()
                    {
                      if constexpr (IO::Reading)
                      {
                        return Io.atBlock() || Io.peek("}");
                      }
                      else
                      {
                        return false;
                      }
                    }) &&
           Io.unindent();
  }

  template <typename IO>
  bool formatFunction(IO &Io, FunctionSyntax &Node)
  {
    Io.offset(Node.Offset);
    bool HasBody = !Node.Blocks.empty();
    if constexpr (IO::Reading)
    {
      Node.Signature.Kind = ExpressionKind::FunctionType;
      Node.Signature.Offset = Node.Offset;
    }
    if (!Io.enumValue(HasBody, FunctionKinds) || !Io.enumValue(Node.Visibility, Visibilities) || !Io.enumValue(Node.Linkage, Linkages) || !expressionChild(Io, Node.Signature, 0) || !Io.reference(Node.Id) || !Io.token("name") || !Io.quoted(Node.Name) || !Io.token("(") || !formatParameters(Io, Node.Signature, &Node.Arguments) || !Io.token(")") || !Io.token("abi") || !Io.enumValue(Node.Signature.ABI.ABI, ABIs) || !Io.token("cc") || !Io.enumValue(Node.Signature.ABI.Convention, Conventions) || !Io.enumValue(Node.Signature.Variadic, VariadicKinds))
    {
      return false;
    }
    if (!HasBody)
    {
      return Io.token(";") && Io.newline();
    }
    if (!Io.beginRegion())
    {
      return false;
    }
    if (!sequence(Io, Node.Blocks, formatBlock<IO>, [&]()
                  {
                    if constexpr (IO::Reading)
                    {
                      return Io.peek("}");
                    }
                    else
                    {
                      return false;
                    }
                  }))
    {
      return false;
    }
    if (Node.Blocks.empty())
    {
      return Io.fail("a function definition must contain a basic block");
    }
    return Io.endRegion();
  }

  template <typename IO>
  bool formatField(IO &Io, FieldSyntax &Node)
  {
    Io.offset(Node.Offset);
    return Io.token("field") && Io.enumValue(Node.Visibility, Visibilities) && Io.enumValue(Node.Kind, MemberKinds) && Io.quoted(Node.Name) && Io.token(":") && formatExpression(Io, Node.Type) && Io.token(";") && Io.newline();
  }

  template <typename IO>
  bool formatMethod(IO &Io, MethodSyntax &Node)
  {
    Io.offset(Node.Offset);
    return Io.token("method") && Io.enumValue(Node.Kind, MemberKinds) && formatFunction(Io, Node.Function);
  }

  template <typename IO>
  bool formatRecord(IO &Io, RecordSyntax &Node)
  {
    Io.offset(Node.Offset);
    if (!Io.reference(Node.Id) || !Io.token("=") || !Io.token("type") || !Io.enumValue(Node.IsObject, RecordKinds))
    {
      return false;
    }
    bool Borrowed = !Node.Registered;
    bool HasBases = !Node.Bases.empty();
    if constexpr (IO::Reading)
    {
      Borrowed = Io.peek("borrowed");
    }
    if (Borrowed && !Io.token("borrowed"))
    {
      return false;
    }
    Node.Registered = !Borrowed;
    if constexpr (IO::Reading)
    {
      HasBases = Io.peek("extends");
    }
    if (HasBases && (!Io.token("extends") || !Io.token("(") || !commaList(Io, Node.Bases, ")", formatExpression<IO>) || !Io.token(")")))
    {
      return false;
    }
    if (!Io.beginRegion() || !sequence(Io, Node.Fields, formatField<IO>, [&]()
                                       {
                                         if constexpr (IO::Reading)
                                         {
                                           return !Io.peek("field");
                                         }
                                         else
                                         {
                                           return false;
                                         }
                                       }) ||
        !sequence(Io, Node.Methods, formatMethod<IO>, [&]()
                  {
                    if constexpr (IO::Reading)
                    {
                      return !Io.peek("method");
                    }
                    else
                    {
                      return false;
                    }
                  }))
    {
      return false;
    }
    return Io.endRegion();
  }

  template <typename IO>
  bool formatGlobal(IO &Io, GlobalSyntax &Node)
  {
    Io.offset(Node.Offset);
    return Io.token("global") && Io.enumValue(Node.Visibility, Visibilities) && Io.enumValue(Node.Linkage, Linkages) && Io.reference(Node.Id) && Io.token("name") && Io.quoted(Node.Name) && Io.token(":") && formatExpression(Io, Node.Type) && Io.token(";") && Io.newline();
  }

  template <typename IO>
  bool formatModule(IO &Io, ModuleSyntax &Node)
  {
    if (!Io.token("module") || !Io.quoted(Node.Name) || !Io.beginRegion())
    {
      return false;
    }
    if constexpr (IO::Reading)
    {
      while (!Io.peek("}"))
      {
        if (Io.peek("global"))
        {
          GlobalSyntax Global;
          if (!formatGlobal(Io, Global))
          {
            return false;
          }
          Node.Globals.push_back(std::move(Global));
        }
        else if (Io.peek("define") || Io.peek("declare"))
        {
          FunctionSyntax FunctionValue;
          if (!formatFunction(Io, FunctionValue))
          {
            return false;
          }
          Node.Functions.push_back(std::move(FunctionValue));
        }
        else
        {
          RecordSyntax Record;
          if (!formatRecord(Io, Record))
          {
            return false;
          }
          Node.Records.push_back(std::move(Record));
        }
      }
    }
    else
    {
      for (auto &Record : Node.Records)
      {
        if (!formatRecord(Io, Record))
        {
          return false;
        }
      }
      for (auto &Global : Node.Globals)
      {
        if (!formatGlobal(Io, Global))
        {
          return false;
        }
      }
      for (auto &FunctionValue : Node.Functions)
      {
        if (!formatFunction(Io, FunctionValue))
        {
          return false;
        }
      }
    }
    return Io.endRegion();
  }
} // namespace ink::ir::text
