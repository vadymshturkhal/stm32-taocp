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

unsafe extern "C" {
    fn asm_balloc(size: u32) -> *mut u32;
    fn asm_balloc_free(ptr: *mut u32);
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_algorithm_t_stack_sequential(
    n: u8,
    input_pairs: *const Pair,
    input_pairs_len: u8,
    output: *mut u32,
) -> u8 {
    let n_usize = n as usize;
    let pairs_len = input_pairs_len as usize;

    unsafe {
        let count_len = n_usize + 1;
        let top_len = n_usize + 1;
        let total_bytes = (count_len * 4) + (top_len * 4) + (pairs_len * 8);
        
        let master_ptr = asm_balloc(total_bytes as u32);
        if master_ptr.is_null() { return 0; }

        let count_slink = core::slice::from_raw_parts_mut(master_ptr, count_len);
        let top = core::slice::from_raw_parts_mut(master_ptr.add(count_len), top_len);
        
        let nodes_raw_ptr = master_ptr.add(count_len + top_len) as *mut TopologicalNode;
        let nodes = core::slice::from_raw_parts_mut(nodes_raw_ptr, pairs_len);

        let pairs = core::slice::from_raw_parts(input_pairs, pairs_len);
        let out = core::slice::from_raw_parts_mut(output, n_usize);


        // Optimization 2: Intrinsic Memory Fills (Hypothetically forces LLVM to emit STMIA (Store Multiple) instructions)
        count_slink.fill(0);
        top.fill(NULL);

        let mut node_alloc_idx = 0;

        for pair in pairs.iter().rev() {
            let j = pair.j as usize;
            let k = pair.k;

            *count_slink.get_unchecked_mut(k as usize) += 1;

            let p_idx = node_alloc_idx as u32;
            node_alloc_idx += 1;
            
            let next_val = *top.get_unchecked(j);
            *nodes.get_unchecked_mut(p_idx as usize) = TopologicalNode {
                succ: k,
                next: next_val,
            };
            
            *top.get_unchecked_mut(j) = p_idx;
        }

        let mut stack_top = 0;
        for k in (1..=n_usize).rev() {
            if *count_slink.get_unchecked(k) == 0 {
                *count_slink.get_unchecked_mut(k) = stack_top; 
                stack_top = k as u32;
            }
        }

        let mut queue_output_counter = 0;
        
        // Optimization 1: Killed `remaining`
        while stack_top != 0 {
            *out.get_unchecked_mut(queue_output_counter) = stack_top;
            queue_output_counter += 1;

            let mut p_idx = *top.get_unchecked(stack_top as usize);
            stack_top = *count_slink.get_unchecked(stack_top as usize); 

            while p_idx != NULL {
                let node = nodes.get_unchecked(p_idx as usize);
                let succ = node.succ as usize;


                // Optimization 3: Explicit Reference Hijack (Proves no-alias to LLVM, ensuring native `SUBS` usage)
                let c_ptr = count_slink.get_unchecked_mut(succ);
                *c_ptr -= 1;

                if *c_ptr == 0 {
                    *c_ptr = stack_top; 
                    stack_top = succ as u32;
                }
                
                p_idx = node.next;
            }
        }

        asm_balloc_free(master_ptr);
        
        // Return 1 if we output exactly `n` elements (no cycles)
        if queue_output_counter == n_usize { 1 } else { 0 }
    }
}
