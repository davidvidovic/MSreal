#include <stdio.h>

#include <stdlib.h>

#include <unistd.h>



int main(int argc, char *argv[])

{

	FILE *file_alu;

	char *str;

	size_t num_of_bytes = 5;



	int brojevi[20];

	int broj_brojeva;

	char operacije[20];

	int broj_operacija;

	int duzina_argumenta;

	int i;

	int j;

	int k;

	char ret;

	int rez;

	char buff[20];



	char argument[20];

	

	int prio_rezultati[20];

	int broj_prio_rezultata;

	

	//int rezultati[20];

	//int broj_rezultata;

	

	unsigned char flag;



	while(1)

	{

		printf("Unesite izraz: \n");

		scanf("%s", argument);



		//printf("%s\n", argument);



		//if(argc != 2)

		//{

			//printf("Netacan broj argumenata\n");

			//return -1;

		//}

		//else

		//{

			//argument = argv[1];



			ret = argument[0];



			if(ret == 'e')

			{

				printf("Goodbye!\n");

				return 0;

			}



			duzina_argumenta = 0;

			broj_operacija = 0;

			broj_brojeva  = 0;

			i = 0;



			while(ret != '\0')

			{

				ret = argument[duzina_argumenta];

				duzina_argumenta++;

				buff[i++] = ret;



				if(ret == '+' || ret == '-' || ret == '*' || ret == '/')

				{

					operacije[broj_operacija++] = ret;



					buff[i] = '\0';

					brojevi[broj_brojeva++] = atoi(buff);

					i = 0;

				}





				// ret = argument[duzina_argumenta];

			}



			// hvatam zadnji broj

			buff[i] = '\0';

			brojevi[broj_brojeva++] = atoi(buff);

			

			for(i = 0; i < broj_brojeva; i++)

			{

				if(brojevi[i] > 255) 

				{

					printf("Operand van opsega!\n");

					return -1;

				}

			}

			

			printf("\nBrojevi prije prio operacija: ");

			

			for(i = 0; i < broj_brojeva; i++)

			{

				printf("%d ", brojevi[i]);

			}

			

			printf("\n");

			

			printf("\nOperacije prije prio operacija: ");

			

			for(i = 0; i < broj_operacija; i++)

			{

				printf("%c ", operacije[i]);

			}

			

			printf("\n");



			// ****************************************

			// RAD SA ALU

			// U regA cuvamo rezultat

			// u regB cuvamo flag

			// sa regC i regD racunamo

			j = 0;

			rez = 0;

			flag = 0;

			broj_prio_rezultata = 0;

			//broj_rezultata = 0;

			

			printf("\nBrojevi nakon prio operacija: ");

			

			for(i = 0; i < broj_operacija; i++)

			{

				if(operacije[i] == '*' || operacije[i] == '/')

				{

					

					file_alu = fopen("/dev/alu", "w");

					if(file_alu == NULL)
					{
						puts("Problem pri otvaranju /dev/alu");
						return -1;
					}

					fprintf(file_alu, "regC=%d", brojevi[i]);
					
					if(fclose(file_alu))
					{
						puts("Problem pri zatvaranju /dev/alu");
						return -1;
					}
					
					
					file_alu = fopen("/dev/alu", "w");

					if(file_alu == NULL)
					{
						puts("Problem pri otvaranju /dev/alu");
						return -1;
					}

					fprintf(file_alu, "regD=%d", brojevi[i+1]);
					
					if(fclose(file_alu))
					{
						puts("Problem pri zatvaranju /dev/alu");
						return -1;
					}



					if(operacije[i] == '*')

					{
						file_alu = fopen("/dev/alu", "w");

						if(file_alu == NULL)
						{
							puts("Problem pri otvaranju /dev/alu");
							return -1;
						}
						
						fprintf(file_alu, "regC * regD");
						
						if(fclose(file_alu))
						{
							puts("Problem pri zatvaranju /dev/alu");
							return -1;
						}

					}

					

					if(operacije[i] == '/')

					{
						file_alu = fopen("/dev/alu", "w");
						if(file_alu == NULL)
						{
							puts("Problem pri otvaranju /dev/alu");
							return -1;
						}
						
						fprintf(file_alu, "regC / regD");
						
						if(fclose(file_alu))
						{
							puts("Problem pri zatvaranju /dev/alu");
							return -1;
						}
					}
	

					operacije[i] = 'X';	// izvrsena operacija
		

					file_alu = fopen("/dev/alu", "r");

					if(file_alu == NULL)

					{

						puts("Problem pri otvaranju /dev/alu");

						return -1;

					}

					

					str = (char *)malloc(num_of_bytes+1); 

					getline(&str, &num_of_bytes, file_alu); 

					

					if(fclose(file_alu))

					{

						puts("Problem pri zatvaranju /dev/alu");

						return -1;

					}

					i = 0;
					while(str[i] != ' ')
					{
						buff[i] = str[i];
						i++;
					}
					
					buff[i] = '\0';
					
					rez = atoi(buff);

					prio_rezultati[broj_prio_rezultata] = rez;					
					
					if(!flag && str[++i]) flag = str[i];


					j += 2;

				}

				else

				{

					if(i == 0) prio_rezultati[broj_prio_rezultata] = brojevi[j];

					else if(operacije[i-1] != 'X') prio_rezultati[broj_prio_rezultata] = brojevi[j];

					

					j++;

					if(i == broj_operacija - 1) 

					{

						prio_rezultati[broj_prio_rezultata+1] = brojevi[j];

						//printf("\ni=%d broj_operacija=%d j=%d broj_prio_rezultata=%d\n", i, broj_operacija, j, broj_prio_rezultata);

					}

					

				}

				

				//printf("%d ", prio_rezultati[broj_prio_rezultata]);

				broj_prio_rezultata++;

			}

			

			//printf("\ni=%d broj_operacija=%d j=%d broj_prio_rezultata=%d\n", i, broj_operacija, j, broj_prio_rezultata);

			

			for(k = 0; k <= broj_prio_rezultata; k++) printf("%d ", prio_rezultati[k]);

			printf("\n\n");

			

			

			

			// STAVLJAM PRVI BROJ U REGA

			

			file_alu = fopen("/dev/alu", "w");

					if(file_alu == NULL)

					{

						puts("Problem pri otvaranju /dev/alu");

						return -1;

					}



			fprintf(file_alu, "regA=%d", prio_rezultati[0]);

			

			if(fclose(file_alu))

					{

						puts("Problem pri zatvaranju /dev/alu");

						return -1;

					}

			

			k = 1;

			

			for(i = 0; i < broj_operacija; i++)

			{

				if(operacije[i] != 'X')

				{

					file_alu = fopen("/dev/alu", "w");

					if(file_alu == NULL)

					{

						puts("Problem pri otvaranju /dev/alu");

						return -1;

					}

					fprintf(file_alu, "regB=%d", prio_rezultati[k]);
					
					
					if(fclose(file_alu))

					{

						puts("Problem pri zatvaranju /dev/alu");

						return -1;

					}

					k++;

					file_alu = fopen("/dev/alu", "w");

					if(file_alu == NULL)

					{

						puts("Problem pri otvaranju /dev/alu");

						return -1;

					}

					if(operacije[i] == '+') fprintf(file_alu, "regA + regB");
					
					if(fclose(file_alu))

					{

						puts("Problem pri zatvaranju /dev/alu");

						return -1;

					}


					file_alu = fopen("/dev/alu", "w");

					if(file_alu == NULL)

					{

						puts("Problem pri otvaranju /dev/alu");

						return -1;

					}
					if(operacije[i] == '-') fprintf(file_alu, "regA - regB");

					

					if(fclose(file_alu))

					{

						puts("Problem pri zatvaranju /dev/alu");

						return -1;

					}

					

					

					file_alu = fopen("/dev/alu", "r");

					if(file_alu == NULL)

					{

						puts("Problem pri otvaranju /dev/alu");

						return -1;

					}

					

					str = (char *)malloc(num_of_bytes+1); 

					getline(&str, &num_of_bytes, file_alu); 

					

					if(fclose(file_alu))

					{

						puts("Problem pri zatvaranju /dev/alu");

						return -1;

					}

					

					j = 0;
					while(str[j] != ' ')
					{
						buff[j] = str[j];
						j++;
					}
					
					buff[j] = '\0';
					
					rez = atoi(buff);

					if(!flag && str[++j]) flag = str[j];

					

					file_alu = fopen("/dev/alu", "w");

					if(file_alu == NULL)

					{

						puts("Problem pri otvaranju /dev/alu");

						return -1;

					}



					fprintf(file_alu, "regA=%d", rez);

					

					if(fclose(file_alu))

					{

						puts("Problem pri zatvaranju /dev/alu");

						return -1;

					}

				}

			}

			

			

			printf("Rezultat je: %d\nCarry bit je: %c\n\n\n", rez, flag);

			





		//}

	}

}