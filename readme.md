

object move
1. EObject::move()
2. lookup **unique** lock
3. insert thread map
4. lookup **unlock** unique 
5. call EObject::onMove()

object remove
1. EObjet::remove()
2. lookup **unique** lock
3. remove thread map
4. lookup remove connection graph
5. lookup **unlock** unique

connect
1. EObject::connect()
2. lookup **unique** lock
3. lookup push connection graph
4. lookup **unique** unlock

emit
1. EObject::emit
2. lookup **shared** lock
3. find connection
4. thread **unique** lock
5. lookup push event
6. thread **unique** unlock
7. lookup **shared** unlock

step
1. EObject::emit
2. lookup **shared** lock
4. thread **unique** lock
5. execute event
6. thread **unique** unlock
7. lookup **shared** unlock
