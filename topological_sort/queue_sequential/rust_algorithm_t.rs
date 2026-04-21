#![no_std]
#![crate_type = "lib"]

#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    loop {}
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Pair {
    pub j: u32,
    pub k: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct TopologicalNode {
    pub succ: u32,
    pub next: u32,
}

const NULL: u32 = u32::MAX;

// 1. Tell Rust about your handwritten C/ASM functions!
unsafe extern "C" {
    fn asm_balloc(size: u32) -> *mut u32;
    fn asm_balloc_free(ptr: *mut u32);
}

// 2. The signature now EXACTLY matches `c_algorithm_t` (4 arguments)
#[unsafe(no_mangle)]
pub extern "C" fn rust_algorithm_t(
    n: u8,
    input_pairs: *const Pair,
    input_pairs_len: u8,
    output: *mut u32,
) -> u8 {
    let n_usize = n as usize;
    let pairs_len = input_pairs_len as usize;

    unsafe {
        // 3. Allocate memory using your bump allocator
        let count_qlink_ptr = asm_balloc(((n_usize + 1) * 4) as u32);
        if count_qlink_ptr.is_null() { return 0; }

        let top_ptr = asm_balloc(((n_usize + 1) * 4) as u32);
        if top_ptr.is_null() { return 0; }

        let nodes_ptr = asm_balloc((pairs_len * 8) as u32) as *mut TopologicalNode;
        if nodes_ptr.is_null() { return 0; }

        // 4. Convert raw pointers to unchecked slices
        let pairs = core::slice::from_raw_parts(input_pairs, pairs_len);
        let out = core::slice::from_raw_parts_mut(output, n_usize);
        let count_qlink = core::slice::from_raw_parts_mut(count_qlink_ptr, n_usize + 1);
        let top = core::slice::from_raw_parts_mut(top_ptr, n_usize + 1);
        let nodes = core::slice::from_raw_parts_mut(nodes_ptr, pairs_len);

        // T1: Zero out arrays
        for i in 0..=n_usize {
            *count_qlink.get_unchecked_mut(i) = 0;
            *top.get_unchecked_mut(i) = NULL;
        }

        let mut node_alloc_idx = 0;

        // T2 & T3: Record relations
        for pair in pairs.iter().rev() {
            let j = pair.j as usize;
            let k = pair.k;

            *count_qlink.get_unchecked_mut(k as usize) += 1;

            let p_idx = node_alloc_idx as u32;
            node_alloc_idx += 1;
            
            let next_val = *top.get_unchecked(j);
            *nodes.get_unchecked_mut(p_idx as usize) = TopologicalNode {
                succ: k,
                next: next_val,
            };
            
            *top.get_unchecked_mut(j) = p_idx;
        }

        // T4: Scan for zeros
        let mut rear = 0;
        *count_qlink.get_unchecked_mut(0) = 0;

        for k in (1..=n_usize).rev() {
            if *count_qlink.get_unchecked(k) == 0 {
                *count_qlink.get_unchecked_mut(rear) = k as u32;
                rear = k;
            }
        }

        let mut front = *count_qlink.get_unchecked(0) as usize;
        let mut queue_output_counter = 0;
        let mut remaining = n;

        // T5 - T7
        while remaining != 0 {
            if front == 0 { break; }

            *out.get_unchecked_mut(queue_output_counter) = front as u32;
            remaining -= 1;

            let mut p_idx = *top.get_unchecked(front);

            // T6: Erase relations
            while p_idx != NULL {
                let node = nodes.get_unchecked(p_idx as usize);
                let succ = node.succ as usize;

                *count_qlink.get_unchecked_mut(succ) -= 1;

                if *count_qlink.get_unchecked(succ) == 0 {
                    *count_qlink.get_unchecked_mut(rear) = succ as u32;
                    rear = succ;
                }
                p_idx = node.next;
            }

            front = *count_qlink.get_unchecked(front) as usize;
            queue_output_counter += 1;
        }

        // Cleanup and return
        asm_balloc_free(count_qlink_ptr);
        if remaining == 0 { 1 } else { 0 }
    }
}
