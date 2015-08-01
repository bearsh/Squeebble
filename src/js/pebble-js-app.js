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

	this.request = function(player, params, callback) {
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

	this.clearPlayList = function(callback) {
		self.rs.request(playerId, ["playlist", "clear"], callback);
	};

	this.getMode = function(callback) {
		self.rs.request(playerId, ["mode", "?"], callback);
	};

	this.setName = function(name, callback) {
		self.rs.request(playerId, ["name", name], callback);
	};

	this.getName = function(callback) {
		self.rs.request(playerId, ["name", "?"], callback);
	};

	this.getCurrentTitle = function(callback) {
		self.rs.request(playerId, ["current_title", "?"], function(reply) {
			if (reply.ok)
				reply.result = reply.result._current_title;
			callback(reply);
		});
	};

	this.getCurrentRemoteMeta = function(callback) {
		self.rs.request(playerId, ["status"], function(reply) {
			if (reply.ok)
				reply.result = reply.result.remoteMeta;
			callback(reply);
		});
	};

	this.getStatus = function(callback) {
		//self.rs.request(playerId, ['status', '-', 1, 'tags:cgABbehldiqtyrSuoKLNJ'], callback);
		self.rs.request(playerId, ['status', '-', 1, 'tags:alt'], callback);
		//self.rs.request(playerId, ["status"], callback);
	};

	this.getStatusWithPlaylist = function(from, to, callback) {
		self.rs.request(playerId, ["status", from, to], function(reply) {
			if (reply.ok)
				reply.result = reply.result;
			callback(reply);
		});
	};

	this.getPlaylist = function(from, to, callback) {
		self.rs.request(playerId, ["status", from, to], function(reply) {
			if (reply.ok)
				reply.result = reply.result.playlist_loop;
			callback(reply);
		});
	};

	this.play = function(callback) {
		self.rs.request(playerId, ["play"], callback);
	};

	this.playIndex = function(index, callback) {
		console.log("index: " + index);
		self.rs.request(playerId, ["playlist", "index", index], callback);
	};

	this.pause = function(callback) {
		self.rs.request(playerId, ["pause"], callback);
	};

	this.previous = function(callback) {
		self.rs.request(playerId, ["button", "jump_rew"], callback);
	};

	this.next = function(callback) {
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

	this.getPlayers = function(callback) {
		self.rs.request(defaultPlayer, ["players",0,100], function(reply) {
//			console.log("SqueezeServer:getPlayers| reply: " + JSON.stringify(reply));
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
		var ok = false;
		if (res.ok) {
			console.log("SqueezeServer: add following players");
			for (var pl in players) {
				console.log("  " + players[pl].name + ":");
				//debugger;
				if (!self.players[players[pl].playerid]) { // player not on the list
					//console.log("  - add " + players[pl].playerid + " to list");
					self.players[players[pl].playerid] = {"name": players[pl].name, "instance": null}; // set in list but don't create instance new SqueezePlayer(players[pl].playerid, players[pl].name, self.address, self.port);
					//console.log("    " + JSON.stringify(self.players[players[pl].playerid]));
				}
			}
			ok = true;
		}
		if (callback) {
			callback(ok, self.players);
		}
	});
}


var SC_STATUS     = 1;
var SC_PLAY       = 2;
var SC_PAUSE      = 3;
var SC_NEXT       = 4;
var SC_PREV       = 5;
var SC_VOLUME     = 6;

var SC_PLAYERS    = 10;
var SC_SEL_PLAYER = 11;

var server_address = "192.168.11.140";
var server_port = 9000;
var squeeze_request = null;
var player = null;


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
		function(ok, pl) {
			if (!ok) {
				console.log("error retrieving players");
				Pebble.sendAppMessage({'command': SC_PLAYERS, 'status': 100});
				return;
			}
			console.log("got player list: " + JSON.stringify(pl));

			var pa = [];
			var nb = 0;
			for (var mac in pl) {
				pa.push(mac + '\0');
				pa.push(pl[mac].name + '\0')
				nb += 1;
			}
			pa.unshift(nb);
			//console.log("list: " + pa);
			// send it to watch
			Pebble.sendAppMessage({'command': SC_PLAYERS, 'status': 0, 'message': pa});
			// select default player ??
			//player = Squeeze_server.getSqueezePlayer("00:00:00:12:34:56"); //00:04:20:28:99:14");
		});
}

// Called when JS is ready
Pebble.addEventListener("ready", readyListener);


function appmessageListener(event) {
	//console.log("Received Status: " + event.payload.status);
	//console.log("Received Cmd: " + event.payload.command);
	//console.log("Received Msg: " + event.payload.message);

	switch(event.payload.command) {
		case SC_STATUS:
			console.log("Cmd status:");
			//player.getStatus(function(e) {console.log(JSON.stringify(e))})
			//player.getCurrentTitle(function(e) {console.log(JSON.stringify(e))})
			/*
			Remode media:
			{"params":["00:00:00:12:34:56",["status"]],"method":"slim.request","id":1,"result":{
				"player_name":"testus","player_connected":1,"player_ip":"10.10.10.10:35419",
				"power":1,"signalstrength":0,"mode":"play","remote":1,
				"current_title":"Secret Agent: The soundtrack for your stylish, mysterious, dangerous life. For Spys and P.I.'s too! [SomaFM]",
				"time":21.8797190208435,"rate":1,"mixer volume":50,"playlist repeat":0,
				"playlist shuffle":0,"playlist mode":"off","seq_no":0,
				"playlist_cur_index":"0","playlist_timestamp":1436353762.43872,
				"playlist_tracks":1,
				"remoteMeta":{
					"id":"-88930184",
					"title":"Dirty Listening",
					"artist":"Groove Armada",
					"duration":"0"
				}
			},"ok":true}
			Local media:
			{"params":["00:00:00:12:34:56",["status"]],"method":"slim.request","id":1,"result":{
				"player_name":"testus","player_connected":1,"player_ip":"192.168.11.144:43665",
				"power":1,"signalstrength":0,"mode":"play",
				"time":30.8910992565155,"rate":1,"duration":300.053,"can_seek":1,"mixer volume":50,"playlist repeat":0,
				"playlist shuffle":0,"playlist mode":"off","seq_no":0,
				"playlist_cur_index":"6","playlist_timestamp":1436378666.19916,
				"playlist_tracks":13,
				"playlist_loop":[{
					"playlist index":5,"id":850,
					"title":"Collapse collide",
					"artist":"Archive",
					"album":"Controlling Crowds"
				}]
			},"ok":true}

			title      string
			artist     string
			album      string
			time       int   (sec)
			duration   int   (sec)
			volume     int
			mode       ??
			track      string ('tracknum'/'playlist_tracks')

			*/
			player.getStatus(function(st) {
				console.log(JSON.stringify(st));
				var reply = {}
				reply.command = SC_STATUS;

				if (!st.ok) {
					reply.status = 110;
					Pebble.sendAppMessage(reply);
					return;
				}
				reply.status = 0;

				if (st.result.remote && st.result.remote == 1) {
					if (st.result.current_title) {
						reply.ctitel = st.result.current_title;
					}
					if (st.result.remoteMeta.title) {
						reply.title = st.result.remoteMeta.title;
					}
					if (st.result.remoteMeta.artist) {
						reply.artist = st.result.remoteMeta.artist;
					}
					if (st.result.remoteMeta.album) {
						reply.album = st.result.remoteMeta.album;
					}
					if (st.result.remoteMeta.duration) {
						reply.duration = st.result.remoteMeta.duration;
					}
				} else if (st.result.playlist_loop && st.result.playlist_loop[0]) {
					if (st.result.current_title) {
						reply.ctitel = st.result.current_title;
					}
					if (st.result.playlist_loop[0].title) {
						reply.title = st.result.playlist_loop[0].title;
					}
					if (st.result.playlist_loop[0].artist) {
						reply.artist = st.result.playlist_loop[0].artist;
					}
					if (st.result.playlist_loop[0].album) {
						reply.album = st.result.playlist_loop[0].album;
					}
					if (st.result.duration) {
						reply.duration = st.result.duration;
					}
					if (st.result.playlist_tracks && st.result.playlist_loop[0].tracknum) {
						reply.track = String(st.result.playlist_loop[0].tracknum) + '/' + String(st.result.playlist_tracks)
					}
				}

				reply.time = st.result.time
				reply.volume = st.result["mixer volume"]
				reply.mode = st.result.mode

				console.log(JSON.stringify(reply))
				Pebble.sendAppMessage(reply);
			});
			break;
		case SC_PLAY:
			console.log("Cmd play:");
			player.play(function(e) {console.log(JSON.stringify(e))})
			break;
		case SC_PAUSE:
			console.log("Cmd pause:");
			player.pause(function(e) {console.log(JSON.stringify(e))})
			break;
		case SC_VOLUME:
			var vol = event.payload.message;
			console.log("Cmd Vol up:" + vol);
			player.setVolume(vol, function(e) {console.log(JSON.stringify(e))});
			break;
		case SC_SEL_PLAYER: // sel player
			var mac = event.payload.message;
			console.log("Cmd set player: '" + mac + "' -> '" + Squeeze_server.players[mac].name + "'");
			player = Squeeze_server.getSqueezePlayer(mac);
			break;
		default:
			console.log("unhandled cmd: " + event.payload.command);
	}
	//sendMessage();
}

// Called when incoming message from the Pebble is received
Pebble.addEventListener("appmessage", appmessageListener);
