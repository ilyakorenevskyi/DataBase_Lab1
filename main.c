#include <stdio.h>
#include <stdbool.h>
#pragma warning(disable : 4996)
char publisher_table[] = "pb_fl.bin";
char publisher_index[] = "pb_ind.bin";
char game_table[] = "game_fl.bin";
#define MAX_REC 20

typedef struct Index {
	int ind;
	int num;
} Index;

typedef struct DataBase {
	int pub_count;
	int game_count;
	Index index[MAX_REC];
} DataBase;


typedef struct Publisher {
	int id;
	int creation_year;
	char company_name[20];
	int game_num;
} Publisher;


typedef struct Game {
	int id;
	char title[20];
	int price;
	int next_game;
} Game;

void load_ind(DataBase *db){
	FILE* fl;
	fl = fopen(publisher_index, "rb");
	Index next;
	int i = 0;
	int temp = fileno(fl) / sizeof(Index);
	while (!feof(fl)) {
		fread(&db->index[i], sizeof(Index), 1, fl);
		if (feof(fl)) break;
		i++;
	}
	db->pub_count = i;
}

void write_ind(DataBase* db) {
	FILE* fl;
	fl = fopen(publisher_index, "wb+");
	Index* beg = &db->index[0];
	fwrite(beg, sizeof(Index), db->pub_count, fl);
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
	fl = fopen(publisher_index, "rb");
	fseek(fl, 0L, SEEK_END);
	db->game_count = ftell(fl) / sizeof(Index);
	fclose(fl);
}

bool insert_ind(DataBase *db, int id) {
	int i = 0;
	Index new_pub = { id, db->pub_count };
	while (db->index[i].ind != -1 ) {
		if (db->index[i].ind == id){
			return 0;
		}
		if (db->index[i].ind > id) {
			break;
		}
		i++;
	}
	do {
		Index temp = db->index[i];
		db->index[i] = new_pub;
		new_pub = temp;
		i++;
	} while (new_pub.ind != -1);
	write_ind(db);
	return 1;
}

void output_pub(Publisher p) {
	printf("%d %d %s", p.id, p.creation_year, p.company_name);
}

int find_m(int to_find, DataBase *db){
	int low, high, middle;
	low = 0;
	high = db->pub_count;
	while (low <= high)
	{
		middle = (low + high) / 2;
		if (to_find < db->index[middle].ind)
			high = middle - 1;
		else if (to_find > db->index[middle].ind)
			low = middle + 1;
		else
			return db->index[middle].num;
	}
	return -1;
}

void append_s(DataBase* db, int prev_game, FILE* games) {
	Game* temp;
	fseek(games, (prev_game - 1) * sizeof(Publisher), SEEK_SET);
	fread(temp, sizeof(Publisher), 1, games);
	if (temp->next_game == -1) {
		temp->next_game = db->game_count;
		fseek(games, (prev_game - 1) * sizeof(Publisher), SEEK_SET);
		fwrite(temp, sizeof(Publisher), 1, games);
		fclose(games);
	}
	else {
		append_s(db, temp->next_game, games);
	}
}

int append_m(DataBase* db, int id, Game *new_game) {
	int row = find_m(id, db);
	if (row == -1) {
		return -1;
	}
	else {
		db->game_count++;
		new_game->next_game = -1;
		FILE* fl;
		Publisher* temp;
		fl = fopen(publisher_table, "rb+");
		fseek(fl, (row - 1) * sizeof(Publisher), SEEK_SET);
		fread(temp, sizeof(Publisher), 1, fl);
		if (temp->game_num == -1) {
			temp->game_num = db->game_count;
			fseek(fl, (row - 1) * sizeof(Publisher), SEEK_SET);
			fwrite(temp, sizeof(Publisher), 1, fl);
		}
		else {
			FILE* games;
			games = fopen(game_table, "rb+");
			append_s(db, temp->game_num, games);
		}
		fclose(fl);
		return 0;
	}
}

void insert_s(DataBase* db) {
	Game new_game;
	Publisher curr_pub;
	int pub_id;
	printf("Enter Game ID\n");
	scanf("%d", &(new_game.id));
	printf("Enter Game title\n");
	scanf("%19s", &(new_game.title));
	printf("Enter Game price\n");
	scanf("%d", &(new_game.price));
	printf("Enter Publisher ID\n");
	scanf("%d", &pub_id);
	if (append_m(db, pub_id, &new_game) != -1) {
		FILE* games;
		games = fopen(game_table, "ab");
		fwrite(&new_game, sizeof(Game), 1, games);
		fclose(games);
	}
}

Publisher* get_m(DataBase* db, int id) {
	int row;
	/*printf("Enter the ID:\n");
	scanf("%d", &get_id);*/
	row = find_m(id, db);
	if (row == -1) {
		printf("Record not found\n");
		return NULL;
	}
	FILE* fl;
	fl = fopen(publisher_table, "rb");
	fseek(fl, (row - 1) * sizeof(Publisher), SEEK_SET);
	Publisher* found;
	fread(found, sizeof(Publisher), 1, fl);
	fclose(fl);
	return found;
}

void insert_m(DataBase *db, Publisher new_pub) {
	if (db->pub_count >= MAX_REC) {
		printf("Maximum number of records reached!\n");
		return;
	}
	/*Publisher new_pub;
	db->pub_count++;
	printf("Enter Publisher ID\n");
	scanf("%d", &(new_pub.id));
	printf("Enter Publisher creation year\n");
	scanf("%d", &(new_pub.creation_year));
	printf("Enter Publisher company name\n");
	scanf("%19s", new_pub.company_name);
	new_pub.game_num = -1;*/
	if (!insert_ind(db, new_pub.id)) {
		printf("Publisher with this ID is already created!\n");
		return;
		db->pub_count--;
	}
	FILE* fl;
	fl = fopen(publisher_table, "ab+");
	fwrite(&new_pub, sizeof(Publisher), 1, fl);
	fclose(fl);
}
int main() {
	DataBase db ;
	init(&db);
	char theName[] = "pb_fl.bin";
	return 0;
}