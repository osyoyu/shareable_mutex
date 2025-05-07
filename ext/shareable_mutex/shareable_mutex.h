#ifndef SHAREABLE_MUTEX_H
#define SHAREABLE_MUTEX_H 1

#include <ruby.h>

VALUE rb_mutex2_locked_p(VALUE self);
VALUE rb_mutex2_trylock(VALUE self);
VALUE rb_mutex2_lock(VALUE self);
VALUE rb_mutex2_unlock(VALUE self);
VALUE rb_mutex2_synchronize_m(VALUE self);
VALUE rb_mutex2_owned_p(VALUE self);

void Init_shareable_mutex(void);

#endif /* SHAREABLE_MUTEX_H */
