

#include <rom_session/client.h>

using namespace Genode;

Rom_session_client::Rom_session_client(Rom_session_capability session)
: Rpc_client<Rom_session>(session) { }

Rom_dataspace_capability Rom_session_client::dataspace() { return call<Rpc_dataspace>(); }

bool Rom_session_client::update() { return call<Rpc_update>(); }

void Rom_session_client::sigh(Signal_context_capability cap) { call<Rpc_sigh>(cap); }


#include <ram_session/client.h>

Ram_session_client::Ram_session_client(Ram_session_capability session)
: Rpc_client<Ram_session>(session) { }

Ram_dataspace_capability Ram_session_client::alloc(size_t size,
                                                   Cache_attribute cached) {
	return call<Rpc_alloc>(size, cached); }

void Ram_session_client::free(Ram_dataspace_capability ds) { call<Rpc_free>(ds); }

int Ram_session_client::ref_account(Ram_session_capability ram_session) {
	return call<Rpc_ref_account>(ram_session); }

int Ram_session_client::transfer_quota(Ram_session_capability ram_session, size_t amount) {
	return call<Rpc_transfer_quota>(ram_session, amount); }

size_t Ram_session_client::quota() { return call<Rpc_quota>(); }

size_t Ram_session_client::used() { return call<Rpc_used>(); }


#include <rm_session/client.h>

Rm_session_client::Rm_session_client(Rm_session_capability session)
: Rpc_client<Rm_session>(session) { }

Rm_session::Local_addr Rm_session_client::attach(Dataspace_capability ds, size_t size,
                                                 off_t offset, bool use_local_addr,
                                                 Local_addr local_addr,
                                                 bool executable)
{
	return call<Rpc_attach>(ds, size, offset,
	                        use_local_addr, local_addr,
	                        executable);
}

void Rm_session_client::detach(Local_addr local_addr) {
	call<Rpc_detach>(local_addr); }

Pager_capability Rm_session_client::add_client(Thread_capability thread) {
	return call<Rpc_add_client>(thread); }

void Rm_session_client::remove_client(Pager_capability pager) {
	call<Rpc_remove_client>(pager); }

void Rm_session_client::fault_handler(Signal_context_capability handler) {
	call<Rpc_fault_handler>(handler); }

Rm_session::State Rm_session_client::state() {
	return call<Rpc_state>(); }

Dataspace_capability Rm_session_client::dataspace() {
	return call<Rpc_dataspace>(); }

