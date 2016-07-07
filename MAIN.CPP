#pragma comment(copyright,"@#Erdmann:2.0.0#@Test PSD Driver for generic PC AT")
#define INCL_BASE
#include <os2.h>
#include <psd.h>
#include <conio.h>
#include <string.h>

ULONG APIENTRY IRQDisable(VOID);
VOID APIENTRY IRQEnable(ULONG flags);
VOID APIENTRY IoDelay(VOID);
VOID APIENTRY ReplaceMFHandler(VOID);

#define ERROR_GENERIC   -1L
#define CHAIN_INTERRUPT  ERROR_GENERIC
#define CONTINUE_DEFAULT ERROR_GENERIC

/* for the IBM VAC 3.x compiler, we need to specify these variables this way
   so that they will show up in the map file and consequently in the sym file
   I don't know why these symbols are not public
*/
#pragma export(gInfo)
#pragma export(IRQ7Counter)
#pragma export(SpuriousIRQ7Counter)
#pragma export(IRQ15Counter )
#pragma export(SpuriousIRQ15Counter )
#pragma export(QueryTimerCounter)
#pragma export(SetTimerCounter)
#pragma export(TimerCounterVirtAddr)
#pragma export(TimerRollOverVirtAddr)

INSTALL gInfo={0};
ULONG IRQ7Counter = 0UL;
ULONG SpuriousIRQ7Counter = 0UL;
ULONG IRQ15Counter = 0UL;
ULONG SpuriousIRQ15Counter = 0UL;
ULONG QueryTimerCounter=0UL;
ULONG SetTimerCounter=0UL;
ULONG TimerCounterVirtAddr=0UL;
ULONG TimerRollOverVirtAddr=0UL;


ULONG PSDENTRY IRQ7Handler(VOID)
{
    ULONG flags,val,rc;

    IRQ7Counter++;

    flags = IRQDisable();
    _outp(0x20,0x0B);
    IoDelay();
    val = _inp(0x20);

    if (val & 0x80)
    {
        /* indeed an interrupt in service, pass on to registered ISRs */
        rc = CHAIN_INTERRUPT;
    }
    else
    {
        /* spurious, claim that this PSD handled it */
        SpuriousIRQ7Counter++;

        rc = NO_ERROR;
    }
    IRQEnable(flags);

    return rc;
}

ULONG PSDENTRY IRQ15Handler(VOID)
{
    ULONG flags,val,rc;

    IRQ15Counter++;

    flags = IRQDisable();
    _outp(0xA0,0x0B);
    IoDelay();
    val = _inp(0xA0);

    if (val & 0x80)
    {
        /* indeed an interrupt in service, pass on to registered ISRs */
        rc = CHAIN_INTERRUPT;
    }
    else
    {
        /* spurious, claim that this PSD handled it */
        SpuriousIRQ15Counter++;

       /* because the slave PIC is chained to the master PIC, we still need to send an EOI to the master PIC */
       _outp(0x20,0x20);

        rc = NO_ERROR;
    }
    IRQEnable(flags);

    return rc;
}

APIRET PSDENTRY psd_install(INSTALL *p)   /* required */
{
    VMALLOC vmac;
    APIRET  rc;

    gInfo.pPSDHlpRouter = p->pPSDHlpRouter;
    gInfo.pParmString   = p->pParmString;
    gInfo.pPSDPLMA      = p->pPSDPLMA;
    gInfo.sizePLMA      = p->sizePLMA;

    /* now probe if this PSD supports the current HW platform */
    vmac.addr   = 0xF0000;
    vmac.cbsize = 64*1024UL;
    vmac.flags  = VMALLOC_PHYS;
    rc = PSDHelp(gInfo.pPSDHlpRouter,PSDHLP_VMALLOC,&vmac);
    if (rc == NO_ERROR)
    {
        rc = PSDHelp(gInfo.pPSDHlpRouter,PSDHLP_VMFREE,vmac.addr);
    }

    return NO_ERROR;
}

APIRET PSDENTRY psd_deinstall(VOID)       /* required */
{
    return NO_ERROR;
}


APIRET PSDENTRY psd_init(INIT *p)         /* required */
{
    APIRET  rc;
    SET_IRQ set_irq;
    ULONG flags,data;

    set_irq.irq     = 7;
    set_irq.flags   = 0;
    set_irq.vector  = 0;
    set_irq.handler = (P_F_2)IRQ7Handler;
    rc = PSDHelp(gInfo.pPSDHlpRouter,PSDHLP_SET_IRQ,&set_irq);

    set_irq.irq     = 15;
    set_irq.flags   = 0;
    set_irq.vector  = 0;
    set_irq.handler = (P_F_2)IRQ15Handler;
    rc = PSDHelp(gInfo.pPSDHlpRouter,PSDHLP_SET_IRQ,&set_irq);


    /* install new exception 16 handler (which branches to original handler) */
    ReplaceMFHandler();

    /* mask IRQ13, we are going to switch over using exception 16 for */
    /* floating point exceptions, see INIT_USE_FPERR_TRAP flag */
    flags = IRQDisable();
    data  = _inp(0xA1);
    data  |= 0x20;    /* mask IRQ13 */
    IoDelay();
    _outp(0xA1,(UCHAR)data);
    IRQEnable(flags);

    p->flags        = INIT_GLOBAL_IRQ_ACCESS | INIT_USE_FPERR_TRAP | INIT_TIMER_CPU0;
    p->version      = 0x10000UL;
    
    return NO_ERROR;
}

APIRET PSDENTRY psd_proc_init(VOID)       /* only required for MP */
{
    return NO_ERROR;
}

APIRET PSDENTRY psd_start_proc(ULONG ulProcIdx) /* only required for MP */
{
    if (ulProcIdx > 0)
    {
        return ERROR_GENERIC;
    }
    else
    {
        return NO_ERROR;
    }
}

ULONG PSDENTRY psd_get_num_of_procs(VOID)
{
    ULONG ulNumProcs = 1UL;
    return ulNumProcs;
}

APIRET PSDENTRY psd_gen_ipi(ULONG ulProcIdx) /* only required for MP */
{
    if (ulProcIdx > 0)
    {
        return ERROR_GENERIC;
    }
    else
    {
        return NO_ERROR;
    }
}

APIRET PSDENTRY psd_end_ipi(ULONG ulProcIdx) /* only required for MP */
{
    if (ulProcIdx > 0)
    {
        return ERROR_GENERIC;
    }
    else
    {
        return NO_ERROR;
    }
}

APIRET PSDENTRY psd_port_io(PORT_IO *p)              /* optional */
{
    ULONG rc = ERROR_GENERIC;

    if (p->port & 0xFFFF0000)
    {
        return rc;
    }

    switch(p->flags & IO_FLAGMASK)
    {
        case IO_READ_BYTE:
            p->data = _inp(p->port);
            rc = NO_ERROR;
            break;

        case IO_READ_WORD:
            p->data = _inpw(p->port);
            rc = NO_ERROR;
            break;

        case IO_READ_DWORD:
            p->data = _inpd(p->port);
            rc = NO_ERROR;
            break;

        case IO_WRITE_BYTE:
            _outp(p->port,p->data);
            rc = NO_ERROR;
            break;

        case IO_WRITE_WORD:
            _outpw(p->port,p->data);
            rc = NO_ERROR;
            break;

        case IO_WRITE_DWORD:
            _outpd(p->port,p->data);
            rc = NO_ERROR;
            break;

        default:
            rc = ERROR_GENERIC;
            break;
    }
    return rc;
}

APIRET PSDENTRY psd_irq_mask(PSD_IRQ *p)             /* optional */
{

    /*
    DevHlp_SetIRQMask:
    AL = IRQ number
    AH = IRQ bitmask (1 = disable,0=enable)

    DevHlp_GetIRQMask:
    AL = IRQ number
    EAX = returned low bit (0=enable,1=disable)
    */

    /*
       However, this PSD routine always receives a bit vector
       where LSB is for IRQ0 and MSB for IRQ31
       a set bits means to mask, a reset bits means to unmask this IRQ
    */

    ULONG data,flags;
    ULONG rc = NO_ERROR;

    if (p->procnum > 0)
    {
        return ERROR_GENERIC;
    }

    flags = IRQDisable();
    if (p->flags != IRQ_NEWMASK)
    {
       data=0UL;
       data = _inp(0xA1);
       IoDelay();
       data <<= 8;
       data |= _inp(0x21);
    }

    switch(p->flags)
    {
        case IRQ_MASK:
            data |= p->data;
            _outp(0x21,(UCHAR)data);
            IoDelay();
            data >>= 8;
            _outp(0xA1,(UCHAR)data);
            rc = NO_ERROR;
            break;

        case IRQ_UNMASK:
            data &= ~(p->data);
            _outp(0x21,(UCHAR)data);
            IoDelay();
            data >>= 8;
            _outp(0xA1,(UCHAR)data);
            rc = NO_ERROR;
            break;

        case IRQ_GETMASK:
            p->data = data;
            rc = NO_ERROR;
            break;

        case IRQ_NEWMASK:
            data = p->data;
            _outp(0x21,(UCHAR)data);
            IoDelay();
            data >>= 8;
            _outp(0xA1,(UCHAR)data);
            rc = NO_ERROR;
            break;

        default:
            rc = ERROR_GENERIC;
            break;
    }
    IRQEnable(flags);

    return rc;
}

APIRET PSDENTRY psd_irq_reg(PSD_IRQ *p)              /* optional */
{
    ULONG rc;
    ULONG data,flags;

    if (p->procnum > 0)
    {
        return ERROR_GENERIC;
    }

    /* need to return a bit vector in p->data where LSB is IRQ0 and MSB is IRQ31 */

    flags = IRQDisable();
    switch(p->flags)
    {
        case IRQ_READ_IRR:
            _outp(0x20,0x0A);
            _outp(0xA0,0x0A);
            IoDelay();
            data = _inp(0xA0);
            data <<= 8;
            data |= _inp(0x20);
            p->data = data;
            rc = NO_ERROR;
            break;

        case IRQ_READ_ISR:
            _outp(0x20,0x0B);
            _outp(0xA0,0x0B);
            IoDelay();
            data = _inp(0xA0);
            data <<= 8;
            data |= _inp(0x20);
            p->data = data;
            rc = NO_ERROR;
            break;

        default:
            rc = ERROR_GENERIC;
            break;
    }
    IRQEnable(flags);

    return rc;
}

APIRET PSDENTRY psd_irq_eoi(PSD_IRQ *p)              /* optional */
{
    ULONG flags;

    if (p->procnum > 0)
    {
        return ERROR_GENERIC;
    }

    if (p->data & 0xFFFFFFF0)                       /* IRQs 0-15 */
    {
        return ERROR_GENERIC;
    }

    flags = IRQDisable();
    if (p->data >= 8)
    {
       _outp(0xA0,0x20);
       IoDelay();
    }
    _outp(0x20,0x20);
    IRQEnable(flags);

    return NO_ERROR;
}

APIRET PSDENTRY psd_app_comm(ULONG funcNum,PVOID arg)/* optional */
{
    return NO_ERROR;
}

APIRET PSDENTRY psd_set_adv_int_mode(VOID)           /* optional */
{
    return NO_ERROR;
}

APIRET PSDENTRY psd_set_proc_state(ULONG procstate)
{
    return NO_ERROR;
}

APIRET PSDENTRY psd_query_system_timer(PSD_QRYTMR *p)
{
   QueryTimerCounter++;
   TimerCounterVirtAddr = p->pqwTmr;
   return CONTINUE_DEFAULT;
}

APIRET PSDENTRY psd_set_system_timer(PSD_SETTMR *p)
{
   SetTimerCounter++;
   TimerRollOverVirtAddr= p->pqwTmrRollover;
   return CONTINUE_DEFAULT;
}

APIRET PSDENTRY psd_reset_mode(VOID)                 /* optional */
{
    return NO_ERROR;
}

#if 0
APIRET PSDENTRY psd_mtrace_start(... unknown args ...)          /* optional */
{
    return NO_ERROR;
}
#endif

