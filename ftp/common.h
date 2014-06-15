/*
 * Modified by Jiajun Li 2014
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <netinet/in.h>
#include <time.h>
#include <dirent.h>

#ifndef BSIZE
  #define BSIZE 1024
#endif

typedef struct Port
{
  int p1;
  int p2;
} Port;

typedef struct State
{
  int mode;

  int logged_in;

  int username_ok;

  char *username;

  char *message;

  int connection;

  int sock_pasv;

  int tr_pid;

} State;

typedef struct Command
{
  char command[5];
  char arg[BSIZE];
} Command;

typedef enum conn_mode{ NORMAL, SERVER, CLIENT }conn_mode;

typedef enum cmdlist
{
  ABOR, CWD, DELE, LIST, MDTM, MKD, NLST, PASS, PASV,
  PORT, PWD, QUIT, RETR, RMD, RNFR, RNTO, SITE, SIZE,
  STOR, TYPE, USER, NOOP
} cmdlist;

// 目前支持的命令
static const char *cmdlist_str[] =
{
  "ABOR", "CWD", "DELE", "LIST", "MDTM", "MKD", "NLST", "PASS", "PASV",
  "PORT", "PWD", "QUIT", "RETR", "RMD", "RNFR", "RNTO", "SITE", "SIZE",
  "STOR", "TYPE", "USER", "NOOP"
};

// 目前支持的用户(都是匿名用户)
static const char *usernames[] =
{
  "ftp","anonymous","public"
};

// client 链接到 server 时返回的欢迎信息
static char *welcome_message = "Welcome to Jiajun Li's ftp server!";

// Server.c 文件的函数原型
void gen_port(Port *);
void parse_command(char *, Command *);
int create_socket(int port);
void write_state(State *);
int accept_connection(int);

// handles.c 文件的函数原型
void response(Command *, State *);
void ftp_user(Command *, State *);
void ftp_pass(Command *, State *);
void ftp_pwd(Command *, State *);
void ftp_cwd(Command *, State *);
void ftp_mkd(Command *, State *);
void ftp_rmd(Command *, State *);
void ftp_pasv(Command *, State *);
void ftp_list(Command *, State *);
void ftp_retr(Command *, State *);
void ftp_stor(Command *, State *);
void ftp_dele(Command *, State *);
void ftp_size(Command *, State *);
void ftp_quit(State *);
void ftp_type(Command *, State *);
void ftp_abor(State *);

void str_perm(int, char *);
void my_wait(int);
