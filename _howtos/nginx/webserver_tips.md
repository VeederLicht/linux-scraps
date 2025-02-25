> FROM LYCHEE DOCS

## Directory Permissions

### Permissions of certain directories must be set correctly for Lychee to run. Note that there are up to three OS users which need to be taken into consideration:

The user who is running the Web server daemon; called the Web user in the following. Depending on your distribution and web server typically this user is called apache, nginx, www or www-data.
(Optional) The user who is running the PHP FPM (or PHP CGI) pool; called the PHP user in the following. Not every setup uses PHP FPM or PHP CGI. For example, Apache executes the PHP interpreter as part of its own process if the extension mod_php is used. In case PHP FPM is used, for most distributions PHP FPM or CGI ships with a sensible default configuration which ensures that the PHP user is the same as the Web user. In both cases the PHP user is not of concern. However, we are at least aware of one exception to this rule for Nginx on Fedora, see FAQ
The user which you use for shell logins and to run scripts; called the CLI user in the following. This user may be of particular concern, if you are planning to upload photos via the web interface and import photos via the shell scripts.

### The following permissions must be granted at least:

All directories and files of the Lychee installation must at least be readable by the Web and PHP user.
Directories and files within the storage/ and the bootstrap/cache/ directories must be writable by the Web and PHP user.
If you wish to customise the CSS via the web frontend, directories and files within the public/dist directory must be writable by the Web and PHP user.
Directories and files within the public/uploads and public/sym must at least be writeable by the Web and PHP user. If you only intend to upload files via the web frontend, this is sufficient. If you also plan to use the CLI to import files, the directories and files must additionally be writeable by the CLI user. It is not sufficient, if only already existing directories are writeable by the Web, PHP and CLI user; all directories and files yet to be created must be so, too. The recommended way is to ensure that all three users are at least members of one joined group, e.g., you may add your CLI user to the group used by the Web and PHP user. The directories and files then need to owned by this group and be group writeable. Further, the special sgid bit needs to be set for all directories. This ensures that newly created directories and files become owned by the joined group and not by the primary group of the creator.
