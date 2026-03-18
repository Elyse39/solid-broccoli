#include "attendance.h"

void encrypt_password(const char *input, char *output)
{
    unsigned long hash = 5381;
    int c;
    while ((c = *input++))
    {
        hash = ((hash << 5) + hash) + c;
    }
    sprintf(output, "%lx", hash);
}

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

void get_current_date(char *date_str)
{
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(date_str, 15, "%Y-%m-%d", tm_info);
}

void get_current_time(char *time_str)
{
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(time_str, 10, "%H:%M", tm_info);
}

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
    return (int)(seconds / (60 * 60 * 24)) + 1;
}

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

int validate_password(const char *pwd)
{
    int len = strlen(pwd);
    if (len < 6 || len > 14)
    {
        return 0;
    }
    for (int i = 0; i < len; i++)
    {
        if (!isalnum(pwd[i]) && pwd[i] != '_')
        {
            return 0;
        }
    }
    return 1;
}

void get_valid_password(char *pwd)
{
    while (1)
    {
        get_masked_password(pwd, 64);
        if (validate_password(pwd))
        {
            break;
        }
        printf("[格式拦截] 密码必须为 6-14 位，且只能包含大小写字母、数字和下划线！\n请重新输入: ");
    }
}

void clear_input_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

int get_int_input(const char *prompt, int min, int max)
{
    char buffer[100];
    int value;

    while (1)
    {
        printf("%s", prompt);

        if (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            buffer[strcspn(buffer, "\n")] = 0;

            if (strlen(buffer) == 0)
                continue;

            int is_valid = 1;
            int start_idx = 0;
            if (buffer[0] == '-')
                start_idx = 1;

            for (int i = start_idx; buffer[i] != '\0'; i++)
            {
                if (!isdigit(buffer[i]))
                {
                    is_valid = 0;
                    break;
                }
            }

            if (is_valid)
            {
                value = atoi(buffer);
                if (value >= min && value <= max)
                {
                    return value;
                }
            }
        }
        printf("[格式拦截] 只能输入纯整数（不可带小数/字母），且有效范围在 %d 到 %d 之间！\n", min, max);
    }
}

double get_double_input(const char *prompt, double min, double max)
{
    double value;
    int status;
    while (1)
    {
        printf("%s", prompt);
        status = scanf("%lf", &value);
        clear_input_buffer();
        if (status == 1 && value >= min && value <= max)
        {
            return value;
        }
        printf("[格式错误] 请输入 %.2f 到 %.2f 之间的有效数值（防止数据溢出）！\n", min, max);
    }
}

void get_string_input(const char *prompt, char *buffer, int max_len)
{
    while (1)
    {
        printf("%s", prompt);
        char format[20];
        sprintf(format, "%%%ds", max_len - 1);
        if (scanf(format, buffer) == 1)
        {
            clear_input_buffer();
            return;
        }
        clear_input_buffer();
    }
}

// 【修复：强制非空输入，吸收残留换行符防死循环】
void get_non_empty_string(const char *prompt, char *buffer, int max_bytes)
{
    while (1)
    {
        printf("%s", prompt);
        char temp[1024];
        if (!fgets(temp, sizeof(temp), stdin))
            continue;

        // 吸收上一层 scanf 可能残留的纯换行符
        if (strcmp(temp, "\n") == 0 || strcmp(temp, "\r\n") == 0)
        {
            continue;
        }

        if (temp[strlen(temp) - 1] != '\n')
        {
            clear_input_buffer();
            printf("[格式错误] 输入内容太长，请精简至 10 个中文字符以内！\n");
            continue;
        }

        temp[strcspn(temp, "\n")] = 0;
        int len = strlen(temp);

        if (len == 0)
        {
            printf("[格式错误] 输入不能为空，请重新输入！\n");
            continue;
        }

        if (len >= max_bytes)
        {
            printf("[格式错误] 输入内容太长，请精简至 10 个中文字符以内！\n");
            continue;
        }

        strcpy(buffer, temp);
        return;
    }
}

void get_date_input(const char *prompt, char *buffer)
{
    int y, m, d;
    char temp[1024];
    while (1)
    {
        printf("%s", prompt);
        if (fgets(temp, sizeof(temp), stdin) != NULL)
        {
            // 吸收残留空行
            if (strcmp(temp, "\n") == 0 || strcmp(temp, "\r\n") == 0)
                continue;

            temp[strcspn(temp, "\n")] = 0;
            if (strlen(temp) == 10)
            {
                if (isdigit(temp[0]) && isdigit(temp[1]) && isdigit(temp[2]) && isdigit(temp[3]) &&
                    temp[4] == '-' &&
                    isdigit(temp[5]) && isdigit(temp[6]) &&
                    temp[7] == '-' &&
                    isdigit(temp[8]) && isdigit(temp[9]))
                {

                    if (sscanf(temp, "%d-%d-%d", &y, &m, &d) == 3)
                    {
                        if (y >= 1900 && y <= 2100 && m >= 1 && m <= 12 && d >= 1 && d <= 31)
                        {
                            strcpy(buffer, temp);
                            return;
                        }
                    }
                }
            }
        }
        printf("[格式拦截] 请严格使用带补零的 10 位 YYYY-MM-DD 格式 (如 2026-03-13)，不可省略 0！\n");
    }
}

void get_month_input(const char *prompt, char *buffer)
{
    int y, m;
    char temp[1024];
    while (1)
    {
        printf("%s", prompt);
        if (fgets(temp, sizeof(temp), stdin) != NULL)
        {
            // 吸收残留空行
            if (strcmp(temp, "\n") == 0 || strcmp(temp, "\r\n") == 0)
                continue;

            temp[strcspn(temp, "\n")] = 0;

            if (strlen(temp) == 7)
            {
                if (isdigit(temp[0]) && isdigit(temp[1]) && isdigit(temp[2]) && isdigit(temp[3]) &&
                    temp[4] == '-' &&
                    isdigit(temp[5]) && isdigit(temp[6]))
                {

                    if (sscanf(temp, "%d-%d", &y, &m) == 2)
                    {
                        if (y >= 1900 && y <= 2100 && m >= 1 && m <= 12)
                        {
                            strcpy(buffer, temp);
                            return;
                        }
                    }
                }
            }
        }
        printf("[格式拦截] 请严格使用带补零的 7 位 YYYY-MM 格式 (如 2026-03)，不可省略 0！\n");
    }
}

void get_line_input(const char *prompt, char *buffer, int max_len)
{
    while (1)
    {
        printf("%s", prompt);
        if (!fgets(buffer, max_len, stdin))
            continue;
        // 吸收空行
        if (strcmp(buffer, "\n") == 0 || strcmp(buffer, "\r\n") == 0)
            continue;

        buffer[strcspn(buffer, "\n")] = 0;
        if (strlen(buffer) == max_len - 1)
        {
            clear_input_buffer();
        }
        return;
    }
}

SalaryRule load_salary_rule()
{
    SalaryRule rule = {50.0, 100.0, 200.0};
    FILE *fp = fopen("salary_rule.dat", "rb");
    if (fp)
    {
        fread(&rule, sizeof(SalaryRule), 1, fp);
        fclose(fp);
    }
    return rule;
}

void save_salary_rule(SalaryRule rule)
{
    FILE *fp = fopen("salary_rule.dat", "wb");
    if (fp)
    {
        fwrite(&rule, sizeof(SalaryRule), 1, fp);
        fclose(fp);
    }
}

Employee *find_employee_global(EmpNode *head, const char *id)
{
    while (head != NULL)
    {
        if (strcmp(head->data.id, id) == 0)
            return &(head->data);
        head = head->next;
    }
    return NULL;
}

// 【系统级修复】移除了内部的 srand，防止同秒内生成的验证码完全一致
int run_captcha_verification()
{
    char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    char captcha[5];
    char user_input[20];

    for (int i = 0; i < 4; i++)
    {
        int index = rand() % 62;
        captcha[i] = charset[index];
    }
    captcha[4] = '\0';

    printf("\n[系统安全防护] 识别到环境风险，请输入验证码以继续操作\n");
    printf(">>> 验证码: 【 %s 】 <<<\n", captcha);
    get_string_input("请输入验证码 (区分大小写): ", user_input, 20);

    if (strcmp(captcha, user_input) == 0)
        return 1;
    return 0;
}

void init_security_question(Employee *emp, EmpNode *emp_list)
{
    printf("\n--- 设置账户密保问题 ---\n");
    printf("1. 您母亲的名字是？\n");
    printf("2. 您就读的小学名字是？\n");
    printf("3. 您第一只宠物的名字是？\n");
    emp->security_q_id = get_int_input("请选择密保问题 (1-3): ", 1, 3);
    get_string_input("请输入密保答案: ", emp->security_answer, 50);
    save_emp_list(emp_list);
    printf(">> 密保问题设置成功！\n");
}

void forgot_password_flow(Employee *emp, EmpNode *emp_list)
{
    printf("\n=============================\n");
    printf("      账户密码找回向导     \n");
    printf("=============================\n");

    if (emp->security_q_id == 0)
    {
        printf("[系统提示] 您未设置密保问题，无法自助找回！请联系超级管理员重置。\n");
        return;
    }

    if (!run_captcha_verification())
    {
        printf("[错误] 验证码输入错误，找回流程中止。\n");
        return;
    }

    printf("\n>>> 密保身份验证 <<<\n");
    if (emp->security_q_id == 1)
        printf("问题：您母亲的名字是？\n");
    else if (emp->security_q_id == 2)
        printf("问题：您就读的小学名字是？\n");
    else if (emp->security_q_id == 3)
        printf("问题：您第一只宠物的名字是？\n");

    char answer_input[50];
    get_string_input("请输入答案: ", answer_input, 50);

    if (strcmp(emp->security_answer, answer_input) != 0)
    {
        printf("[错误] 密保答案不匹配，拒绝重置密码！\n");
        write_log(emp->id, "尝试找回密码失败：密保错误");
        return;
    }

    char new_pwd[64], hashed_new[64];
    printf("\n身份验证通过！\n");
    printf("请输入新的登录密码: ");

    get_valid_password(new_pwd);

    encrypt_password(new_pwd, hashed_new);
    strcpy(emp->password, hashed_new);

    emp->failed_attempts = 0;
    emp->lock_until = 0;

    save_emp_list(emp_list);
    printf("\n>>> 密码重置成功，账户已解锁，请重新登录！ <<<\n");
    write_log(emp->id, "通过密保成功找回并重置了密码");
}