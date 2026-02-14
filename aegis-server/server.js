require('dotenv').config();
const express = require('express');
const mqtt = require('mqtt');
const TelegramBot = require('node-telegram-bot-api');

const app = express();

const bot = new TelegramBot(process.env.TELEGRAM_TOKEN);

const client = mqtt.connect(process.env.MQTT_BROKER);

client.on('connect', () => {
    console.log("Connected to MQTT Broker");
    client.subscribe("aegis/machine01/alert");
});

client.on('message', (topic, message) => {
    console.log("Alert Received:", message.toString());

    bot.sendMessage(process.env.CHAT_ID,
        `🚨 ALERT RECEIVED\n\n${message.toString()}`
    );
});

app.listen(3000, () => {
    console.log("Server running on port 3000");
});
