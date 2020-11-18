#include "hw5-util.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"

#include <algorithm>
#include <map>
#include <set>
#include <vector>

using namespace llvm;
using namespace std;

struct Liveness : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid

  Liveness() : FunctionPass(ID) {}

  set<Value *> setUnion(set<Value *> &a, set<Value *> &b) {
    set<Value *> result;
    std::set_union(a.begin(), a.end(), b.begin(), b.end(),
                   std::inserter(result, result.begin()));
    return result;
  }

  set<Value *> setIntersection(set<Value *> &a, set<Value *> &b) {
    set<Value *> result;
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(),
                          std::inserter(result, result.begin()));
    return result;
  }

  set<Value *> setDifference(set<Value *> &a, set<Value *> &b) {
    set<Value *> result;
    std::set_difference(a.begin(), a.end(), b.begin(), b.end(),
                        std::inserter(result, result.begin()));
    return result;
  }

  map<Instruction *, vector<Instruction *>>
  extractControlFlowGraph(Function &F) {
    map<Instruction *, vector<Instruction *>> controlFlowGraph;
    Instruction *preI = nullptr;
    for (BasicBlock &BB : F) {
      const Instruction *TInst = BB.getTerminator();
      for (Instruction &I : BB) {
        Instruction *pI = &I;
        if (preI != nullptr) {
          vector<Instruction *> successors;
          if (pI == TInst) {
            for (int i = 0, nSucc = TInst->getNumSuccessors(); i < nSucc; i++) {
              BasicBlock *succ = TInst->getSuccessor(i);
              successors.push_back(&succ->front());
            }
            controlFlowGraph.insert({pI, successors});
          }
          successors.push_back(pI);
          controlFlowGraph.insert({preI, successors});
        }
        preI = pI;
      }
    }
    return controlFlowGraph;
  }

  /**
   * This function populates the initial values of different data flows
   * ingredients. For every instruction, if it defines anything, `def` will
   * contain that Value `use` will contain all the values that are used in an
   * instruction. Initially, both `in` and `out` are populated with empty set.
   * `phiUse` and `phiIn` are two special cases.
   * Not all the values are used (and thus `in`) for a `PHINode`.
   * `PHINode` only uses the values from the basicblock it is coming from.
   * Thus, for every instruction, `phiUse`, and `phiIN` contains a map
   * From the incoming `BasicBlock*` to the incoming `Value *` set.
   * Note that, all the parameters are pointers in this function.
   * Thus the changes we are making in this function (i.e. we are populating the
   * values) Will be reflected to the original maps from where it is being
   * ccalled.
   */
  void populateInitialValues(
      vector<Instruction *> &allInstructions,
      map<Instruction *, set<Value *>> &def,
      map<Instruction *, set<Value *>> &use,
      map<Instruction *, set<Value *>> &in,
      map<Instruction *, set<Value *>> &out,
      map<PHINode *, map<BasicBlock *, set<Value *>>> &phiUse,
      map<PHINode *, map<BasicBlock *, set<Value *>>> &phiIn) {
    for (Instruction *pI : allInstructions) {
      Value *p = dyn_cast<Value>(pI);
      set<Value *> s;
      if (!isa<BranchInst>(pI) && !isa<ReturnInst>(pI)) {
        s.insert(p);
      }
      def[pI] = s;
      in[pI] = set<Value *>();
      out[pI] = set<Value *>();

      set<Value *> s2;
      for (auto opnd = pI->op_begin(); opnd != pI->op_end(); ++opnd) {
        Value *val = *opnd;
        if (isa<Instruction>(val) || isa<Argument>(val)) {
          s2.insert(val);
        }
      }
      use[pI] = s2;
      // handle PHI NODE
      if (isa<PHINode>(pI)) {
        PHINode *phiInstruction = dyn_cast<PHINode>(pI);
        map<BasicBlock *, set<Value *>> tempUseMap;
        map<BasicBlock *, set<Value *>> tempInMap;
        for (unsigned int ind = 0; ind < phiInstruction->getNumIncomingValues();
             ind++) {
          set<Value *> tempSet;
          Value *val = phiInstruction->getIncomingValue(ind);
          if (isa<Instruction>(val) || isa<Argument>(val)) {
            BasicBlock *valBlock = phiInstruction->getIncomingBlock(ind);
            if (tempInMap.find(valBlock) == tempInMap.end()) {
              tempInMap[valBlock] = set<Value *>();
            }
            if (tempUseMap.find(valBlock) == tempUseMap.end()) {
              tempSet.insert(val);
              tempUseMap[valBlock] = tempSet;
            } else {
              tempUseMap[valBlock].insert(val);
            }
          }
        }
        phiUse[phiInstruction] = tempUseMap;
        phiIn[phiInstruction] = tempInMap;
      }
    }
  }

  /*
   * The following function is for generating the data flow graph.
   * When you find a data flow edge from an
   * Instruction *FROM to a Instruction *TO
   * That flows the Value *VALUE, call
   * writer.printDataFlowEdge(FROM, TO, VALUE);
   * To print out the edge.
   */
  void
  generateDataFlowGraph(vector<Instruction *> allInstructions,
                        map<Instruction *, set<Value *>> DEF,
                        map<Instruction *, set<Value *>> USE,
                        map<PHINode *, map<BasicBlock *, set<Value *>>> PHI_USE,
                        DataFlowWriter &writer) {


  }

  bool runOnFunction(Function &F) override {
    DataFlowWriter writer(F);
    vector<Instruction *> allInstructions = writer.getAllInstructions();
    /* The Flow graph is represented as adjacency list.
     * For every Instruction in the graph, controlFlowGraph contains a vector
     * Of its neighboring Instruction (only outgoing edges). */
    map<Instruction *, vector<Instruction *>> controlFlowGraph =
        extractControlFlowGraph(F);

    map<Instruction *, set<Value *>> USE;
    map<Instruction *, set<Value *>> DEF;
    map<PHINode *, map<BasicBlock *, set<Value *>>> PHI_USE;
    map<Instruction *, set<Value *>> IN;
    map<Instruction *, set<Value *>> OUT;
    map<PHINode *, map<BasicBlock *, set<Value *>>> PHI_IN;
    populateInitialValues(
        allInstructions, DEF, USE, IN, OUT, PHI_USE, PHI_IN);

    /* These two printers show the DEFs and USEs of every instruction.
     * You can use these two function to see the values of DEF and USE.
     * If you do, make sure to comment out these 2 lines before submission.*/
    // writer.printDefs(DEF);
    // writer.printUses(USE);

    /* Task 1 - Finding the Data Flow Graph */
    generateDataFlowGraph(allInstructions, DEF, USE, PHI_USE, writer);

    /* Task 2 - Identifying the Live-in and Live-out variables for every
     * instruction */
    int iterationCount = 0;
    bool possibleToUpdate = true;
    /* If you change anything in the existing code, and the output format
     * Does not match, your assignment might be penalized.*/

    /*
     * Initially, we assume that dataflow update is possible.
     * We control the update loop using the variable `possibleToUpdate`.
     * If you find that no further update is possible,
     * At the beginning of the loop, we set `possibleToUpdate = false`.
     * Inside the dataflow analysis, if you find that update is still possible
     * Set the variable `possibleToUpdate = true` to keep the data flow loop
     * going.
     */
    while (possibleToUpdate) {
      possibleToUpdate = false;
      iterationCount++;
      writer.updateIteration();
      /* TODO: Add the code for solving data flow equation.
       * Please do not delete any line of code from this function.
       * You can add code as needed.
       * The dataflow equations are as follows (in no particular order)
       * in[n] = use[n] ∪ (out[n] – def[n])
       * out[n] = ∪(s in succ) {in[s]}
       * TODO: Consider `PHINode` specially.
       * Carefully consider the structure of `PHI_USE`, and `PHI_IN` maps.
       * We will provide NO further hint about how to process the PHINode.
       * Please read through the documentation. */
    }
    /* Please do not modify the following lines. */
    writer.printLiveIns(IN);
    writer.printLiveOuts(OUT);
    writer.printIterationCount(iterationCount);
    return true;
  }
};

char Liveness::ID = 0;
static RegisterPass<Liveness> X("liveness", "My Liveness Set Pass");