; Declare the string constant as a global constant.
@.str = private unnamed_addr constant [9 x i8] c"my test \00"

; External declaration of the puts function
declare i32 @puts(ptr nocapture) nounwind

define i32 @main() {
  %1 = alloca i32, align 4
  %2 = alloca ptr, align 8
  store i32 0, ptr %1, align 4
  store ptr null, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = call ptr %3(ptr noundef null)
  ret i32 0
}