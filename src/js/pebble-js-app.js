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
		var p = '["' + player + '",' + '["' + params + '"]]';
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
	var rs = request_server;

	this.clearPlayList = function (callback) {
		rs.request(playerId, ["playlist", "clear"], callback);
	};

	this.getMode = function (callback) {
		rs.request(playerId, ["mode", "?"], callback);
	};

	this.setName = function (name, callback) {
		rs.request(playerId, ["name", name], callback);
	};

	this.getName = function (callback) {
		rs.request(playerId, ["name", "?"], callback);
	};

	this.getCurrentTitle = function (callback) {
		rs.request(playerId, ["current_title", "?"], function (reply) {
			if (reply.ok)
				reply.result = reply.result._current_title;
			callback(reply);
		});
	};

	this.getCurrentRemoteMeta = function (callback) {
		rs.request(playerId, ["status"], function (reply) {
			if (reply.ok)
				reply.result = reply.result.remoteMeta;
			callback(reply);
		});
	};

	this.getStatus = function (callback) {
		rs.request(playerId, ["status"], callback);
	};

	this.getStatusWithPlaylist = function (from, to, callback) {
		rs.request(playerId, ["status", from, to], function (reply) {
			if (reply.ok)
				reply.result = reply.result;
			callback(reply);
		});
	};

	this.getPlaylist = function (from, to, callback) {
		rs.request(playerId, ["status", from, to], function (reply) {
			if (reply.ok)
				reply.result = reply.result.playlist_loop;
			callback(reply);
		});
	};

	this.play = function (callback) {
		rs.request(playerId, ["play"], callback);
	};

	this.playIndex = function (index, callback) {
		console.log("index: " + index);
		rs.request(playerId, ["playlist", "index", index], callback);
	};

	this.pause = function (callback) {
		rs.request(playerId, ["pause"], callback);
	};

	this.next = function (callback) {
		rs.request(playerId, ["button", "jump_rew"], callback);
	};

	this.previous = function (callback) {
		rs.request(playerId, ["button", "jump_rew"], callback);
	};

	this.next = function (callback) {
		rs.request(playerId, ["button", "jump_fwd"], callback);
	};

	this.playlistDelete = function(index, callback) {
		rs.request(playerId, ["playlist", "delete", index], callback);
	};

	this.playlistMove = function(fromIndex, toIndex, callback) {
		rs.request(playerId, ["playlist", "move", fromIndex, toIndex], callback);
	};

	this.playlistSave = function(playlistName, callback) {
		rs.request(playerId, ["playlist", "save", playlistName], callback);
	};

	this.sync = function(syncTo, callback) {
		rs.request(playerId, ["sync", syncTo], callback);
	};

	this.unSync = function(callback) {
		rs.request(playerId, ["sync", "-"], callback);
	};

	this.seek = function(seconds, callback) {
		rs.request(playerId, ["time", seconds], callback);
	};

	this.setVolume = function(volume, callback) {
		rs.request(playerId, ["mixer", "volume", volume], callback);
	};
}


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
	console.log("Ready");
	squeeze_request = new SqueezeRequest(server_address, server_port);
	console.log("Request object created");



	player = new SqueezePlayer("00:04:20:28:99:14", "Squeeze", squeeze_request)
	console.log("Player object created: " + player);
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
