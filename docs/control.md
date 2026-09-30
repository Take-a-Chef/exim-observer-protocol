# Control and inspection

Only QUEUE_COUNT and QUEUE_COUNT_RESPONSE are implemented. A request has a
nonzero uint64 request_id, no body arguments, and sequence zero. A successful
response echoes request_id exactly and carries uint64 queue_count, including zero.
The count is a point-in-time observation; it does not lock the queue or guarantee
that a subsequent query sees the same state. Both require queue-count capability.

Request IDs are connection-scoped and unique among outstanding requests. Responses
may arrive out of order. The requester matches the response type and request_id;
unmatched or duplicate responses are rejected by session integration. An ERROR
for a request echoes its ID instead of returning a success response. ERROR without
request_id is reserved for connection/handshake failure. Exactly one terminal
response is expected per request; timeout and retry policies belong to applications.

## Reserved operations

| Command                | Intended future request       | Intended future successful response |
| ---------------------- | ----------------------------- | ----------------------------------- |
| QUEUE_LIST             | bounded filter, cursor, limit | bounded page and next cursor        |
| QUEUE_GET              | exim_id                       | one message metadata record         |
| MESSAGE_FREEZE         | exim_id                       | correlated mutation outcome         |
| MESSAGE_THAW           | exim_id                       | correlated mutation outcome         |
| MESSAGE_REMOVE         | exim_id                       | correlated mutation outcome         |
| MESSAGE_RETRY          | exim_id                       | correlated scheduling outcome       |
| MESSAGE_FORCE_DELIVERY | exim_id                       | correlated scheduling outcome       |
| MESSAGE_GET_HEADERS    | exim_id and bounded selection | bounded header data                 |
| MESSAGE_GET_BODY       | exim_id and bounded range     | bounded body data                   |
| MESSAGE_GET_LOG        | exim_id and bounded range     | bounded log data                    |

All future requests require request_id and their responses echo it. These are
semantic placeholders only: no payload layouts, pagination, mutation idempotency,
or body chunking are standardized yet. Reserved operations must return
UNSUPPORTED_MESSAGE, not be approximated with QUEUE_COUNT. IDs in commands.yaml
are registry IDs; queue/inspection/response wire namespaces are in protocol.md.

A future mutating command needs explicit authorization and a durable idempotency
strategy beyond connection-scoped request_id before implementation. Event replay
identity does not automatically make destructive commands safe to replay.
