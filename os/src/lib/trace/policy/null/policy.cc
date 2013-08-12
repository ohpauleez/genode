#include <trace/policy.h>

using namespace Genode;

size_t max_event_size()
{
	return 0;
}

size_t rpc_call(char *dst, size_t dst_len, char const *rpc_name, Msgbuf_base const &)
{
	return 0;
}

size_t rpc_returned(char *dst, size_t dst_len, char const *rpc_name, Msgbuf_base const &)
{
	return 0;
}

size_t rpc_dispatch(char *dst, size_t dst_len, char const *rpc_name, Msgbuf_base const &)
{
	return 0;
}

size_t rpc_reply(char *dst, size_t dst_len, char const *rpc_name, Msgbuf_base const &)
{
	return 0;
}

size_t signal_submit(char *dst, size_t dst_len, void const *this_ptr)
{
	return 0;
}

size_t signal_receive(char *dst, size_t dst_len, Signal_context const *context)
{
	return 0;
}

