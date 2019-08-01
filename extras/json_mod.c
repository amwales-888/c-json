/* -------------------------------------------------------------------- *

Copyright 2012 Angelo Masci

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the 
"Software"), to deal in the Software without restriction, including 
without limitation the rights to use, copy, modify, merge, publish, 
distribute, sublicense, and/or sell copies of the Software, and to permit 
persons to whom the Software is furnished to do so, subject to the 
following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR 
THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 * -------------------------------------------------------------------- */

#include <string.h>
#include <stdarg.h>

#include "json.h"
#include "json_mod.h"

/* -------------------------------------------------------------------- */

extern struct jobject *jobject_allocate(struct jhandle *jhandle, unsigned int count);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

static unsigned int json_strdup(struct jhandle *jhandle, char *ptr, int len);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static unsigned int json_strdup(struct jhandle *jhandle,
				char *ptr, int len) {

  char *dptr = (char *)jobject_allocate(jhandle,
					(len + (sizeof(struct jobject)-1)) / sizeof(struct jobject));

  if (dptr == (void *)0) return -1;
  
  memcpy(dptr, ptr, len);
  return JOBJECT_OFFSET(jhandle, dptr);
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
struct jobject *json_string_new(struct jhandle *jhandle,
			    char *ptr, int len) {

  if (!jhandle->hasdecoded) {  

    unsigned int offset = json_strdup(jhandle, ptr, len);
    if (offset != (unsigned int)-1) {
  
      struct jobject *jobject = jobject_allocate(jhandle, 1);
      if (jobject == (void *)0) return (void *)0;

      jobject->bnext           = JSON_STRING << JSON_NEXTBITS;
      jobject->u.string.offset = offset;
      jobject->u.string.len    = len;
    
      return jobject;
    }
  }
  
  return (void *)0;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
struct jobject *json_object_add(struct jhandle *jhandle,
				struct jobject *object,
				struct jobject *string,
				struct jobject *value) {

  if ((!jhandle->hasdecoded) &&
      ((object->bnext >> JSON_NEXTBITS) == JSON_OBJECT)) {

    string->bnext = ((string->bnext & JSON_TYPEMASK) |
		     (JOBJECT_OFFSET(jhandle, value)));

    if (object->u.object.count == 0) {
      
      object->u.object.child = JOBJECT_OFFSET(jhandle, string);

    } else {

      struct jobject *jobject;
      unsigned int next = object->u.object.child;
      
      for (;;) {

	jobject = JOBJECT_AT(jhandle, next);
	if ((jobject->bnext & JSON_NEXTMASK) == JSON_INVALID) break;
	
	next = jobject->bnext & JSON_NEXTMASK;	
      }

      jobject->bnext = ((jobject->bnext & JSON_TYPEMASK) |
			(JOBJECT_OFFSET(jhandle, string)));
    }

    object->u.object.count += 2;
    return object;
  }

  return (void *)0;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
struct jobject *json_object_new(struct jhandle *jhandle, ...) {

  if (!jhandle->hasdecoded) {  

    struct jobject *object;
    va_list ap;

    unsigned int last  = JSON_INVALID;
    unsigned int first = JSON_INVALID;

    int count = 0;
  
    va_start(ap, jhandle);

    for(;;) {
    
      struct jobject *string;
      struct jobject *value;
      struct jobject *jobject;

      string = va_arg(ap, struct jobject *);
      if (string == (void *)0) {
	break;
      }

      count++;
      if (last == JSON_INVALID) {
	first = JOBJECT_OFFSET(jhandle, string);
	last  = first;
      } else {
	jobject = JOBJECT_AT(jhandle, last);

	jobject->bnext = ((jobject->bnext & JSON_TYPEMASK) |
			  (JOBJECT_OFFSET(jhandle, string)));

	last = jobject->bnext & JSON_NEXTMASK;
      }

      value = va_arg(ap, struct jobject *);

      count++;
      jobject = JOBJECT_AT(jhandle, last);

      jobject->bnext = ((jobject->bnext & JSON_TYPEMASK) |
			(JOBJECT_OFFSET(jhandle, value)));

      last = jobject->bnext & JSON_NEXTMASK;
    }

    va_end(ap);

    object = jobject_allocate(jhandle, 1);
    if (object == (void *)0) return (void *)0;

    object->bnext          = JSON_OBJECT << JSON_NEXTBITS;
    object->u.object.child = first;
    object->u.object.count = count;

    return object;
  }

  return (void *)0;  
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
struct jobject *json_array_new(struct jhandle *jhandle, ...) {

  if (!jhandle->hasdecoded) {  

    struct jobject *array;
    va_list ap;

    unsigned int last  = JSON_INVALID;
    unsigned int first = JSON_INVALID;

    int count = 0;
  
    va_start(ap, jhandle);

    for(;;) {
    
      struct jobject *value = va_arg(ap, struct jobject *);
      if (value == (void *)0) {
	break;
      }

      count++;
      if (last == JSON_INVALID) {
	first = JOBJECT_OFFSET(jhandle, value);
	last  = first;
      } else {
	struct jobject *jobject = JOBJECT_AT(jhandle, last);

	jobject->bnext = ((jobject->bnext & JSON_TYPEMASK) |
			  (JOBJECT_OFFSET(jhandle, value)));

	last = jobject->bnext & JSON_NEXTMASK;
      }
    }
  
    va_end(ap);

    array = jobject_allocate(jhandle, 1);
    if (array == (void *)0) return (void *)0;

    array->bnext          = JSON_ARRAY << JSON_NEXTBITS;
    array->u.object.child = first;
    array->u.object.count = count;

    return array;
  }
  
  return (void *)0;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
struct jobject *json_array_add(struct jhandle *jhandle,
			       struct jobject *object,
			       struct jobject *value) {

  if ((!jhandle->hasdecoded) &&
      ((object->bnext >> JSON_NEXTBITS) == JSON_ARRAY)) {

    if (object->u.object.count == 0) {
      
      object->u.object.child = JOBJECT_OFFSET(jhandle, value);

    } else {

      struct jobject *jobject;
      unsigned int next = object->u.object.child;
      
      for (;;) {

	jobject = JOBJECT_AT(jhandle, next);
	if ((jobject->bnext & JSON_NEXTMASK) == JSON_INVALID) break;
	
	next = jobject->bnext & JSON_NEXTMASK;	
      }
      
      jobject->bnext = ((jobject->bnext & JSON_TYPEMASK) |
			(JOBJECT_OFFSET(jhandle, value)));
    }

    object->u.object.count++;
    return object;
  }

  return (void *)0;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
