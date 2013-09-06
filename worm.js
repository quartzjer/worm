var fs = require("fs");
var tele = require("telehash");
var argv = require("optimist")
  .usage("Usage: type '$0' to receive (prints it's hashname) and 'cat foo.txt | $0 hashdestination' to send")
  .default("id", process.env.HOME + "/.worm.json")
  .default("seeds", "./seeds.json")
  .boolean("v").describe("v", "verbose")
  .argv;

if(argv.v) tele.debug(console.log);
var holehash = argv._[0];
if(holehash && !tele.isHashname(holehash))
{
  console.error("invalid destination hashname (must be 64 hex characters):",holehash);
  process.exit(1);
}

var worm;
var hole;

// load or generate our crypto id
if(fs.existsSync(argv.id))
{
  init(require(argv.id));
}else{
  tele.genkey(function(err, key){
    fs.writeFileSync(argv.id, JSON.stringify(key, null, 4));
    init(key);
  });
}

function init(key)
{
  worm = tele.hashname(key);

  require(argv.seeds).forEach(worm.addSeed);

  worm.online(function(err){
    if(err) {
      console.error(err);
      process.exit(0);
    }
    if(holehash) {
      hole = worm.wrap(worm.stream(holehash, "wormhole").send({}));
      process.stdin.pipe(hole);
    } else console.error("This worm is:",worm.hashname);
  });

  worm.listen("wormhole", function(err, stream, js){
    stream.send({}); // ack
    worm.wrap(stream).pipe(process.stdout);
  });
}
