/*
 * Msg.c
 *
 *  Created on: 2010. 09. XX
 *      Author: Che
 */

#include <mqueue.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "Msg.h"

int		g_mqPost = -1;
char	g_buf[SIZE_SMSG - 1];
char	g_bufSmall[SIZE_BUFSMALL];
char	g_bufLarge[SIZE_BUFLARGE];
SMSG	g_post;

int		g_mqBeil = -1;

void Message(PSZ msg, int nLength)
{
	if (nLength == 0) {
		printf((const char*)msg);
		//nLength = strlen((char*)msg);
	}
	else if (g_mqPost >= 0) {
		struct mq_attr attr;
		if (mq_getattr(g_mqPost, &attr) >= 0) {
			if (attr.mq_curmsgs < attr.mq_maxmsg) {
				if (nLength > 0) {
					if (nLength > (SIZE_SMSG - 1))	nLength = SIZE_SMSG - 1;
					memcpy((PVOID)&(g_post.nBuf[1]), (PVOID)msg, nLength);
					g_post.nBuf[0] = '\t';
					g_post.dwLength = ++ nLength;
					g_post.wID = MSG_SEND;
					mq_send(g_mqPost, (char*)&g_post, sizeof(SMSG), NULL);	//PRIORITY_POST);
				}
			}
		}
	}
}

void CmdLine(WORD cmd)
{
	if (g_mqBeil >= 0) {
		struct mq_attr attr;
		if (mq_getattr(g_mqBeil, &attr) >= 0) {
			if (attr.mq_curmsgs < attr.mq_maxmsg) {
				if (cmd > BEILCMD_NON)
					mq_send(g_mqBeil, (char*)&cmd, sizeof(WORD), NULL);
			}
		}
	}
}
