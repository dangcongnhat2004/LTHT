#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    pid_t pid;
    int counter;
    int fd;
    int max_iterations;
    char buffer[100];

    // Kiểm tra số lần lặp tối đa từ tham số dòng lệnh
    if (argc < 2)
        max_iterations = 5;
    else {
        max_iterations = atoi(argv[1]);
        if ((max_iterations <= 0) || (max_iterations > 20))
            max_iterations = 10;
    }

    pid = fork(); // Tạo tiến trình con

    // Kiểm tra lỗi khi tạo tiến trình con
    if (pid < 0) {
        perror("fork error\n");
        exit(1);
    }
    
    // Tiến trình con
    if (pid == 0) {
        // Mở file để ghi log
        fd = open("/tmp/DAEMON.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("daemon txt file open error\n");
            return 1;
        }

        // In ra thông tin của tiến trình con
        printf("Child: pid is %d and ppid is %d\n", getpid(), getppid());
        printf("\nChild process before becoming session leader\n");

        // Hiển thị thông tin về PID và SID trước khi trở thành session leader
        sprintf(buffer, "ps -eo pid,sid,comm,tty | grep %s", argv[0]);
        system(buffer);

        // Tiến trình con trở thành session leader
        setsid();
        
        printf("\nChild process after becoming session leader\n");

        // Hiển thị thông tin về PID và SID sau khi trở thành session leader
        sprintf(buffer, "ps -eo pid,sid,comm,tty | grep %s", argv[0]);
        system(buffer);

        // Đóng tất cả các file descriptors để tách biệt khỏi terminal
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    } else {
        // Tiến trình cha chỉ in thông tin và thoát
        printf("Parent: pid is %d and ppid is %d\n", getpid(), getppid());
        printf("Parent: Exiting\n");
        exit(0);
    }

    // Tiến trình con thực hiện vòng lặp ghi log vào file
    for (counter = 0; counter < max_iterations; counter++) {
        sprintf(buffer, "Daemon process: pid is %d and ppid is %d\n", getpid(), getppid());
        write(fd, buffer, strlen(buffer));
        sleep(2);
    }

    // Ghi thông báo "Done" khi hoàn thành
    strcpy(buffer, "Done\n");
    write(fd, buffer, strlen(buffer));

    // Không thể in ra thông báo sau khi file descriptors đã bị đóng
    close(fd);
    return 0;
}
