#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

// Definicoes -----------------------------------------------------------

#define TOKEN_AMNT 16
#define TOKEN_DELIM " \t\r\n"
#define PROC_AMNT 8

int is_background = 0;

// Struct para processos
typedef struct
{
  int argc;
  char **args;
  pid_t pid;
} proc;

typedef struct
{
  proc *processes;
  int processes_count;
  int max_processes;
} job;

// Implementacoes -------------------------------------------------------

/**
 * Mistério
 **/
void sighand(int signum)
{
  pid_t pid;
  int status;

  pid = waitpid(-1, &status, WNOHANG);
}

proc initialize_proc_struct()
{
  proc new_proc;
  new_proc.argc = 0;
  new_proc.pid = 0;

  return new_proc;
}

job initialize_job_struct()
{
  job new_job;
  new_job.max_processes = PROC_AMNT;
  new_job.processes_count = 0;
  new_job.processes = malloc(new_job.max_processes * sizeof(proc *));

  return new_job;
}

/**
 * Desaloca a memória do vetor de processos e destrói o ponteiro da struct
 * Nao ha retorno
 **/
void destroy_job_struct(job *curr_job)
{
  free(curr_job->processes);
  curr_job = NULL;
}

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

/**
 * Dado um job e um proc, adiciona o proc a lista de processos do job, realocando memoria quando necessario
 * Nao ha retorno
 **/
void add_proc_to_job(job *curr_job, proc *curr_proc)
{
  if (curr_job->processes_count >= curr_job->max_processes)
  {
    curr_job->max_processes += PROC_AMNT;
    curr_job->processes = realloc_buffer(curr_job->processes, curr_job->max_processes * sizeof(proc *));
  }

  curr_job->processes[curr_job->processes_count] = *curr_proc;
  curr_job->processes_count += 1;
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

/**
 * Dado o input  do usuario, separa as diferentes entradas em um vetor de tokens de acordo com a ordem de
 *  entrada.
 * Retorna um vetor de tokens (char**)
 **/
char **parse_line(char *line, int *token_count)
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
  *token_count = i; // Variavel que controla a quantidade de tokens

  return tokens;
}

/**
 * Dado um vetor de tokens, trata e executa as acoes requisitadas
 * Nao ha retorno
 **/
void parse_tokens(char **tokens, int token_count)
{
  int i = 0, argc = 0, proc_c = 0;
  char *curr_args[token_count + 1];
  memset(curr_args, 0, (token_count + 1) * sizeof(char *));

  // Sem comandos, nao ha o que ser manipulado
  if (tokens[0] == NULL || strlen(tokens[0]) == 0)
  {
    return;
  }
  else if (strcmp(tokens[0], "exit") == 0)
  {
    exit(EXIT_SUCCESS); // Chamada para saida do shell
  }

  // Nao e nenhum dos casos anteriormente tratados
  job curr_job = initialize_job_struct();
  proc curr_proc;
  while (tokens[i] != NULL)
  {
    if (strcmp(tokens[i], "&") == 0 || strcmp(tokens[i], "|") == 0 || strcmp(tokens[i], ">") == 0)
    {
      // Definindo dados do proc atual
      curr_proc = initialize_proc_struct();
      curr_proc.args = curr_args;
      curr_proc.argc = argc;

      // Adicionando o proc a lista do job
      add_proc_to_job(&curr_job, &curr_proc);

      if (strcmp(tokens[i], "&") == 0)
      {
        is_background = 1;
        execute_commands(&curr_job);
        return;
      }
    }

    curr_args[i] = tokens[i];
    argc += 1;

    i++;
  }

  // Definindo dados do proc atual
  curr_proc = initialize_proc_struct();
  curr_proc.args = curr_args;
  curr_proc.argc = argc;

  // Adicionando o proc a lista do job
  add_proc_to_job(&curr_job, &curr_proc);

  execute_commands(&curr_job);
  destroy_job_struct(&curr_job);
}

int execute_commands(job *curr_job)
{
  int status;
  pid_t result, pid_filho;

  for (int i = 0; i < curr_job->processes_count; i++)
  {
    result = fork();

    if (result == -1)
    {
      perror("Erro na execucao do comando fork");
      exit(0);
    }

    if (result == 0)
    { // este é o processo filho

      // sobrepoe o processo atual com um novo processo
      execvp(curr_job->processes[i].args[0], curr_job->processes[i].args);
      // Se chegou até aqui, é porque o exec falhou
      printf("Erro na execucao do execl\n");

      exit(1);
    }
    else
    { // este é o processo pai
      // Se o processo for executado em Background terá a global variable is_background, então o pai não deve esperar
      if (is_background == 0)
      {
        // processo pai vai esperar pelo filho
        pid_filho = wait(&status);
        if (WIFEXITED(status))
          printf("%d terminou com status %d\n", (int)pid_filho, WEXITSTATUS(status));
        else
          printf("Filho não terminou normalmente\n");
      }
    }
  }

  is_background = 0;

  return 0;
}

int main()
{
  char *line = NULL, **tokens;
  int line_arg_count = 0;

  // pai: instala rotina de tratamento do sinal
  signal(SIGCHLD, sighand);

  while (1)
  {
    printf("sample_shell >> $ ");

    line = read_line();
    tokens = parse_line(line, &line_arg_count);
    parse_tokens(tokens, line_arg_count);
    //execute_commands(tokens);

    free(tokens);
  }

  return EXIT_SUCCESS;
}