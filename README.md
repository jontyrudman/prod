**Abandoned: this was a cute little project but I don't recommend anyone use it.**
Most languages nowadays have various build systems that will set you up nicely, and every project is different.
Writing this in C was borderline masochistic and lead to some corner-cutting to make things work without yet more platform-dependent libraries.

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

`prod [PROJECT NAME] [LANGUAGE]` will:

1. copy a language-specific boilerplate directory structure into a new directory with the name of the project,
2. use a config file and otherwise questions to determine whether the user would like to run setup scripts and remove them,
3. as well as create a new GitLab/GitHub repo based on the config/flags.

## Requirements

Currently only works on Linux and should work on macOS, but untested.
You'll need to have:

- `git`
- `curl`
- `rename` (the basic util-linux version will do)
- `sed`

## Usage

1. Clone the repository and put `prod` somewhere in your `PATH`.
2. Unpack the config and template folders in `defaults/` to their respective folders in your home directory.
3. Read the config file comments and set up accordingly.
4. If you want to use `git` with `prod`, set the `PROD_TOKEN` variable to your GitLab/GitHub access token for running `prod`.

## TODO

- [ ] Remove template setup script in new project directory
- [ ] Make install script
- [ ] Compile versions and tag the first release
- [ ] Add more languages
- [ ] Support Windows
