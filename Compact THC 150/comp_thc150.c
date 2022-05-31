#include "../cnc.h"

#ifdef ENABLE_INTERPOLATOR_MODULES

#ifndef ARC_ON
#define ARC_ON DOUT0
#endif

#ifndef ARC_OK
#define ARC_OK DIN0
#endif
#ifndef TORCH_DOWN
#define TORCH_DOWN DOUT1
#endif
#ifndef TORCH_UP
#define TORCH_UP DOUT2
#endif

//#define THC_DIR_INV

#ifndef TORCH_STEPPER
#define TORCH_STEPPER 2
#endif

#define _TORCH_STEPPER_MASK_(X) STEP##X##_ITP_MASK
#define TORCH_STEPPER_MASK(X) _TORCH_STEPPER_MASK_(X)



#if (ARC_ON < 0) || (ARC_OK < 0) || (TORCH_DOWN < 0) || (TORCH_UP < 0)
#error Compact THC 150 is missing some pins
#endif

// for better performance hook directly
void mod_itp_step_hook(itp_segment_t *itp_rt_sgm, uint8_t *stepmask)
{
    if (mcu_get_output(ARC_ON) && mcu_get_input(ARC_OK))
    {
        if (mcu_get_input(TORCH_DOWN))
        {
#ifndef THC_DIR_INV
            io_set_dirs(itp_rt_sgm->block->dirbits | TORCH_STEPPER_MASK(TORCH_STEPPER));
#else
            io_set_dirs(itp_rt_sgm->block->dirbits & ~TORCH_STEPPER_MASK(TORCH_STEPPER));
#endif
            *stepmask |= TORCH_STEPPER_MASK(TORCH_STEPPER);
        }
        else if (mcu_get_input(TORCH_UP))
        {
#ifndef THC_DIR_INV
            io_set_dirs(itp_rt_sgm->block->dirbits & ~TORCH_STEPPER_MASK(TORCH_STEPPER));
#else
            io_set_dirs(itp_rt_sgm->block->dirbits | TORCH_STEPPER_MASK(TORCH_STEPPER));
#endif
            *stepmask |= TORCH_STEPPER_MASK(TORCH_STEPPER);
        }
        else
        {
            io_set_dirs(itp_rt_sgm->block->dirbits);
        }
    }
}
#endif

DECL_MODULE(comp_thc150)
{
#ifdef ENABLE_MAIN_LOOP_MODULES
    ADD_LISTENER(cnc_reset_delegate, ucnc_lcd_init, cnc_reset_event);
    ADD_LISTENER(cnc_dotasks_delegate, ucnc_lcd_refresh, cnc_dotasks_event);
#else
#warning "Interpolator extensions are not enabled. Compact THC 150 will not work."
#endif
}