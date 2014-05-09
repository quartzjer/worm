var fs = require("fs");
var path = require("path-extra");
var tele = require("telehash");
var argv = require("optimist")
  .usage("Usage: type '$0' to receive (prints it's hashname) and 'cat foo.txt | $0 hashdestination' to send")
  .boolean("v").describe("v", "verbose")
  .argv;

if(argv.v) tele.debug(console.log);
var holehash = argv._[0];

var base = (holehash) ? "base" : holehash;
argv.id = (argv.id) ? path.resolve(argv.id) : path.join(path.homedir(),".worm_"+base+".json");
if(argv.seeds) argv.seeds = path.resolve(argv.seeds);

function link(sock)
{
  if(!sock) return console.error("invalid destination hashname",holehash);
  process.stdin.pipe(sock);
  sock.pipe(process.stdout);
}
var worm = tele.init(argv, function(err){
  if(err) return console.error("error initializing",err);
  worm.onSock(link);
  if(holehash) link(worm.sock(holehash));
  else console.error("worm listening at",worm.hashname);
});

