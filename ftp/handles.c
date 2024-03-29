/*
 * Modified by Jiajun Li 2014
 */

#include "common.h"

// 根据传入的命令执行相应的响应
// 这里支持的命令有:
// USER, PASS, PASV, LIST, CWD, PWD, MKD, RETR, STOR, DELE
// SIZE, ABOR, QUIT, TYPE, NOOP
void response(Command *cmd, State *state)
{
  switch(lookup_cmd(cmd->command)){
    case USER: ftp_user(cmd,state); break;
    case PASS: ftp_pass(cmd,state); break;
    case PASV: ftp_pasv(cmd,state); break;
    case LIST: ftp_list(cmd,state); break;
    case CWD:  ftp_cwd(cmd,state); break;
    case PWD:  ftp_pwd(cmd,state); break;
    case MKD:  ftp_mkd(cmd,state); break;
    case RMD:  ftp_rmd(cmd,state); break;
    case RETR: ftp_retr(cmd,state); break;
    case STOR: ftp_stor(cmd,state); break;
    case DELE: ftp_dele(cmd,state); break;
    case SIZE: ftp_size(cmd,state); break;
    case ABOR: ftp_abor(state); break;
    case QUIT: ftp_quit(state); break;
    case TYPE: ftp_type(cmd,state); break;
    case NOOP:
      if(state->logged_in)
        state->message = "200 Nice to NOOP you!\n";
      else
        state->message = "530 NOOB hehe.\n";
      write_state(state);
      break;
    default:
      state->message = "500 Unknown command\n";
      write_state(state);
      break;
  }
}

// 处理USER命令
void ftp_user(Command *cmd, State *state)
{
  const int total_usernames = sizeof(usernames)/sizeof(char *);
  if(lookup(cmd->arg,usernames,total_usernames)>=0)
  {
    state->username = malloc(32);
    memset(state->username,0,32);
    strcpy(state->username,cmd->arg);
    state->username_ok = 1;
    state->message = "331 User name okay, need password\n";
  }
  else
    state->message = "530 Invalid username\n";
  write_state(state);
}

// 处理PASS命令
void ftp_pass(Command *cmd, State *state)
{
  if(state->username_ok==1)
  {
    state->logged_in = 1;
    state->message = "230 Login successful\n";
  }
  else
    state->message = "500 Invalid username or password\n";
  write_state(state);
}

// 处理PASV命令
void ftp_pasv(Command *cmd, State *state)
{
  if(state->logged_in)
  {
    int ip[4];
    char buff[255];
    char *response = "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)\n";
    Port *port = malloc(sizeof(Port));
    gen_port(port);
    getip(state->connection,ip);

    // 关闭原来的被动模式套接字（如果有）
    close(state->sock_pasv);

    // server 在这里开始被动监听数据套接字
    state->sock_pasv = create_socket((256*port->p1)+port->p2);
    printf("port: %d\n",256*port->p1+port->p2);
    sprintf(buff,response,ip[0],ip[1],ip[2],ip[3],port->p1,port->p2);
    state->message = buff;
    state->mode = SERVER;
    puts(state->message);
  }
  else
  {
    state->message = "530 Please login with USER and PASS.\n";
    printf("%s",state->message);
  }
  write_state(state);
}

// 处理 LIST 命令
void ftp_list(Command *cmd, State *state)
{
  if(state->logged_in==1)
  {
    struct dirent *entry;
    struct stat statbuf;
    struct tm *time;
    char timebuff[80], current_dir[BSIZE];
    int connection;
    time_t rawtime;

    char cwd[BSIZE], cwd_orig[BSIZE];
    memset(cwd,0,BSIZE);
    memset(cwd_orig,0,BSIZE);

    // 获取当前目录
    getcwd(cwd_orig,BSIZE);

    if(strlen(cmd->arg)>0&&cmd->arg[0]!='-')
      chdir(cmd->arg);

    getcwd(cwd,BSIZE);
    DIR *dp = opendir(cwd);

    if(!dp)
      state->message = "550 Failed to open directory.\n";
    else
      {
      if(state->mode == SERVER)
      {
        connection = accept_connection(state->sock_pasv);
        state->message = "150 Here comes the directory listing.\n";
        puts(state->message);

        while(entry=readdir(dp)){
          if(stat(entry->d_name,&statbuf)==-1)
            fprintf(stderr, "FTP: Error reading file stats...\n");
          else
          {
            char *perms = malloc(9);
            memset(perms,0,9);

            rawtime = statbuf.st_mtime;
            time = localtime(&rawtime);
            strftime(timebuff,80,"%b %d %H:%M",time);
            str_perm((statbuf.st_mode & ALLPERMS), perms);
            dprintf(connection,
                "%c%s %5d %4d %4d %8d %s %s\r\n",
                (entry->d_type==DT_DIR)?'d':'-',
                perms,statbuf.st_nlink,
                statbuf.st_uid,
                statbuf.st_gid,
                statbuf.st_size,
                timebuff,
                entry->d_name);
          }
        }
        write_state(state);
        state->message = "226 Directory send OK.\n";
        state->mode = NORMAL;
        close(connection);
        close(state->sock_pasv);

      }
      else if(state->mode == CLIENT)
        state->message = "502 Command not implemented.\n";
      else
        state->message = "425 Use PASV or PORT first.\n";
    }
    closedir(dp);
    chdir(cwd_orig);
  }
  else
    state->message = "530 Please login with USER and PASS.\n";
  state->mode = NORMAL;
  write_state(state);
}


// 处理 QUIT 命令
void ftp_quit(State *state)
{
  state->message = "221 Goodbye, friend. I never thought I'd die like this.\n";
  write_state(state);
  close(state->connection);
  exit(0);
}

// 处理 PWD 命令
void ftp_pwd(Command *cmd, State *state)
{
  if(state->logged_in)
  {
    char cwd[BSIZE];
    char result[BSIZE];
    memset(result, 0, BSIZE);
    if(getcwd(cwd,BSIZE)!=NULL)
    {
      strcat(result,"257 \"");
      strcat(result,cwd);
      strcat(result,"\"\n");
      state->message = result;
    }
    else
      state->message = "550 Failed to get pwd.\n";
    write_state(state);
  }
}

// 执行 CWD 命令
void ftp_cwd(Command *cmd, State *state)
{
  if(state->logged_in)
  {
    if(chdir(cmd->arg)==0)
      state->message = "250 Directory successfully changed.\n";
    else
      state->message = "550 Failed to change directory.\n";
  }
  else
    state->message = "500 Login with USER and PASS.\n";
  write_state(state);
}

// 处理 MKD 命令
void ftp_mkd(Command *cmd, State *state)
{
  if(state->logged_in)
  {
    char cwd[BSIZE];
    char res[BSIZE];
    memset(cwd,0,BSIZE);
    memset(res,0,BSIZE);
    getcwd(cwd,BSIZE);

    // 绝对命令
    if(cmd->arg[0]=='/')
    {
      if(mkdir(cmd->arg,S_IRWXU)==0)
      {
        strcat(res,"257 \"");
        strcat(res,cmd->arg);
        strcat(res,"\" new directory created.\n");
        state->message = res;
      }else
        state->message = "550 Failed to create directory. Check path or permissions.\n";
    }
    // 相对路径
    else
    {
      if(mkdir(cmd->arg,S_IRWXU)==0)
      {
        sprintf(res,"257 \"%s/%s\" new directory created.\n",cwd,cmd->arg);
        state->message = res;
      }
      else
        state->message = "550 Failed to create directory.\n";
    }
  }
  else
    state->message = "500 Good news, everyone! There's a report on TV with some very bad news!\n";
  write_state(state);
}

// 处理 RETR 命令
void ftp_retr(Command *cmd, State *state)
{

  if(fork()==0)
  {
    int connection;
    int fd;
    struct stat stat_buf;
    off_t offset = 0;
    int sent_total = 0;
    if(state->logged_in)
    {
    // 被动模式
      if(state->mode == SERVER)
      {
        if(access(cmd->arg,R_OK)==0 && (fd = open(cmd->arg,O_RDONLY)))
        {
          fstat(fd,&stat_buf);

          state->message = "150 Opening BINARY mode data connection.\n";

          write_state(state);

          connection = accept_connection(state->sock_pasv);
          close(state->sock_pasv);
          if(sent_total = sendfile(connection, fd, &offset, stat_buf.st_size)){

            if(sent_total != stat_buf.st_size)
            {
              perror("ftp_retr:sendfile");
              exit(EXIT_SUCCESS);
            }

            state->message = "226 File send OK.\n";
          }
          else
            state->message = "550 Failed to read file.\n";
        }
        else
          state->message = "550 Failed to get file\n";
      }
      else
        state->message = "550 Please use PASV instead of PORT.\n";
    }
    else
      state->message = "530 Please login with USER and PASS.\n";

    close(fd);
    close(connection);
    write_state(state);
    exit(EXIT_SUCCESS);
  }
  state->mode = NORMAL;
  close(state->sock_pasv);
}

// 处理 STOR 命令
void ftp_stor(Command *cmd, State *state)
{
  if(fork()==0)
  {
    int connection, fd;
    off_t offset = 0;
    int pipefd[2];
    int res = 1;
    const int buff_size = 8192;

    FILE *fp = fopen(cmd->arg,"w");

    if(fp==NULL)
      perror("ftp_stor:fopen");
    else if(state->logged_in)
    {
      if(!(state->mode==SERVER))
        state->message = "550 Please use PASV instead of PORT.\n";
      else
      {
        fd = fileno(fp);
        connection = accept_connection(state->sock_pasv);
        close(state->sock_pasv);
        if(pipe(pipefd)==-1)perror("ftp_stor: pipe");

        state->message = "125 Data connection already open; transfer starting.\n";
        write_state(state);

        /* Using splice function for file receiving.
         * The splice() system call first appeared in Linux 2.6.17.
         */

        while ((res = splice(connection, 0, pipefd[1], NULL, buff_size, SPLICE_F_MORE | SPLICE_F_MOVE))>0)
          splice(pipefd[0], NULL, fd, 0, buff_size, SPLICE_F_MORE | SPLICE_F_MOVE);

        if(res==-1)
        {
          perror("ftp_stor: splice");
          exit(EXIT_SUCCESS);
        }
        else
          state->message = "226 File send OK.\n";
        close(connection);
        close(fd);
      }
    }
    else
      state->message = "530 Please login with USER and PASS.\n";
    close(connection);
    write_state(state);
    exit(EXIT_SUCCESS);
  }
  state->mode = NORMAL;
  close(state->sock_pasv);

}

// 处理 ABOR 命令
void ftp_abor(State *state)
{
  if(state->logged_in)
  {
    state->message = "226 Closing data connection.\n";
    state->message = "225 Data connection open; no transfer in progress.\n";
  }
  else
    state->message = "530 Please login with USER and PASS.\n";
  write_state(state);

}

// 执行 TYPE 命令
void ftp_type(Command *cmd,State *state)
{
  if(state->logged_in)
  {
    if(cmd->arg[0]=='I')
      state->message = "200 Switching to Binary mode.\n";
    else if(cmd->arg[0]=='A')
      state->message = "200 Switching to ASCII mode.\n";
    else
      state->message = "504 Command not implemented for that parameter.\n";
  }
  else
    state->message = "530 Please login with USER and PASS.\n";
  write_state(state);
}

// 执行 DELE 命令
void ftp_dele(Command *cmd,State *state)
{
  if(state->logged_in)
  {
    if(unlink(cmd->arg)==-1)
      state->message = "550 File unavailable.\n";
    else
      state->message = "250 Requested file action okay, completed.\n";
  }
  else
    state->message = "530 Please login with USER and PASS.\n";
  write_state(state);
}

// 处理 RMD 命令
void ftp_rmd(Command *cmd, State *state)
{
  if(!state->logged_in)
    state->message = "530 Please login first.\n";
  else
  {
    if(rmdir(cmd->arg)==0)
      state->message = "250 Requested file action okay, completed.\n";
    else
      state->message = "550 Cannot delete directory.\n";
  }
  write_state(state);

}

// 处理 SIZE 命令
void ftp_size(Command *cmd, State *state)
{
  if(state->logged_in)
  {
    struct stat statbuf;
    char filesize[128];
    memset(filesize,0,128);

    if(stat(cmd->arg,&statbuf)==0)
    {
      sprintf(filesize, "213 %d\n", statbuf.st_size);
      state->message = filesize;
    }else
      state->message = "550 Could not get file size.\n";
  }
  else
    state->message = "530 Please login with USER and PASS.\n";

  write_state(state);

}

void str_perm(int perm, char *str_perm)
{
  int curperm = 0;
  int flag = 0;
  int read, write, exec;

  char fbuff[3];

  read = write = exec = 0;

  int i;
  for(i = 6; i>=0; i-=3)
  {
    curperm = ((perm & ALLPERMS) >> i ) & 0x7;

    memset(fbuff,0,3);
    read = (curperm >> 2) & 0x1;
    write = (curperm >> 1) & 0x1;
    exec = (curperm >> 0) & 0x1;

    sprintf(fbuff,"%c%c%c",read?'r':'-' ,write?'w':'-', exec?'x':'-');
    strcat(str_perm,fbuff);
  }
}
