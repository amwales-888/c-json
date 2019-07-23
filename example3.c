/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

#include "json.h"

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
int main(int argc __attribute__((unused)),
	 char **argv __attribute__((unused))) {

  struct jhandle jhandle;
  char *json = "{ \"name\" : \"bob\" }";
  
  if (json_alloc(&jhandle, (void *)0, 32) == 0) {
    if (json_decode(&jhandle, json, strlen(json)) == 0) {
      struct jobject *jobject;
     
      if ((jobject = json_query(&jhandle, JOBJECT_ROOT(&jhandle), "name"))) {
	json_dump(&jhandle, jobject, 1);
      }
    }
    json_free(&jhandle);
  }
  
  return 0;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
