#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

// Definicoes -----------------------------------------------------------
#define TOKEN_AMNT 16
#define TOKEN_DELIM " \t\r\n"
#define TOKEN_BACK "&"

int isBackground = 0;

// Implementacoes -------------------------------------------------------
void **realloc_buffer(void **buff, int new_size)
{
  buff = realloc(buff, new_size);

  if (!buff)
  {
    printf('Failed to reallocate memory. Exiting...');
    exit(-1);
  }

  return buff;
}

char *read_line()
{
  char *line = NULL;
  size_t line_len = 0;

  if (getline(&line, &line_len, stdin) == -1)
  {
    exit(EXIT_SUCCESS); // EOF
  }

  return line;
}

char **parse_line(char *line)
{
  int i = 0, curr_size = TOKEN_AMNT;
  char **tokens = malloc(TOKEN_AMNT * sizeof(char *)), *token;

  // TODO: checar possiveis erros de alocacao no malloc
  token = strtok(line, TOKEN_DELIM);

  while (token != NULL)
  {
    if (strcmp(token, "&") == 0) {
      isBackground = 1;
      break;
    }

    tokens[i] = token;
    i++;

    if (i >= curr_size)
    {
      curr_size += TOKEN_AMNT; // Dobrar o valor seria desnecessario
      tokens = realloc_buffer(tokens, curr_size * sizeof(char *));
    }

    token = strtok(NULL, TOKEN_DELIM);
  }

  tokens[i] = NULL; // Explicitando o fim da lista
  return tokens;
}

int execute_commands(char **args)
{
  int status;
  pid_t result, pid_filho;

  result = fork();

  if (result==-1) {
      perror("Erro na execucao do comando fork");
      exit(0);
  }

  if (result==0) { // este é o processo filho

    // sobrepoe o processo atual com um novo processo
    execvp(args[0],args);
    // Se chegou até aqui, é porque o exec falhou :frowning:
    printf("Erro na execucao do execl\n");

    exit(1);

  } else { // este é o processo pai
    // Se o processo for executado em Background terá a global variable isBackground, então o pai não deve esperar
    if ( isBackground == 0 ) {
      // processo pai vai esperar pelo filho
      pid_filho=wait(&status);
      if(WIFEXITED(status))
        printf("%d terminou com status %d\n",(int)pid_filho,WEXITSTATUS(status));
      else
        printf("Filho não terminou normalmente\n");
    }
  }

  isBackground = 0;

  return 0;
}

int main()
{
  char *line = NULL, **tokens;

  while (1)
  {
    printf("sample_shell >> $ ");

    line = read_line();

    if (strcmp(line, "exit\n") == 0)
    {
      break;
    }

    tokens = parse_line(line);
    execute_commands(tokens);

    free(tokens);
  }

  return EXIT_SUCCESS;
}