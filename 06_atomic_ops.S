#if(APPLE)
  #define FUNCTION_NAME(name) _##name
#else
  #define FUNCTION_NAME(name) name
#endif

.globl FUNCTION_NAME(AtomicLoad)
.globl FUNCTION_NAME(AtomicStore)
.globl FUNCTION_NAME(AtomicExchange)

# Solution starts here

/**
 movq is atomic in x86_64 if it uses 8-bit words and they have proper allignment
 (they are multiple of 8 => it is guaranteed by caller), because it uses only
 one cache move operation due to no crossing of cache-line boundary
 because, if we start from 0, 8, 16, ..., and put 8 in cache box, we will take only one box
 that means, it is atomic
 retq deals only with local (not shared memory) (%rax) => AtomicLoad and AtomicStore are atomic
**/

FUNCTION_NAME(AtomicLoad):
    movq (%rdi), %rax
    retq

FUNCTION_NAME(AtomicStore):
    movq %rsi, (%rdi)
    retq

/**
 xchg is also atomic, because it is implicitly locked. All other operations deal with local registers
 That means AtomicExchange is atomic
 P.S. I do movq to restore %rsi value of register, however it is not obligatory in x86_64 calling conventions
 %rsi is caller-saved (not preserved)
 %rdi - is the first parameter
 %rsi - is the second parameter
 xchg (%rdi), %rsi stores the value of the 
 first parameter (cell) value in the second parameter %rsi
 second parameter (value) then is loaded to the first parameter (%rdi)
 thus making *cell equal to "value"
**/

FUNCTION_NAME(AtomicExchange):
    xchg (%rdi), %rsi
    movq %rsi, %rax
    retq
