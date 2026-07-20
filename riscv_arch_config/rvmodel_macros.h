#ifndef RVMODEL_MACROS_H
#define RVMODEL_MACROS_H


/* 1. TERMINATION MACROS*/

/* 1.1 Terminates test with a pass indication. */
#define RVMODEL_HALT_PASS \
    li a0, 0             ;\
    ecall                ; /* halts the program counter*/

/* 1.2 Terminates test with a fail indication. */
#define RVMODEL_HALT_FAIL \
    li a0, 1             ;\
    ecall                ; /* halts the program counter*/


/* 2. PLACEHOLDER MACROS
 *
 * Note: Their defintion is required, but can be left empty or 
 * be given placeholder values.
 */

/* 2.1 Data section */
#define RVMODEL_DATA_SECTION

/* 2.2 Boot code */
#define RVMODEL_BOOT

/* 2.3 Console / printing */
#define RVMODEL_IO_INIT(_R1, _R2, _R3)
#define RVMODEL_IO_WRITE_STR(_R1, _R2, _R3, _STR_PTR)

/* 2.4 Interrupt latency */
#define RVMODEL_INTERRUPT_LATENCY 10

/* 2.5  Machine-mode software / external interrupt helpers */
#define RVMODEL_SET_MSW_INT(_R1, _R2)
#define RVMODEL_CLR_MSW_INT(_R1, _R2)
#define RVMODEL_SET_MEXT_INT(_R1, _R2)
#define RVMODEL_CLR_MEXT_INT(_R1, _R2)

/* 2.6  Supervisor-mode interrupt helpers */
#define RVMODEL_SET_SSW_INT(_R1, _R2)
#define RVMODEL_CLR_SSW_INT(_R1, _R2)
#define RVMODEL_SET_SEXT_INT(_R1, _R2)
#define RVMODEL_CLR_SEXT_INT(_R1, _R2)

/* 2.7 Timer interrupt */
#define RVMODEL_TIMER_INT_SOON_DELAY

#endif /* RVMODEL_MACROS */