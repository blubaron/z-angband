/* File: message.h */

/* Purpose: extern declarations (variables and functions) */
#ifndef INCLUDED_MESSAGE_H
#define INCLUDED_MESSAGE_H

/* message.c */
extern byte get_msg_type_color(byte a);
extern s16b message_num(void);
extern cptr message_str(s16b age);
extern u16b message_type(s16b age);
extern byte message_color(s16b age);
extern errr message_color_define(u16b type, byte color);
extern void message_add(cptr str, u16b type);
extern errr messages_init(void);
extern void messages_free(void);
extern void set_message_type(char *buf, uint max, cptr fmt, va_list *vp);
extern void msgf(cptr fmt, ...);
extern void msg_effect(u16b type, s16b extra);
extern void message_flush(void);

#endif /* INCLUDED_MESSAGE_H */

