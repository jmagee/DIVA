//===-- UnitTests/TestLibScopeView/TestLine.cpp -----------------*- C++ -*-===//
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
/// Tests for LibScopeView::Line.
///
//===----------------------------------------------------------------------===//

#include "Line.h"
#include "Reader.h"

#include "gtest/gtest.h"

using namespace LibScopeView;

TEST(Line, getAsText_Line) {
  Reader R(nullptr);
  setReader(&R);

  Line Ln(0);
  Ln.setIsLineRecord();
  EXPECT_EQ(Ln.getAsText(), "{CodeLine}");
}

TEST(Line, getAsYAML_Line) {
  Reader R(nullptr);
  setReader(&R);

  Line Ln;
  Ln.setIsLineRecord();
  Ln.setLineNumber(52);
  Ln.setFileName("test.cpp");
  Ln.setDieOffset(0x5555);

  EXPECT_EQ(Ln.getAsYAML(), "object: \"CodeLine\"\n"
                            "name: null\n"
                            "type: null\n"
                            "source:\n"
                            "  line: 52\n"
                            "  file: \"test.cpp\"\n"
                            "dwarf:\n"
                            "  offset: 0x5555\n"
                            "  tag: null\n"
                            "attributes:\n"
                            "  NewStatement: false\n"
                            "  PrologueEnd: false\n"
                            "  EndSequence: false\n"
                            "  BasicBlock: false\n"
                            "  Discriminator: false\n"
                            "  EpilogueBegin: false");
}

TEST(Line, getAsText_Line_Attributes) {
  Reader R(nullptr);
  R.getOptions().setFormatIndentation();
  R.getOptions().setPrintCodelineAttributes();
  setReader(&R);

  Line Ln(/*level*/ 3);
  Ln.setIsLineRecord();

  // See Object::getAttributeInfoAsText() for why the indent is this size.
  std::string AttrIndent(3 + 4 + 8 + ((Ln.getLevel() + 1) * 2), ' ');

  Ln.setIsNewStatement();
  std::string Expected(
    std::string("{CodeLine}\n") + AttrIndent + std::string("- NewStatement"));
  EXPECT_EQ(Ln.getAsText(), Expected);

  Ln.setIsPrologueEnd();
  Expected += '\n' + AttrIndent + std::string("- PrologueEnd");
  EXPECT_EQ(Ln.getAsText(), Expected);

  Ln.setIsLineEndSequence();
  Expected += '\n' + AttrIndent + std::string("- EndSequence");
  EXPECT_EQ(Ln.getAsText(), Expected);

  Ln.setIsNewBasicBlock();
  Expected += '\n' + AttrIndent + std::string("- BasicBlock");
  EXPECT_EQ(Ln.getAsText(), Expected);

  Ln.setHasDiscriminator();
  Expected += '\n' + AttrIndent + std::string("- Discriminator");
  EXPECT_EQ(Ln.getAsText(), Expected);

  Ln.setIsEpilogueBegin();
  Expected += '\n' + AttrIndent + std::string("- EpilogueBegin");
  EXPECT_EQ(Ln.getAsText(), Expected);
}

TEST(Line, getAsYAML_Line_Attributes) {
  Reader R(nullptr);
  setReader(&R);

  Line Ln;
  Ln.setIsLineRecord();
  Ln.setLineNumber(52);
  Ln.setFileName("test.cpp");
  Ln.setDieOffset(0x5555);

  std::string Expected("object: \"CodeLine\"\n"
                       "name: null\n"
                       "type: null\n"
                       "source:\n"
                       "  line: 52\n"
                       "  file: \"test.cpp\"\n"
                       "dwarf:\n"
                       "  offset: 0x5555\n"
                       "  tag: null\n"
                       "attributes:");

  Ln.setIsNewStatement();
  EXPECT_EQ(Ln.getAsYAML(), Expected + (
    "\n  NewStatement: true"
    "\n  PrologueEnd: false"
    "\n  EndSequence: false"
    "\n  BasicBlock: false"
    "\n  Discriminator: false"
    "\n  EpilogueBegin: false"
  ));

  Ln.setIsPrologueEnd();
  EXPECT_EQ(Ln.getAsYAML(), Expected + (
    "\n  NewStatement: true"
    "\n  PrologueEnd: true"
    "\n  EndSequence: false"
    "\n  BasicBlock: false"
    "\n  Discriminator: false"
    "\n  EpilogueBegin: false"
  ));

  Ln.setIsLineEndSequence();
  EXPECT_EQ(Ln.getAsYAML(), Expected + (
    "\n  NewStatement: true"
    "\n  PrologueEnd: true"
    "\n  EndSequence: true"
    "\n  BasicBlock: false"
    "\n  Discriminator: false"
    "\n  EpilogueBegin: false"
  ));

  Ln.setIsNewBasicBlock();
  EXPECT_EQ(Ln.getAsYAML(), Expected + (
    "\n  NewStatement: true"
    "\n  PrologueEnd: true"
    "\n  EndSequence: true"
    "\n  BasicBlock: true"
    "\n  Discriminator: false"
    "\n  EpilogueBegin: false"
  ));

  Ln.setHasDiscriminator();
  EXPECT_EQ(Ln.getAsYAML(), Expected + (
    "\n  NewStatement: true"
    "\n  PrologueEnd: true"
    "\n  EndSequence: true"
    "\n  BasicBlock: true"
    "\n  Discriminator: true"
    "\n  EpilogueBegin: false"
  ));

  Ln.setIsEpilogueBegin();
  EXPECT_EQ(Ln.getAsYAML(), Expected + (
    "\n  NewStatement: true"
    "\n  PrologueEnd: true"
    "\n  EndSequence: true"
    "\n  BasicBlock: true"
    "\n  Discriminator: true"
    "\n  EpilogueBegin: true"
  ));
}