#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

#define K 30



typedef struct list_node { //lista per l'inserimento delle frasi da memorizzare
    char *frase;
    struct list_node *next;
    struct list_node *prev;
} List_Node;
typedef List_Node *List;

typedef struct l {
    List Nil;
} Lista;


typedef struct undo_node { //lista per le undo/redo
    int typeI, num1, num2;
    List_Node *head_hook; //gli hook mi servono per agganciare i nodi da tenere in memoria dalla lista principale
    List_Node *tail_hook;
    List_Node *undo_index1; //gli undo_index mi servono per sapere subito la posizione dove effettuare la undo senza prima ricerca lineare
    List_Node *undo_index2;
    struct undo_node *next;
    struct undo_node *prev;
} Undo_Node;
typedef Undo_Node *Undo;

typedef struct u {
    Undo Nil;
} Undo_List;


int total_lines = 0;
int undo_to_do = 0;
int redo_to_do = 0;
int tot_undo = 0;
char phrase[1026];
char command[K];
int ind1 = 0;
int ind2 = 0;

int curs_var = 0;
List_Node *curs, *temp1, *temp2; //cursori da usare sulla lista principale
Undo_Node *undo_curs; //cursore da usare per la lista delle undo


void menu(Lista *Li, Undo_List *Un);
void mini_menu (Lista *Li, Undo_List *Un, char istr);
List_Node* prepare_node ();
void list_insert (Lista *Li, List_Node *x);
Undo_Node* prepare_undo_node (int tipei);
void undo_list_insert (Undo_List *Un, Undo_Node *x);
void c_function (Lista *Li, Undo_List *Un);
void p_function (Lista *Li);
void d_function (Lista *Li, Undo_List *Un);
void u_function (Lista *Li, Undo_List *Un);
void undo_1 (Lista *Li);
void redo_1 (Lista *Li);
void undo_redo_2 ();
void undo_7 (Lista *Li);
void redo_7 (Lista *Li);
void undo_4 ();
void redo_4 ();
void undo_5 (Lista *Li);
void redo_5 (Lista *Li);
void undo_calculator (Lista *Li, Undo_List *Un);
void undo_list_delete (Undo_List *Un);


int main()
{
    Lista L;
    L.Nil = (List_Node *)malloc(sizeof(List_Node));
    L.Nil->next = L.Nil;
    L.Nil->prev = L.Nil;

    Undo_List U;
    U.Nil = (Undo_Node *)malloc(sizeof(Undo_Node));
    U.Nil->next = U.Nil;
    U.Nil->prev = U.Nil;

    do {
        fgets(command, K-1, stdin);
        menu(&L, &U);
    } while (command[0] != 'q');

    return 0;
}

void menu(Lista *Li, Undo_List *Un) //smista le varie istruzioni
{
    Undo_Node *x;
    char *freccia1, *freccia2; //mi servono per trovare gli indici dei comandi
    ind1 = (int)strtol(command, &freccia1, 10);

    if (*freccia1 == 'u') {
        undo_curs = Un->Nil->next;
        tot_undo = ind1;
        if (tot_undo > undo_to_do) tot_undo = undo_to_do;
        undo_calculator(Li, Un);
    }
    else {
        ++freccia1;
        ind2 = (int)strtol(freccia1, &freccia2, 10);
        if (*freccia2 == 'c') {
            if (ind1 > total_lines) { //CASO 1
                x = prepare_undo_node(1);
                undo_list_insert(Un, x);
                Un->Nil->next->undo_index1 = Li->Nil->next;
                c_function(Li, Un);
            }
            else if (ind2 <= total_lines) { //CASO 2
                x = prepare_undo_node(2);
                undo_list_insert(Un, x);
                c_function(Li, Un);
                }
            else if (ind2 > total_lines) {
                x = prepare_undo_node(7 + total_lines);
                undo_list_insert(Un, x);
                c_function(Li, Un);
            }
        }
        if (*freccia2 == 'p') p_function(Li);
        if (*freccia2 == 'd') {
            if (ind1 > total_lines) {
                x = prepare_undo_node(3);
                undo_list_insert(Un, x);
            }
            else if (ind2 <= total_lines) {
                x = prepare_undo_node(4);
                undo_list_insert(Un, x);
                d_function(Li, Un);
            }
            else if (ind2 > total_lines) {
                x = prepare_undo_node(5);
                undo_list_insert(Un, x);
                d_function(Li, Un);
            }
        }
    }
}

void c_function (Lista *Li, Undo_List *Un) //copia le frasi nella lista
{
    int dummy = ind1;
    List_Node *x;
    if (dummy > total_lines) { //se dummy è > di total_lines devo inserire nuovi nodi CASO 1
        while (dummy <= ind2) {
            x = prepare_node();
            list_insert(Li, x);
            ++dummy;
        }
        total_lines = total_lines + (ind2 - ind1 + 1);
    }
    else if (ind2 <= total_lines) { //CASO 2
        temp1 = Li->Nil;
        curs_var = 0;
        while (curs_var < ind1-1) { //trovo gli indici che devo modificar nella lista
            temp1 = temp1->prev;
            ++curs_var;
        }
        Un->Nil->next->undo_index1 = temp1;
        temp2 = temp1;
        while (curs_var <= ind2) {
            temp2 = temp2->prev;
            ++curs_var;
        }
        Un->Nil->next->undo_index2 = temp2;
        Un->Nil->next->head_hook = temp1->prev; //assengamento ganci
        Un->Nil->next->tail_hook = temp2->next;
        Lista Support; //creo una lista di nodi che vado ad inserire in quella da modificare
        Support.Nil = (List_Node *)malloc(sizeof(List_Node));
        Support.Nil->next = Support.Nil;
        Support.Nil->prev = Support.Nil;
        while (dummy <= ind2) {
            x = prepare_node();
            list_insert(&Support, x);
            ++dummy;
        }
        temp1->prev = Support.Nil->prev; //aggiustamento puntatori
        Support.Nil->prev->next = temp1;
        temp2->next = Support.Nil->next;
        Support.Nil->next->prev = temp2;
    }
    else { //CASO 3
        temp1 = Li->Nil;
        curs_var = 0;
        while (curs_var < ind1-1) { //non mi serve temp2 perchè comunque sforo totallines
            temp1 = temp1->prev;
            ++curs_var;
        }
        Un->Nil->next->undo_index1 = temp1;
        Un->Nil->next->head_hook = temp1->prev; //assengamento ganci
        Un->Nil->next->tail_hook = Li->Nil->next;
        Lista Support; //creazione lista di supporto e conseguente inserimento
        Support.Nil = (List_Node *)malloc(sizeof(List_Node));
        Support.Nil->next = Support.Nil;
        Support.Nil->prev = Support.Nil;
        while (dummy <= ind2) {
            x = prepare_node();
            list_insert(&Support, x);
            ++dummy;
        }
        temp1->prev = Support.Nil->prev; //aggiustamento indici
        Support.Nil->prev->next = temp1;
        Li->Nil->next = Support.Nil->next;
        Support.Nil->next->prev = Li->Nil;
        total_lines = total_lines + (ind2 - total_lines);
    }
    getchar();
}

void p_function (Lista *Li) //serve per stampare le frasi desiderate
{
    int dummy = ind1;
    if (dummy > total_lines) { //qui stampo solo . perchè sono oltre le righe totali
        while (dummy <= ind2) {
            putchar('.');
            putchar('\n');
            ++dummy;
        }
    } else {
        if (ind2 == 0) { //altro caso particolare
            putchar('.');
            putchar('\n');
        }
        else if (ind2 <= total_lines) { //cerco l'intervallo di indici da stampare e lo stampo (ind2 <= totallines)
            curs = Li->Nil;
            curs_var = 0;
            while (curs_var < ind1) {
                curs = curs->prev;
                ++curs_var;
            }
            while (curs_var <= ind2) {
                fputs(curs->frase, stdout);
                ++curs_var;
                curs = curs->prev;
            }
        }
        else { //come prima ma ind2 > totallines
            curs = Li->Nil;
            curs_var = 0;
            while (curs_var < ind1) {
                curs = curs->prev;
                ++curs_var;
            }
            while (curs_var <= total_lines) {
                fputs(curs->frase, stdout);
                ++curs_var;
                curs = curs->prev;
            }
            while (curs_var <= ind2) {
                putchar('.');
                putchar('\n');
                ++curs_var;
            }
        }
    }
}

void d_function (Lista *Li, Undo_List *Un) //serve per eliminare i nodi
{
    if (ind1 <= total_lines) { //se non fosse così il comando si verifica ma non ha effetto
        if (ind2 <= total_lines) { //elimino righe nel limite di total_lines (caso in cui ind1 = ind2 oppure diversi con la lista non fa differenza)
            temp1 = Li->Nil;
            curs_var = 0;
            while (curs_var < ind1-1) {
                temp1 = temp1->prev;
                ++curs_var;
            }
            Un->Nil->next->undo_index1 = temp1;
            temp2 = temp1;
            while (curs_var <= ind2) {
                temp2 = temp2->prev;
                ++curs_var;
            }
            Un->Nil->next->undo_index2 = temp2;
            Un->Nil->next->head_hook = temp1->prev;
            Un->Nil->next->tail_hook = temp2->next;
            temp1->prev = temp2;
            temp2->next = temp1;
            total_lines = total_lines - (ind2 - ind1 + 1);
        }
        else { //qui ind2 è > di total_lines
            temp1 = Li->Nil;
            curs_var = 0;
            while (curs_var < ind1-1) {
                temp1 = temp1->prev;
                ++curs_var;
            }
            Un->Nil->next->undo_index1 = temp1;
            Un->Nil->next->head_hook = temp1->prev;
            Un->Nil->next->tail_hook = Li->Nil->next;
            temp1->prev = Li->Nil;
            Li->Nil->next = temp1;
            total_lines = total_lines - (total_lines - ind1 + 1);
        }
    }
}

void undo_calculator (Lista *Li, Undo_List *Un) //funzione che mi calcola quanti undo devo fare in totale
{
    char *freccia1, *freccia2;
    do {
        fgets(command, K-1, stdin);
        ind1 = (int) strtol(command, &freccia1, 10);
        if (*freccia1 == ',') {
            ++freccia1;
            ind2 = (int) strtol(freccia1, &freccia2, 10);
            if (*freccia2 == 'p') { //se il comando seguente è una p devi fare gli undo prima di printare, poi dato che le p non hanno effetto ricominci il ciclo per vedere il prossimo comando
                u_function(Li, Un);
                p_function(Li);
                continue;
            } else { //sei in un caso c o d, prima fai le undo e dopo cancelli i nodi della lista degli undo fino a curs
                if (tot_undo != 0) { u_function(Li, Un); }
                while (Un->Nil->next != undo_curs) { //non serve aggiornare Li->Nil->next perchè ad ogni cancellazione avanza di 1
                    undo_list_delete(Un);
                }
            }
            redo_to_do = 0; //quando cancello la lista non ho più redo possibili
            mini_menu(Li, Un, *freccia2); //eseguo l'istruzione c o d
            break;
        }
        else if (*freccia1 == 'u') {
                tot_undo = tot_undo + ind1;
                if (tot_undo > undo_to_do) { tot_undo = undo_to_do; } //se ho più undo di quelle concesse mi fermo al massimo che posso fare
                continue;
            }
        else if (*freccia1 == 'r') {
            tot_undo = tot_undo - ind1;
            if ((0 - tot_undo) > redo_to_do) { tot_undo = 0 - redo_to_do; } //se ho più redo di quelle concesse mi fermo al massimo che posso fare
            continue;
        }
        if (ind1 == 0) { //se il comando seguente è una q termino
            command[0] = 'q';
            break;
        }
    } while (1);
}

void mini_menu (Lista *Li, Undo_List *Un, char istr) //mi serve per eseguire l'istruzione c o d prima di terminare il processo delle undo
{
    Undo_Node *x;
    if (istr == 'c') {
        if (ind1 > total_lines) { //CASO 1
            x = prepare_undo_node(1);
            undo_list_insert(Un, x);
            Un->Nil->next->undo_index1 = Li->Nil->next;
            c_function(Li, Un);
        } else if (ind2 <= total_lines) { //CASO 2
            x = prepare_undo_node(2);
            undo_list_insert(Un, x);
            c_function(Li, Un);
        } else if (ind2 > total_lines) {
            x = prepare_undo_node(7 + total_lines);
            undo_list_insert(Un, x);
            c_function(Li, Un);
        }
    }
    if (istr == 'd') {
        if (ind1 > total_lines) {
            x = prepare_undo_node(3);
            undo_list_insert(Un, x);
        } else if (ind2 <= total_lines) {
            x = prepare_undo_node(4);
            undo_list_insert(Un, x);
            d_function(Li, Un);
        } else if (ind2 > total_lines) {
            x = prepare_undo_node(5);
            undo_list_insert(Un, x);
            d_function(Li, Un);
        }
    }
}


void u_function (Lista *Li, Undo_List *Un) //funzione che svolge le operazioni di undo e redo
{
    if (tot_undo > 0) { //faccio le undo
        while (undo_to_do > 0 && tot_undo > 0) {

            switch (undo_curs->typeI) {

                case 1:
                    undo_1(Li);
                    break;

                case 2:
                    undo_redo_2();
                    break;

                case 3:
                    break;

                case 4:
                    undo_4();
                    break;

                case 5:
                    undo_5(Li);
                    break;

                default:
                    undo_7(Li);
                    break;
            }

            --tot_undo;
            --undo_to_do;
            ++redo_to_do;
            undo_curs = undo_curs->next;
        }
        tot_undo = 0;
    }

    else if (tot_undo < 0) { //faccio le redo
        while (redo_to_do > 0 && tot_undo < 0) {
            if (undo_curs->prev != Un->Nil) {undo_curs = undo_curs->prev;} //mi sposto in su di uno perchè faccio la redo del comando che cronologicamente è venuto dopo
            switch (undo_curs->typeI) {

                case 1:
                    redo_1(Li);
                    break;

                case 2:
                    undo_redo_2();
                    break;

                case 3:
                    break;

                case 4:
                    redo_4();
                    break;

                case 5:
                    redo_5(Li);
                    break;

                default:
                    redo_7(Li);
                    break;
            }

            ++tot_undo;
            ++undo_to_do;
            --redo_to_do;
        }
        tot_undo = 0; // dopo l'esecuzione delle redo resetta indipendentemente il numero tot_undo
    }
}

void undo_1 (Lista *Li)
{
    undo_curs->head_hook = undo_curs->undo_index1->prev;
    undo_curs->tail_hook = Li->Nil->next;
    undo_curs->undo_index1->prev = Li->Nil;
    Li->Nil->next = undo_curs->undo_index1;
    total_lines = total_lines - (undo_curs->num2 - undo_curs->num1 + 1);
}

void redo_1 (Lista *Li)
{
    undo_curs->head_hook->next = undo_curs->undo_index1;
    undo_curs->undo_index1->prev = undo_curs->head_hook;
    Li->Nil->next = undo_curs->tail_hook;
    undo_curs->tail_hook->prev = Li->Nil;
    total_lines = total_lines + (undo_curs->num2 - undo_curs->num1 + 1);
}

void undo_redo_2 ()
{
    List_Node *temp;
    temp = undo_curs->head_hook;
    undo_curs->head_hook->next = undo_curs->undo_index1;
    undo_curs->head_hook = undo_curs->undo_index1->prev;
    undo_curs->undo_index1->prev = temp;
    temp = undo_curs->tail_hook;
    undo_curs->tail_hook->prev = undo_curs->undo_index2;
    undo_curs->tail_hook = undo_curs->undo_index2->next;
    undo_curs->undo_index2->next = temp;
}

void undo_7 (Lista *Li)
{
    List_Node *temp;
    temp = undo_curs->head_hook;
    undo_curs->head_hook->next = undo_curs->undo_index1;
    undo_curs->head_hook = undo_curs->undo_index1->prev;
    undo_curs->undo_index1->prev = temp;
    temp = undo_curs->tail_hook;
    undo_curs->tail_hook->prev = Li->Nil;
    undo_curs->tail_hook = Li->Nil->next;
    Li->Nil->next = temp;
    total_lines = undo_curs->typeI - 7;
}

void redo_7 (Lista *Li)
{
    List_Node *temp;
    temp = undo_curs->head_hook;
    undo_curs->head_hook = undo_curs->undo_index1->prev;
    undo_curs->undo_index1->prev = temp;
    temp = undo_curs->tail_hook;
    undo_curs->tail_hook = Li->Nil->next;
    temp->prev = Li->Nil;
    total_lines = total_lines + (undo_curs->num2 - total_lines);
}

void undo_4 ()
{
    undo_curs->head_hook->next = undo_curs->undo_index1;
    undo_curs->undo_index1->prev = undo_curs->head_hook;
    undo_curs->tail_hook->prev = undo_curs->undo_index2;
    undo_curs->undo_index2->next = undo_curs->tail_hook;
    total_lines = total_lines + (undo_curs->num2 - undo_curs->num1 + 1);
}

void redo_4 ()
{
    undo_curs->undo_index1->prev = undo_curs->undo_index2;
    undo_curs->undo_index2->next = undo_curs->undo_index1;
    total_lines = total_lines - (undo_curs->num2 - undo_curs->num1 + 1);
}

void undo_5 (Lista *Li)
{
    undo_curs->head_hook->next = undo_curs->undo_index1;
    undo_curs->undo_index1->prev = undo_curs->head_hook;
    undo_curs->tail_hook->prev = Li->Nil;
    Li->Nil->next = undo_curs->tail_hook;
    total_lines = total_lines + (undo_curs->num2 - undo_curs->num1 + 1);
}

void redo_5 (Lista *Li)
{
    undo_curs->undo_index1->prev = Li->Nil;
    Li->Nil->next = undo_curs->undo_index1;
    total_lines = total_lines - (undo_curs->num2 - undo_curs->num1 + 1);
}

List_Node* prepare_node () //prepara il nodo da inserire nella lista
{
    List_Node *x;
    x = (List_Node *)malloc(sizeof(List_Node));
    fgets(phrase, 1025, stdin);
    size_t len = strlen(phrase);
    x->frase = (char *)malloc(len*sizeof(char)+1);
    memcpy(x->frase, phrase, len+1);
    return x;
}

void list_insert (Lista *Li, List_Node *x) //inserisce un nodo nella lista
{
    x->next = Li->Nil->next;
    Li->Nil->next->prev = x;
    Li->Nil->next = x;
    x->prev = Li->Nil;
}

Undo_Node* prepare_undo_node (int tipei) //prepara il nodo da inserire nella lista undo
{
    Undo_Node *x;
    x = (Undo_Node *)malloc(sizeof(Undo_Node));
    x->typeI = tipei;
    x->num1 = ind1;
    x->num2 = ind2;
    if (tipei == 5) x->num2 = total_lines;
    return x;
}

void undo_list_insert (Undo_List *Un, Undo_Node *x) //inserisce un nodo nella lista undo
{
    x->next = Un->Nil->next;
    Un->Nil->next->prev = x;
    Un->Nil->next = x;
    x->prev = Un->Nil;
    ++undo_to_do;
}

void undo_list_delete (Undo_List *Un) //elimina il primo elemento in testa dalla lista ad ogni chiamata
{
    Undo_Node *x;
    x = Un->Nil->next;
    x->prev->next = x->next;
    x->next->prev = x->prev;
    free(x);
}
