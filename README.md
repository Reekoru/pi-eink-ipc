# pi-eink-ipc
This repo is to support the other repo pi-eink. This is communicates with the server to query and mutate the data using Python.

## Python
Python is used to communicate with the server through GraphQL. It processes the responses to prepare the data for processing. Python does not control the server, it acts a the middle man for queries and mutations.

## C
C is what what sends the commands to Python to fetch or mutate anything in the server. The reason why C is used is because the pi-eink repo is an embedded project written in C in the Raspberry Pi Zero W 1/2. This is to make the program run in one process.
