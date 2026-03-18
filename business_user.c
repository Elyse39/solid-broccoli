#include "attendance.h"

// ==========================================
// 创新点 1：入职第 N 天温馨打卡问候
// ==========================================
void clock_in_with_greeting(Employee *emp, RecordNode **rec_list)
{
    char today[15], now_time[10];
    get_current_date(today);
    get_current_time(now_time);

    int days = calculate_days_from(emp->join_date);
    printf("\n>>> 滴！打卡成功 <<<\n");
    printf("打卡时间: %s %s\n", today, now_time);
    printf("温馨提示：今天是您入职的第 %d 天，继续加油哦！\n", days);

    // 组装打卡记录
    AttendRecord new_record;
    sprintf(new_record.record_id, "%s_%s", emp->id, today);
    strcpy(new_record.emp_id, emp->id);
    strcpy(new_record.date, today);
    strcpy(new_record.clock_in, now_time);
    strcpy(new_record.clock_out, "");

    // 简单状态判定 (假设08:30后算迟到)
    if (strcmp(now_time, "08:30") > 0)
    {
        new_record.status = 1; // 迟到
        printf("[系统警告] 您今天迟到了！\n");
    }
    else
    {
        new_record.status = 0; // 正常
    }

    insert_record(rec_list, new_record);

    char log_msg[100];
    sprintf(log_msg, "进行了日常打卡 (状态: %d)", new_record.status);
    write_log(emp->id, log_msg);
}

// ==========================================
// 创新点 2：带原因备注的请假申请
// ==========================================
void apply_for_leave(Employee *emp, LeaveNode **leave_list)
{
    LeaveRequest req;
    char timestamp[20];
    get_current_time(timestamp);

    // 生成唯一单号: 工号_时分
    sprintf(req.leave_id, "LV_%s_%s", emp->id, timestamp);
    strcpy(req.emp_id, emp->id);

    printf("\n--- 提交请假申请 ---\n");
    printf("假期类型 (1-事假, 2-年假, 3-病假): ");
    scanf("%d", &req.leave_type);
    printf("请假天数: ");
    scanf("%d", &req.days);
    printf("起始日期 (YYYY-MM-DD): ");
    scanf("%s", req.start_date);

    // 清理输入缓冲，读取带空格的原因
    while (getchar() != '\n')
        ;
    printf("请填写请假原因备注: ");
    fgets(req.reason, 100, stdin);
    req.reason[strcspn(req.reason, "\n")] = 0; // 去除换行符

    req.status = 0; // 待审批

    insert_leave(leave_list, req);
    printf("申请已提交，等待部门经理审批。单号: %s\n", req.leave_id);
    write_log(emp->id, "提交了新的请假申请");
}

// ==========================================
// 创新点 3：经理审批流转 (查看详情及原因)
// ==========================================
// 辅助函数：需从全量链表中找人，这里由于多文件拆分，做个简单遍历
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
            // 确保是本部门的员工提交的申请
            if (applicant && strcmp(applicant->dept_id, manager->dept_id) == 0)
            {
                count++;
                printf("\n[待审单号]: %s\n", curr->data.leave_id);
                printf("申请人: %s | 天数: %d | 类别: %d\n", applicant->name, curr->data.days, curr->data.leave_type);
                printf("请假原因: %s\n", curr->data.reason);
                printf("操作 (1-同意, 2-驳回, 0-暂不处理): ");
                int action;
                scanf("%d", &action);

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
        printf("当前部门无待审批的请假单。\n");
}

// ==========================================
// 经理专属：部门员工出勤下钻查询 (先模糊查人，再查记录)
// ==========================================
void manager_drill_down_query(Employee *manager, EmpNode *emp_list, RecordNode *rec_list)
{
    char keyword[50];
    printf("\n--- 部门人员出勤精准查询 ---\n");
    printf("请输入员工姓名关键词: ");
    scanf("%s", keyword);

    // 调用 advanced_ops.c 中的模糊查询引擎
    int found = fuzzy_search_employee(emp_list, keyword);

    if (found > 0)
    {
        char target_id[20];
        printf("\n请输入要详细查看的【工号】: ");
        scanf("%s", target_id);

        Employee *target = get_emp_by_id_local(emp_list, target_id);
        if (target && strcmp(target->dept_id, manager->dept_id) == 0)
        {
            // 调用薪资与考勤预测引擎展示详细数据
            char month[10];
            printf("请输入要查询的月份 (如 2026-03): ");
            scanf("%s", month);
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
        printf("3. 我的月度考勤与薪资预测\n");
        printf("4. 退出登录\n");
        printf("请选择操作: ");
        scanf("%d", &choice);

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
            printf("请输入查询月份 (YYYY-MM): ");
            scanf("%s", month);
            predict_monthly_salary(current_user, *rec_list, month);
            break;
        case 4:
            return;
        default:
            printf("无效输入。\n");
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
        printf("    [部门经理管理台: %s]    \n", current_user->dept_id);
        printf("=============================\n");
        printf("1. 个人打卡\n");
        printf("2. 审批部门请假申请\n");
        printf("3. 模糊查询与考勤下钻\n");
        printf("4. 部门考勤红黑榜 (排榜统计)\n");
        printf("5. 退出登录\n");
        printf("请选择操作: ");
        scanf("%d", &choice);

        char month[10];
        switch (choice)
        {
        case 1:
            clock_in_with_greeting(current_user, rec_list);
            break;
        case 2:
            process_leave_approvals(current_user, *emp_list, *leave_list);
            break;
        case 3:
            manager_drill_down_query(current_user, *emp_list, *rec_list);
            break;
        case 4:
            printf("请输入统计月份 (YYYY-MM): ");
            scanf("%s", month);
            // 传当前经理的 dept_id，限制只排榜本部门
            sort_and_stat_attendance(*emp_list, *rec_list, current_user->dept_id, month, 1);
            break;
        case 5:
            return;
        default:
            printf("无效输入。\n");
        }
    }
}