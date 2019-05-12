#include "managerProcess.h"

void manager_exit(){
    exit(0);
}

void manager_process(int _id, pid_t *connections){
    // TODO
    int msqid, recebido, enviado, aux, _ndst;
    msg_packet p, *q;

    signal(SIGQUIT, manager_exit);

    msqid  = get_channel();
    recebido = -1;

    while(1){
        while(1){
            sleep(1);
            recebido = msgrcv(msqid, &p, sizeof(msg_packet), 0x2+_id, 0);
            if(recebido != -1)
                break;
        }

        if(p._mdst != _id){
            for(int i = 1; i < connections[0]+1; i++){
                if(connections[i] < p._mdst && connections[i] != -1)
                    aux = connections[i];
                if(p._mdst == connections[i]){
                    aux = connections[i];
                    break;
                }
            }

            printf("DEST: %d\nTYPE: %ld\n", p._mdst, aux);

            q = malloc(sizeof(msg_packet));
            q->type = 0x2+aux;
            strcpy(q->name, p.name);
            q->delay = p.delay;
            q->_mdst = p._mdst;
            
            enviado = msgsnd(msqid, q, sizeof(msg_packet) - sizeof(long), 0);
        } else {
            printf("Process #%d received a program to be executed!\n", _id);
        }
    }
}