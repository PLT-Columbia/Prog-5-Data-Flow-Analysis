# COMS W4115: Programming Assignment 5 (Data Flow Analysis)

## Course Summary

Course: COMS 4115 Programming Languages and Translators (Fall 2020)  
Website: https://www.rayb.info/fall2020  
University: Columbia University.  
Instructor: Prof. Baishakhi Ray


## Logistics
* **Announcement Date:** Wednesday, November 18, 2020
* **Due Date:** **(TBD)**Wednesday, December 2, 2020 by 11:59 PM. **No extensions!**
* **Total Points:** **(TBD)**100

## Grading Breakdown
* Task 1 - Generating Data Flow Graph - 40
* Task 2 - Liveness analysis - 60 (Live in - 30, Live out - 30)
* Extra Credit - Minimum number of iteration in liveness analysis - 20

## Assignment Objectives

From this assignment:

1. You **will learn** how to generate data flow graphs from LLVM IR.
2. You **will learn** how to analyze the variable liveness from control and data flow graph.

## Assignment

In previous assignments for control flow analysis, you learned how to write a LLVM pass and how to use LLVM pass to
generate control flow graph with `BasicBlock`s. In this assignment, you will learn about how to do Data Flow Analysis 
from an LVM IR. More specifically, you will learn about Liveness analysis, which you already studied in the class. 


### Get started

1. Convert `example.c` from the `examples` directory to IR:
```
LLVM_HOME="<the absolute path to llvm-pproject>"
export PATH="$LLVM_HOME/build/bin:$PATH"

clang -O1 -emit-llvm -c example.c
llvm-dis example.bc
```
You will get `example.bc`, which contains the IR in binary format, and `example.ll`, which contains the IR in human-readable format.

2. Read through the `example.ll` and understand what variable each of the instruction Defined (`DEF`) and Used(`USE`).
Then you can view the CFG of `bubbleSort()` in `bubble.c`. Obviously you can view the CFG of other functions with `dot` 
command, e.g. `dot -Tpdf .printArray.dot -o printArray.pdf`.

3. Create a new folder `clang-hw5` in `llvm-project/llvm/lib/Transforms` for this assignment, and copy the files:
```
cp -r ./src/* "$LLVM_HOME/llvm/lib/Transforms/clang-hw5/"
```

4. Append `add_subdirectory(clang-hw5)` to `$LLVM_HOME/llvm/lib/Transforms/CMakeLists.txt`.

5. Build `clang-hw5`:
```
cd "$LLVM_HOME/build"
make
```
After you successfully run `make` once, you can rebuilt the assignment using `make LLVMliveness`. 


In the [`runOnFunction`](src/Liveness.cpp#L156), we have given you a few things,
1. Instruction Level Control Flow Graph (using [`extractControlFlowGraph` function](src/Liveness.cpp#L43)).
2. Extracted the `DEF` and the `USE` set for every instruction. 
3. Initialized the `LIVE_IN` and `LIVE_OUT` set for every instruction.


### Part 1: Generate Data Flow Graph
Implement the Data Flow Graph Generation inside [`generateDataFlowGraph`](src/Liveness.cpp#L147). A data flow edge is 
defined as an edge from  Instruction `FROM` to Instruction `TO`, if a Variable (Value) (`VALUE`) is defined in Instruction 
`FROM` and used in Instruction `TO`. When you file such an edge, you have to call 
```C
writer.printDataFlowEdge(FROM, TO, VALUE);
```
This part of your assignment will be complete when you can successfully identify all the data flow edges. 

You can run the pass you wrote this way:
```
opt -load $LLVM_HOME/build/lib/LLVMliveness.so -liveness < example.bc
```

Compare the edges you found with `example.ll` that you found in Step1.

### Part 2: Liveness Analysis

As you have already learned about the liveness analysis, you are expected to know how to solve the data flow equations 
for liveness analysis. Since, data flow equations are iterative process, we have provided the skeleton for the [data 
flow iteration](src/Liveness.cpp#L199). Inside the `while` loop, you have to solve the data flow equation and 
update the `LIVE_IN` and `LIVE_OUT` sets for every instruction.  The loop is controlled by a boolean variable 
`possibleToUpdate`. Modify this variable as necessary (_i.e._ set this variable to false, when you determine that
your analysis converged).

### Extra Credit

In the skeleton loop, we are logging the number of iterations needed to converge. You will automatically get 20 points 
as extra credit, if you can make the analysis converge in `minimum_iterations +- 1`. Note that, we will not provide any
hint about what is the minimum number of iterations needed. You can easily achieve this extra credit close paying 
attention to the class lecture and some experiment with this assignment. 

### Things to be careful about
1. Please **DO NOT** remove or modify (except [this line](src/Liveness.cpp#l213)) any of the existing code. 
2. Put all your necessary code in designated positions. You may write helper function, but please put them inside the 
[src/Liveness.cpp](src/Liveness.cpp) file. We will just use this file from your submission for grading. 
3. Read through the steps in this README and TODOs (also other comments) in the code. You will find useful hints 
if you read carefully.
4. Pay close attention to the data structures of `DEF`, `USE`, `IN`, `OUT`. 
5. `PHINode` is a special type of node used in [Single Static Assignment](https://en.wikipedia.org/wiki/Static_single_assignment_form) 
of variables, which selectively uses a value based on which `BasicBlock it is coming from. 
Consider the following example.
```
 1. entry:
 2.   %cmp6 = icmp slt i32 %a, %e
 3.   br i1 %cmp6, label %while.body, label %while.end
 4. 
 5. while.body:                                       ; preds = %entry, %while.body
 6.   %res.09 = phi i32 [ %add, %while.body ], [ 0, %entry ]
 7.   %e.addr.08 = phi i32 [ %div, %while.body ], [ %e, %entry ]
 8.   %a.addr.07 = phi i32 [ %inc, %while.body ], [ %a, %entry ]
 9.   %add = add nsw i32 %res.09, %e.addr.08
10.   %inc = add nsw i32 %a.addr.07, 1
11.   %div = sdiv i32 %e.addr.08, 2
12.   %cmp = icmp slt i32 %inc, %div
13.   br i1 %cmp, label %while.body, label %while.end, !llvm.loop !2
14. 
15. while.end:                                        ; preds = %while.body, %entry
16.   %res.0.lcssa = phi i32 [ 0, %entry ], [ %add, %while.body ]
17.   ret i32 %res.0.lcssa
```  
In line 7, while both `%div`, and `%e` seems to be used, only on of them are actually used. If the predecessor 
`BasicBlock` this instruction (_i.e._ `while.body` BasicBlock) is `while.body`, then `%div` is used, otherwise 
(when the predecessor is `entry` BasicBlock), `%e` is used. 
Thus, we extracted all the uses corresponding to the `PHINode` in a special data structure `PHI_USE`. Look closely at the
data structure, and use it carefully in your code. You may find the following links are useful resources for `PHINode`.
    a. [http://mayuyu.io/2018/06/04/PhiNode-in-LLVM/](http://mayuyu.io/2018/06/04/PhiNode-in-LLVM/)
    b. [https://llvm.org/doxygen/classllvm_1_1PHINode.html](https://llvm.org/doxygen/classllvm_1_1PHINode.html)

## Submission
Only one file should be submitted: `src/Liveness.cpp`. Other files will be ignored when grading.