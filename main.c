#include <stdio.h>
#pragma warning(disable : 4996)
char publisher_table[] = "pb_fl.bin";
char publisher_index[] = "pb_ind.bin";
char game_table[] = "game_fl.bin";
int count = 0;
typedef struct DataBase {
	int publ_count;
	int publ_ind[20];
} DataBase;
typedef struct Publisher {
	int id;
	int creation_year;
	char company_name[20];
	
} Publisher;
void output(Publisher p) {
	printf("%d %d %s", p.id, p.creation_year, p.company_name);
}
void insert_m() {
	Publisher new_pub;
	printf("Enter Publisher ID\n");
	scanf("%d", &(new_pub.id));
	printf("Enter Publisher creation year\n");
	scanf("%d", &(new_pub.creation_year));
	printf("Enter Publisher company name\n");
	scanf("%19s", new_pub.company_name);
	FILE* fl, *ind;
	fl = fopen(publisher_table, "ab+");
	Publisher* p_ = &new_pub;
	fwrite(p_, sizeof(Publisher), 1, fl);
	count++;

}
int main() {
	Publisher new_pub;
	Publisher* curr = &new_pub;
	FILE* f;
	char theName[] = "pb_fl.bin";
	f = fopen(theName, "rb+");
	fread(curr, sizeof(Publisher), 1, f);
	output(new_pub);
	fread(curr, sizeof(Publisher), 1, f);
	output(new_pub);
}