/*
 * \brief  Implementation of linux/wait.h
 * \author Norman Feske
 * \date   2015-09-09
 */

/*
 * Copyright (C) 2015 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

void prepare_to_wait(wait_queue_head_t *q, wait_queue_t *w, int state)
{
	if (!q) {
		PWRN("prepare_to_wait: wait_queue_head_t is 0, ignore, called from: %p",
		    __builtin_return_address(0));
		return;
	}

	Wait_list *list = static_cast<Wait_list *>(q->list);
	Lx::Task *task = Lx::scheduler().current();

	task->wait_enqueue(list);
}


void prepare_to_wait_exclusive(wait_queue_head_t *q, wait_queue_t *w, int state)
{
	prepare_to_wait(q, w, state);
}


void finish_wait(wait_queue_head_t *q, wait_queue_t *w)
{
	if (!q) {
		PWRN("finish_wait: wait_queue_head_t is 0, ignore, called from: %p",
		    __builtin_return_address(0));
		return;
	}

	Wait_list *list = static_cast<Wait_list *>(q->list);
	Lx::Task *task = Lx::scheduler().current();

	task->wait_dequeue(list);
}
