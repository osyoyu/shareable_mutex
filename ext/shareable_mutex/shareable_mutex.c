#include "shareable_mutex.h"

VALUE rb_mShareableMutex;

RUBY_FUNC_EXPORTED void
Init_shareable_mutex(void)
{
  rb_mShareableMutex = rb_define_module("ShareableMutex");
}
