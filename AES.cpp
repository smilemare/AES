#include<stdio.h>
typedef unsigned char uint8_t;
#define TRUE  1
#define FALSE 0
#define xtime(x) ((x<<1) ^ (((x>>7)&1)*0x1b))
#define mul(x,y) (((y &1)*x)^((y>>1 & 1)* xtime(x))^ ((y>>2 & 1)* xtime(xtime(x))) ^ \
				 ((y>>3 & 1)* xtime(xtime(xtime(x)))) ^ ((y>>4 & 1)* xtime(xtime(xtime(xtime(x))))))
uint8_t gadd(uint8_t a, uint8_t b);
uint8_t gsub(uint8_t a, uint8_t b);
uint8_t gmul(uint8_t a, uint8_t b);
uint8_t g8_g4(uint8_t a);
uint8_t g4_g8(uint8_t a);
uint8_t g4_inverse(uint8_t a);
uint8_t gmul4(uint8_t a, uint8_t b);

uint8_t gmule(uint8_t m);
uint8_t gsquare(uint8_t m);
uint8_t g8inverse(uint8_t a);

uint8_t aff_trans(uint8_t a);
uint8_t aff_trans_inv(uint8_t a);

uint8_t sbox(uint8_t a, int enc);
uint8_t subBytes(uint8_t a[4][4]);
uint8_t inv_subBytes(uint8_t a[4][4]);
uint8_t shiftRows(uint8_t a[4][4]);
uint8_t inv_shiftRows(uint8_t a[4][4]);
uint8_t mixColumns(uint8_t a[4][4]);
uint8_t inv_mixColumns(uint8_t a[4][4]);
uint8_t addRoundKey(uint8_t a[4][4],uint8_t b[4][4]);
uint8_t keySchedule(uint8_t key[4][4], uint8_t roundKey[10][4][4]);

uint8_t AES(uint8_t text[4][4], uint8_t key[4][4], int enc);


int main()
{
	unsigned char a, b,c;
	
	int i,j,k;

	for(i=0; i<256; i++)
	{
		a = i;
		b= sbox(a, TRUE);
		c= sbox(b, FALSE);
		
		if(a == c)
		{
			//printf("\ndec ok\n");
		}
		
	}
	uint8_t array[4][4]=
	{{0x19,0xa0,0x9a,0xe9},{0x3d,0xf4,0xc6,0xf8},
		{0xe3,0xe2,0x8d,0x48},{0xbe,0x2b,0x2a,0x08}};

	uint8_t key[4][4] ={{0x2b, 0x28, 0xab, 0x09}, {0x7e, 0xae, 0xf7, 0xcf}, 
		{0x15, 0xd2, 0x15, 0x4f}, {0x16, 0xa6, 0x88, 0x3c}};
	uint8_t roundKey[10][4][4];
	printf("Plain:\n");
	for(i =0; i< 4; i++)
	{
		for(j=0; j<4; j++)
		{
			printf("%5x", array[i][j]);
		}
		printf("\n");
	}
	
	
	printf("\nResults of SBox:\n\n");
	subBytes(array);
	for(i =0; i< 4; i++)
	{
		for(j=0; j<4; j++)
		{
			printf("%5x", array[i][j]);
		}
		printf("\n");
	}
	return 0;
}

uint8_t subbytes(uint8_t **a)
{
	int i,j;
	for(i =0; i<4; i++)
	{
		for(j =0; j<4; j++)
		{
			a[i][j]= sbox(a[i][j], TRUE);
		}
	}
	return 0;
}
uint8_t sbox(uint8_t a, int enc)
{
	uint8_t tmp;
	uint8_t result;
	if(enc == TRUE)
	{
		tmp = g8inverse(a);
		result= aff_trans(tmp);
	}
	else
	{
		tmp = aff_trans_inv(a);
		result = g8inverse(tmp);
	}
	//printf("\na = %x, inverse a= %x, result= %x\n", a, tmp, result);
	return result;
}
//G(2^8)域求逆
uint8_t g8inverse(uint8_t a)
{
	uint8_t b;
	unsigned char bh,bl;
	unsigned char bh_square=0, bl_square=0;
	unsigned char tmp;
	unsigned char d, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;

	b=g8_g4(a);
	//printf("g8 to g4:b =%x(%d)\n", b, b);
	bh =(b&0xf0)>>4;
	bl =b&0x0f;
		
		
	bh_square = gsquare(bh);
	bl_square = gsquare(bl);
		

	tmp1= gadd(gmule(bh_square), bl_square);
	tmp2 = gmul4(bl, bh);
	tmp3 = gadd(bl, bh);

	tmp4= gadd(tmp1, tmp2);

	d = g4_inverse(tmp4);

	tmp5= gmul4(bh, d);
	tmp6= gmul4(tmp3, d);
		
	tmp=(tmp5<<4) | tmp6;
		
	//printf("tmp1 : %x, tmp2 %x, tmp3 %x , tmp4 %x ,tmp5 %x , tmp6 %x, d %x\n", tmp1 ,tmp2, tmp3, tmp4, tmp5, tmp6, d);
		
	//printf("b inverse bh:bl %x\n", tmp);
	b= g4_g8(tmp);
	printf("inverse a: %x(%d)\n", b, b);
	
	printf("gmul(%x,%x) = %d(%d)\n", a, b, gmul(a, b), (a*b)%27);

	return b;
}
/*add two numbers in a GF(2^8) finite field*/
uint8_t gadd(uint8_t a, uint8_t b)
{
	return a^b;
}

/*sub two numbers in a GF(2^8) finite field*/
uint8_t gsub(uint8_t a, uint8_t b)
{
	return a^b;
}

/*GF(2^8)有限域下乘法，用于验证GF(2^8)逆元的正确性*/
uint8_t gmul(uint8_t a, uint8_t b)
{
	uint8_t p=0;
	uint8_t counter;
	uint8_t carry;
	for(counter =0; counter <8; counter ++)
	{
		if(b & 1)
		{
			p ^= a;
		}
		//check if a have potential over bit
		carry = (a & 0x80);
		a <<= 1;
		if(carry)
		{
			a ^= 0x1b; //what x^8 is modulo x^8 + x^4 + x^3 + x +1
		}
		b >>= 1;
	}
	return p;
}
/*GF(2^8)有限域下乘以e={1110}函数模块，具体参见ASIC论文*/
uint8_t gmule(uint8_t m)
{
	int a[8];
	uint8_t p=0;
	int tmpA, tmpB;
	int i;

	for(i=0; i< 8;i ++)
	{
		a[i] = (m & (1<<i)) >> i;		
	}
	tmpA = a[0]^a[1];
	tmpB = a[2]^a[3];

	if(a[1] ^ tmpB)
	{
		p |= 0x01;
	}
	if(tmpA)
	{
		p |= 0x02;
	}
	if(tmpA ^ a[2])
	{
		p |= 0x04;
	}
	if(tmpA ^ tmpB)
	{
		p |= 0x08;
	}
	return p;
}
/*GF(2^8)有限域下平方函数模块，具体参见ASIC论文*/
uint8_t gsquare(uint8_t m)
{
	int a[8];
	uint8_t p=0;
	int i;

	for(i=0; i< 8;i ++)
	{
		a[i] = (m & (1<<i)) >> i;		
	}

	if(a[0] ^ a[2])
	{
		p |= 0x01;
	}
	if(a[2])
	{
		p |= 0x02;
	}
	if(a[1] ^ a[3])
	{
		p |= 0x04;
	}
	if(a[3])
	{
		p |= 0x08;
	}

	return p;
}
/*GF(2^4)有限域下乘法，用于验证GF(2^4)逆元的正确性*/
uint8_t gmul4(uint8_t a, uint8_t b)
{
	uint8_t p=0;
	uint8_t counter;
	uint8_t carry;
	for(counter =0; counter <4; counter ++)
	{
		if(b & 1)
		{
			p ^= a;
		}
		//check if a have potential over bit
		carry = (a & 0x08);
		a <<= 1;
		a &=0x0f;//discard the bit over the last 4 ;
		if(carry)
		{
			a ^= 0x03; //what x^4 is modulo x^4 + x +1
		}
		b >>= 1;
	}
	return p;
}
uint8_t mul4(uint8_t a, uint8_t b)
{
	uint8_t p=0;
	uint8_t counter;
	uint8_t carry;
	for(counter =0; counter <8; counter ++)
	{
		if(b & 1)
		{
			p ^= a;
		}
		//check if a have potential over bit
		carry = (a & 0x80);
		a <<= 1;

		if(carry)
		{
			a ^= 0x11; //what x^4 is modulo x^4 +1
		}
		b >>= 1;
	}
	return p;
}
// another way to multipy in GF(2^4)
uint8_t gmul4_2(uint8_t m, uint8_t n)
{
	uint8_t p=0;
	uint8_t i;
	int tmpA;
	int tmpB;
	int a[8];
	int b[8];
	for(i =0; i <8; i ++)
	{
		a[i]=0;
		b[i]=0;
	}

	for(i=0; i< 8;i ++)
	{
		a[i] = (m & (1<<i)) >> i;
		//printf("c[%d] = %d\n", i, a[i]);
	}
	for(i=0; i< 8;i ++)
	{
		b[i] = (n & (1<<i)) >> i;
		//printf("c[%d] = %d\n", i, b[i]);
	}
	tmpA = a[0] ^ a[3];
	tmpB = a[2] ^ a[3];
	if((a[0]&b[0]) ^ (a[3]&b[1]) 
		^ (a[2]&b[2]) ^ (a[1]&b[3]))

	{
		p|= 0x01;
	}
	if((a[1]&b[0]) ^ (tmpA&b[1]) 
		^ (tmpB&b[2]) ^ ((a[1]^a[2])&b[3]))
	{
		p|= 0x02;
	}
	if((a[2]&b[0]) ^ (a[1]&b[1]) 
		^ (tmpA&b[2]) ^ (tmpB&b[3]))
	{
		p|= 0x04;
	}
	if((a[3]&b[0]) ^ (a[2]&b[1]) 
		^ (a[1]&b[2]) ^ (tmpA&b[3]))
	{
		p|= 0x08;
	}
	
	return p;
}
//从GF(2^8)有限域映射到GF(2^4)有限域
uint8_t g8_g4(uint8_t a)
{
	uint8_t b=0;
	uint8_t c[8];
	uint8_t tmpA, tmpB, tmpC;
	int i;
	//printf("\n g8_g4: a= %x(d)\n", a, a);
	for(i=0; i< 8;i ++)
	{
		c[i] = (a & (1<<i)) >> i;
		//printf("c[%d] = %d\n", i, c[i]);
	}
	tmpA = c[1] ^ c[7];
	tmpB = c[5] ^ c[7];
	tmpC = c[4] ^ c[6];
	//printf("A %d B %d c %d\n", tmpA, tmpB, tmpC);

	if( tmpC ^ c[0] ^ c[5])
	{
		b |= 0x01;
	}
	if(c[1] ^ c[2])
	{
		b |= 0x02;
	}
	if( tmpA )
	{
		b |= 0x04;
	}
	if(c[2] ^ c[4])
	{
		b |= 0x08;
	}
	
	if(tmpC ^ c[5])
	{
		b |=0x10;
	}

	if(tmpA ^ tmpC)
	{
		b |=0x20;
	}
	
	
	if(tmpB ^ c[2] ^ c[3])
	{
		b |=0x40;
	}
	

	if(tmpB)
	{
		b |=0x80;
	}

	return b;
}
//从GF(2^4)有限域映射到GF(2^8)有限域
uint8_t g4_g8(uint8_t a)
{
	uint8_t b=0;
	uint8_t l[4];
	uint8_t h[4];
	uint8_t tmpA, tmpB;
	int i;
	//printf("\n g4_g8: %x(%d)\n", a, a);
	for(i=0; i< 8;i ++)
	{
		if(i<4)
		{
			l[i] = (a & (1<<i)) >> i;
			//printf("l[%d] = %d\n", i, l[i]);
		}
		else
		{
			h[i-4]=(a & (1<<i)) >> i;
			//printf("h[%d] = %d\n", i-4, h[i-4]);
		}	
	}
	
	tmpA = l[1] ^ h[3];
	tmpB = h[0] ^ h[1];

	if(l[0] ^ h[0])
	{
		b |=0x01;
	}
	if(tmpB ^ h[3])
	{
		b |= 0x02;
	}
	if( tmpA ^ tmpB )
	{
		b |= 0x04;
	}
	if(tmpB ^ l[1] ^ h[2])
	{
		b |= 0x08;
	}
	
	if(tmpA ^ tmpB ^ l[3])
	{
		b |=0x10;
	}

	if(tmpB ^ l[2])
	{
		b |=0x20;
	}

	if(tmpA ^ l[2] ^ l[3] ^ h[0])
	{
		b |=0x40;
	}

	if(tmpB ^ l[2] ^ h[3])
	{
		b |=0x80;
	}

	return b;
}


//从GF(2^4)有限域内求取GF(2^4)的逆元
uint8_t g4_inverse(uint8_t a)
{
	uint8_t b=0;
	uint8_t c[4];
	uint8_t tmpA;
	int i;
	//printf("\n g4_inverse:  %d\n", a);
	for(i=0; i< 4;i ++)
	{
		c[i] = (a & (1<<i)) >> i;
		//printf("c[%d] = %d\n", i, c[i]);
	}

	tmpA= c[1] ^ c[2] ^ c[3] ^ (c[1]&c[2]&c[3]);
	//printf("tmpA = %d\n", tmpA);

	if(tmpA ^ c[0] ^ (c[0]&c[2])
		^ (c[1]&c[2]) ^ (c[0]&c[1]&c[2]) )
	{
		b |= 0x01;
	}

	if( (c[0]&c[1]) ^ (c[0]&c[2]) ^ (c[1]&c[2])
		^ c[3] ^ (c[1]&c[3])
		^(c[0]&c[1]&c[3]) )
	{
		b |= 0x02;
	}

	if( (c[0]&c[1]) ^ c[2] ^ (c[0]&c[2])
		^ c[3] ^ (c[0]&c[3])
		^(c[0]&c[2]&c[3]) )
	{
		b |= 0x04;
	}

	if(tmpA ^ (c[0]&c[3]) ^ (c[1]&c[3])
		^ (c[2]&c[3]) )
	{
		b |= 0x08;
	}
	
	//printf("%x(%d)\n", b, b);
	return b;
}

//正向仿射变换
uint8_t aff_trans(uint8_t a)
{
	uint8_t b=0;
	uint8_t c[8];
	uint8_t tmpA, tmpB, tmpC, tmpD;
	int i;
	//printf("\n g8_g4: a= %x(d)\n", a, a);
	for(i=0; i< 8;i ++)
	{
		c[i] = (a & (1<<i)) >> i;
		//printf("c[%d] = %d\n", i, c[i]);
	}
	tmpA = c[0] ^ c[1];
	tmpB = c[2] ^ c[3];
	tmpC = c[4] ^ c[5];
	tmpD = c[6] ^ c[7];
	//printf("A %d B %d c %d\n", tmpA, tmpB, tmpC);

	if( (!c[0]) ^ tmpC ^ tmpD)
	{
		b |= 0x01;
	}
	if( (!c[5]) ^ tmpA ^ tmpD)
	{
		b |= 0x02;
	}
	if( c[2] ^ tmpA ^ tmpD )
	{
		b |= 0x04;
	}
	if( c[7] ^ tmpA ^ tmpB)
	{
		b |= 0x08;
	}
	
	if( c[4] ^ tmpA ^ tmpB)
	{
		b |=0x10;
	}

	if( (!c[1]) ^ tmpB ^ tmpC)
	{
		b |=0x20;
	}
	
	
	if((!c[6]) ^ tmpB ^ tmpC)
	{
		b |=0x40;
	}
	

	if(c[3] ^ tmpC ^ tmpD)
	{
		b |=0x80;
	}

	return b;
}

//正向仿射变换
uint8_t aff_trans_inv(uint8_t a)
{
	uint8_t b=0;
	uint8_t c[8];
	uint8_t tmpA, tmpB, tmpC, tmpD;
	int i;
	//printf("\n g8_g4: a= %x(d)\n", a, a);
	for(i=0; i< 8;i ++)
	{
		c[i] = (a & (1<<i)) >> i;
		//printf("c[%d] = %d\n", i, c[i]);
	}
	tmpA = c[0] ^ c[5];
	tmpB = c[1] ^ c[4];
	tmpC = c[2] ^ c[7];
	tmpD = c[3] ^ c[6];
	//printf("A %d B %d c %d\n", tmpA, tmpB, tmpC);

	if( (!c[5]) ^ tmpC )
	{
		b |= 0x01;
	}
	if( c[0] ^ tmpD)
	{
		b |= 0x02;
	}
	if( (!c[7]) ^ tmpB )
	{
		b |= 0x04;
	}
	if( c[2] ^ tmpA )
	{
		b |= 0x08;
	}
	
	if( c[1] ^ tmpD)
	{
		b |=0x10;
	}

	if( c[4] ^ tmpC)
	{
		b |=0x20;
	}
	
	
	if( c[3] ^ tmpA )
	{
		b |=0x40;
	}
	

	if( c[6] ^ tmpB)
	{
		b |=0x80;
	}

	return b;
}

uint8_t subBytes(uint8_t a[4][4])
{
	int i,j;
	for(i =0; i<4;i++)
	{
		for(j =0; j<4; j++)
		{
			a[i][j] = sbox(a[i][j], 1);
		}
	}
	return 0;
}

uint8_t inv_subBytes(uint8_t a[4][4])
{
	int i,j;
	for(i =0; i<4;i++)
	{
		for(j =0; j<4; j++)
		{
			a[i][j] = sbox(a[i][j], 0);
		}
	}
	return 0;
}