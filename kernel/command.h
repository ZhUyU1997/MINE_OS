#ifndef __COMMAND_H__
#define __COMMAND_H__
typedef struct cmd_table {
	char	*name;		/* Command Name			*/
	/* Implementation function	*/
	int		(*cmd)(struct cmd_table *, int, char *[]);
	char	*usage;		/* Usage message	(short)	*/
	char	*help;		/* Help  message	(long)	*/
} cmd_table;

#define CMD_DEFINE(cmd_name,cmd_usage,cmd_help) int	cmd_##cmd_name(cmd_table *ct, int argc, char *argv[]);\
cmd_table ct_##cmd_name={\
	.name= #cmd_name ,\
	.help=cmd_help,\
	.usage=cmd_usage,\
	.cmd=cmd_##cmd_name\
};\
int	cmd_##cmd_name(cmd_table *ct, int argc, char *argv[])

int cmd_loop();
#endif