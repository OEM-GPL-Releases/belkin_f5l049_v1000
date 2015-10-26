/* 
   Unix SMB/CIFS implementation.
   oplock processing
   Copyright (C) Andrew Tridgell 1992-1998
   Copyright (C) Jeremy Allison 1998 - 2001
   Copyright (C) Volker Lendecke 2005
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#define DBGC_CLASS DBGC_LOCKING
#include "includes.h"

/* Current number of oplocks we have outstanding. */
static int32 exclusive_oplocks_open = 0;
static int32 level_II_oplocks_open = 0;
BOOL global_client_failed_oplock_break = False;

extern uint32 global_client_caps;
extern int smb_read_error;

static struct kernel_oplocks *koplocks;

/****************************************************************************
 Get the number of current exclusive oplocks.
****************************************************************************/

int32 get_number_of_exclusive_open_oplocks(void)
{
  return exclusive_oplocks_open;
}

/****************************************************************************
 Return True if an oplock message is pending.
****************************************************************************/

BOOL oplock_message_waiting(fd_set *fds)
{
	if (koplocks && koplocks->msg_waiting(fds)) {
		return True;
	}

	return False;
}

/****************************************************************************
 Find out if there are any kernel oplock messages waiting and process them
 if so. pfds is the fd_set from the main select loop (which contains any
 kernel oplock fd if that's what the system uses (IRIX). If may be NULL if
 we're calling this in a shutting down state.
****************************************************************************/

void process_kernel_oplocks(fd_set *pfds)
{
	/*
	 * We need to check for kernel oplocks before going into the select
	 * here, as the EINTR generated by the linux kernel oplock may have
	 * already been eaten. JRA.
	 */

	if (!koplocks) {
		return;
	}

	while (koplocks->msg_waiting(pfds)) { 
		files_struct *fsp;
		char msg[MSG_SMB_KERNEL_BREAK_SIZE];

		fsp = koplocks->receive_message(pfds);

		if (fsp == NULL) {
			DEBUG3( ("Kernel oplock message announced, but none "
				  "received\n"));
			return;
		}

		/* Put the kernel break info into the message. */
		SDEV_T_VAL(msg,0,fsp->dev);
		SINO_T_VAL(msg,8,fsp->inode);
		SIVAL(msg,16,fsp->fh->file_id);

		/* Don't need to be root here as we're only ever
		   sending to ourselves. */

		message_send_pid(pid_to_procid(sys_getpid()),
				 MSG_SMB_KERNEL_BREAK,
				 &msg, MSG_SMB_KERNEL_BREAK_SIZE, True);
	}
}

/****************************************************************************
 Attempt to set an oplock on a file. Always succeeds if kernel oplocks are
 disabled (just sets flags). Returns True if oplock set.
****************************************************************************/

BOOL set_file_oplock(files_struct *fsp, int oplock_type)
{
	if (koplocks && !koplocks->set_oplock(fsp, oplock_type)) {
		return False;
	}

	fsp->oplock_type = oplock_type;
	fsp->sent_oplock_break = NO_BREAK_SENT;
	if (oplock_type == LEVEL_II_OPLOCK) {
		level_II_oplocks_open++;
	} else {
		exclusive_oplocks_open++;
	}

	DEBUG5(("set_file_oplock: granted oplock on file %s, 0x%x/%.0f/%lu, "
		    "tv_sec = %x, tv_usec = %x\n",
		 fsp->fsp_name, (unsigned int)fsp->dev, (double)fsp->inode,
		 fsp->fh->file_id, (int)fsp->open_time.tv_sec,
		 (int)fsp->open_time.tv_usec ));

	return True;
}

/****************************************************************************
 Attempt to release an oplock on a file. Decrements oplock count.
****************************************************************************/

void release_file_oplock(files_struct *fsp)
{
	if ((fsp->oplock_type != NO_OPLOCK) &&
	    (fsp->oplock_type != FAKE_LEVEL_II_OPLOCK) &&
	    koplocks) {
		koplocks->release_oplock(fsp);
	}

	if (fsp->oplock_type == LEVEL_II_OPLOCK) {
		level_II_oplocks_open--;
	} else if (EXCLUSIVE_OPLOCK_TYPE(fsp->oplock_type)) {
		exclusive_oplocks_open--;
	}

	SMB_ASSERT(exclusive_oplocks_open>=0);
	SMB_ASSERT(level_II_oplocks_open>=0);
	
	fsp->oplock_type = NO_OPLOCK;
	fsp->sent_oplock_break = NO_BREAK_SENT;
	
	flush_write_cache(fsp, OPLOCK_RELEASE_FLUSH);

	TALLOC_FREE(fsp->oplock_timeout);
}

/****************************************************************************
 Attempt to downgrade an oplock on a file. Doesn't decrement oplock count.
****************************************************************************/

static void downgrade_file_oplock(files_struct *fsp)
{
	if (koplocks) {
		koplocks->release_oplock(fsp);
	}
	fsp->oplock_type = LEVEL_II_OPLOCK;
	exclusive_oplocks_open--;
	level_II_oplocks_open++;
	fsp->sent_oplock_break = NO_BREAK_SENT;
}

/****************************************************************************
 Remove a file oplock. Copes with level II and exclusive.
 Locks then unlocks the share mode lock. Client can decide to go directly
 to none even if a "break-to-level II" was sent.
****************************************************************************/

BOOL remove_oplock(files_struct *fsp)
{
	SMB_DEV_T dev = fsp->dev;
	SMB_INO_T inode = fsp->inode;
	BOOL ret;
	struct share_mode_lock *lck;

	/* Remove the oplock flag from the sharemode. */
	lck = get_share_mode_lock(NULL, fsp->dev, fsp->inode, NULL, NULL);
	if (lck == NULL) {
		DEBUG0(("remove_oplock: failed to lock share entry for "
			 "file %s\n", fsp->fsp_name ));
		return False;
	}
	ret = remove_share_oplock(lck, fsp);
	if (!ret) {
		DEBUG0(("remove_oplock: failed to remove share oplock for "
			 "file %s fnum %d, 0x%x/%.0f\n",
			 fsp->fsp_name, fsp->fnum, (unsigned int)dev,
			 (double)inode));
	}
	release_file_oplock(fsp);
	TALLOC_FREE(lck);
	return ret;
}

/*
 * Deal with a reply when a break-to-level II was sent.
 */
BOOL downgrade_oplock(files_struct *fsp)
{
	SMB_DEV_T dev = fsp->dev;
	SMB_INO_T inode = fsp->inode;
	BOOL ret;
	struct share_mode_lock *lck;

	lck = get_share_mode_lock(NULL, fsp->dev, fsp->inode, NULL, NULL);
	if (lck == NULL) {
		DEBUG0(("downgrade_oplock: failed to lock share entry for "
			 "file %s\n", fsp->fsp_name ));
		return False;
	}
	ret = downgrade_share_oplock(lck, fsp);
	if (!ret) {
		DEBUG0(("downgrade_oplock: failed to downgrade share oplock "
			 "for file %s fnum %d, dev = %x, inode = %.0f\n",
			 fsp->fsp_name, fsp->fnum, (unsigned int)dev,
			 (double)inode));
	}

	downgrade_file_oplock(fsp);
	TALLOC_FREE(lck);
	return ret;
}

/****************************************************************************
 Return the fd (if any) used for receiving oplock notifications.
****************************************************************************/

int oplock_notify_fd(void)
{
	if (koplocks) {
		return koplocks->notification_fd;
	}

	return -1;
}

/****************************************************************************
 Set up an oplock break message.
****************************************************************************/

static char *new_break_smb_message(TALLOC_CTX *mem_ctx,
				   files_struct *fsp, uint8 cmd)
{
	char *result = TALLOC_ARRAY(mem_ctx, char, smb_size + 8*2 + 0);

	if (result == NULL) {
		DEBUG0( ("talloc failed\n"));
		return NULL;
	}

	memset(result,'\0',smb_size);
	set_message(result,8,0,True);
	SCVAL(result,smb_com,SMBlockingX);
	SSVAL(result,smb_tid,fsp->conn->cnum);
	SSVAL(result,smb_pid,0xFFFF);
	SSVAL(result,smb_uid,0);
	SSVAL(result,smb_mid,0xFFFF);
	SCVAL(result,smb_vwv0,0xFF);
	SSVAL(result,smb_vwv2,fsp->fnum);
	SCVAL(result,smb_vwv3,LOCKING_ANDX_OPLOCK_RELEASE);
	SCVAL(result,smb_vwv3+1,cmd);
	return result;
}

/****************************************************************************
 Function to do the waiting before sending a local break.
****************************************************************************/

static void wait_before_sending_break(void)
{
	long wait_time = (long)lp_oplock_break_wait_time();

	if (wait_time) {
		smb_msleep(wait_time);
	}
}

/****************************************************************************
 Ensure that we have a valid oplock.
****************************************************************************/

static files_struct *initial_break_processing(SMB_DEV_T dev, SMB_INO_T inode, unsigned long file_id)
{
	files_struct *fsp = NULL;

	if( DEBUGLVL( 3 ) ) {
		dbgtext( "initial_break_processing: called for 0x%x/%.0f/%u\n",
			(unsigned int)dev, (double)inode, (int)file_id);
		dbgtext( "Current oplocks_open (exclusive = %d, levelII = %d)\n",
			exclusive_oplocks_open, level_II_oplocks_open );
	}

	/*
	 * We need to search the file open table for the
	 * entry containing this dev and inode, and ensure
	 * we have an oplock on it.
	 */

	fsp = file_find_dif(dev, inode, file_id);

	if(fsp == NULL) {
		/* The file could have been closed in the meantime - return success. */
		if( DEBUGLVL( 3 ) ) {
			dbgtext( "initial_break_processing: cannot find open file with " );
			dbgtext( "dev = 0x%x, inode = %.0f file_id = %lu", (unsigned int)dev,
				(double)inode, file_id);
			dbgtext( "allowing break to succeed.\n" );
		}
		return NULL;
	}

	/* Ensure we have an oplock on the file */

	/*
	 * There is a potential race condition in that an oplock could
	 * have been broken due to another udp request, and yet there are
	 * still oplock break messages being sent in the udp message
	 * queue for this file. So return true if we don't have an oplock,
	 * as we may have just freed it.
	 */

	if(fsp->oplock_type == NO_OPLOCK) {
		if( DEBUGLVL( 3 ) ) {
			dbgtext( "initial_break_processing: file %s ", fsp->fsp_name );
			dbgtext( "(dev = %x, inode = %.0f, file_id = %lu) has no oplock.\n",
				(unsigned int)dev, (double)inode, fsp->fh->file_id );
			dbgtext( "Allowing break to succeed regardless.\n" );
		}
		return NULL;
	}

	return fsp;
}

static void oplock_timeout_handler(struct event_context *ctx,
				   struct timed_event *te,
				   const struct timeval *now,
				   void *private_data)
{
	files_struct *fsp = (files_struct *)private_data;

	/* Remove the timed event handler. */
	TALLOC_FREE(fsp->oplock_timeout);
	DEBUG0( ("Oplock break failed for file %s -- replying anyway\n", fsp->fsp_name));
	global_client_failed_oplock_break = True;
	remove_oplock(fsp);
	reply_to_oplock_break_requests(fsp);
}

/*******************************************************************
 Add a timeout handler waiting for the client reply.
*******************************************************************/

static void add_oplock_timeout_handler(files_struct *fsp)
{
	if (fsp->oplock_timeout != NULL) {
		DEBUG0( ("Logic problem -- have an oplock event hanging "
			  "around\n"));
	}

	fsp->oplock_timeout =
		event_add_timed(smbd_event_context(), NULL,
				timeval_current_ofs(OPLOCK_BREAK_TIMEOUT, 0),
				"oplock_timeout_handler",
				oplock_timeout_handler, fsp);

	if (fsp->oplock_timeout == NULL) {
		DEBUG0( ("Could not add oplock timeout handler\n"));
	}
}

/*******************************************************************
 This handles the case of a write triggering a break to none
 message on a level2 oplock.
 When we get this message we may be in any of three states :
 NO_OPLOCK, LEVEL_II, FAKE_LEVEL2. We only send a message to
 the client for LEVEL2.
*******************************************************************/

static void process_oplock_async_level2_break_message(int msg_type, struct process_id src,
						      void *buf, size_t len,
						      void *private_data)
{
	struct share_mode_entry msg;
	files_struct *fsp;
	char *break_msg;
	BOOL sign_state;

	if (buf == NULL) {
		DEBUG0( ("Got NULL buffer\n"));
		return;
	}

	if (len != MSG_SMB_SHARE_MODE_ENTRY_SIZE) {
		DEBUG0( ("Got invalid msg len %d\n", (int)len));
		return;
	}

	/* De-linearize incoming message. */
	message_to_share_mode_entry(&msg, (char *)buf);

	DEBUG10( ("Got oplock async level 2 break message from pid %d: 0x%x/%.0f/%lu\n",
		   (int)procid_to_pid(&src), (unsigned int)msg.dev,
		   (double)msg.inode, msg.share_file_id));

	fsp = initial_break_processing(msg.dev, msg.inode,
				       msg.share_file_id);

	if (fsp == NULL) {
		/* We hit a race here. Break messages are sent, and before we
		 * get to process this message, we have closed the file. 
		 * No need to reply as this is an async message. */
		DEBUG3( ("process_oplock_async_level2_break_message: Did not find fsp, ignoring\n"));
		return;
	}

	if (fsp->oplock_type == NO_OPLOCK) {
		/* We already got a "break to none" message and we've handled it.
		 * just ignore. */
		DEBUG3( ("process_oplock_async_level2_break_message: already broken to none, ignoring.\n"));
		return;
	}

	if (fsp->oplock_type == FAKE_LEVEL_II_OPLOCK) {
		/* Don't tell the client, just downgrade. */
		DEBUG3( ("process_oplock_async_level2_break_message: downgrading fake level 2 oplock.\n"));
		remove_oplock(fsp);
		return;
	}

	/* Ensure we're really at level2 state. */
	SMB_ASSERT(fsp->oplock_type == LEVEL_II_OPLOCK);

	/* Now send a break to none message to our client. */

	break_msg = new_break_smb_message(NULL, fsp, OPLOCKLEVEL_NONE);
	if (break_msg == NULL) {
		exit_server("Could not talloc break_msg\n");
	}

	/* Need to wait before sending a break message if we sent ourselves this message. */
	if (procid_to_pid(&src) == sys_getpid()) {
		wait_before_sending_break();
	}

	/* Save the server smb signing state. */
	sign_state = srv_oplock_set_signing(False);

	show_msg(break_msg);
	if (!send_smb(smbd_server_fd(), break_msg)) {
		exit_server_cleanly("oplock_break: send_smb failed.");
	}

	/* Restore the sign state to what it was. */
	srv_oplock_set_signing(sign_state);

	TALLOC_FREE(break_msg);

	/* Async level2 request, don't send a reply, just remove the oplock. */
	remove_oplock(fsp);
}

/*******************************************************************
 This handles the generic oplock break message from another smbd.
*******************************************************************/

static void process_oplock_break_message(int msg_type, struct process_id src,
					 void *buf, size_t len,
					 void *private_data)
{
	struct share_mode_entry msg;
	files_struct *fsp;
	char *break_msg;
	BOOL break_to_level2 = False;
	BOOL sign_state;

	if (buf == NULL) {
		DEBUG0( ("Got NULL buffer\n"));
		return;
	}

	if (len != MSG_SMB_SHARE_MODE_ENTRY_SIZE) {
		DEBUG0( ("Got invalid msg len %d\n", (int)len));
		return;
	}

	/* De-linearize incoming message. */
	message_to_share_mode_entry(&msg, (char *)buf);

	DEBUG10( ("Got oplock break message from pid %d: 0x%x/%.0f/%lu\n",
		   (int)procid_to_pid(&src), (unsigned int)msg.dev,
		   (double)msg.inode, msg.share_file_id));

	fsp = initial_break_processing(msg.dev, msg.inode,
				       msg.share_file_id);

	if (fsp == NULL) {
		/* a We hit race here. Break messages are sent, and before we
		 * get to process this message, we have closed the file. Reply
		 * with 'ok, oplock broken' */
		DEBUG3( ("Did not find fsp\n"));

		/* We just send the same message back. */
		message_send_pid(src, MSG_SMB_BREAK_RESPONSE,
				 buf, MSG_SMB_SHARE_MODE_ENTRY_SIZE, True);
		return;
	}

	if (fsp->sent_oplock_break != NO_BREAK_SENT) {
		/* Remember we have to inform the requesting PID when the
		 * client replies */
		msg.pid = src;
		ADD_TO_ARRAY(NULL, struct share_mode_entry, msg,
			     &fsp->pending_break_messages,
			     &fsp->num_pending_break_messages);
		return;
	}

	if (EXCLUSIVE_OPLOCK_TYPE(msg.op_type) &&
	    !EXCLUSIVE_OPLOCK_TYPE(fsp->oplock_type)) {
		DEBUG3( ("Already downgraded oplock on 0x%x/%.0f: %s\n",
			  (unsigned int)fsp->dev, (double)fsp->inode,
			  fsp->fsp_name));
		/* We just send the same message back. */
		message_send_pid(src, MSG_SMB_BREAK_RESPONSE,
				 buf, MSG_SMB_SHARE_MODE_ENTRY_SIZE, True);
		return;
	}

	if ((global_client_caps & CAP_LEVEL_II_OPLOCKS) && 
	    !(msg.op_type & FORCE_OPLOCK_BREAK_TO_NONE) &&
	    !koplocks && /* NOTE: we force levelII off for kernel oplocks -
			  * this will change when it is supported */
	    lp_level2_oplocks(SNUM(fsp->conn))) {
		break_to_level2 = True;
	}

	break_msg = new_break_smb_message(NULL, fsp, break_to_level2 ?
					  OPLOCKLEVEL_II : OPLOCKLEVEL_NONE);
	if (break_msg == NULL) {
		exit_server("Could not talloc break_msg\n");
	}

	/* Need to wait before sending a break message if we sent ourselves this message. */
	if (procid_to_pid(&src) == sys_getpid()) {
		wait_before_sending_break();
	}

	/* Save the server smb signing state. */
	sign_state = srv_oplock_set_signing(False);

	show_msg(break_msg);
	if (!send_smb(smbd_server_fd(), break_msg)) {
		exit_server_cleanly("oplock_break: send_smb failed.");
	}

	/* Restore the sign state to what it was. */
	srv_oplock_set_signing(sign_state);

	TALLOC_FREE(break_msg);

	fsp->sent_oplock_break = break_to_level2 ? LEVEL_II_BREAK_SENT:BREAK_TO_NONE_SENT;

	msg.pid = src;
	ADD_TO_ARRAY(NULL, struct share_mode_entry, msg,
		     &fsp->pending_break_messages,
		     &fsp->num_pending_break_messages);

	add_oplock_timeout_handler(fsp);
}

/*******************************************************************
 This handles the kernel oplock break message.
*******************************************************************/

static void process_kernel_oplock_break(int msg_type, struct process_id src,
					void *buf, size_t len,
					void *private_data)
{
	SMB_DEV_T dev;
	SMB_INO_T inode;
	unsigned long file_id;
	files_struct *fsp;
	char *break_msg;
	BOOL sign_state;

	if (buf == NULL) {
		DEBUG0( ("Got NULL buffer\n"));
		return;
	}

	if (len != MSG_SMB_KERNEL_BREAK_SIZE) {
		DEBUG0( ("Got invalid msg len %d\n", (int)len));
		return;
	}

	/* Pull the data from the message. */
	dev = DEV_T_VAL(buf, 0);
	inode = INO_T_VAL(buf, 8);
	file_id = (unsigned long)IVAL(buf, 16);

	DEBUG10( ("Got kernel oplock break message from pid %d: 0x%x/%.0f/%u\n",
		   (int)procid_to_pid(&src), (unsigned int)dev, (double)inode,
		   (unsigned int)file_id));

	fsp = initial_break_processing(dev, inode, file_id);

	if (fsp == NULL) {
		DEBUG3( ("Got a kernel oplock break message for a file "
			  "I don't know about\n"));
		return;
	}

	if (fsp->sent_oplock_break != NO_BREAK_SENT) {
		/* This is ok, kernel oplocks come in completely async */
		DEBUG3( ("Got a kernel oplock request while waiting for a "
			  "break reply\n"));
		return;
	}

	break_msg = new_break_smb_message(NULL, fsp, OPLOCKLEVEL_NONE);
	if (break_msg == NULL) {
		exit_server("Could not talloc break_msg\n");
	}

	/* Save the server smb signing state. */
	sign_state = srv_oplock_set_signing(False);

	show_msg(break_msg);
	if (!send_smb(smbd_server_fd(), break_msg)) {
		exit_server_cleanly("oplock_break: send_smb failed.");
	}

	/* Restore the sign state to what it was. */
	srv_oplock_set_signing(sign_state);

	TALLOC_FREE(break_msg);

	fsp->sent_oplock_break = BREAK_TO_NONE_SENT;

	add_oplock_timeout_handler(fsp);
}

void reply_to_oplock_break_requests(files_struct *fsp)
{
	int i;

	for (i=0; i<fsp->num_pending_break_messages; i++) {
		struct share_mode_entry *e = &fsp->pending_break_messages[i];
		char msg[MSG_SMB_SHARE_MODE_ENTRY_SIZE];

		share_mode_entry_to_message(msg, e);

		message_send_pid(e->pid, MSG_SMB_BREAK_RESPONSE,
				 msg, MSG_SMB_SHARE_MODE_ENTRY_SIZE, True);
	}

	SAFE_FREE(fsp->pending_break_messages);
	fsp->num_pending_break_messages = 0;
	if (fsp->oplock_timeout != NULL) {
		/* Remove the timed event handler. */
		TALLOC_FREE(fsp->oplock_timeout);
		fsp->oplock_timeout = NULL;
	}
	return;
}

static void process_oplock_break_response(int msg_type, struct process_id src,
					  void *buf, size_t len,
					  void *private_data)
{
	struct share_mode_entry msg;

	if (buf == NULL) {
		DEBUG0( ("Got NULL buffer\n"));
		return;
	}

	if (len != MSG_SMB_SHARE_MODE_ENTRY_SIZE) {
		DEBUG0( ("Got invalid msg len %u\n", (unsigned int)len));
		return;
	}

	/* De-linearize incoming message. */
	message_to_share_mode_entry(&msg, (char *)buf);

	DEBUG10( ("Got oplock break response from pid %d: 0x%x/%.0f/%lu mid %u\n",
		   (int)procid_to_pid(&src), (unsigned int)msg.dev,
		   (double)msg.inode, msg.share_file_id,
		   (unsigned int)msg.op_mid));

	/* Here's the hack from open.c, store the mid in the 'port' field */
	schedule_deferred_open_smb_message(msg.op_mid);
}

static void process_open_retry_message(int msg_type, struct process_id src,
				       void *buf, size_t len,
				       void *private_data)
{
	struct share_mode_entry msg;
	
	if (buf == NULL) {
		DEBUG0( ("Got NULL buffer\n"));
		return;
	}

	if (len != MSG_SMB_SHARE_MODE_ENTRY_SIZE) {
		DEBUG0( ("Got invalid msg len %d\n", (int)len));
		return;
	}

	/* De-linearize incoming message. */
	message_to_share_mode_entry(&msg, (char *)buf);

	DEBUG10( ("Got open retry msg from pid %d: 0x%x/%.0f/%lu mid %u\n",
		   (int)procid_to_pid(&src), (unsigned int)msg.dev,
		   (double)msg.inode, msg.share_file_id,
		   (unsigned int)msg.op_mid));

	schedule_deferred_open_smb_message(msg.op_mid);
}

/****************************************************************************
 This function is called on any file modification or lock request. If a file
 is level 2 oplocked then it must tell all other level 2 holders to break to
 none.
****************************************************************************/

void release_level_2_oplocks_on_change(files_struct *fsp)
{
	int i;
	struct share_mode_lock *lck;

	/*
	 * If this file is level II oplocked then we need
	 * to grab the shared memory lock and inform all
	 * other files with a level II lock that they need
	 * to flush their read caches. We keep the lock over
	 * the shared memory area whilst doing this.
	 */

	if (!LEVEL_II_OPLOCK_TYPE(fsp->oplock_type))
		return;

	lck = get_share_mode_lock(NULL, fsp->dev, fsp->inode, NULL, NULL);
	if (lck == NULL) {
		DEBUG0(("release_level_2_oplocks_on_change: failed to lock "
			 "share mode entry for file %s.\n", fsp->fsp_name ));
		return;
	}

	DEBUG10(("release_level_2_oplocks_on_change: num_share_modes = %d\n", 
		  lck->num_share_modes ));

	for(i = 0; i < lck->num_share_modes; i++) {
		struct share_mode_entry *share_entry = &lck->share_modes[i];
		char msg[MSG_SMB_SHARE_MODE_ENTRY_SIZE];

		if (!is_valid_share_mode_entry(share_entry)) {
			continue;
		}

		/*
		 * As there could have been multiple writes waiting at the
		 * lock_share_entry gate we may not be the first to
		 * enter. Hence the state of the op_types in the share mode
		 * entries may be partly NO_OPLOCK and partly LEVEL_II or FAKE_LEVEL_II
		 * oplock. It will do no harm to re-send break messages to
		 * those smbd's that are still waiting their turn to remove
		 * their LEVEL_II state, and also no harm to ignore existing
		 * NO_OPLOCK states. JRA.
		 */

		DEBUG10(("release_level_2_oplocks_on_change: "
			  "share_entry[%i]->op_type == %d\n",
			  i, share_entry->op_type ));

		if (share_entry->op_type == NO_OPLOCK) {
			continue;
		}

		/* Paranoia .... */
		if (EXCLUSIVE_OPLOCK_TYPE(share_entry->op_type)) {
			DEBUG0(("release_level_2_oplocks_on_change: PANIC. "
				 "share mode entry %d is an exlusive "
				 "oplock !\n", i ));
			TALLOC_FREE(lck);
			abort();
		}

		share_mode_entry_to_message(msg, share_entry);

		message_send_pid(share_entry->pid, MSG_SMB_ASYNC_LEVEL2_BREAK,
				 msg, MSG_SMB_SHARE_MODE_ENTRY_SIZE, True);
	}

	/* We let the message receivers handle removing the oplock state
	   in the share mode lock db. */

	TALLOC_FREE(lck);
}

/****************************************************************************
 Linearize a share mode entry struct to an internal oplock break message.
****************************************************************************/

void share_mode_entry_to_message(char *msg, struct share_mode_entry *e)
{
	SIVAL(msg,0,(uint32)e->pid.pid);
	SSVAL(msg,4,e->op_mid);
	SSVAL(msg,6,e->op_type);
	SIVAL(msg,8,e->access_mask);
	SIVAL(msg,12,e->share_access);
	SIVAL(msg,16,e->private_options);
	SIVAL(msg,20,(uint32)e->time.tv_sec);
	SIVAL(msg,24,(uint32)e->time.tv_usec);
	SDEV_T_VAL(msg,28,e->dev);
	SINO_T_VAL(msg,36,e->inode);
	SIVAL(msg,44,e->share_file_id);
	SIVAL(msg,48,e->uid);
	SSVAL(msg,52,e->flags);
}

/****************************************************************************
 De-linearize an internal oplock break message to a share mode entry struct.
****************************************************************************/

void message_to_share_mode_entry(struct share_mode_entry *e, char *msg)
{
	e->pid.pid = (pid_t)IVAL(msg,0);
	e->op_mid = SVAL(msg,4);
	e->op_type = SVAL(msg,6);
	e->access_mask = IVAL(msg,8);
	e->share_access = IVAL(msg,12);
	e->private_options = IVAL(msg,16);
	e->time.tv_sec = (time_t)IVAL(msg,20);
	e->time.tv_usec = (int)IVAL(msg,24);
	e->dev = DEV_T_VAL(msg,28);
	e->inode = INO_T_VAL(msg,36);
	e->share_file_id = (unsigned long)IVAL(msg,44);
	e->uid = (uint32)IVAL(msg,48);
	e->flags = (uint16)SVAL(msg,52);
}

/****************************************************************************
 Setup oplocks for this process.
****************************************************************************/

BOOL init_oplocks(void)
{
	DEBUG3(("init_oplocks: initializing messages.\n"));

	message_register(MSG_SMB_BREAK_REQUEST,
			 process_oplock_break_message,
			 NULL);
	message_register(MSG_SMB_ASYNC_LEVEL2_BREAK,
			 process_oplock_async_level2_break_message,
			 NULL);
	message_register(MSG_SMB_BREAK_RESPONSE,
			 process_oplock_break_response,
			 NULL);
	message_register(MSG_SMB_KERNEL_BREAK,
			 process_kernel_oplock_break,
			 NULL);
	message_register(MSG_SMB_OPEN_RETRY,
			 process_open_retry_message,
			 NULL);

	if (lp_kernel_oplocks()) {
#if HAVE_KERNEL_OPLOCKS_IRIX
		koplocks = irix_init_kernel_oplocks();
#elif HAVE_KERNEL_OPLOCKS_LINUX
		koplocks = linux_init_kernel_oplocks();
#endif
	}

	return True;
}
