int countLetter(char *str, char ch) {

    int i =0;
    int count = 0;

    while (str[i] != '\0') {
        if (str[i] == ch) {
            count++;
        }
        i++;
    }
    return count;
}

int main() {
    char *exapmle_array = "Merhaba salih";
    char example_char = 'a';

    int sonuc = countLetter(exapmle_array, example_char);
    printf("'%c' karakteri %d kez bulundu.\n", example_char, sonuc);

    return 0;
}

