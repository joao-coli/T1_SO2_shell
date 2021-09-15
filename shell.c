#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Definicoes -----------------------------------------------------------
#define TOKEN_AMNT 16
#define TOKEN_DELIM " \t\r\n"

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
  return 0;
}

int main()
{
  char *line = NULL, **tokens;

  while (1)
  {
    printf("sample_shell >> $ ");

    line = read_line();
    tokens = parse_line(line);

    if (strcmp(line, "exit") == 0)
    {
      break;
    }

    free(tokens);
  }

  return EXIT_SUCCESS;
}