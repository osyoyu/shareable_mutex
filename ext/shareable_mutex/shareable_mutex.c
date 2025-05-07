#include "shareable_mutex.h"

#include <ruby.h>
#include <pthread.h>

VALUE rb_cMutex2;

struct safemutex {
    pthread_mutex_t *pthread_mutex;
    pthread_t owner_pthread;
};

static void dfree(void *data);
static size_t dsize(const void *data);

static const rb_data_type_t safemutex_type = {
    .wrap_struct_name = "mutex2",
    .function = {
        .dmark = NULL,
        .dfree = dfree,
        .dsize = dsize,
    },
    .data = NULL,
    .flags = RUBY_TYPED_FREE_IMMEDIATELY | RUBY_TYPED_FROZEN_SHAREABLE
};

VALUE
mutex_alloc(VALUE klass)
{
    pthread_mutex_t *pthread_mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(pthread_mutex, NULL);

    struct safemutex *mutex = malloc(sizeof(struct safemutex));
    mutex->pthread_mutex = pthread_mutex;
    mutex->owner_pthread = 0;

    return TypedData_Wrap_Struct(klass, &safemutex_type, mutex);
}

VALUE
rb_mutex2_locked_p(VALUE self)
{
    struct safemutex *mutex;
    TypedData_Get_Struct(self, struct safemutex, &safemutex_type, mutex);

    return (mutex->owner_pthread != 0) ? Qtrue : Qfalse;
}

VALUE
rb_mutex2_trylock(VALUE self)
{
    struct safemutex *mutex;
    TypedData_Get_Struct(self, struct safemutex, &safemutex_type, mutex);

    if (pthread_mutex_trylock(mutex->pthread_mutex) == 0) {
        mutex->owner_pthread = pthread_self();
        return Qtrue;
    }
    else {
        return Qfalse;
    }
}

VALUE
rb_mutex2_lock(VALUE self)
{
    rb_p(self);
    struct safemutex *mutex;
    TypedData_Get_Struct(self, struct safemutex, &safemutex_type, mutex);

    if (mutex->owner_pthread == pthread_self()) {
        // The current thread already owns the mutex.
        rb_raise(rb_eThreadError, "deadlock; recursive locking");
        return Qfalse;
    }

    int err = pthread_mutex_lock(mutex->pthread_mutex);
    if (err == 0) {
        mutex->owner_pthread = pthread_self();
        return Qtrue;
    }
    else {
        rb_bug("unreachable"); // FIXME?
    }
}

VALUE
rb_mutex2_unlock(VALUE self)
{
    struct safemutex *mutex;
    TypedData_Get_Struct(self, struct safemutex, &safemutex_type, mutex);

    int err = pthread_mutex_unlock(mutex->pthread_mutex);
    if (err == 0) {
        mutex->owner_pthread = 0;
        return Qtrue;
    }
    else {
        rb_bug("unreachable");
    }
}

VALUE
rb_mutex2_synchronize_m(VALUE self)
{
    if (!rb_block_given_p()) {
        rb_raise(rb_eThreadError, "must be called with a block");
    }

    rb_raise(rb_eStandardError, "not implemented");
    return Qnil;

    // TODO: implement
    // rb_mutex2_lock(self);
    // return rb_ensure(rb_yield, Qnil, rb_mutex2_unlock, self);
}

VALUE
rb_mutex2_owned_p(VALUE self)
{
    struct safemutex *mutex;
    TypedData_Get_Struct(self, struct safemutex, &safemutex_type, mutex);

    return (mutex->owner_pthread == pthread_self()) ? Qtrue : Qfalse;
}

static void
dfree(void *data)
{
    struct safemutex *mutex = data;
    if (pthread_mutex_destroy(mutex->pthread_mutex) != 0) {
        if (errno == EBUSY) {
            rb_bug("ShareableMutex was EBUSY");
        }
    }
    free(mutex->pthread_mutex);
    free(mutex);

}

static size_t
dsize(const void *data)
{
    return sizeof(struct safemutex) + sizeof(pthread_mutex_t);
}

RUBY_FUNC_EXPORTED void
Init_shareable_mutex(void)
{
#ifdef HAVE_RB_EXT_RACTOR_SAFE
    rb_ext_ractor_safe(true);
#endif

    rb_cMutex2 = rb_define_class("ShareableMutex", rb_cObject);
    rb_define_alloc_func(rb_cMutex2, mutex_alloc);
    rb_define_method(rb_cMutex2, "locked?", rb_mutex2_locked_p, 0);
    rb_define_method(rb_cMutex2, "try_lock", rb_mutex2_trylock, 0);
    rb_define_method(rb_cMutex2, "lock", rb_mutex2_lock, 0);
    rb_define_method(rb_cMutex2, "unlock", rb_mutex2_unlock, 0);
    rb_define_method(rb_cMutex2, "synchronize", rb_mutex2_synchronize_m, 0);
    rb_define_method(rb_cMutex2, "owned?", rb_mutex2_owned_p, 0);
}
