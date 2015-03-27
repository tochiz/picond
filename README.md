picond
======

Picond(PICO Notify Daemon) is simple network based notification daemon.


Description
-----------

Picond is received message via UDP packets, and show popup window.

    packet
      first  1 byte -> protocol num
      second 1 byte -> message type
      others        -> message with line break "\n" (UTF-8 encoded).
                       first 1 line will be printed bold font(title).

Picond listen `UDP 10514` port.


## Demo

TODO: take and paste some screen shot


## VS. Growl for windows

Growl for windows is too heavy for my PC.
Picond is made to be simple and light.


## Requirement

- Windows Vista or later.
    - Vista(not tested)
    - 7
    - 8
    - 8.1


## Usage

1. start picond
2. send message from some program
3. show some popup and click popup to close


## Contribution

- none

## Licence

- [MIT](https://github.com/tochiz/picond/blob/master/LICENCE)


## Author

- [tochiz](https://github.com/tochiz)
