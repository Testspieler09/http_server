# HTTP_SERVER

A simple HTTP server written in C++ (just for educational purposes)

> [!WARNING]
> Do not use this for anything productive as it is not build with security as the main focus

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

After compiling the server go into the directory you want to call the executable from and create a `server_lists.serverconf` file.
The file is meant to be used by the server to validate if a user is allowed to request a file with get, is allowed to delete a file with a delete request or create / change a file with post and put.
The file should look like this:
```txt
[whitelist]
./index.html

[deletelist]
./data.json

[post_put_list]
./some_file.txt
```

> [!NOTE]
> The files should exist otherwise the server can't work with them, which will lead to error responses.

Start the executable within the folder and open your browser of choice. Navigate to the server
 address (by default 127.0.0.1:8080 or localhost:8080).
Now you can open the developer-tools and send some requests via

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

// Append something to a file
fetch("http://localhost:8080/some_file.txt", {
        method : "Post",
        headers: {
                "Content-Type": "text/plain",
                "Append-Position": "line=1, pos=1"
        },
        body: "This text will be appended to some_file.txt"
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
