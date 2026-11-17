#include "ink/ir/function/function.h"

#include "ink/core/support/core_define.h"
#include "ink/ir/verification/verifier.h"

namespace ink::ir
{
    void Function::initializeArguments() noexcept
    {
        checkMsgf(FunctionType::classof(&getType()), "Function requires a FunctionType signature");
        const auto Parameters = signature().parameters();
        Arguments.reserve(Parameters.size());
        for (std::size_t Index = 0; Index < Parameters.size(); ++Index)
        {
            Arguments.push_back(std::unique_ptr<Argument>(new Argument(Parameters[Index].type(), *this, Index)));
        }
    }

    Argument& Function::argumentAt(std::size_t Index) const noexcept
    {
        checkMsgf(Index < Arguments.size(), "Function argument index is out of range");
        return *Arguments[Index];
    }

    bool Function::hasValidControlFlow() const noexcept
    {
        for (const BasicBlock& Block : basicBlocks())
        {
            const Instruction* Last = Block.terminator();
            if (!Last)
            {
                return false;
            }
            for (const Instruction& Node : Block.instructions())
            {
                if (Node.isTerminator() && &Node != Last)
                {
                    return false;
                }
            }
            for (const BasicBlock& Target : Block.successors())
            {
                if (Target.parent() != this)
                {
                    return false;
                }
            }
        }
        return true;
    }

    void Function::verify(const core::DiagnosticEngine& Diagnostics) const
    {
        verifyFunction(*this, Diagnostics);
    }

    bool Function::verify(std::string* Error) const
    {
        return verifyFunction(*this, Error);
    }
} // namespace ink::ir
