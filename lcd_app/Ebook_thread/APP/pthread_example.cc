#include <stdio.h>
#include <pthread.h>
#define BUFFER_SIZE 16 // ����������
struct prodcons
{
    // ������������ݽṹ
    int buffer[BUFFER_SIZE]; /* ʵ�����ݴ�ŵ�����*/
    pthread_mutex_t lock; /* ������lock ���ڶԻ������Ļ������ */
    int readpos, writepos; /* ��дָ��*/
    pthread_cond_t notempty; /* �������ǿյ��������� */
    pthread_cond_t notfull; /* ������δ������������ */
};
/* ��ʼ���������ṹ */
void init(struct prodcons *b)
{
    pthread_mutex_init(&b->lock, NULL);
    pthread_cond_init(&b->notempty, NULL);
    pthread_cond_init(&b->notfull, NULL);
    b->readpos = 0;
    b->writepos = 0;
}
/* ����Ʒ���뻺����,�����Ǵ���һ������*/
void put(struct prodcons *b, int data)
{
    pthread_mutex_lock(&b->lock);
    /* �ȴ�������δ��*/
    if ((b->writepos + 1) % BUFFER_SIZE == b->readpos)
    {
        pthread_cond_wait(&b->notfull, &b->lock);
    }
    /* д����,���ƶ�ָ�� */
    b->buffer[b->writepos] = data;
    b->writepos++;
    if (b->writepos >= BUFFER_SIZE)
        b->writepos = 0;
    /* ���û������ǿյ���������*/
    pthread_cond_signal(&b->notempty);
    pthread_mutex_unlock(&b->lock);
} 
/* �ӻ�������ȡ������*/
int get(struct prodcons *b)
{
    int data;
    pthread_mutex_lock(&b->lock);
    /* �ȴ��������ǿ�*/
    if (b->writepos == b->readpos)
    {
        pthread_cond_wait(&b->notempty, &b->lock);
    }
    /* ������,�ƶ���ָ��*/
    data = b->buffer[b->readpos];
    b->readpos++;
    if (b->readpos >= BUFFER_SIZE)
        b->readpos = 0;
    /* ���û�����δ������������*/
    pthread_cond_signal(&b->notfull);
    pthread_mutex_unlock(&b->lock);
    return data;
}

/* ����:�������߳̽�1 ��10000 ���������뻺����,��������
   �̴ӻ������л�ȡ����,���߶���ӡ��Ϣ*/
#define OVER ( - 1)
struct prodcons buffer;
void *producer(void *data)
{
    int n;
    for (n = 0; n < 10000; n++)
    {
        printf("%d --->\n", n);
        put(&buffer, n);
    }
	put(&buffer, OVER);
    return NULL;
}

void *consumer(void *data)
{
    int d;
    while (1)
    {
        d = get(&buffer);
        if (d == OVER)
            break;
        printf("--->%d \n", d);
    }
    return NULL;
}

int main(void)
{
    pthread_t th_a, th_b;
    void *retval;
    init(&buffer);
    /* ���������ߺ��������߳�*/
    pthread_create(&th_a, NULL, producer, 0);
    pthread_create(&th_b, NULL, consumer, 0);
    /* �ȴ������߳̽���*/
    pthread_join(th_a, &retval);
    pthread_join(th_b, &retval);
    return 0;
}