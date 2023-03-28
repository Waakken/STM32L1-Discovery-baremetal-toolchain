#pragma once
extern "C" {
int __wrap_atexit(void __attribute__((unused)) (*function)(void));
}
