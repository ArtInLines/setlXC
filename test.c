// #include <stdio.h>

typedef enum {
	TYPE_INT,
	TYPE_STR,
} Type;

typedef struct {
	Type  type;
	void* data;
} Value;

Value test(Value x) {
	return x;
}

int main(void)
{
	void *ptr = 0;
	Value x = { .type = 0, .data = ptr };
	Value (*f)(Value) = 0;
	f(x);
	// char s[] = "test";
	// puts(s);
	return 0;
}
