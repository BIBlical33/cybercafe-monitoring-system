# Cybercafe monitoring system

## Overview
A prototype simulation system for managing a cybercafe.

## Key Features
- Time management;
- Clien handling;
- Table management;
- Event system;
- Financial tracking.

## Prerequisites
- C++20 compatible compiler;
- CMake 3.20;
- make.

## Building the Project
### 1. Clone the repository
```
git clone https://github.com/BIBlical33/cybercafe-monitoring-system.git
cd cybercafe-monitoring-system
```
### 2. Configure with CMake
Windows (Cygwin)/Linux/macOS:
```
mkdir build && cd build
cmake -G "Unix Makefiles" ..
```
Windows (MinGW):
```
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
```
### 3. Build the project
```
cmake --build . --config Release --parallel 4
```

## Running the Application
Create test txt file in `test/` path and use
```
./cybercafe_monitoring_system_run <your test txt file>
```

## TestCase sample
Input:
```
3
09:00 19:00
10
08:48 1 client1
09:41 1 client1
09:48 1 client2
09:52 3 client1
09:54 2 client1 1
10:25 2 client2 2
10:58 1 client3
10:59 2 client3 3
11:30 1 client4
11:35 2 client4 2
11:45 3 client4
12:33 4 client1
12:43 4 client2
15:52 4 client4

```
Output:
```
09:00
08:48 1 client1
08:48 13 NotOpenYet
09:41 1 client1
09:48 1 client2
09:52 3 client1
09:52 13 ICanWaitNoLonger!
09:54 2 client1 1
10:25 2 client2 2
10:58 1 client3
10:59 2 client3 3
11:30 1 client4
11:35 2 client4 2
11:35 13 PlaceIsBusy
11:45 3 client4
12:33 4 client1
12:33 12 client4 1
12:43 4 client2
15:52 4 client4
19:00 11 client3
19:00
1 70 05:58
2 30 02:18
3 90 08:01
```

## Dependencies
- [Google Test](https://github.com/google/googletest) — BSD-3-Clause License
