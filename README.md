Lockstep synchronization
========================

This is an implementation of the lockstep synchronization architecture. Lockstep is often used in games for synchronizing large world states in real-time across jittery networks like the internet.

I wrote this for fun and I have not taken the time to make it 100% production-ready. It should be pretty fast though. It easily syncs a world with 1024 units between four players almost without registering on my CPU or network monitors (Early 2013 Macbook Pro).

I would not recommend using the code in your own project. Rather, I hope a few people can learn from it by studying it.

Have a look at this demo configured to 4 players and 1024 units: [GIF](http://imgur.com/jvvcqVE).

OSX instructions
----------------

The project uses good old `make`. Before you can call `make` you must create the file `osx_project/MakefileSettings` with something like the following:

```
BUILD_DIR = ~/Build/Lockstep
```

Use this file to specify where you want the products to be built.

Then you can build all the targets via the command `make [TARGET_NAME]`. Your current working dir must be `osx_project`. Here's the most important targets:

* all = build all targets
* debug_server
* release_server
* debug_client
* release_client
* test
* rt = compile and run unit tests
* rdp = compile and run debug pair (pair means both client and server)
* rdc = compile and run debug client
* rds = compile and run debug server
* rds = compile and run debug server
* rrc = compile and run release client
* rrc = compile and run release client

Per default the server only expects one player. For example, you can write `make PLAYER_COUNT=4 rrs` to make it wait for 4 players/clients. The `PLAYER_COUNT` param can also be passed to the resulting server binary of course.

Other platforms
---------------

Since this is just a project I made for fun I have not added support for other platforms. But the code is arranged so that it should be very easy to do. For example have a look at `client/osx_main.mm`. This file *only* implements the platform "wrapper" and everything else is delegated to platform agnostic logic.

More info
---------

For more information about lockstep synchronization I recommend these:

* http://gafferongames.com/networked-physics/deterministic-lockstep/
* http://www.gamasutra.com/view/feature/131503/1500_archers_on_a_288_network_.php
