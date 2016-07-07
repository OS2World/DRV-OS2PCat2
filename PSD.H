#ifndef __PSD_H__
#define __PSD_H__

#define INCL_BASE
#include <os2.h>

#ifndef PSDENTRY
    #if __IBMC__ || __IBMCPP__
        #define PSDENTRY __stdcall
    #elif __BORLANDC__
        #define PSDENTRY __stdcall
    #else
        #error define PSDENTRY to be the keyword for the stdcall calling convention for your compiler !
    #endif
#endif


 typedef ULONG (PSDENTRY *P_F_1)(ULONG arg);
 typedef ULONG (PSDENTRY *P_F_2)(ULONG arg1, ULONG arg2);

 #define PSDHelp(router, function, arg) \
    ((*router)((function), (ULONG)(arg)))




 #define WARM_REBOOT_VECTOR_SEG  0x40
 #define WARM_REBOOT_VECTOR_OFF  0x67


 /* PSD Info structure */

 typedef struct info_s {                     /* psd */
    ULONG  flags;                          /* PSD flags */
    ULONG  version;                        /* PSD version */
    ULONG  hmte;                           /* MTE handle of PSD */
    UCHAR *pParmString;                    /* Pointer to ASCIIZ PSD parameter*/
    ULONG  IRQ_IPI;                        /* IRQ for IPI */
    ULONG  IRQ_LSI;                        /* IRQ for LSI */
    ULONG  IRQ_SPI;                        /* IRQ for SPI */
 } PSDINFO;


 /* PSD flags definition */
 #define PSD_KERNEL_PIC          0x10000000  /* Let the kernel interrupt manager EOI */
 #define PSD_ADV_INT_MODE        0x20000000  /* PSD is in adv int mode #81531 */
 #define PSD_INSTALLED           0x40000000  /* PSD has been installed */
 #define PSD_INITIALIZED         0x80000000  /* PSD has been initialized */

 /* PSD function numbers-structures */

 #define PSD_INSTALL             0x00000000  /* Install PSD */

 typedef struct install_s {                  /* install */
    P_F_2   pPSDHlpRouter;                   /* Address of PSDHlpRouter */
    CHAR   *pParmString;                     /* Pointer to parameter string */
    VOID   *pPSDPLMA;                        /* Pointer to PSD's PLMA */
    ULONG sizePLMA;                        /* Size of PLMA in bytes */
 } INSTALL;

 #define PSD_DEINSTALL           0x00000001  /* DeInstall PSD */

 #define PSD_INIT                0x00000002  /* Initialize PSD */

 typedef struct init_s {                     /* init */
    ULONG flags;                           /* Init flags */
    ULONG version;                         /* PSD Version number */
 } INIT;

 #define INIT_GLOBAL_IRQ_ACCESS  0x00000001  /* Platform has global IRQ access */
 #define INIT_USE_FPERR_TRAP     0x00000002  /* Use Trap 16 to report FP err's */
 #define INIT_EOI_IRQ13_ON_CPU0  0x00000004  /* eoi IRQ 13 only if on cpu 0    */
 #define INIT_TIMER_CPU0         0x00000008  /* system timer is on CPU 0       */

 #define PSD_PROC_INIT           0x00000003  /* Initialize processor */

 #define PSD_START_PROC          0x00000004  /* Start processor */

 #define PSD_GET_NUM_OF_PROCS    0x00000005  /* Get number of processors */

 #define PSD_GEN_IPI             0x00000006  /* Generate an IPI */

 #define PSD_END_IPI             0x00000007  /* End an IPI */

 #define PSD_PORT_IO             0x00000008  /* Port I/O */

 typedef struct port_io_s {                  /* port_io */
    ULONG port;                            /* Port number to access */
    ULONG data;                            /* Data read, or data to write */
    ULONG flags;                           /* IO Flags */
 } PORT_IO;

 #define IO_READ_BYTE    0x0000              /* Read a byte from the port */
 #define IO_READ_WORD    0x0001              /* Read a word from the port */
 #define IO_READ_DWORD   0x0002              /* Read a dword from the port */
 #define IO_WRITE_BYTE   0x0003              /* Write a byte to the port */
 #define IO_WRITE_WORD   0x0004              /* Write a word to the port */
 #define IO_WRITE_DWORD  0x0005              /* Write a dword to the port */

 #define IO_FLAGMASK     0x0007              /* Flag mask */

 #define PSD_IRQ_MASK            0x00000009  /* Mask/Unmask IRQ levels */

 typedef struct psd_irq_s {                  /* psd_irq */
    ULONG flags;                           /* IRQ flags */
    ULONG data;                            /* IRQ data */
                                             /*   depending on type of irq */
                                             /*   operation, the data field */
                                             /*   can contain any of the */
                                             /*   following info: */
                                             /*   1) Mask or UNMasking data */
                                             /*   2) IRR or ISR reg values */
                                             /*   3) IRQ # for EOI operations */
    ULONG procnum;                         /* Processor number */
 } PSD_IRQ;

 #define PSD_IRQ_REG             0x0000000A  /* Access IRQ related regs */

 #define PSD_IRQ_EOI             0x0000000B  /* Issue an EOI */

 #define IRQ_MASK                0x00000001  /* Turn on IRQ mask bits */
 #define IRQ_UNMASK              0x00000002  /* Turn off IRQ mask bits */
 #define IRQ_GETMASK             0x00000004  /* Get IRQ mask bits */
 #define IRQ_NEWMASK             0x00000010  /* Set and/or Reset all masks */
 #define IRQ_READ_IRR            0x00000100  /* Read the IRR reg */
 #define IRQ_READ_ISR            0x00000200  /* Read the ISR reg */

 #define PSD_APP_COMM            0x0000000C  /* PSD/APP Communication */

 #define PSD_SET_ADV_INT_MODE    0x0000000D  /* Set advanced int mode */

 #define PSD_SET_PROC_STATE      0x0000000E  /* Set proc state; idle, or busy */

 #define PROC_STATE_IDLE         0x00000000  /* Processor is idle */
 #define PROC_STATE_BUSY         0x00000001  /* Processor is busy */

 #define PSD_QUERY_SYSTEM_TIMER  0x0000000F  /* Query Value of System Timer 0 */

 typedef struct psd_qrytmr_s {             /* psd_qrytmr */
    ULONG qw_ulLo_psd;                     /* Timer count */
    ULONG qw_ulHi_psd;                     /* Timer count */
    ULONG pqwTmr;                          /* 16:16 ptr to qwTmr */
                                           /* PSD needs to read the 64-bit timer value at the given 16:16 memory location */
                                           /* and scale it to the PIT 0 timer base frequency (if PIT 0 is to be used as timebase) */
                                           /* scaled value to be returned in qw_ulLo_psd and qw_ulHi_psd */
 } PSD_QRYTMR;

 #define PSD_SET_SYSTEM_TIMER    0x00000010  /* Set System Timer 0 counter    */

 typedef struct psd_settmr_s {             /* psd_settmr */
    ULONG NewRollOver;                     /* NewRollover*/
    ULONG pqwTmrRollover;                  /* 16:16 ptr to qwTmrRollover */
                                           /* PSD needs to update the 32-bit rollover value saved at the given 16:16 memory location */
 } PSD_SETTMR;

#define PSD_RESET_MODE           0x00000011  /* on shutdown, called after all driver SHUTDOWN calls completed */

#define PSD_MTRACE_START         0x00000012  /* unknown, arguments also unknown */



 /* PSD helper function numbers-structures */

 #define PSDHLP_VMALLOC          0x00000000  /* Allocate memory */

 typedef struct vmalloc_s {                  /* vmalloc */
    ULONG addr;                            /* Physical address to map */
                                             /*  if VMALLOC_PHYS */
                                             /* Lin addr to alloc at */
                                             /*  if VMALLOC_LOCSPECIFIC */
                                             /* on return, addr of allocation */
    ULONG cbsize;                          /* Size of mapping in bytes */
    ULONG flags;                           /* Allocation flags */
 } VMALLOC;

 #define VMALLOC_FIXED           0x00000001  /* Allocate resident memory */
 #define VMALLOC_CONTIG          0x00000002  /* Allocate contiguous memory */
 #define VMALLOC_LOCSPECIFIC     0x00000004  /* Alloc at a specific lin address */
 #define VMALLOC_PHYS            0x00000008  /* Map physical address */
 #define VMALLOC_1M              0x00000010  /* Allocate below 1M */

 #define VMALLOC_FLAGMASK        0x0000001f  /* Valid flag mask */

 #define PSDHLP_VMFREE           0x00000001  /* Free memory */

 #define PSDHLP_SET_IRQ          0x00000002  /* Set up an IRQ */

 typedef struct set_irq_s {                  /* set_irq */
    USHORT irq;                            /* IRQ level */
    USHORT flags;                          /* Set IRQ flags */
    ULONG  vector;                         /* IRQ interrupt vector */
    P_F_2    handler;                        /* IRQ handler */
 } SET_IRQ;

 #define IRQf_IPI  0x0020                    /* IRQ for IPI */
 #define IRQf_LSI  0x0040                    /* IRQ for LSI */
 #define IRQf_SPI  0x0080                    /* IRQ for SPI */
 #define IRQf_SHARED 0x0004                  /* this IRQ can be shared */

 #define PSDHLP_CALL_REAL_MODE   0x00000003  /* Call a function in real mode */

 typedef struct call_real_mode_s {           /* call_real_mode */
    ULONG function;                        /* Function address */
    ULONG pdata;                           /* Pointer to data area */
 } CALL_REAL_MODE;

 #define PSDHLP_VMLINTOPHYS      0x00000004  /* Convert linear addr to phys */

 #define PSDHLP_ADJ_PG_RANGES    0x00000005  /* Adjust page ranges */

 typedef struct _pagerange_s {               /* pagerange */
    ULONG lastframe;                       /* Last valid page in range */
    ULONG firstframe;                      /* First valid page in range */
 };

 typedef struct adj_pg_ranges_s{             /* adj_pg_ranges */
    struct _pagerange_s *pprt;               /* Pointer to page range table */
    ULONG nranges;                         /* Num of ranges in range table */
 } ADJ_PG_RANGES;

 extern VOID PSDENTRY PSDEnter (ULONG function, ULONG arg, P_F_2 altEntry);

 /* PSD function prototypes */
APIRET PSDENTRY psd_install(INSTALL *p);   /* required */
APIRET PSDENTRY psd_deinstall(VOID);       /* required */
APIRET PSDENTRY psd_init(INIT *p);         /* required */
APIRET PSDENTRY psd_proc_init(VOID);       /* only required for MP */
APIRET PSDENTRY psd_start_proc(ULONG ulProcIdx); /* only required for MP */
ULONG  PSDENTRY psd_get_num_of_procs(VOID);
APIRET PSDENTRY psd_gen_ipi(ULONG ulProcIdx); /* only required for MP */
APIRET PSDENTRY psd_end_ipi(ULONG ulProcIdx); /* only required for MP */
APIRET PSDENTRY psd_port_io(PORT_IO *p);              /* optional */
APIRET PSDENTRY psd_irq_mask(PSD_IRQ *p);             /* optional */
APIRET PSDENTRY psd_irq_reg(PSD_IRQ *p);              /* optional */
APIRET PSDENTRY psd_irq_eoi(PSD_IRQ *p);              /* optional */
APIRET PSDENTRY psd_app_comm(ULONG funcNum,PVOID arg);/* optional */
APIRET PSDENTRY psd_set_adv_int_mode(VOID);           /* optional */
APIRET PSDENTRY psd_set_proc_state(ULONG procstate);  /* optional */
APIRET PSDENTRY psd_query_system_timer(PSD_QRYTMR *p);/* optional */
APIRET PSDENTRY psd_set_system_timer(PSD_SETTMR *p);  /* optional */
APIRET PSDENTRY psd_reset_mode(VOID);                 /* optional */
#if 0
APIRET PSDENTRY psd_mtrace_start(... unknown args ...);      /* optional */
#endif

#endif
