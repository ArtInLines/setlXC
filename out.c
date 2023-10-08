#include <stdio.h>

typedef enum {
	__enum_Type_Om = 0,
	__enum_Type_Int,
	__enum_Type_Float,
	__enum_Type_Str,
} __struct_Type;
typedef struct {
	__struct_Type type;
	union {
		void  *x;
		int    i;
		float  f;
		char  *s;
	} data;
} __struct_Value;

__struct_Value __stlx_print(__struct_Value str)
{
	puts(str.data.s);
	return (__struct_Value){0};
}

int main(void)
{
	__struct_Value __stlx_x = {.type = __enum_Type_Str, .data = {.s = "Hello World"}};
	__struct_Value __stlx_0 = __stlx_print((__struct_Value)__stlx_x);
	return 0;
}

