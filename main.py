from scapy.all import rdpcap
import matplotlib.pyplot as plt

# Load pcap file
file = "thread_500.pcapng"
packets = rdpcap(file)

# Initialize dictionaries to store data for each flow
data_size = {}
start_time = {}
end_time = {}

# Iterate over each packet
for packet in packets:
    if 'TCP' in packet:
        ip_layer = packet['IP']
        tcp_layer = packet['TCP']

        flow = (ip_layer.src, ip_layer.dst, tcp_layer.sport, tcp_layer.dport)

        # Calculate data size
        payload_len = len(packet['TCP'].payload)
        data_size[flow] = data_size.get(flow, 0) + payload_len

        # Calculate time
        if flow not in start_time:
            start_time[flow] = packet.time
        end_time[flow] = packet.time

# Initialize lists to store throughput and latency values
throughputs = []
latencies = []

# Calculate average throughput and latency for each flow
for flow, size in data_size.items():
    start = start_time[flow]
    end = end_time[flow]

    if end - start == 0:
        continue

    throughput = size * 8 / (end - start)  # bits/sec
    latency = (end - start) * 1000 / 2  # milliseconds

    print(f'Flow: {flow}, Throughput: {throughput} bits/sec, Latency: {latency} ms')

    # Add throughput and latency to lists
    throughputs.append(throughput)
    latencies.append(latency)

# Create throughput plot
plt.figure()
plt.plot(throughputs, color='r')
plt.title(f'Avg Throughput (bits/sec) through flow - {file}')
plt.xlabel('Flow')
plt.ylabel('Throughput (bits/sec)')
plt.show()

# Create latency plot
plt.figure()
plt.plot(latencies)
plt.title(f'Avg Latency (ms) through flow - {file}')
plt.xlabel('Flow')
plt.ylabel('Latency (ms)')
plt.show()
