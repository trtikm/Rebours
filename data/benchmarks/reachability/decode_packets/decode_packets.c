#include <stdlib.h>
#include <assert.h>

// This code is a bit extended and mainly syntactically corrected version of one
// presented in section 7 of this article accepted to ISSTA 2009:
//      * Precise Pointer Reasoning for Dynamic Test Generation,
//        Elkarablieh, Godefroid, Levin
// note: the original code is in C++. Nevertheless, the only C++ construct outside
//       C code is the use of 'new' instead of 'malloc'.

//extern void* malloc(unsigned int n);
//extern void free(void* p);

int MAX_PACKET;
int PACKET_SIZE;

typedef int elem_type;

elem_type** decode_packets(elem_type* buffer, int* const err_code)
{
    // input format is <byte :number of packets>(<byte : packet#><PACKET SIZE bytes : content of packet#>)∗
    // packet# ranges from 0 to MAXPACKET, each packet contains PACKET SIZE bytes
    // packets can be out of order !

    int i,j;

    elem_type** multi_array = (elem_type**)malloc(MAX_PACKET * sizeof(elem_type*));
    for (i = 0; i < MAX_PACKET; ++i)
        multi_array[i] = (elem_type*)malloc(PACKET_SIZE * sizeof(elem_type));

    for (i = 0; i < MAX_PACKET; ++i)
        for (j = 0; j < PACKET_SIZE; j++)
            multi_array[i][j] = 0;

    // decode packets

    int number_of_packets;
    int packet_id;

    number_of_packets = (int)buffer[0];
    if ((number_of_packets > MAX_PACKET) || (number_of_packets < 0))
    {
        *err_code = 1;
        return multi_array;
    }
    for (int i = 0; i < number_of_packets; i++)
    {
        packet_id = (int)buffer[(i * (PACKET_SIZE + 1)) + 1];
        if ((packet_id >= MAX_PACKET) || (packet_id < 0)) // validate packet_id (*)
        {
            *err_code = 2;
            return multi_array; // abort since input buffer is corrupted
        }
        for (int j = 0; j < PACKET_SIZE; j++)
            multi_array[packet_id][j] = buffer[(i * (PACKET_SIZE + 1)) + j + 2]; // (**)
    }

    // ...

    //if ((number_of_packets < MAX_PACKET) && (multi_array[number_of_packets][0] != 0)) // (***)
    if ((number_of_packets < MAX_PACKET) && (multi_array[number_of_packets][0] != 0) && PACKET_SIZE > 20) // (***)
    {
        *err_code = 3;
        return multi_array;
    }

    // ...

    *err_code = 0;
    return multi_array;
}

// Test driver

int main(int argc, char* argv[])
{
    int N;
    int _MAX_PACKET;
    int _PACKET_SIZE;
    elem_type* A;
    elem_type** B;
    int i;

    if (_MAX_PACKET > 0 && _PACKET_SIZE > 0 && N > 0)
    {
        MAX_PACKET = _MAX_PACKET;
        PACKET_SIZE = _PACKET_SIZE;

        A = (elem_type*)malloc(N * sizeof(elem_type));;
        B = decode_packets(A,&i);
        if (i == 3)
        {
            assert(0);
        }
        for (i = 0; i < MAX_PACKET; ++i)
            free(B[i]);
        free(B);
        free(A);
    }
    return 0;
}
