```
                                          _/   
     _/_/_/    _/  _/_/    _/_/      _/_/_/    
    _/    _/  _/_/      _/    _/  _/    _/     
   _/    _/  _/        _/    _/  _/    _/      
  _/_/_/    _/          _/_/      _/_/_/       
 _/                                            
_/
```

> Heavier than `touch`; **prod**uce.

This is super WIP, but I thought I'd start on this project today (17th Dec 2019).
Once it's done, `prod [PROJECT NAME] [LANGUAGE]` will:

1. copy a language-specific boilerplate directory structure into a new directory with the name of the project,
2. use a config file and otherwise questions to determine whether the user would like to run setup scripts and remove them,
3. as well as create a new GitLab/GitHub repo based on the config/flags.

## Usage

1. Clone the repository and put `prod` somewhere in your `PATH`.
2. Unpack the config and template folders in `defaults/` to their respective folders in your home directory.
3. Read the config file comments and set up accordingly.
4. If you want to use `git` with `prod`, set the `PROD_TOKEN` variable to your GitLab/GitHub access token for running `prod`.

## TODO

- [x] Read the config file correctly
- [x] Take language and name arguments
- [x] Error checking for project names (git repo naming requirements)
- [x] Use access tokens to set up new repositories
- [x] Allow the use of arguments to temporarily adjust config for a specific project creation
- [x] Reduce function length and handle command length more safely
- [x] Develop more discreet access token storage
- [x] Add a --nogit flag
- [x] Find and replace occurrences of `insert_proj_name` in both file names and Makefiles
- [ ] Run scripts in the root-level of a template directory after copying
- [ ] Add more languages
- [ ] Support Windows