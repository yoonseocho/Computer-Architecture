#include <stdio.h>
#include <string.h> // strcmp()
#include <stdlib.h> // strtol() - 진법으로 표기된 문자열을 정수(long)로 변환, atoi() - 문자스트링을 정수로 변환. EXIT_FAILURE

#define NUMBER_OF_REGISTERS 10
#define MAX_INSTRUCTIONS 100

int registers[NUMBER_OF_REGISTERS] = {
    0,
};
char inst_reg[1024];

typedef enum
{
    ADD,
    SUB,
    MUL,
    DIV,
    MOV,
    HLT,
    CMP,
    JMP,
    BEQ,
    INVALID
} Opcode;

typedef struct
{
    Opcode opcode;
    char operand1[10];
    char operand2[10];
} Instruction;

Instruction instructions[MAX_INSTRUCTIONS];
int pc = 1;
int jmp_flag = 0; // JMP 명령어가 실행되면 1, 아니면 0

Opcode get_opcode(const char *op);
int load_instructions(const char *filename);
void execute_instruction(Instruction instruction);
void execute_instructions(int num_instructions);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int num_instructions = load_instructions(argv[1]);
    if (num_instructions > 0)
    {
        execute_instructions(num_instructions);
    }

    return 0;
}

// 연산자 문자열을 Opcode로 변환
Opcode get_opcode(const char *op)
{
    if (strcmp(op, "+") == 0)
        return ADD;
    if (strcmp(op, "-") == 0)
        return SUB;
    if (strcmp(op, "*") == 0)
        return MUL;
    if (strcmp(op, "/") == 0)
        return DIV;
    if (strcmp(op, "M") == 0)
        return MOV;
    if (strcmp(op, "HLT") == 0)
        return HLT;
    if (strcmp(op, "C") == 0)
        return CMP;
    if (strcmp(op, "J") == 0)
        return JMP;
    if (strcmp(op, "BEQ") == 0)
        return BEQ;
    return INVALID;
}

int load_instructions(const char *filename)
{
    FILE *fp;
    fp = fopen(filename, "r");
    if (!fp)
    {
        printf("Error: Fail to open file\n");
        exit(EXIT_FAILURE);
    }

    int count = 0;
    char opcode_str[10];

    // input파일의 입력값이 올바른 때만 계산
    while (fgets(inst_reg, sizeof(inst_reg), fp) != NULL)
    {
        // 순수한 문자열 데이터만 다루기 위해 줄바꿈 문자(개행) 제거하기
        int len = strlen(inst_reg);
        if (inst_reg[len - 1] == '\n')
        {
            inst_reg[len - 1] = '\0';
        }

        // 명령어 파싱하여 instructions배열에 저장
        int num_args = sscanf(inst_reg, "%s %s %s", opcode_str, instructions[count].operand1, instructions[count].operand2);

        if (num_args == 1 && strcmp(opcode_str, "HLT") == 0)
        {
            instructions[count].opcode = get_opcode(opcode_str);
            instructions[count].operand1[0] = '\0';
            instructions[count].operand2[0] = '\0';
            count++;
        }
        else if ((num_args == 2 && strcmp(opcode_str, "J") == 0) || (num_args == 2 && strcmp(opcode_str, "BEQ") == 0))
        {
            instructions[count].opcode = get_opcode(opcode_str);
            instructions[count].operand2[0] = '\0';
            count++;
        }
        else if (num_args == 3)
        {
            instructions[count].opcode = get_opcode(opcode_str);
            count++;
        }
        else
        {
            printf("Error: Invaild instruction format");
            exit(EXIT_FAILURE);
        }
    }

    fclose(fp);
    return count;
}

void execute_instruction(Instruction instruction)
{
    jmp_flag = 0;

    // 문자열로 표현된 operand1, operand2를 모두 숫자로 변환하기
    int op1 = 0;
    int op2 = 0;

    // op1이 레지스터 참조인지 확인 후 op1 값 설정
    if (instruction.operand1[0] == 'R')
    {
        op1 = registers[instruction.operand1[1] - '0']; // operand[1] = '2'이기 때문에 숫자2로 바꾸기 위해선 '0'
    }
    else if (instruction.operand1[0] == '0' && instruction.operand1[1] == 'x')
    { // 16진수 리터럴 값인 경우
        op1 = (int)strtol(instruction.operand1, NULL, 16);
    }

    // op2에 대해서도 동일한 처리
    // MOV 명령어의 경우 op2가 대상 레지스터를 의미할 수 있으므로, 사용 방식에 주의
    if (instruction.operand2[0] == 'R')
    {
        op2 = registers[instruction.operand2[1] - '0'];
    }
    else if (instruction.operand2[0] == '0' && instruction.operand2[1] == 'x')
    { // 16진수 리터럴 값인 경우
        op2 = (int)strtol(instruction.operand2, NULL, 16);
    }

    switch (instruction.opcode)
    {
    case ADD:
        registers[0] = op1 + op2;
        printf("R0: %d = %d+%d\n", registers[0], op1, op2);
        break;
    case SUB:
        registers[0] = op1 - op2;
        printf("R0: %d = %d-%d\n", registers[0], op1, op2);
        break;
    case MUL:
        registers[0] = op1 * op2;
        printf("R0: %d = %d*%d\n", registers[0], op1, op2);
        break;
    case DIV:
        if (op2 == 0)
        {
            printf("Error: Division by zero.\n");
            return;
        }
        registers[0] = op1 / op2;
        printf("R0: %d = %d/%d\n", registers[0], op1, op2);
        break;
    case MOV:
        // op2가 레지스터 참조인 경우, op1 값을 해당 레지스터에 저장
        if (instruction.operand2[0] == 'R')
        {
            registers[instruction.operand1[1] - '0'] = op2;
            printf("R%d: %d\n", instruction.operand1[1] - '0', op2);
        }
        else if (instruction.operand2[0] == '0' && instruction.operand2[1] == 'x')
        {
            registers[instruction.operand1[1] - '0'] = op2;
            printf("R%d: %d\n", instruction.operand1[1] - '0', op2);
        }
        break;
    case HLT:
        printf("Program halted.\n");
        exit(0);
    case CMP:
        if (op1 == op2)
        {
            registers[0] = 0;
            printf("R0: 0\n");
        }
        else if (op1 > op2)
        {
            registers[0] = 1;
            printf("R0: +1\n");
        }
        else
        {
            registers[0] = -1;
            printf("R0: -1\n");
        }
        break;
    case JMP:
        pc = (int)strtol(instruction.operand1, NULL, 16); // 입력된 줄 번호로 pc 설정, 16진법으로 표기된 문자열을 정수로 변환
        // 교수님왈: strtol은 해줄건데 atoi는 16진수는 안해줄거야.
        printf("Jump to line %d\n", pc);
        jmp_flag = 1;
        break;
    case BEQ:
        if (registers[0] == 0)
        {
            pc = (int)strtol(instruction.operand1, NULL, 16);
            printf("BEQ: Jump to line %d\n", pc);
            jmp_flag = 1;
        }
        else
        {
            printf("R0 is not 0. Continue to the next instruction\n");
        }
        break;
    default:
        printf("Unknown operation.\n");
    }
}

void execute_instructions(int num_instructions)
{
    pc = 1;
    while (pc <= num_instructions)
    {
        printf("------current pc: %d\n", pc);
        execute_instruction(instructions[pc - 1]);

        if (jmp_flag != 1)
        {
            pc++;
        }
        printf("\n");
    }
}