#pragma warning(disable:4996)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional or bidirectional
   data transfer protocols (from A to B. Bidirectional transfer of data
   is for extra credit and is not required).  Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

#define BIDIRECTIONAL 1    /* change to 1 if you're doing extra credit */
/* and write a routine called B_output */

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct msg {
    char data[20];
};

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct pkt {
    int seqnum;
    int acknum;
    int checksum;
    char payload[20];
};

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
#define WINDOW_SIZE 8

struct send {

    int send_base;              // sender의 base 번호
    int next_seqnum;            // 다음에 전송될 패킷의 시퀀스 번호
    int expected_seqnum;        // 예상되는 seq 번호
    int send_ack;               // 패킷에 보낼 ACK
    int receive_ack;            // 패킷에서 받은 ACK
    int ack_check;              // ack 같이 전달하는 지 check

    struct pkt snd[100];//보내는 패킷
    struct pkt rcv[100];//받는 패킷    
};

struct send sender_A;  // sender_A
struct send sender_B;  // sender_B

int TRACE = 1;             /* for my debugging */

int Checksum(struct pkt packet)//16비트 checksum구하기 
{
    int checksum = 0;

    checksum += packet.seqnum;
    checksum += packet.acknum;

    for (int i = 0; i < 20; i++)
    {
        checksum += packet.payload[i];
    }
    while (checksum >> 16)
    {
        checksum = (checksum & 0xFFFF) + (checksum >> 16);
    }

    return ~checksum;
}


/* called from layer 5, passed the data to be sent to other side */
A_output(message)
struct msg message;
{
    struct pkt packet;
    if (sender_A.next_seqnum < sender_A.send_base + WINDOW_SIZE)
    {
        // 패킷 저장
        packet.seqnum = sender_A.next_seqnum;
        strncpy(packet.payload, message.data, sizeof(packet.payload) - 1);
        packet.payload[sizeof(packet.payload) - 1] = '\0';

        if (sender_A.ack_check != 0)//ACK가 존재할 때
        {
            packet.acknum = packet.seqnum;
            packet.checksum = Checksum(packet);// checksum계산하기
            tolayer3(0, packet);//layer3으로 보내기
            starttimer(0, 30.0, packet.seqnum);//타이머시작

            printf("A_output : Send packet with ACK (seq: %d ack: %d)\n", packet.seqnum, packet.acknum);
            printf("A_output : %s\n", packet.payload);
        }
        else if (sender_A.ack_check == 0)//data만 존재할때
        {
            packet.acknum = 999;//acknum=999
            packet.checksum = Checksum(packet);// checksum계산하기
            tolayer3(0, packet);//tolayer3으로 보내기
            starttimer(0, 30.0, packet.seqnum);//타이머시작

            printf("A_output : Send packet without ACK (seq = %d)\n", packet.seqnum);
            printf("A_output : %s\n", packet.payload);
        }
        printf("seqnum:%d\n", packet.seqnum);
        printf("acknum:%d\n", packet.acknum);

        sender_A.snd[packet.seqnum] = packet;
        sender_A.next_seqnum++;// Update next_seqnum
    }
}

B_output(message)  /* need be completed only for extra credit */
struct msg message;
{
    struct pkt packet;
    if (sender_B.next_seqnum < sender_B.send_base + WINDOW_SIZE)
    {
        // 패킷 저장
        packet.seqnum = sender_B.next_seqnum;
        strncpy(packet.payload, message.data, sizeof(packet.payload) - 1);
        packet.payload[sizeof(packet.payload) - 1] = '\0';
        
        if (sender_B.ack_check != 0)//ACK가 존재할 때
        {
            packet.acknum = packet.seqnum;
            packet.checksum = Checksum(packet);// checksum계산하기
            tolayer3(1, packet);//tolayer3으로 보내기
            starttimer(1, 30.0, packet.seqnum);//타이머시작

            printf("B_output : Send packet with ACK (seq: %d ack: %d)\n", packet.seqnum, packet.acknum);
            printf("B_output : %s\n", packet.payload);
        }
        else if (sender_B.ack_check == 0)//data만 존재할때
        {
            packet.acknum = 999;//acknum=999
            packet.checksum = Checksum(packet);// checksum계산하기
            tolayer3(1, packet);//tolayer3으로 보내기
            starttimer(1, 30.0, packet.seqnum);//타이머시작

            printf("B_output : Send packet without ACK (seq = %d)\n", packet.seqnum);
            printf("B_output : %s\n", packet.payload);
        }
        printf("seqnum:%d\n", packet.seqnum);
        printf("acknum:%d\n", packet.acknum);

        sender_B.snd[packet.seqnum] = packet;
        sender_B.next_seqnum++;// Update next_seqnum
    }
}

/* called from layer 3, when a packet arrives for layer 4 */
A_input(packet)
struct pkt packet;
{
    printf("seqnum:%d\n", packet.seqnum);
    printf("acknum;%d\n", packet.acknum);

    //when not corrupt
    if (Checksum(packet) == packet.checksum)
    {
        sender_A.rcv[packet.seqnum] = packet;
        sender_A.receive_ack=packet.seqnum;
        if (sender_A.rcv[packet.seqnum].seqnum == sender_A.expected_seqnum)//예상하는 seqnum이 들어올때
        {
            sender_A.ack_check = 1;

            if (packet.acknum != 999)//ack와 data있을 때
            {
                while (sender_A.rcv[sender_A.send_base].seqnum != 0)     //sender_A.rcv[base] != NULL
                {
                    tolayer5(0, sender_A.rcv[sender_A.send_base].payload); 
                    sender_A.send_base++;
                }
                stoptimer(0, packet.acknum);//packet.acknum
                
                printf("A_input : stop timer.\n");
                printf("A_input : Got ACK (ack = %d)\n",packet.acknum );//sender_A.rcv[sender_A.send_base].seqnum
            }
            else if (packet.acknum == 999)//ack 없을 때(맨 처음에 보낼 때)
            {
                tolayer5(0, sender_A.rcv[packet.seqnum].payload);
                printf("A_input : Receive_packet %s (seq: %d ack: %d)\n", packet.payload, packet.seqnum, packet.acknum);
            }

        }

        else if (sender_A.rcv[packet.seqnum].seqnum != sender_A.expected_seqnum)//예상했던 seqnum이 아닐 때
        {
            sender_A.ack_check = 1;
            sender_A.rcv[packet.seqnum].seqnum= 1;//들어온 값 저장해두고

            printf("A_input : Receive_packet %s (seq: %d ack: %d)\n", packet.payload, packet.seqnum, packet.acknum);
        }
        
    }
    else //when corrupt
    {
        sender_A.ack_check = 0;
        printf("A_input : Fail to receive_packet %s\n", packet.payload);    
    }
    sender_A.expected_seqnum = sender_A.send_base;
}

/* called when A's timer goes off */
  /* This is called when the timer goes off
   * I loop from the base till the nextseq number and decrement 1 each time . If the value is 0
   * I will retransmit the packet.
   * If the value is 0 previously I just skip beacuse the ack for the pkt is received
   */
A_timerinterrupt(int seqnum)
{
    struct pkt packet = sender_A.snd[seqnum];

    printf("A_timerinterrupt : Send_packet with ACK (ACK = %d , seq = %d)\n", packet.acknum, packet.seqnum);
    printf("A_timerinterrupt : %s\n", packet.payload);

    //패킷 재전송
    tolayer3(0, packet);//layor3으로 보내기
    starttimer(0, 30.0, packet.seqnum);//타이머시작  
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
A_init()//초기화 부분
{
    sender_A.send_base = 1;
    sender_A.next_seqnum = 1;
    sender_A.expected_seqnum = 1;
    sender_A.receive_ack = 0;
    sender_A.send_ack = 0;
    sender_A.ack_check = 0;

    for (int i = 0; i < 100; i++)
    {
        sender_A.snd->acknum = 0;
        sender_A.snd->seqnum = 0;
        sender_A.snd->checksum = 0;
        for (int j = 0; j < 20; j++)
        {
            sender_A.snd->payload[j] = 0;
        }
    }

    for (int i = 0; i < 100; i++)
    {
        sender_A.rcv->acknum = 0;
        sender_A.rcv->seqnum = 0;
        sender_A.rcv->checksum = 0;
        for (int j = 0; j < 20; j++)
        {
            sender_A.rcv->payload[j] = 0;
        }
    }
}


/* Note that with simplex transfer from a-to-B, there is no B_output() */


/* called from layer 3, when a packet arrives for layer 4 at B*/
B_input(packet)
struct pkt packet;
{
    printf("seqnum:%d\n", packet.seqnum);
    printf("acknum:%d\n", packet.acknum);
    
    //when not corrupt
    if (Checksum(packet) == packet.checksum)
    {
        sender_A.rcv[packet.seqnum] = packet;

        if (sender_B.rcv[packet.seqnum].seqnum == sender_B.expected_seqnum)//예상하는 seqnum이 들어올때
        {
            sender_B.ack_check = 1;

            if (packet.acknum != 999)//ack와 data있을 때
            {
                while (sender_B.rcv[sender_B.send_base].seqnum != 0)     //sender_A.rcv[base] != NULL
                {
                    tolayer5(1, sender_B.rcv[sender_B.send_base].payload);//layor5로 보내기
                    sender_B.send_base++;
                }
                stoptimer(1, packet.acknum);
                
                printf("B_input : stop timer.\n");
                printf("B_input : Got ACK (ack = %d)\n", packet.acknum);//sender_B.rcv[sender_B.send_base].seqnum
            }
            else if (packet.acknum == 999)//ack 없을 때(맨 처음에 보낼 때)
            {
                tolayer5(1, sender_B.rcv[packet.seqnum].payload);//layor5로 보내기
                stoptimer(1, packet.acknum);
                printf("B_input : Receive_packet %s (seq: %d ack: %d)\n", packet.payload, packet.seqnum, packet.acknum);
            }

        }

        else if (sender_B.rcv[packet.seqnum].seqnum != sender_B.expected_seqnum)//예상했던 seqnum이 아닐 때
        {
            sender_B.ack_check = 1;
            sender_B.rcv[packet.seqnum].seqnum = 1;//들어온 값 저장해두고
            
            printf("B_input : Receive_packet %s (seq: %d ack: %d)\n", packet.payload, packet.seqnum, packet.acknum);
        }
    }
    else //when corrupt
    {
        sender_B.ack_check = 0;
        printf("B_input : Fail to receive_packet %s\n", packet.payload);
    }
    sender_B.expected_seqnum = sender_B.send_base;
}

/* called when B's timer goes off */
B_timerinterrupt(int seqnum)
{
    struct pkt packet = sender_B.snd[seqnum];
    printf("B_timerinterrupt : Send_packet with ACK (ACK = %d , seq = %d)\n", packet.acknum, packet.seqnum);
    printf("B_timerinterrupt : %s\n", packet.payload);

    //패킷 재전송
    tolayer3(1, packet);//layor3으로 보내기
    starttimer(1, 30.0, seqnum);//타이머시작
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
B_init()//초기화 부분
{
    sender_B.send_base = 1;
    sender_B.next_seqnum = 1;
    sender_B.expected_seqnum = 1;
    sender_B.receive_ack = 0;
    sender_B.send_ack = 0;
    sender_B.ack_check = 0;

    for (int i = 0; i < 100; i++)
    {
        sender_B.snd->acknum = 0;
        sender_B.snd->seqnum = 0;
        sender_B.snd->checksum = 0;
        for (int j = 0; j < 20; j++)
        {
            sender_B.snd->payload[j] = 0;
        }
    }

    for (int i = 0; i < 100; i++)
    {
        sender_B.rcv->acknum = 0;
        sender_B.rcv->seqnum = 0;
        sender_B.rcv->checksum = 0;
        for (int j = 0; j < 20; j++)
        {
            sender_B.rcv->payload[j] = 0;
        }
    }

}


/*****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 3 and below network environment:
  - emulates the tranmission and delivery (possibly with bit-level corruption
    and packet loss) of packets across the layer 3/4 interface
  - handles the starting/stopping of a timer, and generates timer
    interrupts (resulting in calling students timer handler).
  - generates message to be sent (passed from later 5 to 4)

THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW.  YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
the emulator, you're welcome to look at the code - but again, you should have
to, and you defeinitely should not have to modify
******************************************************************/

struct event {
    float evtime;           /* event time */
    int evtype;             /* event type code */
    int eventity;           /* entity where event occurs */
    struct pkt packet;      /* packet (if any) assoc w/ this event */
    struct event* prev;
    struct event* next;
};
struct event* evlist = NULL;   /* the event list */

/* possible events: */
#define  TIMER_INTERRUPT 0  
#define  FROM_LAYER5     1
#define  FROM_LAYER3     2

#define  OFF             0
#define  ON              1
#define  A               0
#define  B               1

//int TRACE = 1;             /* for my debugging */
int nsim = 0;              /* number of messages from 5 to 4 so far */
int nsimmax = 0;           /* number of msgs to generate, then stop */
float time = 0.000;
float lossprob;            /* probability that a packet is dropped  */
float corruptprob;         /* probability that one bit is packet is flipped */
float lambda;              /* arrival rate of messages from layer 5 */
int   ntolayer3;           /* number sent into layer 3 */
int   nlost;               /* number lost in media */
int ncorrupt;              /* number corrupted by media*/

main()
{
    struct event* eventptr;
    struct msg  msg2give;
    struct pkt  pkt2give;

    int i, j;
    char c;

    init();
    A_init();
    B_init();

    while (1) {
        eventptr = evlist;            /* get next event to simulate */
        if (eventptr == NULL)
            goto terminate;
        evlist = evlist->next;        /* remove this event from event list */
        if (evlist != NULL)
            evlist->prev = NULL;
        if (TRACE >= 2) {
            printf("\nEVENT time: %f,", eventptr->evtime);
            printf("  type: %d", eventptr->evtype);
            if (eventptr->evtype == 0)
                printf(", timerinterrupt  ");
            else if (eventptr->evtype == 1)
                printf(", fromlayer5 ");
            else
                printf(", fromlayer3 ");
            printf(" entity: %d\n", eventptr->eventity);
        }
        time = eventptr->evtime;        /* update time to next event time */
        if (nsim == nsimmax)
            break;                        /* all done with simulation */
        if (eventptr->evtype == FROM_LAYER5) {
            generate_next_arrival();   /* set up future arrival */
            /* fill in msg to give with string of same letter */
            j = nsim % 26;
            for (i = 0; i < 20; i++)
                msg2give.data[i] = 97 + j;
            if (TRACE > 2) {
                printf("          MAINLOOP: data given to student: ");
                for (i = 0; i < 20; i++)
                    printf("%c", msg2give.data[i]);
                printf("\n");
            }
            nsim++;
            if (eventptr->eventity == A)
                A_output(msg2give);
            else
                B_output(msg2give);
        }
        else if (eventptr->evtype == FROM_LAYER3) {
            pkt2give.seqnum = eventptr->packet.seqnum;
            pkt2give.acknum = eventptr->packet.acknum;
            pkt2give.checksum = eventptr->packet.checksum;
            for (i = 0; i < 20; i++)
                pkt2give.payload[i] = eventptr->packet.payload[i];
            if (eventptr->eventity == A)      /* deliver packet by calling */
                A_input(pkt2give);            /* appropriate entity */
            else
                B_input(pkt2give);
        }
        else if (eventptr->evtype == TIMER_INTERRUPT) {
            if (eventptr->eventity == A)
                A_timerinterrupt(eventptr->packet.seqnum);
            else
                B_timerinterrupt(eventptr->packet.seqnum);
        }
        else {
            printf("INTERNAL PANIC: unknown event type \n");
        }
        free(eventptr);
    }

terminate:
    printf(" Simulator terminated at time %f\n after sending %d msgs from layer5\n", time, nsim);
}



init()                         /* initialize the simulator */
{
    int i;
    float sum, avg;
    float jimsrand();


    printf("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n");
    printf("Enter the number of messages to simulate: ");
    scanf("%d", &nsimmax);
    printf("Enter packet loss probability [enter 0.0 for no loss]:");
    scanf("%f", &lossprob);
    printf("Enter packet corruption probability [0.0 for no corruption]:");
    scanf("%f", &corruptprob);
    printf("Enter average time between messages from sender's layer5 [ > 0.0]:");
    scanf("%f", &lambda);
    printf("Enter TRACE:");
    scanf("%d", &TRACE);

    srand(9999);              /* init random number generator */
    sum = 0.0;                /* test random number generator for students */
    for (i = 0; i < 1000; i++)
        sum = sum + jimsrand();    /* jimsrand() should be uniform in [0,1] */
    avg = sum / 1000.0;
    if (avg < 0.25 || avg > 0.75) {
        printf("It is likely that random number generation on your machine\n");
        printf("is different from what this emulator expects.  Please take\n");
        printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
        exit(0);
    }

    ntolayer3 = 0;
    nlost = 0;
    ncorrupt = 0;

    time = 0.0;                    /* initialize time to 0.0 */
    generate_next_arrival();     /* initialize event list */
}

/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/
float jimsrand()
{
    //double mmm = 2147483647;   /* largest int  - MACHINE DEPENDENT!!!!!!!!   */
    double mmm = 32767;   /* largest int  - MACHINE DEPENDENT!!!!!!!!   */
    float x;                   /* individual students may need to change mmm */
    x = rand() / mmm;            /* x should be uniform in [0,1] */
    return(x);
}

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/

generate_next_arrival()
{
    double x, log(), ceil();
    struct event* evptr;
    char* malloc();
    float ttime;
    int tempint;

    if (TRACE > 2)
        printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");

    x = lambda * jimsrand() * 2;  /* x is uniform on [0,2*lambda] */
    /* having mean of lambda        */
    evptr = (struct event*)malloc(sizeof(struct event));
    evptr->evtime = time + x;
    evptr->evtype = FROM_LAYER5;
    if (BIDIRECTIONAL && (jimsrand() > 0.5))
        evptr->eventity = B;
    else
        evptr->eventity = A;
    insertevent(evptr);
}


insertevent(p)
struct event* p;
{
    struct event* q, * qold;

    if (TRACE > 2) {
        printf("            INSERTEVENT: time is %lf\n", time);
        printf("            INSERTEVENT: future time will be %lf\n", p->evtime);
    }
    q = evlist;     /* q points to header of list in which p struct inserted */
    if (q == NULL) {   /* list is empty */
        evlist = p;
        p->next = NULL;
        p->prev = NULL;
    }
    else {
        for (qold = q; q != NULL && p->evtime > q->evtime; q = q->next)
            qold = q;
        if (q == NULL) {   /* end of list */
            qold->next = p;
            p->prev = qold;
            p->next = NULL;
        }
        else if (q == evlist) { /* front of list */
            p->next = evlist;
            p->prev = NULL;
            p->next->prev = p;
            evlist = p;
        }
        else {     /* middle of list */
            p->next = q;
            p->prev = q->prev;
            q->prev->next = p;
            q->prev = p;
        }
    }
}

printevlist()
{
    struct event* q;
    int i;
    printf("--------------\nEvent List Follows:\n");
    for (q = evlist; q != NULL; q = q->next) {
        printf("Event time: %f, type: %d entity: %d\n", q->evtime, q->evtype, q->eventity);
    }
    printf("--------------\n");
}



/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
stoptimer(AorB, seqnum)
int AorB;  /* A or B is trying to stop timer */
int seqnum; /* sequence number of the packet whose timer needs to be stopped */
{
    struct event* q;

    if (TRACE > 2)
        printf("          STOP TIMER: stopping timer at %f for sequence number %d\n", time, seqnum);

    /* Search for the TIMER_INTERRUPT event with the given sequence number and AorB */
    for (q = evlist; q != NULL; q = q->next) {
        if ((q->evtype == TIMER_INTERRUPT) && (q->eventity == AorB) && (q->packet.seqnum == seqnum)) {
            /* remove this event */
            if (q->next == NULL && q->prev == NULL)
                evlist = NULL;         /* remove first and only event on list */
            else if (q->next == NULL) /* end of list - there is one in front */
                q->prev->next = NULL;
            else if (q == evlist) {   /* front of list - there must be event after */
                q->next->prev = NULL;
                evlist = q->next;
            }
            else {      /* middle of list */
                q->next->prev = q->prev;
                q->prev->next = q->next;
            }
            free(q);
            return;
        }
    }
    printf("Warning: unable to cancel your timer. It wasn't running.\n");
}



starttimer(AorB, increment, seqnum)
int AorB;  /* A or B is trying to start timer */
float increment;
int seqnum; /* sequence number of the packet whose timer needs to be started */
{
    struct event* q;
    struct event* evptr;
    char* malloc();

    if (TRACE > 2)
        printf("          START TIMER: starting timer at %f for sequence number %d\n", time, seqnum);

    /* Check if timer is already started for this sequence number */
    for (q = evlist; q != NULL; q = q->next) {
        if ((q->evtype == TIMER_INTERRUPT) && (q->eventity == AorB) && (q->packet.seqnum == seqnum)) {
            printf("Warning: attempt to start a timer that is already started\n");
            return;
        }
    }

    /* create future event for when timer goes off */
    evptr = (struct event*)malloc(sizeof(struct event));
    evptr->evtime = time + increment;
    evptr->evtype = TIMER_INTERRUPT;
    evptr->eventity = AorB;
    evptr->packet.seqnum = seqnum;
    insertevent(evptr);
}


/************************** TOLAYER3 ***************/
tolayer3(AorB, packet)
int AorB;  /* A or B is trying to stop timer */
struct pkt packet;
{
    struct pkt mypktptr;
    struct event* evptr, * q;
    char* malloc();
    float lastime, x, jimsrand();
    int i;


    ntolayer3++;

    /* simulate losses: */
    if (jimsrand() < lossprob) {
        nlost++;
        if (TRACE > 0)
            printf("          TOLAYER3: packet%d being lost\n", packet.seqnum);
        return;
    }

    /* make a copy of the packet student just gave me since he/she may decide */
    /* to do something with the packet after we return back to him/her */
    mypktptr.seqnum = packet.seqnum;
    mypktptr.acknum = packet.acknum;
    mypktptr.checksum = packet.checksum;
    for (i = 0; i < 20; i++)
        mypktptr.payload[i] = packet.payload[i];
    if (TRACE > 2) {
        printf("          TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr.seqnum,
            mypktptr.acknum, mypktptr.checksum);
        for (i = 0; i < 20; i++)
            printf("%c", mypktptr.payload[i]);
        printf("\n");
    }

    /* create future event for arrival of packet at the other side */
    evptr = (struct event*)malloc(sizeof(struct event));
    evptr->evtype = FROM_LAYER3;   /* packet will pop out from layer3 */
    evptr->eventity = (AorB + 1) % 2; /* event occurs at other entity */
    /* finally, compute the arrival time of packet at the other end.
       medium can not reorder, so make sure packet arrives between 1 and 10
       time units after the latest arrival time of packets
       currently in the medium on their way to the destination */
    lastime = time;
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == FROM_LAYER3 && q->eventity == evptr->eventity))
            lastime = q->evtime;
    evptr->evtime = lastime + 1 + 9 * jimsrand();

    /* simulate corruption: */
    if (jimsrand() < corruptprob) {
        ncorrupt++;
        if ((x = jimsrand()) < .75)
            mypktptr.payload[0] = 'Z';   /* corrupt payload */
        else if (x < .875)
            mypktptr.seqnum = 999999;
        else
            mypktptr.acknum = 999999;
        if (TRACE > 0)
            printf("          TOLAYER3: packet%d being corrupted\n", packet.seqnum);
    }
    evptr->packet = mypktptr;       /* save ptr to my copy of packet */
    if (TRACE > 2)
        printf("          TOLAYER3: scheduling arrival on other side\n");
    insertevent(evptr);
}

tolayer5(AorB, datasent)
int AorB;
char datasent[20];
{
    int i;
    if (TRACE > 2) {
        printf("          TOLAYER5: data received: ");
        for (i = 0; i < 20; i++)
            printf("%c", datasent[i]);
        printf("\n");
    }

}
