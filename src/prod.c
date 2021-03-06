#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <wordexp.h>
#include <sys/stat.h>
#include <sys/types.h>
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
                     .git_access_token = "",                                   \
                     .git_private = 1}
#define GIT_COMMANDS_X 7
#define GIT_COMMANDS_Y 500

typedef struct {
  const char *template_directory;
  int execute_scripts;
  int remove_scripts;
  int use_git;
  const char *git_username;
  int git_ssh;
  const char *git_provider;
  const char *git_access_token;
  int git_private;
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
  } else if (MATCH("git", "git_private")) {
    read_bool(&(pconfig->git_private), value, name);
  } else {
    return 0; /* unknown section/name, error */
  }
  return 1;
}

/* Loads the configuration file at path */
int load_config(configuration *config, const char *path) {
  wordexp_t exp_result;

  if (ini_parse(path, handler, config) < 0) {
    printf("Can't load config, falling back to defaults\n");
    return 0;
  }

  /* Expand template_directory */
  wordexp(strdup(config->template_directory), &exp_result, 0);
  config->template_directory = strdup(exp_result.we_wordv[0]);
  wordfree(&exp_result);

  /* Get access token from environment */
  char *token = getenv("PROD_TOKEN");
  if (token != NULL) {
    config->git_access_token = strdup(token);
  }

  printf("Config at %s loaded.\n", path);
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
  printf("  --nogit                 Disable git integration for this "
         "operation\n");
}

/* List of available templates */
void list_languages(configuration *config) {
  DIR *dp;
  struct dirent *ep;

  dp = opendir(config->template_directory);
  printf("Templates available:\n");

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
    perror("Couldn't open the template directory");
}

/**
 * Lists all files and sub-directories at given path.
 */
char* list_files_recursive(const char *basePath)
{
  char path[1000];
  struct dirent *dp;
  DIR *dir = opendir(basePath);

  // Unable to open directory stream
  if (!dir)
    return "fail";

  while ((dp = readdir(dir)) != NULL)
  {
    if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
    {

      // Construct new path from our base path
      strcpy(path, basePath);
      strcat(path, "/");
      strcat(path, dp->d_name);
      printf("%s\n", path);

      list_files_recursive(path);
    }
  }

  closedir(dir);
  return "success";
}

/* Copy over template directory */
int copy_dir(configuration *config, const char *proj_name,
             const char *template_name) {
  DIR *dp;
  struct dirent *ep;
  char command[300];

  /* Lazy - run mkdir to determine if directory already exists */
  if (mkdir(proj_name, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP |
                           S_IROTH | S_IXOTH) == 0) {
    rmdir(proj_name);
  } else {
    printf("Set up failed. It's likely that the directory '%s' exists in your "
           "current directory.\n",
           proj_name);
    return -1;
  }

  /* An extra forward-slash never hurt anybody */
  sprintf(command, "cp -n -r %s/%s %s", config->template_directory,
          template_name, proj_name);

  dp = opendir(config->template_directory);

  if (dp != NULL) {
    while ((ep = readdir(dp)) != NULL) {
      if (ep->d_type == DT_DIR && strcmp(ep->d_name, template_name) == 0) {
        system(command);
        printf("Created project directory '%s'.\n", proj_name);
        return 0;
      }
    }

    (void)closedir(dp);
  } else
    perror("Couldn't open the template directory");

  printf("Couldn't find template '%s'.\n", template_name);
  return -1;
}

/* Imposes the following constraints on git repo names:
 * - non-alphanum chars other than '-' and '_' are replaced with '-'
 * - all upper-case chars made lower-case
 * - if the project name is longer than 100 chars, cut down to 100
 * - cut off trailing and preceding non-alnum chars to make git paths and names
 *   the same on both github and gitlab
 */
void gen_repo_name(const char *proj_name, char *git_name, int len) {
  int i;
  int j = 0;
  int new_len = len;

  /* Shorten new_len to the last alnum in git_name */
  for (i = len - 1; i > -1; i--) {
    if (!isalnum(proj_name[i])) {
      new_len--;
    } else {
      break;
    }
  }

  for (i = 0; i < new_len; i++) {
    if (isalnum(proj_name[i])) {
      git_name[j] = tolower(proj_name[i]);
      j++;
    } else if ((proj_name[i] == '-' || proj_name[i] == '_') && j != 0) {
      git_name[j] = proj_name[i];
      j++;
    } else if (j != 0) {
      git_name[j] = '-';
      j++;
    }
  }

  git_name[j] = '\0';
}

int git_opts_valid(configuration *config, char **repo_name) {
  if ((strcmp(config->git_provider, "") == 0) ||
      (strcmp(config->git_username, "") == 0) ||
      (strcmp(config->git_access_token, "") == 0)) {
    printf(
        "use_git is set to true but you haven't set either the git_provider, "
        "git_username in the config, or the PROD_TOKEN environment "
        "variable!\n");
    free(*repo_name);
    *repo_name = NULL;
    return 0;
  }

  return 1;
}

/* Generates all of the system commands for setting up a git repository. */
int gen_git_commands(configuration *config,
                     char commands[GIT_COMMANDS_X][GIT_COMMANDS_Y],
                     char **repo_name) {
  char *github_private = config->git_private == 1 ? "true" : "false";
  char *gitlab_private = config->git_private == 1 ? "private" : "public";

  if (strcmp(config->git_provider, "github") == 0) {
    sprintf(commands[0],
            "curl -u '%s:%s' https://api.github.com/user/repos -d "
            "'{\"name\":\"%s\", \"private\":\"%s\"}' > response.json",
            config->git_username, config->git_access_token, *repo_name,
            github_private);
  } else if (strcmp(config->git_provider, "gitlab") == 0) {
    sprintf(commands[0],
            "curl -H \"Content-Type:application/json\" "
            "https://gitlab.com/api/v4/projects?private_token=%s -d '{ "
            "\"name\": \"%s\", \"visibility\": \"%s\" }' > response.json",
            config->git_access_token, *repo_name, gitlab_private);
  } else {
    printf("Your git_provider needs to be 'gitlab' or 'github'.\n");
    free(repo_name);
    repo_name = NULL;
    return 0;
  }
  strcpy(commands[1], "touch README.md");
  strcpy(commands[2], "git init .");
  strcpy(commands[3], "git add .");
  strcpy(commands[4], "git commit -m \"Initial commit\"");
  if (config->git_ssh == 1) {
    sprintf(commands[5], "git remote add origin git@%s.com:%s/%s.git",
            config->git_provider, config->git_username, *repo_name);
  } else {
    sprintf(commands[5], "git remote add origin https://%s.com/%s/%s.git",
            config->git_provider, config->git_username, *repo_name);
  }
  strcpy(commands[6], "git push -u origin master");
  return 1;
}

int git_repo_exists(configuration *config,
                    char commands[GIT_COMMANDS_X][GIT_COMMANDS_Y]) {
  if (system(commands[0]) == -1) {
    printf("Creation and initialisation of git repository failed.\n");
    return 0;
  }
  char buffer[150];
  FILE *fptr;
  if ((fptr = fopen("response.json", "r")) == NULL) {
    printf("Error opening file\n");
    // Program exits if file pointer returns NULL.
    return -1;
  }

  if (strcmp(config->git_provider, "github") == 0) {
    fscanf(fptr, "%150[^\n]\n", buffer);
    fscanf(fptr, "%150[^\n]\n", buffer);
    fclose(fptr);
    if (strcmp(buffer, "\"message\": \"Repository creation failed.\",") == 0) {
      printf("Repository already exists!\n");
      return 1;
    }
  } else if (strcmp(config->git_provider, "gitlab") == 0) {
    fscanf(fptr, "%150[^\n]\n", buffer);
    fclose(fptr);
    if (strcmp(buffer, "{\"message\":{\"name\":[\"has already been "
                       "taken\"],\"path\":[\"has already been "
                       "taken\"],\"limit_reached\":[]}}\0") == 0) {
      printf("Repository already exists!\n");
      return 1;
    }
  }
  return 0;
}

int new_git_repo(configuration *config, const char *proj_name) {
  int repo_name_len = strlen(proj_name) > 100 ? 100 : strlen(proj_name);
  char *repo_name = calloc(repo_name_len + 1, sizeof(char));
  char commands[GIT_COMMANDS_X][GIT_COMMANDS_Y];

  gen_repo_name(proj_name, repo_name, repo_name_len);

  if (!git_opts_valid(config, &repo_name))
    return -1;

  if (!gen_git_commands(config, commands, &repo_name))
    return -1;

  printf("Attempting to set up repository for %s...\n", repo_name);
  free(repo_name);
  repo_name = NULL;

  if (git_repo_exists(config, commands))
    return -1;

  /* Remove the temporary output */
  remove("response.json");

  for (int i = 1; i < 7; i++) {
    if (system(commands[i]) == -1) {
      printf("Creation and initialisation of git repository failed.\n");
      return -1;
    }
  }

  return 0;
}

int parse_args(int argc, char *argv[], configuration *config,
               const char **proj_name, const char **template_name) {
  wordexp_t exp_result;
  const char *config_path = "~/.config/prod/config";

  /* Look for custom config first */
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0) {
      if (argv[i + 1] == NULL) {
        printf("Missing arguments! See --help for usage.");
        return -1;
      }
      if (argv[i + 1][0] == '-') {
        printf("Missing arguments! See --help for usage.");
        return -1;
      }
      config_path = argv[i + 1];
      /* Skip the next arg */
      i++;
    }
  }

  /* Expand path and cut final slash if there is one, and load config */
  wordexp(config_path, &exp_result, 0);
  load_config(config, strdup(exp_result.we_wordv[0]));
  wordfree(&exp_result);

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      help_page();
      return 1;
    }
    if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--list") == 0) {
      list_languages(config);
    }
    if (argv[i][0] != '-') {
      *proj_name = argv[i];
      if (argv[i + 1] == NULL) {
        printf("Missing template name! See --help for usage.\n");
        return -1;
      }
      if (argv[i + 1][0] == '-') {
        printf("Missing template name! See --help for usage.\n");
        return -1;
      }
      *template_name = argv[i + 1];
      /* Skip the next arg */
      i++;
    }
    if (strcmp(argv[i], "--nogit") == 0) {
      config->use_git = 0;
    }
  }
  return 0;
}

void replace_proj_name(const char **proj_name) {
  char commands[2][400];

  sprintf(commands[0],
          "find . -type f -name '*' -exec sed -i "
          "'s/insert_proj_name/%s/g' {} \\+",
          *proj_name);
  sprintf(commands[1], "find . -execdir rename insert_proj_name %s {} \\+",
          *proj_name);

  system(commands[0]);
  system(commands[1]);
}

/* Runs setup.sh in cwd */
void run_setup_script() {
  char user_input;
  /* try to open file to read */
  FILE *file;
  file = fopen("./setup.sh", "r");
  if (file) {
    fclose(file);
    printf("A setup script has been found! Would you like to run setup.sh in the new directory? (Y/n): ");
    user_input = fgetc(stdin);
    if (user_input == 'n' || user_input == 'N') {
      printf("Not running setup script.\n");
    } else {
      printf("Running setup script...\n");
      system("/bin/sh ./setup.sh");
    }
  } else {
    printf("No setup script found.\n");
  }
}

int main(int argc, char *argv[]) {
  const char *proj_name = "";
  const char *template_name = "";
  int result = 0;

  /* Initialise config to default values */
  INIT_CONFIG(config);

  /* Validate arguments */
  result = parse_args(argc, argv, &config, &proj_name, &template_name);
  if (result == -1)
    return EXIT_FAILURE;
  if (result == 1)
    return 0;

  if (strlen(proj_name) == 0 || strlen(template_name) == 0) {
    printf(
        "Missing project name and/or template name! See --help for usage.\n");
    return EXIT_FAILURE;
  }
  result = copy_dir(&config, proj_name, template_name);
  if (result == -1)
    return EXIT_FAILURE;

  chdir(proj_name);
  replace_proj_name(&proj_name);
  run_setup_script();

  /* If use_git is true, attempt to create and init a new repo */
  if (config.use_git == 1) {
    result = new_git_repo(&config, proj_name);
  }
  if (result == -1)
    return EXIT_FAILURE;

  return 0;
}
