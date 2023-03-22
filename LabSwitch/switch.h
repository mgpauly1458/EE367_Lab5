/*
 * switch.h
 */

// Forward Table Structure 
struct fwd_table{
   int valid;
   int dst;
   int port;
};

// switch.c Function Declarations 
void initialize_ftable(struct fwd_table * f);
void switch_main(int switch_id);
