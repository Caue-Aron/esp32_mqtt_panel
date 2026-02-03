// Connect to public Mosquitto broker (WebSocket port)
const client = mqtt.connect("wss://test.mosquitto.org:8081");

// Topic must match ESP32
const TOPIC = "blumenau/aronimo/esp32";

// When connected
client.on("connect", () => {
    console.log("Connected to MQTT broker");
});

function onBlink() {
    const blinkTimes = document.getElementById("blink-times");
    const blinkDelay = document.getElementById("blink-delay");

    return JSON.stringify({
        cmd: "LED_CTRL",
        times: Number(blinkTimes.value) || 0,
        on_delay: blinkDelay.value / 2,
        off_delay: blinkDelay.value / 2
    })
}

function onLEDControl() {
    const ledControlTimes = document.getElementById("led-control-times");
    const ledControlOnTimer = document.getElementById("led-control-on-delay");
    const ledControlOffTimer = document.getElementById("led-control-off-delay");

    return JSON.stringify({
        cmd: "LED_CTRL",
        times: Number(ledControlTimes.value) || 0,
        on_delay: Number(ledControlOnTimer.value) || 0,
        off_delay: Number(ledControlOffTimer.value) || 0
    })
}

document.addEventListener("DOMContentLoaded", () => {
    const blinkBtn = document.getElementById("blink-btn");
    const ledControlBtn = document.getElementById("led-control-btn");

    blinkBtn.addEventListener("click", () => {
        client.publish(TOPIC, onBlink());
    });

    ledControlBtn.addEventListener("click", () => {
        client.publish(TOPIC, onLEDControl());
    });
});
