// mini HID parser - only for gamepads
#include "usbh_hid_devices.h"

#ifdef DEBUG_HIDPARSER
#include "usbh_hid_parse_test.h"
#include <stdio.h>
#endif 
#include <string.h> // memset


int USBH_ParseHIDReportDesc(USB_Gamepad_descriptor *desc, uint8_t *data) {
	memset(desc, 0, sizeof(USB_Gamepad_descriptor));

	int end=0; // stop parsing ?

	int globals[16];
	int locals[16];

	uint8_t usages[16]; // multiple usages in a collection
	int nb_usages=0; // nb of currently defined usages 

	int collec_depth=0; // used to track depth of collections & stop if we pop the latest
	int bitpos=0; // current bit position in the report.

    while (!end) {
	    if (*data == 0xFE) return -1; // long item unsupported

	    // read item
	  	uint8_t tag  = data[0]>>4;
	  	uint8_t type = data[0]&0x0c;
	  	uint8_t size =  1+ (data[0]&0x03);

	  	int value;
		switch (size) {
			case 2 : value = (int8_t) data[1];break;
			case 3 : value = (int16_t) (data[2]<<8 | data[1]);break;
			default : value = 0; break;
		}
		data+=size;

		#ifdef DEBUG_HIDPARSER
			char *strtype=(char *[]){"main","global","local"}[type/4];
			printf("//  type: %6s, tag: %d, size: %d, data:%d \n",strtype, tag, size, value);
		#endif

		// interpret item
		int nb_rep, size_rep; // number and size of reports per input / output (shortcuts for globals)
		switch (type) {
			case 0 :  // Main 
				switch(tag) {
					case 0x8: // Input
						size_rep = globals[7];
						nb_rep   = globals[9];

						// ** USAGES
						// usage_max defined ? then it's a usage range
						if (nb_usages<=1 && nb_rep>1) { 
							// fill usages per report
							for (int i=nb_usages;i<nb_rep;i++)
								if (i<16)
									usages[i]=usages[0];
							nb_usages=nb_rep;
						} else if (nb_usages==0) {
							nb_usages=1;
							usages[0]=0; // default value for all
						}

						#ifdef DEBUG_HIDPARSER
						if (nb_rep != nb_usages && !locals[2])  
							printf("### ??");
						printf ("----------\n");
						printf(" -- input %x @%d nb=%d size=%d ",value, bitpos, nb_rep,size_rep);
						printf(" usages(%d): [",nb_usages);
						for (int i=0;i<nb_usages;i++) 
							printf("0x%x,",usages[i]);
						printf("] ");
						// input type in value
						if (value&1) printf("(const)");
						printf(value&2?"(array)":"(variable)");
						printf("\n");
						#endif 

						/* here we will recognize some input patterns, such as a button range, 
						 a hat type, analog X 0-255 and fill the descriptors accordingly
						 we only recognize what is completely understood generally.
						*/

						// (non const) buttons ! define by range, as is. 
						if (globals[0]==9 && !(value&1)) { 
							// asserts size_rep==1 && locals[1]==1 && nb_rep == locals[2]-locals[1] ?

							desc->max_button_index = locals[2]-locals[1]<=7?locals[2]-locals[1]:7;
							
							if (locals[2]-locals[1] == 9) {
								// standard snes on 10 buttons ...
								desc->button_bit[0] = 1+bitpos;
								desc->button_bit[1] = 2+bitpos;
								desc->button_bit[2] = 0+bitpos;
								desc->button_bit[3] = 3+bitpos;
								desc->button_bit[4] = 4+bitpos;
								desc->button_bit[5] = 5+bitpos;
								desc->button_bit[6] = 8+bitpos;
								desc->button_bit[7] = 9+bitpos;
							} else {
								for (int i=0;i<desc->max_button_index+1;i++) 
									desc->button_bit[i]=bitpos+i;								
							}


							// should we recognize certain "good" patterns ? (if 10 buttons use 9&10 as start/sel ...)

							bitpos += size_rep*nb_rep;
						} else if (globals[0]==1) {
							// generic desktop usage page (and usage != 0)

							int datatype=0; // defaults to none
							if (size_rep==8 && globals[1]==0 && globals[2]==255) datatype=2; // u8 0:255
							if (size_rep==8 && (globals[1]==-127 || globals[1]==-128) && globals[2]==127) datatype=1; // i8 -127:127

							for (int uid=0;uid<nb_rep;uid++) {
								switch (usages[uid]) {
									case 0x30 :
										desc->analog_X_bit=bitpos;
										desc->analog_type=datatype;
									break;
									case 0x31 : 
										desc->analog_Y_bit=bitpos;
										desc->analog_type=datatype;
									break;
									case 0x39 : // hat switch
										switch(globals[2]+1) { // logical max+1 = nb of positions
											case 8 : 
												desc->dpad_type = 1; 
												desc->dpad_bit = bitpos;
												break;
											case 4 : 
												desc->dpad_type = 3;
												desc->dpad_bit = bitpos;
												break;
										}
									break;
								}
								bitpos += size_rep;
								}
						} else {
							// just skip this 
							bitpos += size_rep;
						}
						break;


					case 0xA: // Collection				
						collec_depth+=1;
						break;

					case 0xC: // End Collection
						collec_depth-=1;
						if (collec_depth==0)
							end=1;
						break;

					// Non-Implemented, just ignore
					case 0x9: // Output
					case 0xB: // Feature
						break;
				}
				
				// reset locals 
				nb_usages=0;
				memset(locals, 0, sizeof(locals));
				memset(usages, 0, sizeof(usages));
			break;
			
			case 4 : // global variable
				globals[tag]=value;
				#ifdef DEBUG_HIDPARSER 
				printf ("G%2d(%-15s)= %d\n",tag,global_types[tag],value);
				#endif 
				break;
			
			case 8 : // local variable
				locals[tag]=value;
				#ifdef DEBUG_HIDPARSER 
				printf ("L%2d(%-15s)=%d\n",tag,local_types[tag],value);
				#endif 
				if (tag==0) usages[nb_usages++]=value;
				break;
		}


	} // while
	desc->vid=1; //  custom / unknown, but nonzero means valid ! (check this _is_ a gamepad ?)
	return 0;
}

#ifdef DEBUG_HIDPARSER
// test
char *dpad_type[]={"none","8-hat","LDRU","4-hat"};
char *analog_type[]={"none","i8 (rest=0)","u8 (rest=127)"};

void print_desc(USB_Gamepad_descriptor *d) 
{
	printf(" descriptor { \n");
	printf("D-pad type %s, at bit %d \n",
		dpad_type[d->dpad_type],
		d->dpad_bit);

	printf("Analog type %s X bit: %d Y bit:%d \n",
		analog_type[d->analog_type],
		d->analog_X_bit,
		d->analog_Y_bit);
	
	printf("%d buttons @ ",d->max_button_index+1);
	for (int i=0;i<d->max_button_index+1;i++)
		printf("%d ",d->button_bit[i]);
	printf ("\n } \n");
}

int main(void) 
{
	USB_Gamepad_descriptor ram_descr;

	for (int i=0;i<6;i++) {
		if (!USBH_ParseHIDReportDesc(&ram_descr,sample_reports[i])) { // pass by value
			print_desc(&ram_descr);
			printf("--------------------------------------------------------------\n");
		}
	}
}
#endif 
