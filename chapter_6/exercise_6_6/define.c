#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define HASH_SIZE 101
#define MAX_LINE_LEN 1000
#define MAX_WORD_LEN 100

enum boolean
{
  FALSE,
  TRUE
};

struct list_node
{
  char *name;
  char *definition;
  struct list_node *next;
};

// There is a strdup available with POSIX, but it's not part of ISO C.
char *str_dup(char *src);

size_t hash(char *str);
struct list_node *lookup(char *str);
struct list_node *install(char *name, char *definition);
enum boolean undef(char *name);

void consume_blanks(void);
int get_word(char *word, int max_word_len);
int get_number_str(char *number_str, int max_str_len);

char *test = "#define MAX";

static struct list_node *hash_table[HASH_SIZE];

int main(void)
{
  enum boolean inside_str_literal = FALSE;
  enum boolean inside_char_literal = FALSE;

  char word[MAX_WORD_LEN];
  char definition[MAX_WORD_LEN];
  while (get_word(word, MAX_WORD_LEN) != EOF)
  {
    if (isalpha(word[0]))
    {
      struct list_node *node_p;
      if ((node_p = lookup(word)) != NULL)
      {
        printf("%s", node_p->definition);
      }
      else
      {
        printf("%s", word);
      }
    }
    else
    {
      // TODO: A better approach might be to replace the following code with a
      // separate function that only gets the definition. After getting the
      // definition the preprocessor directive can be printed if necessary.
      if (word[0] == '#' && !inside_str_literal && !inside_char_literal)
      {
        putc(word[0], stdout);

        if (get_word(word, MAX_WORD_LEN) == EOF)
        {
          break;
        }
        else if (!isalpha(word[0]))
        {
          printf("Error: invalid preprocessor directive.\n");
          break;
        }

        if (strcmp(word, "define") == 0)
        {
          printf("%s", word);
          consume_blanks();

          if (get_word(word, MAX_WORD_LEN) == EOF)
          {
            break;
          }
          else if (!isalpha(word[0]))
          {
            printf("Error: invalid name.\n");
            break;
          }
          strcpy(definition, word);

          printf("%s", word);
          consume_blanks();

          if (get_word(word, MAX_WORD_LEN) == EOF)
          {
            break;
          }
          else
          {
            if (isdigit(word[0]))
            {
              ungetc(word[0], stdin);
              get_number_str(word, MAX_WORD_LEN);
            }
            else if (!isalpha(word[0]))
            {
              printf("Error: invalid define declaration.\n");
              break;
            }
          }

          printf("%s", word);
          install(definition, word);
        }
        else
        {
          printf("%s", word);
        }
      }
      else
      {
        if (word[0] == '"')
        {
          inside_str_literal = !inside_str_literal;
        }

        if (word[0] == '\'')
        {
          inside_char_literal = !inside_char_literal;
        }

        putc(word[0], stdout);
      }
    }
  }

  return EXIT_SUCCESS;
}

char *str_dup(char *src)
{
  char *dest = (char *)malloc(strlen(src) + 1);
  if (dest != NULL)
  {
    strcpy(dest, src);
  }
  return dest;
}

size_t hash(char *str)
{
  size_t hash_value = 0;
  while (*str != '\0')
  {
    hash_value = *str + 31 * hash_value;
    ++str;
  }
  return hash_value % HASH_SIZE;
}

struct list_node *lookup(char *str)
{
  struct list_node *node_p;
  for (node_p = hash_table[hash(str)]; node_p != NULL; node_p = node_p->next)
  {
    if (strcmp(str, node_p->name) == 0)
    {
      return node_p;
    }
  }
  return NULL;
}

struct list_node *install(char *name, char *definition)
{
  struct list_node *node_p;
  if ((node_p = lookup(name)) == NULL)
  {
    node_p = (struct list_node *)malloc(sizeof(*node_p));
    if (node_p == NULL || (node_p->name = str_dup(name)) == NULL)
    {
      return NULL;
    }
    size_t hash_value = hash(name);
    node_p->next = hash_table[hash_value];
    hash_table[hash_value] = node_p;
  }
  else
  {
    free(node_p->definition);
  }

  if ((node_p->definition = str_dup(definition)) == NULL)
  {
    return NULL;
  }

  return node_p;
}

enum boolean undef(char *name)
{
  struct list_node *node_p;
  struct list_node *prev_node_p;
  size_t hash_value = hash(name);
  for (node_p = hash_table[hash_value], prev_node_p = NULL;
       node_p != NULL;
       prev_node_p = node_p, node_p = node_p->next)
  {
    if (strcmp(name, node_p->name) == 0)
    {
      free(node_p->name);
      free(node_p->definition);

      if (prev_node_p == NULL)
      {
        hash_table[hash_value] = node_p->next;
      }
      else
      {
        prev_node_p->next = node_p->next;
      }

      free(node_p);
      return TRUE;
    }
  }

  return FALSE;
}

void consume_blanks(void)
{
  int c;
  while ((c = getc(stdin)) && isblank(c))
  {
    putc(c, stdout);
  }
  ungetc(c, stdin);
}

int get_word(char *word, int max_word_len)
{
  int c = getc(stdin);
  size_t i = 0;

  if (c != EOF)
  {
    word[i++] = c;
  }

  if (!isalpha(c) && c != '_')
  {
    word[i] = '\0';
    return c;
  }

  while ((isalnum(c = getc(stdin)) || c == '_') && i < max_word_len)
  {
    word[i++] = c;
  }
  ungetc(c, stdin);
  word[i] = '\0';

  return word[0];
}

int get_number_str(char *number_str, int max_str_len)
{
  int c = getc(stdin);
  size_t i = 0;

  if (c != EOF)
  {
    number_str[i++] = c;
  }

  if (!isdigit(c))
  {
    number_str[i] = '\0';
    return c;
  }

  while (isdigit(c = getc(stdin)) && i < max_str_len)
  {
    number_str[i++] = c;
  }
  ungetc(c, stdin);
  number_str[i] = '\0';

  return number_str[0];
}