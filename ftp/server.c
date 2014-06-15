/*
 *  Modified by Jiajun Li 2014
 */

#include "common.h"
#include <time.h>

// 服务器从这里提供服务
void server(int port)
{
  int sock = create_socket(port);
  struct sockaddr_in client_address;
  int len = sizeof(client_address);
  int connection, pid, bytes_read;

  // 创建套接字成功,打印欢迎信息
  printf("************************\n");
  printf("Welcome to kimi's ftp server\n");
  printf("Port: 8021\n");
  time_t t;
  time(&t);
  printf("Date: %s\n", ctime(&t));
  printf("************************\n");

  // server 监听8021端口, 处理来自client 的请求
  while(1)
  {
    connection = accept(sock, (struct sockaddr*) &client_address,&len);
    char buffer[BSIZE];
    Command *cmd = malloc(sizeof(Command));
    State *state = malloc(sizeof(State));
    pid = fork();

    memset(buffer,0,BSIZE);

    if(pid < 0)
    {
      fprintf(stderr, "Cannot create child process.");
      exit(EXIT_FAILURE);
    }

    // 子进程中处理 命令&数据 传送
    if(pid == 0){
      close(sock);
      char welcome[BSIZE] = "220 ";
      if(strlen(welcome_message)<BSIZE-4)
        strcat(welcome,welcome_message);
      else
        strcat(welcome, "Welcome to nice FTP service.");

      // 通过控制端口返回欢迎信息
      strcat(welcome,"\n");
      write(connection, welcome,strlen(welcome));

      // 读取客户端发来的命令
      while (bytes_read = read(connection,buffer,BSIZE))
      {

        signal(SIGCHLD,my_wait);

        if(!(bytes_read>BSIZE))
        {
          printf("User %s sent command: %s\n",(state->username==0)?"unknown":state->username,buffer);

          // 解析命令, 存储到Command结构中
          parse_command(buffer,cmd);
          state->connection = connection;

          // 根据request请求命令 产生相应(包括命令响应 & 数据响应)
          if(buffer[0] <= 127 || buffer[0] >= 0)
            response(cmd,state);

          memset(buffer,0,BSIZE);
          memset(cmd,0,sizeof(cmd));
        }
        else
          perror("server:read");
      }

      printf("Client disconnected.\n");
      exit(0);
    }
    else
    {
      printf("closing... :(\n");
      close(connection);
    }
  }
}

// 创建监听套接字
int create_socket(int port)
{
  int sock;
  int reuse = 1;

  struct sockaddr_in server_address = (struct sockaddr_in)
  {
     AF_INET,
     htons(port),
     (struct in_addr){INADDR_ANY}
  };


  if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    fprintf(stderr, "Cannot open socket");
    exit(EXIT_FAILURE);
  }

  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);

  if(bind(sock,(struct sockaddr*) &server_address, sizeof(server_address)) < 0)
  {
    fprintf(stderr, "Cannot bind socket to address");
    exit(EXIT_FAILURE);
  }

  listen(sock,5);
  return sock;
}

// 返回通过监听套接字返回的连接套接字
int accept_connection(int socket)
{
  int addrlen = 0;
  struct sockaddr_in client_address;
  addrlen = sizeof(client_address);
  return accept(socket,(struct sockaddr*) &client_address,&addrlen);
}

// 获取客户端连接的服务器 ip
// 主要应用于多宿主主机
void getip(int sock, int *ip)
{
  socklen_t addr_size = sizeof(struct sockaddr_in);
  struct sockaddr_in addr;
  getsockname(sock, (struct sockaddr *)&addr, &addr_size);
  int host,i;

  host = (addr.sin_addr.s_addr);
  for(i=0; i<4; i++)
  {
    ip[i] = (host>>i*8)&0xff;
  }
}

// 查找命令
// 目前支持的命令包括: ABOR, CWD, DELE, LIST, MDTM, MKD, NLST, PASS, PASV
// PORT, PWD, QUIT, RETR, RMD, RNFR, RNTD, SITE, SIZE, STOR, TYPE, USER, NOOP
int lookup_cmd(char *cmd){
  const int cmdlist_count = sizeof(cmdlist_str)/sizeof(char *);
  return lookup(cmd, cmdlist_str, cmdlist_count);
}

// 字符串查找
// 可用于查找已知用户/可用命令等
int lookup(char *needle, const char **haystack, int count)
{
  int i;
  for(i=0;i<count; i++){
    if(strcmp(needle,haystack[i])==0)return i;
  }
  return -1;
}


// 向用户返回 reponse 控制命令
void write_state(State *state)
{
  write(state->connection, state->message, strlen(state->message));
}

// 在PASV(被动)模式下，服务器动态产生被动端口号
void gen_port(Port *port)
{
  srand(time(NULL));
  port->p1 = 128 + (rand() % 64);
  port->p2 = rand() % 0xff;

}

// 解析client等控制命令
// 包括命令码 & 参数
void parse_command(char *cmdstring, Command *cmd)
{
  sscanf(cmdstring,"%s %s",cmd->command,cmd->arg);
}

// 当子进程返回，链接断开，释放僵尸子进程的资源  
void my_wait(int signum)
{
  int status;
  wait(&status);
}

/*
 * 程序入口
 * 默认在8021端口监听
 */

main()
{
  server(8021);
  return 0;
}
