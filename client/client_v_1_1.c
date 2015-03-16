#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <time.h>

// structure to hold the fields used by each client
struct info{
	char name[20];
	char email[30];
	char coordenates[40];
	char postal[50];
//	struct info next;
};

// create contents of locations in a list to use as random locations
int create_list(struct info *list, int elements){
	for(int x=0; x < elements; x++){
		memset(&list[x], 0, sizeof(struct info));
	}

	for(int x=0; x < elements; x++){
		strcpy(list[x].name, "Raghu");
		strcpy(list[x].email, "Raghu.lastname@aalto.fi");
	}

	strcpy(list[0].coordenates, "48d 51m 12s N, 2d 20m 55s E");
	strcpy(list[0].postal, "Rue de la baguette 23, Paris 43029, France");

	strcpy(list[1].coordenates, "60d 10m 15s N, 24d 56m 15s E");
	strcpy(list[1].postal, "helsinginkatu 40, Helsinki 00745, Finland");

	strcpy(list[2].coordenates, "18d 58m 39s N, 72d 49m 33s E");
	strcpy(list[2].postal, "Bollywood street 102, Mumbai 32902, India");

	strcpy(list[3].coordenates, "58d 18m 1s N, 134d 25m 2s W");
	strcpy(list[3].postal, "Polarbear street 34, Juneau 00010, Alaska");

	strcpy(list[4].coordenates, "35d 41m 5s N, 139d 41m 4s E");
	strcpy(list[4].postal, "Anime street 290, Tokio 54032, Japan");

	strcpy(list[5].coordenates, "23d 33m 3s S, 46d 38m 7s W");
	strcpy(list[5].postal, "Bikini street 128, Rio de Janeiro 45930, Brasil");

	return 0;
}

// function to build the string to be sent to the server
int build_location(char *locstr, struct info extract, int num_cl, int indic, int option, char *query){
	memset(locstr, 0, strlen(locstr));

	// build the string with a zero as client ID and location
	if(!num_cl){
		strcpy(locstr, "0$");
		strcat(locstr, extract.coordenates);
		strcat(locstr, "\n");
		return 0;
	}
	else{
		// indicator value of zero means the function was called as the initial location advertisement, before user inputs anything
		if(!indic){
//			char temp[10] = "";
			sprintf(locstr, "%d$", num_cl);
//			strcpy(locstr, temp);
			strcat(locstr, extract.coordenates);
			strcat(locstr, "\n");
			return 0;
		}
		// this part is when user requests either a query or an update
		else{										// contruir para update
			// build the query with a name to be resolved
			if(option == 1){
				sprintf(locstr, "%d$", num_cl);
				strcat(locstr, "QUERY$NAME$");
				strcat(locstr, query);
				strcat(locstr, extract.coordenates);
				strcat(locstr, "\n");
				return 0;
			}
			// build the query with an email address to be resolved
			else if(option == 2){
				sprintf(locstr, "%d$", num_cl);
				strcat(locstr, "QUERY$EMAIL$");
				strcat(locstr, query);
				strcat(locstr, extract.coordenates);
				strcat(locstr, "\n");
				return 0;
			}
		}
	}

//	CLIENTID$TYPEOFCOM$SERVICE$<string>$LOCATIONUPDATE\n
//	000001$QUERY$NAME$raghu$10.19.19\n

	return -1;
}

int main(){
	struct sockaddr_in serv;
	memset(&serv, 0, sizeof(serv));
	int check = 0, option = 0, sokk = 0, ran_loc = 0, id = 0, indic = 0;
	char location[101] = "", query[50] = "", /*query2[50] = "",*/ answer[250] = "";
	struct info arr[6];
	FILE *idfile = NULL;
//	struct info my_info_0, my_info_1, my_info_2, my_info_3, my_info_4, my_info_5;

	// create array of different locations
	if(create_list(arr, 6) != 0){
		printf("An error happened while creating the locations\n");
		return -1;
	}

	// pick a location randomly to use as current location
	srand(time(NULL));
	ran_loc = rand() % 6;

	// connect to the server
	sokk = socket(AF_INET, SOCK_STREAM, 0);
	serv.sin_family = AF_INET;
	serv.sin_port = htons(7000);
	inet_pton(AF_INET, "127.0.0.0", &serv.sin_addr);
	check = connect(sokk, (struct sockaddr *) &serv, sizeof(serv)); 
	if(check == -1){
		perror("some error happened when calling connect\n");
		return -1;
	}
	else if(check == 0)
		printf("Connect function returned 0, connection was succssful\n");
	else{
		printf("Connection function returned %d\n", check);
		return -1;
	}

	// check if this client has already an ID assigned
	idfile = fopen("ID_file", "r")
	// no ID assigned, fopens fails because the file doesn't exist
	if(idfile == NULL){
		perror("This client has not yet assigned an ID:");

		// build a string containing the location information
		if(build_location(location, arr[ran_loc], id, indic, 0, NULL) != 0){
			printf("Error while building the string to be sent to the server");
			return -1;
		}

		// send the string to the server containing zero as ID
		if(strlen(location) != write(sokk, location, strlen(location))){
			printf("some error happened, the location information could not be sent properly\n");
			return -1;
		}

		// read from the server what ID it assigns to this client and save it into a file
		if(read(sokk, id, sizeof(int)) == -1){					// modify this in order to decode properly the answer from server
			perror("some error happened at reading the ID sent by the server\n");
			return -1;
		}
		idfile = fopen("ID_file", "w");
		if(idfile == NULL){
			perror("An error happened when creating the ID_file in order to write the newly assigned ID:");
			return -1;
		}
		if(sizeof(id) != write(idfile, id, sizeof(id))){
			printf("The newly assigned ID could not be written into the file\n");
			return -1;
		}
	}
	// this client has already an ID, read it from the file and store it in variable id
	else{
		if(read(idfile, id, sizeof(int)) == -1){
			perror("some error happened at reading the ID stored in the file\n");
			return -1;
		}

		// indicator to specify that an ID has been assigned
		indic++;

		// build a string containing the location information
		if(build_location(location, arr[ran_loc], id, indic, 0, NULL) != 0){
			printf("Error while building the string to be sent to the server");
			return -1;
		}

		// send location information to the server
		if(strlen(location) != write(sokk, location, strlen(location))){
			printf("some error happened, the location information could not be sent properly\n");
			return -1;
		}
	}

	// greet the user and ask what operation s/he wants
	printf("Please select the number from the following options that best suits your request\n");
	printf("1) I have a name and want to get the rest of the information\n");
	printf("2) I have an email address and want to get the rest of the information\n");
	for( ; ; ){
		scanf("%d\n", &option);
		if(option == 1){
			// ask the user to input the information s/he has
			printf("Please write the name from which you want to get information and press enter\n");
			scanf("%s", query);

			// build the query string with name as parameter
			if(build_location(location, arr[ran_loc], id, indic, option, query) != 0){
				printf("Error while creating string for query by name\n");
				return -1;
			}
			break;
		}
		else if(option == 2){
			// ask the user to input the information s/he has
			printf("Please write the email address from which you want to get information and press enter\n");
			scanf("%s", query);

			// build the query string with email as parameter
			if(build_location(location, arr[ran_loc], id, indic, option, query) != 0){
				printf("Error while creating string for query by email\n");
				return -1;
			}
			break;
		}
		else
			printf("The option you provided is invalid, please indicate the desired option by its number (1 or 2)");
	}

	// send the information to the server
	if(strlen(location) != write(sokk, location, strlen(location))){
		printf("some error happened, the query could not be sent to the server\n");
		return -1;
	}

	// read the answer from the server
	if(read(sokk, answer, 249) == -1){
		perror("some error happened at reading the answer of the query\n");
		return -1;
	}

	// display the info of the query to the user
	printf("%s", answer);

	// close everything and exit
	close(sokk);
	return 0;

}

/*	following steps to do

/	create a function to separate the fields from the server answer to then be printed to screen
/	do the update version
/	incluir postal address junto con coordenates en el update location string
/	cambiar conexion TCP por UDP

*/

