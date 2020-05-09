#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RR 0
#define SJF 1
#define PQ 2
#define FIFO 3
#define SERVE 4

int lastPid = 0;
int serving = 0; // cpu에서 작업중인 프로세스 pid, 없으면 0
int needSwitching = 1; // 스위칭이 필요하면 1, 첫 작업엔 필요
int switchingTime = 0; // 스위칭 하는데 남은 시간
int timeCycleShare = -1;

/* 스위칭, 생성된 프로세스, 종료된 프로세스 횟수 */
int switchingCount = 0;
int generatedTotal = 0;
int terminatedTotal = 0;

/* SUMMARY 용 */
int totalServiceTime = 0;
int totalIdleTime = 0;
float totalTurnaroundTime = 0;
float maximumTurnaround = 0;
float maximumWaitingTime = 0;
int totalWaitingTime = 0;

typedef struct Process
{
	int pid;
	int state;
	int priority;
	int burstTime;
	int arrivalTime;
	int initBurstTime;

	int age;

	struct Process *pc;
} Process;

typedef struct Queue
{
	Process *front;
	Process *rear;

	int quantum;
	int state;
	int count;
	int preemptive;
} Queue;

int GenerateBurstTime(float, float);
void GeneratePriority(Process *);
void Terminate(Process *);

Process *CreateProcess(int alpha, int estimate)
{
	int pid = ++lastPid;

	Process *new = (Process *)malloc(sizeof(Process));
	new->pc = NULL;
	new->pid = pid;
	new->state = -1;
	new->priority = -1;
	new->burstTime = GenerateBurstTime((float) alpha, (float) estimate);
	new->initBurstTime = new->burstTime;
	new->arrivalTime = timeCycleShare;
	new->age = 0;

	return new;
}

Queue *CreateQueue(void)
{
	Queue *new = (Queue *)malloc(sizeof(Queue));

	new->front = NULL;
	new->rear = NULL;
	new->count = 0;
	new->preemptive = -1;
	new->state = -1;

	return new;
}

int IsEmptyQueue(Queue *queue)
{

	if(queue->count <= 0)
		queue->count = 0;

	return queue->count == 0;
}

void Enqueue_RR_FIFO(Queue *queue, Process *process)
{
	if(process == NULL)
		return;

	process->state = queue->state;
	process->pc = NULL;

	if(process->burstTime == -1)
		printf("BURST TIME ERROR IN FIFO.\n");

	if(IsEmptyQueue(queue))
	{
		queue->front = process;
		queue->rear = process;
		queue->count++;
		return;
	}
	else
	{
		queue->rear->pc = process;
		queue->rear = process;
		queue->count++;
		return;
	}
}

void Enqueue_SJF(Queue *queue, Process *process)
{
	if(process->burstTime == -1)
		printf("BURST TIME ERROR IN SJF.\n");

	if(process == NULL || queue == NULL)
		return;

	process->state = queue->state;
	process->pc = NULL;

	if(IsEmptyQueue(queue))
	{
		queue->front = process;
		queue->rear = process;
		queue->count++;
		return;
	}
	else
	{
		Process *current = queue->front;
		Process *prev = NULL;

		while(current != NULL)
		{
			if(current->burstTime < process->burstTime)
			{
				prev = current;
				current = current->pc;
			}
			else
				break;
		}

		if(prev == NULL)
		{
			process->pc = current;
			queue->front = process;
		}
		else if(current == NULL)
		{
			queue->rear->pc = process;
			queue->rear = process;
		}
		else
		{
			process->pc = current;
			prev->pc = process;
		}
		queue->count++;
	}
}

void Enqueue_PQ(Queue *queue, Process *process)
{
	if(process->priority == -1)
		GeneratePriority(process);

	if(process == NULL || queue == NULL)
		return;

	process->state = queue->state;
	process->pc = NULL;

	if(process->burstTime == -1)
		printf("BURST TIME ERROR IN PQ.\n");

	if(IsEmptyQueue(queue))
	{
		queue->front = process;
		queue->rear = process;
		queue->count++;
		return;
	}
	else
	{
		Process *current = queue->front;
		Process *prev = NULL;

		while(current)
		{
			if(current->priority < process->priority)
			{
				prev = current;
				current = current->pc;
			}
			else
				break;
		}

		if(prev == NULL)
		{
			process->pc = current;
			queue->front = process;
		}
		else if(current == NULL)
		{
			queue->rear->pc = process;
			queue->rear = process;
		}
		else
		{
			process->pc = current;
			prev->pc = process;
		}
		queue->count++;
	}
}

void Enqueue(Queue *queue, Process *process)
{
	if(queue->state == SJF)
		Enqueue_SJF(queue, process);
	else if(queue->state == PQ)
		Enqueue_PQ(queue, process);
	else
		Enqueue_RR_FIFO(queue, process);
}

Process *Dequeue(Queue *queue)
{
	if(IsEmptyQueue(queue))
	{
		return NULL;
	}

	Process *process = queue->front;

	if(process == NULL)
	{
		queue->count = 0;
		return NULL;
	}

	queue->front = process->pc;

	process->pc = NULL;

	queue->count--;

	return process;
}

Process *Dequeue_Process(Queue *queue, Process *prev)
{
	Process *temp;

	if(IsEmptyQueue(queue))
		return NULL;
	
	if(prev != NULL)
	{
		temp = prev->pc;
		prev->pc = temp->pc;
		temp->pc = NULL;
		queue->count--;
	}
	else
	{
		temp = Dequeue(queue);
	}

	return temp;
}

void PrintQueue(Queue *queue) // 코드 완성되면 지울것
{
	Process *process = queue->front;
	
	for(int i = 0; i < queue->count; i++)
	{
		printf("[%d]: %d\n", process->pid, process->burstTime);
		process = process->pc;
	}
}

void PrintQueue2(Queue *queue) // 테스트용 2
{
	Process *process = queue->front;

	for(int i = 0; i< queue->count; i++)
	{
		printf("[%d]: %d\n", process->pid, process->priority);
		process = process->pc;
	}
}

int GenerateBurstTime(float alpha, float estimate)
{

	float coef = rand() / (float) RAND_MAX;

	float result = coef * alpha + (1 - coef) * estimate;

	if(result < 0)
		result *= -1;
	else if(result == 0)
		result = 1;

	if(timeCycleShare != -1)
		printf("Process execution time: %d\n\n", (int) result);

	return (int) result;
}

void GeneratePriority(Process *process)
{
	int random = rand() % 100;

	process->priority = random;

}

/* 프로세스를 서비스하는 함수, burstTime이 줄어듬.     */
/* 만약 0이 되면 needSwitching이 올라가고 Terminate함. */
int Service(Process *process)
{
	if(process == NULL)
	{
		printf("실행될리 없는 구간.\n");
		return 0;
	}

	process->burstTime--;
	totalServiceTime++;

	if(!process->burstTime) // 서비스 종료
	{
		serving = 0;
		needSwitching = 1;
		Terminate(process);

		return 1;
	}

	return 0;
}

/* 큐에서 새로운 프로세스를 선택하는 함수, 없다면 NULL */
/* 선택할 때 serving이 pid가 됨, 마찬가지로 없다면 그대로 */
Process *Select(Queue *queue)
{
	if(!IsEmptyQueue(queue))
	{
		Process *servedProcess = Dequeue(queue);
		if(servedProcess == NULL)
			return NULL;
		serving = servedProcess->pid;
		
		needSwitching = 0;
		return servedProcess;
	}
	else
		return NULL;
}

void Terminate(Process *process)
{
	terminatedTotal++;

	float turnaround = timeCycleShare - process->arrivalTime;
	totalTurnaroundTime += turnaround;

	if(turnaround > maximumTurnaround)
		maximumTurnaround = turnaround;

	float waiting = turnaround - (float) process->initBurstTime;
	totalWaitingTime += waiting;

	if(waiting > maximumWaitingTime)
		maximumWaitingTime = waiting;

	printf("----------\n\nprocess terminated.\n-------------\n\n");

	free(process);
}

/* currentProcess 변경함 */
Process *ContextSwitching(Queue *queue, Process *currentProcess)
{
	if(IsEmptyQueue(queue))
	{
		return currentProcess;
	}
	else
	{
		switchingCount++;
		Process *new = Select(queue);

		if(new == NULL)
			return currentProcess;

		Enqueue(queue, currentProcess);
		
		return new;
	}
}

//각 큐의 aging integer은 aging threshold
void Aging(Queue *queueRR, Queue *queueSJF, Queue *queuePQ, Queue *queueFIFO, int agingSJF, int agingPQ, int agingFIFO)
{
	Process *process = queueSJF->front;
	Process *prev = NULL;
	int count = 0;

	while(process != NULL)
	{
		process->age++;
		if(process->age >= agingSJF)
		{
			
			process->age = 0;

			if(prev == NULL)
			{
				process = Dequeue(queueSJF);

				if(process == NULL)
					break;

				Enqueue_RR_FIFO(queueRR, process);
				process = queueSJF->front;
			}
			else
			{
				process = Dequeue_Process(queueSJF, prev);
				if(process == NULL)
					break;
				
				Enqueue_RR_FIFO(queueRR, process);

				process = prev->pc;
			}
		}
		else
		{
			prev = process;
			process = process->pc;
		}
		if(prev == process)
			break;
	}

	process = queuePQ->front;
	prev = NULL;

	while(process != NULL)
	{
		process->age++;
		if(process->age >= agingPQ)
		{
			
			if(prev == NULL)
			{
				process = Dequeue(queuePQ);

				if(process == NULL)
					break;

				Enqueue_SJF(queueSJF, process);

				process = queuePQ->front;
			}
			else
			{
				process = Dequeue_Process(queuePQ, prev);

				if(process == NULL)
					break;

				Enqueue_SJF(queueSJF, process);

				process = prev->pc;
			}
		}
		else
		{
			prev = process;
			process = process->pc;
		}

		if(prev == process)
			break;
	}

	process = queueFIFO->front;
	prev = NULL;

	while(process != NULL)
	{
		process->age++;
		if(process->age >= agingFIFO)
		{
			
			if(prev == NULL)
			{
				process = Dequeue(queueFIFO);
				if(process == NULL)
					break;

				Enqueue_PQ(queuePQ, process);
				
				process = queuePQ->front;
			}
			else
			{
				process = Dequeue_Process(queueFIFO, prev);
				if(process == NULL)
					break;

				Enqueue_PQ(queuePQ, process);

				process = prev->pc;
			}
		}
		else
		{
			prev = process;
			process = process->pc;
		}

		if(prev == process)
			break;
	}
}

int main(void)
{
	Queue *queueRR = CreateQueue();
	queueRR->state = RR;
	Queue *queueSJF = CreateQueue();
	queueSJF->state = SJF;
	Queue *queuePQ = CreateQueue();
	queuePQ->state = PQ;
	Queue *queueFIFO = CreateQueue();
	queueFIFO->state = FIFO;

	Process *currentProcess = CreateProcess(0, 0);

	int timeCycle = 0;
	int initSwitchingTime = -1;
	int initQuantum = 0;

	while(timeCycle < 1)
	{
		printf("Enter the time cycles: ");
		scanf("%d", &timeCycle);
		if(timeCycle < 1)
			printf("1 이상의 양수로 입력해주십시오.\n");
	}

	while(initQuantum < 1)
	{
		printf("Enter the vaule of time slot for RR: ");
		scanf("%d", &initQuantum);
		if(initQuantum < 1)
			printf("1 이상의 양수로 입력해주십시오.\n");
	}
//	initQuantum = 5;

	while(initSwitchingTime < 0)
	{
		printf("Enter the context switching time: ");
		scanf("%d", &initSwitchingTime);
		if(initSwitchingTime < 0)
			printf("0 이상의 정수로 입력해주십시오.\n");
	}
//	initSwitchingTime = 1;

	queueRR->quantum = initQuantum;

	int preemptionSJF = -1;
	int preemptionPQ = -1;

	while(preemptionSJF != 0 && preemptionSJF != 1)
	{
		printf("SJF with pre-emption(1-yes/0-no): ");
		scanf("%d", &preemptionSJF);

		if(preemptionSJF != 0 && preemptionSJF != 1)
			printf("preemptive 여부는 0과 1만 입력이 가능합니다.\n");
	}

//	preemptionSJF = 0;
	queueSJF->preemptive = preemptionSJF;

	while(preemptionPQ != 0 && preemptionPQ != 1)
	{
		printf("PQ with pre-emption(1-yes/0-no): ");
		scanf("%d", &preemptionPQ);
		if(preemptionPQ != 0 && preemptionPQ != 1)
			printf("preemptive 여부는 0과 1만 입력이 가능합니다.\n");
	}
//	preemptionPQ = 0;
	queuePQ->preemptive = preemptionPQ;

	int alphaRR = 0;
	int alphaSJF = 0;
	int alphaPQ = 0;
	int alphaFIFO = 0;

	while(alphaRR < 1)
	{
		printf("Enter the alpha co-eff for RR: ");
		scanf("%d", &alphaRR);
		
		if(alphaRR < 1)
			printf("alpha 계수값은 1 이상의 양수만 입력가능합니다.\n");
	}

	while(alphaSJF < 1)
	{	
		printf("Enter the alpha co-eff for SJF: ");
		scanf("%d", &alphaSJF);
		if(alphaSJF < 1)
			printf("alpha  계수값은 1이상의 양수만 입력가능합니다.\n");
	}

	while(alphaPQ < 1)
	{
		printf("Enter the alpha co-eff for PQ: ");
		scanf("%d", &alphaPQ);
		if(alphaPQ < 1)
			printf("alpha 계수값은 1이상의 양수만 입력가능합니다.\n");
	}

	while(alphaFIFO < 1)
	{
		printf("Enter the alpha co-eff for FIFO: ");
		scanf("%d", &alphaFIFO);
		if(alphaFIFO < 1)
			printf("alpha 계수값은 1이상의 양수만 입력가능합니다.\n");
	}

	int agingRR = 0;
	int agingSJF = 0;
	int agingPQ = 0;
	int agingFIFO = 0;

	while(agingRR < 1)
	{
		printf("Enter the aging time for RR: ");
		scanf("%d", &agingRR);
		if(agingRR < 1)
			printf("aging time은 1이상의 양수만 입력가능합니다.\n");
	}

	while(agingSJF < 1)
	{
		printf("Enter the aging time for SJF: ");
		scanf("%d", &agingSJF);

		if(agingSJF < 1)
			printf("aging time은 1이상의 양수만 입력가능합니다.\n");
	}

	while(agingPQ < 1)
	{
		printf("Enter the aging time for PQ: ");
		scanf("%d", &agingPQ);

		if(agingPQ < 1)
			printf("aging time은 1이상의 양수만 입력가능합니다.\n");
	}

	while(agingFIFO < 1)
	{
		printf("Enter the aging time for FIFO: ");
		scanf("%d", &agingFIFO);

		if(agingFIFO < 1)
			printf("aging time은 1이상의 양수만 입력가능합니다.\n");
	}

	int estimateRR = 0;
	int estimateSJF = 0;
	int estimatePQ = 0;
	int estimateFIFO = 0;

	while(estimateRR < 1)
	{
		printf("Enter the initial estimated time for RR: ");
		scanf("%d", &estimateRR);

		if(estimateRR < 1)
			printf("예측 시간은 1이상의 양수만 입력가능합니다.\n");
	}
	
	while(estimateSJF < 1)	
	{
		printf("Enter the initial estimated time for SJF: ");
		scanf("%d", &estimateSJF);

		if(estimateSJF < 1)
			printf("예측 시간은 1이상의 양수만 입력가능합니다.\n");
	}

	while(estimatePQ < 1)
	{
		printf("Enter the initial estimated time for PQ: ");
		scanf("%d", &estimatePQ);

		if(estimatePQ < 1)
			printf("예측 시간은 1이상의 양수만 입력가능합니다.\n");
	}

	while(estimateFIFO < 1)
	{
		printf("Enter the initial estimated time for FIFO: ");
		scanf("%d", &estimateFIFO);

		if(estimateFIFO < 1)
			printf("예측 시간은 1이상의 양수만 입력가능합니다.\n");
	}

	srand(time(NULL));

	queueRR->quantum = initQuantum;

	for(int i = 0; i < timeCycle; i++)
	{
		timeCycleShare = i;
		printf("-------------------------------------\n[CYCLE TIME: %d]\n----------------------------------\n\n", i);

		if(queueSJF == NULL)
		{
			queueSJF = CreateQueue();
			queueSJF->state = SJF;
			queueSJF->preemptive = preemptionSJF;
		}

		if(queuePQ == NULL)
		{
			queuePQ = CreateQueue();
			queuePQ->state = PQ;
			queuePQ->preemptive = preemptionPQ;
		}

		if(rand() % 2 == 0 || i == 0) // 50% 프로세스 생성
		{
			generatedTotal++;
			switch(rand() % 4)
			{
				case 0:
					printf("Process appended to RR queue.\n");
					
					Enqueue_RR_FIFO(queueRR, CreateProcess(alphaRR, estimateRR));
					
					break;
				case 1:
					printf("Process appended to SJF queue.\n");
					Enqueue_SJF(queueSJF, CreateProcess(alphaSJF, estimateSJF));
					if(currentProcess == NULL)
						needSwitching = 1;
					else if(currentProcess->state == SJF && currentProcess->burstTime > queueSJF->front->burstTime && preemptionSJF == 0)
						needSwitching = 1;
					
					break;
				case 2:
					printf("Process appended to PQ queue.\n");
					Enqueue_PQ(queuePQ, CreateProcess(alphaPQ, estimatePQ));
					if(currentProcess == NULL)
						needSwitching = 1;
					else if(currentProcess->state == PQ && currentProcess->priority > queuePQ->front->burstTime && preemptionPQ == 0)
						needSwitching = 1;
					
					break;
				case 3:
					printf("Process appended to FIFO queue.\n");
					Enqueue_RR_FIFO(queueFIFO, CreateProcess(alphaFIFO, estimateFIFO));
					
					break;
			}
		}
		else
		{
			printf("no new process was generated.\n\n");
		}

		if(switchingTime > 0) // 스위칭 중이면 작업없이 다음 사이클로
		{
			Aging(queueRR, queueSJF, queuePQ, queueFIFO, agingSJF, agingPQ, agingFIFO);

			totalIdleTime++;

			switchingTime--;
			printf("# of jobs in RR = %d\n\n", queueRR->count);
			printf("# of jobs in SJF = %d\n\n", queueSJF->count);
			printf("# of jobs in PQ = %d\n\n", queuePQ->count);
			printf("# of jobs in FIFO = %d\n\n", queueFIFO->count);

			printf("#context switches = %d\n\n", switchingCount);
			printf("#Total processes generated = %d\n\n", generatedTotal);
			printf("#Total processes completed = %d\n\n", terminatedTotal);
			continue;
		}

		if((serving == 0 && needSwitching == 1) || i == 0) // 작업중이던 프로세스 TERMINATED.
		{

			currentProcess = NULL;

			if(currentProcess == NULL) // 다음 작업할 프로세스 선택
			{
				if(IsEmptyQueue(queueRR))
					if(IsEmptyQueue(queueSJF))
						if(IsEmptyQueue(queuePQ))
							currentProcess = Select(queueFIFO);
						else
							currentProcess = Select(queuePQ);
					else
						currentProcess = Select(queueSJF);
				else
				{
					currentProcess = Select(queueRR);
					queueRR->quantum = initQuantum;
				}
			}

			if(currentProcess) // 프로세스가 선택되었다면
			{
				switchingTime = initSwitchingTime;
			}
			
		}
		else if(needSwitching == 1) // 컨텍스트 스위칭이 필요할때
		{
			switch(currentProcess->state)
			{
				case SJF:
					currentProcess = ContextSwitching(queueSJF, currentProcess);
					break;
				case PQ:
					currentProcess = ContextSwitching(queuePQ, currentProcess);
					break;
			}

			needSwitching = 0;
		}
		else // 그냥 계속 서비스
		{

			if(Service(currentProcess))
			{
				needSwitching = 1;
			}

			if(currentProcess->state == RR)
			{
				queueRR->quantum--;
				if(queueRR->quantum == 0)
				{
					if(IsEmptyQueue(queueRR))
						queueRR->quantum = initQuantum;
					else
						currentProcess = ContextSwitching(queueRR, currentProcess);
				}
			}
		}

		Aging(queueRR, queueSJF, queuePQ, queueFIFO, agingSJF, agingPQ, agingFIFO);

		printf("# of jobs in RR = %d\n\n", queueRR->count);
		printf("# of jobs in SJF = %d\n\n", queueSJF->count);
		printf("# of jobs in PRQ = %d\n\n", queuePQ->count);
		printf("# of jobs in FIFO = %d\n\n", queueFIFO->count);

		printf("#context switches = %d\n\n", switchingCount);
		printf("#Total processes generated = %d\n\n", generatedTotal);
		printf("#Total processes completed = %d\n\n", terminatedTotal);

	}

	printf("--------------------\n\nSUMMARY\n\n--------------------\n\n");
	printf("AVERAGE WAITING TIME: %.4f\n\n", totalWaitingTime / (float) terminatedTotal);
	printf("AVERAGE TURNAROUND TIME: %.4f\n\n", totalTurnaroundTime / (float) terminatedTotal);
	printf("CPU UTILIZATION: %.4f%\n\n", ( (float) (timeCycleShare - totalIdleTime) / (float) timeCycleShare ) * 100);
	printf("MAXIMUM TURNAROUND TIME: %d\n\n", (int) maximumTurnaround);
	printf("MAXIMUM WAIT TIME: %d\n\n", (int) maximumWaitingTime);
	printf("CPU THROUGHPUT: %.4f%\n\n", ((float) terminatedTotal / totalTurnaroundTime) * 100);

	return 0;
}
