#include "attendance.h"

// 创新点：密码加密存储 (采用简单的纯C哈希混淆，防止明文存储)
// 将输入的明文转换为一串十六进制哈希字符串
void encrypt_password(const char *input, char *output)
{
    unsigned long hash = 5381;
    int c;
    while ((c = *input++))
    {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    sprintf(output, "%lx", hash);
}

// 创新点：操作日志系统
// 记录格式：[时间] 操作人工号: 操作内容
void write_log(const char *operator_id, const char *action)
{
    FILE *fp = fopen("system_log.txt", "a");
    if (fp)
    {
        char date[15], time_str[10];
        get_current_date(date);
        get_current_time(time_str);
        fprintf(fp, "[%s %s] User:%s | Action:%s\n", date, time_str, operator_id, action);
        fclose(fp);
    }
}

// 获取当前系统日期 YYYY-MM-DD
void get_current_date(char *date_str)
{
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(date_str, 15, "%Y-%m-%d", tm_info);
}

// 获取当前系统时间 HH:MM
void get_current_time(char *time_str)
{
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(time_str, 10, "%H:%M", tm_info);
}

// 创新点：计算“入职第N天”
// 将 YYYY-MM-DD 字符串转为时间戳并计算与今天的差值
int calculate_days_from(const char *start_date)
{
    struct tm start_tm = {0};
    int year, month, day;
    if (sscanf(start_date, "%d-%d-%d", &year, &month, &day) != 3)
    {
        return 0;
    }
    start_tm.tm_year = year - 1900;
    start_tm.tm_mon = month - 1;
    start_tm.tm_mday = day;

    time_t start_time = mktime(&start_tm);
    time_t now = time(NULL);

    double seconds = difftime(now, start_time);
    return (int)(seconds / (60 * 60 * 24)) + 1; // 加上第一天
}

// 安全交互：控制台密码掩码输入
void get_masked_password(char *pwd, int max_len)
{
    int i = 0;
    char ch;
    while (1)
    {
        ch = _getch();
        if (ch == '\r' || ch == '\n')
        {
            pwd[i] = '\0';
            printf("\n");
            break;
        }
        else if (ch == '\b' && i > 0)
        {
            i--;
            printf("\b \b");
        }
        else if (i < max_len - 1 && ch != '\b')
        {
            pwd[i++] = ch;
            printf("*");
        }
    }
}