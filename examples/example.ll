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
