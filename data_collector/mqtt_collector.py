#!/usr/bin/env python3
"""
MQTT Wind Data Collector Script
Subscribes to an MQTT topic, collects 200 wind readings in format {speed:X.XX,direction:XXX}, 
and saves them to CSV with separate wind speed and direction statistics.
"""

import paho.mqtt.client as mqtt
import csv
import time
import statistics
from datetime import datetime
import sys

class MQTTDataCollector:
    def __init__(self, host, port=1883):
        self.host = host
        self.port = port
        self.client = mqtt.Client()
        self.data = []
        self.start_time = None
        self.csv_filename = None
        self.topic = None
        self.target_count = 200
        
        # Setup MQTT callbacks
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.client.on_disconnect = self.on_disconnect
    
    def on_connect(self, client, userdata, flags, rc):
        """Callback for when client connects to MQTT broker"""
        if rc == 0:
            print(f"✓ Connected to MQTT broker at {self.host}")
            print(f"✓ Subscribing to topic: {self.topic}")
            client.subscribe(self.topic)
            self.start_time = time.time()
            print(f"✓ Started collecting wind data at {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
            print(f"Waiting for {self.target_count} wind readings...")
            print("Expected format: {speed:X.XX,direction:XXX}")
            print("(Invalid format messages will be skipped)")
        else:
            print(f"✗ Failed to connect to MQTT broker. Return code: {rc}")
            sys.exit(1)
    
    def parse_wind_data(self, message):
        """Parse wind data from format: {speed:5.50,direction:126}"""
        try:
            # Remove curly braces and split by comma
            cleaned = message.strip('{}')
            parts = cleaned.split(',')
            
            speed = None
            direction = None
            
            for part in parts:
                if ':' in part:
                    key, value = part.split(':', 1)
                    key = key.strip()
                    value = value.strip()
                    
                    if key == 'speed':
                        speed = float(value)
                    elif key == 'direction':
                        direction = float(value)
            
            if speed is not None and direction is not None:
                return speed, direction
            else:
                return None, None
                
        except (ValueError, AttributeError):
            return None, None
    
    def on_message(self, client, userdata, msg):
        """Callback for when a message is received"""
        message = msg.payload.decode().strip()
        timestamp = datetime.now().isoformat()
        
        # Parse wind data
        speed, direction = self.parse_wind_data(message)
        
        if speed is not None and direction is not None:
            self.data.append({
                'timestamp': timestamp,
                'topic': msg.topic,
                'wind_speed': speed,
                'wind_direction': direction
            })
            
            print(f"Received reading {len(self.data)}/{self.target_count}: Speed={speed}, Direction={direction}")
            
            # Check if we've collected enough data
            if len(self.data) >= self.target_count:
                self.save_data_and_finish()
        else:
            print(f"Skipped invalid format: {message}")
            # Don't increment counter for invalid messages
    
    def on_disconnect(self, client, userdata, rc):
        """Callback for when client disconnects"""
        if rc != 0:
            print("✗ Unexpected disconnection from MQTT broker")
    
    def save_data_and_finish(self):
        """Save collected data to CSV and display statistics"""
        # Save to CSV
        with open(self.csv_filename, 'w', newline='') as csvfile:
            fieldnames = ['timestamp', 'topic', 'wind_speed', 'wind_direction']
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            writer.writeheader()
            writer.writerows(self.data)
        
        # Calculate statistics
        end_time = time.time()
        duration = end_time - self.start_time
        
        print(f"\n{'='*50}")
        print("DATA COLLECTION COMPLETE!")
        print(f"{'='*50}")
        print(f"📁 File saved as: {self.csv_filename}")
        print(f"📊 Total readings collected: {len(self.data)}")
        print(f"⏱️  Time taken: {duration:.2f} seconds")
        print(f"📈 Collection rate: {len(self.data)/duration:.2f} readings/second")
        
        # Extract wind speed and direction values
        wind_speeds = [item['wind_speed'] for item in self.data]
        wind_directions = [item['wind_direction'] for item in self.data]
        
        if wind_speeds and wind_directions:
            print(f"\n🌬️  WIND SPEED STATISTICS:")
            print(f"   Average: {statistics.mean(wind_speeds):.3f}")
            print(f"   Min: {min(wind_speeds):.3f}")
            print(f"   Max: {max(wind_speeds):.3f}")
            if len(wind_speeds) > 1:
                print(f"   Standard deviation: {statistics.stdev(wind_speeds):.3f}")
            
            print(f"\n🧭 WIND DIRECTION STATISTICS:")
            print(f"   Average: {statistics.mean(wind_directions):.1f}°")
            print(f"   Min: {min(wind_directions):.1f}°")
            print(f"   Max: {max(wind_directions):.1f}°")
            if len(wind_directions) > 1:
                print(f"   Standard deviation: {statistics.stdev(wind_directions):.1f}°")
        
        print(f"{'='*50}")
        
        # Disconnect and stop
        self.client.disconnect()
        self.client.loop_stop()
    
    def collect_data(self, topic, csv_filename):
        """Main method to start data collection"""
        self.topic = topic
        self.csv_filename = csv_filename
        
        try:
            print(f"🔌 Connecting to MQTT broker: {self.host}:{self.port}")
            self.client.connect(self.host, self.port, 60)
            
            # Start the network loop
            self.client.loop_forever()
            
        except KeyboardInterrupt:
            print("\n\n⚠️  Collection interrupted by user")
            if self.data:
                print(f"Saving {len(self.data)} collected values...")
                self.save_data_and_finish()
            else:
                print("No data collected.")
        except Exception as e:
            print(f"✗ Error: {e}")
            sys.exit(1)

def main():
    print("MQTT Wind Data Collector")
    print("=" * 30)
    
    # Get CSV filename from user
    csv_filename = input("Enter the CSV filename to save wind data (e.g., 'wind_data.csv'): ").strip()
    
    # Add .csv extension if not provided
    if not csv_filename.endswith('.csv'):
        csv_filename += '.csv'
    
    # Get MQTT topic from user
    topic = input("Enter the MQTT topic to subscribe to (e.g., 'weather/wind'): ").strip()
    
    if not topic:
        print("✗ Topic cannot be empty!")
        sys.exit(1)
    
    # Confirm settings
    print(f"\nSettings:")
    print(f"  MQTT Host: mqtt.cetools.org")
    print(f"  Topic: {topic}")
    print(f"  CSV File: {csv_filename}")
    print(f"  Target readings: 200")
    print(f"  Expected format: {{speed:X.XX,direction:XXX}}")
    
    confirm = input("\nProceed? (y/N): ").strip().lower()
    if confirm != 'y' and confirm != 'yes':
        print("Collection cancelled.")
        sys.exit(0)
    
    # Create collector and start
    collector = MQTTDataCollector("mqtt.cetools.org")
    collector.collect_data(topic, csv_filename)

if __name__ == "__main__":
    main()