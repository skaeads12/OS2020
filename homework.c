#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RR 0
#define SJF 1
#define PQ 2
#define FIFO 3

/* initial queue */
typedef struct Process
{
	int pid;
	int burstTime;

	struct Process *next;
} Process;

/* new state queue and 4 ready queue */
typedef struct Queue
{
	Process *front;
	Process *rear;
	int count;
	int state;

	int isPreemptive;
	int quantum;
} Queue;

void InitQueue(Queue *);
void Enqueue(Queue *, int, int);
int Dequeue(Queue *);
void ServiceProcess(void);
int BurstTimeCal(int, int);
void CreateProcess(Queue *, int, int);
void SelectProcess(void);
void CheckQueue(Queue *);

int lastPid = 0;

void CheckQueue(Queue *queue)
{
	if(queue->state == RR)
	{
		
	}
	else if(queue->state == SJF)
	{
		if(queue->isPreemptive)
			printf("preemptive sjf checked.\n");
		else
			printf("nonpreemptive sjf checked.\n");
	}
	else if(queue->state == PQ)
	{
		if(queue->isPreemptive)
		{
		}

		printf("pq checked.\n");
	}
	else if(queue->state == FIFO)
	{
		printf("fifo checked.\n");
	}
	else
	{
		
	}	
}

void CreateProcess(Queue *queue, int alpha, int estimate)
{
	srand(time(NULL));

	int random = rand() % 2;

	if(random > -1)
	{
		int pid = ++lastPid;

		int burst = BurstTimeCal(alpha, estimate);
		Enqueue(queue, pid, burst);

		printf("Process created. pid: %d\n", pid);
	}
	else
	{
		return;
	}
}

int BurstTimeCal(int alpha, int estimate)
{
	float coef = 0.5;

	float result = coef * (float) alpha + (1 - coef) * (float) estimate;	

	int resultInt = (int) result;

	printf("[bustTimeCal] alpha: %.1f, estimate: %.1f, result: %.3f\n", (float) alpha, (float) estimate, result);

	return (int) result;
}

void InitQueue(Queue *queue)
{
	queue->front = queue->rear = NULL;
	queue->count = 0;
}

int IsEmpty(Queue *queue)
{
	return queue->count == 0;
}

void Enqueue(Queue *queue, int pid, int burstTime)
{
	Process *new = (Process *)malloc(sizeof(Process));
	new->pid = pid;
	new->burstTime = burstTime;
	new->next = NULL;

	printf("enqueued -> pid: %d, burstTime: %d\n", pid, burstTime);

	if(IsEmpty(queue))
	{
		queue->front = new;
	}
	else
	{
		queue->rear->next = new;
	}
	queue->rear = new;
	queue->count++;

	printf("enqueue successed\n");
}

int Dequeue(Queue *queue)
{
	int pid;
	Process *process;

	if(IsEmpty(queue))
		return 0;

	process = queue->front;
	pid = process->pid;
	queue->front = process->next;
	free(process);
	queue->count--;

	printf("dequeued -> pid: %d\n", pid);

	return pid;
}

int main(void)
{
	int timeCycle;
	int quantumRR;
	int contextSwitchingTime;
	
	int preemptionSJF = -1;		//1 - yes
	int preemptionPQ = -1;		//0 - no

	/* alpha: previously measured value. */
	int alphaRR;
	int alphaSJF;
	int alphaPQ;
	int alphaFIFO;

	/* parameter for aging threshold. */
	int agingRR;
	int agingSJF;
	int agingPQ;
	int agingFIFO;

	/* previous estimated values used to estimate the following values. */
	int estimateRR;
	int estimateSJF;
	int estimatePQ;
	int estimateFIFO;

	printf("Enter the time cycles: ");
	scanf("%d", &timeCycle);
/*
	printf("Enter the vaule of time slot for RR: ");
	scanf("%d", &quantumRR);

	printf("Enter the context switching time: ");
	scanf("%d", &contextSwitchingTime);
*/
	/* if it's not 0 or 1, don't accept it. */
	while(preemptionSJF != 0 && preemptionSJF != 1)
	{
		printf("SJF with pre-emption (1-yes/0-no): ");
		scanf("%d", &preemptionSJF);

		if(preemptionSJF != 0 && preemptionSJF != 1)
			printf("Choose from 0 or 1.\n");
	}

	while(preemptionPQ != 0 && preemptionPQ != 1)
	{
		printf("PQ with pre-emption (1-yes/0-no): ");
		scanf("%d", &preemptionPQ);

		if(preemptionPQ != 0 && preemptionPQ != 1)
			printf("Choose from 0 or 1.\n");

	}

	/* alpha coefficient  */
	printf("Enter the alpha co-eff for RR: ");
	scanf("%d", &alphaRR);

	printf("Enter the alpha co-eff for SJF: ");
	scanf("%d", &alphaSJF);

	printf("Enter the alpha co-eff for PQ: ");
	scanf("%d", &alphaPQ);

	printf("Enter the alpha co-eff for FIFO: ");
	scanf("%d", &alphaFIFO);

	/* aging threshold */
/*	printf("Enter the aging time for RR: ");
	scanf("%d", &agingRR);

	printf("Enter the aging time for SJF: ");
	scanf("%d", &agingSJF);

	printf("Enter the aging time for PQ: ");
	scanf("%d", &agingPQ);

	printf("Enter the aging time for FIFO: ");
	scanf("%d", &agingFIFO);
*/
	/* previous estimated values */
	printf("Enter the initial estimated time for RR: ");
	scanf("%d", &estimateRR);

	printf("Enter the initial estimated time for SJF: ");
	scanf("%d", &estimateSJF);

	printf("Enter the initial estimated time for PQ: ");
	scanf("%d", &estimatePQ);

	printf("Enter the initial estimated time for FIFO: ");
	scanf("%d", &estimateFIFO);

	Queue queueRR;
	InitQueue(&queueRR);
	queueRR.quantum = quantumRR;
	queueRR.state = RR;

	Queue queueSJF;
	InitQueue(&queueSJF);
	queueSJF.isPreemptive = preemptionSJF;
	queueSJF.state = SJF;

	Queue queuePQ;
	InitQueue(&queuePQ);
	queuePQ.isPreemptive = preemptionPQ;
	queuePQ.state = PQ;

	Queue queueFIFO;
	InitQueue(&queueFIFO);
	queueFIFO.state = FIFO;

	srand((unsigned int) time(NULL));

	while(timeCycle > 0)
	{
		printf("time cycle: %d\n", timeCycle);		
		int random = rand() % 4;

		switch(random) // The initial process enters a random queue.
		{
			case 0:
				CreateProcess(&queueRR, alphaRR, estimateRR);
				printf("queueRR entered: %d\n", lastPid+1);
				break;
			case 1:
				CreateProcess(&queueSJF, alphaSJF, estimateSJF);
				printf("queueSJF entered: %d\n", lastPid+1);
				break;
			case 2:
				CreateProcess(&queuePQ, alphaPQ, estimatePQ);
				printf("queuePQ entered: %d\n", lastPid+1);
				break;
			case 3:
				CreateProcess(&queueFIFO, alphaFIFO, estimateFIFO);
				printf("queueFIFO entered: %d\n", lastPid+1);
				break;
		}

		CheckQueue(&queueRR);
		CheckQueue(&queueSJF);
		CheckQueue(&queuePQ);
		CheckQueue(&queueFIFO);

		timeCycle--;
	}

	return 0;
}
