#include "ink/semantic/model/type/type_pool.h"
#include "ink/semantic/context.h"
#include "ink/semantic/ir_builder.h"

#include <gtest/gtest.h>

#include <concepts>
#include <type_traits>
#include <utility>

namespace ink::semantic::test
{
  static_assert(!std::is_copy_constructible_v<TypePool>);
  static_assert(!std::is_copy_assignable_v<TypePool>);
  static_assert(!std::is_move_constructible_v<TypePool>);
  static_assert(!std::is_move_assignable_v<TypePool>);
  static_assert(!std::is_constructible_v<TypePool, SemanticContext &>);

  template <typename Provider, TypeKind Kind, typename... Arguments>
  concept CanGetType = requires(Provider &Owner, Arguments &&...Args)
  {
    Owner.template getType<Kind>(std::forward<Arguments>(Args)...);
  };

  // Pool factories preserve exact return types and reject unsupported kinds, signatures and const factories.
  template <typename Provider>
  constexpr bool hasConstrainedTypeFactories()
  {
    static_assert(requires(Provider &Owner, const Provider &ConstOwner, const Type &Component, std::span<const Type *const> Parameters)
    {
      { ConstOwner.template getType<TypeKind::Meta>() } noexcept -> std::same_as<const BuiltinType &>;
      { ConstOwner.template getType<TypeKind::Void>() } noexcept -> std::same_as<const BuiltinType &>;
      { ConstOwner.template getType<TypeKind::Bool>() } noexcept -> std::same_as<const BuiltinType &>;
      { ConstOwner.template getType<TypeKind::Label>() } noexcept -> std::same_as<const BuiltinType &>;
      { ConstOwner.template getType<TypeKind::Module>() } noexcept -> std::same_as<const BuiltinType &>;
      { Owner.template getType<TypeKind::Integer>(32, true) } -> std::same_as<const IntegerType *>;
      { Owner.template getType<TypeKind::Float>(64) } -> std::same_as<const FloatType *>;
      { Owner.template getType<TypeKind::Array>(Component, 4) } -> std::same_as<const ArrayType *>;
      { Owner.template getType<TypeKind::Slice>(Component, AccessKind::ReadOnly) } -> std::same_as<const SliceType *>;
      { Owner.template getType<TypeKind::Pointer>(Component, AccessKind::ReadWrite) } -> std::same_as<const PointerType *>;
      { Owner.template getType<TypeKind::Reference>(Component, AccessKind::ReadOnly) } -> std::same_as<const ReferenceType *>;
      { Owner.template getType<TypeKind::Function>(Component, Parameters) } -> std::same_as<const FunctionType *>;
      { Owner.template getType<TypeKind::Function>(Component) } -> std::same_as<const FunctionType *>;
      { Owner.template getType<TypeKind::Function>(Component, {}) } -> std::same_as<const FunctionType *>;
    });
    static_assert(!CanGetType<Provider, TypeKind::Bool, int>);
    static_assert(!CanGetType<Provider, TypeKind::Integer>);
    static_assert(!CanGetType<Provider, TypeKind::Integer, std::uint32_t>);
    static_assert(!CanGetType<Provider, TypeKind::Float, std::uint32_t, bool>);
    static_assert(!CanGetType<Provider, TypeKind::Array, const Type &, AccessKind>);
    static_assert(!CanGetType<Provider, TypeKind::Slice, const Type &>);
    static_assert(!CanGetType<Provider, TypeKind::Pointer, const Type &, std::uint64_t>);
    static_assert(!CanGetType<Provider, TypeKind::Reference, const Type &, std::uint64_t>);
    static_assert(!CanGetType<Provider, TypeKind::Function>);
    static_assert(!CanGetType<Provider, TypeKind::Function, const Type &, AccessKind>);
    static_assert(!CanGetType<Provider, TypeKind::Class, Name>);
    static_assert(!CanGetType<Provider, TypeKind::Enum, Name>);
    static_assert(!CanGetType<Provider, TypeKind::Interface, Name>);
    static_assert(!CanGetType<Provider, static_cast<TypeKind>(255)>);
    static_assert(!CanGetType<const Provider, TypeKind::Integer, std::uint32_t, bool>);
    static_assert(!CanGetType<const Provider, TypeKind::Float, std::uint32_t>);
    static_assert(!CanGetType<const Provider, TypeKind::Array, const Type &, std::uint64_t>);
    static_assert(!CanGetType<const Provider, TypeKind::Slice, const Type &, AccessKind>);
    static_assert(!CanGetType<const Provider, TypeKind::Pointer, const Type &, AccessKind>);
    static_assert(!CanGetType<const Provider, TypeKind::Reference, const Type &, AccessKind>);
    static_assert(!CanGetType<const Provider, TypeKind::Function, const Type &>);
    return true;
  }

  static_assert(hasConstrainedTypeFactories<TypePool>());

  // Pool construction bootstraps the self-typed metatype before constants use the shared bool type.
  TEST(SemanticTypePoolTest, BuiltinsBootstrapMetaTypeAndConstantPool)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const SemanticContext &ConstContext = Context;
    const TypePool &Pool = ConstContext.typePool();
    EXPECT_EQ(&Pool, &Context.typePool());
    EXPECT_EQ(&Pool.context(), &Context);
    const Type *Builtins[] = {
        &Pool.getType<TypeKind::Meta>(),
        &Pool.getType<TypeKind::Void>(),
        &Pool.getType<TypeKind::Bool>(),
        &Pool.getType<TypeKind::Label>(),
        &Pool.getType<TypeKind::Module>(),
    };
    for (const Type *Builtin : Builtins)
    {
      EXPECT_EQ(&Builtin->type(), &Pool.getType<TypeKind::Meta>());
      EXPECT_EQ(&Builtin->context(), &Context);
    }
    EXPECT_EQ(&Context.constantPool().getBoolConstant(false).type(), &Pool.getType<TypeKind::Bool>());
    EXPECT_EQ(&Context.constantPool().getBoolConstant(true).type(), &Pool.getType<TypeKind::Bool>());
  }

  // Repeated pool calls reuse every structural type, and cache growth preserves prior addresses and edges.
  TEST(SemanticTypePoolTest, StructuralTypesRemainCanonicalAcrossGrowth)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    TypePool &Pool = Context.typePool();
    const IntegerType *Integer = Pool.getType<TypeKind::Integer>(32, true);
    ASSERT_NE(Integer, nullptr);
    const FloatType *Float = Pool.getType<TypeKind::Float>(64);
    const ArrayType *Array = Pool.getType<TypeKind::Array>(*Integer, 4);
    const SliceType *Slice = Pool.getType<TypeKind::Slice>(*Array, AccessKind::ReadOnly);
    const PointerType *Pointer = Pool.getType<TypeKind::Pointer>(*Integer, AccessKind::ReadWrite);
    const ReferenceType *Reference = Pool.getType<TypeKind::Reference>(*Integer, AccessKind::ReadOnly);
    const Type *Parameters[] = {Slice, Pointer, Reference};
    const FunctionType *Signature = Pool.getType<TypeKind::Function>(*Float, Parameters);
    ASSERT_NE(Signature, nullptr);
    for (std::uint32_t Width = 1; Width <= 256; ++Width)
    {
      const IntegerType *Element = Pool.getType<TypeKind::Integer>(Width, false);
      const ArrayType *GrowingArray = Pool.getType<TypeKind::Array>(*Element, Width);
      ASSERT_NE(GrowingArray, nullptr);
      EXPECT_EQ(GrowingArray, Pool.getType<TypeKind::Array>(*Element, Width));
    }
    EXPECT_EQ(Integer, Pool.getType<TypeKind::Integer>(32, true));
    EXPECT_EQ(Float, Pool.getType<TypeKind::Float>(64));
    EXPECT_EQ(Array, Pool.getType<TypeKind::Array>(*Integer, 4));
    EXPECT_EQ(Slice, Pool.getType<TypeKind::Slice>(*Array, AccessKind::ReadOnly));
    EXPECT_EQ(Pointer, Pool.getType<TypeKind::Pointer>(*Integer, AccessKind::ReadWrite));
    EXPECT_EQ(Reference, Pool.getType<TypeKind::Reference>(*Integer, AccessKind::ReadOnly));
    EXPECT_EQ(Signature, Pool.getType<TypeKind::Function>(*Float, Parameters));
    EXPECT_EQ(&Array->elementType(), Integer);
    EXPECT_EQ(&Slice->elementType(), Array);
    EXPECT_EQ(&Signature->returnType(), Float);
    EXPECT_EQ(&Signature->context(), &Context);
  }

  // Default parameters, explicit empty braces and empty spans share a zero-argument signature in the pool.
  TEST(SemanticTypePoolTest, EmptyFunctionParameterFormsShareCanonicalSignature)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    TypePool &Pool = Context.typePool();
    const Type &Void = Pool.getType<TypeKind::Void>();
    const FunctionType *Signature = Pool.getType<TypeKind::Function>(Void);
    ASSERT_NE(Signature, nullptr);
    EXPECT_TRUE(Signature->parameterTypes().empty());
    EXPECT_EQ(&Signature->returnType(), &Void);
    EXPECT_EQ(Signature, Pool.getType<TypeKind::Function>(Void, {}));
    const std::span<const Type *const> Parameters;
    EXPECT_EQ(Signature, Pool.getType<TypeKind::Function>(Void, Parameters));
  }

  // Nominal creation through either entry point retains independent identities even for identical names.
  TEST(SemanticTypePoolTest, NominalTypesAreNeverMergedByName)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    IRBuilder Factory(Context);
    TypePool &Pool = Context.typePool();
    const Name TypeName = Context.namePool().intern("Named");
    const ClassType *Class = Pool.createClassType(TypeName);
    const ClassType *OtherClass = Factory.createClassType(TypeName);
    ASSERT_NE(Class, nullptr);
    ASSERT_NE(OtherClass, nullptr);
    EXPECT_NE(Class, OtherClass);
    const EnumType *Enum = Pool.createEnumType(TypeName);
    const InterfaceType *Interface = Pool.createInterfaceType(TypeName);
    ASSERT_NE(Enum, nullptr);
    ASSERT_NE(Interface, nullptr);
    EXPECT_NE(Enum, Factory.createEnumType(TypeName));
    EXPECT_NE(Interface, Factory.createInterfaceType(TypeName));
    const Type *Nominals[] = {Class, OtherClass, Enum, Interface};
    for (const Type *Nominal : Nominals)
    {
      EXPECT_EQ(&Nominal->context(), &Context);
      EXPECT_EQ(&Nominal->type(), &Pool.getType<TypeKind::Meta>());
    }
    EXPECT_NE(Pool.getType<TypeKind::Pointer>(*Class, AccessKind::ReadOnly), Pool.getType<TypeKind::Pointer>(*OtherClass, AccessKind::ReadOnly));
  }

  // Direct pool calls reject invalid widths, access modes, names, foreign components and null parameters.
  TEST(SemanticTypePoolTest, InvalidRequestsCannotCreateTypes)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Foreign(Compilation);
    TypePool &Pool = Context.typePool();
    const Type &Local = Pool.getType<TypeKind::Bool>();
    const Type &Other = Foreign.typePool().getType<TypeKind::Bool>();
    const AccessKind InvalidAccess = static_cast<AccessKind>(255);
    EXPECT_EQ(Pool.getType<TypeKind::Integer>(0, true), nullptr);
    EXPECT_EQ(Pool.getType<TypeKind::Float>(80), nullptr);
    EXPECT_EQ(Pool.getType<TypeKind::Array>(Other, 1), nullptr);
    EXPECT_EQ(Pool.getType<TypeKind::Slice>(Other, AccessKind::ReadOnly), nullptr);
    EXPECT_EQ(Pool.getType<TypeKind::Pointer>(Other, AccessKind::ReadWrite), nullptr);
    EXPECT_EQ(Pool.getType<TypeKind::Reference>(Other, AccessKind::ReadOnly), nullptr);
    EXPECT_EQ(Pool.getType<TypeKind::Slice>(Local, InvalidAccess), nullptr);
    EXPECT_EQ(Pool.getType<TypeKind::Pointer>(Local, InvalidAccess), nullptr);
    EXPECT_EQ(Pool.getType<TypeKind::Reference>(Local, InvalidAccess), nullptr);
    EXPECT_EQ(Pool.getType<TypeKind::Function>(Other), nullptr);
    const Type *ForeignParameters[] = {&Other};
    const Type *NullParameters[] = {nullptr};
    EXPECT_EQ(Pool.getType<TypeKind::Function>(Local, ForeignParameters), nullptr);
    EXPECT_EQ(Pool.getType<TypeKind::Function>(Local, NullParameters), nullptr);
    EXPECT_EQ(Pool.createClassType(Name{}), nullptr);
    EXPECT_EQ(Pool.createEnumType(Name{}), nullptr);
    EXPECT_EQ(Pool.createInterfaceType(Name{}), nullptr);
    EXPECT_NE(&Pool.getType<TypeKind::Bool>(), &Other);
  }

  // Types supplied by the pool work with constants and IRBuilder and survive deletion of their using module.
  TEST(SemanticTypePoolTest, SharedTypesOutliveModuleIR)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    IRBuilder Factory(Context);
    TypePool &Pool = Context.typePool();
    const IntegerType *Integer = Pool.getType<TypeKind::Integer>(32, true);
    const FunctionType *Signature = Pool.getType<TypeKind::Function>(*Integer);
    const IntegerConstant *One = Context.constantPool().getIntegerConstant(*Integer, IntegerBits(32, 1));
    ASSERT_NE(One, nullptr);
    Module *Root = Factory.createModule(Context.namePool().intern("Root"));
    auto Function = Factory.createFunction(Context.namePool().intern("Function"), *Signature);
    ASSERT_NE(Function, nullptr);
    BasicBlock *Body = Factory.createFunctionBody(*Function);
    ASSERT_NE(Body, nullptr);
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Body));
    AddInstruction *Sum = Builder.createAddInstruction(*One, *One);
    ASSERT_NE(Sum, nullptr);
    ASSERT_NE(Builder.createReturnInstruction(Sum), nullptr);
    EXPECT_EQ(&Sum->type(), Integer);
    EXPECT_EQ(&Body->type(), &Pool.getType<TypeKind::Label>());
    EXPECT_EQ(&Root->type(), &Pool.getType<TypeKind::Module>());
    ASSERT_TRUE(Factory.appendValue(Root->entryBlock(), std::move(Function)));
    Builder.clearInsertPoint();
    ASSERT_TRUE(Factory.eraseModule(*Root));
    EXPECT_EQ(Integer, Context.typePool().getType<TypeKind::Integer>(32, true));
    EXPECT_EQ(Signature, Pool.getType<TypeKind::Function>(*Integer));
    EXPECT_EQ(One, Context.constantPool().getIntegerConstant(*Integer, IntegerBits(32, 1)));
    EXPECT_EQ(&One->type(), Integer);
  }
} // namespace ink::semantic::test
