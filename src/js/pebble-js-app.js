// creates a squeezebox jsonrpc request
function SqueezeRequest(address, port) {
	this.address = address;
	this.port = (port !== undefined) ? port : 9000;
	var req;

	function handle(cb) {
		var result = {};
		if (req.readyState == 4 && cb) {
			if (req.status == 200) {
				result = JSON.parse(req.responseText);
//				debugger;
				result.ok = true;
			} else {
				result.ok = false;
			}
			cb(result);
		}
	}

	this.request = function (player, params, callback) {
		var p = '["' + player + '",' + JSON.stringify(params) + ']';
		var data = '{"id":1,"method":"slim.request","params":' + p + '}';
		console.log('data: ' + data);
		req = new XMLHttpRequest();
		req.open('POST', 'http://' + this.address + ':' + this.port + '/jsonrpc.js', true);
		req.setRequestHeader("Content-type", "application/x-www-form-urlencoded; charset=UTF-8");
		req.onreadystatechange = function() { handle(callback); };
		req.send(data);
	};
}


function SqueezePlayer(playerId, name, request_server) {
	this.playerId = playerId;
	this.name = name;
	this.rs = request_server;
	var self = this;

	this.clearPlayList = function (callback) {
		self.rs.request(playerId, ["playlist", "clear"], callback);
	};

	this.getMode = function (callback) {
		self.rs.request(playerId, ["mode", "?"], callback);
	};

	this.setName = function (name, callback) {
		self.rs.request(playerId, ["name", name], callback);
	};

	this.getName = function (callback) {
		self.rs.request(playerId, ["name", "?"], callback);
	};

	this.getCurrentTitle = function (callback) {
		self.rs.request(playerId, ["current_title", "?"], function (reply) {
			if (reply.ok)
				reply.result = reply.result._current_title;
			callback(reply);
		});
	};

	this.getCurrentRemoteMeta = function (callback) {
		self.rs.request(playerId, ["status"], function (reply) {
			if (reply.ok)
				reply.result = reply.result.remoteMeta;
			callback(reply);
		});
	};

	this.getStatus = function (callback) {
		self.rs.request(playerId, ["status"], callback);
	};

	this.getStatusWithPlaylist = function (from, to, callback) {
		self.rs.request(playerId, ["status", from, to], function (reply) {
			if (reply.ok)
				reply.result = reply.result;
			callback(reply);
		});
	};

	this.getPlaylist = function (from, to, callback) {
		self.rs.request(playerId, ["status", from, to], function (reply) {
			if (reply.ok)
				reply.result = reply.result.playlist_loop;
			callback(reply);
		});
	};

	this.play = function (callback) {
		self.rs.request(playerId, ["play"], callback);
	};

	this.playIndex = function (index, callback) {
		console.log("index: " + index);
		self.rs.request(playerId, ["playlist", "index", index], callback);
	};

	this.pause = function (callback) {
		self.rs.request(playerId, ["pause"], callback);
	};

	this.next = function (callback) {
		self.rs.request(playerId, ["button", "jump_rew"], callback);
	};

	this.previous = function (callback) {
		self.rs.request(playerId, ["button", "jump_rew"], callback);
	};

	this.next = function (callback) {
		self.rs.request(playerId, ["button", "jump_fwd"], callback);
	};

	this.playlistDelete = function(index, callback) {
		self.rs.request(playerId, ["playlist", "delete", index], callback);
	};

	this.playlistMove = function(fromIndex, toIndex, callback) {
		self.rs.request(playerId, ["playlist", "move", fromIndex, toIndex], callback);
	};

	this.playlistSave = function(playlistName, callback) {
		self.rs.request(playerId, ["playlist", "save", playlistName], callback);
	};

	this.sync = function(syncTo, callback) {
		self.rs.request(playerId, ["sync", syncTo], callback);
	};

	this.unSync = function(callback) {
		self.rs.request(playerId, ["sync", "-"], callback);
	};

	this.seek = function(seconds, callback) {
		self.rs.request(playerId, ["time", seconds], callback);
	};

	this.setVolume = function(volume, callback) {
		self.rs.request(playerId, ["mixer", "volume", volume], callback);
	};
}


function SqueezeServer(address, port, callback) {
	this.rs = new SqueezeRequest(address, port);
	this.players = {};
	var self = this;
	var defaultPlayer = ""; //"00:00:00:00:00:00";

	this.getPlayers = function (callback) {
		self.rs.request(defaultPlayer, ["players",0,100], function (reply) {
//			console.log("SqueezeServer:getPlayers| repy: " + JSON.stringify(reply));
			if (reply.ok) {
				reply.result = reply.result.players_loop;
//				console.log("SqueezeServer:getPlayers| result: " + JSON.stringify(reply.result));
			}
			callback(reply);
		});
	};

	this.getSqueezePlayer = function(id) {
		if (!self.players[id].instance) {
			self.players[id].instance = new SqueezePlayer(id, self.players[id].name, self.rs);
		}
		return self.players[id].instance;
	}

	// query server for available players
	self.getPlayers(function(res) {
		var players = res.result;
		console.log("SqueezeServer: add following players");
		for (var pl in players) {
			console.log("  " + players[pl].name + ":");
			debugger;
			if (!self.players[players[pl].playerid]) { // player not on the list
				console.log("  - add " + players[pl].playerid + " to list");
				self.players[players[pl].playerid] = {"name": players[pl].name, "instance": null}; // set in list but don't create instance new SqueezePlayer(players[pl].playerid, players[pl].name, self.address, self.port);
				console.log("    " + JSON.stringify(self.players[players[pl].playerid]));
			}
		}
		if (callback) {
			callback(self.players);
		}
	});
}


var SC_STATUS     = 1;
var SC_PLAY       = 2;
var SC_PAUSE      = 3;
var SC_NEXT       = 4;
var SC_PREV       = 5;
var SC_VOL_UP     = 6;
var SC_VOL_DOwN   = 7;

var SC_PLAYERS    = 10;
var SC_SEL_PLAYER = 11;

var server_address = "192.168.11.140";
var server_port = 9000;
var squeeze_request = null;
var player = null;

function commandHandler(cmd) {
	
}


// Function to send a message to the Pebble using AppMessage API
function sendMessage() {
	Pebble.sendAppMessage({"status": 0});
	
	// PRO TIP: If you are sending more than one message, or a complex set of messages, 
	// it is important that you setup an ackHandler and a nackHandler and call 
	// Pebble.sendAppMessage({ /* Message here */ }, ackHandler, nackHandler), which 
	// will designate the ackHandler and nackHandler that will be called upon the Pebble 
	// ack-ing or nack-ing the message you just sent. The specified nackHandler will 
	// also be called if your message send attempt times out.
}


function readyListener(event) {
	console.log("create server object")
	Squeeze_server = new SqueezeServer(server_address, server_port,
		function(pl) {

			var pa = [];
			var nb = 0;
			for (var mac in pl) {
				pa.push(mac + '\0');
				pa.push(pl[mac].name + '\0')
				nb += 1;
			}
			pa.unshift(nb);
			console.log("list: " + pa);
			console.log("got player list: " + JSON.stringify(pl));
			// send it to watch
			Pebble.sendAppMessage({'command': SC_PLAYERS, 'message': pa});
			// select default player ??
			//player = Squeeze_server.getSqueezePlayer("00:00:00:12:34:56"); //00:04:20:28:99:14");
		});
}

// Called when JS is ready
Pebble.addEventListener("ready", readyListener);


function appmessageListener(event) {
	console.log("Received Status: " + event.payload.status);
	console.log("Received Cmd: " + event.payload.command);

	switch(event.payload.command) {
		case 0:
			console.log("status...");
			player.getStatus(function(e) {console.log(JSON.stringify(e))})
			break;
		case 1:
			console.log("play...");
			player.play(function(e) {console.log(JSON.stringify(e))})
			break;
		case 2:
			console.log("pause...");
			player.pause(function(e) {console.log(JSON.stringify(e))})
			break;
		default:
			console.log("???...");
	}
	//sendMessage();
}

// Called when incoming message from the Pebble is received
Pebble.addEventListener("appmessage", appmessageListener);
