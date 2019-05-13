#include "managerProcess.h"

void manager_exit(){
    exit(0);
}

void manager_process(int _id, pid_t *connections, char *option){
    // TODO
    char *program;
    int  delay;
    int msqid, recebido, enviado, aux, _ndst;
    msg_packet p, *q;

    signal(SIGQUIT, manager_exit);

    msqid  = get_channel();
    recebido = -1;

    while(1){
        while(1){
            recebido = msgrcv(msqid, &p, sizeof(msg_packet), 0x2+_id, IPC_NOWAIT);
            if(recebido != -1)
                break;

            recebido = msgrcv(msqid, &p, sizeof(msg_packet), 19+_id, IPC_NOWAIT);
            if(recebido != -1)
                break;
            // Receber msg de volta para o escalonador!
        }

        // Checar o tipo de recepção realizado e tratar de acordo!

        if(p._mdst != _id){
            if(p.type == 0x2 + _id){
                for(int i = 1; i < connections[0]+1; i++){
                    if(connections[i] < p._mdst && connections[i] != -1)
                        aux = connections[i];
                    if(p._mdst == connections[i]){
                        aux = connections[i];
                        break;
                    }
                }

                q = malloc(sizeof(msg_packet));
                q->type = 0x2+aux;
                strcpy(q->name, p.name);
                q->delay = p.delay;
                q->_mdst = p._mdst;
                
                enviado = msgsnd(msqid, q, sizeof(msg_packet) - sizeof(long), 0);
                free(q);
            } else if(p.type == 19+_id){
                aux = 99;
                for(int i = 1; i < connections[0]+1; i++){
                    if(connections[i] < aux)
                        aux = connections[i];
                    if(connections[i] == p._mdst){
                        aux = connections[i];
                        break;
                    }
                }

                q = malloc(sizeof(msg_packet));
                q->type = 19+aux;
                q->_mdst = p._mdst;
                q->_id = p._id;
                q->ready = p.ready;

                enviado = msgsnd(msqid, q, sizeof(msg_packet)-sizeof(long), 0);
                free(q);
            }
        } else {
            program = p.name;
            delay = p.delay;

            q = malloc(sizeof(msg_packet));
            
            aux = 99;
            for(int i = 1; i < connections[0]+1; i++){
                if(connections[i] < aux)
                    aux = connections[i];
            }

            q->type  = 19+aux;
            q->_mdst = -1;
            q->_id   = _id;
            q->ready = 1;

            enviado = msgsnd(msqid, q, sizeof(msg_packet)-sizeof(long), 0);
            free(q);
        }
    }
}