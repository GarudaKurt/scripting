#!/usr/bin/env ruby
require "mqtt"
require "json"
require "time"

BROKER_HOST = "broker.hivemq.com"
BROKER_PORT = 1883
TOPIC = "embedded/attendance"
LOG_FILE = "attendance_events.log"

def log(message)
  timestamp = Time.now.strftime("%Y-%m-%d %H:%M:%S")
  line = "[#{timestamp}] #{message}"

  puts line
  File.open(LOG_FILE, "a") { |f| f.puts(line) }
end

def classify_event(data)
  if data["timeIn"] && !data["timeIn"].to_s.empty?
    :time_in
  elsif data["timeOut"] && !data["timeOut"].to_s.empty?
    :time_out
  else
    :unknown
  end
end

def handle_message(raw_payload)
  data = JSON.parse(raw_payload)

  event_type = classify_event(data)
  device_id = data["device_id"] || "unknown"
  employee_id = data["employeeId"] || "unknown"

  case event_type
  when :time_in
    log("TIME_IN  | device=#{device_id} | employee=#{employee_id} | time=#{data['timeIn']}")
  when :time_out
    log("TIME_OUT | device=#{device_id} | employee=#{employee_id} | time=#{data['timeOut']}")
  else
    log("UNKNOWN EVENT | raw=#{data.inspect}")
  end

rescue JSON::ParserError => e
  log("ERROR | Malformed JSON payload: #{e.message} | raw=#{raw_payload}")
rescue => e
  log("ERROR | Unexpected error processing message: #{e.class} - #{e.message}")
end

log("Connecting to #{BROKER_HOST}:#{BROKER_PORT}...")

begin
  MQTT::Client.connect(host: BROKER_HOST, port: BROKER_PORT) do |client|
    client.subscribe(TOPIC)
    log("Connected. Subscribed to '#{TOPIC}'. Listening for events... (Ctrl+C to stop)")

    client.get do |topic, raw_payload|
      handle_message(raw_payload)
    end
  end
rescue Interrupt
  log("Observer stopped by user (Ctrl+C).")
rescue MQTT::Exception, Errno::ECONNREFUSED => e
  log("CRITICAL | MQTT connection error: #{e.message}")
rescue => e
  log("CRITICAL | Unexpected error: #{e.class} - #{e.message}")
end