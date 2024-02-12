#ifndef SCHC_Macros_hpp
#define SCHC_Macros_hpp

/* DEBUG Level */
// comment the line of the log level you want to activate
#define MYDEBUG
#define MYINFO
#define SERIAL_CONNECTION


#define _SESSION_POOL_SIZE 10   // Sessions numbers

/* Fragmentation traffic direction */
#define SCHC_FRAG_DIRECTION_UPLINK 1
#define SCHC_FRAG_DIRECTION_DOWNLINK 2

/* L2 Protocols */
#define SCHC_FRAG_PROTOCOL_LORAWAN 1
#define SCHC_FRAG_PROTOCOL_NBIOT 2
#define SCHC_FRAG_PROTOCOL_SIGFOX 3

/* Machine States */
#define STATE_TX_INIT 0
#define STATE_TX_SEND 1
#define STATE_TX_WAIT_x_ACK 2
#define STATE_TX_RESEND_MISSING_FRAG 3
#define STATE_TX_ERROR 4
#define STATE_TX_END 5

/* SCHC Messages */
#define SCHC_REGULAR_FRAGMENT_MSG 0
#define SCHC_ALL1_FRAGMENT_MSG 1
#define SCHC_ACK_MSG 2
#define SCHC_ACK_REQ_MSG 3
#define SCHC_SENDER_ABORT_MSG 4
#define SCHC RECEIVER_ABORT_MSG 5 

/* ACK Modes */
#define ACK_MODE_ACK_END_WIN 1
#define ACK_MODE_ACK_END_SES 2
#define ACK_MODE_COMPOUND_ACK 3

#endif