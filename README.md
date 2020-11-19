# COMS W4115: Programming Assignment 5 (Data Flow Analysis)

## Course Summary

Course: COMS 4115 Programming Languages and Translators (Fall 2020)  
Website: https://www.rayb.info/fall2020  
University: Columbia University.  
Instructor: Prof. Baishakhi Ray


## Logistics
* **Announcement Date:** Wednesday, November 18, 2020
* **Due Date:** Wednesday, December 2, 2020 by 11:59 PM. **No extensions!**
* **Total Points:** 100

## Grading Breakdown
* **Task 1 (Data Flow Graph Generation)**: 40
* **Task 2 (Liveness Analysis)**: 60
* **Extra Credit**: 20

## Assignment Objectives

From this assignment:

1. You **will learn** how to generate data flow graphs from the LLVM IR.
2. You **will learn** how to analyze the liveness of variables from the control and data flow graphs.

## Assignment

In the previous assignment on control flow analysis, you learned how to write an LLVM pass and how to use the pass to generate control flow graphs using `BasicBlock`s. In this assignment, you will learn how to perform *data flow analysis* given an LLVM IR. More specifically, you will learn about liveness analysis, which you already studied in lecture.


### Getting Started

1. Convert the `example.c` C program (from the `examples` directory) to an IR by running the following, just as you did in the previous assignment:
```
export LLVM_HOME="<the absolute path to llvm-project>";
export PATH="$LLVM_HOME/build/bin:$PATH";

clang -O1 -emit-llvm -c example.c
llvm-dis example.bc
```
You will now receive an `example.bc` file, which contains the IR in binary format. You will also see an `example.ll` file, which contains the IR in human-readable format.

2. Read through the `example.ll` file, and especially try to understand what variable(s) each instruction "defines" (`DEF`) and "uses" (`USE`). You should also view the CFG of the function `bubbleSort` in `bubble.c`. Similarly, you can view the CFGs of other functions via the `dot` command (*e.g.*, `dot -Tpdf .printArray.dot -o printArray.pdf` for the `printArray` function).

3. Create a directory `clang-hw5` in `llvm-project/llvm/lib/Transforms` for this assignment, and copy the files from the `src` directory to this new directory, as follows:

```
cp -r ./src/* "$LLVM_HOME/llvm/lib/Transforms/clang-hw5/"
```

4. Append `add_subdirectory(clang-hw5)` to the `$LLVM_HOME/llvm/lib/Transforms/CMakeLists.txt` file.

5. Build `clang-hw5` by running the following commands (you should do this every time you make changes):

```
cd "$LLVM_HOME/build"
make
```

After you successfully run `make` once, you can rebuild the project using `make LLVMliveness`.

In the [`runOnFunction`](src/Liveness.cpp#L152), we have provided you a few important items to help you get started:
1. An instruction-level CFG representation (using the [`extractControlFlowGraph`](src/Liveness.cpp#L43) function)
2. An initialization of the `DEF` and `USE` sets for every instruction
3. An initialization of the `LIVE_IN` and `LIVE_OUT` sets for every instruction


### Task 1: Data Flow Graph Generation (40 points)
In this task, you will generate a data flow graph. You will add your implementation inside the provided [`generateDataFlowGraph`](src/Liveness.cpp#L143) function. A *data flow edge* is defined as an edge from instruction `FROM` to instruction `TO` if a variable (`VALUE`) is "defined" in instruction `FROM` and "used" in Instruction `TO`. When you find such an edge, please use the following code to print out your edge:

```C
writer.printDataFlowEdge(FROM, TO, VALUE);
```

You can run the pass you created as follows:

```
opt -load $LLVM_HOME/build/lib/LLVMliveness.so -liveness < example.bc
```

As a sanity check, you can compare the edges you found with those in the `example.ll` file that you generated earlier. If you are able to successfully identify all of the data flow edges, Task 1 will be complete.

### Task 2: Liveness Analysis (60 points)

As you have already learned about liveness analysis, there are data flow equations that you can solve to determine liveness of variables at every node. You will now have the opportunity to solve these data flow equations by implementing the algorithm! Because solving data flow equations is an iterative process, we have provided for you the skeleton code for the [data flow iteration](src/Liveness.cpp#L193). Inside the `while` loop, you will need to solve the data flow equations and update the `LIVE_IN` and `LIVE_OUT` sets after visiting every instruction. The loop is controlled by a Boolean variable called `possibleToUpdate`. Modify this variable as necessary (*i.e.*, set this variable to "false" when you determine that your algorithm has converged).

Here is the grading breakdown for Task 2:
* **Live-in sets**: 30
* **Live-out sets**: 30

### Extra Credit (20 points)

In the skeleton loop, we log the number of iterations required for data flow analysis convergence. Let `C` represent the *minimum* number of iterations needed for the algorithm to converge. It is your task to determine what `C` is; your answer may be within 1 iteration of `C`, *i.e.*, you will get full credit if you correctly output `C - 1`, `C`, or `C + 1`. Note that since this is an extra credit task, we will not provide any hints as to what `C` may be. However, if you have been paying close attention to lecture and have experimented a bit already with tasks 1 and 2 of this assignment, you should be able to easily solve this extra credit task.

### Important Notes
1. Please **DO NOT** remove or modify any of the existing code (except [this line](src/Liveness.cpp#L205)).
2. Place all of your code in the sections that we have outlined for you. You may include helper functions as necessary, but please make sure to put them inside the [src/Liveness.cpp](src/Liveness.cpp) file. Keep in mind that we will only use this file from your submission for grading. 
3. Carefully read through the steps highlighted in this README, as well as the TODOs and other comments in the code. You will find useful hints.
4. Pay close attention to the `DEF`, `USE`, `LIVE_IN`, and `LIVE_OUT` data structures.
5. `PHINode` is a special type of node used in the [single static assignment](https://en.wikipedia.org/wiki/Static_single_assignment_form) of variables; it selectively chooses a value based on the predecessor `BasicBlock` it comes from.

Consider the following example:
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
In line 7, while both `%div` and `%e` appear to both be used, exactly one of them is actually used. If the chosen predecessor `BasicBlock` of this instruction is the `while.body` `BasicBlock`, then `%div` is used. Otherwise, the chosen predecessor `BasicBlock` must be the `entry` `BasicBlock`, and `%e` is used as the value for `%e.addr.08`.

Thus, we extract all the uses corresponding to the `PHINode` in a special data structure called `PHI_USE`. Look closely at this data structure, and use it carefully in your code. You may find the following are useful resources for `PHINode`:

 1. [http://mayuyu.io/2018/06/04/PhiNode-in-LLVM/](http://mayuyu.io/2018/06/04/PhiNode-in-LLVM/)
 2. [https://llvm.org/doxygen/classllvm_1_1PHINode.html](https://llvm.org/doxygen/classllvm_1_1PHINode.html)

## Submission
Only one file should be submitted: `src/Liveness.cpp`. Other files will be ignored during grading. Please make sure that your code is properly committed and pushed to the `master` branch.

## Piazza
If you have any questions about this programming assignment, please post them in the Piazza forum for the course, and an instructor will reply to them as soon as possible. Any updates to the assignment itself will be available in Piazza.

## Disclaimer
This assignment belongs to Columbia University. It may be freely used for educational purposes.
