// Swtich.C
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>

#include "main.h"
#include "net.h"
#include "man.h"
#include "host.h"
#include "packet.h"
#include "switch.h"

#define MAX_FILE_BUFFER 1000
#define MAX_MSG_LENGTH 100
#define MAX_DIR_NAME 100
#define MAX_FILE_NAME 100
#define PKT_PAYLOAD_MAX 100
#define TENMILLISEC 10000   /* 10 millisecond sleep */
#define MAX_FTABLE_SIZE 100

//Andee: Main
void switch_main(int switch_id){

   /* State */
   char dir[MAX_DIR_NAME];
   int dir_valid = 0;

   char man_msg[MAN_MSG_LENGTH];
   char man_reply_msg[MAN_MSG_LENGTH];
   char man_cmd;
   struct man_port_at_host *man_port;  // Port to the manager

   struct net_port *node_port_list;
   struct net_port **node_port;  // Array of pointers to node ports
   int node_port_num;            // Number of node ports

   int ping_reply_received;

   int i, k, n;
   int dst;
   char name[MAX_FILE_NAME];
   char string[PKT_PAYLOAD_MAX+1]; 

   FILE *fp;

   struct packet *in_packet; /* Incoming packet */
   struct packet *new_packet;

   struct net_port *p;
   struct host_job *new_job;
   struct host_job *new_job2;

   struct job_queue job_q;

   struct fwd_table forwardTable[MAX_FTABLE_SIZE];

   // Initialize the valid values to zero
   for (i=0; i<MAX_FTABLE_SIZE; i++) {
      forwardTable[i].valid = 0;
   }

   // Get the port the host is using to connect to the switch node (current manager port)
   man_port = net_get_host_port(switch_id);

   /*
    * Create an array node_port[ ] to store the network link ports
    * at the host.  The number of ports is node_port_num
    */
   node_port_list = net_get_port_list(switch_id);

   /*  Count the number of network link ports */
   node_port_num = 0;
   for (p=node_port_list; p!=NULL; p=p->next) {
      node_port_num++;
   }
   /* Create memory space for the array */
   node_port = (struct net_port **)
      malloc(node_port_num*sizeof(struct net_port *));

   // Now man port has the port linking the switch to the manager
   // And node_port[] has the ports linking other nodes to each other

   /* Load ports into the array */
   p = node_port_list;
   for (k = 0; k < node_port_num; k++) {
      node_port[k] = p;
      p = p->next;
   }

   /* Initialize the job queue */
   job_q_init(&job_q);

   while(1){

      // Add any new job requests to the queue
      for(k = 0; k < node_port_num; k++){ // Scan all ports 
         in_packet = (struct packet *) malloc(sizeof(struct packet));  // Allocate Space
         n = packet_recv(node_port[k], in_packet); // Receive packet from the current port

         // Check if the packet was received
         if (n > 0) {
            new_job = (struct host_job *) malloc(sizeof(struct host_job));
            new_job->in_port_index = k;
            new_job->packet = in_packet;

            job_q_add(&job_q, new_job);
         }
         else{
            free(in_packet);  // Job was not meant for a switch node
         }
      }

      // If there is still a job left in the queue execute the first one
      if(job_q_num(&job_q) > 0){
         new_job = job_q_remove(&job_q);
         // Initialize a port # to send to
         // -1 ensures no accidental sends
         int output_port = -1;
         
         // Check if any forward table rows match the new job request
         for(i = 0; i < MAX_FTABLE_SIZE; i++){
            if(forwardTable[i].valid && forwardTable[i].dst == (int) new_job->packet->dst){
               output_port = forwardTable[i].port;
               break;
            }
         }

         // If the output port was not found in the ftable, send the packet 
         if( output_port == -1 ){
            // Find a non-valid entry in the fTable
            for(int i = 0; i < MAX_FTABLE_SIZE; i++){
               // Add the src + port to the ftable slot
               if(forwardTable[i].valid == 0){
                  forwardTable[i].valid == 1;
                  forwardTable[i].dst = (int) new_job->packet->src;
                  forwardTable[i].port = new_job->in_port_index;
                  break;
               }
            }

            // Send packet to all ports
            for( k = 0; k < node_port_num; k++ ){
               // Only send to ports when its not the src
               if( k != new_job->in_port_index ){
                  packet_send(node_port[k], new_job->packet);
               }
            }

         }
         // Else if it is found in the ftable send the packet
         else{
            packet_send(node_port[output_port], new_job->packet);
         }

         free(new_job->packet);
         free(new_job);
      }

      usleep(TENMILLISEC);
   }//While Loop End
}//Main end
 
