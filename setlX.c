#ifndef SETLX_H_
#define SETLX_H_

#define AIL_ALL_IMPL
#include "ail/ail.h"
#include <stdlib.h> // For exit
#include <stdio.h>  // For printf, fprintf, stderr

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
	switch (str.type) {
		case __enum_Type_Om:
			printf("om\n");
			break;
		case __enum_Type_Int:
			printf("%i\n", str.data.i);
			break;
		case __enum_Type_Float:
			printf("%f\n", str.data.f);
			break;
		case __enum_Type_Str:
			printf("%s\n", str.data.s);
			break;
	}
	return (__struct_Value){0};
}

char *__dbg_print_value(__struct_Value val)
{

}

__struct_Value __neg_value(__struct_Value val)
{
	switch (val.type) {
		case __enum_Type_Om:
		case __enum_Type_Str:
			fprintf(stderr, "Error in trying to negate %s\n", __dbg_print_value(val));
			exit(1);
		case __enum_Type_Int:
			return (__struct_Value) { .type = val.type, .data = { .i = -val.data.i }};
		case __enum_Type_Float:
			return (__struct_Value) { .type = val.type, .data = { .f = -val.data.f }};
	}
	AIL_UNREACHABLE();
}

__struct_Value __add_values(__struct_Value l, __struct_Value r)
{
	switch (l.type) {
		case __enum_Type_Om:
			fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
			exit(1);
		case __enum_Type_Int:
			switch (r.type) {
				case __enum_Type_Om:
					fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
					exit(1);
				case __enum_Type_Int:
					return (__struct_Value){ .type = __enum_Type_Int, .data =  { .i = l.data.i + r.data.i }};
				case __enum_Type_Float:
					return (__struct_Value){ .type = __enum_Type_Float, .data = { .f = ((float)l.data.i) + r.data.f }};
				case __enum_Type_Str:
					AIL_TODO();
			}
		case __enum_Type_Float:
			switch (r.type) {
				case __enum_Type_Om:
					fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
					exit(1);
				case __enum_Type_Int:
					return (__struct_Value){ .type = __enum_Type_Float, .data =  { .f = l.data.f + ((float)r.data.i) }};
				case __enum_Type_Float:
					return (__struct_Value){ .type = __enum_Type_Float, .data = { .f = l.data.f + r.data.f }};
				case __enum_Type_Str:
					AIL_TODO();
			}
		case __enum_Type_Str:
			AIL_TODO();
	}
}

__struct_Value __sub_values(__struct_Value l, __struct_Value r)
{
	switch (l.type) {
		case __enum_Type_Om:
			fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
			exit(1);
		case __enum_Type_Int:
			switch (r.type) {
				case __enum_Type_Om:
					fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
					exit(1);
				case __enum_Type_Int:
					return (__struct_Value){ .type = __enum_Type_Int, .data =  { .i = l.data.i - r.data.i }};
				case __enum_Type_Float:
					return (__struct_Value){ .type = __enum_Type_Float, .data = { .f = ((float)l.data.i) - r.data.f }};
				case __enum_Type_Str:
					AIL_TODO();
			}
		case __enum_Type_Float:
			switch (r.type) {
				case __enum_Type_Om:
					fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
					exit(1);
				case __enum_Type_Int:
					return (__struct_Value){ .type = __enum_Type_Float, .data =  { .f = l.data.f - ((float)r.data.i) }};
				case __enum_Type_Float:
					return (__struct_Value){ .type = __enum_Type_Float, .data = { .f = l.data.f - r.data.f }};
				case __enum_Type_Str:
					AIL_TODO();
			}
		case __enum_Type_Str:
			AIL_TODO();
	}
}

__struct_Value __mul_values(__struct_Value l, __struct_Value r)
{
	switch (l.type) {
		case __enum_Type_Om:
			fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
			exit(1);
		case __enum_Type_Int:
			switch (r.type) {
				case __enum_Type_Om:
					fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
					exit(1);
				case __enum_Type_Int:
					return (__struct_Value){ .type = __enum_Type_Int, .data =  { .i = l.data.i * r.data.i }};
				case __enum_Type_Float:
					return (__struct_Value){ .type = __enum_Type_Float, .data = { .f = ((float)l.data.i) * r.data.f }};
				case __enum_Type_Str:
					AIL_TODO();
			}
		case __enum_Type_Float:
			switch (r.type) {
				case __enum_Type_Om:
					fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
					exit(1);
				case __enum_Type_Int:
					return (__struct_Value){ .type = __enum_Type_Float, .data =  { .f = l.data.f * ((float)r.data.i) }};
				case __enum_Type_Float:
					return (__struct_Value){ .type = __enum_Type_Float, .data = { .f = l.data.f * r.data.f }};
				case __enum_Type_Str:
					AIL_TODO();
			}
		case __enum_Type_Str:
			AIL_TODO();
	}
}

__struct_Value __div_values(__struct_Value l, __struct_Value r)
{
	switch (l.type) {
		case __enum_Type_Om:
			fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
			exit(1);
		case __enum_Type_Int:
			switch (r.type) {
				case __enum_Type_Om:
					fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
					exit(1);
				case __enum_Type_Int:
					return (__struct_Value){ .type = __enum_Type_Float, .data =  { .f = ((float)l.data.i) / ((float)r.data.i) }};
				case __enum_Type_Float:
					return (__struct_Value){ .type = __enum_Type_Float, .data = { .f = ((float)l.data.i) / r.data.f }};
				case __enum_Type_Str:
					AIL_TODO();
			}
		case __enum_Type_Float:
			switch (r.type) {
				case __enum_Type_Om:
					fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
					exit(1);
				case __enum_Type_Int:
					return (__struct_Value){ .type = __enum_Type_Float, .data =  { .f = l.data.f / ((float)r.data.i) }};
				case __enum_Type_Float:
					return (__struct_Value){ .type = __enum_Type_Float, .data = { .f = l.data.f / r.data.f }};
				case __enum_Type_Str:
					AIL_TODO();
			}
		case __enum_Type_Str:
			AIL_TODO();
	}
}

#endif // SETLX_H_