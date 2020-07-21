#include <stdio.h>
#include <string.h>
int main() {
    char *trail =
			"GGE.... SGE.... HGE.... MGE.... DS?.... "
			"GST.... SST.... HST.... MST.... DD1....";
    printf("%s\n", trail);
    int round = (strlen(trail)+1)/40;
    printf("Round: %d\n", round);

    int first_index = (round - 1) * 40;
	int next_turn = ((strlen(trail) - first_index)/8 + 1) % 5;
    printf("Next Turn: %d\n", next_turn);

    printf("%ld\n", strlen(""));

    // int numSpaces = 0;
	// for (int i = 0; i < 100 && trail[i] != '\0'; i++) {
	// 	if (trail[i] == ' ') numSpaces++;
	// }
    // printf("%d\n", numSpaces);
    // int round = numSpaces/5;
    // // Index is the start of next turn
    // int index = round * 40;
	// int turn = (strlen(trail) - index)/8;
    // printf("%c\n", trail[index]);
    // printf("Round: %d Index: %d Turn: %d\n", round, index, turn);
}