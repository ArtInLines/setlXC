; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.37.32824"

%struct.Value = type { i32, ptr }

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @test(ptr noalias sret(%struct.Value) align 8 %0, ptr noundef %1) #0 {
  %3 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %0, ptr align 8 %1, i64 16, i1 false)
  ret void
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca ptr, align 8
  %3 = alloca %struct.Value, align 8
  %4 = alloca ptr, align 8
  %5 = alloca %struct.Value, align 8
  %6 = alloca %struct.Value, align 8
  store i32 0, ptr %1, align 4
  store ptr null, ptr %2, align 8
  %7 = getelementptr inbounds %struct.Value, ptr %3, i32 0, i32 0
  store i32 0, ptr %7, align 8
  %8 = getelementptr inbounds %struct.Value, ptr %3, i32 0, i32 1
  %9 = load ptr, ptr %2, align 8
  store ptr %9, ptr %8, align 8
  store ptr null, ptr %4, align 8
  %10 = load ptr, ptr %4, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %6, ptr align 8 %3, i64 16, i1 false)
  call void %10(ptr sret(%struct.Value) align 8 %5, ptr noundef %6)
  ret i32 0
}

attributes #0 = { noinline nounwind optnone uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 2}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{!"clang version 16.0.0"}
