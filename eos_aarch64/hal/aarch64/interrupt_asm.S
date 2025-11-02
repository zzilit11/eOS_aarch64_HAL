.text

.global hal_disable_interrupt
hal_disable_interrupt:
    mrs     x0, daif       // Read current DAIF state
    msr daifset, #2        // Set the I bit to disable IRQs
    isb                    // Instruction Synchronization Barrier
    ret

.global hal_enable_interrupt
hal_enable_interrupt:
    msr daifclr, #2        // Clear the I bit to enable IRQs
    isb                    // Instruction Synchronization Barrier
    ret

.global hal_restore_interrupt
hal_restore_interrupt:
    // Input: x0 - previous DAIF state
    msr daif, x0           // Restore DAIF register with previous state
    isb                    // Instruction Synchronization Barrier
    ret