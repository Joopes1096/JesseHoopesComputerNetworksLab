/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: jesse
 * Created on January 27, 2016, 3:09 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>



int count;          //count for L2PDU
char *db;           //data buffer
char *transmission; //transmission buffer
char *reception;    //reception buffer
int mss=60;         //mss constant

char cksum(char *buf, int count){
    register u_long sum =0;
    
    while(count--){
        sum+= *buf++;
        if(sum & 0xFFFF0000){
            sum &= 0xFFFF;
            sum++;
        }
    }
    
    return ~(sum & 0xFFFF);
}

void layer4(char *datab, bool trans){
    if (trans == true){
        /*Reads file and assigns contents to a data buffer*/
        FILE *f = fopen("sendfile.txt", "r");
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);
        datab = malloc(fsize + 1);
        fread(datab, fsize, 1, f);
        fclose(f);
        datab[fsize] = 0;
        db=datab;
        printf("%s\n",db);
    }
    else{
        /*Writes contents of buffer to file*/
        FILE *f = fopen("recvfile.txt", "w");
        fprintf(f,"%s",datab);
        fclose(f);
        printf("%s",datab);
    }
}

void layer3(char *buffer, bool trans){
    if (trans==true){
        count=0;
        /*Declares string of 0's to prepend L2PDU's*/
        char prep[]= "0000000000000000000000000000000000000000\0";
        /*Declares all necessary variables*/
        char *str3;
        char *temp;
        temp = malloc(mss*sizeof(char));
        str3 = malloc(sizeof(prep)*strlen(buffer)*sizeof(char));
        transmission = malloc(sizeof(prep)*strlen(buffer)*sizeof(char));
        /*Adds prep to beginning of every chunk of size "mss" if large enough*/
        int i;
        if(strlen(buffer)>mss){
            for(i=0;i<strlen(buffer);i+=mss){
                memcpy(str3, prep, sizeof(prep));
                memcpy(temp,buffer+i,mss);
                strcat(str3,temp);
                strcat(transmission,str3);
                count++;
            }
            
        }
        /*If not large enough, just passes buffer to transmission buffer
         * and sets count to 1*/
        else{
            transmission=buffer;
            count=0;
        }
        printf("%s", transmission);
    }
    else{
        /*strips 0's*/
        if(strlen(buffer)>mss){
            char* tempo;
            tempo=malloc(strlen(buffer)*strlen(buffer));
            //printf("%s", buffer);
            int j;
            for(j=0; j<count; j++){
                memcpy(tempo+j*(mss), buffer+40+j*(mss+40), mss);
            }
            memcpy(buffer, tempo, strlen(tempo)+1);
            buffer=tempo;
            
        }
        reception=buffer;
        /*does nothing because there's no prepended 0's to remove*/
        printf("%s\n",buffer);
    }
}

void layer2(char *buffer, bool trans){
    if (trans == true){
        /*If file is large enough, attaches header and tail*/
        if(strlen(buffer)>mss){
            /*Initializes necessary variables*/
            char *stx = "\2";
            char *etx = "\3";
            char *dle = "\20";
            int j;
            int i;
            
            char *head;
            char *temp;
            char *temporary;
            char *temporary2;
            char *stuftemp;
            char *stuffed;
            char *tail;//tail
            char *final;
            char *check;
            char *tempo;
            temporary2 = malloc(strlen(buffer)*strlen(buffer)*sizeof(char));
            final = malloc(strlen(buffer)*strlen(buffer)*sizeof(char));
            head = malloc(strlen(buffer)*sizeof(char));
            temp = malloc(strlen(buffer)*strlen(buffer)*sizeof(char));
            temporary = malloc(strlen(buffer)*strlen(buffer)*sizeof(char));
            stuffed = malloc(strlen(buffer)*strlen(buffer)*sizeof(char));
            tail = malloc(4*sizeof(char));
            stuftemp = malloc(strlen(buffer)*strlen(buffer)*sizeof(char));
            check = malloc(strlen(buffer)*strlen(buffer)*sizeof(char));
            tempo = malloc(strlen(buffer)*strlen(buffer)*sizeof(char));
            
            int astuf=0;
            int seq;
            for(j=0;j<count;j++){
                /*keeps track of sequence*/
                int k=j;
                seq=k%10;
                int lcount;
                /*head = "2" + count + "0" + seq + "00";*/
                /*tail = "003"*/
                memcpy(head, "\2", 2);//stx
                char s[2];
                sprintf(s,"%d",seq);
                
                /*Bit Stuffing*/
                strncat(temporary, buffer+j*(mss+40), mss+40);
                
                
                if(!strstr(stx, temporary) || !strstr(etx, temporary) || !strstr(dle, temporary)){
                    for(i=0;i<strlen(temporary);i++){
                        if(!strncmp(temporary+i, stx, 1) || !strncmp(temporary+i, etx, 1) || !strncmp(temporary+i, dle, 1)){
                            memcpy(stuffed, temporary, i);
                            memcpy(stuffed+i, dle, 2);
                            memcpy(stuffed+i+1, temporary+i, strlen(temporary)-i);
                            memcpy(stuffed+i+1+strlen(temporary), "\0", 2);
                            memcpy(temporary, stuffed, strlen(stuffed));
                            astuf++;
                            i++;

                        }
                    }
                }
                
                
                
                char c[4];
                if(j==count-1){
                    lcount = strlen(buffer)%(mss+40+20+astuf);
                    sprintf(c,"%03i",lcount);
                }
                else{
                    sprintf(c,"%03i",mss+40+astuf);
                }
                memcpy(head+1, c, strlen(c));//count
                memcpy(head+1+strlen(c), "0",2);//ack
                memcpy(head+1+strlen(c), s,strlen(s));//seq
                if(j!=count-1){
                    memcpy(head+1+strlen(c)+strlen(s),"0",2);
                }
                else{
                    memcpy(head+1+strlen(c)+strlen(s),"1",2);
                }
                
                memcpy(head+strlen(c)+strlen(s)+2,"0000000000",11);
                
                
                /*compounds string in order head + body + tail*/
                
                memcpy(temp, head, strlen(head)+1);
                
                
                memcpy(temp+strlen(head),temporary, strlen(temporary)+1);
                memcpy(tempo, buffer+j*(mss+40)+1, mss+40+16);
                
                //printf("%s", temp+1);
                
                int chksum = cksum(temp+1, strlen(temp));
                char cheksum[3];
                snprintf(cheksum,3,"%02d",chksum);
                memcpy(tail, cheksum, 3);
                memcpy(tail+2, "\3", 2);
                
                printf("%s", temp+1);
                
                memcpy(temp+strlen(temporary)+strlen(head), tail, strlen(tail)+1);
                strcat(final,temp);
                
                memcpy(temp,"",0);
                memcpy(head,"",0);
                memcpy(tail,"",0);
                memcpy(temporary,"\0",1);
                
                memcpy(temporary2, final, strlen(final)+1);
                memcpy(temporary2+strlen(final), "\0",2);
                                                                  
            }
            memcpy(buffer, final, strlen(final)+1);
            memcpy(buffer+strlen(final), "\0", 2);
            //printf("%s\n", buffer);
    }
        /*If file is not large enough*/
        else{
            /*Initializes variables*/
            char *stx = "\2";
            char *etx = "\3";
            char *dle = "\20";
            int j;
            int i;
            int astuf=0;
            char *head;
            char *temp;
            char *temporary;
            char *stuffed;
            char *tail;//tail
            char *final;
            int seq=0;
            final = malloc(strlen(buffer)*strlen(buffer)*sizeof(char));
            head = malloc(strlen(buffer)*sizeof(char));
            temp = malloc(strlen(buffer)*strlen(buffer)*sizeof(char));
            temporary = malloc(strlen(buffer)*strlen(buffer)*sizeof(char));
            stuffed = malloc(strlen(buffer)*strlen(buffer)*sizeof(char));
            tail = malloc(4*sizeof(char));
            int lcount;
            char s[2];
            sprintf(s,"%d",seq);
            /*Declares header and trailer*/
            memcpy(head, "\2",2);//stx
            char c[4];
            lcount = strlen(buffer);
            sprintf(c,"%03i",lcount);


            memcpy(head+1, c, strlen(c));//count
            memcpy(head+1+strlen(c), "0",2);//ack
            memcpy(head+1+strlen(c), s,strlen(s));//seq

            if(j!=count-1){
            memcpy(head+1+strlen(c)+strlen(s),"0",2);
            }
            else{
            memcpy(head+1+strlen(c)+strlen(s),"1",2);
            }

            memcpy(head+strlen(c)+strlen(s)+2,"0000000000",11);
            
            
            
            memcpy(temporary, buffer, strlen(buffer));
            if(!strstr(stx, temporary) || !strstr(etx, temporary) || !strstr(dle, temporary)){
                for(i=0;i<strlen(temporary);i++){
                    if(!strncmp(temporary+i, stx, 1) || !strncmp(temporary+i, etx, 1) || !strncmp(temporary+i, dle, 1)){
                        memcpy(stuffed, temporary, i);
                        memcpy(stuffed+i, dle, 2);
                        memcpy(stuffed+i+1, temporary+i, strlen(temporary)-i);
                        memcpy(stuffed+i+1+strlen(temporary), "\0", 2);
                        memcpy(temporary, stuffed, strlen(stuffed));

                        i++;

                    }
                }
            }
            
            
            
            /*Compounds string*/
            memcpy(temp,head,strlen(head));
            memcpy(temp+strlen(head),temporary,strlen(temporary));
            
            int chksum = cksum(temp+1, strlen(temp));
            char cheksum[3];
            sprintf(cheksum,"%02d",chksum);
            memcpy(tail,cheksum,3);
            memcpy(tail+2, "\3", 2);
            
            memcpy(temp+strlen(head)+strlen(temporary),tail,strlen(tail));
            buffer=temp;
        }
        transmission=buffer;
        printf("%s",transmission);
    }
    /*if receiving*/
    else{
        /*if file is large enough*/
        if(strlen(buffer)>mss){
            /*Initialize variables*/
            int k=0;
            int i=0;
            int j=0;
            int astuf=0;
            int stuf=0;
            bool check;
            char *final;
            char *tempor;
            char* temp;
            char* temp1;
            char* temp2;
            char* temp3;
            char* temporary;
            char* temporary2;
            char* tempo;
            char* tempo1;
            char* ckcheck;
            char* ckcheck1;
            char* chksm;
            temp = malloc((strlen(buffer)*strlen(buffer)+20*count)*sizeof(char));
            temp1 = malloc((strlen(buffer)*strlen(buffer)+20*count)*sizeof(char));
            temp2 = malloc((strlen(buffer)*strlen(buffer)+20*count)*sizeof(char));
            temp3 = malloc((strlen(buffer)*strlen(buffer)+20*count)*sizeof(char));
            temporary = malloc(strlen(buffer)*strlen(buffer)*sizeof(char));
            temporary2 = malloc(strlen(buffer)*strlen(buffer)*sizeof(char));
            final = malloc(strlen(buffer)*strlen(buffer)*sizeof(char));
            tempo = malloc(strlen(buffer)*strlen(buffer)*sizeof(char));
            ckcheck = malloc(strlen(buffer)*strlen(buffer)*sizeof(char));
            ckcheck1 = malloc(strlen(buffer)*strlen(buffer)*sizeof(char));
            tempo1 = malloc(strlen(buffer)*strlen(buffer)*sizeof(char));
            tempor = malloc(strlen(buffer)*strlen(buffer)*sizeof(char));
            chksm = malloc(strlen(buffer)*strlen(buffer)*sizeof(char));
            
            
            //printf("%s\n\n\n", buffer);
            /*Checksumming*/
            for(k=0; k<count; k++){
                if(!strstr("\20\3", buffer+1+k*(mss+40+16+3)) || !strstr("\20\2", buffer+1+k*(mss+40+16+3)) || !strstr("\20\20", buffer+1+k*(mss+40+16+3))){
                    int index = strchr(buffer+k, '\20') - buffer;
                    stuf++;
                    k=index;
                }
                
                memcpy(tempor, buffer+1, mss+40+17+stuf-2);
                int chksum = cksum(tempor, strlen(tempor));
                char cheksum[3];
                sprintf(cheksum,"%02d",chksum);
                memcpy(chksm, buffer+mss+40+17+stuf-1, 2);
                
                if(!strcmp(cheksum, chksm)){
                    printf("\nTHE CHECKSUM IS CORRECT\n");
                }
            }
            
            
            
            
            
            for(j=0; j<strlen(buffer); j++){
                if(!strstr("\20\3", buffer+j) || !strstr("\20\2", buffer+j) || !strstr("\20\20", buffer+j)){
                    int index = strchr(buffer+j, '\20') - buffer;
                    astuf++;
                    j=index;
                }
            }
            int m=-1;
            for(i=0;i<strlen(buffer); i++){
                if(!strstr("\20\3", buffer+i) || !strstr("\20\2", buffer+i) || !strstr("\20\20", buffer+i)){
                    m++;
                    int index = strchr(buffer+i, '\20') - buffer;
                    
                    if(index<strlen(buffer)|| index>0){
                        memcpy(temporary, buffer, index);
                        memcpy(tempo, buffer+index+1, strlen(buffer)-index);
                        
                        memcpy(tempo1, temporary, strlen(temporary)+1);
                        memcpy(tempo1+strlen(temporary), tempo, strlen(tempo)+1);
                        
                        memcpy(buffer, tempo1, strlen(tempo1)+1);
                        memcpy(temporary, "\0", 1);
                        memcpy(tempo,"\0", 1);
                        memcpy(tempo1, "\0",1);
                        i=index;
                        
                    }
                }
                memcpy(final, buffer, strlen(buffer)+1);
                
            }
            memcpy(buffer, final, strlen(final)+1);
            
            
            /*strip header and tail*/
            for(k=0; k<count; k++){
                memcpy(temp, buffer+16+k*(mss+40+3+16), mss+40+3);
                memcpy(temp1, temp, mss+40);
                strcat(temp2, temp1);
                memcpy(temp, "\0", 1);  
            }
            memcpy(temp3, temp2, strlen(temp2)-3);
            memcpy(buffer, temp3, strlen(temp3)+1);
            
        }
        /*if file is not large enough*/
        else{
            /*Initialize variables*/
            int i=0;
            int j=0;
            int astuf=0;
            char* temp;
            char* temp1;
            char* temporary;
            char* tempo;
            char* tempo1;
            char* ckcheck;
            char* ckcheck1;
            temp = malloc((strlen(buffer)*strlen(buffer) + 20*count)*sizeof(char));
            temp1 = malloc((strlen(buffer)*strlen(buffer) + 20*count)*sizeof(char));
            temporary = malloc((strlen(buffer)*strlen(buffer) + 20*count)*sizeof(char));
            ckcheck = malloc((strlen(buffer)*strlen(buffer) + 20*count)*sizeof(char));
            ckcheck1 = malloc((strlen(buffer)*strlen(buffer) + 20*count)*sizeof(char));
            tempo = malloc((strlen(buffer)*strlen(buffer) + 20*count)*sizeof(char));
            tempo1 = malloc((strlen(buffer)*strlen(buffer) + 20*count)*sizeof(char));
            
            /*strips header and tail*/
            memcpy(temp,buffer,strlen(buffer)-3);
            int chksum = cksum(temp+1, strlen(temp)+1);
            char cheksum[3];
            sprintf(cheksum,"%02d",chksum);
            memcpy(ckcheck, cheksum, 2);
            
            memcpy(ckcheck1, buffer+strlen(buffer)-3, 2);
            
            if(!strcmp(ckcheck1,ckcheck)){
                printf("THE CHECKSUM IS CORRECT!\n");
            }
            
            memcpy(temp1,temp+16,strlen(temp));
            memcpy(buffer, temp1, strlen(temp1)+1);
            for(j=0; j<strlen(buffer); j++){
                if(!strstr("\20\3", buffer+j) || !strstr("\20\2", buffer+j) || !strstr("\20\20", buffer)){
                    astuf++;
                }
            }
            //printf("%d", astuf);
            for(i=0;i<astuf+1; i++){
                if(!strstr("\20\3", buffer+i) || !strstr("\20\2", buffer+i) || !strstr("\20\20", buffer+i)){
                    
                    int index = strchr(buffer+i, '\20') - buffer;
                    
                    if(index<strlen(buffer-i)|| index>0){
                        memcpy(temporary, buffer, index);
                        memcpy(temporary+index, buffer+index+1, strlen(buffer)-index);
                        memcpy(tempo, temporary, strlen(temporary));
                        memcpy(tempo1, tempo, strlen(tempo));
                        memcpy(buffer, tempo1, strlen(tempo1));/**/
                    }
                    
                }
            }
        }
        memcpy(reception, buffer, strlen(buffer));
        printf("%s\n",reception);
    }
}

void layer1(char *buffer, bool trans){
    if (trans == true){
        /*dummy layer*/
        printf("%s\n",buffer);
    }
    else{
        /*initializes and assigns transmission buffer to reception buffer*/
        reception = malloc(strlen(buffer)*strlen(buffer)*sizeof(char));
        memcpy(reception, transmission, strlen(transmission));
        printf("%s\n",reception);
    }
}

void transmit(){
    bool trans = true;
    printf("\n\nExpecting layer 4\n\n");
    layer4(db, trans);
    printf("\n\nExpecting layer 3\n\n");
    layer3(db, trans);
    printf("\n\nExpecting layer 2\n\n");
    layer2(transmission, trans);
    printf("\n\nExpecting layer 1\n\n");
    layer1(transmission, trans);/**/
}

void receive(){
    bool trans=false;
    printf("\n\nExpecting layer 1\n\n");
    layer1(transmission, trans);
    printf("\n\nExpecting layer 2\n\n");
    layer2(reception, trans);
    printf("\n\nExpecting layer 3\n\n");
    layer3(reception, trans);
    printf("\n\nExpecting layer 4\n\n");
    layer4(reception, trans);
}

int main(int argc, char** argv) {
    transmit();
    receive();
    return (EXIT_SUCCESS);
}