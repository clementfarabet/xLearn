/***********************************************************
 * libetherflow
 *
 * content: a collection of routines to interface neuFlow
 *          through ethernet.
 *
 * date: July  3, 2010, 1:29PM
 *
 * author: P.Akselrod, C.Farabet
 **********************************************************/
// Open and close an eth socket
int open_socket_C(const char *dev);
int close_socket_C();

// Send/Receive tensors
int send_tensor_double_C(double * data, int size);
int send_tensor_float_C(float * data, int size);
int send_tensor_byte_C(unsigned char * data, int size);
int receive_tensor_double_C(double *data, int size, int height);
int receive_tensor_float_C(float *data, int size, int height);

// Send/Receive frames (strings)
unsigned char * receive_frame_C(int *lengthp);
int send_frame_C(short int length, const unsigned char * data_p);
