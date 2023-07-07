; Declare the string constant as a global constant.
@.str = private unnamed_addr constant [9 x i8] c"my test \00"

; External declaration of the puts function
declare i32 @puts(ptr nocapture) nounwind

define i32 @main() {
entry:
	; %retval = alloca i32, align 4
	%z = alloca i32, align 4
	%y = alloca i32, align 4
	%x = alloca i32, align 4
	; store i32 0, i32* %retval, align 4
	store i32 7, i32* %y, align 4
	%0 = load i32, i32* %y, align 4
	%add = add nsw i32 %0, 2
	store i32 %add, i32* %x, align 4
	call i32 @puts(ptr @.str)
	%2 = load i32, i32* %x, align 4
	%3 = add nsw i32 %2, 48
	store i32 %3, i32* %z, align 4
	call i32 @puts(ptr %z)
	ret i32 0
}