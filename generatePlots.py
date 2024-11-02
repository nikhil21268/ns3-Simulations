import re
import matplotlib.pyplot as plt

# Initialize lists to hold client IDs and completion times
client_ids = []
completion_times = []

# Read the terminal output from the file
with open('./scratch/forPlots/e3.txt', 'r') as f:
    lines = f.readlines()

# Regular expression pattern to match the lines
pattern = r'Client (\d+) completed at time ([\d.]+) seconds'

# Parse each line
for line in lines:
    match = re.search(pattern, line)
    if match:
        client_id = int(match.group(1))
        completion_time = float(match.group(2))
        client_ids.append(client_id)
        completion_times.append(completion_time)

# Check if data was found
if not client_ids:
    print("No completion times found in the file.")
    exit()

# Compute the average completion time
average_completion_time = sum(completion_times) / len(completion_times)

# Sort the clients by client ID for plotting
sorted_data = sorted(zip(client_ids, completion_times))
client_ids_sorted = [client for client, _ in sorted_data]
completion_times_sorted = [time for _, time in sorted_data]

# Plotting
plt.figure(figsize=(10, 6))
plt.plot(client_ids_sorted, completion_times_sorted, marker='o', linestyle='-', label='Client Completion Times')

# Plot the average completion time as a horizontal line
plt.axhline(y=average_completion_time, color='r', linestyle='--', label=f'Average Completion Time ({average_completion_time:.2f} s)')

# Add labels and title
plt.xlabel('Client ID')
plt.ylabel('Completion Time (s)')
plt.title('Client Completion Times and Average Completion Time')
plt.xticks(client_ids_sorted)  # Ensure x-axis ticks are client IDs
plt.legend()
plt.grid(True)

# Display the plot
plt.show()
