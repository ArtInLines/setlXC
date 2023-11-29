#ifndef SETLX_H_
#define SETLX_H_

#define AIL_ALL_IMPL
#include "ail/ail.h"
#include <stdlib.h> // For exit, memcpy (used by transpilatio of Range)
#include <stdio.h>  // For printf, fprintf, stderr

typedef enum {
	__enum_Type_Om = 0,
	__enum_Type_Bool,
	__enum_Type_Int,
	__enum_Type_Float,
	__enum_Type_Str,
	__enum_Type_List,
	__enum_Type_Proc,
} __struct_Type;

typedef struct {
	__struct_Type type;
	union {
		void  *x;
		bool   b;
		int    i;
		float  f;
		char  *s;
		AIL_DA(char) l;
	} data;
} __struct_Value;
AIL_DA_INIT(__struct_Value);

typedef __struct_Value (*__funcptr)(u8 argc, __struct_Value *argv);

char *__dbg_print_value(__struct_Value val);
char *__builtin_stlx_get_type_str(__struct_Value val);
char *__builtin_stlx_print_helper(__struct_Value val);

__funcptr __builtin_stlx_get_funcptr(__struct_Value val)
{
	if (val.type != __enum_Type_Proc) AIL_UNREACHABLE();
	return (__funcptr) val.data.x;
}

char *__builtin_stlx_print_helper(__struct_Value val)
{
	char *buf = malloc(256);
	switch (val.type) {
		case __enum_Type_Om:
			sprintf(buf, "om");
			break;
		case __enum_Type_Bool:
			if (val.data.b) sprintf(buf, "true");
			else            sprintf(buf, "false");
			break;
		case __enum_Type_Int:
			sprintf(buf, "%i", val.data.i);
			break;
		case __enum_Type_Float:
			sprintf(buf, "%f", val.data.f);
			break;
		case __enum_Type_Str:
			sprintf(buf, "%s", val.data.s);
			break;
		case __enum_Type_List: {
			int len = sprintf(buf, "[");
			__struct_Value *l = (__struct_Value *) val.data.l.data;
			for (u32 i = 0; i < val.data.l.len; i++) {
				if (i > 0) len += sprintf(&buf[len], ", ");
				char *s = __builtin_stlx_print_helper(l[i]);
				len += sprintf(&buf[len], "%s", s);
				free(s);
			}
			sprintf(&buf[len], "]");
		} break;
		case __enum_Type_Proc:
			sprintf(buf, "/* procedure */");
			break;
	}
	return buf;
}

char *__builtin_stlx_get_type_str(__struct_Value val)
{
	switch (val.type) {
		case __enum_Type_Om:
			return "Om";
		case __enum_Type_Bool:
			return "Bool";
		case __enum_Type_Int:
			return "Int";
		case __enum_Type_Float:
			return "Float";
		case __enum_Type_Str:
			return "String";
		case __enum_Type_List:
			return "List";
		case __enum_Type_Proc:
			return "Procedure";
	}
	return NULL;
}

char *__dbg_print_value(__struct_Value val)
{
	char *buf = malloc(256);
	char *type_str = __builtin_stlx_get_type_str(val);
	int len = sprintf(buf, "{ type: %s, val: ", type_str);
	char *s = __builtin_stlx_print_helper(val);
	sprintf(&buf[len], "%s }\n", s);
	free(s);
	return buf;
}

u32 __builtin_stlx_get_idx(__struct_Value idx, __struct_Value list)
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

__struct_Cmp_Res __builtin_stlx_cmp_values(__struct_Value x, __struct_Value y)
{
	static const __struct_Cmp_Res EQ     = { .can_cmp = true, .res = 0 };
	static const __struct_Cmp_Res NO_CMP = { .can_cmp = false, .res = -1 };
	bool eq_type = x.type == y.type;
	switch (x.type) {
		case __enum_Type_Om:
			return (__struct_Cmp_Res) {
				.can_cmp = false,
				.res     = eq_type ? 0 : -1,
			};
		case __enum_Type_Bool:
			return eq_type && (x.data.b == y.data.b) ? EQ : NO_CMP;
		case __enum_Type_Int:
			if (y.type == __enum_Type_Float) {
				return __builtin_stlx_cmp_values((__struct_Value) {.type = __enum_Type_Float, .data = {.f = (float)x.data.i}}, y);
			} else if (eq_type) {
				return (__struct_Cmp_Res) {.can_cmp = true, .res = x.data.i - y.data.i};
			} else {
				return NO_CMP;
			}
		case __enum_Type_Float:
			if (y.type == __enum_Type_Int) {
				return __builtin_stlx_cmp_values(x, (__struct_Value) {.type = __enum_Type_Float, .data = {.f = (float)y.data.i}});
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
				out.res = __builtin_stlx_cmp_values(((__struct_Value *)xl.data)[i], ((__struct_Value *)yl.data)[i]).res;
			}
			return out;
		case __enum_Type_Str:
			return eq_type && (strcmp(x.data.s, y.data.s) == 0) ? EQ : NO_CMP;
		case __enum_Type_Proc:
			AIL_TODO();
	}
	return NO_CMP;
}

__struct_Value __builtin_stlx_eq_values(__struct_Value x, __struct_Value y)
{
	__struct_Value out = { .type = __enum_Type_Bool, .data = {.b = false} };
	if (__builtin_stlx_cmp_values(x, y).res == 0) out.data.b = true;
	return out;
}

__struct_Value __builtin_stlx_le_values(__struct_Value x, __struct_Value y)
{
	__struct_Value out = { .type = __enum_Type_Bool, .data = {.b = false} };
	__struct_Cmp_Res cmp = __builtin_stlx_cmp_values(x, y);
	if (!cmp.can_cmp) AIL_UNREACHABLE();
	if (cmp.res <= 0) out.data.b = true;
	return out;
}

__struct_Value __builtin_stlx_lt_values(__struct_Value x, __struct_Value y)
{
	__struct_Value out = { .type = __enum_Type_Bool, .data = {.b = false} };
	__struct_Cmp_Res cmp = __builtin_stlx_cmp_values(x, y);
	if (!cmp.can_cmp) AIL_UNREACHABLE();
	if (cmp.res < 0) out.data.b = true;
	return out;
}

__struct_Value __builtin_stlx_ge_values(__struct_Value x, __struct_Value y)
{
	__struct_Value out = { .type = __enum_Type_Bool, .data = {.b = false} };
	__struct_Cmp_Res cmp = __builtin_stlx_cmp_values(x, y);
	if (!cmp.can_cmp) AIL_UNREACHABLE();
	if (cmp.res >= 0) out.data.b = true;
	return out;
}

__struct_Value __builtin_stlx_gt_values(__struct_Value x, __struct_Value y)
{
	__struct_Value out = { .type = __enum_Type_Bool, .data = {.b = false} };
	__struct_Cmp_Res cmp = __builtin_stlx_cmp_values(x, y);
	if (!cmp.can_cmp) AIL_UNREACHABLE();
	if (cmp.res > 0) out.data.b = true;
	return out;
}

__struct_Value __builtin_stlx_neg_value(__struct_Value val)
{
	switch (val.type) {
		case __enum_Type_Om:
		case __enum_Type_Bool:
		case __enum_Type_Proc:
		case __enum_Type_Str:
		case __enum_Type_List:
			fprintf(stderr, "Error in trying to negate %s\n", __dbg_print_value(val));
			exit(1);
			break;
		case __enum_Type_Int:
			return (__struct_Value) { .type = val.type, .data = { .i = -val.data.i }};
		case __enum_Type_Float:
			return (__struct_Value) { .type = val.type, .data = { .f = -val.data.f }};
	}
	AIL_UNREACHABLE();
}

__struct_Value __builtin_stlx_logical_not(__struct_Value val)
{
	if (val.type != __enum_Type_Bool) AIL_UNREACHABLE();
	val.data.b = !val.data.b;
	return val;
}

__struct_Value __builtin_stlx_add_values(__struct_Value l, __struct_Value r)
{
	switch (l.type) {
		case __enum_Type_Om:
		case __enum_Type_Bool:
		case __enum_Type_Proc:
			fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
			exit(1);
		case __enum_Type_Int:
			switch (r.type) {
				case __enum_Type_Om:
				case __enum_Type_Bool:
				case __enum_Type_Proc:
					fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
					exit(1);
				case __enum_Type_Int:
					return (__struct_Value){ .type = __enum_Type_Int, .data =  { .i = l.data.i + r.data.i }};
				case __enum_Type_Float:
					return (__struct_Value){ .type = __enum_Type_Float, .data = { .f = ((float)l.data.i) + r.data.f }};
				case __enum_Type_Str:
				case __enum_Type_List:
					AIL_TODO();
			} break;
		case __enum_Type_Float:
			switch (r.type) {
				case __enum_Type_Om:
				case __enum_Type_Bool:
				case __enum_Type_Proc:
					fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
					exit(1);
				case __enum_Type_Int:
					return (__struct_Value){ .type = __enum_Type_Float, .data =  { .f = l.data.f + ((float)r.data.i) }};
				case __enum_Type_Float:
					return (__struct_Value){ .type = __enum_Type_Float, .data = { .f = l.data.f + r.data.f }};
				case __enum_Type_Str:
				case __enum_Type_List:
					AIL_TODO();
			} break;
		case __enum_Type_Str:
			AIL_TODO();
			break;
		case __enum_Type_List:
			AIL_TODO();
			break;
	}
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_sub_values(__struct_Value l, __struct_Value r)
{
	switch (l.type) {
		case __enum_Type_Om:
		case __enum_Type_Bool:
		case __enum_Type_Proc:
			fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
			exit(1);
		case __enum_Type_Int:
			switch (r.type) {
				case __enum_Type_Om:
				case __enum_Type_Bool:
				case __enum_Type_Proc:
					fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
					exit(1);
				case __enum_Type_Int:
					return (__struct_Value){ .type = __enum_Type_Int, .data =  { .i = l.data.i - r.data.i }};
				case __enum_Type_Float:
					return (__struct_Value){ .type = __enum_Type_Float, .data = { .f = ((float)l.data.i) - r.data.f }};
				case __enum_Type_Str:
				case __enum_Type_List:
					AIL_TODO();
			} break;
		case __enum_Type_Float:
			switch (r.type) {
				case __enum_Type_Om:
				case __enum_Type_Bool:
				case __enum_Type_Proc:
					fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
					exit(1);
				case __enum_Type_Int:
					return (__struct_Value){ .type = __enum_Type_Float, .data =  { .f = l.data.f - ((float)r.data.i) }};
				case __enum_Type_Float:
					return (__struct_Value){ .type = __enum_Type_Float, .data = { .f = l.data.f - r.data.f }};
				case __enum_Type_Str:
				case __enum_Type_List:
					AIL_TODO();
			} break;
		case __enum_Type_Str:
			AIL_TODO();
			break;
		case __enum_Type_List:
			AIL_TODO();
			break;
	}
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_mul_values(__struct_Value l, __struct_Value r)
{
	switch (l.type) {
		case __enum_Type_Om:
		case __enum_Type_Bool:
		case __enum_Type_Proc:
			fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
			exit(1);
			break;
		case __enum_Type_Int:
			switch (r.type) {
				case __enum_Type_Om:
				case __enum_Type_Bool:
				case __enum_Type_Proc:
					fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
					exit(1);
				case __enum_Type_Int:
					return (__struct_Value){ .type = __enum_Type_Int, .data =  { .i = l.data.i * r.data.i }};
				case __enum_Type_Float:
					return (__struct_Value){ .type = __enum_Type_Float, .data = { .f = ((float)l.data.i) * r.data.f }};
				case __enum_Type_Str:
				case __enum_Type_List:
					AIL_TODO();
			} break;
		case __enum_Type_Float:
			switch (r.type) {
				case __enum_Type_Om:
				case __enum_Type_Bool:
				case __enum_Type_Proc:
					fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
					exit(1);
				case __enum_Type_Int:
					return (__struct_Value){ .type = __enum_Type_Float, .data =  { .f = l.data.f * ((float)r.data.i) }};
				case __enum_Type_Float:
					return (__struct_Value){ .type = __enum_Type_Float, .data = { .f = l.data.f * r.data.f }};
				case __enum_Type_Str:
				case __enum_Type_List:
					AIL_TODO();
			} break;
		case __enum_Type_Str:
			AIL_TODO();
			break;
		case __enum_Type_List:
			AIL_TODO();
			break;
	}
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_div_values(__struct_Value l, __struct_Value r)
{
	switch (l.type) {
		case __enum_Type_Om:
		case __enum_Type_Bool:
		case __enum_Type_Proc:
			fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
			exit(1);
			break;
		case __enum_Type_Int:
			switch (r.type) {
				case __enum_Type_Om:
				case __enum_Type_Bool:
				case __enum_Type_Proc:
					fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
					exit(1);
				case __enum_Type_Int:
					return (__struct_Value){ .type = __enum_Type_Float, .data =  { .f = ((float)l.data.i) / ((float)r.data.i) }};
				case __enum_Type_Float:
					return (__struct_Value){ .type = __enum_Type_Float, .data = { .f = ((float)l.data.i) / r.data.f }};
				case __enum_Type_Str:
				case __enum_Type_List:
					AIL_TODO();
			} break;
		case __enum_Type_Float:
			switch (r.type) {
				case __enum_Type_Om:
				case __enum_Type_Bool:
				case __enum_Type_Proc:
					fprintf(stderr, "Error in trying to add %s and %s\n", __dbg_print_value(l), __dbg_print_value(r));
					exit(1);
				case __enum_Type_Int:
					return (__struct_Value){ .type = __enum_Type_Float, .data =  { .f = l.data.f / ((float)r.data.i) }};
				case __enum_Type_Float:
					return (__struct_Value){ .type = __enum_Type_Float, .data = { .f = l.data.f / r.data.f }};
				case __enum_Type_Str:
				case __enum_Type_List:
					AIL_TODO();
			} break;
		case __enum_Type_Str:
			AIL_TODO();
			break;
		case __enum_Type_List:
			AIL_TODO();
			break;
	}
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_load(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_arb(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_collect(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_first(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_last(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_from(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_fromB(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_fromE(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_domain(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_max(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_min(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_pow(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_range(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_reverse(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_sort(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_char(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_endsWith(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_eval(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_matches(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_join(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_replace(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_replaceFirst(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_split(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_str(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_toLowerCase(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_toUpperCase(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_trim(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_args(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_evalTerm(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_fct(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_getTerm(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_makeTerm(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_canonical(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_parse(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_parseStatements(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_toTerm(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_parseTerm(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_fromTerm(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_abs(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_ceil(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_floor(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_mathConst(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_nextProbablePrime(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_int(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_rational(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_double(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_sin(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_cos(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_tan(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_asin(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_acos(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_atan(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_atan2(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_hypot(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_exp(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_expm1(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_log(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_log1p(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_log10(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_sqrt(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_cbrt(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_round(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_nDecimalPlaces(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_ulp(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_signum(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_sinh(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_cosh(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_tanh(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_isPrime(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_isProbablePrime(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_random(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_resetRandom(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_rnd(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_shuffle(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_nextPermutation(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_permutations(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_isBoolean(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_isDouble(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_isError(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_isInfinite(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_isInteger(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_isList(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_isMap(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_isNumber(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_isProcedure(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_isRational(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_isSet(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_isString(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_isTerm(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_isObject(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_trace(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_stop(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_assert(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_appendFile(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_ask(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_deleteFile(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_get(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_multiLineMode(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_nPrint(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_nPrintErr(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_print(u8 argc, __struct_Value *argv)
{
	if (argc != 1) AIL_UNREACHABLE();
	char *s = __builtin_stlx_print_helper(argv[0]);
	printf("%s\n", s);
	free(s);
	return (__struct_Value){0};
}

__struct_Value __builtin_stlx_printErr(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_read(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_readFile(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_writeFile(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_abort(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_cacheStats(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_clearCache(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_compare(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_getOsID(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_getScope(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_logo(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_now(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_run(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}

__struct_Value __builtin_stlx_sleep(u8 argc, __struct_Value *argv)
{
	(void)argc;
	(void)argv;
	AIL_TODO();
	return (__struct_Value) {0};
}


#endif // SETLX_H_