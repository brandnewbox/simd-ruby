#include "simd_floatarray.h"

VALUE SIMD_FloatArray = Qnil;

/* Internal: Create the SIMD::FloatArray class. */
void Init_SIMD_FloatArray(VALUE parent)
{
	SIMD_FloatArray = rb_define_class_under(parent, "FloatArray", rb_cObject);
	rb_define_alloc_func(SIMD_FloatArray, allocate);
	rb_define_method(SIMD_FloatArray, "initialize", method_initialize, 1);
	rb_define_method(SIMD_FloatArray, "*", method_multiply, 1);
	rb_define_method(SIMD_FloatArray, "/", method_divide, 1);
	rb_define_method(SIMD_FloatArray, "+", method_add, 1);
	rb_define_method(SIMD_FloatArray, "-", method_subtract, 1);
	rb_define_method(SIMD_FloatArray, "length", method_length, 0);
	rb_define_method(SIMD_FloatArray, "to_a", method_to_a, 0);
}

/* Public: Initialize the FloatArray object given a Ruby Array of values
 * which can be cast to a double. */
static VALUE method_initialize(VALUE self, VALUE rb_array)
{
	vector_t *vector;
	d2v_t *data;
	unsigned long n,m,i;

	Check_Type(rb_array, T_ARRAY);
	Data_Get_Struct(self, vector_t, vector);

	vector->len = n = RARRAY_LEN(rb_array);

	if(vector->len < 2)
	{
		rb_raise(rb_eArgError, "Vectors must be at least 4 long");
	}

	vector->data = internal_allocate_vector_array(vector->len, sizeof(d2v_t));

	data = (d2v_t *)vector->data;
	for(i = 0; i < vector->len; i++)
	{
		data[i/2].f[i%2] = NUM2DBL(rb_ary_entry(rb_array, i));
	}

	/* If the array is an odd number of elements, set the final element to 1 */
	m = n + (n % 2);
	for(i = n % 4; i > 0; i--)
	{
		data[m/2].f[1] = 1.0;
	}

	return(self);
}

/* Public: Multiply values contained in the data array with those contained in
 * another FloatArray object, returning a new FloatArray. */
static VALUE method_multiply(VALUE self, VALUE obj)
{
	return(internal_apply_operation(self, obj, func_multiply));
}

/* Public: Divide values contained in the data array by those contained in
 * another FloatArray object, returning a new FloatArray. */
static VALUE method_divide(VALUE self, VALUE obj)
{
	return(internal_apply_operation(self, obj, func_divide));
}

/* Public: add values contained in the data array with those contained in
 * another FloatArray object, returning a new FloatArray. */
static VALUE method_add(VALUE self, VALUE obj)
{
	return(internal_apply_operation(self, obj, func_add));
}

/* Public: Subtract values contained in another FloatArray object from those
 * contained in the current data array object, returning a new FloatArray. */
static VALUE method_subtract(VALUE self, VALUE obj)
{
	return(internal_apply_operation(self, obj, func_subtract));
}

/* Public: Return a Ruby Array containing the doubles within the data array. */
static VALUE method_to_a(VALUE self)
{
	unsigned long i;
	vector_t *vector;
	d2v_t *data;
	VALUE rb_array = rb_ary_new();

	Data_Get_Struct(self, vector_t, vector);
	data = (d2v_t *)vector->data;
	for(i = 0; i < vector->len; i++)
	{
		rb_ary_store(rb_array, i, DBL2NUM(data[i/2].f[i%2]));
	}

	return(rb_array);
}

/* Internal: Given another FloatArray object, perform an action specified via a
 * function pointer against both. */
static VALUE internal_apply_operation(VALUE self, VALUE obj, b_operation func)
{
	unsigned long size, i;
	int align;
	vector_t *v1, *v2, *rv;
	d2v_t *d1, *d2, *r;
	VALUE result_obj = allocate(SIMD_FloatArray);

	Data_Get_Struct(self, vector_t, v1);
	Data_Get_Struct(obj, vector_t, v2);
	Data_Get_Struct(result_obj, vector_t, rv);
	rv->data = internal_allocate_vector_array(v1->len, sizeof(d2v_t));

	align = internal_align_vectors(v1->len, v2->len, 2);

	/* Ensure that size will be the result of ceil(len / 4.0) */
	size = (v1->len + 1) / 2;

	d1 = (d2v_t *)v1->data;
	d2 = (d2v_t *)v2->data;
	r  = (d2v_t *)rv->data;

	rv->len = v1->len;

	switch(align)
	{
		case 0: /* Same size arrays */
			for(i = 0; i < size; i++)
			{
				func(&d1[i].v, &d2[i].v, &r[i].v);
			}
			break;
		case 1: /* Operand is exactly 2 long (size of 1 sse register) */
			for(i = 0; i < size; i++)
			{
				func(&d1[i].v, &d2[0].v, &r[i].v);
			}
			break;
		default: /* Self is a multiple of operand's length long */
			for(i = 0; i < size; i++)
			{
				func(&d1[i].v, &d2[i % v2->len].v, &r[i].v);
			}
	}

	if(rv->len != rv->len + (rv->len % 2))
	{
		r[size].f[1] = 1;
	}

	return(result_obj);
}

/* Function: Multiply two vectors. */
static void func_multiply(void *v1, void *v2, void *r)
{
	*(d2v *)r = *(d2v *)v1 * *(d2v *)v2;
}

/* Function: Divide two vectors. */
static void func_divide(void *v1, void *v2, void *r)
{
	*(d2v *)r = *(d2v *)v1 / *(d2v *)v2;
}

/* Function: Add two vectors. */
static void func_add(void *v1, void *v2, void *r)
{
	*(d2v *)r = *(d2v *)v1 + *(d2v *)v2;
}

/* Function: Subtract two vectors. */
static void func_subtract(void *v1, void *v2, void *r)
{
	*(d2v *)r = *(d2v *)v1 - *(d2v *)v2;
}
