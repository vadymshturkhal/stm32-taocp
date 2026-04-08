#![no_std]

use core::panic::PanicInfo;

// -----------------------------------------------------------------------------
// FFI Definitions
// -----------------------------------------------------------------------------
unsafe extern "C" {
    fn asm_balloc(size: u32) -> *mut core::ffi::c_void;
    fn asm_balloc_free(ptr: *mut core::ffi::c_void);
}

// -----------------------------------------------------------------------------
// Struct Definitions 
// -----------------------------------------------------------------------------
#[repr(C)]
pub struct Node {
    pub info: u32,
    pub link: *mut Node,
}

#[repr(C)]
pub struct Queue {
    pub front: *mut Node,
    pub rear: *mut *mut Node,
    pub avail: *mut Node,
}

// -----------------------------------------------------------------------------
// Queue Initialization
// -----------------------------------------------------------------------------

unsafe fn init_queue_storage_pool(queue: *mut Queue, mut size: u32) -> *mut Node {
    let mut current = queue.offset(1) as *mut Node;
    let head = current;

    // Clean, forward-linking loop to prevent LLVM Duff's Device unrolling bloat
    while size > 1 {
        let next = current.offset(1);
        (*current).link = next;
        current = next;
        size -= 1;
    }
    
    // Null-terminate the final node of the Avail pool
    (*current).info = 1;
    (*current).link = core::ptr::null_mut();

    head
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn rust_create_queue(memory: *mut core::ffi::c_void, size: u32) -> *mut Queue {
    let queue = memory as *mut Queue;
    
    (*queue).front = core::ptr::null_mut();
    (*queue).rear = core::ptr::addr_of_mut!((*queue).front);
    (*queue).avail = init_queue_storage_pool(queue, size);

    queue
}

// -----------------------------------------------------------------------------
// The Hot-Loop Execution (Inlined & Register-Pinned)
// -----------------------------------------------------------------------------

#[unsafe(no_mangle)]
pub unsafe extern "C" fn rust_perform_queue_operations(max_nodes: u16) -> u8 {
    if max_nodes == 0 {
        return 0;
    }

    let node_size = core::mem::size_of::<Node>() as u32;
    let queue_size = core::mem::size_of::<Queue>() as u32;
    let total_size = (max_nodes as u32 * node_size) + queue_size;

    let queue_memory = asm_balloc(total_size);
    if queue_memory.is_null() {
        return 0;
    }

    let queue = rust_create_queue(queue_memory, max_nodes as u32);

    // =========================================================================
    // ENQUEUE HOT LOOP: L0 Caching + Deferred NULL Linkage
    // =========================================================================
    let mut avail = (*queue).avail;
    let mut rear = (*queue).rear;
    let mut i = max_nodes as u32;

    while i > 0 {
        if avail.is_null() { 
            asm_balloc_free(queue_memory);
            return 0; 
        }
        
        let p = avail;
        avail = (*avail).link;     // Advance Avail

        (*p).info = i;
        
        // NO SAFETY TAX: We deliberately leave (*p).link dirty here!

        *rear = p;                 // Torvalds Linkage
        rear = core::ptr::addr_of_mut!((*p).link); // Advance Rear

        i -= 1;
    }
    
    // CAPPING THE MATRIX: Apply the Deferred NULL exactly once
    *rear = core::ptr::null_mut(); 

    // FLUSH L0 CACHE TO SRAM
    (*queue).avail = avail;
    (*queue).rear = rear;


    // =========================================================================
    // DEQUEUE HOT LOOP: Redundant Load Elimination
    // =========================================================================
    let mut front = (*queue).front;
    // 'avail' is already in our local register context from the Enqueue loop

    i = max_nodes as u32;
    while i > 0 {
        if front.is_null() { 
            asm_balloc_free(queue_memory);
            return 0; 
        }
        
        let p = front;
        front = (*p).link;         // Advance Front

        (*p).link = avail;         // Return to Avail pool
        avail = p;                 // Update Avail head

        i -= 1;
    }

    // FLUSH L0 CACHE TO SRAM AND FIX REAR POINTER
    if front.is_null() {
        (*queue).rear = core::ptr::addr_of_mut!((*queue).front);
    }
    (*queue).front = front;
    (*queue).avail = avail;

    // =========================================================================

    asm_balloc_free(queue_memory);
    1
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}
