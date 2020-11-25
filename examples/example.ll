; ModuleID = 'example.bc'
source_filename = "example.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readnone uwtable
define dso_local i32 @sum(i32 %e) local_unnamed_addr #0 {
entry:
  %cmp7 = icmp sgt i32 %e, 0
  br i1 %cmp7, label %while.body, label %while.end

while.body:                                       ; preds = %entry, %while.body
  %res.08 = phi i32 [ %spec.select, %while.body ], [ 0, %entry ]
  %0 = and i32 %res.08, 1
  %cmp1 = icmp eq i32 %0, 0
  %add = select i1 %cmp1, i32 %e, i32 0
  %spec.select = add nsw i32 %add, %res.08
  %cmp = icmp slt i32 %spec.select, %e
  br i1 %cmp, label %while.body, label %while.end, !llvm.loop !2

while.end:                                        ; preds = %while.body, %entry
  %res.0.lcssa = phi i32 [ 0, %entry ], [ %spec.select, %while.body ]
  ret i32 %res.0.lcssa
}

attributes #0 = { norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0 (https://github.com/llvm/llvm-project.git 915310bf14cbac58a81fd60e0fa9dc8d341108e2)"}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.unroll.disable"}
