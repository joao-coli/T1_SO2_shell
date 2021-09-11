/* Keep track of attributes of the shell.  */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

int main()
{
  char *line = NULL;

  while (1)
  {
    printf("sample_shell >> $ ");

    line = read_line();

    if (strcmp(line, "exit\n") == 0)
    {
      break;
    }
  }

  return EXIT_SUCCESS;
}