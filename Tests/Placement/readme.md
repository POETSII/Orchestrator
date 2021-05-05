These example applications interact with the placement system. Each example
has:

 - An application file to load.

 - A hardware description file to place with.

 - A set of Orchestrator commands to run the test, which assumes the above two
   files are in the root directory of the Orchestrator repository.

 - A device-map dump to be used as a comparison, to verify the outcome of the
   test.

Directory
---

001: Spread-filling placement example with an application containing three
     device types. The hardware model has five (unpaired) cores, each with one
     thread.

002: As with 001, but each core has three threads instead of one.

003: As with 002, but with:
      - double the number of each device type,
      - double the number of mailboxes,
      - four cores per mailbox (instead of five),
      - core-pairing enabled.

004: As with 003, but all devices are the same type. 15 devices per core.

005: As with 004, but we constrain the number of threads used per core. Still
     15 devices per core, but the thread distribution is different.
