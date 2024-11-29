#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Estrutura de um livro
typedef struct Livro {
    char titulo[50];
    char autor[50];
    double avaliacaoMedia; // avaliação do livro (0 - 5)
    int numeroAvaliacoes;
    struct Livro *esquerda, *direita;
} Livro;

// Estrutura para tabela hash de livros
#define TAMANHO_HASH_BOOKS 100

typedef struct NoHash {
    Livro *livro;
    struct NoHash *proximo;
} NoHash;

// Estrutura de tabela hash para usuários
#define TAMANHO_HASH_USERS 100

typedef struct Usuario {
    char nome[50];
    char livroFavorito[100];
    struct Usuario *proximo;
} Usuario;


NoHash *tabelaHashLivros[TAMANHO_HASH_BOOKS];
Usuario *tabelaHashUsuarios[TAMANHO_HASH_USERS];

//FUNÇÕES REFERENTES AOS LIVROS

// Função hash para o título
unsigned int hashTitulo(const char *titulo) {
    unsigned int hash = 0;
    while (*titulo)
        hash = (hash << 5) + *titulo++;
    return hash % TAMANHO_HASH_BOOKS;
}

// Inserir livro na tabela hash
void inserirLivroNaHash(Livro *livro) {
    unsigned int indice = hashTitulo(livro->titulo);
    NoHash *novoNo = (NoHash *)malloc(sizeof(NoHash));
    novoNo->livro = livro;
    novoNo->proximo = tabelaHashLivros[indice];
    tabelaHashLivros[indice] = novoNo;
}

// inserir livro na árvore de recomendação
void inserirLivro(Livro **raiz, const char *titulo, const char *autor, double avaliacaoMedia, int numeroAvaliacoes) {
    if (*raiz == NULL) {
        // Criação de um novo nó
        Livro *novoLivro = (Livro *)malloc(sizeof(Livro));
        strcpy(novoLivro->titulo, titulo);
        strcpy(novoLivro->autor, autor);
        novoLivro->avaliacaoMedia = avaliacaoMedia;
        novoLivro->numeroAvaliacoes = numeroAvaliacoes;
        novoLivro->esquerda = novoLivro->direita = NULL;
        *raiz = novoLivro;
        inserirLivroNaHash(novoLivro);
        return;
    }

    // Se a pontuação for menor, insere na esquerda
    if (avaliacaoMedia < (*raiz)->avaliacaoMedia) {
        inserirLivro(&(*raiz)->esquerda, titulo, autor, avaliacaoMedia, numeroAvaliacoes);
    }
    // Se a pontuação for maior, insere na direita
    else if (avaliacaoMedia > (*raiz)->avaliacaoMedia) {
        inserirLivro(&(*raiz)->direita, titulo, autor, avaliacaoMedia, numeroAvaliacoes);
    }
    // Critério de desempate para pontuações iguais
    else {
        // Se o número de avaliações for maior, insere na direita
        if (numeroAvaliacoes > (*raiz)->numeroAvaliacoes) {
            inserirLivro(&(*raiz)->direita, titulo, autor, avaliacaoMedia, numeroAvaliacoes);
        }
        // Caso contrário, insere na esquerda
        else {
            inserirLivro(&(*raiz)->esquerda, titulo, autor, avaliacaoMedia, numeroAvaliacoes);
        }
    }
}

// Buscar livro pelo título na tabela hash
Livro *buscarLivro(const char *titulo) {
    unsigned int indice = hashTitulo(titulo);
    NoHash *no = tabelaHashLivros[indice];
    while (no) {
        if (strcmp(no->livro->titulo, titulo) == 0)
            return no->livro;
        no = no->proximo;
    }
    return NULL; // Livro não encontrado
}

// Remover livro da tabela hash
void removerLivroDaHash(const char *titulo) {
    unsigned int indice = hashTitulo(titulo);
    NoHash *no = tabelaHashLivros[indice];
    NoHash *anterior = NULL;
    while (no) {
        if (strcmp(no->livro->titulo, titulo) == 0) {
            if (anterior) {
                anterior->proximo = no->proximo;
            } else {
                tabelaHashLivros[indice] = no->proximo;
            }
            free(no); // Libera a memória
            return;
        }
        anterior = no;
        no = no->proximo;
    }
}

// Remover livro da árvore binária baseado na nota e número de avaliações
Livro* removerLivroArvore(Livro *raiz, char* titulo, double avaliacaoMedia, int numeroAvaliacoes) {
    if (raiz == NULL)
        return raiz;

    // Se a avaliação for menor, insere na subárvore esquerda
    if (avaliacaoMedia < raiz->avaliacaoMedia || 
       (avaliacaoMedia == raiz->avaliacaoMedia && numeroAvaliacoes < raiz->numeroAvaliacoes)) {
        raiz->esquerda = removerLivroArvore(raiz->esquerda, titulo, avaliacaoMedia, numeroAvaliacoes);
    }
    // Se a avaliação for maior, insere na subárvore direita
    else if (avaliacaoMedia > raiz->avaliacaoMedia || 
            (avaliacaoMedia == raiz->avaliacaoMedia && numeroAvaliacoes > raiz->numeroAvaliacoes)) {
        raiz->direita = removerLivroArvore(raiz->direita, titulo, avaliacaoMedia, numeroAvaliacoes);
    }
    // Caso o livro a ser removido seja encontrado
    else {
        // Caso 1: No a ser removido não tem filhos
        if (raiz->esquerda == NULL && raiz->direita == NULL) {
            free(raiz);
            return NULL;
        }
        // Caso 2: No a ser removido tem um filho
        else if (raiz->esquerda == NULL) {
            Livro *temp = raiz->direita;
            free(raiz);
            return temp;
        } else if (raiz->direita == NULL) {
            Livro *temp = raiz->esquerda;
            free(raiz);
            return temp;
        }
        // Caso 3: No a ser removido tem dois filhos
        else {
            // Encontrar o sucessor in-order (menor na subárvore direita)
            Livro *temp = raiz->direita;
            while (temp->esquerda != NULL)
                temp = temp->esquerda;

            // Substituir o valor do nó a ser removido com o sucessor
            strcpy(raiz->titulo, temp->titulo);
            raiz->avaliacaoMedia = temp->avaliacaoMedia;
            raiz->numeroAvaliacoes = temp->numeroAvaliacoes;

            // Remover o sucessor na subárvore direita
            raiz->direita = removerLivroArvore(raiz->direita, titulo, temp->avaliacaoMedia, temp->numeroAvaliacoes);
        }
    }
    return raiz;
}


// Função para apagar livro usando o título
void removerLivro(Livro **raiz, const char *titulo) {
    // Primeiro, buscamos o livro na tabela hash
    Livro *livro = buscarLivro(titulo);
    if (livro) {
        // Remover da árvore binária
        removerLivroArvore(*raiz, livro->titulo, livro->avaliacaoMedia, livro->numeroAvaliacoes);
        // Remover da tabela hash
        removerLivroDaHash(titulo);
        printf("\nLivro '%s' removido com sucesso.\n", titulo);
    } else {
        printf("\nLivro '%s' nao encontrado.\n", titulo);
    }
}


// Lista todos os livros existentes na árvore binária (ordem decrescente)
void listarTodosLivros(Livro *raiz) {
    if (raiz != NULL) {
        listarTodosLivros(raiz->direita);
        printf("Titulo: %s, Autor: %s, Nota: %.2f, Numero de Avaliacoes: %d\n",
               raiz->titulo, raiz->autor, raiz->avaliacaoMedia, raiz->numeroAvaliacoes);
        listarTodosLivros(raiz->esquerda);
    }
}

// Lista os 5 primeiros livros mais bem avaliados (ordem decrescente)
void mostrarLivrosMaisRecomendadosRec(Livro *raiz, int *rep) {
    if (raiz == NULL || *rep > 5) {
        return;
    }

    mostrarLivrosMaisRecomendadosRec(raiz->direita, rep);

    if (*rep <= 5) {
        printf("Titulo: %s, Autor: %s, Nota: %.2f, Numero de Avaliacoes: %d\n",
               raiz->titulo, raiz->autor, raiz->avaliacaoMedia, raiz->numeroAvaliacoes);
        (*rep)++;
    }

    mostrarLivrosMaisRecomendadosRec(raiz->esquerda, rep);
}

// função controladora
void mostrarLivrosMaisRecomendados(Livro *raiz) {
    int rep = 1; // Controlador do número de livros impressos
    mostrarLivrosMaisRecomendadosRec(raiz, &rep);
}


//FUNÇÕES REFERENTES AOS USUÁRIOS

// Função hash
unsigned int hashUsers(const char *str) {
    unsigned int hash = 0;
    while (*str)
        hash = (hash << 5) + *str++;
    return hash % TAMANHO_HASH_USERS;
}

// Inserir usuário na tabela hash
void inserirUsuario(const char *nome, const char *livroFavorito, Livro *raiz) {
    unsigned int indice = hashUsers(nome);
    Usuario *novoUsuario = (Usuario *)malloc(sizeof(Usuario));
    strcpy(novoUsuario->nome, nome);

    //Verificando a existência do livro
    Livro* livro = buscarLivro(livroFavorito);

    if (livro) { // livro encontrado no sistema
        strcpy(novoUsuario->livroFavorito, livroFavorito);
    }
    else {
        char res[2]; // respostas dos usuários
        char titulo[50];
        int controller = 1;

        printf("\n'%s' nao existe no sistema\n\n", livroFavorito);
        printf("Deseja ver nossos livros recomendados? (y/n): ");
        fflush(stdin);
        scanf("%c", &res[0]);
        printf("\n");

        if (res[0] == 'y') {
            mostrarLivrosMaisRecomendados(raiz);
            printf("\n1 - Escolher livro\n");
            printf("2 - Sair\n");
            printf("\nSelecione uma opcao: ");
            fflush(stdin);
            scanf("%c", &res[1]);

            if (res[1] == '1') {
                do {
                    printf("\nDigite o titulo do livro que deseja: ");
                    fflush(stdin);
                    fgets(titulo, sizeof(titulo), stdin);

                    // Remove o '\n' do final da string, se necessário
                    titulo[strcspn(titulo, "\n")] = '\0';

                    if (buscarLivro(titulo) == NULL) {
                        printf("\nTitulo incorreto, tente novamente\n");
                    }
                    else {
                        controller = 0;
                    }

                } while (controller);

                strcpy(novoUsuario->livroFavorito, titulo);
            }
            else {
                printf("\nCadastro cancelado\n");
                free(novoUsuario);
                return; 
            }
        }
        else {
            printf("\nCadastro cancelado\n");
            free(novoUsuario);
            return;
        }  
    }

    novoUsuario->proximo = tabelaHashUsuarios[indice];
    tabelaHashUsuarios[indice] = novoUsuario;
    printf("\nUsuario adicionado com sucesso\n");
}

// Procurar usuário na tabela hash
Usuario *procurarUsuario(const char *nome) {
    unsigned int indice = hashUsers(nome);
    Usuario *usuario = tabelaHashUsuarios[indice];
    while (usuario) {
        if (strcmp(usuario->nome, nome) == 0)
            return usuario;
        usuario = usuario->proximo;
    }
    return NULL;
}

int main() {

    // Criar árvore de livros
    Livro *raiz = NULL;
    inserirLivro(&raiz, "O Senhor dos Aneis", "J.R.R. Tolkien", 4.9, 6000);
    inserirLivro(&raiz, "Harry Potter e a Pedra Filosofal", "J.K. Rowling", 4.8, 4500);
    inserirLivro(&raiz, "O Hobbit", "J.R.R. Tolkien", 4.7, 4000);
    inserirLivro(&raiz, "1984", "George Orwell", 4.0, 3200);
    inserirLivro(&raiz, "A Revolucao dos Bichos", "George Orwell", 4.5, 3100);
    inserirLivro(&raiz, "Dom Quixote", "Miguel de Cervantes", 4.4, 2700);
    inserirLivro(&raiz, "O Apanhador no Campo de Centeio", "J.D. Salinger", 2.3, 2300);
    inserirLivro(&raiz, "Moby Dick", "Herman Melville", 4.9, 5000);
    inserirLivro(&raiz, "Guerra e Paz", "Liev Tolstoi", 3.1, 2100);
    inserirLivro(&raiz, "Orgulho e Preconceito", "Jane Austen", 4.8, 4100);
    inserirLivro(&raiz, "O Pequeno Principe", "Antoine de Saint-Exupery", 5.0, 3500);
    

    // Imprimir os livros ordenados
    printf("Lista os livros por ordem de avaliacao:\n");
    listarTodosLivros(raiz);


    removerLivro(&raiz, "Orgulho e Preconceito");

    printf("\nLista os livros por ordem de avaliacao:\n");
    listarTodosLivros(raiz);

    // Inserir usuários na tabela hash
    inserirUsuario("Gabryel", "O Pequen Principe", raiz);

    // Procurar um usuário e exibir seu livro favorito
    Usuario *usuario1 = procurarUsuario("Gabryel");

    if (usuario1)
        printf("\nUsuario: %s, Livro Favorito: %s\n", usuario1->nome, usuario1->livroFavorito);
    else
        printf("\nUsuario nao encontrado.\n");


    // Inserir usuários na tabela hash
    inserirUsuario("Rafael", "1984", raiz);

    // Procurar um usuário e exibir seu livro favorito
    Usuario *usuario2 = procurarUsuario("Rafael");

    if (usuario2)
        printf("\nUsuario: %s, Livro Favorito: %s\n", usuario2->nome, usuario2->livroFavorito);
    else
        printf("\nUsuario nao encontrado.\n");


    return 0;
}
