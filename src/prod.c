#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ini.h"

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
#define INIT_CONFIG(X)                                                         \
  configuration X = {.editorconfig = 1,                                        \
                     .execute_scripts = 0,                                     \
                     .remove_scripts = 0,                                      \
                     .use_git = 0,                                             \
                     .git_username = "",                                       \
                     .git_ssh = 0,                                             \
                     .git_provider = "",                                       \
                     .git_access_token = ""}

typedef struct {
  int editorconfig;
  int execute_scripts;
  int remove_scripts;
  int use_git;
  const char *git_username;
  int git_ssh;
  const char *git_provider;
  const char *git_access_token;
} configuration;

/* Read a boolean in string form into an integer.
 * The integer is set to -1 if invalid string */
void read_bool(int *field, const char *value, const char *name) {
  if (strcmp("true", value) == 0) {
    *field = 1;
  } else if (strcmp("false", value) == 0) {
    *field = 0;
  } else {
    printf("%s has an invalid boolean (use 'true' or 'false'), falling back to "
           "default.\n",
           name);
  }
}

/* The ini_handler given to ini_parse, containing logic
 * for picking up the necessary keys */
static int handler(void *user, const char *section, const char *name,
                   const char *value) {
  /* Pointer to the config file 'user' */
  configuration *pconfig = (configuration *)user;

  if (MATCH("options", "editorconfig")) {
    read_bool(&(pconfig->editorconfig), value, name);
  } else if (MATCH("options", "execute_scripts")) {
    read_bool(&(pconfig->execute_scripts), value, name);
  } else if (MATCH("options", "remove_scripts")) {
    read_bool(&(pconfig->remove_scripts), value, name);
  } else if (MATCH("git", "use_git")) {
    read_bool(&(pconfig->use_git), value, name);
  } else if (MATCH("git", "git_username")) {
    pconfig->git_username = strdup(value);
  } else if (MATCH("git", "git_ssh")) {
    read_bool(&(pconfig->git_ssh), value, name);
  } else if (MATCH("git", "git_provider")) {
    pconfig->git_provider = strdup(value);
  } else {
    return 0; /* unknown section/name, error */
  }
  return 1;
}

int load_config(configuration *config) {
  if (ini_parse("/home/jonty/.local/share/prod/config", handler, config) < 0) {
    printf("Can't load config, falling back to defaults\n");
    return 0;
  }
  /* purely for testing */
  printf("Config loaded:\n%d\n%d\n%d\n%d\n%s\n%d\n%s\n%s\n",
         config->editorconfig, config->execute_scripts, config->remove_scripts,
         config->use_git, config->git_username, config->git_ssh,
         config->git_provider, config->git_access_token);
  return 0;
}

int main(int argc, char *argv[]) {
  /* Initialise config to default values */
  INIT_CONFIG(config);
  load_config(&config);

  /* Check, if use_git is true, that there is a username and provider */
  if ((config.use_git == 1) && ((strcmp(config.git_provider, "") == 0) ||
                                (strcmp(config.git_username, "") == 0))) {
    printf("use_git is set to true but you haven't set git_provider or "
           "git_username in the config!\n");
    return 1;
  }

  return 0;
}
