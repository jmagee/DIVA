//===-- UnitTests/TestLibScopeView/TestScope.cpp ----------------*- C++ -*-===//
///
/// Copyright (c) 2017 by Sony Interactive Entertainment Inc.
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
//===----------------------------------------------------------------------===//
///
/// \file
/// Tests for LibScopeView::Scope.
///
//===----------------------------------------------------------------------===//

#include "Line.h"
#include "Reader.h"
#include "Type.h"

#include "dwarf.h"
#include "gtest/gtest.h"

using namespace LibScopeView;

TEST(Scope, getAsText_Alias) {
  Reader R(nullptr);
  setReader(&R);

  ScopeAlias Alias;
  Alias.setIsTemplateAlias();
  EXPECT_EQ(Alias.getAsText(), "{Alias} \"\" -> \"\"");

  Alias.setName("test<int>");
  EXPECT_EQ(Alias.getAsText(), "{Alias} \"test<int>\" -> \"\"");

  Type Ty;
  Alias.setType(&Ty);
  EXPECT_EQ(Alias.getAsText(), "{Alias} \"test<int>\" -> \"\"");

  Ty.setName("foo<int, int>");
  EXPECT_EQ(Alias.getAsText(), "{Alias} \"test<int>\" -> \"foo<int, int>\"");

  Ty.setQualifiedName("Class::");
  Ty.setHasQualifiedName();
  EXPECT_EQ(Alias.getAsText(), "{Alias} \"test<int>\" -> \"foo<int, int>\"")
      << "Qualified name should not be printed if the show-qualified option is "
         "not set";

  R.getOptions().setFormatQualifiedName();
  EXPECT_EQ(Alias.getAsText(),
            "{Alias} \"test<int>\" -> \"Class::foo<int, int>\"");
}

TEST(Scope, getAsYAML_Alias) {
  Reader R(nullptr);
  setReader(&R);

  ScopeAlias Alias;
  Alias.setIsTemplateAlias();
  Alias.setName("test<int>");
  Alias.setLineNumber(18);
  Alias.setFileName("test.cpp");
  Alias.setDieOffset(0x123f);
  Alias.setDieTag(DW_TAG_template_alias);
  Type Ty;
  Alias.setType(&Ty);
  Ty.setName("foo<int, int>");

  EXPECT_EQ(Alias.getAsYAML(), "object: \"Alias\"\n"
                               "name: \"test<int>\"\n"
                               "type: \"foo<int, int>\"\n"
                               "source:\n"
                               "  line: 18\n"
                               "  file: \"test.cpp\"\n"
                               "dwarf:\n"
                               "  offset: 0x123f\n"
                               "  tag: \"DW_TAG_template_alias\"\n"
                               "attributes: {}");

  Ty.setQualifiedName("Class::");
  Ty.setHasQualifiedName();
  EXPECT_EQ(Alias.getAsYAML(), "object: \"Alias\"\n"
                               "name: \"test<int>\"\n"
                               "type: \"Class::foo<int, int>\"\n"
                               "source:\n"
                               "  line: 18\n"
                               "  file: \"test.cpp\"\n"
                               "dwarf:\n"
                               "  offset: 0x123f\n"
                               "  tag: \"DW_TAG_template_alias\"\n"
                               "attributes: {}");
}

TEST(Scope, getAsText_Array) {
  Reader R(nullptr);
  setReader(&R);

  ScopeArray Array;
  Array.setIsArrayType();
  Array.setName("int 5,10");
  EXPECT_EQ(Array.getAsText(), "{Array} \"int 5,10\"");
}

TEST(Scope, getAsText_Block) {
  Reader R(nullptr);
  R.getOptions().setFormatIndentation();
  R.getOptions().setPrintBlockAttributes();
  setReader(&R);

  Scope Block(/*Level*/ 3);
  Block.setIsBlock();
  EXPECT_EQ(Block.getAsText(), "{Block}");

  // See Object::getAttributeInfoAsText() for why the indent is this size.
  const std::string AttrIndent(3 + 4 + 8 + ((Block.getLevel() + 1) * 2), ' ');

  Scope TryBlock(/*Level*/ 3);
  TryBlock.setIsBlock();
  TryBlock.setIsTryBlock();
  EXPECT_EQ(TryBlock.getAsText(),
    std::string("{Block}\n") + AttrIndent + std::string("- try"));

  Scope CatchBlock(/*Level*/ 3);
  CatchBlock.setIsBlock();
  CatchBlock.setIsCatchBlock();
  EXPECT_EQ(CatchBlock.getAsText(),
    std::string("{Block}\n") + AttrIndent + std::string("- catch"));
}

TEST(Scope, getAsYAML_Block) {
  Reader R(nullptr);
  setReader(&R);

  Scope Block;
  Block.setIsBlock();
  Block.setLineNumber(10);
  Block.setFileName("test.cpp");
  Block.setDieOffset(0xff);
  Block.setDieTag(DW_TAG_lexical_block);
  EXPECT_EQ(Block.getAsYAML(), "object: \"Block\"\n"
                               "name: null\n"
                               "type: null\n"
                               "source:\n"
                               "  line: 10\n"
                               "  file: \"test.cpp\"\n"
                               "dwarf:\n"
                               "  offset: 0xff\n"
                               "  tag: \"DW_TAG_lexical_block\"\n"
                               "attributes:\n"
                               "  try: false\n"
                               "  catch: false");

  Scope TryBlock;
  TryBlock.setIsBlock();
  TryBlock.setIsTryBlock();
  TryBlock.setLineNumber(11);
  TryBlock.setFileName("test.cpp");
  TryBlock.setDieOffset(0x100);
  TryBlock.setDieTag(DW_TAG_try_block);
  EXPECT_EQ(TryBlock.getAsYAML(), "object: \"Block\"\n"
                                  "name: null\n"
                                  "type: null\n"
                                  "source:\n"
                                  "  line: 11\n"
                                  "  file: \"test.cpp\"\n"
                                  "dwarf:\n"
                                  "  offset: 0x100\n"
                                  "  tag: \"DW_TAG_try_block\"\n"
                                  "attributes:\n"
                                  "  try: true\n"
                                  "  catch: false");

  Scope CatchBlock;
  CatchBlock.setIsBlock();
  CatchBlock.setIsCatchBlock();
  CatchBlock.setLineNumber(12);
  CatchBlock.setFileName("test.cpp");
  CatchBlock.setDieOffset(0x111);
  CatchBlock.setDieTag(DW_TAG_catch_block);
  EXPECT_EQ(CatchBlock.getAsYAML(), "object: \"Block\"\n"
                                    "name: null\n"
                                    "type: null\n"
                                    "source:\n"
                                    "  line: 12\n"
                                    "  file: \"test.cpp\"\n"
                                    "dwarf:\n"
                                    "  offset: 0x111\n"
                                    "  tag: \"DW_TAG_catch_block\"\n"
                                    "attributes:\n"
                                    "  try: false\n"
                                    "  catch: true");
}

TEST(Scope, getAsText_Class) {
  Reader R(nullptr);
  setReader(&R);

  ScopeAggregate Class;
  Class.setIsAggregate();
  Class.setIsClassType();
  Class.setName("TestClass");
  EXPECT_EQ(Class.getAsText(), "{Class} \"TestClass\"");

  // See Object::getAttributeInfoAsText() for why the indent is this size.
  R.getOptions().resetFormatIndentation();
  std::string AttrIndent(3 + 4 + 8, ' ');
  AttrIndent += "- ";

  Class.setIsTemplate();
  EXPECT_EQ(Class.getAsText(), std::string("{Class} \"TestClass\"\n") +
                                   AttrIndent + std::string("Template"));
}

TEST(Scope, getAsYAML_Class) {
  Reader R(nullptr);
  setReader(&R);

  ScopeAggregate Class;
  Class.setIsAggregate();
  Class.setIsClassType();
  Class.setName("TestClass");
  Class.setLineNumber(5);
  Class.setFileName("testclass.cpp");
  Class.setDieOffset(0xD3ADB33F);
  Class.setDieTag(DW_TAG_class_type);

  ScopeAggregate ParentClass;
  ParentClass.setName("TestParentClass");

  TypeImport * TyImport = new TypeImport;
  TyImport->setIsInheritance();
  TyImport->setType(&ParentClass);
  TyImport->setInheritanceAccess(AccessSpecifier::Public);

  Class.addObject(TyImport);

  std::string Expected("object: \"Class\"\n"
                       "name: \"TestClass\"\n"
                       "type: null\n"
                       "source:\n"
                       "  line: 5\n"
                       "  file: \"testclass.cpp\"\n"
                       "dwarf:\n"
                       "  offset: 0xd3adb33f\n"
                       "  tag: \"DW_TAG_class_type\"\n"
                       "attributes:\n");

  EXPECT_EQ(Class.getAsYAML(),
            Expected + std::string("  is_template: false\n"
                                   "  inherits_from:\n"
                                   "    - parent: \"TestParentClass\"\n"
                                   "      access_specifier: \"public\""));

  Class.setIsTemplate();
  EXPECT_EQ(Class.getAsYAML(),
            Expected + std::string("  is_template: true\n"
                                   "  inherits_from:\n"
                                   "    - parent: \"TestParentClass\"\n"
                                   "      access_specifier: \"public\""));
}

TEST(Scope, getAsYAML_multiInheritance) {
  Reader R(nullptr);
  setReader(&R);

  ScopeAggregate Class;
  Class.setIsAggregate();
  Class.setIsClassType();
  Class.setName("TestClass");
  Class.setLineNumber(5);
  Class.setFileName("testclass.cpp");
  Class.setDieOffset(0xD3ADB33F);
  Class.setDieTag(DW_TAG_class_type);

  ScopeAggregate ParentClassA;
  ParentClassA.setName("TestParentClassA");

  TypeImport * TyImportA = new TypeImport;
  TyImportA->setIsInheritance();
  TyImportA->setType(&ParentClassA);
  TyImportA->setInheritanceAccess(AccessSpecifier::Public);

  ScopeAggregate ParentClassB;
  ParentClassB.setName("TestParentClassB");

  TypeImport * TyImportB = new TypeImport;
  TyImportB->setIsInheritance();
  TyImportB->setType(&ParentClassB);
  TyImportB->setInheritanceAccess(AccessSpecifier::Protected);

  Class.addObject(TyImportA);
  Class.addObject(TyImportB);

  EXPECT_EQ(Class.getAsYAML(), "object: \"Class\"\n"
                               "name: \"TestClass\"\n"
                               "type: null\n"
                               "source:\n"
                               "  line: 5\n"
                               "  file: \"testclass.cpp\"\n"
                               "dwarf:\n"
                               "  offset: 0xd3adb33f\n"
                               "  tag: \"DW_TAG_class_type\"\n"
                               "attributes:\n"
                               "  is_template: false\n"
                               "  inherits_from:\n"
                               "    - parent: \"TestParentClassA\"\n"
                               "      access_specifier: \"public\"\n"
                               "    - parent: \"TestParentClassB\"\n"
                               "      access_specifier: \"protected\"");
}

TEST(Scope, getAsYAML_Unspecified_Class) {
  Reader R(nullptr);
  setReader(&R);

  ScopeAggregate Class;
  Class.setIsAggregate();
  Class.setIsClassType();
  Class.setName("TestClass");
  Class.setLineNumber(5);
  Class.setFileName("testclass.cpp");
  Class.setDieOffset(0xD3ADB33F);
  Class.setDieTag(DW_TAG_class_type);

  ScopeAggregate ParentStruct;
  ParentStruct.setName("TestParentStruct");

  TypeImport * TyImport = new TypeImport;
  TyImport->setIsInheritance();
  TyImport->setType(&ParentStruct);
  TyImport->setInheritanceAccess(AccessSpecifier::Unspecified);

  Class.addObject(TyImport);

  EXPECT_EQ(Class.getAsYAML(), "object: \"Class\"\n"
                               "name: \"TestClass\"\n"
                               "type: null\n"
                               "source:\n"
                               "  line: 5\n"
                               "  file: \"testclass.cpp\"\n"
                               "dwarf:\n"
                               "  offset: 0xd3adb33f\n"
                               "  tag: \"DW_TAG_class_type\"\n"
                               "attributes:\n"
                               "  is_template: false\n"
                               "  inherits_from:\n"
                               "    - parent: \"TestParentStruct\"\n"
                               "      access_specifier: \"private\"");
}

TEST(Scope, getAsText_CompileUnit) {
  Reader R(nullptr);
  setReader(&R);

  ScopeCompileUnit compileUnit;
  compileUnit.setIsCompileUnit();
  EXPECT_EQ(compileUnit.getAsText(), "{CompileUnit} \"\"");

  compileUnit.setName("c:/fakedir/fakefile.cpp");

  EXPECT_EQ(compileUnit.getAsText(), "{CompileUnit} \"c:/fakedir/fakefile.cpp\"");
}

TEST(Scope, getAsYAML_CompileUnit) {
  Reader R(nullptr);
  setReader(&R);

  ScopeCompileUnit CompileUnit;
  CompileUnit.setIsCompileUnit();
  CompileUnit.setName("c:/fakedir/fakefile.cpp");
  CompileUnit.setDieOffset(0xbeef);
  CompileUnit.setDieTag(DW_TAG_compile_unit);
  EXPECT_EQ(CompileUnit.getAsYAML(), "object: \"CompileUnit\"\n"
                                     "name: \"c:/fakedir/fakefile.cpp\"\n"
                                     "type: null\n"
                                     "source:\n"
                                     "  line: null\n"
                                     "  file: null\n"
                                     "dwarf:\n"
                                     "  offset: 0xbeef\n"
                                     "  tag: \"DW_TAG_compile_unit\"\n"
                                     "attributes: {}");
}

TEST(Scope, getAsText_Enumeration) {
  Reader R(nullptr);
  setReader(&R);

  ScopeEnumeration ScpEnumeration;
  ScpEnumeration.setIsEnumerationType();
  EXPECT_EQ(ScpEnumeration.getAsText(), "{Enum} \"\"");

  ScpEnumeration.setName("days");
  EXPECT_EQ(ScpEnumeration.getAsText(), "{Enum} \"days\"");

  ScpEnumeration.setIsClass();
  EXPECT_EQ(ScpEnumeration.getAsText(),
            "{Enum} class \"days\"");

  Type Ty;
  Ty.setName("unsigned int");
  ScpEnumeration.setName("days");
  ScpEnumeration.setType(&Ty);
  EXPECT_EQ(ScpEnumeration.getAsText(),
            "{Enum} class \"days\" -> \"unsigned int\"");
}

TEST(Scope, getAsYAML_Enumeration) {
  Reader R(nullptr);
  setReader(&R);

  ScopeEnumeration Enum;
  Enum.setIsEnumerationType();
  Enum.setName("days");
  Enum.setLineNumber(42);
  Enum.setFileName("enum.cpp");
  Enum.setDieOffset(0xfeed);
  Enum.setDieTag(DW_TAG_enumeration_type);
  Type Ty;
  Ty.setName("unsigned int");
  Enum.setName("days");
  Enum.setType(&Ty);
  std::string ExpectedYAML(
    "object: \"Enum\"\n"
    "name: \"days\"\n"
    "type: \"unsigned int\"\n"
    "source:\n"
    "  line: 42\n"
    "  file: \"enum.cpp\"\n"
    "dwarf:\n"
    "  offset: 0xfeed\n"
    "  tag: \"DW_TAG_enumeration_type\"\n"
    "attributes:"
  );
  EXPECT_EQ(Enum.getAsYAML(),
            ExpectedYAML + "\n  class: false\n  enumerators: []");

  Enum.setIsClass();
  ExpectedYAML += "\n  class: true";
  EXPECT_EQ(Enum.getAsYAML(), ExpectedYAML + "\n  enumerators: []");

  auto *Monday = new TypeEnumerator;
  Monday->setIsEnumerator();
  Monday->setName("monday");
  Monday->setValue("10");
  Enum.addObject(Monday);
  auto *Tuesday = new TypeEnumerator;
  Tuesday->setIsEnumerator();
  Tuesday->setName("tuesday");
  Tuesday->setValue("20");
  Enum.addObject(Tuesday);
  ExpectedYAML += "\n  enumerators:"
                  "\n    - enumerator: \"monday\""
                  "\n      value: 10"
                  "\n    - enumerator: \"tuesday\""
                  "\n      value: 20";
  EXPECT_EQ(Enum.getAsYAML(), ExpectedYAML);
}

TEST(Scope, getAsText_Function) {
  Reader R(nullptr);
  setReader(&R);

  // See Object::getAttributeInfoAsText() for why the indent is this size.
  R.getOptions().resetFormatIndentation();
  std::string AttrIndent(3 + 4 + 8, ' ');
  AttrIndent += "- ";

  R.getOptions().setFormatQualifiedName();

  ScopeFunction ScpFunc(0);
  ScpFunc.setIsScope();
  ScpFunc.setIsFunction();
  EXPECT_EQ(ScpFunc.getAsText(),
            std::string("{Function} \"\" -> \"\"\n" + AttrIndent +
                        std::string("No declaration")));

  ScpFunc.setName("qaz");
  EXPECT_EQ(ScpFunc.getAsText(),
            std::string("{Function} \"qaz\" -> \"\"\n" + AttrIndent +
                        std::string("No declaration")));

  Type RetType(0);
  RetType.setName("wsx");
  ScpFunc.setType(&RetType);
  EXPECT_EQ(ScpFunc.getAsText(),
            std::string("{Function} \"qaz\" -> \"wsx\"\n" + AttrIndent +
                        std::string("No declaration")));

  ScopeNamespace NS;
  NS.setIsNamespace();
  NS.setName("TestNamespace");
  ScpFunc.setParent(&NS);
  ScpFunc.resolveName();
  EXPECT_EQ(ScpFunc.getAsText(),
            std::string("{Function} \"TestNamespace::qaz\" -> \"wsx\"\n" +
                        AttrIndent + std::string("No declaration")));

  ScopeNamespace NS2;
  NS2.setIsNamespace();
  NS2.setName("BaseNamespace");
  NS.setParent(&NS2);
  ScpFunc.resolveName();
  EXPECT_EQ(
      ScpFunc.getAsText(),
      std::string(
          "{Function} \"BaseNamespace::TestNamespace::qaz\" -> \"wsx\"\n" +
          AttrIndent + std::string("No declaration")));

  ScopeFunction StaticFunc;
  StaticFunc.setIsScope();
  StaticFunc.setIsFunction();
  StaticFunc.setName("sf");
  StaticFunc.setIsStatic();
  EXPECT_EQ(StaticFunc.getAsText(),
            std::string("{Function} static \"sf\" -> \"\"\n" + AttrIndent +
                        std::string("No declaration")));

  ScopeFunction DecInlineFunc;
  DecInlineFunc.setIsScope();
  DecInlineFunc.setIsFunction();
  DecInlineFunc.setName("dif");
  DecInlineFunc.setIsDeclaredInline();
  EXPECT_EQ(DecInlineFunc.getAsText(),
            std::string("{Function} inline \"dif\" -> \"\"\n" + AttrIndent +
                        std::string("No declaration")));

  ScopeFunction ScpQualFunc(0);
  ScpQualFunc.setIsScope();
  ScpQualFunc.setIsFunction();
  ScpQualFunc.setName("qaz");
  ScpQualFunc.setType(&RetType);
  EXPECT_EQ(ScpQualFunc.getAsText(),
            std::string("{Function} \"qaz\" -> \"wsx\"\n" + AttrIndent +
                        std::string("No declaration")));

  RetType.setHasQualifiedName();
  RetType.setQualifiedName("base::");
  EXPECT_EQ(ScpQualFunc.getAsText(),
            std::string("{Function} \"qaz\" -> \"base::wsx\"\n" + AttrIndent +
                        std::string("No declaration")));

  R.getOptions().resetFormatQualifiedName();

  const std::string ErrorMessage("Qualified name should not be printed if the "
                                 "show-qualified option is not set");

  EXPECT_EQ(ScpQualFunc.getAsText(),
            std::string("{Function} \"qaz\" -> \"wsx\"\n" + AttrIndent +
                        std::string("No declaration")))
      << ErrorMessage;

  EXPECT_EQ(ScpFunc.getAsText(),
            std::string("{Function} \"qaz\" -> \"wsx\"\n" + AttrIndent +
                        std::string("No declaration")))
      << ErrorMessage;
}

TEST(Scope, getAsText_Function_Attributes) {
  Reader R(nullptr);
  setReader(&R);

  // See Object::getAttributeInfoAsText() for why the indent is this size.
  R.getOptions().resetFormatIndentation();
  std::string AttrIndent(3 + 4 + 8, ' ');
  AttrIndent += "- ";

  // Test attributes.
  std::string Expected("{Function} \"\" -> \"\"");
  ScopeFunction Func;
  Func.setIsFunction();

  ScopeFunction DeclFunc;
  DeclFunc.setIsFunction();
  Expected.append("\n").append(AttrIndent);
  EXPECT_EQ(DeclFunc.getAsText(), Expected + std::string("No declaration"));

  DeclFunc.setIsDeclaration();
  EXPECT_EQ(DeclFunc.getAsText(), Expected + std::string("Is declaration"));

  DeclFunc.setIsFunction();
  DeclFunc.setFileName("test/file.h");
  DeclFunc.setLineNumber(24);
  Func.setReference(&DeclFunc);
  Expected.append("Declaration @ ");
  EXPECT_EQ(Func.getAsText(), Expected + std::string("test/file.h,24"));

  R.getOptions().setFormatFileName();
  EXPECT_EQ(Func.getAsText(), Expected + std::string("file.h,24"));

  DeclFunc.setInvalidFileName();
  Expected.append("?,24");
  EXPECT_EQ(Func.getAsText(), Expected);

  Func.setIsTemplate();
  Expected.append("\n").append(AttrIndent).append("Template");
  EXPECT_EQ(Func.getAsText(), Expected);

  Func.setIsInlined();
  Expected.append("\n").append(AttrIndent).append("Inlined");
  EXPECT_EQ(Func.getAsText(), Expected);

  Func.setIsDeclaration();
  Expected.append("\n").append(AttrIndent).append("Is declaration");
  EXPECT_EQ(Func.getAsText(), Expected);

  Line Low1;
  Line Low2;
  Line High1;
  Line High2;
  Low1.setLineNumber(9);
  Low2.setLineNumber(4);
  High1.setLineNumber(20);
  High2.setLineNumber(30);
  EXPECT_EQ(Func.getAsText(), Expected);

  // ScopeFunctionInlined.
  ScopeFunctionInlined inlined(0);
  inlined.setIsInlinedSubroutine();
  inlined.setName("Foo");
  EXPECT_EQ(inlined.getAsText(),
    std::string("{Function} \"Foo\" -> \"\"\n") +
      AttrIndent + std::string("No declaration\n") +
      AttrIndent + std::string("Inlined"));
}

TEST(Scope, getAsYAML_Function) {
  Reader R(nullptr);
  setReader(&R);

  ScopeFunction Func(0);
  Func.setIsScope();
  Func.setIsFunction();
  Func.setName("Foo");
  Func.setLineNumber(17);
  Func.setFileName("foo.cpp");
  Func.setDieOffset(0xce);
  Func.setDieTag(DW_TAG_subprogram);

  // Normal function.
  // No type is "void".
  EXPECT_EQ(Func.getAsYAML(),
    "object: \"Function\"\n"
    "name: \"Foo\"\n"
    "type: \"void\"\n"
    "source:\n"
    "  line: 17\n"
    "  file: \"foo.cpp\"\n"
    "dwarf:\n"
    "  offset: 0xce\n"
    "  tag: \"DW_TAG_subprogram\"\n"
    "attributes:\n"
    "  declaration:\n"
    "    file: null\n"
    "    line: null\n"
    "  is_template: false\n"
    "  static: false\n"
    "  inline: false\n"
    "  is_inlined: false\n"
    "  is_declaration: false"
  );

  Type Ty(0);
  Ty.setName("int");
  Func.setType(&Ty);
  EXPECT_EQ(Func.getAsYAML(),
    "object: \"Function\"\n"
    "name: \"Foo\"\n"
    "type: \"int\"\n"
    "source:\n"
    "  line: 17\n"
    "  file: \"foo.cpp\"\n"
    "dwarf:\n"
    "  offset: 0xce\n"
    "  tag: \"DW_TAG_subprogram\"\n"
    "attributes:\n"
    "  declaration:\n"
    "    file: null\n"
    "    line: null\n"
    "  is_template: false\n"
    "  static: false\n"
    "  inline: false\n"
    "  is_inlined: false\n"
    "  is_declaration: false"
  );

  // Declaration for inlined function.
  Func.setIsInlined();
  Func.setIsDeclaredInline();
  EXPECT_EQ(Func.getAsYAML(),
    "object: \"Function\"\n"
    "name: \"Foo\"\n"
    "type: \"int\"\n"
    "source:\n"
    "  line: 17\n"
    "  file: \"foo.cpp\"\n"
    "dwarf:\n"
    "  offset: 0xce\n"
    "  tag: \"DW_TAG_subprogram\"\n"
    "attributes:\n"
    "  declaration:\n"
    "    file: null\n"
    "    line: null\n"
    "  is_template: false\n"
    "  static: false\n"
    "  inline: true\n"
    "  is_inlined: true\n"
    "  is_declaration: false"
  );

  // Declaration for Static function.
  Func.setIsStatic();
  Func.setIsDeclaration();
  EXPECT_EQ(Func.getAsYAML(),
    "object: \"Function\"\n"
    "name: \"Foo\"\n"
    "type: \"int\"\n"
    "source:\n"
    "  line: 17\n"
    "  file: \"foo.cpp\"\n"
    "dwarf:\n"
    "  offset: 0xce\n"
    "  tag: \"DW_TAG_subprogram\"\n"
    "attributes:\n"
    "  declaration:\n"
    "    file: null\n"
    "    line: null\n"
    "  is_template: false\n"
    "  static: true\n"
    "  inline: true\n"
    "  is_inlined: true\n"
    "  is_declaration: true"
  );

  // Template function.
  Func.setIsTemplate();
  EXPECT_EQ(Func.getAsYAML(),
    "object: \"Function\"\n"
    "name: \"Foo\"\n"
    "type: \"int\"\n"
    "source:\n"
    "  line: 17\n"
    "  file: \"foo.cpp\"\n"
    "dwarf:\n"
    "  offset: 0xce\n"
    "  tag: \"DW_TAG_subprogram\"\n"
    "attributes:\n"
    "  declaration:\n"
    "    file: null\n"
    "    line: null\n"
    "  is_template: true\n"
    "  static: true\n"
    "  inline: true\n"
    "  is_inlined: true\n"
    "  is_declaration: true"
  );

  // Function to be used as Reference.
  ScopeFunction Reference(0);
  Reference.setIsScope();
  Reference.setIsFunction();
  Reference.setName("Ref");
  Reference.setLineNumber(620);
  Reference.setFileName("ref.cpp");
  Reference.setDieOffset(0xba);

  // Reference to function.
  Func.setReference(&Reference);
  EXPECT_EQ(Func.getAsYAML(),
    "object: \"Function\"\n"
    "name: \"Foo\"\n"
    "type: \"int\"\n"
    "source:\n"
    "  line: 17\n"
    "  file: \"foo.cpp\"\n"
    "dwarf:\n"
    "  offset: 0xce\n"
    "  tag: \"DW_TAG_subprogram\"\n"
    "attributes:\n"
    "  declaration:\n"
    "    file: \"ref.cpp\"\n"
    "    line: 620\n"
    "  is_template: true\n"
    "  static: true\n"
    "  inline: true\n"
    "  is_inlined: true\n"
    "  is_declaration: true"
  );

  // Invalid Reference to function.
  Reference.setInvalidFileName();
  Reference.setLineNumber(0);
  EXPECT_EQ(Func.getAsYAML(),
    "object: \"Function\"\n"
    "name: \"Foo\"\n"
    "type: \"int\"\n"
    "source:\n"
    "  line: 17\n"
    "  file: \"foo.cpp\"\n"
    "dwarf:\n"
    "  offset: 0xce\n"
    "  tag: \"DW_TAG_subprogram\"\n"
    "attributes:\n"
    "  declaration:\n"
    "    file: \"?\"\n"
    "    line: 0\n"
    "  is_template: true\n"
    "  static: true\n"
    "  inline: true\n"
    "  is_inlined: true\n"
    "  is_declaration: true"
  );

  // ScopeFunctionInlined.
  ScopeFunctionInlined Inlined(0);
  Inlined.setIsInlinedSubroutine();
  Inlined.setName("Foo");
  Inlined.setDieOffset(0x100);
  Inlined.setDieTag(DW_TAG_inlined_subroutine);

  EXPECT_EQ(Inlined.getAsYAML(),
    "object: \"Function\"\n"
    "name: \"Foo\"\n"
    "type: \"void\"\n"
    "source:\n"
    "  line: null\n"
    "  file: null\n"
    "dwarf:\n"
    "  offset: 0x100\n"
    "  tag: \"DW_TAG_inlined_subroutine\"\n"
    "attributes:\n"
    "  declaration:\n"
    "    file: null\n"
    "    line: null\n"
    "  is_template: false\n"
    "  static: false\n"
    "  inline: false\n"
    "  is_inlined: true\n"
    "  is_declaration: false"
  );
}

TEST(Scope, getAsText_Namespace) {
  Reader R(nullptr);
  setReader(&R);

  ScopeNamespace NS;
  NS.setIsNamespace();
  EXPECT_EQ(NS.getAsText(), "{Namespace}");

  NS.setName("TestNamespace");
  EXPECT_EQ(NS.getAsText(), "{Namespace} \"TestNamespace\"");

  R.getOptions().setFormatQualifiedName();
  ScopeNamespace Parent;
  Parent.setName("Base");
  NS.setParent(&Parent);
  EXPECT_EQ(NS.getAsText(), "{Namespace} \"Base::TestNamespace\"");
}

TEST(Scope, getAsYAML_Namespace) {
  Reader R(nullptr);
  setReader(&R);

  ScopeNamespace NS;
  NS.setIsNamespace();
  NS.setName("TestNamespace");
  NS.setLineNumber(17);
  NS.setFileName("test.cpp");
  NS.setDieOffset(0x1212);
  NS.setDieTag(DW_TAG_namespace);
  EXPECT_EQ(NS.getAsYAML(), "object: \"Namespace\"\n"
                            "name: \"TestNamespace\"\n"
                            "type: null\n"
                            "source:\n"
                            "  line: 17\n"
                            "  file: \"test.cpp\"\n"
                            "dwarf:\n"
                            "  offset: 0x1212\n"
                            "  tag: \"DW_TAG_namespace\"\n"
                            "attributes: {}");
}

TEST(Scope, getAsText_Root) {
  Reader R(nullptr);
  setReader(&R);

  ScopeRoot Root;
  Root.setIsRoot();
  EXPECT_EQ(Root.getAsText(), "{InputFile} \"\"");

  Root.setName("test/file.o");
  EXPECT_EQ(Root.getAsText(), "{InputFile} \"test/file.o\"");
}

TEST(Scope, getAsText_Struct) {
  Reader R(nullptr);
  setReader(&R);

  ScopeAggregate Struct;
  Struct.setIsAggregate();
  Struct.setIsStructType();
  Struct.setName("TestStruct");
  EXPECT_EQ(Struct.getAsText(), "{Struct} \"TestStruct\"");

  // See Object::getAttributeInfoAsText() for why the indent is this size.
  R.getOptions().resetFormatIndentation();
  std::string AttrIndent(3 + 4 + 8, ' ');
  AttrIndent += "- ";

  Struct.setIsTemplate();
  EXPECT_EQ(Struct.getAsText(), std::string("{Struct} \"TestStruct\"\n") +
                                    AttrIndent + std::string("Template"));
}

TEST(Scope, getAsYAML_Struct) {
  Reader R(nullptr);
  setReader(&R);

  ScopeAggregate Struct;
  Struct.setIsAggregate();
  Struct.setIsStructType();
  Struct.setName("TestStruct");
  Struct.setLineNumber(5);
  Struct.setFileName("teststruct.cpp");
  Struct.setDieOffset(0xD3ADB33F);
  Struct.setDieTag(DW_TAG_class_type);

  ScopeAggregate ParentClass;
  ParentClass.setName("TestParentClass");

  TypeImport * TyImport = new TypeImport;
  TyImport->setIsInheritance();
  TyImport->setType(&ParentClass);
  TyImport->setInheritanceAccess(AccessSpecifier::Private);

  Struct.addObject(TyImport);

  std::string Expected("object: \"Struct\"\n"
                       "name: \"TestStruct\"\n"
                       "type: null\n"
                       "source:\n"
                       "  line: 5\n"
                       "  file: \"teststruct.cpp\"\n"
                       "dwarf:\n"
                       "  offset: 0xd3adb33f\n"
                       "  tag: \"DW_TAG_class_type\"\n"
                       "attributes:\n");

  EXPECT_EQ(Struct.getAsYAML(),
            Expected + std::string("  is_template: false\n"
                                   "  inherits_from:\n"
                                   "    - parent: \"TestParentClass\"\n"
                                   "      access_specifier: \"private\""));

  Struct.setIsTemplate();
  EXPECT_EQ(Struct.getAsYAML(),
            Expected + std::string("  is_template: true\n"
                                   "  inherits_from:\n"
                                   "    - parent: \"TestParentClass\"\n"
                                   "      access_specifier: \"private\""));
}

TEST(Scope, getAsYAML_Unspecified_Struct) {
  Reader R(nullptr);
  setReader(&R);

  ScopeAggregate Struct;
  Struct.setIsAggregate();
  Struct.setIsStructType();
  Struct.setName("TestStruct");
  Struct.setLineNumber(5);
  Struct.setFileName("teststruct.cpp");
  Struct.setDieOffset(0xD3ADB33F);
  Struct.setDieTag(DW_TAG_class_type);

  ScopeAggregate ParentClass;
  ParentClass.setName("TestParentClass");
  ParentClass.setIsStructType();

  TypeImport * TyImport = new TypeImport;
  TyImport->setIsInheritance();
  TyImport->setType(&ParentClass);
  TyImport->setInheritanceAccess(AccessSpecifier::Unspecified);


  Struct.addObject(TyImport);

  EXPECT_EQ(Struct.getAsYAML(), "object: \"Struct\"\n"
                                "name: \"TestStruct\"\n"
                                "type: null\n"
                                "source:\n"
                                "  line: 5\n"
                                "  file: \"teststruct.cpp\"\n"
                                "dwarf:\n"
                                "  offset: 0xd3adb33f\n"
                                "  tag: \"DW_TAG_class_type\"\n"
                                "attributes:\n"
                                "  is_template: false\n"
                                "  inherits_from:\n"
                                "    - parent: \"TestParentClass\"\n"
                                "      access_specifier: \"public\"");
}

TEST(Scope, getAsText_TemplatePack) {
  Reader R(nullptr);
  setReader(&R);

  ScopeTemplatePack ScpTP;
  ScpTP.setIsScope();
  ScpTP.setIsTemplatePack();
  EXPECT_EQ(ScpTP.getAsText(), "{TemplateParameter} \"\"");

  ScpTP.setName("qaz");
  EXPECT_EQ(ScpTP.getAsText(), "{TemplateParameter} \"qaz\"");
}

TEST(Scope, getAsYAML_TemplatePack) {
  Reader R(nullptr);
  setReader(&R);

  ScopeTemplatePack Pack;
  Pack.setIsTemplatePack();
  Pack.setName("TPack");
  Pack.setLineNumber(11);
  Pack.setFileName("test.cpp");
  Pack.setDieOffset(0x11);
  Pack.setDieTag(DW_TAG_GNU_template_parameter_pack);

  std::string Expected("object: \"TemplateParameter\"\n"
                       "name: \"TPack\"\n"
                       "type: null\n"
                       "source:\n"
                       "  line: 11\n"
                       "  file: \"test.cpp\"\n"
                       "dwarf:\n"
                       "  offset: 0x11\n"
                       "  tag: \"DW_TAG_GNU_template_parameter_pack\"\n"
                       "attributes:\n"
                       "  types:");
  EXPECT_EQ(Pack.getAsYAML(), Expected + std::string(" []"));

  auto *TempType = new TypeParam;
  Type Ty;
  Ty.setName("Ty");
  TempType->setType(&Ty);
  TempType->setIsTemplateType();
  Pack.addObject(TempType);

  Expected.append("\n    - \"Ty\"");
  EXPECT_EQ(Pack.getAsYAML(), Expected);

  auto *TempValue = new TypeParam;
  TempValue->setIsTemplateValue();
  TempValue->setValue("101");
  Pack.addObject(TempValue);

  Expected.append("\n    - 101");
  EXPECT_EQ(Pack.getAsYAML(), Expected);

  auto *TempTemp = new TypeParam;
  TempTemp->setIsTemplateTemplate();
  TempTemp->setValue("vector");
  Pack.addObject(TempTemp);

  Expected.append("\n    - \"vector\"");
  EXPECT_EQ(Pack.getAsYAML(), Expected);
}

TEST(Scope, getAsText_Union) {
  Reader R(nullptr);
  setReader(&R);

  ScopeAggregate Union;
  Union.setIsAggregate();
  Union.setIsUnionType();
  Union.setName("TestUnion");
  EXPECT_EQ(Union.getAsText(), "{Union} \"TestUnion\"");

  // See Object::getAttributeInfoAsText() for why the indent is this size.
  R.getOptions().resetFormatIndentation();
  std::string AttrIndent(3 + 4 + 8, ' ');
  AttrIndent += "- ";

  Union.setIsTemplate();
  EXPECT_EQ(Union.getAsText(), std::string("{Union} \"TestUnion\"\n") +
                                   AttrIndent + std::string("Template"));
}

TEST(Scope, getAsYAML_Union) {
  Reader R(nullptr);
  setReader(&R);

  ScopeAggregate Union;
  Union.setIsAggregate();
  Union.setIsUnionType();
  Union.setName("TestUnion");
  Union.setLineNumber(5);
  Union.setFileName("testunion.cpp");
  Union.setDieOffset(0xD3ADB33F);
  Union.setDieTag(DW_TAG_class_type);

  std::string Expected("object: \"Union\"\n"
                       "name: \"TestUnion\"\n"
                       "type: null\n"
                       "source:\n"
                       "  line: 5\n"
                       "  file: \"testunion.cpp\"\n"
                       "dwarf:\n"
                       "  offset: 0xd3adb33f\n"
                       "  tag: \"DW_TAG_class_type\"\n"
                       "attributes:\n"
                       "  is_template: ");
  EXPECT_EQ(Union.getAsYAML(), Expected + std::string("false"));

  Union.setIsTemplate();
  EXPECT_EQ(Union.getAsYAML(), Expected + std::string("true"));
}

TEST(Scope, notPrintedAsObject) {
  EXPECT_FALSE(ScopeArray().getIsPrintedAsObject());
  EXPECT_FALSE(ScopeRoot().getIsPrintedAsObject());
}
