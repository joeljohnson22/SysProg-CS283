1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

_The client uses special end-of-message markers like the EOF character (0x04). The client reads data in a loop until it receives this marker, which signals that the complete output has been transmitted. Other techniques include length prefixing (sending message size first), timeout-based approaches, and connection closing._

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

_A networked shell protocol should use explicit delimiters (like NULL or EOF characters) to mark message boundaries. Another approach is to include a header with the message length. Without proper boundary management, messages may be split across multiple reads or combined in a single read, causing command parsing errors, truncated outputs, or security vulnerabilities._

3. Describe the general differences between stateful and stateless protocols.

_Stateful protocols maintain information about previous interactions between client and server (like session data, authentication status). The server remembers client state between requests.

Stateless protocols treat each request as independent, with no memory of previous interactions. Each request must contain all necessary information. Stateless protocols are simpler but may require more data per reques_

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

_UDP is used when speed is more important than reliability. It offers lower latency, less overhead (smaller packet headers, no connection management), better for real-time applications where occasional data loss is acceptable, and etc...

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

_The socket API is the primary abstraction provided by operating systems for network communications. Sockets serve as endpoints for sending and receiving data across networks, supporting different protocols (TCP, UDP) and addressing methods. The API includes functions like socket(), connect(), bind(), listen(), accept(), send(), and recv() that abstract away the low-level details of network protocols and hardware._