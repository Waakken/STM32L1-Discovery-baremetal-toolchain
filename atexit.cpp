extern "C" {
// https://stackoverflow.com/questions/48711221/how-to-prevent-inclusion-of-c-library-destructors-and-atexit
int __wrap_atexit(void __attribute__((unused)) (*function)(void)) { return -1; }
}
