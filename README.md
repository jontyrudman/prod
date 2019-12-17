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

### TODO

- [x] Read the config file correctly
- [ ] Take language and name arguments
- [ ] Error checking for project names (git repo naming requirements)
- [ ] Use access tokens to set up new repositories
- [ ] Allow the use of arguments to temporarily adjust config for a specific project creation
- [ ] Add more languages
