# atbrokerlink/network.c — bug review

Generated 2026-11-19 from `atlantis/to-do-fixt-list.md`. **3 findings** (severity H: 0; confidence H and severity H: 0; on the priority list: 0), plus 1 lower-confidence notes.

Line numbers in **Listed lines** are from the review (10–12 Oct 2026). The code shown is from the working tree on 2026-11-19. Where the code has moved, the snippet follows it and says so. ⚠ means the quoted code could not be found: it may have been changed or already fixed.

Conf/Sev: confidence it's a bug / severity if it is (H = wrong results or crash, M = wrong in some configs, L = cosmetic, log, leak or latent). ✔ = the reviewer re-checked it by hand. ⭐ = on the high-priority list.

## Summary

| ID | Listed lines | Conf/Sev | Category | Short description | Decision | Done |
|---|---|---|---|---|---|---|
| [LINK-65](#link-65) | 19-21 | H/M | unchecked return | The return values of `zmq_init`, `zmq_socket` and `zmq_bind` are ignored. A bind failure (port in use, bad URL… |  | [ ] |
| [LINK-66](#link-66) | 31-40 | H/M | error path continues | When `zmq_recv` returns -1 the code prints an error and then still builds a message from the uninitialised/emp… |  | [ ] |
| [LINK-67](#link-67) | 52-61 | M/L | leak / unchecked | `zmq_msg_init_size` result unchecked. On a send failure it returns without `zmq_msg_close(&response)`. Functio… |  | [ ] |

---

## LINK-65

**unchecked return** · Conf H / Sev M  
**Listed lines:** 19-21

**Function:** `Network_Initialise()`

**Why it's thought to be a bug**

The return values of `zmq_init`, `zmq_socket` and `zmq_bind` are ignored. A bind failure (port in use, bad URL) leaves Atlantis blocking forever in recv with no diagnostic. Function: `Network_Initialise()`.

**Current code** (`atbrokerlink/network.c` lines 17–23)

```c
   17: void Network_Initialise(Network *network, char *modelurl) {
   18: 	network->modelURL = strdup(modelurl);
   19: 	network->context = zmq_init(1);
   20: 	network->socket = zmq_socket(network->context, ZMQ_REP);
   21: 	zmq_bind(network->socket, network->modelURL);
   22: }
   23: 
```

**Suggested fix (from review):** Check each for NULL/-1, then printErrorMessage and quit.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## LINK-66

**error path continues** · Conf H / Sev M  
**Listed lines:** 31-40

**Function:** `Network_Receive()`

**Why it's thought to be a bug**

When `zmq_recv` returns -1 the code prints an error and then still builds a message from the uninitialised/empty msg (size 0, `malloc(0)` can be NULL), and closes it. The caller cannot tell a failure from a valid message. The malloc results are not checked. Function: `Network_Receive()`.

**Current code** (`atbrokerlink/network.c` lines 29–42)

```c
   29: 
   30: 	zmq_msg_init(&request);
   31: 	result = zmq_recv(network->socket, &request, 0);
   32: 	if (result == -1) {
   33: 		printErrorMessage("Network_Receive()");
   34: 	}
   35: 
   36: 	messageReceived = (Message *) malloc(sizeof(Message));
   37: 	messageReceived->size = zmq_msg_size(&request);
   38: 	messageReceived->bytes = malloc(sizeof(void) * (messageReceived->size));
   39: 	memcpy(messageReceived->bytes, zmq_msg_data(&request),
   40: 			messageReceived->size);
   41: 
   42: 	zmq_msg_close(&request);
```

**Suggested fix (from review):** On error, close the msg and return NULL; check the mallocs.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## LINK-67

**leak / unchecked** · Conf M / Sev L  
**Listed lines:** 52-61

**Function:** `Network_Send()`

**Why it's thought to be a bug**

`zmq_msg_init_size` result unchecked. On a send failure it returns without `zmq_msg_close(&response)`. Function: `Network_Send()`.

**Current code** (`atbrokerlink/network.c` lines 50–63)

```c
   50: 
   51: 	/* Send the data */
   52: 	zmq_msg_init_size(&response, message->size);
   53: 	memcpy(zmq_msg_data(&response), message->bytes, message->size);
   54: 	result = zmq_send(network->socket, &response, 0);
   55: 
   56: 	/* Check to see if we sent successfully */
   57: 	if (result == -1) {
   58: 		printf("error message\n");
   59: 		printErrorMessage("Network_Send()");
   60: 		return -1;
   61: 	}
   62: 	zmq_msg_close(&response);
   63: 
```

**Suggested fix (from review):** Check init and always close the msg.

**Your response** (tick one; add detail in comments)
- [ ] A. Agree it's a bug — apply the suggested fix
- [ ] B. Agree it's a bug — but fix it differently (describe below)
- [ ] C. Not a bug / intended behaviour — leave as is
- [ ] D. Unsure — Claude to investigate further and report back
- [ ] E. Real but defer (low priority / later)

**Your comments:**

> 

**Outcome** _(Claude fills this in after the fix: date, change made, commit batch)_:

---

## Lower-confidence notes (speculative — check before acting)

These came from section B.3 of the list. They were not raised as findings. Tick if you want one followed up.

### LC-1

atbrokerlink/network.c uses the ZeroMQ 2.x API (`zmq_recv(sock, zmq_msg_t*, flags)`, `zmq_init`, `zmq_term`). Against ZeroMQ >= 3 these signatures differ (`zmq_recv` takes a buffer and length), so it will not compile or will misbehave. `zmq_msg_recv` / `zmq_msg_send` / `zmq_ctx_new` are the modern equivalents.

- [ ] Investigate   - [ ] Ignore

**Your comments:**

> 
