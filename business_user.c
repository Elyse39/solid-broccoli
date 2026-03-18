#include "attendance.h"

// ==========================================
// 入职第 N 天温馨打卡问候 + 【修复：上下班闭环与防重复】
// ==========================================
void clock_in_with_greeting(Employee *emp, RecordNode **rec_list)
{
    char today[15], now_time[10];
    get_current_date(today);
    get_current_time(now_time);

    RecordNode *curr = *rec_list;
    while (curr != NULL)
    {
        if (strcmp(curr->data.emp_id, emp->id) == 0 && strcmp(curr->data.date, today) == 0)
        {
            // 如果已存在今日记录，检查是否已经下班打卡
            if (strlen(curr->data.clock_out) > 0)
            {
                printf("\n>>> 【系统提示】您今日 (%s) 上下班打卡均已完成，请勿重复打卡！ <<<\n", today);
                return;
            }
            else
            {
                // 补充下班打卡时间
                strcpy(curr->data.clock_out, now_time);
                printf("\n>>> 滴！下班打卡成功 <<<\n");
                printf("下班打卡时间: %s %s\n", today, now_time);
                write_log(emp->id, "进行了下班打卡");
                save_record_list(*rec_list); // 实时落盘
                return;
            }
        }
        curr = curr->next;
    }

    int days = calculate_days_from(emp->join_date);
    printf("\n>>> 滴！上班打卡成功 <<<\n");
    printf("打卡时间: %s %s\n", today, now_time);
    printf("温馨提示：今天是您入职的第 %d 天，继续加油哦！\n", days);

    AttendRecord new_record;
    sprintf(new_record.record_id, "%s_%s", emp->id, today);
    strcpy(new_record.emp_id, emp->id);
    strcpy(new_record.date, today);
    strcpy(new_record.clock_in, now_time);
    strcpy(new_record.clock_out, "");

    if (strcmp(now_time, "08:30") > 0)
    {
        new_record.status = 1;
        printf("[系统警告] 您今天迟到了！\n");
    }
    else
    {
        new_record.status = 0;
    }

    insert_record(rec_list, new_record);

    char log_msg[100];
    sprintf(log_msg, "进行了上班打卡 (状态: %d)", new_record.status);
    write_log(emp->id, log_msg);
    save_record_list(*rec_list); // 实时落盘
}

// ==========================================
// 带原因备注的请假申请 (附带日期防呆拦截)
// ==========================================
void apply_for_leave(Employee *emp, LeaveNode **leave_list)
{
    LeaveRequest req;
    char timestamp[20];
    char today[15];
    get_current_time(timestamp);
    get_current_date(today);

    sprintf(req.leave_id, "LV_%s_%s", emp->id, timestamp);
    strcpy(req.emp_id, emp->id);

    printf("\n--- 提交请假申请 ---\n");

    req.leave_type = get_int_input("假期类型 (1-事假, 2-年假, 3-病假): ", 1, 3);
    req.days = get_int_input("请假天数 (1-365): ", 1, 365);

    // 利用 strcmp 实现请假日期必须 >= 今天 的业务拦截
    while (1)
    {
        get_date_input("起始日期 (YYYY-MM-DD): ", req.start_date);

        if (strcmp(req.start_date, today) < 0)
        {
            printf("[业务拦截] 无法回到过去请假！起始日期不能早于今天 (%s)，请重新输入。\n", today);
        }
        else
        {
            break;
        }
    }

    get_line_input("请填写请假原因备注: ", req.reason, 100);

    req.status = 0;

    insert_leave(leave_list, req);
    printf("申请已提交，等待部门经理审批。单号: %s\n", req.leave_id);
    write_log(emp->id, "提交了新的请假申请");
    save_leave_list(*leave_list); // 实时落盘
}

// ==========================================
// 新增：查看个人历史请假记录与审批状态
// ==========================================
void view_my_leaves(Employee *emp, LeaveNode *leave_list)
{
    printf("\n--- 我的历史请假申请记录 ---\n");
    printf("%-20s %-12s %-6s %-10s %-10s\n", "请假单号", "起始日期", "天数", "类型", "当前状态");
    printf("------------------------------------------------------------\n");

    int count = 0;
    LeaveNode *curr = leave_list;
    while (curr != NULL)
    {
        if (strcmp(curr->data.emp_id, emp->id) == 0)
        {
            char type_str[10];
            char status_str[15];

            if (curr->data.leave_type == 1)
                strcpy(type_str, "事假");
            else if (curr->data.leave_type == 2)
                strcpy(type_str, "年假");
            else
                strcpy(type_str, "病假");

            if (curr->data.status == 0)
                strcpy(status_str, "待审批");
            else if (curr->data.status == 1)
                strcpy(status_str, "已通过");
            else if (curr->data.status == 2)
                strcpy(status_str, "已驳回");
            else
                strcpy(status_str, "已取消");

            printf("%-20s %-12s %-6d %-10s %-10s\n",
                   curr->data.leave_id, curr->data.start_date, curr->data.days, type_str, status_str);
            count++;
        }
        curr = curr->next;
    }
    if (count == 0)
        printf("您目前没有提交过任何请假申请。\n");
    printf("------------------------------------------------------------\n");
}

Employee *get_emp_by_id_local(EmpNode *head, const char *id)
{
    while (head)
    {
        if (strcmp(head->data.id, id) == 0)
            return &(head->data);
        head = head->next;
    }
    return NULL;
}

// ==========================================
// 经理审批流 (使用中文部门名称鉴权)
// ==========================================
void process_leave_approvals(Employee *manager, EmpNode *emp_list, LeaveNode *leave_list)
{
    printf("\n=== 部门请假审批台 ===\n");
    int count = 0;
    LeaveNode *curr = leave_list;

    while (curr != NULL)
    {
        if (curr->data.status == 0)
        {
            Employee *applicant = get_emp_by_id_local(emp_list, curr->data.emp_id);
            // 确保只审批本部门且不是自己的请假条
            if (applicant && strcmp(applicant->dept_name, manager->dept_name) == 0 && strcmp(applicant->id, manager->id) != 0)
            {
                count++;
                printf("\n[待审单号]: %s\n", curr->data.leave_id);
                printf("申请人: %s | 天数: %d | 类别: %d\n", applicant->name, curr->data.days, curr->data.leave_type);
                printf("起始时间: %s\n", curr->data.start_date);
                printf("请假原因: %s\n", curr->data.reason);

                int action = get_int_input("操作 (1-同意, 2-驳回, 0-暂不处理): ", 0, 2);

                if (action == 1)
                {
                    curr->data.status = 1;
                    printf(">> 已同意\n");
                    write_log(manager->id, "审批同意了请假申请");
                }
                else if (action == 2)
                {
                    curr->data.status = 2;
                    printf(">> 已驳回\n");
                    write_log(manager->id, "审批驳回了请假申请");
                }
            }
        }
        curr = curr->next;
    }
    if (count == 0)
        printf("当前部门无待您审批的请假单。\n");
    else
        save_leave_list(leave_list); // 审批完毕实时落盘
}

// ==========================================
// 经理下钻查询 (使用中文部门名称鉴权)
// ==========================================
void manager_drill_down_query(Employee *manager, EmpNode *emp_list, RecordNode *rec_list)
{
    char keyword[50];
    printf("\n--- 部门人员出勤精准查询 ---\n");
    get_string_input("请输入员工姓名关键词: ", keyword, 50);

    int found = fuzzy_search_employee(emp_list, keyword);

    if (found > 0)
    {
        char target_id[20];
        get_string_input("\n请输入要详细查看的【工号】: ", target_id, 20);

        Employee *target = get_emp_by_id_local(emp_list, target_id);
        if (target && strcmp(target->dept_name, manager->dept_name) == 0)
        {
            char month[10];
            get_month_input("请输入要查询的月份 (如 2026-03): ", month);
            predict_monthly_salary(target, rec_list, month);
        }
        else
        {
            printf("权限不足或工号输入错误：只能查询本部门员工！\n");
        }
    }
}

// ==========================================
// 职员端菜单分发
// ==========================================
void employee_menu(Employee *current_user, EmpNode **emp_list, RecordNode **rec_list, LeaveNode **leave_list)
{
    int choice;
    while (1)
    {
        printf("\n=============================\n");
        printf("    [员工考勤自助平台]    \n");
        printf("=============================\n");
        printf("1. 上下班打卡\n");
        printf("2. 提交请假申请\n");
        printf("3. 查询我的历史请假记录\n");
        printf("4. 我的月度考勤与薪资预测\n");
        printf("5. 修改个人登录密码\n");
        printf("6. 退出登录\n");

        choice = get_int_input("请选择操作: ", 1, 6);

        char month[10];
        switch (choice)
        {
        case 1:
            clock_in_with_greeting(current_user, rec_list);
            break;
        case 2:
            apply_for_leave(current_user, leave_list);
            break;
        case 3:
            view_my_leaves(current_user, *leave_list);
            break;
        case 4:
            get_month_input("请输入查询月份 (YYYY-MM): ", month);
            predict_monthly_salary(current_user, *rec_list, month);
            break;
        case 5:
            change_password(current_user, *emp_list);
            break;
        case 6:
            return;
        }
    }
}

// ==========================================
// 经理端菜单分发
// ==========================================
void manager_menu(Employee *current_user, EmpNode **emp_list, RecordNode **rec_list, LeaveNode **leave_list)
{
    int choice;
    while (1)
    {
        printf("\n=============================\n");
        printf("    [部门经理管理台: %s]    \n", current_user->dept_name);
        printf("=============================\n");
        printf("1. 个人上下班打卡\n");
        printf("2. 提交个人请假申请\n");
        printf("3. 查询我的历史请假记录\n");
        printf("4. 审批部门请假申请\n");
        printf("5. 模糊查询与考勤下钻\n");
        printf("6. 部门考勤红黑榜\n");
        printf("7. 修改个人登录密码\n");
        printf("8. 退出登录\n");

        choice = get_int_input("请选择操作: ", 1, 8);

        char month[10];
        switch (choice)
        {
        case 1:
            clock_in_with_greeting(current_user, rec_list);
            break;
        case 2:
            apply_for_leave(current_user, leave_list);
            break;
        case 3:
            view_my_leaves(current_user, *leave_list);
            break;
        case 4:
            process_leave_approvals(current_user, *emp_list, *leave_list);
            break;
        case 5:
            manager_drill_down_query(current_user, *emp_list, *rec_list);
            break;
        case 6:
            get_month_input("请输入统计月份 (YYYY-MM): ", month);
            sort_and_stat_attendance(*emp_list, *rec_list, current_user->dept_name, month, 1);
            break;
        case 7:
            change_password(current_user, *emp_list);
            break;
        case 8:
            return;
        }
    }
}