#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h> 
#pragma warning(disable : 4996)
char publisher_table[] = "pb_fl.bin";
char publisher_index[] = "pb_ind.bin";
char game_table[] = "game_fl.bin";
#define MAX_REC 50

typedef struct Index {
	int ind;
	int num;
} Index;

typedef struct DataBase {
	int pub_count;
	int pub_table_count;
	int game_count;
	Index index[MAX_REC];
} DataBase;

typedef struct Publisher {
	int is_exist;
	int id;
	int creation_year;
	char company_name[20];
	int first_game;
} Publisher;

typedef struct Game {
	int is_exist;
	int id;
	char title[20];
	int price;
	int next_game;
} Game;



Publisher input_pub() {
	Publisher new_pub;
	new_pub.is_exist = 1;
	printf("Enter Publisher ID\n");
	scanf("%d", &(new_pub.id));
	printf("Enter company name\n");
	scanf("%19s", &(new_pub.company_name));
	printf("Enter creation year\n");
	scanf("%d", &(new_pub.creation_year));
	new_pub.first_game = -1;
	/*printf("Enter Publisher ID\n");
	scanf("%d", &pub_id);*/
	return new_pub;
	
}

Game input_game() {
	Game new_game;
	new_game.is_exist = 1;
	printf("Enter Game ID\n");
	scanf("%d", &(new_game.id));
	printf("Enter Game title\n");
	scanf("%19s", &(new_game.title));
	printf("Enter Game price\n");
	scanf("%d", &(new_game.price));
	/*printf("Enter Publisher ID\n");
	scanf("%d", &pub_id);*/
	return new_game;
}


void output_pub(Publisher p) {
	printf("%d %d %s %d \n", p.id, p.creation_year, p.company_name, p.first_game);
}

void output_game(Game g) {
	printf("%d %s %d %d \n", g.id, g.title, g.price, g.next_game);
}


void load_ind(DataBase* db) {   //Loading index table to the array
	FILE* fl;
	fl = fopen(publisher_index, "rb");
	Index next;
	int i = 0;
	while (!feof(fl)) {
		fread(&db->index[i], sizeof(Index), 1, fl);
		if (feof(fl)) break;
		i++;
	}
	db->pub_count = i;
	fclose(fl);
}

void write_ind(DataBase* db) { // Writing index table from RAM to the binary file
	FILE* fl;
	fl = fopen(publisher_index, "wb+");
	Index* beg = &db->index[0];
	fwrite(beg, sizeof(Index), db->pub_count , fl);
	fclose(fl);
}


void init(DataBase *db) {
	for (int i = 0; i < 20; i++) {
		db->index[i].ind = -1;
	}
	db->pub_count = 0;
	db->game_count = 0;
	load_ind(db);
	FILE* fl;
	fl = fopen(game_table, "rb");
	fseek(fl, 0L, SEEK_END);
	db->game_count = ftell(fl) / sizeof(Game);
	fclose(fl);
}

bool insert_ind(DataBase *db, int id) {
	int i = 0;
	FILE* fl;
	fl = fopen(publisher_table, "rb");
	fseek(fl, 0L, SEEK_END);
	int row = ftell(fl) / sizeof(Publisher);
	Index new_pub = { id, row + 1 };   
	while (db->index[i].ind != -1 ) { //Finding place to insert
		if (db->index[i].ind == id){
			return 0;
		}
		if (db->index[i].ind > id) {
			break;
		}
		i++;
	}
	do {  //Insertion of the new element to the index table
		Index temp = db->index[i];
		db->index[i] = new_pub;
		new_pub = temp;
		i++;
	} while (new_pub.ind != -1);
	db->pub_count++;
	fclose(fl);
	write_ind(db);
	return 1;
}


void ut_m(DataBase* db) {
	FILE* publishers;
	Publisher curr_publ;
	publishers = fopen(publisher_table, "rb");
	while (!feof(publishers)) {
		fread(&curr_publ, sizeof(Publisher), 1, publishers);
		if (feof(publishers)) break;
		if (curr_publ.is_exist == 1) {
			output_pub(curr_publ);
		}
	}
}

void ut_s(DataBase* db) {
	FILE* games;
	Game curr_game;
	games = fopen(game_table, "rb");
	while (!feof(games)) {
		fread(&curr_game, sizeof(Game), 1, games);
		if (feof(games)) break;
		if (curr_game.is_exist == 1) {
			output_game(curr_game);
		}
	}
}


int get_row(int id, DataBase *db){  //Getting row from the index table using publisher's id with binary search
	int low, high, middle;
	low = 0;
	high = db->pub_count-1;
	while (low <= high)
	{
		middle = (low + high) / 2;
		if (id < db->index[middle].ind)
			high = middle - 1;
		else if (id > db->index[middle].ind)
			low = middle + 1;
		else
			return db->index[middle].num;
	}
	return -1;
}


void append_s(DataBase* db, int prev_game, FILE* games) {  // Adding a game to the publisher's games list
	Game temp;
	fseek(games, (prev_game - 1) * sizeof(Game), SEEK_SET);
	fread(&temp, sizeof(Game), 1, games);
	if (temp.next_game == -1) {
		temp.next_game = db->game_count;
		fseek(games, (prev_game - 1) * sizeof(Game), SEEK_SET);
		fwrite(&temp, sizeof(Game), 1, games);
		fclose(games);
	}
	else {
		append_s(db, temp.next_game, games);
	}
}

int append_m(DataBase* db, int id, Game *new_game) { //Linking a game to a publisher
	int row = get_row(id, db);
	if (row == -1) {
		return -1;
	}
	else {
		db->game_count++;
		new_game->next_game = -1;
		FILE* fl;
		Publisher temp;
		fl = fopen(publisher_table, "rb+");
		fseek(fl, (row - 1) * sizeof(Publisher), SEEK_SET);
		fread(&temp, sizeof(Publisher), 1, fl);
		if (temp.first_game == -1) {
			temp.first_game = db->game_count;
			fseek(fl, (row - 1) * sizeof(Publisher), SEEK_SET);
			fwrite(&temp, sizeof(Publisher), 1, fl);
		}
		else {
			FILE* games;
			games = fopen(game_table, "rb+");
			append_s(db, temp.first_game, games);
		}
		fclose(fl);
		return 0;
	}
}


Game* find_s(int curr_row, int game_id, FILE* games) { //Recursively search for a game in the publisher's game list
	if (curr_row == -1) {
		return NULL;
	}
	Game curr_game;
	fseek(games, (curr_row - 1) * sizeof(Game), SEEK_SET);
	fread(&curr_game, sizeof(Game), 1, games);
	if (game_id == curr_game.id) {
		Game* new_game = malloc(sizeof(Game));
		*new_game = curr_game;
		return new_game;
	}
	find_s(curr_game.next_game, game_id, games);
}

Publisher* get_pub(int row) {
	if (row == -1) {
		printf("Publisher not found\n");
		return NULL;
	}
	FILE* fl;
	fl = fopen(publisher_table, "rb");
	fseek(fl, (row - 1) * sizeof(Publisher), SEEK_SET);
	Publisher* found = malloc(sizeof(Publisher));
	fread(found, sizeof(Publisher), 1, fl);
	fclose(fl);
	return found;
}

Publisher* get_m(DataBase* db, int id) {
	return get_pub(get_row(id, db));
}

Game* get_s(DataBase* db, int pub_id, int game_id) {
	Publisher* pub = get_m(db, pub_id);
	if (pub == NULL) return NULL;
	else {
		FILE* games;
		games = fopen(game_table, "rb+");
		Game* found = find_s(pub->first_game, game_id, games);
		fclose(games);
		if (found == NULL) { return NULL; }
		else return found;
	}
}


void update_m(DataBase* db, int id) {
	int row = get_row(id, db);
	if (row == -1) {
		printf("Publisher not found\n");
		return;
	}
	Publisher* curr = get_pub(row);
	printf("Enter new name\n");
	scanf("%19s", curr->company_name);
	printf("Enter new creation year\n");
	scanf("%d", &(curr->creation_year));
	FILE* publishers;
	publishers = fopen(publisher_table, "rb+");
	fseek(publishers, sizeof(Publisher) * (row - 1), SEEK_SET);
	fwrite(curr, sizeof(Publisher), 1, publishers);
	fclose(publishers);
	free(curr);
}

void update_s(DataBase* db, int game_id, int pub_id) {
	Publisher* pub = get_m(db, pub_id);
	if (pub == NULL) return;
	else {
		FILE* games;
		games = fopen(game_table, "rb+");
		Game* found = find_s(pub->first_game, game_id, games);
		if (found != NULL) {
			printf("Enter New title\n");
			scanf("%19s", &(found->title));
			printf("Enter New price\n");
			scanf("%d", &(found->price));
			int row = (ftell(games) / sizeof(Game)) - 1;
			fseek(games, sizeof(Game) * row, SEEK_SET);
			fwrite(found, sizeof(Game), 1, games);
		}
		fclose(games);
		free(found);
	}
}


void insert_m(DataBase *db, Publisher new_pub) {
	if (db->pub_count >= MAX_REC) {
		printf("Maximum number of records reached!\n");
		return;
	}
	
	if (!insert_ind(db, new_pub.id)) {
		printf("Publisher with this ID is already created!\n");
		return;
	}
	FILE* fl;
	fl = fopen(publisher_table, "ab+");
	fwrite(&new_pub, sizeof(Publisher), 1, fl);
	fclose(fl);
}

void insert_s(DataBase* db, Game to_insert) {
	Publisher curr_pub;
	int pub_id;
	printf("Enter Publisher ID\n");
	scanf("%d", &pub_id);
	if (append_m(db, pub_id, &to_insert) != -1) {
		FILE* games;
		games = fopen(game_table, "ab");
		fwrite(&to_insert, sizeof(Game), 1, games);
		fclose(games);
	}
}


void casscade_delete(DataBase* db, int curr_row) { //Deleting all games linked to the publisher
	if (curr_row == -1) return;
	FILE* games = fopen(game_table, "rb+");
	fseek(games, (curr_row - 1) * sizeof(Game), SEEK_SET);
	Game to_delete;
	fread(&to_delete, sizeof(Game), 1, games);
	to_delete.is_exist = 0;
	fseek(games, -1* sizeof(Game), SEEK_CUR);
	fwrite(&to_delete, sizeof(Game), 1, games);
	fclose(games);
	casscade_delete(db, to_delete.next_game);
}

void delete_m(DataBase* db, int id) {
	int row = -1;
	int low, high, middle;
	low = 0;
	high = db->pub_count;
	while (low <= high){
			middle = (low + high) / 2;
			if (id < db->index[middle].ind)
				high = middle - 1;
			else if (id > db->index[middle].ind)
				low = middle + 1;
			else {
				row = db->index[middle].num;
				
				while (middle != db->pub_count-1) {
					db->index[middle] = db->index[middle + 1];
					middle++;
				}

				db->index[db->pub_count - 1].ind = -1;
				db->pub_count--;
			}
	}
	if (row == -1) {
		printf("Publisher not found!\n"); 
		return;
	}
	write_ind(db);
	Publisher* to_delete = get_pub(row);
	to_delete->is_exist = 0;
	FILE* publishers;
	publishers = fopen(publisher_table, "rb+");
	fseek(publishers, sizeof(Publisher)*(row-1), SEEK_SET);
	fwrite(to_delete, sizeof(Publisher), 1, publishers);
	fclose(publishers);
	casscade_delete(db, to_delete->first_game);
	free(to_delete);
	printf("Removal successful\n");
}

void delete_s(DataBase* db,int pub_id, int game_id){
	FILE* games = fopen(game_table, "rb+");
	Publisher* pub = get_m(db, pub_id);

	if (pub == NULL) {
		printf("Publisher not found\n");
		return;
	}

	int prev_pos = -1;
	int curr_pos = pub->first_game;

	Game curr, prev;

	while (curr_pos != -1) {
		fseek(games, sizeof(Game) * (curr_pos - 1), SEEK_SET);
		fread(&curr, sizeof(Game), 1, games);
		if (curr.id == game_id) {
			curr.is_exist = 0;
			fseek(games, sizeof(Game)*-1, SEEK_CUR);
			fwrite(&curr, sizeof(Game), 1, games);
			if (prev_pos == -1) {
				FILE* publisher = fopen(publisher_table, "rb+");
				pub->first_game = curr.next_game;
				fseek(publisher, (get_row(pub_id, db) - 1) * sizeof(Publisher), SEEK_SET);
				fwrite(pub, sizeof(Publisher), 1, publisher);
				fclose(publisher);
				free(pub);
			}
			else {
				fseek(games, sizeof(Game) * (prev_pos - 1), SEEK_SET);
				prev.next_game = curr.next_game;
				fwrite(&prev, sizeof(Game), 1, games);
			}
			fclose(games);
			printf("Success");
			return;
		}
		prev_pos = curr_pos;
		curr_pos = curr.next_game;
		prev = curr;
	}
	fclose(games);
	printf("Not found!");
	return;
}


int count_pub(DataBase* db) {
	return db->pub_count;
}

int count_games(DataBase* db) {
	FILE* games;
	Game curr_game;
	int count = 0;
	games = fopen(game_table, "rb");
	while (!feof(games)) {
		fread(&curr_game, sizeof(Game), 1, games);
		if (feof(games)) break;
		if (curr_game.is_exist == 1) {
			count++;
		}
	}
	fclose(games);
	return count;
}

int count_s(DataBase* db, int pub_id) {
	Publisher* pub = get_m(db, pub_id);
	if (pub->first_game == -1) return 0;
	FILE* games = fopen(game_table, "rb+");
	fseek(games, sizeof(Game) * (pub->first_game - 1), SEEK_SET);
	Game curr;
	fread(&curr, sizeof(Game), 1, games);
	int count = 1;
	while (curr.next_game != -1) {
		count++;
		fseek(games, sizeof(Game) * (curr.next_game - 1), SEEK_SET);
		fread(&curr, sizeof(Game), 1, games);
	}
	fclose(games);
	free(pub);
	return count;
}

int main() {
	DataBase* db = malloc(sizeof(DataBase)) ;
	init(db);
	int task;
	do {
		printf("Select comand:\n1 - insert-m\n2 - insert-s\n3 - get-m\n4 - get-s\n5 - update-m\n6 - update-s\n7 - delete-m \n8 - delete-s\n9 - ut-m \n10 - ut-s\n-1 - exit\n");
		scanf("%d", &task);
		switch (task){
		case(1): {
			insert_m(db, input_pub());
			break;
		}
		case(2): {
			insert_s(db, input_game());
			break;
		}
		case(3): {
			printf("Enter publisher id:\n");
			int id;
			scanf("%d", &id);
			Publisher* to_get = get_m(db, id);
			if (to_get != NULL) output_pub(*to_get);
			else printf("Publisher not found\n");
			break;
		}
		case(4): {
			int pub_id, game_id;
			printf("Enter publisher id:\n");
			scanf("%d", &pub_id);
			printf("Enter game id:\n");
			scanf("%d", &game_id);
			Game* to_get = get_s(db, pub_id, game_id);
			if (to_get != NULL) output_game(*to_get);
			else printf("Game not found\n");
			break;
		}
		case(5): {
			printf("Enter publisher id:\n");
			int id;
			scanf("%d", &id);
			update_m(db, id);
			break;
		}
		case(6): {
			int pub_id, game_id;
			printf("Enter publisher id:\n");
			scanf("%d", &pub_id);
			printf("Enter game id:\n");
			scanf("%d", &game_id);
			update_s(db, game_id, pub_id);
			break;
		}
		case(7): {
			printf("Enter publisher id:\n");
			int id;
			scanf("%d", &id);
			delete_m(db, id);
			break;
		}
		case(8): {
			int pub_id, game_id;
			printf("Enter publisher id:\n");
			scanf("%d", &pub_id);
			printf("Enter game id:\n");
			scanf("%d", &game_id);
			delete_s(db, pub_id, game_id);
			break;
		}
		case(9): {
			ut_m(db);
			break;
		}
		case(10): {
			ut_s(db);
			break;
		}
		case(-1): {
			break;
		}
		default:
			printf("Incorrect input!\n");
			break;
		}
	} while (task != -1);
	/*insert_m(db, input_pub());
	insert_m(db, input_pub());
	insert_m(db, input_pub());
	insert_s(db, input_game());
	insert_s(db, input_game());
	insert_s(db, input_game());*/
	/*ut_m(db);
	ut_s(db);
	delete_s(db, 1,1);*/
	/*delete_m(db, 1);
	ut_m(db);
	ut_s(db);*/
	int i = count_pub(db);
	int j = count_games(db);
	int k = count_s(db, 1);
	free(db);
	return 0;
}