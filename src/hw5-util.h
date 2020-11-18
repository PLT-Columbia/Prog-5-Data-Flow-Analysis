//
// Created by saikatc on 11/14/20.
//

#ifndef LLVM_HW5_UTIL_H
#define LLVM_HW5_UTIL_H
#pragma once

#include "tee.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"

#include <fstream>
#include <iostream>
#include <set>
#include <type_traits>
#include <vector>

using namespace std;
using namespace llvm;

// trim from start (in place)
static inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
    return !std::isspace(ch);
  }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
  ltrim(s);
  rtrim(s);
}

std::string getShortValueName(Value *v) {
  if (v->getName().str().length() > 0) {
    return "%" + v->getName().str() + "";
  } else if (isa<Instruction>(v)) {
    std::string s = "";
    raw_string_ostream *strm = new raw_string_ostream(s);
    v->print(*strm);
    std::string inst = strm->str();
    size_t idx1 = inst.find("%");
    size_t idx2 = inst.find(" ", idx1);
    if (idx1 != std::string::npos && idx2 != std::string::npos) {
      return inst.substr(idx1, idx2 - idx1);
    } else {
      return inst;
    }
  } else if (ConstantInt *cint = dyn_cast<ConstantInt>(v)) {
    std::string s = "";
    raw_string_ostream *strm = new raw_string_ostream(s);
    cint->getValue().print(*strm, true);
    return strm->str();
  } else {
    std::string s = "";
    raw_string_ostream *strm = new raw_string_ostream(s);
    v->print(*strm);
    std::string inst = strm->str();
    return inst;
  }
}

std::string instructionToString(Instruction *inst) {
  std::string s = "";
  raw_string_ostream *strm = new raw_string_ostream(s);
  inst->print(*strm);
  std::string instStr = strm->str();
  trim(instStr);
  return instStr;
}

class DataFlowWriter {
private:
  std::fstream file;
  Tee<std::fstream &, std::ostream &> tee;
  vector<Instruction *> allInstructions;
  int iterationCount;

  void printValues(map<Instruction *, set<Value *>> &vals, std::string name) {
    tee << "Printing " << name << "\n";
    tee << "================================================================\n";
    for (Instruction *inst : allInstructions) {
      set<Value *> valsForThisInstruction = vals[inst];
      tee << "" << instructionToString(inst) << "";
      tee << " "
          << "\n"
          << "\t-> " << name << ":\t[ ";
      for (Value *v : valsForThisInstruction) {
        tee << getShortValueName(v) << " , ";
      }
      tee << "]\n";
    }
    tee << "================================================================\n";
  }

public:
  DataFlowWriter(Function &function)
      : file(function.getName().str() + ".txt",
             std::ios::out | std::ios::trunc),
        tee(file, std::cout) {
    for (BasicBlock &basicBlocks : function) {
      for (Instruction &instruction : basicBlocks) {
        Instruction *instPtr = &instruction;
        allInstructions.emplace_back(instPtr);
      }
    }
    iterationCount = 0;
  }

  vector<Instruction *> &getAllInstructions() { return allInstructions; }

  void printDefs(map<Instruction *, set<Value *>> &def) {
    printValues(def, "DEF");
  }

  void printUses(map<Instruction *, set<Value *>> &use) {
    printValues(use, "USE");
  }

  void printLiveIns(map<Instruction *, set<Value *>> &ins) {
    printValues(ins, "LIVE-IN");
  }

  void printLiveOuts(map<Instruction *, set<Value *>> &outs) {
    printValues(outs, "LIVE-OUT");
  }

  void printIterationCount(int iteration) {
    tee << "Total Number of Iteration Needed : " << iteration << "\n";
  }

  void printDataFlowEdge(Instruction *from, Instruction *to, Value *v) {
    tee << "\"" << instructionToString(from) << "\" --- " << getShortValueName(v)
        << " ---> \"" << instructionToString(to) << "\"\n";
  }

  void updateIteration() { iterationCount++; }
};

#endif // LLVM_HW5_UTIL_H
