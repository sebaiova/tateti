#define MSG_STATE_SIZE 6
#define MSG_BOARD_SIZE 10
#define MSG_SIZE sizeof(struct message_t)

/* MSG STATES */
#define YOU_ARE_P1 "UR P1"
#define YOU_ARE_P2 "UR P2"
#define IS_YOUR_TURN "PLAY"
#define WAIT_FOR_TURN "WAIT"
#define GAME_IS_DRAW "DRAW"
#define YOU_WON "WON"
#define YOU_LOST "LOST"

struct message_t
{
    char state[MSG_STATE_SIZE];
    char board[MSG_BOARD_SIZE];
};

void send_message(int to_sockfd, char* new_state, struct message_t* msg)
{
    memcpy(msg->state, new_state, MSG_STATE_SIZE);
    write(to_sockfd, msg, MSG_SIZE);
}

void recv_message(int from_sockfd, struct message_t* msg)
{
    read(from_sockfd, (void*)msg, MSG_SIZE);
}

int message_state_is(struct message_t* msg, char* state)
{
    return !(strcmp(msg->state, state)!=0);
}