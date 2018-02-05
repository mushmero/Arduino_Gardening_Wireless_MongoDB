var mqtt = require('mqtt');
var mongodb = require('mongodb');
var moment = require('moment');
var mongoClient = mongodb.MongoClient;
var mongoURI = 'mongodb://username:password@host:port/database';
var topic = '/research/'; //topic name based on arduino code
var collection, client;

var timestamp = moment().format('D-M-Y hh:mm:ss A').utcOffset("+8.00");

function insertEvent(topic, payload){
	mongoClient.connect(mongoURI, function(err,db){
		if(err){
			console.log(err);
			return;
		}else{
			var obj = JSON.parse(payload.toString());
			var key = topic.replace(topic,'research-data-date');
			collection = db.collection(key);
			collection.insertOne(
				obj,
				function(err,docs){
					if(err){
						console.log("Insert failed: " +err+ "\n Timestamp: "+timestamp+ ", ");
					}else{
						console.log("Insert success! Timestamp: " +timestamp+ ", ");
						db.close();
					}
				});
		}
	});
}

client = mqtt.connect({host:'HOST', port:17528, username:'USERNAME',password:'PASSWORD'});
client.on('connect',function(){
	client.subscribe(topic);
});

client.on('message',insertEvent);