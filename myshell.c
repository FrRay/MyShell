/*
 * =====================================================================================
 *       Filename:  myshell.c
 *    Description:  my simple shell
 *        Version:  1.0
 *        Created:  2022.4.23
 *         Author:  FuYongliang,1077376663@qq.com
 *        Company:  XCU
 * =====================================================================================
 */

/*

编译运行： gcc myshell.c -o Myshell;./Myshell

*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#define MAXARG 512  // max number of char
#define MAXLINE 128 // max length of every parameter
#define MAXCOUNT 16 // max count of parameters
#define MAXLIST 10  // max list of parameters
struct passwd *pwd; // the working url

void myprint(char *);
int myread(char **);
int mycmd(char **);
void proc();

int main()
{
    proc();
    return 0;
}
void proc()
{
    char *show;               //提示符，让用户输入新的命令
    char **parameters;        //读取用户由键盘输入的命令行
    int status, i;            //子进程的pid和结束代码
    pwd = getpwuid(getuid()); // pwd->pw_dir check if or not cd in home dir
    pid_t fpid;               //指示进程
    while (1)
    {

        show = (char *)malloc(sizeof(char) * MAXARG);
        myprint(show);
        printf("%s", show);

        parameters = malloc(MAXCOUNT * MAXLINE);
        int count = myread(parameters); //用count记录parameters中有几条【参数】

        if (strchr(parameters[count], '&'))
        {
            // (5)如果命令末尾有&号（后台命令符号），
            //         则终端进程不用系统调用wait( )等待，立即发提示符，
            //             让用户输入下一个命令，转⑴。
            printf("wait for naxt command……\n");
            // printf("count！is %d **p[]is %s**\n\n",count,parameters[count]);

            char **parameters2;
            parameters2 = malloc((MAXCOUNT * MAXLINE));
            int count2 = myread(parameters2);
            if (count2 < 0)
            {
                printf("command is erro in the end!!!\n"); //
            }
            for (int i = 0; i <= count2; i++)
            {
                parameters[count + i] = parameters2[i]; //把第二次的拼接上去
            }
            count += count2;
            free(parameters2);
            parameters2 = NULL;
            // printf("count is %d **p[0]2is %s**\n\n",count,parameters[0]);
            // printf("count is %d **p[1]2is %s**\n\n",count,parameters[count]);
        }

        // printf("%s\n",parameters[0]);
        if (mycmd(parameters)) //先调用自建命令
            continue;
        // printf("hhhhh%s\n",parameters[0]);

        //(2)分析命令，以命令名作为文件名，并将其它参数改造为
        //     系统调用execve( )内部处理所要求的形式。

        // (3)终端进程调用fork( )建立一个子进程。
        fpid = fork();
        if (fpid < 0)
        {

            printf("error in fork!");
        }
        else if (fpid == 0)
        { //子进程
            // printf("i am the child process, my process id is %d/n", getpid());

            //当子进程运行时调用execve( )，
            //         子进程根据文件名（即命令名）到目录中查找有关文件（这是命令解释程序构成的文件），
            //             将它调入内存，执行这个程序（解释这条命令）。

            if (execvp(parameters[0], parameters) == -1) //调用execv，返回-1表示出错
            {
                printf("command is little error!\n");
            }

            exit(110); //子进程返回代码

            //     当子进程运行时调用execve( )，
            //         子进程根据文件名（即命令名）到目录中查找有关文件（这是命令解释程序构成的文件），
            //             将它调入内存，执行这个程序（解释这条命令）。
        }
        else
        { //父进程

            wait(&status); // Wait for a child to die. When one does, put its status(pid) in *STAT_LOC and return its process ID.
            // i = WEXITSTATUS(status); // WEXITSTATUS(status)取得子进程 exit()返回的结束代码
            continue;
            // (4)终端进程本身用系统调用wait( )来等待子进程完成
            //         （如果是后台命令，则不等待）。
            // }
        }

        printf("\n");
        free(show);
        show = NULL; //防止内存泄漏及野指针

        free(parameters);
        parameters = NULL;
    }
}

const int max_name_len = 256;
const int max_path_len = 1024;
int length; // to add prompt's a other string
void myprint(char *prompt)
{
    // struct passwd *pwd; // use struct of passwd to record cd
    extern struct passwd *pwd; // pwd->pw_dir check if or not cd in home dir
    char hostname[max_name_len];
    char pathname[max_name_len];
    // check if or not get the hostname
    if (gethostname(hostname, max_name_len) == 0)
    {
        sprintf(prompt, "[ishell]%s@%s:", pwd->pw_name, hostname);
    }
    else
    {
        sprintf(prompt, "[ishell]%s@unkown:", pwd->pw_name);
    }
    length = strlen(prompt);
    getcwd(pathname, max_path_len); // chdir 会调整这个getcwd的工作目录
    if (strlen(pathname) < strlen(pwd->pw_dir) ||
        strncmp(pathname, pwd->pw_dir, strlen(pwd->pw_dir)) != 0)
    {
        sprintf(prompt + length, "%s", pathname);
    }
    else
    {
        sprintf(prompt + length, "~%s", pathname + strlen(pwd->pw_dir)); //表示从pwd->pw_dir目录截取到最后，pwd->pw_dir默认表示到home目录
    }
    length = strlen(prompt);
    if (geteuid() == 0)
    { //如果用户识别码为0，则证明不在home路径，拼接“#”
        sprintf(prompt + length, "#");
    }
    else
    {
        sprintf(prompt + length, "$");
    }
}

int myread(char **parameters)
{

    char *buffer;
    buffer = malloc(sizeof(char) * MAXLINE);
    char *Res_fgets = fgets(buffer, MAXLINE, stdin);
    if (Res_fgets == NULL)
    {
        printf("\n");
        exit(0);
    }
    // buffer = readline();
    //对存放一整行的buffer进行 分割

    if (buffer == NULL)
    {
        printf("\n");
        exit(0);
    }

    parameters[0] = strtok(buffer, " ");
    int count = 0;
    while ((parameters[count + 1] = strtok(NULL, " ")))
    {
        count++;
    }
    parameters[count][strlen(parameters[count]) - 1] = '\0'; //去掉最后一个的换行符

    return count;
}
int mycmd(char **parameters)
{
    extern struct passwd *pwd;

    if (strcmp(parameters[0], "exit") == 0 || strcmp(parameters[0], "quit") == 0)
        exit(0);
    else if (strcmp(parameters[0], "about") == 0)
    {
        printf("This is a eazy of shell (bash) in Linux.\n");
        return 1;
    }
    else if (strcmp(parameters[0], "cd") == 0) //实现cd命令
    {
        char *cd_path = NULL;

        if (parameters[1] == NULL)
        // make "cd" to "cd .." as in bash
        {
            parameters[1] = malloc(3 * sizeof(char));
            parameters[1][0] = '.';
            parameters[1][1] = '.';
            parameters[1][2] = '\0';
        }
        if (parameters[1][0] == '~')
        {
            cd_path = malloc(strlen(pwd->pw_dir) + strlen(parameters[1]));
            //'~' makes length 1 more,but instead of '\0'
            if (cd_path == NULL)
            {
                printf("cd:malloc failed.\n");
            }
            strcpy(cd_path, pwd->pw_dir);                                                     //复制Home目录
            strncpy(cd_path + strlen(pwd->pw_dir), parameters[1] + 1, strlen(parameters[1])); //继续复制~后的目录
            // printf("path with ~:\n%s\n",cd_path);
        }
        else
        {
            cd_path = malloc(strlen(parameters[1] + 1));
            if (cd_path == NULL)
            {
                printf("cd:malloc failed.\n");
            }
            strcpy(cd_path, parameters[1]);
        }

        if (chdir(cd_path) != 0) // chdir()用来将当前的工作目录改变成以参数path 所指的目录.
            printf("-ishell: cd: %s:%s\n", cd_path, strerror(errno));

        free(cd_path);
    }
    else if (strcmp(parameters[0], "^C") == 0)
    {
        /* code */

        printf("hhhh\n");
    }

    return 0;
}
