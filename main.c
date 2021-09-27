int main() {
    register unsigned long r;
    asm volatile("mrs %0, control" : "=r"(r));
    /* int a = 0x111; */
    /* int b = 0x222; */
    /* int c = a + b; */
    return 0;
}
