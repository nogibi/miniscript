# Node.js CLI Build

Build the single-file Node.js CLI with Emscripten:

```sh
make miniscript-node.js
```

Run it with either command:

```sh
node miniscript.js compile 'and(pk(A),older(1000))'
node miniscript.js analyze 'and_v(v:pk(A),older(1000))'
node miniscript.js help
```

`compile` and `analyze` show the original web output as command-line text.

The generated `miniscript.js` embeds its WebAssembly and is the only file
needed on machines with Node.js.
