Clean software packages:
`sudo pacman -Scc`

Remove all old keys:
`sudo rm -rf /etc/pacman.d/gnupg`

Reinitialize files & folders for keys:
`sudo pacman-key --init`

Repopulate keys:
`sudo pacman-key --populate archlinux manjaro`

Reinstall latest keyrings:
`sudo pacman -Sy archlinux-keyring manjaro-keyring`
(gnupg eruitgehaald, die gaf problemen)

Refresh the signature keys:
`sudo pacman-key --refresh-keys`
(deze stap duurde erg lang en leek niet iets toe te voegen)

Update:
`sudo pacman -Syu --noconfirm`
(-Syyu gaf vreemd genoeg problemen)
