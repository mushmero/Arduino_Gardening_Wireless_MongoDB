var mqtt = require('mqtt');
var mongodb = require('mongodb');
var mongoClient = mongodb.MongoClient;
var mongoURI = 'mongodb://username:password@host:port/database';
var deviceRoot = "arduino/sensors/"; //topic name
var collection, client;

var mqtt_user = ""; //mqtt broker account username
var mqtt_pass = ""; //mqtt broker account password
var mqtt_host = ""; //mqtt broker host
var mqtt_port = 17528; //mqtt broket port

mongoClient.connect(mongoURI, setupCollection);

function setupCollection(err, db) {
    if (err) throw err;
    collection = db.collection("sensors"); //name of the collection in the database
    client = mqtt.connect({ host: mqtt_host, port: mqtt_port, username: mqtt_user, password: mqtt_pass }); //connecting the mqtt server with the MongoDB database
    client.subscribe(deviceRoot + "+"); //subscribing to the topic name 
    client.on('message', insertEvent); //inserting the event
}

//function that displays the data in the MongoDataBase
function insertEvent(topic, message) {
    var key = topic.replace(deviceRoot, '');

    collection.update({ _id: key }, { $push: { events: { event: { value: message, when: new Date() } } } }, { upsert: true },

        function(err, docs) {
            if (err) {
                console.log("Insert fail") // Improve error handling		
            }
        }

    );

}