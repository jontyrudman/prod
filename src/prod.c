#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wordexp.h>
#include <dirent.h>
#include "ini.h"

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
#define INIT_CONFIG(X)                                                         \
  configuration X = {.template_directory = "~/.local/share/prod",              \
                     .execute_scripts = 0,                                     \
                     .remove_scripts = 0,                                      \
                     .use_git = 0,                                             \
                     .git_username = "",                                       \
                     .git_ssh = 0,                                             \
                     .git_provider = "",                                       \
                     .git_access_token = ""}

typedef struct {
  const char *template_directory;
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

  if (MATCH("options", "template_directory")) {
    pconfig->template_directory = strdup(value);
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

int load_config(configuration *config, char *path) {
  wordexp_t exp_result;

  if (ini_parse(path, handler, config) < 0) {
    printf("Can't load config, falling back to defaults\n");
    return 0;
  }

  /* Expand template_directory */
  wordexp(strdup(config->template_directory), &exp_result, 0);
  config->template_directory = strdup(exp_result.we_wordv[0]);
  wordfree(&exp_result);

  /* purely for testing */
  printf("Config %s loaded:\n%s\n%d\n%d\n%d\n%s\n%d\n%s\n%s\n", path,
         config->template_directory, config->execute_scripts,
         config->remove_scripts, config->use_git, config->git_username,
         config->git_ssh, config->git_provider, config->git_access_token);
  return 0;
}

void help_page() {
  printf("Usage: prod [OPTION]... [PROJECT NAME] [LANGUAGE]\n");
  printf("Creates a project structure from a template. Can be used to set "
         "up a git repo for it, too.\n\n");
  printf("  -h --help               Display this help page\n");
  printf("  -l --list               List all languages there are templates "
         "for\n");
  printf("  -c --config [STRING]    Set the config file for this "
         "operation\n");
}

/* TODO:  Implement listing of languages */
void list_languages(configuration *config) {
  DIR *dp;
  struct dirent *ep;

  dp = opendir(config->template_directory);
  printf("Languages available:\n");

  if (dp != NULL) {
    while ((ep = readdir(dp)) != NULL) {
      if (ep->d_type == DT_DIR &&
          !(strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)) {
        printf("%s ", ep->d_name);
      }
    }

    (void)closedir(dp);
    printf("\n");
  } else
    perror("Couldn't open the directory");
}

int main(int argc, char *argv[]) {
  char *config_path = "~/.config/prod/config";
  wordexp_t exp_result;

  /* Initialise config to default values */
  INIT_CONFIG(config);

  /* Validate arguments */
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      help_page();
      return 1;
    }
    if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0) {
      if (argv[i + 1] == NULL) {
        printf("Missing arguments! See --help for usage.");
        return 1;
      }
      if (argv[i + 1][0] == '-') {
        printf("Missing arguments! See --help for usage.");
        return 1;
      }
      config_path = argv[i + 1];
    }
  }

  /* Expand path */
  wordexp(config_path, &exp_result, 0);
  config_path = strdup(exp_result.we_wordv[0]);
  wordfree(&exp_result);

  load_config(&config, config_path);

  /* Print list of languages available in the template directory */
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--list") == 0) {
      list_languages(&config);
    }
  }

  /* Check, if use_git is true, that there is a username and provider */
  if ((config.use_git == 1) && ((strcmp(config.git_provider, "") == 0) ||
                                (strcmp(config.git_username, "") == 0))) {
    printf("use_git is set to true but you haven't set git_provider or "
           "git_username in the config!\n");
    return 1;
  }

  return 0;
}
