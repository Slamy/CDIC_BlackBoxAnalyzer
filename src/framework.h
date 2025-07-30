
extern char cdic_irq_occured;
extern unsigned short int_abuf;
extern unsigned short int_xbuf;
extern unsigned short int_dbuf;
extern unsigned short int_audctl;
extern int timecnt;

/* Used to store register information during a test */
/* We don't want to make any prints during the test as the baud rate is too slow */
extern unsigned long reg_buffer[100][40];
extern int bufpos;

void print(char *format, ...);
void resetcdic();
void print_state();

/* Only enable this, if sound parameter corruption shall be tested */
/* #define SINE_CORRUPTION_TEST */
