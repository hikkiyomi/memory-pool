import matplotlib.pyplot as plt

pool_allocator_xaxis, pool_allocator_yaxis = [], []
pool_allocator_xaxis_changed, pool_allocator_yaxis_changed = [], []
pool_manager_xaxis, pool_manager_yaxis = [], []
stl_allocator_xaxis, stl_allocator_yaxis = [], []

with open("stress-tests\PoolAllocator-vector.txt", "r") as f:
    for line in f:
        numbers = line.split()
        pool_allocator_xaxis.append(int(numbers[0]))
        pool_allocator_yaxis.append(float(numbers[1]))

with open("stress-tests\PoolAllocator-sizeof4-vector.txt", "r") as f:
    for line in f:
        numbers = line.split()
        pool_allocator_xaxis_changed.append(int(numbers[0]))
        pool_allocator_yaxis_changed.append(float(numbers[1]))

with open("stress-tests\PoolManager-vector.txt", "r") as f:
    for line in f:
        numbers = line.split()
        pool_manager_xaxis.append(int(numbers[0]))
        pool_manager_yaxis.append(float(numbers[1]))

with open("stress-tests\stl-allocator-vector.txt", "r") as f:
    for line in f:
        numbers = line.split()
        stl_allocator_xaxis.append(int(numbers[0]))
        stl_allocator_yaxis.append(float(numbers[1]))

fig, ax = plt.subplots()

ax.plot(pool_allocator_xaxis, pool_allocator_yaxis, label='Single Pool')
ax.plot(pool_allocator_xaxis_changed, pool_allocator_yaxis_changed, label='Single Pool (diff chunks)')
ax.plot(pool_manager_xaxis, pool_manager_yaxis, label='Pool Manager')
ax.plot(stl_allocator_xaxis, stl_allocator_yaxis, label='STL')

ax.set(xlabel="PushBacks", ylabel="Seconds", title="Allocators with vector")
ax.grid()
ax.legend()

fig.savefig("Allocators-vector.png")
plt.show()
