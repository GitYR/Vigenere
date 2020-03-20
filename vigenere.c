#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define BUF 1024
#define MIN(x, y) (((x) < (y)) ? (x) : (y))  

typedef struct alpha
{
	char ch;
	int freq;
}Alpha;

typedef struct pos
{
	char str[BUF];
	Alpha alpha[26];
}Pos;

Alpha alpha[26];
char file[2048]; // 2KB
char plaintext[2048];

typedef struct redundancy
{
	char str[BUF];
	int redun;
	int gcd;
}Red;

Red red[50];

void split(int idx,int len,char ** keybox);

void sort_frequency(Alpha * alpha);

void substring(char * sub,char * sub1,int p,int len);

void lcp(char * sub,char * sub1,char * x);

void get_redun(void);

int get_proper_gcd(void);

int gcd(int a,int b); // to get a gcd value.

void get_key(char * key,Pos * pos,int idx);

void get_plain(char * key,char ** keybox,int keylen,int num);

int main(void)
{
	int keylen,p = 0;
	int i,j,num = 65;
	int fd,answer;
	char string[BUF];
	char sub[BUF],sub1[BUF],x[BUF];
	char ** keybox;
	char * key;
	Pos pos[BUF];
	//char ** position;

	/* initializing */
	memset((Alpha *)alpha,0,26);
	memset((char *)string,0,BUF);
	memset((char *)sub,0,BUF);
	memset((char *)sub1,0,BUF);
	memset((char *)x,0,BUF);
	memset((char *)file,0,2048);
	memset((char *)plaintext,0,2048);

	for(i = 0; i < 26; i++)
	{
		alpha[i].ch = num++;
		alpha[i].freq = 0;
	}

	alpha[0].freq = 8; alpha[1].freq = 2; alpha[2].freq = 4; alpha[3].freq = 3; // ABCD
	alpha[4].freq = 11; alpha[5].freq = 1; alpha[6].freq = 2; alpha[7].freq = 3; // EFGH
	alpha[8].freq = 7; alpha[9].freq = 0; alpha[10].freq = 1; alpha[11].freq = 5; // IJKL
	alpha[12].freq = 3; alpha[13].freq = 6; alpha[14].freq = 7; alpha[15].freq = 3; // MNOP
	alpha[16].freq = 0; alpha[17].freq = 7; alpha[18].freq = 5; alpha[19].freq = 6; // QRST
	alpha[20].freq = 3; alpha[21].freq = 1; alpha[22].freq = 1; alpha[23].freq = 0; // UVWX
	alpha[24].freq = 1; alpha[25].freq = 0;											// YZ

	for(i = 0; i < 50; i++)
	{
		memset((char *)red[i].str,0,BUF);
		red[i].redun = 0;
		red[i].gcd = 0;
	}

	if((fd = open("example.txt",O_RDONLY)) == 0)
	{
		fprintf(stderr,"open error\n");
		exit(1);
	}
	/*
	if((answer = open("plaintext.txt",O_WRONLY | O_CREAT | O_TRUNC,0644)) == 0)
	{
		fprintf(stderr,"open error\n");
		exit(1);
	}
	*/
	while(read(fd,string,BUF) != 0) // find the frequency of input file string
	{
		strcat(file,string);
		memset((char *)string,0,BUF);
	}
	
	sort_frequency(alpha);
	num = strlen(file);

	for(i = 0; i < num; i++)
	{
		for(j = i+1; j < num; j++)
		{
			substring(file,sub,i,num);
			substring(file,sub1,j,num);
			lcp(sub,sub1,x);
			
			strncpy(red[strlen(x)].str,x,strlen(x));
		}
	}

	for(i = 0; i < 50; i++)
		printf("%d : %s\n", i , red[i].str);

	get_redun();
	keylen = get_proper_gcd();
	key = (char *)malloc(sizeof(char)*(keylen+1));
	memset((char *)key,0,keylen+1);
	num = strlen(file) / keylen + 1;

	memset((Pos *)pos,0,sizeof(pos));
	for(i = 0; i < keylen; i++)
	{
		int temp = 65;
		memset((char *)pos[i].str,0,BUF);

		for(j = 0; j < 26; j++)
		{
			pos[i].alpha[j].ch = temp++;
			pos[i].alpha[j].freq = 0;
		}
	}

	keybox = (char **)malloc(sizeof(char *) * num);
	for(i = 0; i < num; i++)
	{
		keybox[i] = (char *)malloc(sizeof(char)*(keylen+1));
		memset((char *)keybox[i],0,keylen+1);
	}

	split(num,keylen,keybox);

	for(i = 0; i < keylen; i++)
	{
		int sum = 0;
		memset((char *)pos[i].str,0,BUF);
	
		for(j = 0; j < num; j++)
		{
			pos[i].str[j] = keybox[j][p];
			if(keybox[j][p] >= 'A' && keybox[j][p] <= 'Z')
				pos[i].alpha[keybox[j][p]-65].freq++;
		}

		for(j = 0; j < num; j++)
			sum += pos[i].alpha[j].freq;

		for(j = 0; j < num; j++) // percent
		{
			double temp = (double)pos[i].alpha[j].freq / sum;
			temp *= 100;
			pos[i].alpha[j].freq = temp;
		}
		sort_frequency(pos[i].alpha);
		p++;
	}

	/* TEST PRINTING */
	
	for(i = 0; i < num; i++)
		printf("keybox[%d] : %s\n", i, keybox[i]);

	for(i = 0; i < 26; i++)
		printf("alpha[%d].ch : %c, alpha[%d].freq : %d\n", i,alpha[i].ch,i,alpha[i].freq);

	for(i = 0; i < keylen; i++)
	{
		printf("pos[%d].str : %s\n", i, pos[i].str);
		for(j = 0; j < 26; j++)
			printf("\tpos[%d].alpha[%d].ch : %c\tpos[%d].alpha[%d].freq : %d\n", i,j,pos[i].alpha[j].ch,i,j,pos[i].alpha[j].freq);
	}

	get_key(key,pos,keylen);
	printf("KEY : %s\n", key);
	get_plain(key,keybox,keylen,num);
	printf("%s\n", plaintext);

	close(fd);
	//close(answer);
	exit(0);
}

void get_key(char * key,Pos * pos,int idx)
{
	int i,j;
	int cipher,plain;

	for(i = 0; i < idx; i++)
	{
		cipher = pos[i].alpha[0].ch;
		plain = cipher - alpha[0].ch;
		plain += 65;
		
		for(j = 1; j < 26; j++)
		{
			if(!(plain >= 'A' && plain <= 'Z'))
			{
				cipher = pos[i].alpha[j].ch;
				plain = cipher - alpha[0].ch;
				plain += 65;
			}
			else
				break;
		}
		key[i] = plain;
	}
}

void get_plain(char * key,char ** keybox,int keylen,int num)
{
	int i,j,len = strlen(file);
	char * p;

	p = (char *)malloc(sizeof(char)*(keylen+1));

	for(i = 0; i < num; i++)
	{
		memset((char *)p,0,keylen+1);

		for(j = 0; j <= keylen && keybox[i][j] != 0; j++)
		{
			p[j] = keybox[i][j] - key[j] + 65;
			if(p[j] < 65)
				p[j] += 26;
		}
		strncat(plaintext,p,strlen(p));
	}
}

void split(int idx,int len,char ** keybox)
{
	int i,j,pos = 0;

	for(i = 0; i < idx; i++)
	{
		for(j = 0; j < len; j++)
			keybox[i][j] = file[pos++];
	}
}

void sort_frequency(Alpha * alpha)
{
	int i,j;
	Alpha temp;

	for(i = 0; i < 26; i++)
	{
		for(j = i+1; j < 26; j++)
		{
			if(alpha[i].freq < alpha[j].freq)
			{
				memcpy((Alpha *)&temp,(Alpha *)&alpha[j],sizeof(Alpha));
				memcpy((Alpha *)&alpha[j],(Alpha *)&alpha[i],sizeof(Alpha));
				memcpy((Alpha *)&alpha[i],(Alpha *)&temp,sizeof(Alpha));
			}
		}
	}
}

void substring(char * sub, char * sub1, int p, int len)
{  
	int c = 0; 

	while (c < len)
	{  
	//	printf("%p %p\n", sub1+c, sub+c);
		sub1[c] = sub[p+c];  
		c++;  
	}  
	sub1[c] = '\0';  
}  

void lcp(char * sub, char * sub1,char * x)
{  
	int n = MIN(strlen(sub),strlen(sub1));  

	for(int i = 0; i < n; i++)
	{  
		if(sub[i] != sub1[i])
		{  
			substring(sub,x,0,i);  
			return;  
		}  
	}  
	substring(sub,x,0,n);  
	return;  
}

void get_redun(void)
{
	int i,j,num,len;
	char * ptr;
	char temp[2048];

	for(i = 0; i < 50; i++)
	{
		int len_set[6] = {0};

		num = 0;
		memset((char *)temp,0,2048);
		strncpy(temp,file,strlen(file));

		if(red[i].str[0] != 0) // if the string is not null
		{
			ptr = strstr(temp,red[i].str);
			while(ptr != NULL)
			{
				int bef = (int)ptr;

				num++;
				ptr++;
				ptr = strstr(ptr,red[i].str);

				if(ptr != NULL)
					len = (int)(ptr - bef);
				else 
					break;

				len_set[num-1] = len;
			}
			len = len_set[0];
			red[i].redun = num;

			for(j = 1; j < num - 2; j++)
				len = gcd(len,len_set[j+1]);
			red[i].gcd = len;
		}
	}
}

int gcd(int a,int b)
{
	int c1,c2;
	int temp;

	if(a > b)
	{
		c1 = a;
		c2 = b;
	}
	else
	{
		c1 = b;
		c2 = a;
	}

	while(c2 != 0)
	{
		temp = c1 % c2;
		c1 = c2;
		c2 = temp;
	}

	return c1;
}

int get_proper_gcd(void)
{
	int i,max,idx;

	max = red[2].redun;
	idx = red[2].gcd;

	for(i = 3; i < 50; i++)
	{
		if(max < red[i].redun)
		{
			max = red[i].redun;
			idx = red[i].gcd;
		}
	}

	return idx;
}
