/* 
 * host.h 
 */

enum host_job_type {
	JOB_SEND_PKT_ALL_PORTS,
	JOB_PING_SEND_REQ,	
	JOB_PING_SEND_REPLY,
	JOB_PING_WAIT_FOR_REPLY,
	JOB_FILE_UPLOAD_SEND,
	JOB_FILE_UPLOAD_RECV_START,
	JOB_FILE_UPLOAD_RECV_END,
   // Added job types to send a request for a download (send) and to recv the request and act accordingly (recv)
   JOB_FILE_DOWNLOAD_SEND,
   JOB_FILE_DOWNLOAD_RECV
};

struct host_job {
	enum host_job_type type;
	struct packet *packet;
	int in_port_index;
	int out_port_index;
	char fname_download[100];
	char fname_upload[100];
	int ping_timer;
	int file_upload_dst;
   // Variable to hold the host id of the download src
   int file_download_src;
	struct host_job *next;
};


struct job_queue {
	struct host_job *head;
	struct host_job *tail;
	int occ;
};



void host_main(int host_id);

// Function declarations for host.c 
void job_q_add(struct job_queue *j_q, struct host_job *j);
struct host_job *job_q_remove(struct job_queue *j_q);
void job_q_init(struct job_queue *j_q);
int job_q_num(struct job_queue *j_q);

