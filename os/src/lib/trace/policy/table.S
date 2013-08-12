#if defined (__i386__) || defined (__arm__)
#define addr_t .long
#elif  defined (__x86_64__)
#define addr_t .quad
#endif

.globl max_event_size
.globl rpc_call
.globl rpc_returned
.globl rpc_dispatch
.globl rpc_reply
.globl signal_submit
.globl signal_receive

.text

policy_jump_table:
	addr_t max_event_size
	addr_t rpc_call
	addr_t rpc_returned
	addr_t rpc_dispatch
	addr_t rpc_reply
	addr_t signal_submit
	addr_t signal_receive
