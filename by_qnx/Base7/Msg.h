/*
 * Msg.h
 *
 *  Created on: 2010. 09. XX
 *      Author: Che
 */

#ifndef MSG_H_
#define MSG_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include "..\Inform\Internal.h"

extern int	g_mqPost;
extern char	g_buf[];
extern char	g_bufSmall[];
extern char	g_bufLarge[];
extern SMSG	g_post;
#define	SIZE_BUFSMALL	512
#define	SIZE_BUFLARGE	SIZE_SMSG

extern int	g_mqBeil;

void	Message(PSZ msg, int nLength);
void	CmdLine(WORD cmd);

#ifdef	__cplusplus
}
#endif

#endif /* MSG_H_ */
