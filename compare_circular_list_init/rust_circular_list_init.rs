#![no_std]

use core::ptr;

#[repr(C)]
pub struct CircularNode {
    pub info: u32,
    pub link: *mut CircularNode,
}

#[repr(C)]
pub struct CircularList {
    pub ptr: *mut CircularNode,
    pub avail: *mut CircularNode,
}

// We don't need to mangle or export the internal helper, 
// we WANT the compiler to inline this directly into the main function.
unsafe fn init_circular_list_storage_pool(
    circular_list: *mut CircularList,
    mut nodes: u32,
) -> *mut CircularNode {
    unsafe {
        let mut avail = circular_list.add(1) as *mut CircularNode;
        (*avail).link = ptr::null_mut();
        nodes -= 1;

        while nodes > 0 {
            let tmp = avail.add(1);
            (*tmp).link = avail;
            avail = tmp;
            nodes -= 1;
        }
        avail
    }
}

// THE FIX: Wrap the attribute in unsafe() to satisfy the 2024 edition linker safety rules
#[unsafe(no_mangle)]
pub unsafe extern "C" fn rust_circular_list_init(memory: *mut u8, nodes: u32) -> *mut CircularList {
    unsafe {
        let circular_list = memory as *mut CircularList;
        (*circular_list).ptr = ptr::null_mut();

        let avail = init_circular_list_storage_pool(circular_list, nodes);
        (*circular_list).avail = avail;

        circular_list
    }
}
