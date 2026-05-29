var Module = typeof Module !== "undefined" ? Module : {};

function printCompileHelp() {
  console.error("Command: compile");
  console.error("  Compile a P2WSH spending policy to Miniscript and print spending cost analysis");
  console.error("  and the resulting script structure.");
  console.error("");
  console.error("Usage:");
  console.error("  node miniscript.js compile <policy>");
  console.error("");
  console.error("Example:");
  console.error("  node miniscript.js compile 'and(pk(A),older(1000))'");
  console.error("");
  console.error("Supported policies:");
  console.error("  pk(NAME)                       Require public key NAME to sign; NAME can be up to 16 characters.");
  console.error("  after(NUM), older(NUM)         Require nLockTime/nSequence to be at least NUM; NUM cannot be 0.");
  console.error("  sha256(HEX), hash256(HEX)      Require the preimage of 64-character HEX; H may be used as HEX.");
  console.error("  ripemd160(HEX), hash160(HEX)   Require the preimage of 40-character HEX; H may be used as HEX.");
  console.error("  and(POL,POL)                   Require both subpolicies.");
  console.error("  or([N@]POL,[N@]POL)            Require one subpolicy; N is the relative probability.");
  console.error("  thresh(NUM,POL,POL,...)        Require NUM of the subpolicies; combinations are equally likely.");
  console.error("");
  console.error("Examples:");
  console.error("  A single key:");
  console.error("    node miniscript.js compile 'pk(key_1)'");
  console.error("  One of two keys (equally likely):");
  console.error("    node miniscript.js compile 'or(pk(key_1),pk(key_2))'");
  console.error("  One of two keys (one likely, one unlikely):");
  console.error("    node miniscript.js compile 'or(99@pk(key_likely),pk(key_unlikely))'");
  console.error("  A user and a 2FA service need to sign off, but after 90 days the user alone is enough:");
  console.error("    node miniscript.js compile 'and(pk(key_user),or(99@pk(key_service),older(12960)))'");
  console.error("  A 3-of-3 that turns into a 2-of-3 after 90 days:");
  console.error("    node miniscript.js compile 'thresh(3,pk(key_1),pk(key_2),pk(key_3),older(12960))'");
  console.error("  The BOLT #3 to_local policy:");
  console.error("    node miniscript.js compile 'or(pk(key_revocation),and(pk(key_local),older(1008)))'");
}

function printAnalyzeHelp() {
  console.error("Command: analyze");
  console.error("  Analyze the structure of a Miniscript expression, including type properties, script length,");
  console.error("  max ops, max stack size, and the resulting script.");
  console.error("");
  console.error("Usage:");
  console.error("  node miniscript.js analyze <miniscript>");
  console.error("");
  console.error("Example:");
  console.error("  node miniscript.js analyze 'and_v(v:pk(A),older(1000))'");
  console.error("  Provide a well-typed miniscript expression of type \"B\".");
}

function printUsage() {
  console.error("Miniscript CLI");
  console.error("");
  console.error("Usage:");
  console.error("  node miniscript.js <command> <expression>");
  console.error("");
  console.error("Commands:");
  console.error("  compile <policy>              Compile a policy to Miniscript.");
  console.error("  analyze <miniscript>          Analyze a well-typed Miniscript expression of type \"B\".");
  console.error("");
  console.error("Help:");
  console.error("  node miniscript.js compile help");
  console.error("  node miniscript.js analyze help");
  console.error("");
  printCompileHelp();
  console.error("");
  printAnalyzeHelp();
}

Module["onRuntimeInitialized"] = function () {
  var command = process.argv[2];
  var args = process.argv.slice(3);
  var input = args.join(" ");

  if (command === "--help" || command === "-h" || command === "help") {
    printUsage();
    return;
  }

  if (command === "compile" && (args[0] === "--help" || args[0] === "-h" || args[0] === "help")) {
    printCompileHelp();
    return;
  }

  if (command === "analyze" && (args[0] === "--help" || args[0] === "-h" || args[0] === "help")) {
    printAnalyzeHelp();
    return;
  }

  if ((command !== "compile" && command !== "analyze") || !input) {
    printUsage();
    process.exit(2);
  }

  var msout = Module._malloc(10000);
  var costout = Module._malloc(50000);
  var asmout = Module._malloc(100000);

  try {
    if (command === "compile") {
      var compile = Module.cwrap(
        "miniscript_compile",
        null,
        ["string", "number", "number", "number", "number", "number", "number"]
      );

      compile(input, msout, 10000, costout, 50000, asmout, 100000);

      var miniscript = Module.UTF8ToString(msout);
      var cost = Module.UTF8ToString(costout);
      var asm = Module.UTF8ToString(asmout);

      if (
        miniscript.indexOf("[compile error]") === 0 ||
        miniscript.indexOf("[exception:") === 0
      ) {
        console.log(miniscript);
        process.exitCode = 1;
      } else {
        console.log("Miniscript output:");
        console.log(miniscript);
        console.log("\nSpending cost analysis:");
        console.log(cost);
        if (asm) {
          console.log("\nResulting script structure:");
          console.log(asm);
        }
      }
    } else {
      var analyze = Module.cwrap(
        "miniscript_analyze",
        null,
        ["string", "number", "number", "number", "number"]
      );

      analyze(input, costout, 50000, asmout, 100000);

      var analysis = Module.UTF8ToString(costout);
      var asm = Module.UTF8ToString(asmout);
      console.log(analysis);

      if (asm) {
        console.log("\nResulting script structure:");
        console.log(asm);
      }

      if (
        analysis.indexOf("[analysis error]") === 0 ||
        analysis.indexOf("[exception:") === 0
      ) {
        process.exitCode = 1;
      }
    }
  } finally {
    Module._free(msout);
    Module._free(costout);
    Module._free(asmout);
  }
};
