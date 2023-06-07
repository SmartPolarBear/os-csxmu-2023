
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <string.h>  
#include <ctype.h>  
#include <pwd.h>  
#include <signal.h>  
#include <string.h>  
#include <sys/types.h>  
#include <sys/wait.h>
#include "File.h"  
#include "Disk.h"
/**  
 * ctrl-C的信号处理程序  
 * @param sig 信号值  
 */  
static void sigHandler(int sig) {  
    exitSystem();  
    exit(0);  
}
int main() {  
    //初始化内存文件系统  
    initFileSystem();  
    printf("Welcome to myRAMFS!\n");  
    //注册Ctrl-C处理事件  
    if (signal(SIGINT, sigHandler) == SIG_ERR) {  
        perror("signal");  
        exit(1);  
    }  
    //初始化根目录  
    initRootDir();  
    //循环捕捉用户命令并执行  
    while (1) {  
        //仿照shell显示当前所在的绝对路径  
        printf("[myRAMFS@localhost %s] $ ", getPath());  
        //清空缓冲流  
        fflush(stdout);  
        //字符串缓冲池  
        char buf[1024];  
        int s = read(0, buf, 1024);  
        if (s > 0)//有读取到字符  
        {  
            int i = 0;  
            for (i = 0; i < s; ++i) {  
                if (buf[i] == '\b' && i >= 1) {  
                    int j = 0;  
                    for (j = i + 1; j < s; ++j) {  
                        buf[j - 2] = buf[j];  
                    }  
                    buf[s - 2] = 0;  
                    i -= 1;  
                } else if (buf[i] == '\b' && i == 0) {  
                    int j = 0;  
                    for (j = 1; j < s; ++j) {  
                        buf[j - 1] = buf[j];  
                    }  
                    buf[s - 1] = 0;  
                } else {  
                    continue;  
                }  
            }  
            buf[s] = 0;  
        } else {  
            continue;  
        }  
        char *start = buf;  
        int i = 1;  
        char *args[100] = {0};  
        char content[100] = "";  
        args[0] = start;  
        //根据空格拆分字符串  
        while (*start) {  
            if (isspace(*start)) {  
                *start = 0;  
                start++;  
                args[i++] = start;  
            } else {  
                ++start;  
            }  
        }  
        args[i - 1] = NULL;  
        //依次处理args[0]代表的命令，参数不匹配时提供参数列表  
        if (!strcmp(args[0], "ls")) {  
            showDir();  
        } else if (!strcmp(args[0], "cd")) {  
            if (args[1] != NULL)  
                changeDir(args[1]);  
            else {  
                printf("usage:cd dirName\n");  
            }  
        } else if (!strcmp(args[0], "rn")) {  
            if (args[1] != NULL && args[2] != NULL)  
                changeName(args[1], args[2]);  
            else {  
                printf("usage:rn oldName, newName\n");  
            }  
        } else if (!strcmp(args[0], "touch")) {  
            if (args[1] != NULL && args[2] != NULL)  
                creatFile(args[1], atoi(args[2]));  
            else {  
                printf("usage:touch filename fileSize\n");  
            }  
        } else if (!strcmp(args[0], "mkdir")) {  
            if (args[1] != NULL)  
                creatDir(args[1]);  
            else {  
                printf("usage:mkdir dirName\n");  
            }  
        } else if (!strcmp(args[0], "rm")) {  
            if (args[1] != NULL)  
                deleteFile(args[1]);  
            else {  
                printf("usage:rm filename\n");  
            }  
        } else if (!strcmp(args[0], "rmdir")) {  
            if (args[1] != NULL)  
                deleteDir(args[1]);  
            else {  
                printf("usage:rmdir filename\n");  
            }  
        } else if (!strcmp(args[0], "read")) {  
            if (args[1] != NULL && args[2] != NULL)  
                read_file(args[1], atoi(args[2]));  
            else {  
                printf("usage:read filename byteSize\n");  
            }  
        } else if (!strcmp(args[0], "write")) {  
            if (args[1] != NULL && args[2] != NULL) {  
                for (i = 2; i < 100; i++) {  
                    if (args[i] == NULL)  
                        break;  
                    strcat(content, args[i]);  
                    strcat(content, " ");  
                }  
                write_file(args[1], content);  
            } else {  
                printf("usage:write filename content\n");  
            }  
        } else if (!strcmp(args[0], "exit")) {  
            exitSystem();  
            exit(0);  
        } else {  
            printf("myRAMFS: command not found\n");  
        }  
    }  
    return 0;  
}  