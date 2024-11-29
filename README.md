# HTTP_SERVER

A simple HTTP server written in C++ (just for educational purposes)

> [!WARNING]
> Do not use this for anything productive as it is not build to be secure
> The server does not check if a user is allowed to request a file with `GET`
> It also does not check if a user is allowed to delete a file with `DELETE`

> [!NOTE]
> The `POST` request does not support append operations yet but may in the
 future.

## Supported request methods

- HEAD
- GET
- POST
- PUT
- DELETE

## Compile the server

1. Run `make` in the projects root directory
2. Navigate into the build directory and execute the output executable with
 `./output`

> [!TIP]
> The server supports optional flags for the ipaddress and port used.
> Run `./output -h` for more info
> ```txt
> Usage: HTTP-Server [--help] [--version] [--ipaddress VAR] [--port VAR]
>
> Optional arguments:
>   -h, --help       shows help message and exits
>   -v, --version    prints version information and exits
>   -i, --ipaddress  The IP-Address of the HTTP-Server [nargs=0..1] [default: "127.0.0.1"]
>   -p, --port       The port you want the HTTP server to be open at. [nargs=0..1] [default: 8080]
> ```

## Usage

After compiling the server open your browser of choice. Navigate to the server
 address (by default 127.0.0.1:8080 or localhost:8080).
Now you can open the developertools and send some requests via

```js
// Simple HEAD request
fetch("http://localhost:8080/some_file.txt", {method : "Head"})

// Simple GET request
fetch("http://localhost:8080/some_file.txt", {method : "Get"})

// Simple POST request
fetch("http://localhost:8080/some_file.txt", {
        method : "Post",
        body: "Hello World!"
    }
)

// Simple PUT request
fetch("http://localhost:8080/some_file.txt", {
        method : "Put",
        body: "This is the new content of some_file.txt"
    }
)

// Simple DELETE request
fetch("http://localhost:8080/some_file.txt", {method : "Delete"})
```

and watch what happens in the CLI as well as the dev-console and network tab.

> [!TIP]
> If the browser is not displaying anything but you have an index.html file
> please make shure that it is in the folder you called the executable from.
