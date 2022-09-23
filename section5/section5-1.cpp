#include<stdio.h>
void byteorder()
{
    union {
        short value;
        char union_bytes[ sizeof( short ) ];
    } test;
    test.value = 0x0102;
    for( int i = 0; i < sizeof(short); i++ ) {
        printf("%d", test.union_bytes[ i ]);
    }
    if ( ( test.union_bytes[ 0 ] == 1) && ( test.union_bytes[ 1 ] == 2) ) {
        printf("big endian\n");
    } else if ((test.union_bytes[0] == 2 ) && ( test.union_bytes[ 1 ] == 1) ) {
        printf( "little endian\n" );
    } else {
        printf( "unknow...\n" );
    }
}
int main()
{
    byteorder();
    return 0;
}
