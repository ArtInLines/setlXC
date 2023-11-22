#ifndef SETLX_H_
#define SETLX_H_

#define AIL_ALL_IMPL
#include "ail/ail.h"
#include <stdlib.h> // For exit, memcpy (used by transpilatio of Range)
#include <stdio.h>  // For printf, fprintf, stderr

typedef enum {
	__enum_Type_Om = 0,
	__enum_Type_Int,
	__enum_Type_Float,
	__enum_Type_Str,
	__enum_Type_List,
} __struct_Type;

typedef struct {
	__struct_Type type;
	union {
		void  *x;
		int    i;
		float  f;
		char  *s;
		AIL_DA(char) l;
	} data;
} __struct_Value;
AIL_DA_INIT(__struct_Value);

void __stlx_print_helper(__struct_Value val)
{
	switch (val.type) {
		case __enum_Type_Om:
			printf("om");
			break;
		case __enum_Type_Int:
			printf("%i", val.data.i);
			break;
		case __enum_Type_Float:
			printf("%f", val.data.f);
			break;
		case __enum_Type_Str:
			printf("%s", val.data.s);
			break;
		case __enum_Type_List:
			printf("[");
			__struct_Value *l = (__struct_Value *) val.data.l.data;
			for (u32 i = 0; i < val.data.l.len; i++) {
				if (i > 0) printf(", ");
				__stlx_print_helper(l[i]);
			}
			printf("]");
			break;
	}
}

__struct_Value __stlx_print(__struct_Value val)
{
	__stlx_print_helper(val);
	printf("\n");
	return (__struct_Value){0};
}

char *__dbg_print_value(__struct_Value val)
{
	AIL_TODO();
}

u32 __stlx_get_idx(__struct_Value idx, __struct_Value list)
{
	if (idx.type != __enum_Type_Int || list.type != __enum_Type_List) AIL_UNREACHABLE();
	if (AIL_UNLIKELY(idx.data.i == 0)) AIL_UNREACHABLE();
	u32 out;
	if (idx.data.i < 0) out = list.data.l.len + idx.data.i;
	else out = idx.data.i - 1;
	if (AIL_UNLIKELY(out >= list.data.l.len)) AIL_UNREACHABLE();
	return out;
}

typedef struct {
	bool can_cmp;
	i32  res;
} __struct_Cmp_Res;

__struct_Cmp_Res __cmp_values(__struct_Value x, __struct_Value y)
{
	const static __struct_Cmp_Res EQ = { .can_cmp = true, .res = 0 };
	const static __struct_Cmp_Res NO_CMP = { .can_cmp = false, .res = -1 };
	bool eq_type = x.type == y.type;
	switch (x.type) {
		case __enum_Type_Om:
			return (__struct_Cmp_Res) {
				.can_cmp = false,
				.res     = eq_type ? 0 : -1,
			};
		case __enum_Type_Int:
			if (y.type == __enum_Type_Float) {
				return __cmp_values((__struct_Value) {.type = __enum_Type_Float, .data = {.f = (float)x.data.i}}, y);
			} else if (eq_type) {
				return (__struct_Cmp_Res) {.can_cmp = true, .res = x.data.i - y.data.i};
			} else {
				return NO_CMP;
			}
		case __enum_Type_Float:
			if (y.type == __enum_Type_Int) {
				return __cmp_values(x, (__struct_Value) {.type = __enum_Type_Float, .data = {.f = (float)y.data.i}});
			} else if (eq_type) {
				__struct_Cmp_Res out = { .can_cmp = true };
				f32 res = x.data.f - y.data.f;
				if (res < 0) out.res = -1;
				else if (res > 0) out.res = 1;
				return out;
			} else {
				return NO_CMP;
			}
		case __enum_Type_List:
			if (!eq_type) return NO_CMP;
			AIL_DA(char) xl = x.data.l;
			AIL_DA(char) yl = y.data.l;
			__struct_Cmp_Res out = { .can_cmp = false, .res = 0 };
			if (xl.len != yl.len) out.res = -1;
			else for (u32 i = 0; i < xl.len && out.res == 0; i++) {
				out.res = __cmp_values(((__struct_Value *)xl.data)[i], ((__struct_Value *)yl.data)[i]).res;
			}
			return out;
		case __enum_Type_Str:
			return eq_type && (strcmp(x.data.s, y.data.s) == 0) ? EQ : NO_CMP;
	}
}

__struct_Value __eq_values(__struct_Value x, __struct_Value y)
{
	__struct_Value out = { .type = __enum_Type_Int, .data = {.i = 0} };
	if (__cmp_values(x, y).res == 0) out.data.i = 1;
	return out;
}

__struct_Value __le_values(__struct_Value x, __struct_Value y)
{
	__struct_Value out = { .type = __enum_Type_Int, .data = {.i = 0} };
	__struct_Cmp_Res cmp = __cmp_values(x, y);
	if (!cmp.can_cmp) AIL_UNREACHABLE();
	if (cmp.res <= 0) out.data.i = 1;
	return out;
}

__struct_Value __lt_values(__struct_Value x, __struct_Value y)
{
	__struct_Value out = { .type = __enum_Type_Int, .data = {.i = 0} };
	__struct_Cmp_Res cmp = __cmp_values(x, y);
	if (!cmp.can_cmp) AIL_UNREACHABLE();
	if (cmp.res < 0) out.data.i = 1;
	return out;
}

__struct_Value __ge_values(__struct_Value x, __struct_Value y)
{
	__struct_Value out = { .type = __enum_Type_Int, .data = {.i = 0} };
	__struct_Cmp_Res cmp = __cmp_values(x, y);
	if (!cmp.can_cmp) AIL_UNREACHABLE();
	if (cmp.res >= 0) out.data.i = 1;
	return out;
}

__struct_Value __gt_values(__struct_Value x, __struct_Value y)
{
	__struct_Value out = { .type = __enum_Type_Int, .data = {.i = 0} };
	__struct_Cmp_Res cmp = __cmp_values(x, y);
	if (!cmp.can_cmp) AIL_UNREACHABLE();
	if (cmp.res > 0) out.data.i = 1;
	return out;
}

__struct_Value __neg_value(__struct_Value val)
{
	switch (val.type) {
		case __enum_Type_Om:
		case __enum_Type_Str:
		case __enum_Type_List:
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
		case __enum_Type_List:
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
		case __enum_Type_List:
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
		case __enum_Type_List:
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
		case __enum_Type_List:
			AIL_TODO();
	}
}

#endif // SETLX_H_