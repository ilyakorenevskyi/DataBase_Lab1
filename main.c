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
} Publisher;


typedef struct Game {
	int id;
	char title[20];
	int release_year;
	int price;
	int next_game;
} Game;

void load_ind(DataBase *db){
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

void output(Publisher p) {
	printf("%d %d %s", p.id, p.creation_year, p.company_name);
}

int binarysearch(int to_find, DataBase *db){
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

void get_m(DataBase* db) {
	int get_id, row;
	printf("Enter the ID:\n");
	scanf("%d", &get_id);
	row = binarysearch(get_id, db);
	if (row == -1) {
		printf("Record not found\n");
		return;
	}
	FILE* fl;
	fl = fopen(publisher_table, "rb");
	fseek(fl, (row - 1) * sizeof(Publisher), SEEK_SET);
	Publisher found;
	fread(&found, sizeof(Publisher), 1, fl);
	fclose(fl);
	output(found);
	return;
}

void insert_m(DataBase *db) {
	if (db->pub_count >= MAX_REC) {
		printf("Maximum number of records reached!\n");
		return;
	}
	Publisher new_pub;
	db->pub_count++;
	printf("Enter Publisher ID\n");
	scanf("%d", &(new_pub.id));
	printf("Enter Publisher creation year\n");
	scanf("%d", &(new_pub.creation_year));
	printf("Enter Publisher company name\n");
	scanf("%19s", new_pub.company_name);
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
	insert_m(&db);
	return 0;
}