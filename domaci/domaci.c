#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main ()
{
	FILE *button_file, *switch_file, *led_file;
	char *str;
	size_t num_of_bytes = 6;
	
	unsigned char val_switch[4];
	unsigned char val_button[4];
	int val_led = 0;
	
	int old_button, new_button;
	old_button = 0;
	
	int num_on_switch = 0;

	while(1)
	{

		//Citanje vrednosti tastera
		button_file = fopen("/dev/button", "r");
		if(button_file == NULL)
		{
			puts("Problem pri otvaranju /dev/button");
			return -1;
		}


		str = (char *)malloc(num_of_bytes+1); 
		getline(&str, &num_of_bytes, button_file); 

		if(fclose(button_file))
		{
			puts("Problem pri zatvaranju /dev/button");
			return -1;
		}


		val_button[3] = str[2] - 48;
		val_button[2] = str[3] - 48;
		val_button[1] = str[4] - 48;
		val_button[0] = str[5] - 48;
		free(str);
		
		new_button = 0;
		new_button += val_button[0] + 2*val_button[1] + 4*val_button[2] + 8*val_button[3];
		
		// ------------------------
		
		if(old_button != new_button){
			
			// ------------------------
			
			//Citanje vrednosti tastera
			switch_file = fopen("/dev/switch", "r");
			if(switch_file == NULL)
			{
				puts("Problem pri otvaranju /dev/switch");
				return -1;
			}


			str = (char *)malloc(num_of_bytes+1); 
			getline(&str, &num_of_bytes, switch_file); 

			if(fclose(switch_file))
			{
				puts("Problem pri zatvaranju /dev/switch");
				return -1;
			}


			val_switch[3] = str[2] - 48;
			val_switch[2] = str[3] - 48;
			val_switch[1] = str[4] - 48;
			val_switch[0] = str[5] - 48;
			free(str);
			
			// ------------------------
			
			num_on_switch = 0;
			num_on_switch = val_switch[0] + 2*val_switch[1] + 4*val_switch[2] + 8*val_switch[3];
			
			if(val_button[0] == 1){
				val_led += num_on_switch;				
				if(val_led > 15) val_led = 15;
			}
			
			if(val_button[1] == 1){
				val_led -= num_on_switch;
				if(val_led < 0) val_led = 0;
			}
			
			if(val_button[2] == 1){
				val_led = 2*val_led;
				if(val_led > 15) val_led = 15;
			}
			
			if(val_button[3] == 1){
				val_led = val_led / 2;
			}
			
			
			// ------------------------
			
			led_file = fopen("/dev/led", "w");
			if(led_file == NULL)
			{
				printf("Problem pri otvaranju /dev/led\n");
				return -1;
			}
			
			switch(val_led){
				case 0:
					fputs("0b0000\n", led_file);
					break;
				case 1:
					fputs("0b0001\n", led_file);
					break;
				case 2:
					fputs("0b0010\n", led_file);
					break;
				case 3:
					fputs("0b0011\n", led_file);
					break;
				case 4:
					fputs("0b0100\n", led_file);
					break;
				case 5:
					fputs("0b0101\n", led_file);
					break;
				case 6:
					fputs("0b0110\n", led_file);
					break;
				case 7:
					fputs("0b0111\n", led_file);
					break;
				case 8:
					fputs("0b1000\n", led_file);
					break;
				case 9:
					fputs("0b1001\n", led_file);
					break;
				case 10:
					fputs("0b1010\n", led_file);
					break;
				case 11:
					fputs("0b1011\n", led_file);
					break;
				case 12:
					fputs("0b1100\n", led_file);
					break;
				case 13:
					fputs("0b1101\n", led_file);
					break;
				case 14:
					fputs("0b1110\n", led_file);
					break;
				case 15:
					fputs("0b1111\n", led_file);
					break;
			}
			
			
			if(fclose(led_file))
			{
				printf("Problem pri zatvaranju /dev/led\n");
				return -1;
			}
			
			
			printf("PRITISNUTI TASTER JE: %d\n", new_button);
			printf("STANJE SWITCHA JE: %d\n", num_on_switch);
			printf("ISPIS NA LED JE: %d\n", val_led);
			printf("-----------------------\n\n");
		}
		
		old_button = new_button;
		usleep(10000); // 0.1 sek
	}
}

