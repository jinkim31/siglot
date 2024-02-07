
## TODO
- thread name
- thread status(n events in queue, queue push/pop ratio)

## Thread-safe Procedures
### Object Move
1. Global mutex **unique** lock
2. insert thread map
3. Global mutex **unlock** unique 
4. call EObject::onMove()

### Object Remove
1. Global mutex **unique** lock
2. Remove from lookup object list
3. Remove related connections from the connection graph
4. Global mutex **unlock** unique

### Signal-slot Connection
1. Global mutex **unique** lock
2. lookup push connection graph
3. Global mutex **unique** unlock

### Signal Emission
1. Global mutex **shared** lock
2. Find connections with matching signal
3. Thread event queue **unique** lock
4. Push event to thread event queue
5. Thread event queue **unique** unlock
6. Global mutex **shared** unlock

### Thread Step
1. Global mutex **shared** lock
2. Thread event queue **unique** lock
3. Execute event
4. Thread event queue **unique** unlock
5. Global mutex **shared** unlock
